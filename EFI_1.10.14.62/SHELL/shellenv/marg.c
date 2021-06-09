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

  marg.c
  
Abstract:

Revision History

--*/

#include "shelle.h"

//
//
//
typedef struct _CWD {
  struct _CWD     *Next;
  CHAR16          Name[1];
} SENV_CWD;

CHAR16 *
SEnvFileHandleToFileName (
  IN EFI_FILE_HANDLE      Handle
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN                   BufferSize;
  UINTN                   BufferSize1;
  SENV_CWD                *CwdHead;
  SENV_CWD                *Cwd;
  POOL_PRINT              Str;
  EFI_FILE_INFO           *Info;
  EFI_STATUS              Status;
  EFI_FILE_HANDLE         NextDir;

  ASSERT_LOCKED (&SEnvLock);

  Info = NULL;

  Status = EFI_SUCCESS;
  CwdHead = NULL;
  ZeroMem (&Str, sizeof (Str));

  //
  //
  //
  Status = Handle->Open (Handle, &Handle, L".", EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    Handle = NULL;
    goto Done;
  }


  BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
  Info = AllocatePool (BufferSize);
  if (!Info) {
    goto Done;
  }

  //
  // Reverse out the current directory on the device
  //
  for (; ;) {
    BufferSize1 = BufferSize;
    Status = Handle->GetInfo (Handle, &GenericFileInfo, &BufferSize1, Info);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    //
    // Allocate & chain in a new name node
    //
    Cwd = AllocatePool (sizeof (SENV_CWD) + StrSize (Info->FileName));
    if (!Cwd) {
      goto Done;
    }

    StrCpy (Cwd->Name, Info->FileName);

    Cwd->Next = CwdHead;
    CwdHead = Cwd;

    //
    // Move to the parent directory
    //
    Status = Handle->Open (Handle, &NextDir, L"..", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR (Status)) {
      break;
    }

    Handle->Close (Handle);
    Handle = NextDir;
  }

  //
  // Build the name string of the current path
  //
  if (CwdHead->Next) {
    for (Cwd=CwdHead->Next; Cwd; Cwd=Cwd->Next) {
      CatPrint (&Str, L"\\%s", Cwd->Name);
    }
  } else {
    //
    // Must be in the root
    //
    Str.str = StrDuplicate (L"\\");
  }

Done:
  while (CwdHead) {
    Cwd = CwdHead;
    CwdHead = CwdHead->Next;
    FreePool (Cwd);
  }

  if (Info) {
    FreePool (Info);
  }

  if (Handle) {
    Handle->Close (Handle);
  }

  return Str.str;
}

  
VOID
SEnvFreeFileArg (
  IN SHELL_FILE_ARG   *Arg
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // Free memory for all elements belonging to Arg
  //
  if (Arg->Parent) {
    Arg->Parent->Close (Arg->Parent);
  }

  if (Arg->ParentName) {
    FreePool (Arg->ParentName);
  }

  if (Arg->ParentDevicePath) {
    FreePool (Arg->ParentDevicePath);
  }

  if (Arg->FullName) {
    FreePool (Arg->FullName);
  }

  if (Arg->FileName) {
    FreePool (Arg->FileName);
  }

  if (Arg->Handle) {
    Arg->Handle->Close (Arg->Handle);
  }

  if (Arg->Info) {
    FreePool (Arg->Info);
  }

  if (Arg->Link.Flink) {
    RemoveEntryList (&Arg->Link);
  }
  
  //
  // Free memory for Arg
  //
  FreePool (Arg);
}


EFI_STATUS
SEnvFreeFileList (
  IN OUT EFI_LIST_ENTRY       *ListHead
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  SHELL_FILE_ARG          *Arg;

  while (!IsListEmpty (ListHead)) {
    Arg = CR (ListHead->Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    SEnvFreeFileArg (Arg);
  }

  return EFI_SUCCESS;
}


SHELL_FILE_ARG *
SEnvNewFileArg (
  IN EFI_FILE_HANDLE            Parent,
  IN UINT64                     OpenMode,
  IN EFI_DEVICE_PATH_PROTOCOL   *ParentPath,
  IN CHAR16                     *ParentName,
  IN CHAR16                     *FileName
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  SHELL_FILE_ARG                *Arg;
  CHAR16                        *LPath;
  CHAR16                        *Ptr;
  UINTN                         Len;

  Arg = NULL;

  //
  // Allocate a new arg structure
  //
  Arg = AllocateZeroPool (sizeof (SHELL_FILE_ARG));
  if (!Arg) {
    goto Done;
  }

  Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
  Parent->Open (Parent, &Arg->Parent, L".", OpenMode, 0);
  Arg->ParentDevicePath = DuplicateDevicePath (ParentPath);
  Arg->ParentName = StrDuplicate (ParentName);
  if (!Arg->Parent || !Arg->ParentDevicePath || !Arg->ParentName) {
    Arg->Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // Open the target file
  //
  Arg->Status = Parent->Open (
          Parent,
          &Arg->Handle,
          FileName,
          OpenMode,
          0
          );

  if (Arg->Status == EFI_WRITE_PROTECTED || Arg->Status == EFI_ACCESS_DENIED) {
    OpenMode = OpenMode & ~EFI_FILE_MODE_WRITE;
    Arg->Status = Parent->Open (
            Parent,
            &Arg->Handle,
            FileName,
            OpenMode,
            0
            );
  }

  Arg->OpenMode = OpenMode;
  if (Arg->Handle) {
    Arg->Info = LibFileInfo (Arg->Handle);
  }


  //
  // Compute the file's full name
  //
  Arg->FileName = StrDuplicate (FileName);
  if (StriCmp (FileName, L".") == 0) {
    
    //
    // It is the same as the parent
    //
    Arg->FullName = StrDuplicate (Arg->ParentName);
  } else if (StriCmp (FileName, L"..") == 0) {

    LPath = NULL;
    for (Ptr=Arg->ParentName; *Ptr; Ptr++) {
      if (*Ptr == L'\\') {
        LPath = Ptr;
      }
    }

    if (LPath) {
      Arg->FullName = PoolPrint (L"%.*s", 
                                 (UINTN) (LPath - Arg->ParentName + 1), 
                                 Arg->ParentName);
    }

    Len = StrLen (Arg->FullName);
    
    if (Len && Arg->FullName[Len-2] != ':') {
      Arg->FullName[Len-1] = 0;
    }
  }

  if (!Arg->FullName) {
    //
    // Append filename to parent's name to get the file's full name
    //
    Len = StrLen (Arg->ParentName);
    if (Len && Arg->ParentName[Len-1] == '\\') {
      Len -= 1;
    }

    if (FileName[0] == '\\') {
      FileName += 1;
    }
    
    Arg->FullName = PoolPrint (L"%.*s\\%s", Len, Arg->ParentName, FileName);
  }

  if (!Arg->FileName || !Arg->FullName) {
    Arg->Status = EFI_OUT_OF_RESOURCES;
  }

Done:
  if (Arg && Arg->Status == EFI_OUT_OF_RESOURCES) {
    SEnvFreeFileArg (Arg);
    Arg = NULL;
  }

  if (Arg && !EFI_ERROR (Arg->Status) && !Arg->Handle) {
    Arg->Status = EFI_NOT_FOUND;
  }
  
  return Arg;
}


EFI_STATUS
SEnvFileMetaArg (
  IN CHAR16               *Path,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  VARIABLE_ID                 *Var;
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *RPath;
  EFI_DEVICE_PATH_PROTOCOL    *TPath;
  EFI_DEVICE_PATH_PROTOCOL    *ParentPath;
  FILEPATH_DEVICE_PATH        *FilePath;
  EFI_FILE_INFO               *Info;
  UINTN                       BufferSize;
  UINTN                       BufferSize1;
  EFI_FILE_HANDLE             Parent;
  EFI_FILE_HANDLE             Parent1;
  SHELL_FILE_ARG              *Arg;
  CHAR16                      *ParentName;
  CHAR16                      *LPath;
  CHAR16                      *Ptr;
  UINT64                      OpenMode;
  BOOLEAN                     Found;
  
  CHAR16                      *Path1;
  CHAR16                      *Path11;
  UINTN                       Index;
  UINTN                       Index1;
  UINTN                       Pos;

  RPath = NULL;
  Parent = NULL;
  Parent1 = NULL;
  ParentPath = NULL;
  ParentName = NULL;
  Path11 = NULL;

  AcquireLock (&SEnvLock);

  Path1 = StrDuplicate (Path);

  //
  // Remove redundant continuous '*' from path
  //
  Index = 0;
  Pos = 0;
  while (Path1[Index] != '\0') {
    Pos = Index;
    Index1 = Index + 1;
    if (Path1[Pos] == '*') {
      /*
      BUGBUG:
        The following two lines of code are concise to filter redundant '*'s,
        but it does not follow coding convention, so we cannot use it
      
        while (Path1[++Pos] == '*');
        while ((Path1[Index1++] = Path1[Pos++]) != '\0');
      */
      //
      // Skip redundant continuous '*'
      //
      while (Path1[Pos] == '*') {
        Pos++;
      }
      //
      // Copy remaining characters forward
      //
      while (Path1[Index1] != '\0') {
        Path1[Index1++] = Path1[Pos++];
      }
    }
    Index++;
  }

  //
  // Remember the position, to be freed at the end
  //
  Path11 = Path1;
  
  //
  // Strip off the trailing duplicate '\'s in Path1
  //
  for (Index = StrLen (Path1) - 1; Index > 0; Index--) {
    if (Path1[Index] != '\\') {
      break;
    }
  }
  
  if (Path1[Index + 1] == '\\') {
    Path1[Index + 2] = 0;
  }
  
  BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
  Info = AllocatePool (BufferSize);
  if (!Info) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // Get the device
  //
  Var = SEnvMapDeviceFromName (&Path1);
  if (!Var) {
    Arg = AllocateZeroPool (sizeof (SHELL_FILE_ARG));
    Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
    Arg->Status = EFI_NO_MAPPING;
    Arg->ParentName = StrDuplicate (Path1);
    Arg->FullName = StrDuplicate (Path1);
    Arg->FileName = StrDuplicate (Path1);
    InsertTailList (ListHead, &Arg->Link);
    Status = EFI_SUCCESS;
    goto Done;
  } 

  ParentPath = DuplicateDevicePath ((EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value);

  //
  // If the path is relative, append the current dir of the device to the dpath
  //
  if (*Path1 != '\\') {
    RPath = SEnvIFileNameToPath (Var->CurDir ? Var->CurDir : L"\\");
    TPath = AppendDevicePath (ParentPath, RPath);
    if (!RPath || !TPath) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    FreePool (ParentPath);
    FreePool (RPath);
    RPath = NULL;
    ParentPath = TPath;
  }

  //
  // If there is a path before the last node of the name, then
  // append it and strip path to the last node.
  //
  LPath = NULL;
  for (Ptr=Path1; *Ptr; Ptr++) {
    if (*Ptr == '\\' && *(Ptr+1) != '\\') {
      LPath = Ptr;
    }
  }

  if (LPath) {
    *LPath = 0;
    RPath = SEnvIFileNameToPath (Path1);
    TPath = AppendDevicePath (ParentPath, RPath);
    if (!RPath || !TPath) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    FreePool (ParentPath);
    FreePool (RPath);
    RPath = NULL;
    ParentPath = TPath;
    Path1 = LPath + 1;
  }

  if (StrLen (Path1) == 0) {
    Path1 = L".";
  }
  
  //
  // Open the parent dir.
  // First is ReadOnly open, second is RW open,
  // this order can not reversed, if we concern that
  // MediaChange could happen.
  //  
  OpenMode = EFI_FILE_MODE_READ;
  Parent = ShellOpenFilePath (ParentPath, OpenMode);
  if (Parent) {
    OpenMode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    Parent1 = ShellOpenFilePath (ParentPath, OpenMode);
    if (Parent1) {
      Parent->Close (Parent);
      Parent = Parent1;
    } else {
      OpenMode = EFI_FILE_MODE_READ;
    }
  }

  if (Parent) {
    Ptr = SEnvFileHandleToFileName (Parent);
    if (Ptr) {
      ParentName = PoolPrint (L"%s:%s", Var->Name, Ptr);
      FreePool (Ptr);
    }
  }

  if (!Parent) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  BufferSize1 = BufferSize;
  Status = Parent->GetInfo (Parent, &GenericFileInfo, &BufferSize1, Info);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Parent - file handle to parent directory
  // ParentPath - device path of parent dir
  // ParentName - name string of parent directory
  // ParentGuid - last guid of parent path
  //
  // Path1 - remaining node name
  //

  //
  // BUGBUG: if the name doesn't have any meta chars,
  // then just open the one file
  //
  Found = FALSE;
  for (Ptr=Path1; *Ptr && !Found; Ptr++) {
    
    //
    // BUGBUG: need to handle '^'
    //
    switch (*Ptr) {
    case '*':
    case '[':
    case '?':
      Found = TRUE;
      break;
    }
  }

  if (!Found) {
    TPath = SEnvIFileNameToPath (Path1);
    ASSERT (DevicePathType (TPath) == MEDIA_DEVICE_PATH && DevicePathSubType (TPath) == MEDIA_FILEPATH_DP);
    FilePath = (FILEPATH_DEVICE_PATH *) TPath;

    Arg = SEnvNewFileArg (Parent, OpenMode, ParentPath, 
                          ParentName, FilePath->PathName);
    FreePool (TPath);

    if (!Arg) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    InsertTailList (ListHead, &Arg->Link);
  } else {
    //
    // Check all the files for matches
    //
    Parent->SetPosition (Parent, 0);

    Found = FALSE;
    for (; ;) {
      //
      // Read each file entry
      //
      BufferSize1 = BufferSize;
      Status = Parent->Read (Parent, &BufferSize1, Info);
      if (EFI_ERROR (Status) || BufferSize1 == 0) {
        break;
      }

      //
      // Skip "." and ".."
      //
      if (StriCmp (Info->FileName, L".") == 0 ||
        StriCmp (Info->FileName, L"..") == 0) {
        continue;
      }

      //
      // See if this one matches
      //
      if (!MetaiMatch (Info->FileName, Path1)) {
        continue;
      }

      Found = TRUE;
      Arg = SEnvNewFileArg (Parent, OpenMode, ParentPath, ParentName, 
                            Info->FileName);
      if (!Arg) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      InsertTailList (ListHead, &Arg->Link);

      //
      // check next file entry
      //
    }

    //
    // If no match was found, then add a not-found entry for this name
    //
    if (!Found) {
      Arg = SEnvNewFileArg (Parent, OpenMode, ParentPath, 
                            ParentName, Path1);
      if (!Arg) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      Arg->Status = EFI_NOT_FOUND;
      InsertTailList (ListHead, &Arg->Link);
    }
  }

  //
  // Done
  //
Done:
  ReleaseLock (&SEnvLock);

  if (Parent) {
    Parent->Close (Parent);
  }

  if (RPath) {
    FreePool (RPath);
  }

  if (ParentPath) {
    FreePool (ParentPath);
  }

  if (ParentName) {
    FreePool (ParentName);
  }

  if (Info) {
    FreePool (Info);
  }

  if (Path11) {
    FreePool (Path11);
  }    
    
  return Status;
}
