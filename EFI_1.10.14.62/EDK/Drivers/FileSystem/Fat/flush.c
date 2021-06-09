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

  flush.c

Abstract:

  Routines that check references and flush OFiles

Revision History

--*/

#include "fat.h"

EFI_STATUS
EFIAPI
FatFlush (
  IN EFI_FILE  *FHand
  )
/*++

Routine Description:

  Flushes all data associated with the file handle

Arguments:

  FHand   - Handle to file to flush
  
Returns:

  Status code

--*/
{
  FAT_IFILE           *IFile;
  FAT_OFILE           *OFile;
  FAT_VOLUME          *Vol;
  EFI_STATUS          Status;

  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  Vol = OFile->Vol;

  //
  // If the file has a permanent error, return it
  //
  if (EFI_ERROR(OFile->Error)) {
    return OFile->Error;
  }
  
  //
  // If read only, return error
  //
  if (IFile->ReadOnly) {
    if (Vol->ReadOnly) {
      return EFI_WRITE_PROTECTED;
    } else {
      return EFI_ACCESS_DENIED;
    }
  }
  
  //
  // Flush the OFile
  //
  FatAcquireLock ();
  Status = FatOFileFlush(OFile);  
  if (EFI_ERROR(Status)) {    
    Status = FatCleanupVolume (OFile->Vol, OFile, Status);
    FatReleaseLock();
    return Status;
  }

  //
  // Flush the Block device
  //
  Status = Vol->BlkIo->FlushBlocks (Vol->BlkIo);
  if (EFI_ERROR(Status)) {
    Status = FatCleanupVolume (OFile->Vol, OFile, Status);
    FatReleaseLock ();
    return Status;
  }

  Status = FatCleanupVolume (OFile->Vol, OFile, Status);
  FatReleaseLock ();
  return Status;  
}


EFI_STATUS
EFIAPI
FatClose (
  EFI_FILE  *FHand
  )
/*++

Routine Description:

  Flushes & Closes the file handle.
  
Arguments:

  FHand       - Handle to the file to delete
  
Returns:

  Status code

--*/
{
  FAT_IFILE           *IFile;
  FAT_OFILE           *OFile;
  FAT_VOLUME          *Vol;
  EFI_STATUS          Status;

  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  Vol = OFile->Vol;

  if (OFile->Error == EFI_NOT_FOUND) {
    return EFI_SUCCESS;
  }
  //
  // Lock the volume
  //
  FatAcquireLock ();

  //
  // Close the file instance handle
  //
  Status = FatIFileClose (IFile);

  //
  // Done. Unlock the volume
  //

  FatCleanupVolume (OFile->Vol, OFile, Status);
  FatReleaseLock ();
  
  //
  // Close always succeed
  //
  return EFI_SUCCESS;
}


EFI_STATUS
FatIFileClose (
  FAT_IFILE           *IFile
  )
/*++

Routine Description:

  Close the open file instance.
  
Arguments:

  IFile   - Open file instance
  
Returns:

  Status code

--*/
{
  FAT_OFILE           *OFile;
  FAT_VOLUME          *Vol;

  OFile = IFile->OFile;
  Vol = OFile->Vol;

  ASSERT_VOLUME_LOCKED(Vol);

  //
  // Remove the IFile struct
  //
  RemoveEntryList (&IFile->Link);
  
  //
  // Add the OFile to the check reference list
  //
  if (!OFile->CheckLink.ForwardLink) {
    InsertHeadList (&Vol->CheckRef, &OFile->CheckLink);
  }

  //
  // Done. Free the open instance structure
  //
  gBS->FreePool (IFile);
  return EFI_SUCCESS;
}


EFI_STATUS
FatOFileFlush (
  IN FAT_OFILE    *OFile
  )
/*++

Routine Description:

  Flush the data associated with an open file.
  In this implementation, only last Mod/Access time is updated.
  
Arguments:

  OFile   - The open file
  
Returns:

  Status code

--*/
{
  EFI_STATUS          Status, FinalStatus;
  FAT_VOLUME          *Vol;

  //
  // If the file has a permanant error, then don't write any
  // of its data to the device (may be from different media)
  //
  if (OFile->Error) {
    return OFile->Error;
  }

  Vol = OFile->Vol;
  FinalStatus = EFI_SUCCESS;

  //
  // Flush each entry up the tree while dirty
  //

  while (OFile) {
    
    //
    // If the file has a permanent error, return error status
    //
    if (OFile->Error) {
      FinalStatus = OFile->Error;
      break;
    }

    if (OFile->Dirty) {
      
      //
      // Remove the old Hash Node of this OFile
      //
      if (OFile->HashEntry1) {
        FatRemoveHashNode (OFile->HashEntry1, (UINT32)(OFile->DirPosition));
        OFile->HashEntry1 = NULL;
      }
      
      if (OFile->HashEntry2) {
        FatRemoveHashNode (OFile->HashEntry2, (UINT32)(OFile->DirPosition));
        OFile->HashEntry2 = NULL;
      }      
      
      //
      // Update the last modification time
      //
      if (OFile->PreserveLastMod) {
        gRT->GetTime (&OFile->LastAccess, NULL);
        OFile->PreserveLastMod = FALSE;
      } else {       
        gRT->GetTime (&OFile->LastModification, NULL);
        EfiCopyMem (&OFile->LastAccess,
                    &OFile->LastModification,
                    sizeof(EFI_TIME)
                    );
      }      
    
      //
      // Write the directory entry
      //
      Status = FatOFileWriteDir (OFile);
      if (EFI_ERROR(Status)) {
        FinalStatus = Status;
        break;
      }
    
      OFile->Dirty = FALSE;
    }
    
    //
    // Insert Hash Node for this OFile if the parent uses a Hash
    //
    if (OFile->Parent && (OFile->DirType == IsFile || OFile->DirType == IsDir)){
      if (!OFile->HashEntry1) {
        OFile->HashEntry1 = FatInsertHashNode (
                              OFile->Parent,
                              (UINT8*)(OFile->FileString),
                              EfiStrSize (OFile->FileString),
                              (UINT32)(OFile->DirPosition)
                              );
        OFile->HashEntry2 = FatInsertHashNode (
                              OFile->Parent,
                              OFile->File8Dot3Name,
                              sizeof (OFile->File8Dot3Name),
                              (UINT32)(OFile->DirPosition)
                              );
      }
    }
    
    //
    // Check the parent
    //
    OFile = OFile->Parent;
  }

  return FinalStatus;
}

BOOLEAN
FatCheckOFileRef (
  IN FAT_OFILE   *OFile
  )
/*++

Routine Description:

  Check the references of the OFile.
  If the OFile (that is checked) that is no longer
  referenced, then it is freed.
  
Arguments:

  OFile     - The OFile to be checked
  
Returns:

  TRUE      - The OFile is not referenced and freed
  FALSE     - The OFile is kept.

--*/
{
  EFI_STATUS      Status;
  FAT_VOLUME      *Vol;
  
  Status = EFI_SUCCESS;
  Vol = OFile->Vol;
  
  //
  // If the OFile is on the check ref list, remove it
  //
  if (OFile->CheckLink.ForwardLink) {
    RemoveEntryList (&OFile->CheckLink);
    OFile->CheckLink.ForwardLink = NULL;
  }

  //
  // Are there any references to this OFile?
  //
  if (!IsListEmpty(&OFile->Opens) || !IsListEmpty(&OFile->ChildHead)) {

    //
    // The OFile cannot be freed
    //
    Status = FatOFileFlush(OFile);
    return FALSE;
  }

  //
  // No more references - first flush the file.
  //
  Status = FatOFileFlush(OFile);

  //
  // Free the Ofile
  //
  
  if (!OFile->Parent) {
    Vol->Root = NULL;    
    FatFreeOFile (OFile);
  } else {  
    RemoveEntryList (&OFile->ChildLink);  
    FatFreeOFile (OFile);
  }
  
  return TRUE;
}

EFI_STATUS
FatCheckVolumeRef (
  IN FAT_VOLUME   *Vol
  )
/*++

Routine Description:

  Check the references of all open files on the volume.
  Any open file (that is checked) that is no longer
  referenced, is freed - and it's parent open file
  is then referenced checked.
  
Arguments:

  Vol     - The volume to check the pending open file list
  
Returns:

  Status value.

--*/
{
  FAT_OFILE       *OFile;
  FAT_OFILE       *Parent;
  BOOLEAN         Freed;

  //
  // Check all files on the pending check list
  //
  while (!IsListEmpty(&Vol->CheckRef)) {

    //
    // Start with the first file listed
    //
    Parent = CR(Vol->CheckRef.ForwardLink,
                FAT_OFILE,
                CheckLink,
                FAT_OFILE_SIGNATURE
                );

    //
    // Go up the tree cleaning up any un-referenced OFiles
    //
    while (Parent) {
      OFile = Parent;
      Parent = OFile->Parent;
      Freed = FatCheckOFileRef (OFile);
      if (!Freed) {
        break;
      }
    }
  }
  
  return EFI_SUCCESS;
}


EFI_STATUS
FatCleanupVolume (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *OFile    OPTIONAL,
  IN EFI_STATUS           EfiStatus OPTIONAL
  )
/*++

Routine Description:

  Set error status for a specific OFile, reference checking the volume.
  If volume is already marked as invalid, and all resources are freed 
  after reference checking, the file system protocol is uninstalled and
  the volume structure is freed.
  
Arguments:

  Vol           - the Volume that is to be reference checked and unlocked.
  OFile         - the OFile whose permanent error code is to be set.
  EfiStatus     - error code to be set.
  
Returns

  Status code.

--*/
{
  EFI_STATUS              Status;
  UINTN                   Index;

  Status = EFI_SUCCESS;
  
  //
  // Flag the OFile
  //
  if (OFile) {
    FatSetVolumeError (OFile, EfiStatus);    
  }  
  
  //
  // Clean up any dangling OFiles that don't have IFiles
  // we don't check return status here because we want the 
  // volume be cleaned up even the volume is invalid.  
  //

  FatCheckVolumeRef (Vol);
  
  //
  // Update the free hint info. Vol->FreeInfoPos != 0 
  // indicates this a FAT32 volume
  //
  if (Vol->Valid && Vol->FreeInfoValid && Vol->FatDirty && Vol->FreeInfoPos) {

    Status = FatDiskIo (
        Vol,
        WRITE_DISK,
        Vol->FreeInfoPos, 
        sizeof(FAT_INFO_SECTOR), 
        &Vol->FatInfoSector
        );
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  //
  // Update that the volume is not dirty
  //
  if (Vol->Valid && Vol->FatDirty) {
    Vol->FatDirty = FALSE;
    Status = FatSetVolumeDirty (Vol, FALSE);
    if (EFI_ERROR(Status)) {
      return Status;
    }      
  }

  //
  // Flush all dirty cache entries to disk
  //
  if (Vol->Valid) {
    FatVolumeFlushCache (Vol);
  }  

  //
  // Flush block device
  //
  if (Vol->Valid) {
    Status = Vol->BlkIo->FlushBlocks (Vol->BlkIo);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  //
  // If the volume is cleared , remove it.
  // The only time Vol be invalidated is in DriverBindingStop.
  //
  
  if (!Vol->Root && !Vol->Valid) {
    //
    // At this time, the protocol interface should have been uninstalled
    // by FATDriverBindingStop
    //
    
    for (Index = 0; Index < FAT_CACHE_SIZE; Index++) {
      gBS->FreePool (Vol->Cache[Index].Data);
    }
    
    //
    // Free the volume structure
    //
    gBS->FreePool (Vol);
  }
  
  if (EFI_ERROR(EfiStatus)) {
    return EfiStatus;
  } else {
    return Status;
  }  
}


VOID
FatSetVolumeError (
  IN FAT_OFILE            *OFile,
  IN EFI_STATUS           Status
  )
{
  EFI_LIST_ENTRY          *Link;
  FAT_OFILE               *ChildOFile;

  //
  // If this OFile doesn't already have an error, set one
  //

  if (!EFI_ERROR(OFile->Error)) {
    OFile->Error = Status;
  }

  //
  // Set the error on each child OFile
  //

  for (Link = OFile->ChildHead.ForwardLink;
       Link != &OFile->ChildHead;
       Link = Link->ForwardLink) {
    ChildOFile = CR(Link, FAT_OFILE, ChildLink, FAT_OFILE_SIGNATURE);
    FatSetVolumeError (ChildOFile, Status);
  }
}
