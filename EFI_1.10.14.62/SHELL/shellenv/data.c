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

  data.c
  
Abstract:

  Shell Environment driver global data

Revision History

--*/

#include "shelle.h"

//
// IDs of different variables stored by the shell environment
//
EFI_GUID SEnvEnvId = ENVIRONMENT_VARIABLE_ID;
EFI_GUID SEnvMapId = DEVICE_PATH_MAPPING_ID;
EFI_GUID SEnvProtId = PROTOCOL_ID_ID;
EFI_GUID SEnvAliasId = ALIAS_ID;

//
//
//
EFI_SHELL_ENVIRONMENT SEnvInterface = {
  SEnvExecute,
  SEnvGetEnv,
  SEnvGetMap,
  SEnvAddCommand,
  SEnvAddProtocol,
  SEnvGetProtocol,
  SEnvGetCurDir,
  SEnvFileMetaArg,
  SEnvFreeFileList,

  SEnvNewShell,

  SEnvBatchIsActive,

  SEnvFreeResources
} ;


//
// SEnvIoFromCon - used to access the console interface as a file handle
//
EFI_FILE SEnvIOFromCon = {
  EFI_FILE_HANDLE_REVISION,
  SEnvConIoOpen,
  SEnvConIoNop,
  SEnvConIoNop,
  SEnvConIoRead,
  SEnvConIoWrite,
  SEnvConIoGetPosition,
  SEnvConIoSetPosition,
  SEnvConIoGetInfo,
  SEnvConIoSetInfo,
  SEnvConIoNop
} ;

EFI_FILE SEnvErrIOFromCon = {
  EFI_FILE_HANDLE_REVISION,
  SEnvConIoOpen,
  SEnvConIoNop,
  SEnvConIoNop,
  SEnvErrIoRead,
  SEnvErrIoWrite,
  SEnvConIoGetPosition,
  SEnvConIoSetPosition,
  SEnvConIoGetInfo,
  SEnvConIoSetInfo,
  SEnvConIoNop
} ;

//
// SEnvConToIo - used to access the console interface as a file handle
//
EFI_SIMPLE_TEXT_OUTPUT_MODE SEnvConToIoMode = {
  0,
  0,
  EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK),
  0,
  0,
  TRUE
} ;

EFI_SIMPLE_TEXT_OUTPUT_MODE SEnvDummyConToIoMode = {
  0,
  0,
  EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK),
  0,
  0,
  TRUE
} ;

EFI_SIMPLE_TEXT_OUT_PROTOCOL SEnvConToIo = {
  SEnvReset,
  SEnvOutputString,
  SEnvTestString,
  SEnvQueryMode,
  SEnvSetMode,
  SEnvSetAttribute,
  SEnvClearScreen,
  SEnvSetCursorPosition,
  SEnvEnableCursor,
  &SEnvConToIoMode
} ;

EFI_SIMPLE_TEXT_OUT_PROTOCOL SEnvDummyConToIo = {
  SEnvDummyReset,
  SEnvDummyOutputString,
  SEnvDummyTestString,
  SEnvDummyQueryMode,
  SEnvDummySetMode,
  SEnvDummySetAttribute,
  SEnvDummyClearScreen,
  SEnvDummySetCursorPosition,
  SEnvDummyEnableCursor,
  &SEnvDummyConToIoMode
} ;

//
// SEnvLock - guards all shell data except the guid database
//
FLOCK SEnvLock;

//
// SEnvGuidLock - guards the guid data
//
FLOCK SEnvGuidLock;
