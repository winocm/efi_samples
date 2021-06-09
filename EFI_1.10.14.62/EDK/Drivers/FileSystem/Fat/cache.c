/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  cache.c
    
Abstract:

  Cache implementation for EFI FAT File system driver



Revision History

--*/

#include "fat.h"


EFI_STATUS
FatGetCacheBuffer (
  IN  FAT_VOLUME         *Vol,
  IN  UINTN              PageNo,
  OUT CACHE_BUFFER       **CacheEntry
  );
  
EFI_STATUS
FatCacheFlushRange (
  IN FAT_VOLUME         *Vol,
  IN UINTN              StartPageNo,
  IN UINTN              EndPageNo
  );
  

EFI_STATUS
FatGetCacheBuffer (
  IN  FAT_VOLUME         *Vol,
  IN  UINTN              PageNo,
  OUT CACHE_BUFFER       **CacheEntry
  )
/*++

Routine Description:
  Get one cache entry by specified PageNo.
  
  If PageNo is in the cache return it in CacheEntry. If PageNo is not
  in the cache allocate a new entry and read the page pointed by PageNo 
  into it. If the read fails return an EFI_ERROR().

  1. If there is invalid entry left, a new CacheEntry will use this invalid 
     entry.
  2. If there is no invalid entries existing, the Least Recently Used valid
     entry is invalidated and returned. Before invalidating this entry, if this 
     entry is dirty need to write this entry back to disk first.
  
Arguments:
  Vol            - FAT file system volume.
  PageNo         - PageNo to match with the cache.
  CacheEntry     - Last PageNo to invalidate in the cache.

Returns: 
  EFI_SUCCESS  - Return CacheEntry buffer that matches PageNo
  other        - An error occurred reading the data

--*/
{
  EFI_STATUS    Status;
  UINTN         Index;
  UINTN         MaxLru;
  UINTN         MinLru;
  UINTN         MinLruIndex;
  UINTN         FreeIndex;
  UINTN         CacheIndex;
  EFI_TPL       EntryTpl;
  CACHE_BUFFER  *Cache;
  
  FreeIndex = (UINTN) -1;
  MinLru = (UINTN) -1;
  MinLruIndex = 0;
  MaxLru = 0;
  
  for (Index = 0; Index < FAT_CACHE_SIZE; Index++) {
    if (Vol->Cache[Index].Valid) {
      if (Vol->Cache[Index].PageNo == PageNo) {
        //
        // Hit in the cache
        //
        Vol->Cache[Index].Lru++;
        *CacheEntry = &Vol->Cache[Index];
        return EFI_SUCCESS;
      }
      
      if (Vol->Cache[Index].Lru > MaxLru) {
        //
        // Tally the MaxLru value in case we need to allocate a new entry
        //
        MaxLru = Vol->Cache[Index].Lru;
      }
      
      if (Vol->Cache[Index].Lru < MinLru) {
        MinLru = Vol->Cache[Index].Lru;
        MinLruIndex = Index;
      }
    } else {
      FreeIndex = Index;
    }
  }
  
  if (FreeIndex == -1) {
    //
    // Replace the Least Recently Used entry
    //
    CacheIndex = MinLruIndex;
  } else {
    //
    // Fill an invalid entry
    //
    CacheIndex = FreeIndex;
  }
  
  //
  // Mark Cache Update a critical section
  //
  EntryTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  Cache = &Vol->Cache[CacheIndex];
  
  //
  // If the selected cache entry is not from invalid entry 
  // (means using LRU mechanism) and it is dirty, then write back
  // to disk before doing read
  //
  if (Cache->Valid == TRUE && Cache->Dirty == TRUE) {
    
    //
    // If write failed, set the cache as invalid
    // Not returning here on error condition in case the media is readonly
    //
    Status = Vol->DiskIo->WriteDisk (
                              Vol->DiskIo, 
                              Vol->MediaId, 
                              DriverLibMultU64x32 (Cache->PageNo, Vol->CachePageSize), 
                              Vol->CachePageSize, 
                              Cache->Data
                             );
    if (EFI_ERROR(Status)) {
      Cache->Valid = FALSE;
    }    
  }

  if (Vol->Valid) {
    Status = Vol->DiskIo->ReadDisk (
                                  Vol->DiskIo, 
                                  Vol->MediaId, 
                                  DriverLibMultU64x32 (PageNo, Vol->CachePageSize), 
                                  Vol->CachePageSize, 
                                  Cache->Data
                                 );
  } else {
    Status = EFI_MEDIA_CHANGED;
  }
    
  if (EFI_ERROR (Status)) {
    gBS->RestoreTPL (EntryTpl);
    return Status;
  }
  
  Cache->Valid = TRUE;
  Cache->Dirty = FALSE;
  Cache->PageNo = PageNo;
  Cache->Lru = MaxLru;
      
  *CacheEntry = Cache;
  
  //
  // Exit Cache Update critical section
  //
  gBS->RestoreTPL (EntryTpl);
  
  return Status;
}

  
EFI_STATUS
FatCacheFlushRange (
  IN FAT_VOLUME         *Vol,
  IN UINTN              StartPageNo,
  IN UINTN              EndPageNo
  )
/*++

Routine Description:
  Flush any cache entry for StartPageNo through EndPageNo.
  
  Make all entries in this range invalid.
  
Arguments:
  Vol               - FAT file system volume.
  StartPageNo       - First PageNo to invalidate in the cache.
  EndPageNo         - Last PageNo to invalidate in the cache.

Returns: 
  None 

--*/
{
  EFI_STATUS Status;
  UINTN   Index;
  EFI_TPL EntryTpl;
  
  Status = EFI_SUCCESS;
  
  //
  // Make Cache Update a critical section
  //
  EntryTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);
  
  for (Index = 0; Index < FAT_CACHE_SIZE; Index++) {
    if (Vol->Cache[Index].PageNo >= StartPageNo &&
        Vol->Cache[Index].PageNo <= EndPageNo) {
      if (Vol->Cache[Index].Valid == TRUE) {
        //
        // Make all valid entries in this range invalid.
        //
        Vol->Cache[Index].Valid = FALSE;
      }
    }
  }
  
  //
  // Exit Cache Update critical section
  //
  gBS->RestoreTPL (EntryTpl);
  
  return Status;
}

  
EFI_STATUS
FatDiskIoReadVolume (
  IN FAT_VOLUME         *Vol,
  IN UINT64             Offset,
  IN UINTN              BufferSize,
  OUT VOID              *Buffer
  )
/*++
  Routine Description:

    Read BufferSize bytes from Offset into Buffer.

    Reads may require a read cache to support reads that are not aligned on 
    page boundaries. There are three cases:

      UnderRun - The first byte is not on a page boundary or the read request is
                 less than a page in length.

      Aligned  - A read of N contiguous pages.

      OverRun  - The last byte is not on a page boundary.


  Arguments:
    Vol        - FAT file system volume.
    Offset     - The starting byte offset to read from.
    BufferSize - Size of Buffer.
    Buffer     - Buffer containing read data.

  Returns:
    EFI_SUCCESS           - The data was read correctly from the device.
    EFI_DEVICE_ERROR      - The device reported an error while performing the 
                            read.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_INVALID_PARAMETER - The read request contains device addresses that are 
                            not valid for the device.

--*/
{
  EFI_STATUS    Status;
  UINTN         PageNo;
  UINTN         OverRunPageNo;
  UINTN         UnderRun;
  UINTN         OverRun;
  UINTN         WorkingBufferSize;
  UINT8         *WorkingBuffer;
  UINTN         Length;
  CACHE_BUFFER  *Cache;

  if (Vol->BlkIo->Media->MediaId != Vol->MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  if (!Vol->Valid) {
    return EFI_MEDIA_CHANGED;
  }
  
  WorkingBuffer = Buffer;
  WorkingBufferSize = BufferSize;

  PageNo = (UINTN) DriverLibDivU64x32 (Offset, Vol->CachePageSize, &UnderRun);
 
  Length = Vol->CachePageSize - UnderRun;

  Status = EFI_SUCCESS;
  
  if (UnderRun != 0) {
    //
    // Offset starts in the middle of a page, so read the entire page
    //
    Status = FatGetCacheBuffer (Vol, PageNo, &Cache);
    
    if (EFI_ERROR (Status)) {
      goto Done;
    }
    
    if (Length > BufferSize) {
      Length = BufferSize;
    }

    EfiCopyMem (WorkingBuffer, Cache->Data + UnderRun, Length);
    
    if (Length == BufferSize) {
      goto Done;
    }	
    
    WorkingBuffer += Length;
    WorkingBufferSize  -= Length;
    
    PageNo += 1;
  }

  OverRunPageNo = PageNo + 
            (UINTN) DriverLibDivU64x32 (WorkingBufferSize, Vol->CachePageSize, &OverRun);

    if (WorkingBufferSize >= Vol->CachePageSize) {
    //
    // If the DiskIo maps directly to a BlkIo device do the read
    //
    if (OverRun != 0) {
      WorkingBufferSize -= OverRun;
    }

    Status = Vol->DiskIo->ReadDisk (
                                    Vol->DiskIo, 
                                    Vol->MediaId, 
                                    DriverLibMultU64x32 (PageNo, Vol->CachePageSize),
                                    WorkingBufferSize, 
                                    WorkingBuffer
                                   );
    if (!Vol->Valid) {
      Status = EFI_MEDIA_CHANGED;
      goto Done;
    }    

    WorkingBuffer += WorkingBufferSize;
  } 


  if (OverRun != 0) {
    //
    // Last read is not a complete page
    //
    Status = FatGetCacheBuffer (Vol, OverRunPageNo, &Cache);
    
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    EfiCopyMem (WorkingBuffer, Cache->Data, OverRun);    
  }

Done:
  return Status;
}



EFI_STATUS
FatDiskIoWriteVolume (
  IN FAT_VOLUME         *Vol,
  IN UINT64             Offset,
  IN UINTN              BufferSize,
  IN VOID               *Buffer
  )
/*++
   
  Routine Description:

    Write BufferSize bytes from Offset into Buffer.

    Writes may require a read modify write to support writes that are not 
    aligned on page boundaries. There are three cases:
    
      UnderRun - The first byte is not on a page boundary or the write request
                 is less than a page in length. Read modify write is required.

      Aligned  - A write of N contiguous pages.

      OverRun  - The last byte is not on a page boundary. Read modified write 
                 required.

  Arguments:
    Vol        - FAT file system volume.
    Offset     - The starting byte offset to read from.
    BufferSize - Size of Buffer.
    Buffer     - Buffer containing read data.

  Returns:
    EFI_SUCCESS           - The data was written correctly to the device.
    EFI_WRITE_PROTECTED   - The device can not be written to.
    EFI_DEVICE_ERROR      - The device reported an error while performing the 
                            write.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_INVALID_PARAMETER - The write request contains device addresses that are
                            not valid for the device.

--*/
{
  EFI_STATUS    Status;
  UINTN         PageNo;
  UINTN         OverRunPageNo;
  UINTN         UnderRun;
  UINTN         OverRun;
  UINTN         WorkingBufferSize;
  UINT8         *WorkingBuffer;
  UINTN         Length;
  CACHE_BUFFER  *Cache;

  if (Vol->BlkIo->Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  if (Vol->BlkIo->Media->MediaId != Vol->MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  if (!Vol->Valid) {
    return EFI_MEDIA_CHANGED;
  }

  WorkingBuffer = Buffer;
  WorkingBufferSize = BufferSize;

  PageNo = (UINTN) DriverLibDivU64x32 (Offset, Vol->CachePageSize, &UnderRun);
 
  Length = Vol->CachePageSize - UnderRun;

  Status = EFI_SUCCESS;
  
  if (UnderRun != 0) {
    //
    // Offset starts in the middle of a page,
    // so read modify write to the entire page
    //
    Status = FatGetCacheBuffer (Vol, PageNo, &Cache);
    
    if (EFI_ERROR (Status)) {
      goto Done;
    }
    
    if (Length > BufferSize) {
      Length = BufferSize;
    }

    EfiCopyMem (Cache->Data + UnderRun, WorkingBuffer, Length);
  
    //
    // Do not need to write to disk directly, instead, only need
    // to set cache entry to dirty. Dirty cache entry will be written
    // back to disk when it is flushed.
    //
    Cache->Dirty = TRUE;
    
    if (Length == BufferSize) {
      goto Done;	
    }

    WorkingBuffer += Length;
    WorkingBufferSize  -= Length;

    PageNo += 1;
  }


  OverRunPageNo = PageNo + 
            (UINTN) DriverLibDivU64x32 (WorkingBufferSize, Vol->CachePageSize, &OverRun);

  if (WorkingBufferSize >= Vol->CachePageSize) {
    //
    // If the DiskIo maps directly to a BlkIo device do the write
    //
    if (OverRun != 0) {
      WorkingBufferSize -= OverRun;
    }

    Status = Vol->DiskIo->WriteDisk (
                                      Vol->DiskIo, 
                                      Vol->MediaId, 
                                      DriverLibMultU64x32 (PageNo, Vol->CachePageSize),
                                      WorkingBufferSize, 
                                      WorkingBuffer
                                    );
    if (!Vol->Valid) {
      Status = EFI_MEDIA_CHANGED;
      goto Done;
    }
    
    WorkingBuffer += WorkingBufferSize;

    //
    // If this write over laps a read buffer it must be flushed
    //
    FatCacheFlushRange (Vol, PageNo, OverRunPageNo);    
  } 

  if (OverRun != 0) {
    //
    // Last bit is not a complete block, so do a read modify write
    //
    Status = FatGetCacheBuffer (Vol, OverRunPageNo, &Cache);
    
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    EfiCopyMem (Cache->Data, WorkingBuffer, OverRun);    

    //
    // Do not need to write to disk directly, instead, only need
    // to set cache entry to dirty. Dirty cache entry will be written
    // back to disk when it is flushed.
    //
    Cache->Dirty = TRUE;
  }

Done:
  return Status;
}


EFI_STATUS
FatVolumeFlushCache (
  IN FAT_VOLUME         *Vol
  )
/*++
   
  Routine Description:

  Arguments:
    Vol        - FAT file system volume.

  Returns:
    EFI_STATUS
    
--*/
{
  EFI_STATUS Status;
  UINTN   Index;
  EFI_TPL EntryTpl;
  
  Status = EFI_SUCCESS;
  
  if (Vol->Valid == FALSE) {
    return EFI_MEDIA_CHANGED;
  }

  //
  // Make Cache Update a critical section
  //
  EntryTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  for (Index = 0; Index < FAT_CACHE_SIZE; Index++) {
    if (Vol->Cache[Index].Valid == TRUE) {
      if (Vol->Cache[Index].Dirty == TRUE) {
        //
        // Write back all Dirty cache entries to disk
        //
        Status = Vol->DiskIo->WriteDisk (
                      Vol->DiskIo, 
                      Vol->MediaId, 
                      DriverLibMultU64x32 (Vol->Cache[Index].PageNo, Vol->CachePageSize), 
                      Vol->CachePageSize, 
                      Vol->Cache[Index].Data
                    );
        if (EFI_ERROR (Status)) {
          Vol->Cache[Index].Valid = FALSE;
          gBS->RestoreTPL (EntryTpl);
          return Status;
        }

        Vol->Cache[Index].Dirty = FALSE;
      }
    }
  }
  
  //
  // Exit Cache Update critical section
  //
  gBS->RestoreTPL (EntryTpl);
  
  return Status;
}
