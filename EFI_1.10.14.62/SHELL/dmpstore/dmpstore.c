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

  dmpstore.c
  
Abstract:

  Shell app "dmpstore"



Revision History

--*/

#include "shell.h"

#define DEBUG_NAME_SIZE 1050

static CHAR16 *AttrType[] = {
  L"invalid",     // 000
  L"invalid",     // 001
  L"BS",          // 010
  L"NV+BS",       // 011
  L"RT+BS",       // 100
  L"NV+RT+BS",    // 101
  L"RT+BS",       // 110
  L"NV+RT+BS",    // 111
};

//
//
//

EFI_STATUS
InitializeDumpStore (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
DumpVariableStore (
  CHAR16 *VarName
  );

//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeDumpStore)
#endif

EFI_STATUS
InitializeDumpStore (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  CHAR16      **Argv;
  UINTN       Argc;
  UINTN       Index;
  BOOLEAN     Delete;
  BOOLEAN     PageBreak;
  CHAR16      *VarName;
  EFI_STATUS  Status;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeDumpStore,
    L"dmpstore",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  InitializeShellApplication (ImageHandle, SystemTable);

  Argv = SI->Argv;
  Argc = SI->Argc;

  Delete = FALSE;
  PageBreak = FALSE;
  VarName = NULL;
  Status = EFI_SUCCESS;
  
  //
  // Check flags
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (Argv[Index][0] == '-') {
      if (Argv[Index][1] == 'B'||Argv[Index][1] == 'b') {
        if (Delete == TRUE) {
          Print (L"dmpstore: Too many flags\n");
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
        //
        // -b (PageBreak)
        //
        PageBreak = TRUE;
      } else {
        Print (L"dmpstore: Unknown flag %hs\n", Argv[Index]);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      VarName = Argv[Index];
    }
  }
  
  //
  // Set page break
  //
  if (PageBreak == TRUE) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }
  
  //
  // Dump all variables in store
  //
  if (VarName == NULL && Delete != TRUE) {
    Status = DumpVariableStore (NULL);
    goto Done;
  }

  //
  // Dump one variable in store
  //
  if (VarName != NULL && Delete != TRUE) {
    Status = DumpVariableStore (VarName);
    goto Done;
  }

  //
  // Done
  //
Done:
  
  return Status;
}


EFI_STATUS
DumpVariableStore (
  CHAR16 *VarName
  )
{
  EFI_STATUS    Status;
  EFI_GUID      Guid;
  UINT32        Attributes;
  CHAR16        Name[DEBUG_NAME_SIZE/2];
  UINTN         NameSize;
  CHAR16        Data[DEBUG_NAME_SIZE/2];
  UINTN         DataSize;
  BOOLEAN       Found;

  Found = FALSE;
  
  if (VarName != NULL) {
    Print (L"Dump Variable %hs\n", VarName);
  } else {
    Print (L"Dump Variable Stores\n");
  }
  
  Name[0] = 0x0000;
  do {
    NameSize = DEBUG_NAME_SIZE;
    Status = RT->GetNextVariableName (&NameSize, Name, &Guid);
    if (VarName != NULL) {
      if (StrCmp (Name, VarName) != 0) {
        continue;
      }
    }
    if (!EFI_ERROR(Status)) {
      Found = TRUE;
      DataSize = DEBUG_NAME_SIZE;
      Status = RT->GetVariable (Name, &Guid, &Attributes, &DataSize, Data);
      if (!EFI_ERROR (Status)) {
        //
        // dump variables
        //
        Print (L"Variable %hs '%hg:%hs' DataSize = %x\n",
              AttrType[Attributes & 7],
              &Guid,
              Name,
              DataSize);
        DumpHex (2, 0, DataSize, Data);
      }
    }
  } while (!EFI_ERROR(Status));
  
  if (Found == FALSE) {
    if (VarName != NULL) {
      Print (L"Variable %hs not found\n", VarName);
    } else {
      Print (L"Variable Stores are empty\n");
    }
    return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}
