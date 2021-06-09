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

  goto.c
  
Abstract:

  Shell Environment batch goto command

Revision History

--*/

#include "shelle.h"

//
//  Statics
//
STATIC CHAR16                  *TargetLabel;

EFI_STATUS
SEnvCmdGoto(
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Transfers execution of batch file to location following a label (:labelname).

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully
  
--*/
{
  EFI_STATUS                   Status;
  
#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle, 
    SystemTable, 
    SEnvCmdGoto,
    L"goto",    // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    Print (L"goto: Only supported in script files\n");
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 2) {
    Print (L"goto: Too many arguments(line %d)\n", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (SI->Argc < 2) {
    Print (L"goto: Too few arguments(line %d)\n", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }
 
  TargetLabel = StrDuplicate (SI->Argv[1]);
  if (TargetLabel == NULL) {
    Print (L"goto: Out of Resources\n");
    SEnvSetBatchAbort ();
    return EFI_OUT_OF_RESOURCES;
  }

  SEnvBatchSetGotoActive ();
  Status = SEnvBatchResetJumpStmt ();
  if (EFI_ERROR (Status)) {
    Print (L"goto: Cannot execute script - %r\n", Status);
    SEnvSetBatchAbort ();
    return Status;
  }
  
  return Status;
}


EFI_STATUS
SEnvCheckForGotoTarget (
  IN  CHAR16                   *Candidate,
  IN  UINT64                   GotoFilePos, 
  IN  UINT64                   FilePosition, 
  OUT UINTN                    *GotoTargetStatus
  )
/*++

Routine Description:

  Check to see if we have found the target label of a GOTO command.

Arguments:
  Candidate        String to be checked for goto target
  GotoFilePos      File position of the goto statement
  FilePosition     Current file position
  GotoTargetStatus The status of searching goto target
  
Returns:
  EFI_SUCCESS      The command finished sucessfully

--*/
{
  if (!Candidate) {
    return EFI_INVALID_PARAMETER;
  }

  //
  //  See if we found the label (strip the leading ':' off the candidate)
  //  or if we have searched the whole file without finding it.
  //
  if ( GotoFilePos == FilePosition ) {
    *GotoTargetStatus = GOTO_TARGET_DOESNT_EXIST;
    return EFI_SUCCESS;

  } else if (Candidate[0] != ':') {
    *GotoTargetStatus = GOTO_TARGET_NOT_FOUND;
    return EFI_SUCCESS;
  
  } else if (StriCmp (&Candidate[1], TargetLabel) == 0) {
    *GotoTargetStatus = GOTO_TARGET_FOUND;
    return EFI_SUCCESS;

  } else {
    *GotoTargetStatus = GOTO_TARGET_NOT_FOUND;
    return EFI_SUCCESS;
  }
}


VOID
SEnvPrintLabelNotFound ( 
  VOID
  )
/*++

Routine Description:

  Print an error message when a label referenced by a GOTO is not
  found in the script file..

Arguments:

Returns:

--*/
{
  Print( L"goto: Target label \":%s\" not found\n", TargetLabel );
  return;
}


VOID
SEnvInitTargetLabel (
  VOID
  )
/*++

Routine Description:

  Initialize the target label for the GOTO command.

Arguments:

Returns:

--*/
{
  TargetLabel = NULL;
  return;
}


VOID
SEnvFreeTargetLabel (
  VOID
  )
/*++

Routine Description:

  Free the target label saved from the GOTO command.

Arguments:

Returns:

--*/
{
  if (TargetLabel) {
    FreePool (TargetLabel);
    TargetLabel = NULL;
  }
  return;
}
