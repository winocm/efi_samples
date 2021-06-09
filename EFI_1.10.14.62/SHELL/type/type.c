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

  type.c
  
Abstract:

  EFI  Shell command "type"

Revision History

--*/

#include "shell.h"

//
//
//
EFI_STATUS
InitializeType (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );


EFI_STATUS
TypeFile (
  IN SHELL_FILE_ARG     *Arg
  );


BOOLEAN   TypeAscii;
BOOLEAN   TypeUnicode;


//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeType)
#endif

EFI_STATUS
InitializeType (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Displays the contents of a file on the standard output device.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Files not found

--*/
{
  EFI_STATUS        Status;
  CHAR16            **Argv;
  UINTN             Argc;
  UINTN             Index;
  EFI_LIST_ENTRY    FileList;
  EFI_LIST_ENTRY    *Link;
  SHELL_FILE_ARG    *Arg;
  CHAR16            *Ptr;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeType,
    L"type",     // command
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

  Argv = SI->Argv;
  Argc = SI->Argc;

  if (Argc == 1) {
    Print (L"type: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"type: Argument with zero length is not allowed\n");
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Scan args for flags
  //
  InitializeListHead (&FileList);
  TypeAscii = FALSE;
  TypeUnicode = FALSE;
  for (Index = 1; Index < Argc; Index += 1) {
    if (Argv[Index][0] == '-') {
      for (Ptr = Argv[Index]+1; *Ptr; Ptr++) {
        switch (*Ptr) {
        case 'a':
        case 'A':
          TypeAscii = TRUE;
          break;
          
        case 'u':
        case 'U':
          TypeUnicode = TRUE;
          break;
          
        case 'b' :
        case 'B' :
          EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
          break;

        default:
          Print (L"type: Unknown flag %hs\n", Argv[Index]);
          Status = EFI_INVALID_PARAMETER;
          ShellFreeFileList (&FileList);
          return Status;
        }
      }
    }
  }

  if ( TypeAscii && TypeUnicode ) {
    Print(L"type: Cannot use ASCII and Unicode at the same time\n"); 
    Status = EFI_INVALID_PARAMETER;
    ShellFreeFileList (&FileList);
    return Status;
  }

  //
  // Expand each arg
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (Argv[Index][0] != '-') {
      ShellFileMetaArg (Argv[Index], &FileList);
    }
  }

  if (IsListEmpty(&FileList)) {
    Print (L"type: File not found\n");
    Status = EFI_NOT_FOUND;
    ShellFreeFileList (&FileList);
    return Status;
  }

  //
  // Type each file
  //
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status = TypeFile (Arg);
  }

  ShellFreeFileList (&FileList);
  return Status;
}


EFI_STATUS
TypeFile (
  IN  SHELL_FILE_ARG  *Arg
  )
{
  EFI_STATUS          Status;
  CHAR16              *Buffer;
  CHAR8               *CharBuffer;
  CHAR16              *WideBuffer;
  EFI_FILE_HANDLE     Handle;
  UINTN               BufferSize, Size;
  BOOLEAN             FormatAscii;
  UINTN               Index;
  UINTN               TotalSize;

  Status = Arg->Status;
  if (!EFI_ERROR(Status)) {
    if (Arg->Info->Attribute & EFI_FILE_DIRECTORY) {
      Print(L"type: Target %hs is a directory\n", Arg->FullName);
      return Status;
    }

    Handle = Arg->Handle;
    Print(L"%HFile: %s, Size %,ld%N\n\n", Arg->FullName, Arg->Info->FileSize);

    //
    // Allocate Buffer according to file size
    //
    Buffer = NULL;
    Buffer = AllocatePool((UINTN)Arg->Info->FileSize);
    BufferSize = (UINTN)Arg->Info->FileSize;
    //
    // if out of resource, allocate 64k buffer
    //
    if (Buffer == NULL) {
      Buffer = AllocatePool(64*1024);
      if (Buffer == NULL) {
        Print (L"type: Out of memory\n");
        return EFI_OUT_OF_RESOURCES;
      }
      BufferSize = 64*1024;
    }
    
    //
    // Unicode files start with a marker of 0xff, 0xfe.  Skip it.
    //
    Size = 2;
    Status = Handle->Read (Handle, &Size, Buffer);
    if (Buffer[0] == EFI_UNICODE_BYTE_ORDER_MARK) {
      FormatAscii = FALSE;
    } else {
      FormatAscii = TRUE;
      if (TypeUnicode) {
        Print (L"%.*s", Size/sizeof(CHAR16), Buffer);
      } else {
        Print (L"%.*a", Size, Buffer);
      }
    }      
    
    TotalSize = 0;
    for (; ;) {
      Size = BufferSize;
      Status = Handle->Read (Handle, &Size, Buffer);
      if (EFI_ERROR(Status) || !Size) {
        break;
      }

      TotalSize += Size;
      if (TotalSize > Arg->Info->FileSize) {
        //
        // Prevent inifinite loop when "type abc.txt > abc.txt"
        //
        break;
      }
      
      WideBuffer = (CHAR16 *)Buffer;
      CharBuffer = (CHAR8 *)Buffer;

      //
      // if type unicode file as ascii file, convert '0' to space character
      //
      if (TypeAscii && !FormatAscii) {
        for (Index = 0; Index < Size; Index++)
        {
          if (CharBuffer[Index] == 0x0d) {
            CharBuffer[Index + 1] = 0x0d;
            Index++;
            continue;
          }  
          if (CharBuffer[Index] == 0x0a) {
            CharBuffer[Index + 1] = 0x0a;
            Index++;
            continue;
          }  
          if (CharBuffer[Index] == 0) {
            CharBuffer[Index] = 0x20;
          }
        }
      }

      //
      // Type file according to type option, or file format
      //
      FormatAscii = (BOOLEAN)((TypeAscii || TypeUnicode) ? TypeAscii : FormatAscii);
      if (FormatAscii) {
        Size = Size / sizeof(CHAR8);
      } else {
        Size = Size / sizeof(CHAR16);
      } 
        
      if (FormatAscii) {
        Print(L"%.*a", Size, CharBuffer);
      } else {
        Print(L"%.*s", Size, WideBuffer);
      }
    }

    FreePool(Buffer);
  }

  if (EFI_ERROR(Status)) {
    Print (L"type: Type %hs error - %r\n", Arg->FullName, Status);
  }
  return Status;
}
