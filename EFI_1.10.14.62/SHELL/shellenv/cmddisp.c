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

  cmddisp.c
  
Abstract:

  Shell Environment internal command management

Revision History

--*/

#include "shelle.h"

//
// Internal prototype
//
EFI_STATUS
SEnvHelp (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvBreak (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

//
//
//
#ifdef EFI_MONOSHELL
EFI_STATUS
InitializeAttrib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeBCfg (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeCls (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeComp (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeCP (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeDate (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
InitializeCompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeDecompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeEFIEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeEFIHexEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeLoad (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
LoadBmpInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
InitializeLoadPciRom (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
InitializeLS (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

EFI_STATUS
InitializeMemmap (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeMkDir (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeMode (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeMv (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeReset (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
InitializeRM (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeType (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeTime (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
InitializeVer (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeVol (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
DumpBlockDev (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
DumpMem (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
InitializeDumpStore (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeError (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeGetMTC (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
DumpIoModify (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
PciDump (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

#ifdef EFI64
EFI_STATUS
SALTest (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );
#endif

EFI_STATUS
InitializeSetSize (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeStall (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
InitializeTouch (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

/*  
EFI_STATUS
InitializeBootMaint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
*/

/*
EFI_STATUS
INTERNAL
InitializeShell (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
*/

/*
EFI_STATUS
DumpPciIoModify (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );
*/

#endif


struct {
  SHELLENV_INTERNAL_COMMAND   Dispatch;
  CHAR16                      *Cmd;
} SEnvInternalCommands[] = {
  SEnvHelp,                   L"?",   
  SEnvHelp,                   L"help", 
  SEnvLoadDefaults,           L"_load_defaults",
  SEnvCmdProt,                L"guid",
  SEnvCmdOpenInfo,            L"openinfo",
  SEnvCmdSet,                 L"set", 
  SEnvCmdAlias,               L"alias",
  SEnvCmdDH,                  L"dh", 
  SEnvCmdDevices,             L"devices", 
  SEnvCmdDeviceTree,          L"devtree", 
  SEnvCmdDrivers,             L"drivers", 
  SEnvCmdDriverConfiguration, L"drvcfg", 
  SEnvCmdDriverDiagnostics,   L"drvdiag", 
  SEnvCmdUnload,              L"unload",
  SEnvCmdConnect,             L"connect",
  SEnvCmdDisconnect,          L"disconnect", 
  SEnvCmdReconnect,           L"reconnect", 
  SEnvCmdMap,                 L"map",     
  SEnvCmdMount,               L"mount",  
  SEnvCmdCd,                  L"cd", 
  SEnvCmdEcho,                L"echo",
  SEnvCmdIf,                  L"if",  
  SEnvCmdElse,                L"else", 
  SEnvCmdEndif,               L"endif", 
  SEnvCmdGoto,                L"goto", 
  SEnvCmdFor,                 L"for", 
  SEnvCmdEndfor,              L"endfor",
  SEnvCmdPause,               L"pause",
  SEnvExit,                   L"exit",  
// for test script only       
  SEnvCmdWait,                L"...",   
#ifdef EFI_DEBUG              
  SEnvBreak,                  L"break",
#endif                        
#ifdef EFI_MONOSHELL          
  InitializeAttrib,           L"attrib",
  InitializeBCfg,             L"bcfg",  
  InitializeCls,              L"cls",     
  InitializeComp,             L"comp", 
  InitializeCP,               L"cp", 
  InitializeDate,             L"date",   
  InitializeEFIEditor,        L"edit", 
  InitializeEFIHexEditor,     L"hexedit", 
  InitializeLoad,             L"load",  
  LoadBmpInitialize,          L"loadbmp",
  InitializeLS,               L"ls",     
  InitializeMemmap,           L"memmap", 
  InitializeMkDir,            L"mkdir", 
  InitializeMode,             L"mode",  
  InitializeMv,               L"mv",     
//  InitializeShell,          L"nshell", 
  InitializeReset,            L"reset",  
  InitializeRM,               L"rm",    
  InitializeType,             L"type",  
  InitializeTime,             L"time",   
  InitializeVer,              L"ver",  
  InitializeVol,              L"vol",    
  DumpBlockDev,               L"dblk",   
  DumpMem,                    L"dmem",  
  InitializeDumpStore,        L"dmpstore", 
  InitializeError,            L"err", 
  InitializeGetMTC,           L"getmtc", 
  DumpIoModify,               L"mm",  
  PciDump,                    L"pci",  
  InitializeSetSize,          L"setsize",
  InitializeStall,            L"stall", 
  InitializeTouch,            L"touch",  
  InitializeCompress,         L"EfiCompress",
  InitializeDecompress,       L"EfiDecompress",
  InitializeLoadPciRom,       L"LoadPciRom",
  
//  DumpPciIoModify,        L"pcimm",
//  InitializeBootMaint,    L"BootMaint", 
#endif
  NULL
} ;

//
//
//
VOID
SEnvInitCommandTable (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN           Index;

  //
  // Add all of our internal commands to the command dispatch table
  //
  InitializeListHead (&SEnvCmds);
  for (Index=0; SEnvInternalCommands[Index].Dispatch; Index += 1) {
  SEnvAddCommand (
    SEnvInternalCommands[Index].Dispatch,
    SEnvInternalCommands[Index].Cmd,
    NULL,
    NULL,
    NULL
    );
  }
}


EFI_STATUS
SEnvAddCommand (
  IN SHELLENV_INTERNAL_COMMAND    Handler,
  IN CHAR16                       *CmdStr,
  IN CHAR16                       *CmdFormat,
  IN CHAR16                       *CmdHelpLine,
  IN CHAR16                       *CmdVerboseHelp
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  COMMAND                     *Cmd;
  COMMAND                     *Command=NULL;
  EFI_LIST_ENTRY              *Link;

  Cmd = AllocateZeroPool (sizeof(COMMAND));

  if (Cmd) {
    AcquireLock (&SEnvLock);

    Cmd->Signature = COMMAND_SIGNATURE;
    Cmd->Dispatch = Handler;
    Cmd->Cmd = CmdStr;
    Cmd->CmdFormat = GetCmdFormat (CmdStr);
    Cmd->CmdHelpLine = GetLineHelp (CmdStr);
    Cmd->CmdVerboseHelp = GetVerboseHelp (CmdStr);   
    InsertTailList (&SEnvCmds, &Cmd->Link);

    //
    // Find the proper place of Cmd
    //
    for (Link=SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      Command = CR(Link, COMMAND, Link, COMMAND_SIGNATURE);
      if (StriCmp (Command->Cmd, Cmd->Cmd) > 0) {
        //
        //  Insert it to proper place
        //
        SwapListEntries (&Command->Link, &Cmd->Link);
        break;
      }
    }

    ReleaseLock (&SEnvLock);
  }

  return Cmd ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}


SHELLENV_INTERNAL_COMMAND  
SEnvGetCmdDispath(
  IN CHAR16                   *CmdName
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY              *Link;
  COMMAND                     *Command;
  SHELLENV_INTERNAL_COMMAND   Dispatch;

  Dispatch = NULL;
  AcquireLock (&SEnvLock);

  //
  // Walk through SEnvCmds linked list
  // to get corresponding command dispatch
  //
  for (Link=SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
    Command = CR(Link, COMMAND, Link, COMMAND_SIGNATURE);
    if (StriCmp (Command->Cmd, CmdName) == 0) {
      Dispatch = Command->Dispatch;
      break;
    }
  }

  ReleaseLock (&SEnvLock);
  return Dispatch;
}


EFI_STATUS
SEnvBreak (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_BREAKPOINT();
  return EFI_SUCCESS;
}

