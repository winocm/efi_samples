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

  info.c
  
Abstract:

  Routines dealing with setting/getting file/volume info

Revision History

--*/

#include "fat.h"


EFI_STATUS
FatGetVolumeInfo (
  IN FAT_VOLUME       *Vol,
  IN OUT UINTN        *BufferSize, 
  OUT VOID            *Buffer    
  );

EFI_STATUS
FatSetVolumeInfo (
  IN FAT_VOLUME       *Vol,
  IN OUT UINTN        BufferSize, 
  OUT VOID            *Buffer    
  );

EFI_STATUS
FatGetFileInfo (
  IN FAT_VOLUME       *Vol,
  IN FAT_OFILE        *OFile,
  IN OUT UINTN        *BufferSize,
  OUT VOID            *Buffer    
  )
{
  UINTN               Size, NameSize, ResultSize, Rem;
  EFI_STATUS          Status;
  EFI_FILE_INFO       *Info;

  ASSERT_VOLUME_LOCKED (Vol);

  Size = SIZE_OF_EFI_FILE_INFO;
  NameSize = EfiStrSize (OFile->FileString);
  ResultSize = Size + NameSize;

  Status = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    EfiZeroMem (Buffer, ResultSize);

    Status = EFI_SUCCESS;
    Info = Buffer;

    Info->Size = ResultSize;
    Info->FileSize = OFile->FileSize;
    Info->PhysicalSize = DriverLibDivU64x32(OFile->FileSize, Vol->ClusterSize, &Rem);
    Info->PhysicalSize = DriverLibMultU64x32(
                            Info->PhysicalSize, 
                            Vol->ClusterSize) + (Rem ? Vol->ClusterSize : 0
                            );
    EfiCopyMem (&Info->CreateTime, &OFile->CreateTime, sizeof(EFI_TIME));
    EfiCopyMem (&Info->ModificationTime,
                &OFile->LastModification,
                sizeof(EFI_TIME)
                );
    EfiCopyMem (&Info->LastAccessTime, &OFile->LastAccess, sizeof(EFI_TIME));
    Info->Attribute = OFile->Attributes & EFI_FILE_VALID_ATTR;

    EfiCopyMem ((CHAR8 *) Buffer + Size, OFile->FileString, NameSize);
  }

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
FatGetVolumeInfo (
  IN FAT_VOLUME       *Vol,
  IN OUT UINTN        *BufferSize, 
  OUT VOID            *Buffer    
  )
{
  UINTN                   Size, NameSize, ResultSize;
  CHAR16                  Name[12];
  EFI_STATUS              Status;
  EFI_FILE_SYSTEM_INFO    *Info;
  UINTN                   ClusterSize;

  Size = SIZE_OF_EFI_FILE_SYSTEM_INFO; 
  Status = FatGetVolumeEntry  (Vol,Name);
  NameSize = EfiStrSize (Name);
  ResultSize = Size + NameSize;
  ClusterSize = Vol->ClusterSize;

  //
  // If we don't have valid info, compute it now
  //
  FatComputeFreeInfo (Vol);

  Status = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status = EFI_SUCCESS;

    Info = Buffer;
    EfiZeroMem (Info, SIZE_OF_EFI_FILE_SYSTEM_INFO);
    
    Info->Size = ResultSize;
    Info->ReadOnly = Vol->BlkIo->Media->ReadOnly;
    Info->BlockSize = (UINT32) ClusterSize;
    Info->VolumeSize = DriverLibMultU64x32 (Vol->MaxCluster, ClusterSize);
    Info->FreeSpace = DriverLibMultU64x32 (Vol->FatInfoSector.FreeInfo.ClusterCount,
                                  ClusterSize
                                  );
    EfiCopyMem ((CHAR8 *) Buffer + Size, Name, NameSize);
  }    

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
FatGetVolumeLabelInfo (
  IN FAT_VOLUME       *Vol,
  IN OUT UINTN        *BufferSize, 
  OUT VOID            *Buffer    
  )
{
  UINTN                             Size, NameSize, ResultSize;
  CHAR16                            Name[12];
  EFI_STATUS                        Status;
  EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *Info;

  Size = SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO; 
  Status = FatGetVolumeEntry  (Vol,Name);
  NameSize = EfiStrSize (Name);
  ResultSize = Size + NameSize;

  Status = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status = EFI_SUCCESS;
    Info = Buffer;
    EfiZeroMem (Info, SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO);
    EfiCopyMem ((CHAR8 *) Buffer + Size, Name, NameSize);
  }    

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
FatSetVolumeInfo (
  IN FAT_VOLUME       *Vol,
  IN UINTN            BufferSize, 
  IN VOID             *Buffer    
  )
{
  EFI_STATUS              Status;
  EFI_FILE_SYSTEM_INFO    *Info;

  if (Vol->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }
  Info = (EFI_FILE_SYSTEM_INFO *)Buffer;
  
  if (BufferSize < SIZE_OF_EFI_FILE_SYSTEM_INFO + 2 || Info->Size > BufferSize){
    return EFI_BAD_BUFFER_SIZE;
  }

  Status = FatSetVolumeEntry(Vol,Info->VolumeLabel);
  return Status;
}

EFI_STATUS
FatSetVolumeLabelInfo (
  IN FAT_VOLUME       *Vol,
  IN UINTN            BufferSize, 
  IN VOID             *Buffer    
  )
{
  EFI_STATUS                        Status;
  EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *Info;

  if (Vol->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  Info = (EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *)Buffer;
  
  if (BufferSize < SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO + 2) {
    return EFI_BAD_BUFFER_SIZE;
  }

  Status = FatSetVolumeEntry(Vol,Info->VolumeLabel);
  return Status;
}

EFI_STATUS
FatSetFileInfo (
  IN FAT_VOLUME       *Vol,
  IN FAT_IFILE        *IFile, 
  IN FAT_OFILE        *OFile,
  IN UINTN            BufferSize,
  IN VOID             *Buffer    
  )
{
  EFI_FILE_INFO       *NewInfo;
  FAT_IFILE           *NewIFile, *Open;    
  FAT_OFILE           *NewOFile;
  EFI_STATUS          Status;
  BOOLEAN             Renamed;
  EFI_TIME            ZeroTime;
  EFI_LIST_ENTRY      *List;
  FAT_OFILE           *Entry;
  EFI_STATUS          SyncStatus;  
  UINT64              Position; 
  UINTN            *ZeroBufferSize;
  void             *ZeroBuffer;
  
  EfiZeroMem(&ZeroTime, sizeof(EFI_TIME));
  Renamed = FALSE;

  NewIFile = NULL;
  NewOFile = NULL;
  Entry = NULL;
  ZeroBuffer = NULL;
  ZeroBufferSize = NULL;
  //
  // Make sure there's a valid input buffer
  //

  NewInfo = Buffer;
  if (BufferSize < SIZE_OF_EFI_FILE_INFO + 2 || NewInfo->Size > BufferSize) {
    return EFI_BAD_BUFFER_SIZE;
  }

  //
  // if a zero time is specified, then the original time is preserved
  //
  
  if (!EfiCompareMem (&ZeroTime, &NewInfo->CreateTime, sizeof(EFI_TIME))) {
    EfiCopyMem (&NewInfo->CreateTime, &OFile->CreateTime, sizeof(EFI_TIME));
  }
  if (!EfiCompareMem (&ZeroTime, &NewInfo->ModificationTime, sizeof(EFI_TIME))){
    EfiCopyMem (&NewInfo->ModificationTime,
                &OFile->LastModification,
                sizeof(EFI_TIME)
                );
  }
  if (!EfiCompareMem (&ZeroTime, &NewInfo->LastAccessTime, sizeof (EFI_TIME))) {
    EfiCopyMem (&NewInfo->LastAccessTime, &OFile->LastAccess, sizeof(EFI_TIME));
  }
  
  if ((NewInfo->Attribute & ~EFI_FILE_VALID_ATTR) || 
    !FatIsValidTime(&NewInfo->CreateTime) || 
    !FatIsValidTime(&NewInfo->ModificationTime)) {

    return EFI_INVALID_PARAMETER;
  }    

  // Can not change the directory attribute bit
  if ((NewInfo->Attribute ^ OFile->Attributes) & EFI_FILE_DIRECTORY) {
    return EFI_ACCESS_DENIED;
  }

  // If this is a directory, can't change the file size
  if ((OFile->Attributes & EFI_FILE_DIRECTORY) && 
    NewInfo->FileSize != OFile->FileSize) {
    return EFI_ACCESS_DENIED;
  }

  //
  // If this is the root directory, we can't make any updates
  //

  if (!OFile->Parent) {
    return EFI_ACCESS_DENIED;
  }

  //
  // If the IFile is read-only, the only change allowed is to
  // update the attributes.
  //

  if (IFile->ReadOnly || (OFile->Attributes & EFI_FILE_READ_ONLY)) {

    //
    // If the volume is read-only, then it doesn't matter we can't update it
    //

    if (Vol->ReadOnly) {
      return EFI_WRITE_PROTECTED;
    }

    //
    // See if the caller is trying to change anything else
    //

    if (NewInfo->FileSize != OFile->FileSize ||
      EfiCompareMem(&OFile->CreateTime, &NewInfo->CreateTime, sizeof(EFI_TIME)) ||
      EfiCompareMem (&OFile->LastModification, &NewInfo->ModificationTime, sizeof(EFI_TIME)) ||
      EfiCompareMem (&OFile->LastAccess, &NewInfo->LastAccessTime, sizeof(EFI_TIME))) {

      return EFI_ACCESS_DENIED;
    }
  }
  
  //
  // Just in case, flush the current file info
  //

  Status = FatOFileFlush (OFile);
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // Open the filename and see if it refers to an existing file
  //

  Status = FatOFileOpen (OFile->Parent,
                         &NewIFile,
                         NewInfo->FileName,
                         EFI_FILE_MODE_READ,
                         0
                         );
  if (!EFI_ERROR(Status)) {

    if (NewIFile->OFile != OFile) {
      // filename is to a different filename that already exists
      Status = EFI_ACCESS_DENIED;
      goto Done;
    }

  } else {

    //
    // File was not found.  We do not allow rename of the current directory if
    // there are open files below the current directory
    //
    
    Renamed = TRUE;

    if (!IsListEmpty(&OFile->ChildHead)) {
      Status = EFI_ACCESS_DENIED;
      goto Done;
    }

    if (OFile->Attributes & EFI_FILE_READ_ONLY) {
      Status = EFI_WRITE_PROTECTED;
      goto Done;
    }

    //
    // See if we can create the new filename
    // N.B. If this is a rename of a directory, we first create the
    // name as a file as to not allocate any space for it.
    //

    Status = FatOFileOpen(
          OFile->Parent, 
          &NewIFile, 
          NewInfo->FileName,
          EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
          OFile->Attributes & ~EFI_FILE_DIRECTORY
          );

    if (EFI_ERROR(Status)) {
      goto Done;
    }

    //
    // File was created. 
    // Move the data stream to the new file.  Update the attributes
    // to capture the directory attribute bit.
    //

    NewOFile = NewIFile->OFile;
    if (NewOFile->FileCluster != FAT_CLUSTER_FREE) {
      Status = EFI_VOLUME_CORRUPTED;
      goto Done;
    }    

    NewOFile->FileSize = OFile->FileSize;
    NewOFile->FileCluster = OFile->FileCluster;
    NewOFile->FileCurrentCluster = OFile->FileCluster;
    NewOFile->FileLastCluster = OFile->FileLastCluster;
    NewOFile->DirType = OFile->DirType;
    NewOFile->Attributes = OFile->Attributes;
    NewOFile->ChildHashTable = OFile->ChildHashTable;
    NewOFile->HashTopPosition = OFile->HashTopPosition;
    NewOFile->HashEntry1 = OFile->HashEntry1;
    NewOFile->HashEntry2 = OFile->HashEntry2;
    NewOFile->CurrentEntryPos = OFile->CurrentEntryPos;
    NewOFile->FreeEntryPos = OFile->FreeEntryPos;
    NewOFile->NameNotFound = OFile->NameNotFound;

    OFile->FileSize = 0;
    OFile->FileCluster = FAT_CLUSTER_FREE;
    OFile->FileCurrentCluster = FAT_CLUSTER_FREE;
    OFile->FileLastCluster = 0;
    OFile->Dirty = TRUE;
    OFile->DirType = IsEmpty;
    OFile->ChildHashTable = NULL;
    OFile->NameNotFound = NULL;

    //
    // Move all the open handles to the new entry
    // This version could pass /O1 and /Ow optimization switch
    //
    
    List = OFile->Opens.ForwardLink;
    while (List != &OFile->Opens) {
      Open = CR(List, FAT_IFILE, Link, FAT_IFILE_SIGNATURE);
      List = List->ForwardLink;
      RemoveEntryList (&Open->Link);
      Open->OFile = NewOFile;
      InsertTailList (&NewOFile->Opens, &Open->Link);
    }      
         
    //
    // Add the OFile to the check reference list
    //

    if (!OFile->CheckLink.ForwardLink) {
      InsertHeadList (&Vol->CheckRef, &OFile->CheckLink);
    }

    //
    // Apply the remaining SetInfo updates to the new ofile
    //

    OFile = NewOFile;
  }

  //
  // Set file info is dirty
  //

  OFile->Dirty = TRUE;

  //
  // If the file size has changed, apply it
  //

  if (NewInfo->FileSize < OFile->FileSize) {
    OFile->FileSize = NewInfo->FileSize;
    Status = FatShrinkEof (OFile);
    if (EFI_ERROR(Status)) {
      goto Done;
    }
  }

  if (NewInfo->FileSize > OFile->FileSize) {
    Status = FatGrowEof (OFile, NewInfo->FileSize);
    if (EFI_ERROR(Status)) {
      FatShrinkEof (OFile);
      goto Done;
    }
    *ZeroBufferSize = (UINTN)(NewInfo->FileSize - OFile->FileSize);       
    ZeroBuffer = EfiLibAllocateZeroPool (*ZeroBufferSize);
    if (!ZeroBuffer) {
      gBS->FreePool (ZeroBuffer);
      return EFI_OUT_OF_RESOURCES;
    }
    FatOFileWrite (OFile, OFile->FileSize, ZeroBufferSize, ZeroBuffer);
    if (ZeroBuffer) {
      gBS->FreePool (ZeroBuffer);
    }
    OFile->FileSize = NewInfo->FileSize;            
  }


  //
  // Set the time info
  //

  EfiCopyMem (&OFile->CreateTime, &NewInfo->CreateTime, sizeof(EFI_TIME));
  if (EfiCompareMem (&OFile->LastModification,
                     &NewInfo->ModificationTime,
                     sizeof(EFI_TIME))) {
    
    //
    // User wants to specify a modification time, don't update it when flush
    //
    OFile->PreserveLastMod = TRUE;
    EfiCopyMem (&OFile->LastModification,
                &NewInfo->ModificationTime,
                sizeof(EFI_TIME)
                );
  }  
  EfiCopyMem (&OFile->LastAccess, &NewInfo->LastAccessTime, sizeof(EFI_TIME));

    //
    // If this is a directory, synchronize it's  contained dot file's 
    // date and time field
    // Be noted that now Sync failure is simply ignored
    //
    
    if (OFile->DirType == IsDir &&(!FatIsDotEntry(OFile))) {
      Position = 0;
      for (; ;) {   
        SyncStatus = FatGetDirOFile (OFile, &Position, &Entry);
        //if some msg need to info user, we can use Sync_Status
        if (EFI_ERROR(SyncStatus)) {
        break;
        }

        if (Entry->DirType != IsEmpty  && FatIsDotEntry(Entry)) {   
          Entry->PreserveLastMod = TRUE;     
          Entry->Dirty = TRUE;
          EfiCopyMem (&Entry->CreateTime, 
                      &NewInfo->CreateTime, 
                      sizeof(EFI_TIME));                 
          EfiCopyMem (&Entry->LastAccess,
                      &NewInfo->LastAccessTime, 
                      sizeof(EFI_TIME));
          EfiCopyMem (&Entry->LastModification,
                      &NewInfo->ModificationTime,
                      sizeof(EFI_TIME));
         break; 
        }
      }
    }         

  //
  // Set the current attributes
  //

  OFile->Attributes = (UINT8)((OFile->Attributes & ~EFI_FILE_VALID_ATTR) |
                              (UINT8) NewInfo->Attribute);
  
  //
  // If the file is renamed, we should append the ARCHIVE attribute
  //
  
  if (Renamed) {
    OFile->Attributes = (UINT8)(OFile->Attributes | FAT_ATTRIBUTE_ARCHIVE);
  }
    
  //
  // Done
  //

Done:

  //
  // If we opened a handle, close it
  //

  if (NewIFile) {
    FatIFileClose (NewIFile);
  }

  //
  // Write the dirent changes for the new file
  //

  if (NewOFile) {
    FatOFileFlush(NewOFile);
  }

  return Status;
}


EFI_STATUS
EFIAPI
FatGetInfo (
  IN EFI_FILE   *FHand,
  IN EFI_GUID   *Type,
  IN OUT UINTN  *BufferSize,
  OUT VOID      *Buffer
  )
{
  FAT_IFILE               *IFile;
  FAT_OFILE               *OFile;
  FAT_VOLUME              *Vol;
  EFI_STATUS              Status;
  
  IFile =  IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  Vol   = OFile->Vol;

  Status = OFile->Error;
  if (Status == EFI_NOT_FOUND) {
    return EFI_DEVICE_ERROR;	
  }

  FatAcquireLock ();

  //
  // Verify the file handle isn't in an error state
  //


  if (!Status) {

    //
    // Get the proper information based on the request
    //

    Status = EFI_UNSUPPORTED;
    if (EfiCompareGuid (Type, &gEfiFileInfoGuid)) {
      Status = FatGetFileInfo (Vol, OFile, BufferSize, Buffer);
    }

    if (EfiCompareGuid (Type, &gEfiFileInfoIdGuid)) {
      Status = FatGetVolumeInfo (Vol, BufferSize, Buffer);
    }

    if (EfiCompareGuid (Type, &gEfiFileSystemVolumeLabelInfoIdGuid)) {
      Status = FatGetVolumeLabelInfo (Vol, BufferSize, Buffer);
    }

  }

  Status = FatCleanupVolume (OFile->Vol, NULL, Status);
  
  FatReleaseLock();
  return Status;
}
  

EFI_STATUS
EFIAPI
FatSetInfo (
  IN EFI_FILE  *FHand,
  IN EFI_GUID  *Type,
  IN UINTN     BufferSize,
  IN VOID      *Buffer
  )
{
  FAT_IFILE               *IFile;
  FAT_OFILE               *OFile;
  FAT_VOLUME              *Vol;
  EFI_STATUS              Status;
  
  IFile =  IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  Vol   = OFile->Vol;

  Status = OFile->Error;
  if (Status == EFI_NOT_FOUND) {
    return EFI_DEVICE_ERROR;	
  }
  
  FatAcquireLock ();

  //
  // Verify the file handle isn't in an error state
  //


  if (!Status) {

    //
    // Get the proper information based on the request
    //

    Status = EFI_UNSUPPORTED;
    if (EfiCompareGuid (Type, &gEfiFileInfoGuid)) {
      Status = FatSetFileInfo (Vol, IFile, OFile, BufferSize, Buffer);
    }

    if (EfiCompareGuid (Type, &gEfiFileInfoIdGuid)) {
      Status = FatSetVolumeInfo (Vol, BufferSize, Buffer);
    }

    if (EfiCompareGuid (Type, &gEfiFileSystemVolumeLabelInfoIdGuid)) {
      Status = FatSetVolumeLabelInfo (Vol, BufferSize, Buffer);
    }
  }

  Status = FatCleanupVolume (OFile->Vol, NULL, Status);
  
  FatReleaseLock();
  return Status;
}

