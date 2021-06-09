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

  for.c
  
Abstract:

  Internal Shell cmd "for" & "endfor"

Revision History

--*/

#include "shelle.h"

EFI_STATUS
SEnvCmdFor (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Shell command "for" for loop in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully
  
--*/
{
  EFI_STATUS                   Status;
  BOOLEAN                      Success;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle, SystemTable, SEnvCmdFor,
    L"for",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    Print( L"for: Only supported in script files\n" );
    return EFI_UNSUPPORTED;
  }

  //
  // Check the command line syntax. The syntax of statement for is:
  //   for %<var> in <string | file [[string | file]...]>
  //
  if (SI->Argc < 4) {
    Print (L"for: Too few arguments(line %d)\n", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }
  
  if (StriCmp (SI->Argv[2], L"in") != 0) {
    Print (L"for: Incorrect syntax(line %d)\n", SEnvGetLineNumber());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }
  
  if (!SEnvBatchGetGotoActive ()) {
    //
    // Extra check. This checking is only done when Goto is not active, for
    // it's legal for encountering a loop use a same named variable as the 
    // current loop when we're searching for the target label.
    //
    if ((StrLen (SI->Argv[1]) != 1) || !IsAlpha (SI->Argv[1][0])) {
      Print (L"for: Incorrect syntax(line %d)\n", SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return EFI_INVALID_PARAMETER;
    }

    //
    //  If Goto is not active, then push it to the statement stack.
    //    
    Status = SEnvBatchPushStmtStack (StmtFor, FALSE);
    if (EFI_ERROR (Status)) {
      Print (L"for: Cannot execute script - %r\n", Status);
      SEnvSetBatchAbort ();
      return Status;
    }
  
  } else {
    //
    // If Goto is active, maintain the JumpStmt or the extra statement stack,
    // so that JumpStmt points to the current statement, or extra stack holds
    // the current statement.
    //
    if (SEnvBatchGetRewind()) {
      Status = SEnvTryMoveUpJumpStmt (StmtFor, &Success);
      if (EFI_ERROR (Status)) {
        Print (L"for: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }
      
      if (Success) {
        return Status;
      }
    }
    Status = SEnvBatchPushStmtStack (StmtFor, TRUE);
    if (EFI_ERROR (Status)) {
      Print (L"for: Cannot execute script - %r\n", Status);
      SEnvSetBatchAbort ();
      return Status;
    }
  }
  
  return Status;
}


EFI_STATUS
SEnvCmdEndfor (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

    Shell command "endfor".

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully

--*/
{
  EFI_STATUS                   Status;
  EFI_LIST_ENTRY               *VarLink;
  EFI_BATCH_STATEMENT          *Stmt;
  EFI_BATCH_VAR_VALUE          *VarValue;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle, SystemTable, SEnvCmdEndfor,
    L"endfor",  // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive()) {
    Print( L"endfor: Only supported in script files\n" );
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 1) {
    Print (L"endfor: Too many arguments(line %d)\n", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (!SEnvBatchGetGotoActive ()) {
    //
    // If Goto is not active, do the following steps:
    //   1. Check for corresponding "for"
    //   2. Delete one node from variable value list
    //   3. If the variable value list is empty, stop looping, otherwise
    //      continue loop
    //
    Stmt = SEnvBatchStmtStackTop ();
    if (Stmt == NULL || Stmt->StmtType != StmtFor) {
      Print (L"endfor: No corresponding FOR statement(line %d)\n",
        SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }

    //
    // It's possible for ValueList to be empty(for example, a "for" in a false
    // condition "if"). If so, we need not delete a value node from it.
    // 
    if (!IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
      VarLink = Stmt->StmtInfo.ForInfo.ValueList.Flink;
      VarValue = CR (
                   VarLink, 
                   EFI_BATCH_VAR_VALUE, 
                   Link, 
                   EFI_BATCH_VAR_SIGNATURE
                   );
    
      //
      //  Free the string contained in the first node of variable value list
      //
      if (VarValue->Value != NULL) {
        FreePool(VarValue->Value);
        VarValue->Value = NULL;
      }

      //
      //  Remove the first node from the variable value list
      //
      RemoveEntryList (&VarValue->Link);
      FreePool(VarValue);
      VarValue = NULL;
    }
    
    //
    //  If there is another value, then jump back to top of loop,
    //  otherwise, exit this FOR loop & pop out the statement.
    //
    if (!IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
     
      //
      //  Set script file position back to top of this loop
      //
      SEnvSetLineNumber (Stmt->StmtInfo.ForInfo.BeginLineNum);
      Status = SEnvBatchSetFilePosition (Stmt->BeginFilePos);
      if (EFI_ERROR (Status)) {
        Print (L"endfor: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }
      
    } else {
      //
      // Pop the statement out of stack to exit loop
      //
      Status = SEnvBatchPopStmtStack (1, FALSE);
      if (EFI_ERROR (Status)) {
        Print (L"endfor: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }
    }
    
    return EFI_SUCCESS;
  
  } else {
    //
    // if Goto is active, maintain the JumpStmt or the extra statement stack.
    //
    if (!SEnvBatchExtraStackEmpty ()) {
      Stmt = SEnvBatchExtraStackTop ();
      if (Stmt->StmtType != StmtFor) {
        Print (L"endfor: No corresponding FOR statement(line %d)\n",
          SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }

      Status = SEnvBatchPopStmtStack (1, TRUE);
      if (EFI_ERROR(Status)) {
        Print (L"endfor: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }

    } else {
      Status = SEnvMoveDownJumpStmt (StmtFor);
      if (EFI_ERROR (Status)) {
        if (Status == EFI_NOT_FOUND) {
          Print (L"endfor: No corresponding FOR statement(line %d)\n",
            SEnvGetLineNumber ());
        } else {
          Print (L"endfor: Cannot execute script - %r\n", Status);
        }
        SEnvSetBatchAbort ();
        return Status;
      }
    }
    return Status;
  }  
}
