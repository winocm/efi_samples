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

  dirent.c

Abstract:

  Functions for performing directory entry io

Revision History

--*/

#include "fat.h"

STATIC
EFI_STATUS
FatReadEntry (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *Parent,
  IN UINT64               *Position,
  IN FAT_OFILE            *OFile
  )
{
  EFI_STATUS              Status;

  //
  // Seek the OFile to the file position
  //

  Status = FatOFilePosition (Parent, (UINTN) *Position);
  if (!EFI_ERROR(Status)) {

    //
    // Read the entry
    //

    Status = FatDiskIo(Vol,
                       READ_DISK,
                       Parent->PosDisk,
                       sizeof(FAT_DIRECTORY_ENTRY),
                       &OFile->u.DirEntry
                       );

    //
    // Update the position & size of this OFile direntry
    //

    if (!EFI_ERROR(Status)) {
      *Position += sizeof(FAT_DIRECTORY_ENTRY);
      OFile->DirCount += 1;
    }
  }

  return Status;
}

EFI_STATUS
FatGetVolumeEntry (
  IN FAT_VOLUME           *Vol,
  IN CHAR16               *Name
  )
{
  FAT_OFILE            OFile;
  UINT64               Position;
  EFI_STATUS           Status;

  EfiStrCpy(Name,L"");
  Position = 0;
  do {
    if (Position >= Vol->Root->FileSize) {
      return EFI_NOT_FOUND;
    }
    Status = FatReadEntry(Vol,Vol->Root,&Position,&OFile);
    if (!EFI_ERROR(Status)) {
      if ((OFile.u.DirEntry.Attributes & (~FAT_ATTRIBUTE_ARCHIVE)) == 
          FAT_ATTRIBUTE_VOLUME_ID) {
        FatNameToStr(OFile.u.DirEntry.FileName,11,FALSE,Name);
        return EFI_SUCCESS;
      }
    }
  } while(!EFI_ERROR(Status));

  return EFI_NOT_FOUND;
}



EFI_STATUS
FatOpenEntry (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *Parent,
  IN UINT64               *Position,
  OUT FAT_OFILE           **pOFile
  )
{
  FAT_OFILE               *OFile;
  EFI_STATUS              Status;
  UINT8                   LfnOrdinal;
  UINT8                   LfnChecksum;
  CHAR16                  *Pos;
  CHAR16                  *TempFileString;
  UINTN                   FileStringBufferSize;

  TempFileString = NULL;
  OFile = FatAllocateOFile(Vol);
  if (!OFile) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  TempFileString = EfiLibAllocateZeroPool (2*(13*MAX_LFN_ENTRIES+1));
  if (!TempFileString) {
    gBS->FreePool (OFile);
    return EFI_OUT_OF_RESOURCES;
  }
  
  OFile->DirPosition = *Position;
  if ((((UINTN) *Position) % sizeof(FAT_DIRECTORY_ENTRY)) != 0) {
    Status = EFI_VOLUME_CORRUPTED;
    goto Done;
  }  

  //
  // read the packed data from the directory
  //

  Status = FatReadEntry (Vol, Parent, Position, OFile);
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // If the entry is the start of a long file name, start processing it
  //

  if (OFile->u.DirEntry.Attributes == FAT_ATTRIBUTE_LFN &&
    OFile->u.LfnEntry.MustBeZero == 0  &&
    (OFile->u.LfnEntry.Ordinal & FAT_LFN_LAST) &&
    (OFile->u.LfnEntry.Ordinal & ~FAT_LFN_LAST) > 0 &&
    (OFile->u.LfnEntry.Ordinal & ~FAT_LFN_LAST) <= MAX_LFN_ENTRIES) {

    LfnOrdinal = (UINT8)(OFile->u.LfnEntry.Ordinal & ~FAT_LFN_LAST);
    LfnChecksum = OFile->u.LfnEntry.Checksum;

    //
    // Loop and read each portion of the name
    //

    for (; ;) {

      //
      // Copy this portion of the name to the file string
      //

      Pos = TempFileString + (LfnOrdinal - 1) * 13;
      EfiCopyMem (Pos +  0, OFile->u.LfnEntry.Name1, sizeof(CHAR16) * 5);
      EfiCopyMem (Pos +  5, OFile->u.LfnEntry.Name2, sizeof(CHAR16) * 6);
      EfiCopyMem (Pos + 11, OFile->u.LfnEntry.Name3, sizeof(CHAR16) * 2);

      //
      // If this is the last LFN dir entry, stop
      //

      LfnOrdinal -= 1;
      if (LfnOrdinal == 0) {
        break;
      }

      //
      // Read next name fragment
      //

      Status = FatReadEntry (Vol, Parent, Position, OFile);
      if (EFI_ERROR(Status)) {
        goto Done;
      }

      //
      // If something is wrong with the entry, just return it as unknown
      //

      if (OFile->u.DirEntry.Attributes != FAT_ATTRIBUTE_LFN ||
        OFile->u.LfnEntry.MustBeZero != 0  ||
        OFile->u.LfnEntry.Ordinal != LfnOrdinal ||
        OFile->u.LfnEntry.Checksum != LfnChecksum) {

        OFile->DirType = IsPreserve;
        break;
      }
    }

    //
    // Read the 8.3 file entry for this lfn
    //

    Status = FatReadEntry (Vol, Parent, Position, OFile);
    if (EFI_ERROR(Status)) {
      goto Done;
    }

    //
    // Verify the LFN is for this entry
    //

    if (FatDirEntryChecksum (&OFile->u.DirEntry) != LfnChecksum) {
      OFile->DirType = IsPreserve;
    }
  }

  FileStringBufferSize = ((EfiStrLen (TempFileString) + 1 + 12) / 13) * 26;
  OFile->FileString = EfiLibAllocateZeroPool (FileStringBufferSize);
  if (!OFile->FileString) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }  
  EfiSetMem (OFile->FileString, FileStringBufferSize, 0xFF);
  EfiStrCpy (OFile->FileString, TempFileString);

  //
  // Unpack the dir entry
  //

  Status = FatUnpackDirEntry (OFile);
  if (EFI_ERROR(Status)) {
    goto Done;
  }
  
  //
  // If this is a directory, compute its file size
  //

  if (OFile->DirType == IsDir) {
    OFile->FileSize = FatDirSize (OFile);
  }

  //
  // Add the new OFile structure to the parent
  //

  OFile->Parent = Parent;
  InsertTailList (&Parent->ChildHead, &OFile->ChildLink);
  InsertHeadList (&Vol->CheckRef, &OFile->CheckLink);

  *pOFile = OFile;
  Status = EFI_SUCCESS;

Done:
  if (EFI_ERROR(Status)) {
    if (OFile) {
      FatFreeOFile (OFile);
    }
  }
  if (TempFileString) {
    gBS->FreePool (TempFileString);
  }  
  return Status;
}

EFI_STATUS
FatGetDirOFile (
  IN FAT_OFILE            *OFile,
  IN OUT UINT64           *Position,
  OUT FAT_OFILE           **pOFile
  )
{
  FAT_OFILE               *Entry;
  FAT_VOLUME              *Vol;
  EFI_LIST_ENTRY          *Item;
  EFI_STATUS              Status;
  UINTN                   BlockSize;
  
  Vol = OFile->Vol;
  BlockSize = Vol->BlockSize;
  *pOFile = NULL;

  ASSERT_VOLUME_LOCKED(Vol);
  
  if (OFile->DirType != IsDir) {
    return EFI_VOLUME_CORRUPTED;
  }

  //
  // If past eof, forget it
  //

  if (*Position >= OFile->FileSize) {
    //
    // Communicate that we had a partial entry left over in the cluster.
    //
    return EFI_NOT_FOUND;
  }

  //
  // See if file entry is already loaded
  //

  Entry = NULL;
  Status = EFI_SUCCESS;
  for (Item=OFile->ChildHead.ForwardLink;
       Item != &OFile->ChildHead;
       Item=Item->ForwardLink) {
    Entry = CR(Item, FAT_OFILE, ChildLink, FAT_OFILE_SIGNATURE);
    
    //
    // If we find an existing match, break.
    // We have to avoid picking an existing OFile that represents
    // a deleted file.
    //
    if (Entry->DirPosition == *Position && 
      !(Entry->DirType == IsEmpty && Entry->Error == EFI_NOT_FOUND)) {
      if (!Entry->DirCount) {
        return EFI_VOLUME_CORRUPTED;
      }
      *Position += Entry->DirCount * sizeof(FAT_DIRECTORY_ENTRY);
      break;
    }
  }


  //
  // It's not found, load it
  //

  if (Item == &OFile->ChildHead) {
    Status = FatOpenEntry (Vol, OFile, Position, &Entry);
  }

  //
  // Done. If success, move the position to the next block and return the entry
  //

  if (!EFI_ERROR(Status)) {
    *pOFile = Entry;
  }

  return Status;
}

STATIC
EFI_STATUS
FatWriteEntry (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *Parent,
  IN UINT64               *Position,
  IN VOID                 *Entry    
  )
{
  EFI_STATUS              Status;

  //
  // Seek the OFile to the file position
  //

  Status = FatOFilePosition (Parent, (UINTN) *Position);
  if (!EFI_ERROR(Status)) {

    //
    // Write the entry
    //

    Status = FatDiskIo(Vol,
                       WRITE_DISK,
                       Parent->PosDisk,
                       sizeof(FAT_DIRECTORY_ENTRY),
                       Entry
                       );

    //
    // Update the position
    //

    if (!EFI_ERROR(Status)) {
      *Position += sizeof(FAT_DIRECTORY_ENTRY);
    }
  }

  return Status;
}


EFI_STATUS
FatOFileWriteDir (
  IN FAT_OFILE            *OFile
  )
{
  FAT_VOLUME              *Vol;
  FAT_OFILE               *Parent;
  EFI_STATUS              Status;
  FAT_DIRECTORY_LFN       LfnEntry;
  UINT64                  Position;    
  UINTN                   LfnOrdinal;
  CHAR16                  *Pos;


  Vol = OFile->Vol;
  Parent = OFile->Parent;
  Position = OFile->DirPosition;
  if (!Parent){
    return EFI_VOLUME_CORRUPTED;
  }  

  //
  // If there's not an 8.3 name for the file generate one now
  //
  
  if (!OFile->File8Dot3Name[0]) {
    Status = FatGenerate8Dot3Name (Vol, Parent, OFile);
    if (EFI_ERROR(Status)) {
      goto Done;
    }
  }

  //
  // Pack the current file data to a fat file entry
  //

  FatPackDirEntry (OFile);

  //
  // If the dir entry has an Lfn, write those entries first
  //

  if (!OFile->DirCount) {
    return EFI_VOLUME_CORRUPTED;
  }
  
  EfiZeroMem (&LfnEntry, sizeof(LfnEntry));

  LfnOrdinal = OFile->DirCount - 1;
  LfnEntry.Ordinal = (UINT8) (LfnOrdinal | FAT_LFN_LAST);
  LfnEntry.Checksum = FatDirEntryChecksum (&OFile->u.DirEntry);
  LfnEntry.Attributes = FAT_ATTRIBUTE_LFN;
  
  while (LfnOrdinal) {

    Pos = OFile->FileString + (LfnOrdinal - 1) * 13;
    EfiCopyMem (LfnEntry.Name1, Pos +  0, sizeof(CHAR16) * 5);
    EfiCopyMem (LfnEntry.Name2, Pos +  5, sizeof(CHAR16) * 6);
    EfiCopyMem (LfnEntry.Name3, Pos + 11, sizeof(CHAR16) * 2);

    // If the file has been deleted, mark it as such
    if (OFile->DirType == IsEmpty) {
      LfnEntry.Ordinal = 0xE5;
      LfnEntry.Attributes = 0;
    }

    Status = FatWriteEntry (Vol, Parent, &Position, &LfnEntry);
    if (EFI_ERROR(Status)) {
      goto Done;
    }

    LfnOrdinal = LfnOrdinal - 1;
    LfnEntry.Ordinal = (UINT8) LfnOrdinal;
  }

  //
  // Write the directory entry
  // 

  Status = FatWriteEntry (Vol, Parent, &Position, &OFile->u.DirEntry);

Done:
  return Status;
}

EFI_STATUS
FatSetVolumeEntry (
  IN FAT_VOLUME           *Vol,
  IN CHAR16               *Name
  )
{
  FAT_OFILE            OFile;
  FAT_OFILE            *NewOFile;
  UINT64               Position;
  EFI_STATUS           Status;
  CHAR8                FatTemp[12];
  BOOLEAN              Result;

  Result = FatStrToFat (Name, 11, FatTemp);

  //
  //TRUE means long file name, and it is not support in FAT
  //
  if(Result == TRUE) {
    return EFI_UNSUPPORTED;
  }

  Status = EFI_SUCCESS;
  for (Position = 0;Position < Vol->Root->FileSize && !EFI_ERROR(Status);) {
    Status = FatReadEntry(Vol,Vol->Root,&Position,&OFile);
    if (!EFI_ERROR(Status)) {
      if ((OFile.u.DirEntry.Attributes & (~FAT_ATTRIBUTE_ARCHIVE)) == 
          FAT_ATTRIBUTE_VOLUME_ID) {
        EfiSetMem (OFile.u.DirEntry.FileName, 11, ' ');
        OFile.u.DirEntry.FileCluster = 0;
        OFile.u.DirEntry.FileClusterHigh = 0;
        FatStrToFat (Name,11,OFile.u.DirEntry.FileName);
        gRT->GetTime (&OFile.LastModification, NULL);
        FatEfiTimeToFatTime (&OFile.LastModification, 
                             &OFile.u.DirEntry.FileModificationTime
                             );
        Position -= sizeof (FAT_DIRECTORY_ENTRY);
        Status = FatWriteEntry (Vol, Vol->Root, &Position, &OFile.u.DirEntry);
        return Status;
      }
    }
  }

  Status = FatNewDirEntry (Vol->Root, 1, &NewOFile);
  if (!EFI_ERROR(Status)) {
    EfiSetMem (NewOFile->u.DirEntry.FileName, 11, ' ');
    FatStrToFat (Name, 11, NewOFile->u.DirEntry.FileName);
    NewOFile->u.DirEntry.Attributes = FAT_ATTRIBUTE_VOLUME_ID;
    NewOFile->u.DirEntry.FileCluster = 0;
    NewOFile->u.DirEntry.FileClusterHigh = 0;
    gRT->GetTime (&NewOFile->LastModification, NULL);
    FatEfiTimeToFatTime (&NewOFile->LastModification, 
                         &NewOFile->u.DirEntry.FileModificationTime
                         );
    Position = NewOFile->DirPosition;
    Status = FatWriteEntry (Vol, Vol->Root, &Position, &NewOFile->u.DirEntry);
    NewOFile->Dirty = FALSE;
    return Status;
  }

  return Status;
}
