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

  help.c

Abstract:

  EFI shell command "help"

Revision History

--*/

#include "shelle.h"

extern EFI_LIST_ENTRY  SEnvCmds;

EFI_STATUS
SEnvHelp (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                  Status;
  EFI_LIST_ENTRY              *Link;
  COMMAND                     *Command;
  UINTN                       SynLen, Len;
  UINTN                       Index;
  CHAR16                      *p;

  //
  // Initialize application
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  for (Index = 1; Index < SI->Argc; Index += 1) {
    p = SI->Argv[Index];
    if (*p == '-') {
      switch (p[1]) {
      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        PrintHelpInfo (L"help");
        return EFI_SUCCESS;

      default:
        Print (L"help: Unknown flag %hs\n", p);
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  AcquireLock (&SEnvLock);

  if (SI->Argc >= 2 && SI->Argv[1][0] != '-') {
    //
    // Process 'help cmd'
    //
    for (Link=SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      Command = CR(Link, COMMAND, Link, COMMAND_SIGNATURE);
      if ( StriCmp (Command->Cmd, SI->Argv[1]) == 0) {
        PrintHelpInfo (Command->Cmd);
        break;
      }
    }
    if(Link == &SEnvCmds) {
      Print(L"help: Unknown command %hs\n", SI->Argv[1]);
      Status = EFI_NOT_FOUND;
    }
  }
  else {
    //
    // Process 'help' / 'help -b'
    //
    SynLen = 0;
    for (Link=SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      Command = CR(Link, COMMAND, Link, COMMAND_SIGNATURE);
      if (Command->Cmd && Command->CmdHelpLine) {
        Len = StrLen(Command->Cmd);
        //
        // Make help line aligned 
        //
        if (Len > SynLen) {
          SynLen = Len;
        }
      }
    }

    for (Link=SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      Command = CR(Link, COMMAND, Link, COMMAND_SIGNATURE);
      if (Command->Cmd && Command->CmdHelpLine) {
        Print (L"%-.*hs - %s\n", SynLen, Command->Cmd, Command->CmdHelpLine);
      }
    }
    if (! GetPageBreak()) {
      Print (L"\nUse '%Hhelp -b%N' to display commands one screen at a time.\n");
    }
  }

  ReleaseLock (&SEnvLock);
  return Status;
}
