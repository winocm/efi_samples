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

  dirsup.c

Abstract:

  Functions for manipulating directory entries

Revision History

--*/

#include "fat.h"



VOID
FatPackDirEntry (
  IN FAT_OFILE        *OFile
  )
{
  FAT_DIRECTORY_ENTRY *Dir;
  FAT_DATE_TIME       FatTime;

  //
  // Create the dir entry for the file
  //

  Dir = &OFile->u.DirEntry;
  EfiZeroMem (Dir, sizeof(FAT_DIRECTORY_ENTRY));
  
  EfiCopyMem (Dir->FileName, OFile->File8Dot3Name, sizeof(Dir->FileName));
  
  //
  // Convert the first char as required by FAT spec
  //
  if (Dir->FileName[0] == 0xE5) {
    Dir->FileName[0] = 0x05;
  }

  Dir->CaseFlag = (UINT8)(OFile->CaseFlag);
  Dir->FileSize = (UINT32) OFile->FileSize;
  
  switch (OFile->DirType) {
  case IsEmpty:
    Dir->FileName[0] = 0xE5;
    Dir->Attributes = 0;
    
    //
    // Record this empty entry's position in parent's FreeEntryPos
    //
    if (OFile->Parent) {
      if (OFile->Parent->FreeEntryPos > OFile->DirPosition) {
        OFile->Parent->FreeEntryPos = OFile->DirPosition;
      }      
    }    
    break;

  case IsFile:
    Dir->Attributes = (UINT8)(OFile->Attributes & ~(FAT_ATTRIBUTE_DIRECTORY));
    break;

  case IsDir: 
    Dir->Attributes = OFile->Attributes;
    Dir->Attributes |= FAT_ATTRIBUTE_DIRECTORY;
    Dir->FileSize = 0;
    break;

  case IsPreserve:
    Dir->Attributes = OFile->Attributes;
    break;
  default:
    return;
  }

  FatEfiTimeToFatTime (&OFile->CreateTime, &FatTime);
  EfiCopyMem (&Dir->FileCreateTime, &FatTime, sizeof(Dir->FileCreateTime));

  FatEfiTimeToFatTime (&OFile->LastAccess, &FatTime);
  EfiCopyMem (&Dir->FileLastAccess, &FatTime.Date, sizeof(Dir->FileLastAccess));

  FatEfiTimeToFatTime (&OFile->LastModification, &FatTime);
  EfiCopyMem 
      (&Dir->FileModificationTime, &FatTime, sizeof(Dir->FileModificationTime));

  Dir->FileCluster = (UINT16) OFile->FileCluster;
  if (OFile->Vol->FatType == FAT32) {
    Dir->FileClusterHigh = (UINT16) (OFile->FileCluster >> 16);
  } else {
    Dir->FileClusterHigh = 0;
  }

  OFile->IsBlank = (UINT8)(OFile->FileString[0] ? FALSE : TRUE);
}    

VOID
FatNameToStr (
  IN CHAR8            *Fatname,
  IN UINTN            Len,
  IN UINTN            LowerCase,
  IN CHAR16           *Str
  )
{
  UINTN               Index;

  //
  // Convert string to unicode string
  //

  FatFatToStr (Len, Fatname, Str);

  //
  // Strip trailing spaces
  //

  for (Index = EfiStrLen(Str); Index && Str[Index-1] == ' '; Index -= 1) {
    ;
  }
  
  Str[Index] = 0;

  //
  // If the name is to be lower cased, do it now
  //

  if (LowerCase) {
    FatStrLwr (Str);
  }
}

  
EFI_STATUS
FatUnpackDirEntry (
  IN FAT_OFILE        *OFile
  )
{
  FAT_DIRECTORY_ENTRY *Dir;
  FAT_DATE_TIME       FatTime;
  CHAR16              Name[10], Ext[10];
  UINTN               StringLength;
  Dir = &OFile->u.DirEntry;

  // If it's already been marked as preserve, just leave it
  if (OFile->DirType == IsPreserve) {
    goto Done;
  }

  // If there's no name, it's empty
  if (Dir->FileName[0] == 0) {
    OFile->DirType = IsEmpty;
    OFile->IsBlank = TRUE;
    goto Done;
  }

  //
  // Store original 8dot3 name
  //
  EfiCopyMem (OFile->File8Dot3Name,
              Dir->FileName,
              sizeof(OFile->File8Dot3Name)
              );
    
  //
  // Convert the first char as required by FAT spec
  //
  if (OFile->File8Dot3Name[0] == 0x05) {
    OFile->File8Dot3Name[0] = 0xE5;
  }
  
  //
  // if there is only one direntry, use the 8Dot3 name as the file name
  //

  if (OFile->DirCount == 1) {

    //
    // Not an Lfn.. create the FileString from the 8.3 name
    //
    FatNameToStr (OFile->File8Dot3Name,
                  8,
                  Dir->CaseFlag & FAT_CASE_NAME_LOWER,
                  Name
                  );
    FatNameToStr (OFile->File8Dot3Name + 8,
                  3,
                  Dir->CaseFlag & FAT_CASE_EXT_LOWER,
                  Ext
                  );
    
    if (!OFile->FileString) {
      OFile->FileString = EfiLibAllocateZeroPool (26);
    }
    if (!OFile->FileString) {
      return EFI_OUT_OF_RESOURCES;
    }
      
    EfiSetMem (OFile->FileString, 26, 0xFF);
    EfiStrCpy (OFile->FileString, Name);
    StringLength = EfiStrLen (Name);
    if (Ext[0]) {
      OFile->FileString[StringLength] = '.';
      StringLength++;
    }
    EfiStrCpy (OFile->FileString + StringLength, Ext);
  }

  OFile->Attributes = Dir->Attributes;
  OFile->CaseFlag = Dir->CaseFlag;

  //
  // Do LastAccess date first as it's "shorter" then all the other
  // fat time fields
  //
  EfiZeroMem (&FatTime, sizeof(FatTime));
  EfiCopyMem (&FatTime.Date, &Dir->FileLastAccess, sizeof(Dir->FileLastAccess));
  FatFatTimeToEfiTime (&FatTime, &OFile->LastAccess);

  EfiCopyMem (&FatTime, &Dir->FileCreateTime, sizeof(Dir->FileCreateTime));
  FatFatTimeToEfiTime (&FatTime, &OFile->CreateTime);

  EfiCopyMem (&FatTime,
              &Dir->FileModificationTime,
              sizeof(Dir->FileModificationTime)
              );
  FatFatTimeToEfiTime (&FatTime, &OFile->LastModification);

  OFile->FileSize = Dir->FileSize;
  OFile->FileCluster = OFile->FileCurrentCluster = Dir->FileCluster;
  if (OFile->Vol->FatType == FAT32) {
    OFile->FileCluster = OFile->FileCluster | (Dir->FileClusterHigh <<16);
    OFile->FileCurrentCluster = 
            OFile->FileCurrentCluster | (Dir->FileClusterHigh <<16);
  }

  //
  // Set the dir type
  //

  OFile->DirType = IsEmpty;
  if ((Dir->FileName[0] != 0) && (Dir->FileName[0] != 0xE5)) {
    OFile->DirType = IsPreserve;
    if (!(OFile->Attributes & (FAT_ATTRIBUTE_VOLUME_ID | FAT_ATTRIBUTE_DEVICE))) {
      OFile->DirType = IsFile;
      if (OFile->Attributes & FAT_ATTRIBUTE_DIRECTORY) {
        OFile->DirType = IsDir;
      }
    }
  }

Done:

  if (!OFile->FileString) {
    //
    // Defensive, make sure file string has some space
    //
    OFile->FileString = EfiLibAllocateZeroPool (26);
  }
  
  if (OFile->DirType == IsEmpty) {
    OFile->FileString[0] = 0;
    OFile->File8Dot3Name[0] = 0;
  }
  
  return EFI_SUCCESS;
}    

EFI_STATUS 
FatGetBlankFileEntries (
  IN FAT_OFILE        *OFile,
  IN UINTN            EntryCount,    
  IN FAT_OFILE        **NewEntry,
  IN OUT UINT64       *Position
  )
{
  FAT_OFILE       *Entry;
  EFI_STATUS      Status;
  UINTN           Count;
  BOOLEAN         FirstEmptyEntry;

  *NewEntry = NULL;
    
  //
  // Start at the beginning
  //

  FirstEmptyEntry = TRUE;
  *Position = OFile->FreeEntryPos;
  Count = 0;

  Status = EFI_SUCCESS;
  while (Count < EntryCount) {
    //
    // Read this entry
    //
    Status = FatGetDirOFile (OFile, Position, &Entry);
    if (EFI_ERROR(Status)) {
      /*
      if (!Entry && (EntryCount < (OFile->Vol->BlockSize / 0x20))) {
        //
        // There aren't enough blank entries in this directory's cluster
        // to fulfill the write request. (This must have been due to an LFN 
        // entry.  Let's grow the directory one more cluster and start this 
        // write's request there.  
        //
        // LFN file structures are sized 20h boundaries, thus even though we 
        // didn't have enough room for this LFN entry someone later might
        // occupy the space we left behind (8.3 file only requires 20h for
        // an entry). At worst we have a couple of paragraphs left behind on
        // the prior cluster of the directory.  These can later get reclaimed
        // by an 8.3 file.
        //
        *NewEntry = NULL;
      }
      */
      break;
    }

    //
    // If we have a blank entry, count it
    //

    if (Entry->DirType == IsEmpty) {
      
      //
      // Remember the lowest Empty Entry position on the road
      //
      if (FirstEmptyEntry) {
        FirstEmptyEntry = FALSE; 
        OFile->FreeEntryPos = Entry->DirPosition;
      }      

      if (Count == 0) {        
        //
        // If this is the first empty record, remember the start
        //
        *NewEntry = Entry;
        Count += 1;

      } else {

        //
        // Combine this entry to the head entry.  Turn this
        // structure into Preserve as to not write it out.
        //

        (*NewEntry)->DirCount += Entry->DirCount;
        Count += Entry->DirCount;

        Entry->DirType = IsPreserve;
        Entry->DirCount = 0;
        Entry->DirPosition = (UINT64) -1;
        if (Entry->Dirty) {
          return EFI_VOLUME_CORRUPTED;
        }
        
        //
        // Free this OFile if possible
        //
        FatCheckOFileRef (Entry);
      }

    } else {
      //
      // Not an empty entry, start over
      //
      Count = 0;
      
      //
      // Free the Ofile of NewEnry and this Entry if possible
      //
      if (*NewEntry) {
        FatCheckOFileRef (*NewEntry);
        *NewEntry = NULL;
      }      
      FatCheckOFileRef (Entry);
    }
  }

  if (Count >= EntryCount) {
    if ((*NewEntry)->DirPosition == OFile->FreeEntryPos) {
      //
      // remember the position after what we have allocated
      //
      OFile->FreeEntryPos = *Position;
    }

  } else {
    //
    // It's necessary to get rid of the NewEntry. (*NewEntry)->DirCount
    // could be larger than 1 which will mislead following call of this
    // function (actuall the FatGetDirOFile()).
    // FatCheckOFileRef will free the NewEntry because it's not referenced
    // by any IFile.
    // Note that FatGetDirOFile() will not pick any already-in-memory OFile
    // which represents a deleted file.
    // 
    if (*NewEntry) {
      FatCheckOFileRef (*NewEntry);
    }    
  }
  
  return Status;
}


EFI_STATUS
FatNewDirEntry (
  IN FAT_OFILE            *OFile,
  IN UINTN                EntryCount,
  OUT FAT_OFILE           **NewOFile
  )
{
  FAT_VOLUME              *Vol;
  UINTN                   ClusterSize;
  FAT_DIRECTORY_ENTRY     *FatEntry;
  EFI_STATUS              Status;
  UINT64                  Position;
  UINT64                  NewSize;
  
  FatEntry = NULL;
  Vol = OFile->Vol;
  ClusterSize = Vol->ClusterSize;

  //
  // Find & use an existing blank file entry 
  //

  Status = FatGetBlankFileEntries (OFile, EntryCount, NewOFile, &Position);
  
  //
  // if found, goto Done
  // 
  if (!EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // Only EFI_NOT_FOUND indicates we run out of spaces,
  // Error status other than EFI_NOT_FOUND indicates an error
  //
  if (Status != EFI_NOT_FOUND) {
    goto Done;
  }

  //
  // Grow the directory
  //

  if (OFile->IsFixedRootDir) {
    Status = EFI_VOLUME_FULL;
    goto Done;
  }

  FatEntry = EfiLibAllocateZeroPool (ClusterSize);
  if (!FatEntry) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  NewSize = 
    FatSizeToClusters (Position, ClusterSize) * ClusterSize + ClusterSize;
  Status = FatGrowEof (OFile, NewSize);
  if (EFI_ERROR(Status)) {
    FatShrinkEof (OFile);
    goto Done;
  }

  //
  // Write blank entries
  //

  FatOFilePosition (OFile, Position);
  Status = FatDiskIo (Vol, WRITE_DISK, OFile->PosDisk, ClusterSize, FatEntry);
  if (EFI_ERROR(Status)) {
    goto Done;
  }
  
  OFile->FileSize = NewSize;

  //
  // Now allocate one
  //

  Status = FatGetBlankFileEntries (OFile, EntryCount, NewOFile, &Position);

Done: 
  //
  // Initialize the returned structure
  //

  if (!EFI_ERROR(Status)) {
    OFile = *NewOFile;
    gRT->GetTime (&OFile->CreateTime, NULL);
    gRT->GetTime (&OFile->LastAccess, NULL);
    gRT->GetTime (&OFile->LastModification, NULL);

    OFile->FileSize = 0;
    OFile->FileCluster = 0;
    OFile->FileCurrentCluster = 0;
    OFile->FileLastCluster = 0;
    OFile->IsFixedRootDir = FALSE;
    OFile->IsBlank = FALSE;
    OFile->Dirty = TRUE;
  }

  if (FatEntry) {
    gBS->FreePool (FatEntry);
  }

  return Status;
}
