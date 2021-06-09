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

  shelle.h

Abstract:

Revision History

--*/

#ifndef _SHELLE_H_
#define _SHELLE_H_

#include "shell.h"
#include "shellenv.h"

//
// Internal defines
//
#define DEFAULT_CMD_SIGNATURE  EFI_SIGNATURE_32('d','f','c','s')
typedef struct {
  UINTN           Signature;
  EFI_LIST_ENTRY  Link;
  CHAR16          *Line;
  CHAR16          Buffer[80];
} DEFAULT_CMD;

#define COMMON_CMDLINE_LENGTH      32
#define COMMON_ARG_COUNT           32
#define COMMON_ARG_LENGTH          32

//
// Keep these 2 macros because conio.c and map.c(maybe more other files) uses
// them!!
//
#define MAX_CMDLINE         256
#define MAX_ARG_LENGTH      256

#define NON_VOL             1
#define VOL                 0


#define IsWhiteSpace(c)     (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0)
#define IsValidChar(c)      (c >= ' ')
#define IsDigit(c)          (c >= '0' && c <= '9')
#define IsAlpha(c)          ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z' ))

#define GOTO_TARGET_FOUND        (1)
#define GOTO_TARGET_NOT_FOUND    (2)
#define GOTO_TARGET_DOESNT_EXIST (3)

//
// Internal structures
//
#define VARIABLE_SIGNATURE  EFI_SIGNATURE_32('v','i','d',' ')
typedef struct {
  UINTN               Signature;
  EFI_LIST_ENTRY      Link;
  CHAR16              *Name;

  UINTN               ValueSize;
  union {
    UINT8             *Value;
    CHAR16            *Str;
  } u;

  CHAR16              *CurDir;
  UINT8               Flags ;
} VARIABLE_ID;


//
// IDs of different variables stored by the shell environment
//
#define ENVIRONMENT_VARIABLE_ID  \
  { 0x47c7b224, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define DEVICE_PATH_MAPPING_ID  \
  { 0x47c7b225, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define PROTOCOL_ID_ID  \
  { 0x47c7b226, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define ALIAS_ID  \
  { 0x47c7b227, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

//
//
//
#define ENV_REDIR_SIGNATURE         EFI_SIGNATURE_32('r','i','d','s')
typedef struct {
  UINTN                           Signature;
  BOOLEAN                         Ascii;
  EFI_STATUS                      WriteError;
  EFI_FILE_HANDLE                 File;
  EFI_DEVICE_PATH_PROTOCOL        *FilePath;
  EFI_HANDLE                      Handle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL    Out;
  EFI_SIMPLE_TEXT_IN_PROTOCOL     In;
} ENV_SHELL_REDIR_FILE;

typedef struct {
  EFI_SHELL_INTERFACE             ShellInt;
  EFI_SYSTEM_TABLE                *SystemTable;

  ENV_SHELL_REDIR_FILE            StdIn;
  ENV_SHELL_REDIR_FILE            StdOut;
  ENV_SHELL_REDIR_FILE            StdErr;

} ENV_SHELL_INTERFACE;

//
// Internal prototypes from init.c
//
EFI_SHELL_INTERFACE *
SEnvNewShell (
  IN EFI_HANDLE                   ImageHandle
  );

//
// Internal prototypes from cmddisp.c
//
VOID
SEnvInitCommandTable (
  VOID
  );

EFI_STATUS
SEnvAddCommand (
  IN SHELLENV_INTERNAL_COMMAND    Handler,
  IN CHAR16                       *Cmd,
  IN CHAR16                       *CmdFormat,
  IN CHAR16                       *CmdHelpLine,
  IN CHAR16                       *CmdVerboseHelp
  );

SHELLENV_INTERNAL_COMMAND
SEnvGetCmdDispath(
  IN CHAR16                   *CmdName
  );

//
// From exec.c
//
VOID
SEnvFreeArgument (
  IN  UINTN                   *Argc,
  IN  CHAR16                  ***Argv
  );

EFI_STATUS
SEnvExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN BOOLEAN              DebugOutput
  );

EFI_STATUS
SEnvDoExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN ENV_SHELL_INTERFACE  *Shell,
  IN BOOLEAN              Output
  );

EFI_STATUS
SEnvExit (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvStringToArg (
  IN CHAR16       *Str,
  IN BOOLEAN      Output,
  OUT CHAR16      ***pArgv,
  OUT UINTN       *pArgc
  );

//
// Internal prototypes from protid.c
//
VOID
SEnvInitProtocolInfo (
  VOID
  );

EFI_STATUS
SEnvLoadDefaults (
  IN EFI_HANDLE           Parent,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvReloadDefaults (
  IN EFI_HANDLE           Parent,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN OUT BOOLEAN          *MappingModified
  );

VOID
SEnvLoadInternalProtInfo (
  VOID
  );

VOID
SEnvFreeHandleProtocolInfo (
  VOID
  );

VOID
SEnvAddProtocol (
  IN EFI_GUID                     *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpInfo OPTIONAL,
  IN CHAR16                       *IdString
  );

VOID
SEnvIAddProtocol (
  IN BOOLEAN                      SaveId,
  IN EFI_GUID                     *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpInfo OPTIONAL,
  IN CHAR16                       *IdString
  );

VOID
SEnvLoadHandleProtocolInfo (
  IN EFI_GUID                     *Skip
  );

CHAR16 *
SEnvGetProtocol (
  IN EFI_GUID                     *ProtocolId,
  IN BOOLEAN                      GenId
  );

EFI_STATUS
SEnvCmdProt (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

EFI_STATUS
SEnvCmdOpenInfo (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_STATUS
SEnvDHProt (
  IN BOOLEAN                      Verbose,
  IN BOOLEAN                      DriverModel,
  IN UINTN                        HandleNo,
  IN EFI_HANDLE                   Handle,
  IN CHAR8                        *Language
  );

EFI_STATUS
SEnvCmdDH (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdDeviceTree (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdDriverConfiguration (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdDriverDiagnostics (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdDrivers (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdDevices (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvIGetProtID (
  IN CHAR16           *Str,
  OUT EFI_GUID        *ProtId
  );

EFI_STATUS
SEnvCmdUnload (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

//
// Handle.c
//
VOID
SEnvInitHandleGlobals(
  VOID
  );

VOID
SEnvLoadHandleTable (
  VOID
  );

VOID
SEnvFreeHandleTable (
  VOID
  );

UINTN
SEnvHandleNoFromStr(
  IN CHAR16       *Str
  );

EFI_HANDLE
SEnvHandleFromStr(
  IN CHAR16       *Str
  );

UINTN
SEnvHandleToNumber (
  IN  EFI_HANDLE  Handle
  );

//
// Internal prototypes from var.c
//
VOID
SEnvInitVariables (
  VOID
  );

EFI_STATUS
SEnvCmdSet (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdAlias (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

CHAR16 *
SEnvGetMap (
  IN CHAR16           *Name
  );

CHAR16 *
SEnvGetEnv (
  IN CHAR16           *Name
  );

CHAR16 *
SEnvGetAlias (
  IN CHAR16           *Name
  );

//
// Prototypes from conio.c
//
VOID
SEnvConIoInitDosKey (
  VOID
  );

EFI_STATUS
SEnvConIoOpen (
  IN struct _EFI_FILE_HANDLE  *File,
  OUT struct _EFI_FILE_HANDLE **NewHandle,
  IN CHAR16                   *FileName,
  IN UINT64                   OpenMode,
  IN UINT64                   Attributes
  );

EFI_STATUS
SEnvConIoNop (
  IN struct _EFI_FILE_HANDLE  *File
  );

EFI_STATUS
SEnvConIoGetPosition (
  IN struct _EFI_FILE_HANDLE  *File,
  OUT UINT64                  *Position
  );

EFI_STATUS
SEnvConIoSetPosition (
  IN struct _EFI_FILE_HANDLE  *File,
  OUT UINT64                  Position
  );

EFI_STATUS
SEnvConIoGetInfo (
  IN struct _EFI_FILE_HANDLE  *File,
  IN EFI_GUID                 *InformationType,
  IN OUT UINTN                *BufferSize,
  OUT VOID                    *Buffer
  );

EFI_STATUS
SEnvConIoSetInfo (
  IN struct _EFI_FILE_HANDLE  *File,
  IN EFI_GUID                 *InformationType,
  IN UINTN                    BufferSize,
  OUT VOID                    *Buffer
  );

EFI_STATUS
SEnvConIoWrite (
  IN struct _EFI_FILE_HANDLE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  );

EFI_STATUS
SEnvConIoRead (
  IN struct _EFI_FILE_HANDLE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  );

EFI_STATUS
SEnvErrIoWrite (
  IN struct _EFI_FILE_HANDLE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  );

EFI_STATUS
SEnvErrIoRead (
  IN struct _EFI_FILE_HANDLE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  );


EFI_STATUS
SEnvReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  );

EFI_STATUS
SEnvOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
SEnvTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
SEnvQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber,
  OUT UINTN                       *Columns,
  OUT UINTN                       *Rows
  );

EFI_STATUS
SEnvSetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber
  );

EFI_STATUS
SEnvSetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN UINTN                            Attribute
  );

EFI_STATUS
SEnvClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This
  );

EFI_STATUS
SEnvSetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        Column,
  IN UINTN                        Row
  );

EFI_STATUS
SEnvEnableCursor (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                      Enable
  );


EFI_STATUS
SEnvDummyReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  );

EFI_STATUS
SEnvDummyOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
SEnvDummyTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS 
SEnvDummyQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber,
  OUT UINTN                       *Columns,
  OUT UINTN                       *Rows
  );

EFI_STATUS
SEnvDummySetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber
  );

EFI_STATUS
SEnvDummySetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN UINTN                            Attribute
  );

EFI_STATUS
SEnvDummyClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This
  );

EFI_STATUS
SEnvDummySetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        Column,
  IN UINTN                        Row
  );

EFI_STATUS
SEnvDummyEnableCursor (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                      Enable
  );

//
// Prototypes from dprot.c
//
CHAR8 *GetPdbPath (VOID *ImageBase);
VOID  PrintShortPdbFileName (CHAR8  *PdbFileName,UINTN  Length);

VOID SEnvBlkIo (EFI_HANDLE, VOID *);
VOID SEnvComponentName (EFI_HANDLE, VOID *);
VOID SEnvDPath (EFI_HANDLE, VOID *);
VOID SEnvDPathTok (EFI_HANDLE, VOID *);
VOID SEnvDebugSupport (EFI_HANDLE, VOID *);
VOID SEnvImage (EFI_HANDLE, VOID *);
VOID SEnvImageTok (EFI_HANDLE, VOID *);
VOID SEnvIsaIo (EFI_HANDLE, VOID *);
VOID SEnvPciIo (EFI_HANDLE, VOID *);
VOID SEnvPciRootBridgeIo (EFI_HANDLE, VOID *);
VOID SEnvTextOut (EFI_HANDLE, VOID *);
VOID SEnvUsbIo (EFI_HANDLE, VOID *);
VOID SEnvBusSpecificDriverOverride (EFI_HANDLE, VOID *);

//
// Prototypes from map.c
//
VOID
SEnvInitMap (
  VOID
  );

CHAR16 *
SEnvGetDefaultMapping (
  IN EFI_HANDLE ImageHandle
  );

EFI_STATUS
SEnvCmdMap (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdConnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdDisconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdReconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvCmdMount (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

VARIABLE_ID *
SEnvMapDeviceFromName (
  IN OUT CHAR16   **pPath
  );

EFI_DEVICE_PATH_PROTOCOL *
SEnvIFileNameToPath (
  IN CHAR16               *Path
  );

EFI_DEVICE_PATH_PROTOCOL *
SEnvFileNameToPath (
  IN CHAR16               *Path
  );

EFI_DEVICE_PATH_PROTOCOL *
SEnvNameToPath (
  IN CHAR16                   *PathName
  );

EFI_STATUS
SEnvSetCurrentDevice (
  IN CHAR16       *Name
  );

CHAR16 *
SEnvGetCurDir (
  IN CHAR16       *DeviceName OPTIONAL
  );

EFI_STATUS
SEnvCmdCd (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from echo.c
//
EFI_STATUS
SEnvCmdEcho (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from if.c
//
EFI_STATUS
SEnvCmdIf (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvCmdElse (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvCmdEndif (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from wait.c
//
EFI_STATUS
SEnvCmdWait (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from goto.c
//
EFI_STATUS
SEnvCmdGoto (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvCheckForGotoTarget(
  IN  CHAR16 *Candidate,
  IN  UINT64 GotoFilePos,
  IN  UINT64 FilePosition,
  OUT UINTN  *GotoTargetStatus
  );

VOID
SEnvPrintLabelNotFound(
  VOID
  );

VOID
SEnvInitTargetLabel(
  VOID
  );

VOID
SEnvFreeTargetLabel(
  VOID
  );

//
// Prototypes from for.c
//
VOID
SEnvInitForLoopInfo (
  VOID
  );

EFI_STATUS
SEnvSubstituteForLoopIndex(
  IN CHAR16  *Str,
  OUT CHAR16 **Val
  );

EFI_STATUS
SEnvCmdFor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvCmdEndfor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from pause.c
//
EFI_STATUS
SEnvCmdPause (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from marg.c
//
CHAR16 *
SEnvFileHandleToFileName (
  IN EFI_FILE_HANDLE      Handle
  );

EFI_STATUS
SEnvFreeFileList (
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
SEnvFileMetaArg (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

VOID
EFIStructsPrint (
  IN  VOID                     *Buffer,
  IN  UINTN                    BlockSize,
  IN  UINT64                   BlockAddress,
  IN  EFI_BLOCK_IO_PROTOCOL    *BlkIo
);

EFI_STATUS
DumpBlockDev (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

//
// File system mapping info
//
#define MAPPING_INFO_SIGNATURE    EFI_SIGNATURE_32('m','p','i','s')
typedef struct {
  UINTN                           Signature;
  EFI_LIST_ENTRY                  Link;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  CHAR16                          *MappedName;
  UINT32                          MediaId;
  BOOLEAN                         Valid;
  BOOLEAN                         Accessed;
} MAPPING_INFO;

//
// Device path info
//
#define DEVICEPATH_INFO_SIGNATURE EFI_SIGNATURE_32('d','p','i','s')
typedef struct {
  UINTN                           Signature;
  EFI_LIST_ENTRY                  Link;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  BOOLEAN                         Found;
} DEVICEPATH_INFO;

VOID SEnvGetCurMappings ();
VOID SEnvCheckValidMappings (MAPPING_INFO *CurMappingInfo);
VOID SEnvGetDevicePathList ();
BOOLEAN SEnvCompareDevicePathList ();

//
// Global data
//
extern EFI_GUID SEnvEnvId;
extern EFI_GUID SEnvMapId;
extern EFI_GUID SEnvProtId;
extern EFI_GUID SEnvAliasId;
extern EFI_SHELL_ENVIRONMENT SEnvInterface;
extern EFI_FILE SEnvIOFromCon;
extern EFI_FILE SEnvErrIOFromCon;
extern FLOCK SEnvLock;
extern FLOCK SEnvGuidLock;
extern UINTN SEnvNoHandles;
extern EFI_HANDLE *SEnvHandles;
extern EFI_SIMPLE_TEXT_OUT_PROTOCOL SEnvConToIo;
extern EFI_SIMPLE_TEXT_OUT_PROTOCOL SEnvDummyConToIo;
//
// Data structures and Prototypes from batch.c
//
//
// Goto target searching status
//
#define GOTO_TARGET_FOUND        (1)
#define GOTO_TARGET_NOT_FOUND    (2)
#define GOTO_TARGET_DOESNT_EXIST (3)

//
// Data structure for batch script processing
//

//
//  Definitions for the statement stack
//
//  To allow checking if/endif or for/endif matching, and to prevent from
//  jumpting into another loop using goto, for each script, a statement 
//  stack is maintained. It contain the nesting information of the if/for 
//  statements. For the if statement, the true or false of the condition
//  is recorded; for the for statement, the variable name and a list of 
//  variable values are recorded.
//
#define EFI_BATCH_VAR_SIGNATURE EFI_SIGNATURE_32('b','v','a','r')
typedef struct {
  UINTN                        Signature;
  EFI_LIST_ENTRY               Link;
  
  CHAR16                       *Value;
} EFI_BATCH_VAR_VALUE;

typedef enum {
  StmtFor, 
  StmtIf,
  StmtUndefined
} EFI_BATCH_STMT_TYPE;

#define EFI_BATCH_STMT_SIGNATURE EFI_SIGNATURE_32('b','s','m','t')
typedef struct {
  CHAR16                       VariableName[1];
  EFI_LIST_ENTRY               ValueList;
  UINTN                        BeginLineNum;
} EFI_BATCH_FOR_INFO;

typedef struct {
  BOOLEAN                      Condition;
  BOOLEAN                      FoundElse;
} EFI_BATCH_IF_INFO;
  
typedef union {
  EFI_BATCH_FOR_INFO           ForInfo;
  EFI_BATCH_IF_INFO            IfInfo;
} EFI_BATCH_STMT_INFO;

typedef struct {
  UINTN                        Signature;
  EFI_LIST_ENTRY               Link;
  
  EFI_BATCH_STMT_TYPE          StmtType;
  UINT64                       BeginFilePos;
  EFI_BATCH_STMT_INFO          StmtInfo;
} EFI_BATCH_STATEMENT;

typedef struct {
  UINTN                        NestLevel;
  EFI_LIST_ENTRY               StmtList;
  
} EFI_BATCH_STMT_STACK;

//
//  Definitions for the script stack
//
//  In order to support nested scripts (script calling script calling 
//  script...). There is an script stack "ScriptStack". It is a list of
//  scripts, each script contain a argument list, a statement stack, and
//  the current file position. The argument list contains Argv[0] - Argv[n]
//  of the script, this allows positional argument substitution to be done
//  when each line is read and scanned. 
//
#define EFI_BATCH_ARG_SIGNATURE EFI_SIGNATURE_32('b','a','r','g')
typedef struct {
  UINTN                        Signature;
  EFI_LIST_ENTRY               Link;
  
  CHAR16                       *ArgValue;
} EFI_BATCH_ARGUMENT;

#define EFI_BATCH_SCRIPT_SIGNATURE EFI_SIGNATURE_32('b','s','p','t')
typedef struct {
  UINTN                        Signature;
  EFI_LIST_ENTRY               Link;          
  
  EFI_LIST_ENTRY               ArgListHead;   // Head of argument list
  EFI_BATCH_STMT_STACK         StmtStack;
  UINT64                       FilePosition;  // Current file position
  BOOLEAN                      BatchAbort;
  UINTN                        LineNumber;
//  EFI_SIMPLE_TEXT_OUT_PROTOCOL *TextOut;      // For use in SEnvEchoCommand
} EFI_BATCH_SCRIPT;

typedef struct {
  UINTN                        NestLevel;
  EFI_LIST_ENTRY               ScriptList;
} EFI_BATCH_SCRIPT_STACK;


//
// Prototype declaration
//
VOID
SEnvInitBatch(
  VOID
  );

EFI_STATUS
SEnvBatchSetFilePosition( 
  IN UINT64 NewPos
  );
  
EFI_STATUS
SEnvBatchFindVariable (
  IN  CHAR16                   *VariableName,
  OUT CHAR16                   **Value
  );
  
EFI_BATCH_STATEMENT *
SEnvBatchExtraStackTop (
  VOID
  );
  
EFI_BATCH_STATEMENT *
SEnvBatchStmtStackTop (
  );

/*
BOOLEAN
SEnvBatchIsStmtStackEmpty (
  VOID
  );
*/

EFI_BATCH_STATEMENT *
SEnvGetJumpStmt (
  VOID
  );
    
EFI_STATUS
SEnvBatchFindArgument (
  IN  UINTN                    ArgumentNum,
  OUT CHAR16                   **Value
  );

VOID
SEnvBatchSetGotoActive( 
  VOID
  );
  
BOOLEAN
SEnvBatchGetGotoActive( 
  VOID
  );

UINTN
SEnvGetLineNumber ( 
  VOID
  );

VOID
SEnvSetLineNumber ( 
  UINTN                        LineNumber
  );

BOOLEAN
SEnvBatchGetRewind ( 
  VOID 
  );

EFI_STATUS
SEnvTryMoveUpJumpStmt (
  IN  EFI_BATCH_STMT_TYPE      StmtType,
  OUT BOOLEAN                  *Success
  );

EFI_STATUS
SEnvMoveDownJumpStmt (
  IN  EFI_BATCH_STMT_TYPE      StmtType
  ) ;

EFI_STATUS
SEnvBatchResetJumpStmt (
  VOID
  );

BOOLEAN
SEnvBatchExtraStackEmpty (
  VOID
  );

BOOLEAN
SEnvBatchIsActive (
  VOID
  );

VOID
SEnvFreeResources (
  VOID
  );

VOID
SEnvSetBatchAbort (
  VOID
  );

CHAR16*
SEnvBatchGetLastError (
  VOID
  );

EFI_STATUS
SEnvBatchEchoCommand (
  IN  ENV_SHELL_INTERFACE      *Shell
  );

BOOLEAN
SEnvBatchGetEcho (
  VOID
  );

VOID
SEnvBatchSetEcho (
  IN  BOOLEAN                  Val
  );

EFI_STATUS
SEnvExecuteScript (
  IN  ENV_SHELL_INTERFACE      *Shell,
  IN  EFI_FILE_HANDLE          File
  );

BOOLEAN
SEnvBatchVarIsLastError (
  IN  CHAR16                   *Name
  );

VOID
SEnvBatchGetConsole (
  OUT EFI_SIMPLE_TEXT_IN_PROTOCOL  **ConIn,
  OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL **ConOut
  );

EFI_STATUS
SEnvBatchPushStmtStack (
  IN  EFI_BATCH_STMT_TYPE      StmtType,
  IN  BOOLEAN                  PushExtraStack
  );

EFI_STATUS
SEnvBatchPopStmtStack (
  IN  UINTN                    PopCount,
  IN  BOOLEAN                  PushExtraStack
  );

EFI_STATUS
SEnvBatchSetCondition( 
  IN  BOOLEAN                  Val
  );

BOOLEAN
SEnvBatchGetCondition ( 
  VOID
  );

#endif // _SHELLE_H_
