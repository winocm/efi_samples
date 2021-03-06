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

  shell.h

Abstract:

  Defines for shell applications



Revision History

--*/

#ifndef _SHELL_H_
#define _SHELL_H_

//
// This module is included by shell applications
//

#if defined(__cplusplus)
extern "C" {
#endif

#include "Efi.h"

// #include "EfiUtilityLib.h"
#include "EfiLib.h"
#include "ShellDef11.h"

//
//
//
#define SHELL_FILE_ARG_SIGNATURE    EFI_SIGNATURE_32('g','r','a','f')
typedef struct {
  UINT32                      Signature;
  EFI_LIST_ENTRY              Link;
  EFI_STATUS                  Status;

  EFI_FILE_HANDLE             Parent;
  UINT64                      OpenMode;
  CHAR16                      *ParentName;
  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;

  CHAR16                      *FullName;
  CHAR16                      *FileName;

  EFI_FILE_HANDLE             Handle;
  EFI_FILE_INFO               *Info;
} SHELL_FILE_ARG;


EFI_STATUS
ShellFileMetaArg (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
ShellFreeFileList (
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

//
// Shell application library functions
//
EFI_STATUS
InitializeShellApplication (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_INTERNAL_COMMAND) (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

VOID
InstallInternalShellCommand (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable,
  IN SHELLENV_INTERNAL_COMMAND    Dispatch,
  IN CHAR16                       *Cmd,
  IN CHAR16                       *CmdFormat,
  IN CHAR16                       *CmdHelpLine,
  IN VOID                         *CmdVerboseHelp
  );

//
// Publics in shell.lib
//
extern EFI_GUID ShellInterfaceProtocol;
extern EFI_GUID ShellEnvProtocol;

//
// GetEnvironmentVariable - returns a shell environment variable
//
CHAR16 *
GetEnvironmentVariable (
  IN CHAR16       *Name
  );

//
// GetProtocolId - returns the short ID strings for a protocol guid
//
CHAR16 *
GetProtocolId (
  IN EFI_GUID     *Protocol
  );

//
// AddProtoclId - records a new ID for a protocol guid such that anyone
// performing a GetProtocolId can find our id
//
VOID
AddProtocolId (
  IN EFI_GUID     *Protocol,
  IN CHAR16       *ProtocolId
  );

//
// ShellExecute - causes the shell to parse & execute the command line
//
EFI_STATUS
ShellExecute (
  IN EFI_HANDLE   ParentImageHandle,
  IN CHAR16       *CommandLine,
  IN BOOLEAN      Output
  );

//
// Misc
//
CHAR16 *
MemoryTypeStr (
  IN EFI_MEMORY_TYPE  Type
  );

//
// IO
//
EFI_FILE_HANDLE
ShellOpenFilePath (
  IN EFI_DEVICE_PATH_PROTOCOL     *FilePath,
  IN UINT64                       FileMode
  );

//
// ShellCurDir - returns the current directory on the current mapped device
//               (note the result is allocated from pool and the caller must
//               free it)
//
CHAR16 *
ShellCurDir (
  IN CHAR16               *DeviceName OPTIONAL
  );

//
// ShellBatchIsActive - return current execution is in batch script or not
//
BOOLEAN
ShellBatchIsActive ( 
  VOID
  );

//
// ShellFreeResouces - free all resources that allocated in Shell
//
VOID
ShellFreeResources ( 
  VOID
  );

//
// ShellGetEnv - returns the current mapping for the Env Name
//
CHAR16 *
ShellGetEnv (
  IN CHAR16       *Name
  );

CHAR16 *
ShellGetMap (
  IN CHAR16       *Name
  );

//
// **************************************
//   Shell Interface prototypes
//


//
// Shell Interface - additional information (over image_info) provided
// to an application started by the shell.
//
// ConIo - provides a file style interface to the console.  Note that the
// ConOut & ConIn interfaces in the system table will work as well, and both
// all will be redirected to a file if needed on a command line
//
// The shell interface's and data (including ConIo) are only valid during
// the applications Entry Point.  Once the application returns from it's
// entry point the data is freed by the invoking shell.
//
#define SHELL_INTERFACE_PROTOCOL \
  { 0x47c7b223, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }


typedef struct _EFI_SHELL_INTERFACE {
  // Handle back to original image handle & image info
  EFI_HANDLE                  ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL   *Info;

  // Parsed arg list
  CHAR16                      **Argv;
  UINTN                       Argc;

  // Storage for file redirection args after parsing
  CHAR16                      **RedirArgv;
  UINTN                       RedirArgc;

  // A file style handle for console io
  EFI_FILE_HANDLE             StdIn;
  EFI_FILE_HANDLE             StdOut;
  EFI_FILE_HANDLE             StdErr;

} EFI_SHELL_INTERFACE;

//
// RT - pointer to the runtime table
//
EFI_RUNTIME_SERVICES    *RT;

//
// LibStandalone - TRUE if lib is linked in as part of the firmware.
// N.B. The EFI fw sets this value directly
//
BOOLEAN  LibFwInstance;

//
// Shell library globals
//
extern EFI_SHELL_INTERFACE     *SI;
extern EFI_GUID ShellInterfaceProtocol;
extern EFI_GUID ShellEnvProtocol;
extern UINTN EFIDebug;

//
// Print with page break
//

#define DEFAULT_INIT_ROW   1
#define DEFAULT_AUTO_LF    FALSE //auto line-feed

VOID 
EnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  );

VOID  
DisablePageBreak (VOID);

BOOLEAN
GetPageBreak (VOID);

//
// Help support
//
#define COMMAND_SIGNATURE       EFI_SIGNATURE_32('c','m','d','s')

typedef struct {
  UINTN                       Signature;
  EFI_LIST_ENTRY              Link;
  SHELLENV_INTERNAL_COMMAND   Dispatch;
  CHAR16                      *Cmd;
  CHAR16                      *CmdFormat;
  CHAR16                      *CmdHelpLine;
  VOID                        *CmdVerboseHelp;
} COMMAND;

CHAR16*
GetVerboseHelp(
  IN CHAR16* CmdName
  );

CHAR16*
GetLineHelp(
  IN  CHAR16* CmdName
  );

CHAR16*
GetCmdFormat(
  IN  CHAR16* CmdName
  );

VOID
PrintVerboseHelp(
  IN  CHAR16* VerboseHelp
  );

VOID
PrintHelpInfo(
  IN  CHAR16  *CmdName
  );

extern CHAR16 gHelpData[];

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // _SHELL_H_
