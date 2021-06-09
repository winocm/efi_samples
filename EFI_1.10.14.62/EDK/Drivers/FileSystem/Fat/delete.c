/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  delete.c
  
Abstract:

  Function that deletes a file

Revision History

--*/

#include "fat.h"


//
//
//

EFI_STATUS
EFIAPI
FatDelete (
  EFI_FILE  *FHand
  )
/*++

Routine Description:

  Deletes the file & Closes the file handle.
  
Arguments:

  FHand       - Handle to the file to delete
  
Returns:

  Status code

--*/
{
  FAT_IFILE           *IFile;
  FAT_OFILE           *OFile;
  FAT_OFILE           *Entry;
  FAT_VOLUME          *Vol;
  EFI_STATUS          Status;
  UINT64              Position;

  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  Vol = OFile->Vol;

  //
  // Lock the volume
  //

  FatAcquireLock ();

  //
  // If the file is read-only, then don't delete it
  //

  if (IFile->ReadOnly) {
    Status = EFI_WRITE_PROTECTED;
    goto Done;
  }

  //
  // If the file is the root dir, then don't delete it
  //
  
  if (!OFile->Parent) {
    Status = EFI_ACCESS_DENIED;
    goto Done;
  }
  
  //
  // If the file has a permanant error, skip the delete
  //

  Status = OFile->Error;
  if (!Status) {
    //
    // If this is a directory, make sure it's empty before
    // allowing it to be deleted
    //

    if (OFile->DirType == IsDir) {
      Position = 0;
      for (; ;) {
        Status = FatGetDirOFile (OFile, &Position, &Entry);
        if (Status == EFI_NOT_FOUND || (!EFI_ERROR(Status)) && Entry->IsBlank) {
          Status = EFI_SUCCESS;
          break;
        }
          
        if (EFI_ERROR(Status)) {
          goto Done;
        }

        if (Entry->DirType != IsEmpty  && !FatIsDotEntry(Entry)) {
          Status = EFI_ACCESS_DENIED;
          goto Done;
        }
      }
    }

    //
    // Return the file's space by setting its size to 0
    //

    OFile->FileSize = 0;
    FatShrinkEof (OFile);


    //
    // Free the directory entry for this file
    //

    OFile->DirType = IsEmpty;
    OFile->Dirty = TRUE;
    
    Status = FatOFileFlush(OFile);
    
    //
    // Always close the handle
    //

    FatIFileClose (IFile);
    
    //
    // Set a permanent error for this OFile in case there 
    // are still opened IFiles attached
    //
    OFile->Error = EFI_NOT_FOUND;
    
    Status = FatCleanupVolume (OFile->Vol, NULL, Status);
    FatReleaseLock ();
  
    if (EFI_ERROR(Status)) {
      Status = EFI_WARN_DELETE_FAILURE;
    }
  
    return Status;
  }
  else if (OFile->Error == EFI_NOT_FOUND) {
    FatReleaseLock();
    return EFI_SUCCESS;
  }

Done:

  //
  // Always close the handle
  //

  FatIFileClose (IFile);
  
  //
  // Done
  //

  Status = FatCleanupVolume (OFile->Vol, NULL, Status);
  FatReleaseLock ();
  
  if (EFI_ERROR(Status)) {
    Status = EFI_WARN_DELETE_FAILURE;
  }
  
  return Status;
}


