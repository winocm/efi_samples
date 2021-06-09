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

  if.c
  
Abstract:

  Internal Shell cmd "if" & "endif"

Revision History

--*/

#include "shelle.h"


//
// Internal prototypes
//
EFI_STATUS
CheckIfFileExists ( 
  IN  CHAR16                   *FileName,
  OUT BOOLEAN                  *FileExists
  );

EFI_STATUS
SEnvCmdIf (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Shell command "if" for conditional execution in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully

--*/
{
  BOOLEAN                      ExistNot;
  UINTN                        NotPos;
  EFI_STATUS                   Status;
  CHAR16                       *FileName;
  BOOLEAN                      FileExists;
  CHAR16                       *String1;
  CHAR16                       *String2;
  BOOLEAN                      Success;
  BOOLEAN                      Condition;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle, 
    SystemTable, 
    SEnvCmdIf,
    L"if",      // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    Print (L"if: Only supported in script files\n");
    return EFI_UNSUPPORTED;
  }

  //
  //  There are 2 forms of the if command:
  //    if [not] exist file then
  //    if [not] string1 == string2
  //  Both forms have argument count not less than 4
  //
  if (SI->Argc < 4) {
    Print (L"if: Too few arguments(line %d)\n", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (!SEnvBatchGetGotoActive ()) {
    //
    // If Goto is not active, do argument checking, judge the condition,
    // then push the statement to stack.
    //
    if ((StriCmp( SI->Argv[1], L"not" ) == 0)) {
      ExistNot = TRUE;
      NotPos = 1;
  
    } else {
      ExistNot = FALSE;  
      NotPos = 0;
    }  

    if (StriCmp (SI->Argv[NotPos+1], L"exist") == 0) {
      //
      //  first form of the command, test for file existence
      //
      if((SI->Argc != NotPos + 4) || 
        (StriCmp (SI->Argv[NotPos + 3], L"then") != 0)) {
        Print (L"if: Incorrect syntax(line %d)\n", SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_INVALID_PARAMETER;
      }

      FileName = SI->Argv[NotPos + 2];

      //
      //  Test for file existence
      //
      Status = CheckIfFileExists (FileName, &FileExists);
      if (EFI_ERROR (Status)) {
        Print (L"if: Check file existence fail - %r(line %d)\n", 
          Status, SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return Status;
      }

      Status = SEnvBatchPushStmtStack (StmtIf, FALSE);
      if (EFI_ERROR (Status)) {
        Print (L"if: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }
    
      Condition = (BOOLEAN)(ExistNot? !FileExists : FileExists);
      Status = SEnvBatchSetCondition (Condition);
      if (EFI_ERROR (Status)) {
        Print (L"if: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }
    
    } else {
      //
      //  Second form of the command, compare two strings
      //
      if ((SI->Argc != NotPos + 5) || 
        (StriCmp (SI->Argv[NotPos + 2], L"==") != 0) ||
        (StriCmp (SI->Argv[NotPos + 4], L"then" ) != 0)) {
        Print (L"if: Incorrect syntax(line %d)\n", SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_INVALID_PARAMETER;
      }

      String1 = SI->Argv[NotPos + 1];
      String2 = SI->Argv[NotPos + 3];

      Status = SEnvBatchPushStmtStack (StmtIf, FALSE);
      if (EFI_ERROR (Status)) {
        Print (L"if: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }
      
      if (ExistNot) {
        Condition = (BOOLEAN)(StriCmp (String1, String2) != 0);
      
      } else {
        Condition = (BOOLEAN)(StriCmp (String1, String2) == 0);
      }
    
      Status = SEnvBatchSetCondition (Condition);
      if (EFI_ERROR (Status)) {
        Print (L"if: Cannot execute script -%r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }
    }
    return Status;
    
  } else {
    //
    // If Goto is active, maintain the JumpStmt so that it always points to
    // the current statement, or maintain the ExtraStmtStack if this statement
    // does not belong the the statement stack. 
    //
    if (SEnvBatchGetRewind ()) {
      Status = SEnvTryMoveUpJumpStmt (StmtIf, &Success);
      if (EFI_ERROR (Status)) {
        Print (L"if: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }

      if (Success) {
        return Status;
      }
    }
    
    Status = SEnvBatchPushStmtStack (StmtIf, TRUE);
    if (EFI_ERROR (Status)) {
      Print (L"if: Cannot execute script - %r\n", Status);
      SEnvSetBatchAbort ();
      return Status;
    }
      
    return Status;
  }
}


EFI_STATUS
CheckIfFileExists ( 
  IN  CHAR16                   *FileName,
  OUT BOOLEAN                  *FileExists
  )
/*++

Routine Description:

  Check file parameter to see if file exists.  Wildcards are supported. If 
  the argument expands to more than one file names, we still return TRUE in 
  the output parameter FileExists.

Arguments:
  FileName         The file name needs to check existence
  FileExists       Output if the file exists

Returns:
  EFI_SUCCESS      The function completed successfully

--*/
{
  EFI_LIST_ENTRY               FileList;
  EFI_LIST_ENTRY               *Link;
  SHELL_FILE_ARG               *Arg;
  EFI_STATUS                   Status;
  UINTN                        FileCount;

  *FileExists = FALSE;
  FileCount = 0;
  InitializeListHead (&FileList);

  //
  // Attempt to open the file, expanding any wildcards.
  //
  Status = ShellFileMetaArg (FileName, &FileList);
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      ShellFreeFileList (&FileList);
      return EFI_SUCCESS;
    
    } else {
      return Status;
    }
  }
  
  //
  // Go through the list and count the files which already exists
  //
  FileCount = 0;
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (Arg->Handle) {
      //
      //  Non-NULL handle means file was there and open-able
      //
      FileCount += 1;
    }
  }

  if (FileCount > 0) {
    //
    //  Found one or more files, so set the flag to be TRUE
    //
    *FileExists = TRUE;
  }

  ShellFreeFileList (&FileList);
  return Status;
}


EFI_STATUS
SEnvCmdEndif (
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE          *SystemTable
  )
/*++


Routine Description:

  Shell command "endif" for conditional execution in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully

--*/
{
  EFI_STATUS                   Status;
  EFI_BATCH_STATEMENT          *Stmt;
  
#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle, 
    SystemTable, 
    SEnvCmdEndif,
    L"endif",   // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    Print (L"endif: Only supported in script files\n");
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 1) {
    Print (L"endif: Too many arguments(line %d)\n", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }
  
  if (!SEnvBatchGetGotoActive ()) {
    //
    // If Goto is not active, pop from the statement stack
    //
    Stmt = SEnvBatchStmtStackTop ();
    if (Stmt == NULL || Stmt->StmtType != StmtIf) {
      Print (L"endif: No corresponding IF statement(line %d)\n", 
        SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }
  
    Status = SEnvBatchPopStmtStack (1, FALSE);
    if (EFI_ERROR (Status)) {
      Print (L"endif: Cannot execute script - %r\n", Status);
      SEnvSetBatchAbort ();
      return Status;
    }
  
  } else {
    //
    // If Goto is active, if ExtraStmtStack is not empty, pop a node from it,
    // otherwise move the JumpStmt down so that it still points to current 
    // statement.
    //
    if (!SEnvBatchExtraStackEmpty ()) {
      Stmt = SEnvBatchExtraStackTop ();
      if (Stmt->StmtType != StmtIf) {
        Print (L"endif: No corresponding IF statement(line %d)\n",
          SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }
      
      Status = SEnvBatchPopStmtStack (1, TRUE);
      if (EFI_ERROR(Status)) {
        Print (L"endif: Cannot execute script - %r\n", Status);
        SEnvSetBatchAbort ();
        return Status;
      }

    } else {
      Status = SEnvMoveDownJumpStmt (StmtIf);
      if (EFI_ERROR(Status)) {
        if (Status == EFI_NOT_FOUND) {
          Print (L"endif: No corresponding IF statement(line %d)\n",
            SEnvGetLineNumber ());

        } else {
          Print (L"endif: Cannot execute script - %r\n", Status);
        }

        SEnvSetBatchAbort ();
        return Status;
      }
    }
  }
  return Status;
}

EFI_STATUS
SEnvCmdElse (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Shell command "else" for conditional execution in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully

--*/
{
  EFI_STATUS                   Status;
  BOOLEAN                      Condition;
  EFI_BATCH_STATEMENT          *Stmt;
  
#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle, 
    SystemTable, 
    SEnvCmdEndif,
    L"endif",   // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    Print (L"else: Only supported in script files\n");
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 1) {
    Print (L"else: Too many arguments(line %d)\n", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }
  
  if (!SEnvBatchGetGotoActive ()) {
    Stmt = SEnvBatchStmtStackTop ();
    if (Stmt == NULL || Stmt->StmtType != StmtIf) {
      Print (L"else: No corresponding IF statement(line %d)\n", 
        SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }
    
    if (Stmt->StmtInfo.IfInfo.FoundElse) {
      Print (L"else: Only 1 ELSE is allowed in a IF statement(line %d)\n", 
        SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }

    //
    // Reverse the Condition flag, and set the FoundElse flag
    //     
    Condition = SEnvBatchGetCondition ();
    Stmt->StmtInfo.IfInfo.FoundElse = TRUE;
    Status = SEnvBatchSetCondition ((BOOLEAN)!Condition);
    if (EFI_ERROR (Status)) {
      Print (L"else: Cann't execute script - %r\n");
      SEnvSetBatchAbort ();
      return Status;
    }
  
  } else {
    //
    // If Goto is active, if ExtraStmtStack is not empty, set FoundElse flag of
    // its top node, otherwise set the FoundElse flag of the node JumpStmt is
    // pointing to.
    //
    if (!SEnvBatchExtraStackEmpty ()) {
      Stmt = SEnvBatchExtraStackTop ();
      if (Stmt->StmtType != StmtIf) {
        Print (L"else: No corresponding IF statement(line %d)\n", 
          SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }

      if (Stmt->StmtInfo.IfInfo.FoundElse) {
        Print (L"else: Only 1 ELSE is allowed in a IF statement(line %d)\n",
          SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }
      Stmt->StmtInfo.IfInfo.FoundElse = TRUE;      

    } else {
      if (SEnvGetJumpStmt()->StmtInfo.IfInfo.FoundElse) {
        Print (L"else: Only 1 ELSE is allowed in a IF statement(line %d)\n",
          SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }
      SEnvGetJumpStmt()->StmtInfo.IfInfo.FoundElse = TRUE;
    }
  }
  return Status;
}
