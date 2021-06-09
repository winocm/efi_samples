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
  main.c

  Abstract:
  Main entry point of editor
  

--*/

#include "heditor.h"

EFI_STATUS
InitializeEFIHexEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeEFIHexEditor)
#endif


EFI_HANDLE  HImageHandleBackup;


VOID
PrintUsage  (
  VOID
  )
{
  Print(L"\n\n    %EHexEditor Usage%N\n");
  Print(L"    %H[-f] Filename%N                                 Open File For Editing\n");
  Print(L"    %H-d   DiskName FirstBlockNo.   BlockNumber%N     Open Disk Block For Editing\n");
  Print(L"    %H-m   Offset   Size%N                            Open Memory Region For Editing\n");
  Print(L"\n\n");
}


EFI_STATUS
InitializeEFIHexEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description: 

  Entry point of editor

Arguments:

  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:

  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  EFI_STATUS      Status;
  EFI_STATUS      Result;
  CHAR16          *Buffer;
  CHAR16          *Name;
  BOOLEAN         FreeName;
  UINTN           Offset;

  UINTN           Size;
  UINT64          LastOffset;
  
  IMAGE_TYPE      WhatToDo;

  
  //
  // variable initialization
  //
  Buffer = NULL;
  Name = NULL;
  //----------2003-08-08-------------------
  FreeName = FALSE;
  //---------------------------------------
  Offset = 0;
  Size = 0;
  LastOffset = 0;
  WhatToDo = OPEN_FILE;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeEFIHexEditor,
    L"hexedit",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // backup this variable to transfer to BufferImageInit
  //
  HImageHandleBackup = ImageHandle;
  
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  if ( SI->Argc > 1 ) {

    if (StriCmp(SI->Argv[1], L"-d") == 0) {
      if (SI->Argc < 5) {
        PrintUsage();
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }

      Name = SI->Argv[2];
  
      Result = HXtoi(SI->Argv[3], &Offset);
      if ( EFI_ERROR ( Result ) ) {
        Print ( L"HexEdit: Invalid arguments\n" );
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }
  
      Result = HXtoi(SI->Argv[4], &Size);
      if ( EFI_ERROR ( Result ) ) {
        Print ( L"Hexedit: Invalid arguments\n" );
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }

      if ( Offset < 0 || Size <= 0 ) {
        Print ( L"Hexedit: Invalid arguments\n" );
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }

      WhatToDo = OPEN_DISK;
    } else if (StriCmp(SI->Argv[1], L"-m") == 0) {
      if (SI->Argc < 4) {
        PrintUsage();
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }

      Result = HXtoi(SI->Argv[2], &Offset);
      if ( EFI_ERROR ( Result ) ) {
        Print ( L"Hexedit: Invalid arguments\n" );
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }

      Result = HXtoi(SI->Argv[3], &Size);
      if ( EFI_ERROR ( Result ) ) {
        Print ( L"Hexedit: Invalid arguments\n" );
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }
    
      if ( Offset < 0 || Size <= 0 ) {
        Print ( L"Hexedit: Invalid arguments\n" );
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }
  
      LastOffset = ( UINT64 ) Offset + ( UINT64 ) Size - ( UINT64) 1;
      if ( LastOffset > 0xffffffff ) {
        Print ( L"Hexedit: Invalid arguments\n" );
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }

      WhatToDo = OPEN_MEMORY;
    } else {

      if (StriCmp(SI->Argv[1], L"-f") == 0) {
        if (SI->Argc > 2) {
          Name = SI->Argv[2];
        } else {
          PrintUsage();
          Status = EFI_INVALID_PARAMETER;
          return Status;
        }
      } else {
        Name = SI->Argv[1];
      }

      if ( ! HIsValidFileName ( Name ) ) {
        Print (L"Invalid File Name\n");
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }

      WhatToDo = OPEN_FILE;
    }

  } else {
   
    Name  = PoolPrint(L"NewFile.bin");
    WhatToDo = OPEN_FILE;
	FreeName = TRUE;
  }



  Status = HMainEditorInit();
  if (EFI_ERROR(Status)) {
    Out->ClearScreen(Out);
    Out->EnableCursor(Out,TRUE);
    Print(L"hexedit: Initialization Failed\n");
    return Status;
  }
  
  HMainEditorBackup ();

  switch (WhatToDo) {
    case OPEN_FILE:
      Status = HBufferImageRead( Name, 
                                 NULL, 
                                 0 , 
                                 0, 
                                 0, 
                                 0, 
                                 FILE_BUFFER, 
                                 FALSE
                               );
      break;

    case OPEN_DISK:
      Status = HBufferImageRead( 
                NULL, 
                Name, 
                Offset , 
                Size, 
                0, 
                0, 
                DISK_BUFFER, 
                FALSE
                );
      break;
  
    case OPEN_MEMORY:
      Status = HBufferImageRead ( 
        NULL,
        NULL, 
        0, 
        0, 
        (UINT32 ) Offset, 
        Size, 
        MEM_BUFFER, 
        FALSE 
        );
      break;

  }

  
  if ( !EFI_ERROR ( Status ) ) {
    HMainEditorRefresh ();
    Status = HMainEditorKeyInput ();
  }

  if ( Status != EFI_OUT_OF_RESOURCES ) {
    //
    // back up the status string
    //
    Buffer = PoolPrint (L"%s", HMainEditor.StatusBar->StatusString);
  }
  
  HMainEditorCleanup();
  
  // 
  // print editor exit code on screen
  //
  switch ( Status ) {
    case EFI_SUCCESS:
      break;

    case EFI_OUT_OF_RESOURCES:
      Print (L"hexedit: Out of Resources\n");
      break;

    default:
      if ( Buffer!= NULL ) {
        if ( StrCmp ( Buffer, L"" ) != 0 ) {
          //
          // print out the status string
          //
          Print (L"hexedit: %s\n", Buffer);
        } else {
          Print (L"hexedit: Unknown Editor Error\n");
        }
      } else {
       Print (L"hexedit: Unknown Editor Error\n");
      }
  
      break;
  }


  if ( Status != EFI_OUT_OF_RESOURCES ) {
    HEditorFreePool ( Buffer );
  }
  //--------2003-08-08------------------
  if(FreeName){
	  FreePool(Name);
  }
  //------------------------------------
  return Status;
}

