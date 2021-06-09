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

  open.c
  
Abstract:

  Routines dealing with file open

Revision History

--*/

#include "fat.h"


EFI_STATUS
FatAllocateIFile (
  IN FAT_OFILE    *OFile,
  OUT FAT_IFILE   **pIFile
  )
{
  FAT_IFILE       *IFile;

  ASSERT_VOLUME_LOCKED (OFile->Vol);

  //
  // Allocate a new open instance
  //

  IFile = EfiLibAllocateZeroPool (sizeof(FAT_IFILE));
  if (!IFile) {
    return EFI_OUT_OF_RESOURCES;
  }

  IFile->Signature = FAT_IFILE_SIGNATURE;

  EfiCopyMem(&(IFile->Handle), &FATFileInterface,sizeof(EFI_FILE));

  IFile->Position  = 0;
  IFile->OFile     = OFile;
  InsertTailList (&OFile->Opens, &IFile->Link);

  *pIFile = IFile;
  return EFI_SUCCESS;
}

BOOLEAN
FatMatchFileName (
  IN FAT_OFILE        *Entry,
  IN CHAR16           *Name
  )
/*++

Routine Description:
  
  Check if the file matches a name. Compare both LFN and 8Dot3Name

Arguments:

  Entry   - The file
  Name    - The Name to match
  
Returns:

  TRUE    - Name matches
  FALSE   - Name not match
  
--*/
{
  CHAR16              UnicodeBuffer[30];
  CHAR16              *Ptr;
  
  if (FatStriCmp (Name, Entry->FileString) == 0) {    
    //
    // Just match
    //
    return TRUE;
  } else {
    
    //
    // match after starting/trailing blanks and trailing periods
    // is removed
    //
    FatLfnIsValid (Name);
    if (FatStriCmp (Name, Entry->FileString) == 0) {
      return TRUE;
    }      
    
    //
    // match with the 8Dot3 Name
    //
    
    FatFatToStr (8, Entry->File8Dot3Name, UnicodeBuffer);
    
    for (Ptr = UnicodeBuffer + EfiStrLen(UnicodeBuffer) - 1;
         Ptr >= UnicodeBuffer && *Ptr == ' ';
         Ptr --) {
      ;
    }
    
    *(Ptr + 1) = 0;
    
    if (! (Entry->File8Dot3Name[8] == ' ' &&
           Entry->File8Dot3Name[9] == ' ' && 
           Entry->File8Dot3Name[10]== ' ')
       ) {
      UnicodeBuffer[EfiStrLen(UnicodeBuffer) + 1] = 0;
      UnicodeBuffer[EfiStrLen(UnicodeBuffer)] = '.';
    
      FatFatToStr (3, Entry->File8Dot3Name + 8, 
                    UnicodeBuffer + EfiStrLen(UnicodeBuffer));
    
      for (Ptr = UnicodeBuffer + EfiStrLen(UnicodeBuffer) - 1;
         Ptr >= UnicodeBuffer && *Ptr == ' ';
         Ptr --) {
        ;
      }
      
      *(Ptr + 1) = 0;
    }
  
    if (FatStriCmp (Name, UnicodeBuffer) == 0) {
      return TRUE;
    }        
  }
  
  return FALSE;
}

EFI_STATUS
FatOFileFromPath (
  IN FAT_OFILE        *OFile,    
  IN OUT CHAR16       **FileName,
  IN FAT_OFILE        **pOFile,
  OUT CHAR16          *CompName
  )
/*++

Routine Description:

  Traverse filename opening all OFiles that can be opened.
  Update filename pointer to the bits that can't be opened.
  If more than one name component remains, returns an error.

Arguments:

  OFile     - The file that serves as a starting reference point.
  FileName  - The file name relevant to the OFile.
  pOFile    - The result OFile.
  CompName  - The last component of the file name if the file name is opened,
              otherwise the component after the component that cannot be
              opened. The buffer should be already allocated when passed in.

Returns:

  Status Code. If error, indicates the file name can't be opened and 
  there is more than one components within the name left (this means 
  the name can not be created either)

--*/
{
  FAT_OFILE           *Entry;
  EFI_STATUS          Status;
  CHAR16              *Next;
  UINT64              Position;
  UINT64              RefPosition;
  UINT64              StartPosition;
  BOOLEAN             FirstRound;

  //
  // Start at current location
  //
  Next = *FileName;

  //
  // If name starts with '\' then move to root
  //
  if (*FileName[0] == '\\') {
    OFile = OFile->Vol->Root;
    Next += 1;
  }

  Status = EFI_SUCCESS;
  while (!EFI_ERROR(Status)) {

    //
    // Get the next component name
    //

    *FileName = Next;
    Next = FatGetNameComp (*FileName, CompName);

    //
    // If end of the file name, we're done
    //

    if (CompName[0] == 0) {
      break;
    }

    //
    // if it is an all-blank file name, then current
    //
    FatLfnIsValid (CompName);
    if (CompName[0] == 0) {
      continue;
    }

    //
    // If "dot", then current
    //

    if (EfiStrCmp(CompName, L".") == 0) {
      continue;
    }    

    //
    // If "dot dot", then parent
    //

    if (EfiStrCmp(CompName, L"..") == 0) {
      OFile = OFile->Parent;
      if (!OFile) {
        Status = EFI_NOT_FOUND;
        break;
      }
      continue;
    }

    //
    // We have a component name, try to open it
    //

    if (OFile->DirType != IsDir) {
      //
      // This file isn't a directory, can't open it
      //

      Status = EFI_NOT_FOUND;
      break;
    }
    
    //
    // If this name is just the not existing one we knew
    //
    if (OFile->NameNotFound) {
      if (FatStriCmp (CompName, OFile->NameNotFound) == 0) {
        Status = EFI_NOT_FOUND;
        FatGetNameComp (Next, CompName);
        if (CompName[0] == 0) {
          // it's the
          // last component name - return with the open 
          // path and the remaining name
          //
          Status = EFI_SUCCESS;
        }
        break;      
      }    
    }
    
    //
    // Search the hash table first
    //
    
    Entry = FatHashSearch (OFile, CompName, (UINT32)(OFile->CurrentEntryPos));
    
    if (Entry) {      
      //
      // Found in the Hash table
      //
      
      OFile = Entry;
    } else {
            
      //
      // Not found in the Hash table, search from the entry at HashTopPosition.
      // OFile->CurrentEntryPos is favoured.
      //
                    
      FirstRound = TRUE;
      Position = OFile->CurrentEntryPos > OFile->HashTopPosition ?
                 OFile->CurrentEntryPos : OFile->HashTopPosition;
      RefPosition = Position;
      StartPosition = OFile->HashTopPosition;
      for (; ;) {
        
        Status = FatGetDirOFile (OFile, &Position, &Entry);
   
        if (EFI_ERROR(Status) && FirstRound) {
          Position = StartPosition;
          FirstRound = FALSE;
          continue;
        }

        if (EFI_ERROR(Status) && !FirstRound) {
          break;
        }
  
        if (Entry->IsBlank && FirstRound) {
          //
          // This is not what we want.
          // Try to free this OFile if it is not referenced.
          //
          FatCheckOFileRef (Entry);

          Position = StartPosition;
          FirstRound = FALSE;
          continue;
        }
        
        if ((Position > RefPosition || Entry->IsBlank) && !FirstRound) {
          Status = EFI_NOT_FOUND;
          break;
        }
  
        //
        // If this isn't a file or directory, skip it
        //
        if (Entry->DirType != IsFile && Entry->DirType != IsDir) {
          //
          // This is not what we want.
          // Try to free this OFile if it is not referenced.
          //
          FatCheckOFileRef (Entry);
          
          continue;
        }
  
        //
        // If the filename matches, we're done
        //        
        if (FatMatchFileName (Entry, CompName)) {
          OFile->CurrentEntryPos = Entry->DirPosition;
          OFile = Entry;
          break;
        }
        
        //
        // This is not what we want.
        // Try to free this OFile if it is not referenced.
        //
        FatCheckOFileRef (Entry);
      }
    }

    if (Status == EFI_NOT_FOUND) {
      //
      // The component name isn't found, remember the name
      //      
      if (OFile->NameNotFound) {
        gBS->FreePool (OFile->NameNotFound);
      }
      OFile->NameNotFound = EfiLibAllocateZeroPool (EfiStrSize(CompName));
      if (OFile->NameNotFound) {
        EfiStrCpy(OFile->NameNotFound, CompName);
      }      
      
      FatGetNameComp (Next, CompName);
      if (CompName[0] == 0) {
        // it's the
        // last component name - return with the open 
        // path and the remaining name
        //
        Status = EFI_SUCCESS;
      }
      break;      
    }    
  }


  if (!EFI_ERROR(Status)) {
    *pOFile = OFile;
  }

  return Status;
}


EFI_STATUS
FatOFileOpen (
  IN FAT_OFILE            *OFile,
  OUT FAT_IFILE           **NewIFile,
  IN CHAR16               *FileName,
  IN UINT64               OpenMode,
  IN UINT64               Attributes
  )
/*++

Routine Description:

  Open a file for a file name relative to an existing OFile.
  The IFile of the newly opened file is passed out.

Arguments:

  OFile       - The file that serves as a starting reference point.
  NewIFile    - The newly generated IFile instance.
  FileName    - The file name relative to the OFile.
  OpenMode    - Open mode.
  Attributes  - Attributes to set if the file is created.

Returns:

  Status Code.

--*/
{
  FAT_OFILE               *SubDir;
  FAT_VOLUME              *Vol;
  EFI_STATUS              Status;
  CHAR16                  CompName[EFI_FILE_STRING_SIZE];
  CHAR16                  *OrigName;
  UINTN                   DirCount;
  UINT8                   CaseFlag;
  UINTN                   FileStringBufferSize;
  BOOLEAN                 DirIntended;

  Vol = OFile->Vol;
  OrigName = FileName;
  DirIntended = FALSE;
  if (EfiStrLen(FileName) > 0) {
    if (FileName [EfiStrLen(FileName) - 1] == '\\') {
      DirIntended = TRUE;
    }
  }
  
  ASSERT_VOLUME_LOCKED(Vol);

  //
  // If attempting to open for write access, make a couple of checks
  //
  if (OpenMode & EFI_FILE_MODE_WRITE) {

    //
    // Can't open for create and apply the read only attribute
    //
    if ((OpenMode & EFI_FILE_MODE_CREATE) && (Attributes & EFI_FILE_READ_ONLY)) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Conficts between dir intention and attribute
    // 
    if ((OpenMode & EFI_FILE_MODE_CREATE) && !(Attributes & EFI_FILE_DIRECTORY)
        && DirIntended) {
      return EFI_NOT_FOUND;
    }

    //
    // Can't open for write if the volume is read only
    //
    if (Vol->ReadOnly) {
      return EFI_WRITE_PROTECTED;
    }
  }
  
  //
  // Verify the source file handle isn't in an error state
  //

  Status = OFile->Error;
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // Get new OFile for the file
  //

  Status = FatOFileFromPath (OFile, &FileName, &OFile, CompName);
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  if (*FileName) {
    
    //
    // If there's a remaining part of the name, then we had
    // better be creating the file in the directory
    //
    
    //
    // can only create new files/dirs in existing dirs
    //
    if (OFile->DirType != IsDir) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }

    if (!(OpenMode & EFI_FILE_MODE_CREATE)) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }

    //
    // Make sure volume is read/write
    //

    if (Vol->ReadOnly) {
      Status = EFI_WRITE_PROTECTED;
      goto Done;
    }

    FatGetNameComp (FileName, CompName);
    
    //
    // Check if the file name to be created is valid
    //
    if (!FatLfnIsValid(CompName)) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }    
    
    //
    // Determine the file's directory entry size
    //
    DirCount = FatNameToDirEntryCount (CompName, &CaseFlag);
    
    //
    //
    // Create the file in the OFile directory
    //

    Status = FatNewDirEntry (OFile, DirCount, &OFile);
    if (EFI_ERROR(Status)) {
      goto Done;
    }
    
    //
    // Invalidate parent directory's NameNotFound
    //
    if (OFile->Parent->NameNotFound) {
      gBS->FreePool (OFile->Parent->NameNotFound);
      OFile->Parent->NameNotFound = NULL;
    }
    
    //
    // Initialize file entry
    //
    // N.B. need to fill the filestring with 0xFF for unused chars 
    // written to Lfn dir entries
    //

    OFile->DirType = (Attributes & EFI_FILE_DIRECTORY) ? IsDir : IsFile;
    OFile->DirCount = DirCount;
    OFile->Attributes = (UINT8) (Attributes | FAT_ATTRIBUTE_ARCHIVE);
    OFile->CaseFlag = CaseFlag;
    FileStringBufferSize = ((EfiStrLen (CompName) + 1 + 12) / 13) * 26;
    if (OFile->FileString) {
      gBS->FreePool (OFile->FileString);
    }
    OFile->FileString = EfiLibAllocateZeroPool (FileStringBufferSize);
    if (!OFile->FileString) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }    
    EfiSetMem (OFile->FileString, FileStringBufferSize, 0xFF);
    EfiStrCpy (OFile->FileString, CompName);
    Status = FatGenerate8Dot3Name (OFile->Vol, OFile->Parent, OFile);
    
    //
    // Case flag is the NtReserved field in the DirEntry. Set to 0
    // when a file is created as required by Fat Spec
    //
    OFile->CaseFlag = 0;
    
    if (EFI_ERROR(Status) || OFile->File8Dot3Name[0] == ' ') {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }    
    
    DEBUG ((EFI_D_INFO, "FSOpen: Created new file '%S'\n", OFile->FileString));

    //
    // If we just created a directory, we need to create "." and ".."
    //

    if (Attributes & EFI_FILE_DIRECTORY) {

      //
      // Create "."
      //
      
      Status = FatNewDirEntry (OFile, 1, &SubDir);
      if (EFI_ERROR(Status)) {
        OFile->DirType = IsEmpty;
        goto Done;
      }

      SubDir->DirType = IsDir;
      SubDir->Attributes = EFI_FILE_DIRECTORY;
      SubDir->FileCluster = OFile->FileCluster;
      SubDir->FileCurrentCluster = OFile->FileCluster;
      //EfiStrCpy (SubDir->FileString, L".");
      if (SubDir->FileString) {
        gBS->FreePool (SubDir->FileString);
      }
      SubDir->FileString = EfiLibAllocateZeroPool (26);
      if (!SubDir->FileString) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }    
      EfiSetMem (SubDir->FileString, 26, 0xFF);
      EfiStrCpy (SubDir->FileString, L".");


      //
      // Create ".."
      //
      
      Status = FatNewDirEntry (OFile, 1, &SubDir);
      if (EFI_ERROR(Status) || !(OFile->Parent)) {
        Status = EFI_VOLUME_CORRUPTED;
        goto Done;
      }      

      SubDir->DirType = IsDir;
      SubDir->Attributes = EFI_FILE_DIRECTORY;

      //
      // The root directory in FAT32 can and will have a valid beginning
      // cluster number.  However for COMPATABILITY reasons, we need to
      // zero the FileCluster field for the first subdirectory under the root.
      // If we didn't, the corresponding CHKDSK/SCANDISK utilities for 
      // Windows 200x and Windows 98 will detect an error, and force the fields
      // to zero anyway.
      //
      if (OFile->Parent->FileCluster == OFile->Vol->RootCluster) {
        SubDir->FileCluster = 0;
        SubDir->FileCurrentCluster = 0;
      } else {
        SubDir->FileCluster = OFile->Parent->FileCluster; 
        SubDir->FileCurrentCluster = OFile->Parent->FileCluster; 
      }

      //EfiStrCpy (SubDir->FileString, L"..");
      if (SubDir->FileString) {
        gBS->FreePool (SubDir->FileString);
      }
      SubDir->FileString = EfiLibAllocateZeroPool (26);
      if (!SubDir->FileString) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }    
      EfiSetMem (SubDir->FileString, 26, 0xFF);
      EfiStrCpy (SubDir->FileString, L"..");

    }

  }

  //
  // If the file's attribute is read only, and the open is for 
  // read-write fail it.
  //

  if ((OFile->Attributes & EFI_FILE_READ_ONLY) && (OpenMode & EFI_FILE_MODE_WRITE)
      && (OFile->DirType == IsFile)) {
    Status = EFI_ACCESS_DENIED;
    goto Done;
  }

  //
  // Conflicts between dir intention and attribute
  // 
  if (!(OFile->Attributes & EFI_FILE_DIRECTORY) && DirIntended) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // Create an open instance of the OFile
  //

  Status = FatAllocateIFile(OFile, NewIFile);
  if (!EFI_ERROR(Status)) {

    //
    // If the file was opened read-only, set the read-only flag
    //

    if (!(OpenMode & EFI_FILE_MODE_WRITE)) {
      (*NewIFile)->ReadOnly = TRUE;
    }
  }

Done:
  DEBUG((EFI_D_INFO, "FSOpen: Open '%S' %r\n", OrigName, Status));
  return Status;
}


EFI_STATUS
EFIAPI
FatOpen (
  IN EFI_FILE   *FHand,
  OUT EFI_FILE  **NewHandle,
  IN CHAR16     *FileName,
  IN UINT64     OpenMode,
  IN UINT64     Attributes
  )
/*++
Routine Description:

  Implements Open() of Simple File System Protocol.
  
Arguments:

  FHand     - File handle of the file serves as a starting reference point.
  NewHandle - Handle of the file that is newly opened.
  FileName  - File name relative to FHand.
  OpenMode  - Open mode.
  Attributes- Attributes to set if the file is created.
  
Returns:

  Status Code.

--*/
{
  CHAR16                  *FileNameStr;
  FAT_IFILE               *IFile;
  FAT_IFILE               *NewIFile;
  FAT_OFILE               *OFile;
  EFI_STATUS              Status;

  //
  // Perform some parameter checking
  //
  if (FileName == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // The passed in file name may be a const string, so we shouldn't modify it directly.
  // Duplicate it first.
  //
  FileNameStr = EfiLibAllocatePool (EfiStrSize (FileName));
  if (FileNameStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  EfiStrCpy (FileNameStr, FileName);
  
  // Don't allow empty file name
  EfiStrTrim (FileNameStr, ' ');
  if (*FileNameStr == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  
  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;


  //
  // Check for a valid mode
  //
  
  switch (OpenMode) {
    case EFI_FILE_MODE_READ :
    case EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE :
    case EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE :
      break;
    default :
      return EFI_INVALID_PARAMETER;
  }

  //
  // Check for valid attributes
  //
  
  if (Attributes & ~(EFI_FILE_VALID_ATTR)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Lock
  //

  FatAcquireLock ();

  //
  // Open the file
  //

  Status = FatOFileOpen (OFile, &NewIFile, FileNameStr, OpenMode, Attributes);

  //
  // If the file was opened, return the handle to the caller
  // 

  if (!EFI_ERROR(Status)) {
    *NewHandle = &NewIFile->Handle;
    OFile = NewIFile->OFile;
  }

  //
  // Unlock
  //

  Status = FatCleanupVolume (OFile->Vol, NULL, Status);
  FatReleaseLock();
  
  gBS->FreePool (FileNameStr);
  
  return Status;
}
