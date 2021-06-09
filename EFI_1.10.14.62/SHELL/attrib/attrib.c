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

  attrib.c
  
Abstract:

  EFI shell command "attrib" - set file attributes

Revision History

--*/

#include "shell.h"

//
// Function declarations
//
EFI_STATUS
InitializeAttrib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
AttribSet (
  IN CHAR16               *Str,
  IN OUT UINT64           *Attr
  );

EFI_STATUS
AttribFile (
  IN SHELL_FILE_ARG       *Arg,
  IN UINT64               Remove,
  IN UINT64               Add
  );

//
// Entry Point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeAttrib)
#endif

EFI_STATUS
InitializeAttrib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:

  ImageHandle    The image handle. 
  SystemTable    The system table.

Returns:

  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error
  
--*/
{
  CHAR16                  **Argv;
  UINTN                   Argc;
  UINTN                   Index;
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  UINT64                  Remove;
  UINT64                  Add;
  EFI_STATUS              Status;    

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeAttrib,
    L"attrib",  // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Local Variable Initializations
  //
  Argv = SI->Argv;
  Argc = SI->Argc;
  Index = 0;
  InitializeListHead (&FileList);
  Link = NULL;
  Arg = NULL;
  Remove = 0;
  Add = 0;

  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"attrib: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      ShellFreeFileList (&FileList);
      return Status;
    }
  }

  //
  // Parse command line arguments
  //
    
  for (Index = 1; Index < Argc; Index += 1) {
    
    if (Argv[Index][0] == '-') {
      
      //
      // Attributes to remove
      // 
      Status = AttribSet (Argv[Index]+1, &Remove);
      if (EFI_ERROR(Status)) {
        Print(L"attrib: Invalid argument %hs - %r\n", Argv[Index], 
          Status);
        ShellFreeFileList (&FileList);
        return Status;
      }
    } else if (Argv[Index][0] == '+') {
      
      //
      // Attributes to Add
      //          
      Status = AttribSet (Argv[Index]+1, &Add);
      if (EFI_ERROR(Status)) {
        Print(L"attrib: Invalid argument %hs - %r\n", Argv[Index], 
          Status);
        ShellFreeFileList (&FileList);
        return Status;
      }
    } else {
      
      //
      // Treat as file name
      //
      Status = ShellFileMetaArg (Argv[Index], &FileList);
      if (EFI_ERROR(Status)) {
        Print(L"attrib: Cannot open %hs - %r\n", Argv[Index], Status);
        ShellFreeFileList (&FileList);
        return Status;
      }
    }
  }
  
  //
  // if no file is specified, get the whole directory
  //
  if (IsListEmpty(&FileList)) {
    Status = ShellFileMetaArg (L"*", &FileList);
    if (EFI_ERROR(Status)) {
      Print(L"attrib: Cannot open current directory - %r\n", Status);
      ShellFreeFileList (&FileList);
      return Status;
    }
  }

  //
  // Attrib each file
  //
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status = AttribFile (Arg, Remove, Add);
    //
    // User Pressed 'q' to exit command
    //
    if (EFI_ERROR (Status)) {
      break;
    }
  }

  ShellFreeFileList (&FileList);
  return Status;
}


EFI_STATUS
AttribSet (
  IN CHAR16       *Str,
  IN OUT UINT64   *Attr
  )
{
  //
  // Convert to Lower, lest case/break not equal
  //
  StrLwr (Str);

  while (*Str) {
    //
    // Check one by one
    //
    switch (*Str) {
      case 'b' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case 'a':
        *Attr |= EFI_FILE_ARCHIVE;
        break;

      case 's':
        *Attr |= EFI_FILE_SYSTEM;
        break;

      case 'h':
        *Attr |= EFI_FILE_HIDDEN;
        break;

      case 'r':
        *Attr |= EFI_FILE_READ_ONLY;
        break;

      default:
        Print (L"attrib: Invalid file attribute '%hc'\n", *Str);                
        return EFI_INVALID_PARAMETER;
    }
    Str += 1;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
AttribFile (
  IN SHELL_FILE_ARG           *Arg,
  IN UINT64                   Remove,
  IN UINT64                   Add
  )
/*++

Routine Description:

  Set/remove attributes of a file

Arguments:

  Arg             The file whose attributes are to be modified. 
  Remove          Bitmap of the attributes to be removed.
  Add             Bitmap of the attributes to be added.

Returns:

  EFI_SUCCESS     Set attribute of file successfully
  EFI_ABORTED     Press 'q' to abort
  
--*/
{
  UINT64                      Attr;
  EFI_STATUS                  Status;
  EFI_FILE_INFO               *Info;

  //
  // Local variable initializations
  //
  Attr = 0;
  Status = EFI_SUCCESS;
  Info = NULL;

  // 
  // Validate the file
  //
  Status = Arg->Status;
  if (! EFI_ERROR(Status)) {
    //
    // Attrib it
    //
    Info = Arg->Info;
    if (Add || Remove) {
      Info->Attribute = Info->Attribute & (~Remove) | Add;
      Status = Arg->Handle->SetInfo(  
            Arg->Handle,
            &GenericFileInfo,
            (UINTN) Info->Size,
            Info
            );
    }
  }

  //
  // Output result
  //
  if (EFI_ERROR(Status)) {
    Print (L"attrib: Cannot set attributes of %hs - %r\n", Arg->FullName,
      Status);
  } else {
    Attr = Info->Attribute;
    Print (L"%c%c%c%c%c %s\n",
      Attr & EFI_FILE_DIRECTORY ? 'D' : ' ',
      Attr & EFI_FILE_ARCHIVE   ? 'A' : ' ',
      Attr & EFI_FILE_SYSTEM    ? 'S' : ' ',
      Attr & EFI_FILE_HIDDEN    ? 'H' : ' ',
      Attr & EFI_FILE_READ_ONLY ? 'R' : ' ',
      Arg->FullName
      );
  }

  return EFI_SUCCESS;
}
