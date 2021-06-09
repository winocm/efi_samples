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
    

--*/

#include "editor.h"

EFI_STATUS
InitializeEFIEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeEFIEditor)
#endif


EFI_HANDLE   ImageHandleBackup;


//
// Name:
//    InitializeEFIEditor -- Entry point of editor
// In:
//    ImageHandle
//    SystemTable
// Out:
//    EFI_SUCCESS
//    

EFI_STATUS
InitializeEFIEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS      Status;
  CHAR16          *Buffer;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeEFIEditor,
    L"edit",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  Buffer = (CHAR16 *)"1";

  //
  // backup this variable to transfer to FileBufferInit
  //
  ImageHandleBackup = ImageHandle;
  
  InitializeShellApplication (ImageHandle, SystemTable);


  if (SI->Argc > 1) {
    if ( !IsValidFileName ( SI -> Argv[1] ) ) {
      Print (L"edit: Invalid File Name\n");
      return EFI_INVALID_PARAMETER;
    }
  }


  Status = MainEditorInit();
  if (EFI_ERROR(Status)) {
    Out->ClearScreen(Out);
    Out->EnableCursor(Out,TRUE);
    Print(L"edit: Initialization Failed\n");
    return Status;
  }
  
  MainEditorBackup ();

  //
  // if editor launched with file named
  //
  if (SI->Argc > 1) {

    FileBufferSetFileName(SI->Argv[1]);
  }

  
  Status = FileBufferRead( MainEditor.FileBuffer->FileName, FALSE);
  if ( !EFI_ERROR ( Status ) ) {
    MainEditorRefresh();

    Status = MainEditorKeyInput();
  }

  if ( Status != EFI_OUT_OF_RESOURCES ) {
    //
    // back up the status string
    //
    Buffer = PoolPrint (L"%s", MainEditor.StatusBar->StatusString);
  }
  
  MainEditorCleanup();
  
  // 
  // print editor exit code on screen
  //
  switch ( Status ) {
    case EFI_SUCCESS:
      break;

    case EFI_OUT_OF_RESOURCES:
      Print (L"edit: Out of Resources\n");
      break;

    default:
      if ( Buffer!= NULL ) {
        if ( StrCmp ( Buffer, L"" ) != 0 ) {
          //
          // print out the status string
          //
          Print (L"edit: %s\n", Buffer);
        } else {
          Print (L"edit: Unknown Editor Error\n");
        }
      } else {
        Print (L"edit: Unknown Editor Error\n");
      }
      
      break;
  }

  if ( Status != EFI_OUT_OF_RESOURCES ) {
    EditorFreePool ( Buffer );
  }
  
  return Status;
}

