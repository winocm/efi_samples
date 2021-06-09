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

  openvol.c
  
Abstract:

  OpenVolume() function of Simple File System Protocol

Revision History

--*/

#include "fat.h"


EFI_STATUS
EFIAPI
FatOpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE                        **File
  )
/*++

Routine Description:
  Implements Simple File System Protocol interface function OpenVolume().
  
Arguments:
  This          - Calling context.
  File          - the Root Directory of the volume

Returns:
  Status Code.
  
--*/
{
  EFI_STATUS                  Status;
  EFI_STATUS                  Status1;
  FAT_VOLUME                  *Vol;
  FAT_OFILE                   *Root;
  FAT_IFILE                   *IFile;

  Vol = CR(This, FAT_VOLUME, VolInterface, FAT_VOLUME_SIGNATURE);
  FatAcquireLock ();

  //
  // If the root isn't already opened, open it now
  //

  if (!Vol->Root) {

    //
    // Read the partition header
    //
    Status = FatOpenDevice (Vol);
    if (EFI_ERROR(Status)) {
      goto Done;
    }
  
    //
    // The root directory on a fat device is different than
    // any other file.  It's information comes from the fat boot
    // sector.
  
    //
    // Allocate a special root ofile
    //
  
    Root = FatAllocateOFile (Vol);
    if (!Root) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
  
    Root->FileString = EfiLibAllocateZeroPool(26);
    if (!Root->FileString) {
      FatFreeOFile (Root);
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
    
    Root->DirType = IsDir;
    Root->Attributes = EFI_FILE_DIRECTORY;
  
    switch (Vol->FatType) {
      case FAT12:
      case FAT16:
        Root->IsFixedRootDir = TRUE;
        Root->FileSize = Vol->RootEntries * sizeof(FAT_DIRECTORY_ENTRY);
        break;
    
      case FAT32:
        Root->FileCluster = Vol->RootCluster;
        Root->FileCurrentCluster = Vol->RootCluster;
        break;
    
      default:
        Status = EFI_VOLUME_CORRUPTED;
        goto Done;
    }
  
    Root->FileSize = FatDirSize (Root);
  
    //
    // -------
    // Count the number of outstanding root OFile's there are.  This
    // tells us when all references to Vol are freed.  (We can "abandoned"
    // opened root OFiles from the Vol->Root pointer on MEDIA_CHANGE errors
    // such that we can open the root of the volume again with the new
    // media without waiting for the old root to need to close).  But we
    // can't free the Vol structure until all handles to the volume close.
    // -------
    // Above is the original comment that reflects the old thoughts
    // New implementation doesn't make use of RootCount
    //
  
    Vol->Root = Root;
  
    InsertHeadList (&Vol->CheckRef, &Root->CheckLink);
    DEBUG((EFI_D_INIT, "%HOpened EFI file system on blkdev %x%N\n", 
          Vol->Handle));
  }

  //
  // Open a new instance to the root 
  //
  Status = FatAllocateIFile(Vol->Root, &IFile);
  if (!EFI_ERROR(Status)) {
    *File = &IFile->Handle;
  }

Done:

  Status1 = FatCleanupVolume (Vol, Vol->Root, Status);
  FatReleaseLock();

  if (!EFI_ERROR(Status)) {
    Status = Status1;
  }

  return Status;
}

