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

  batch.c
  
Abstract:

  Functions implementing batch scripting service in shell.

Revision History

--*/

#include "shelle.h"

//
//  Constants
//
#define ASCII_LF                     ((CHAR8)0x0a)
#define ASCII_CR                     ((CHAR8)0x0d)
#define UNICODE_LF                   ((CHAR16)0x000a)
#define UNICODE_CR                   ((CHAR16)0x000d)

//
// Buffer size, designed to hold 64-bit hex error numbers + null char
//
#define LASTERROR_BUFSIZ             (17)

//
//  Statics
//  (needed to maintain state across multiple calls or for callbacks)
//
static UINTN                         LastError;
static CHAR16                        LastErrorBuf[LASTERROR_BUFSIZ];
static BOOLEAN                       GotoIsActive;
static UINT64                        GotoFilePos;
static BOOLEAN                       OldEchoIsOn;
static BOOLEAN                       EchoIsOn;
static EFI_SIMPLE_TEXT_IN_PROTOCOL   *OrigConIn;
static EFI_SIMPLE_TEXT_OUT_PROTOCOL  *OrigConOut;
static BOOLEAN                       Rewind;
static EFI_BATCH_STATEMENT           *JumpStmt;

static EFI_BATCH_SCRIPT_STACK        ScriptStack;
static EFI_BATCH_STMT_STACK          ExtraStmtStack;

//
//  Prototypes
//
STATIC EFI_STATUS
BatchIsAscii (
  IN EFI_FILE_HANDLE                 File, 
  OUT BOOLEAN                        *IsAscii
  );

STATIC EFI_STATUS
BatchGetLine (
  IN  EFI_FILE_HANDLE                File, 
  IN  BOOLEAN                        Ascii,
  IN  OUT UINT64                     *FilePosition,
  OUT UINTN                          *CmdLineSize,
  OUT CHAR16                         **CommandLine
  );

//
// Implemetation
//
EFI_STATUS
SEnvBatchResetJumpStmt (
  VOID
  )
/*++

Routine Description:

  Reset the JumpStmt to top of the current statement stack. If the stack is
  empty, set JumpStmt = NULL;

Arguments:

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  EFI_LIST_ENTRY                     *StmtLink;

  //
  // Initiailze JumpStmt so that if no statement in stack, it is NULL
  // pointer. It's legal for JumpStmt to be NULL.
  //
  JumpStmt = NULL;
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );

  if (!IsListEmpty (&Script->StmtStack.StmtList)) {
    StmtLink = Script->StmtStack.StmtList.Flink;
    JumpStmt = CR (
                 StmtLink, 
                 EFI_BATCH_STATEMENT, 
                 Link, 
                 EFI_BATCH_STMT_SIGNATURE
                 );
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvTryMoveUpJumpStmt (
  IN  EFI_BATCH_STMT_TYPE            StmtType,
  OUT BOOLEAN                        *Success
  ) 
/*++

Routine Description:

  Try to move the JumpStmt up along the current statement stack. The JumpStmt
  will be moved up only if the file position matches the upper node in the
  stack.

Arguments:
  Success          If JumpStmt is moved up successfully

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  EFI_LIST_ENTRY                     *StmtLink;
  EFI_BATCH_STATEMENT                *TopStmt;
  EFI_BATCH_STATEMENT                *UpperStmt;

  *Success = FALSE;
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );

  //
  // If JumpStmt == NULL, we're not in any statement, so check if we're 
  // entering statement which is at the stack bottom
  //
  if (JumpStmt == NULL) {
    if (!IsListEmpty (&Script->StmtStack.StmtList)) {
      UpperStmt = CR (
                    Script->StmtStack.StmtList.Blink, 
                    EFI_BATCH_STATEMENT,
                    Link, 
                    EFI_BATCH_STMT_SIGNATURE
                    );
                    
      if (UpperStmt->BeginFilePos == Script->FilePosition) {
        JumpStmt = UpperStmt;
        if (StmtType == StmtIf) {
          JumpStmt->StmtInfo.IfInfo.FoundElse = FALSE;
        }
        *Success = TRUE;
      }
    }
    return EFI_SUCCESS;
  }
  
  if (!IsListEmpty (&Script->StmtStack.StmtList)) {  
    //
    // First check if JumpStmt already points to the top element in stack. If
    // so, we fail to move up. Otherwise check file position to if we are
    // entering upper statement.
    //
    StmtLink = Script->StmtStack.StmtList.Flink;
    TopStmt = CR (StmtLink, EFI_BATCH_STATEMENT, Link, 
      EFI_BATCH_STMT_SIGNATURE);
    if (JumpStmt != TopStmt) {
      UpperStmt = CR (
                    &JumpStmt->Link.Blink->Flink, 
                    EFI_BATCH_STATEMENT, 
                    Link,
                    EFI_BATCH_STMT_SIGNATURE
                    );
                    
      if (UpperStmt->BeginFilePos == Script->FilePosition) {
        
        //
        // File position matches, so move up JumpStmt and set Success to TRUE.
        //
        JumpStmt = UpperStmt;
        if (StmtType == StmtIf) {
          JumpStmt->StmtInfo.IfInfo.FoundElse = FALSE;
        }
        *Success = TRUE;
      }
    }
  }  
  return EFI_SUCCESS; 
}
  
EFI_STATUS
SEnvMoveDownJumpStmt (
  IN  EFI_BATCH_STMT_TYPE            StmtType
  ) 
/*++

Routine Description:

  Move the JumpStmt down along the current statement stack. The JumpStmt will
  be moved down only if the file position matches the lower node in the stack.
  JumpStmt may be NULL after this function, which means goes out of all 
  nesting statements.

Arguments:
  StmtType         Current statement type, used to do if/endif or for/endfor 
                   matching.

Returns:
  EFI_SUCCESS      The function finished sucessfully
  EIF_NOT_FOUND    if/endif or for/endfor mismatch

--*/
{
  EFI_BATCH_SCRIPT                          *Script;
  EFI_LIST_ENTRY                            *StmtLink;
  EFI_BATCH_STATEMENT                       *BottomStmt;

  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );

  if (JumpStmt == NULL || IsListEmpty (&Script->StmtStack.StmtList)) {
    return EFI_NOT_FOUND;
  }
  
  if (JumpStmt->StmtType != StmtType) {
    return EFI_NOT_FOUND;
  }

  StmtLink = Script->StmtStack.StmtList.Blink;
  BottomStmt = CR (
                 StmtLink, 
                 EFI_BATCH_STATEMENT, 
                 Link, 
                 EFI_BATCH_STMT_SIGNATURE
                 );
  //
  // If already stack bottom, set JumpStmt = NULL
  //
  if (BottomStmt == JumpStmt) {
    JumpStmt = NULL;
  
  } else {
    JumpStmt = CR (
                 &JumpStmt->Link.Flink->Flink, 
                 EFI_BATCH_STATEMENT, 
                 Link,
                 EFI_BATCH_STMT_SIGNATURE
                 );
  }
  return EFI_SUCCESS; 
}

EFI_STATUS
SEnvBatchFindVariable (
  IN  CHAR16                         *VariableName,
  OUT CHAR16                         **Value
  )
/*++

Routine Description:

  Find the current value of given variable.

Arguments:
  VariableName     Name of the variable
  Value            Current value of the given variable

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_LIST_ENTRY                     *StmtLink;
  EFI_LIST_ENTRY                     *VariableLink;
  EFI_BATCH_STATEMENT                *Stmt;
  EFI_BATCH_VAR_VALUE                *VariableVal;
  EFI_BATCH_SCRIPT                   *Script;
  
  *Value = NULL;
  if (VariableName == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (VariableName[0] != '%' || !IsAlpha (VariableName[1])) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );
 
  if (IsListEmpty (&Script->StmtStack.StmtList)) {
    return EFI_NOT_FOUND;
  }
  
  //
  // Go down through the statement stack, stop searching after first matching
  // is found
  //
  for (StmtLink = Script->StmtStack.StmtList.Flink; 
       StmtLink != &Script->StmtStack.StmtList; 
       StmtLink=StmtLink->Flink
    ) {
    
    Stmt = CR (StmtLink, EFI_BATCH_STATEMENT, Link, EFI_BATCH_STMT_SIGNATURE);

    if (Stmt->StmtType != StmtFor) {
      continue;
    }
      
    if (VariableName[1] == Stmt->StmtInfo.ForInfo.VariableName[0]) {
      //
      // Found a match
      //
      if (IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
        return EFI_NOT_FOUND;
      }
      VariableLink = Stmt->StmtInfo.ForInfo.ValueList.Flink;
      VariableVal = CR (
                      VariableLink, 
                      EFI_BATCH_VAR_VALUE, 
                      Link, 
                      EFI_BATCH_VAR_SIGNATURE
                      );
                      
      if (VariableVal->Value) {
        *Value = VariableVal->Value;
        return EFI_SUCCESS;
      
      } else {
        return EFI_ABORTED;
      }
    }
  }
  return EFI_NOT_FOUND;
}

EFI_STATUS
SEnvBatchPopStmtStack (
  IN  UINTN                          PopCount,
  IN  BOOLEAN                        PopExtraStack
  )
/*++

Routine Description:

  Pop one or more node out of the statement stack
  
Arguments:
  PopCount         Count of nodes to be poped out
  PopExtraStack    Pop from extra statement stack or current statement stack

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  UINTN                              Index;
  EFI_BATCH_VAR_VALUE                *Variable;
  EFI_LIST_ENTRY                     *StmtLink;
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_STATEMENT                *Stmt;
  
  Script = NULL;
  if (!PopExtraStack) {
    //
    // Popping from the statement stack of current script
    //
    if (IsListEmpty (&ScriptStack.ScriptList)) {
      return EFI_ABORTED;
    }
    
    Script = CR (
      ScriptStack.ScriptList.Flink, 
      EFI_BATCH_SCRIPT, 
      Link,
      EFI_BATCH_SCRIPT_SIGNATURE
      );
  
    if (Script->StmtStack.NestLevel < PopCount) {
      return EFI_INVALID_PARAMETER;
    }
  
  } else {
    //
    // Popping from the extra statement stack
    //
    if (ExtraStmtStack.NestLevel < PopCount) {
      return EFI_INVALID_PARAMETER;
    }
  }

  for (Index=0; Index<PopCount; Index ++) {
    if (PopExtraStack) {
      if (IsListEmpty (&ExtraStmtStack.StmtList)) {
        return EFI_ABORTED;
      }
      StmtLink = ExtraStmtStack.StmtList.Flink;
      ExtraStmtStack.NestLevel --;
    
    } else {
  
      if (IsListEmpty (&Script->StmtStack.StmtList)) {
        return EFI_ABORTED;
      }
      StmtLink = Script->StmtStack.StmtList.Flink;
      Script->StmtStack.NestLevel --;
    }
    
    Stmt = CR (StmtLink, EFI_BATCH_STATEMENT, Link, EFI_BATCH_STMT_SIGNATURE);
    
    //
    // If a for statement, free all its variable values
    //
    if (Stmt->StmtType == StmtFor) {
      while (!IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
        Variable = CR (
                     Stmt->StmtInfo.ForInfo.ValueList.Flink, 
                     EFI_BATCH_VAR_VALUE, 
                     Link, 
                     EFI_BATCH_VAR_SIGNATURE
                     );
                     
        FreePool (Variable->Value);
        RemoveEntryList (&Variable->Link);
        FreePool (Variable);
      }
    }
    
    RemoveEntryList (&Stmt->Link);
    FreePool (Stmt);
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvBatchPushStmtStack (
  IN  EFI_BATCH_STMT_TYPE            StmtType,
  IN  BOOLEAN                        PushExtraStack
  )
/*++

Routine Description:

  Push a node into statement stack.

Arguments:
  StmtType         Statement type to be pushed
  PushExtraStack   Push into extra statement stack or current statement stack

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_BATCH_STATEMENT                *Stmt;
  EFI_LIST_ENTRY                     FileList;
  EFI_LIST_ENTRY                     *Link;
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_VAR_VALUE                *VarValue;
  SHELL_FILE_ARG                     *Arg;
  UINTN                              Index;
  EFI_STATUS                         Status;
  BOOLEAN                            NeedExpand;
  BOOLEAN                            Expanded;
  UINTN                              CharIndex;
  CHAR16                             Char;
  
  Status   = EFI_SUCCESS;
  VarValue = NULL;
  
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );

  Stmt = AllocateZeroPool (sizeof (EFI_BATCH_STATEMENT));
  if (Stmt == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Stmt->Signature    = EFI_BATCH_STMT_SIGNATURE;
  Stmt->StmtType     = StmtType;
  Stmt->BeginFilePos = Script->FilePosition;
  InitializeListHead (&Stmt->StmtInfo.ForInfo.ValueList);

  //
  // If we're pushing if statement to extra statement stack, set condition 
  // to TRUE, for in case we need to jump into this statement later, the 
  // condition should be treated as TRUE.
  //
  if (Stmt->StmtType == StmtIf && PushExtraStack) {
    Stmt->StmtInfo.IfInfo.Condition = TRUE;
  }

  if (Stmt->StmtType == StmtFor) {
    Stmt->StmtInfo.ForInfo.BeginLineNum = SEnvGetLineNumber();
  }

  //
  // if we're pushing FOR statement to the current statement stack, expand the
  // command line argument to a list of variable values.
  //
  if (Stmt->StmtType == StmtFor && 
      !PushExtraStack && 
      SEnvBatchGetCondition ()
      ) {
    
    Stmt->StmtInfo.ForInfo.VariableName[0] = SI->Argv[1][0];
    
    for (Index = 3; Index < SI->Argc; Index ++) {
      //
      // For each argument in the "set", if the length of it is between 0 and 
      // 255, and there exists any wildcard, use ShellFileMetaArg() to expand
      // to a list of file names.
      //
      NeedExpand = FALSE;
      if (SI->Argv[Index][0] != 0 && 
        StrLen (SI->Argv[Index]) < MAX_ARG_LENGTH) {
        CharIndex = 0;
        while (SI->Argv[Index][CharIndex] != 0) {
          Char = SI->Argv[Index][CharIndex];
          if (Char == '*' || Char == '?' || Char == '[' || Char == ']') {
            NeedExpand = TRUE;
            break;
          }
          CharIndex ++;
        }
      }
      
      //
      //  Expand any wildcard filename arguments
      //
      Expanded = FALSE;
      if (NeedExpand) {
        InitializeListHead (&FileList);
        Status = ShellFileMetaArg (SI->Argv[Index], &FileList);
        if (EFI_ERROR (Status)) {
          goto Done;
        }
        //
        //  Build the list of index values from the file list
        //  This will contain either the unexpanded argument or
        //  all the filenames matching an argument with wildcards
        //
        for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
          Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
          if (Arg->Status != EFI_SUCCESS) {
            continue;
          }
          
          Expanded = TRUE;
          VarValue = AllocateZeroPool (sizeof(EFI_BATCH_VAR_VALUE));
          if (VarValue == NULL) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
          }
          VarValue->Signature = EFI_BATCH_VAR_SIGNATURE;
        
          VarValue->Value = AllocateZeroPool (
            StrSize (Arg->FullName) + sizeof(CHAR16));
          if (VarValue->Value == NULL) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
          }
          StrCpy (VarValue->Value, Arg->FullName);
        
          InsertTailList (&Stmt->StmtInfo.ForInfo.ValueList, &VarValue->Link);
        }
        //
        //  Free the file list that was allocated by ShellFileMetaArg
        //
        ShellFreeFileList (&FileList);
      }
      
      //
      // If the argument need not to be expanded, or cannot be expanded 
      // successfully, the original string is added to the value list.
      //
      if (!NeedExpand || !Expanded) {
        VarValue = AllocateZeroPool (sizeof(EFI_BATCH_VAR_VALUE));
        if (VarValue == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          goto Done;
        }
        VarValue->Signature = EFI_BATCH_VAR_SIGNATURE;
        
        VarValue->Value = AllocateZeroPool (
          StrSize (SI->Argv[Index]) + sizeof(CHAR16));
        if (VarValue->Value == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          goto Done;
        }
        StrCpy (VarValue->Value, SI->Argv[Index]);
     
        InsertTailList (&Stmt->StmtInfo.ForInfo.ValueList, &VarValue->Link);
      }
    }
  }
  
  //
  // Push the node to stack and increment the nest level
  //
  if (PushExtraStack) {
    InsertHeadList (&ExtraStmtStack.StmtList, &Stmt->Link);
    ExtraStmtStack.NestLevel ++;
  
  } else {
    InsertHeadList (&Script->StmtStack.StmtList, &Stmt->Link);
    Script->StmtStack.NestLevel ++;
  }

Done:
  if (EFI_ERROR (Status)) {
    if (VarValue != NULL) {
      if (VarValue->Value != NULL) {
        FreePool (VarValue->Value);
      }
      FreePool (VarValue);
    }
    
    if (Stmt->StmtType == StmtFor) {
      while (!IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
        Link = Stmt->StmtInfo.ForInfo.ValueList.Flink;
        
        VarValue = CR (
                     Link, 
                     EFI_BATCH_VAR_VALUE, 
                     Link, 
                     EFI_BATCH_VAR_SIGNATURE
                     );
                     
        if (VarValue->Value != NULL) {
          FreePool (VarValue->Value);
        }
        
        RemoveEntryList (&VarValue->Link);
        FreePool (VarValue);
      }
    }
    
    ShellFreeFileList (&FileList);
    FreePool (Stmt);
  }
    
  return Status;
}

BOOLEAN
SEnvBatchExtraStackEmpty (
  VOID
  )
/*++

Routine Description:

  Test if the extra statement stack if empty

Arguments:

Returns:
  TRUE             The extra statement stack is empty
  FALSE            The extra statement stack is not empty

--*/
{
  //
  // Check if the extra statement stack is empty
  //
  return (BOOLEAN)IsListEmpty (&ExtraStmtStack.StmtList);
}

EFI_STATUS
SEnvPopUntilJumpStmt (
  VOID
  )
/*++

Routine Description:

  Pop from the current statement stack until the element JumpStmt points to

Arguments:

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_LIST_ENTRY                     *StmtLink;
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_STATEMENT                *Stmt;
  EFI_STATUS                         Status;
  
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );

  //
  // If JumpStmt == NULL, we are jumping out of all the nesting statements,
  // so pop out all the statements.
  //
  if (JumpStmt == NULL) {
    Status = SEnvBatchPopStmtStack (Script->StmtStack.NestLevel, FALSE);
    return Status;
  }

  while (!IsListEmpty (&Script->StmtStack.StmtList)) {
    StmtLink = Script->StmtStack.StmtList.Flink;
    Stmt = CR (StmtLink, EFI_BATCH_STATEMENT, Link, EFI_BATCH_STMT_SIGNATURE);

    if (Stmt == JumpStmt) {
      return EFI_SUCCESS;
    }

    Status = SEnvBatchPopStmtStack (1, FALSE);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  
  return EFI_NOT_FOUND;
}

EFI_STATUS
SEnvBatchConnectStmtStack (
  VOID
  )
/*++

Routine Description:

  Move the nodes in extra statement stack to the current statement stack

Arguments:

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_LIST_ENTRY                     *StmtLink;
  EFI_BATCH_STATEMENT                *Stmt;
  EFI_BATCH_SCRIPT                   *Script;
  
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );
  
  //
  // For each element in extra statement stack, check the statement type first,
  // then remove it from extra statement stack and push it to the statement
  // stack of the current script.
  //
  while (!IsListEmpty (&ExtraStmtStack.StmtList)) {
    StmtLink = ExtraStmtStack.StmtList.Blink;
    Stmt = CR (StmtLink, EFI_BATCH_STATEMENT, Link, EFI_BATCH_STMT_SIGNATURE);
    if (Stmt->StmtType != StmtIf) {
      return EFI_UNSUPPORTED;
    }
    
    ExtraStmtStack.NestLevel --;
    RemoveEntryList (StmtLink);
    Script->StmtStack.NestLevel ++;
    InsertHeadList (&Script->StmtStack.StmtList, StmtLink);
  }
  
  return EFI_SUCCESS;    
}

EFI_STATUS
BatchPushScriptStack (
  IN  ENV_SHELL_INTERFACE      *Shell
  )
/*++

Routine Description:

  Push a node into the script stack.

Arguments:
  Shell            The shell envrionment information of the script

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_ARGUMENT                 *ArgEntry;
  UINTN                              Index;
  EFI_STATUS                         Status;
  
  ArgEntry = NULL;
  Status   = EFI_SUCCESS;
  
  Script = AllocateZeroPool (sizeof (EFI_BATCH_SCRIPT));
  if (Script == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Script->Signature = EFI_BATCH_SCRIPT_SIGNATURE;
  Script->FilePosition = (UINT64)0x00;
  Script->BatchAbort = FALSE;
  Script->LineNumber = 0;

  InitializeListHead (&Script->StmtStack.StmtList);
  Script->StmtStack.NestLevel = 0;
  
  InitializeListHead (&Script->ArgListHead);
  
  //
  // Store the command line argument in to the list in script stack
  //
  for (Index = 0; Index < Shell->ShellInt.Argc; Index ++) {
    //
    //  Allocate the new element of the argument list
    //
    ArgEntry = AllocateZeroPool (sizeof(EFI_BATCH_ARGUMENT));
    if (ArgEntry == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    //
    // Allocate space for the argument string in the arglist element
    //
    ArgEntry->ArgValue = AllocateZeroPool (
                           StrSize (Shell->ShellInt.Argv[Index])
                           );
                           
    if (ArgEntry->ArgValue == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    //
    // Copy in the argument string
    //
    StrCpy (ArgEntry->ArgValue, Shell->ShellInt.Argv[Index]);
    ArgEntry->Signature = EFI_BATCH_ARG_SIGNATURE;

    //
    // Add the arglist element to the end of the list
    //
    InsertTailList (&Script->ArgListHead, &ArgEntry->Link);
  }

  //
  // Push the node into stack, and increment the nest level
  //
  InsertHeadList (&ScriptStack.ScriptList, &Script->Link);
  ScriptStack.NestLevel ++;
  
Done:
  if (EFI_ERROR (Status)) {
    if (ArgEntry != NULL) {
      if (ArgEntry->ArgValue != NULL) {
        FreePool (ArgEntry->ArgValue);
      }
      FreePool (ArgEntry);
    }
    while (!IsListEmpty (&Script->ArgListHead)) {
      ArgEntry = CR (
                   Script->ArgListHead.Flink, 
                   EFI_BATCH_ARGUMENT, 
                   Link,
                   EFI_BATCH_ARG_SIGNATURE
                   );
      
      if (ArgEntry->ArgValue != NULL) {
        FreePool (ArgEntry->ArgValue);
      }
      
      RemoveEntryList (&ArgEntry->Link);
      FreePool (ArgEntry);
    }
    
    FreePool (Script);
  }
  
  return Status;
}

EFI_STATUS
BatchPopScriptStack (
  IN  UINTN                          PopCount
  )
/*++

Routine Description:

  Pop one or more nodes out of the script stack.

Arguments:
  PopCount         Count of nodes need to pop out

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  UINTN                              Index;
  EFI_LIST_ENTRY                     *ScriptLink;
  EFI_BATCH_ARGUMENT                 *Argument;
  EFI_BATCH_SCRIPT                   *Script;
  EFI_STATUS                         Status;
  
  Status = EFI_SUCCESS;
  
  if (ScriptStack.NestLevel < PopCount) {
    return EFI_INVALID_PARAMETER;
  }
  
  for (Index=0; Index<PopCount; Index ++) {
    ScriptLink = ScriptStack.ScriptList.Flink;
    Script = CR (
               ScriptLink, 
               EFI_BATCH_SCRIPT, 
               Link, 
               EFI_BATCH_SCRIPT_SIGNATURE
               );
    
    //
    // Free the argument list
    //
    while (!IsListEmpty (&Script->ArgListHead)) {
      Argument = CR (
                   Script->ArgListHead.Flink, 
                   EFI_BATCH_ARGUMENT, 
                   Link, 
                   EFI_BATCH_ARG_SIGNATURE 
                   );
                   
      if (Argument->ArgValue != NULL) {
        FreePool (Argument->ArgValue);
      }
      
      RemoveEntryList (&Argument->Link);
      FreePool (Argument);
    }
    
    //
    // Free the statement stack in this script
    //
    Status = SEnvBatchPopStmtStack (Script->StmtStack.NestLevel, FALSE);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    
    //
    // Remove & free the node itself
    //
    RemoveEntryList (&Script->Link);
    FreePool (Script);
    ScriptStack.NestLevel --;
  }
  
  return Status;
}

EFI_STATUS
SEnvBatchFindArgument (
  IN  UINTN                          ArgumentNum,
  OUT CHAR16                         **Value
  )
/*++

Routine Description:

  Find the value of the specified argument

Arguments:
  ArgumentNum      The number of argument to be found
  Value            Value of the specified argument, may be an empty string

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_LIST_ENTRY                     *ArgLink;
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_ARGUMENT                 *Argument;
  UINTN                              Index;

  if (Value == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Index = 0;
  //
  // Initialize the argument value, so that if no found, an emtry string
  // will be returned as its value.
  //
  *Value = L"";
  if (!IsListEmpty (&ScriptStack.ScriptList)) {
    Script = CR (
               ScriptStack.ScriptList.Flink, 
               EFI_BATCH_SCRIPT, 
               Link, 
               EFI_BATCH_SCRIPT_SIGNATURE
               );
    
    //
    // Go through the argument list, until the N'th argument we're looking for
    // is met.
    //
    for (ArgLink = Script->ArgListHead.Flink; 
         ArgLink != &Script->ArgListHead;
         ArgLink = ArgLink->Flink
      ) {
      
      Argument = CR (
                   ArgLink, 
                   EFI_BATCH_ARGUMENT, 
                   Link, 
                   EFI_BATCH_ARG_SIGNATURE
                   );
      
      if (Index == ArgumentNum) {
        *Value = Argument->ArgValue;
        return EFI_SUCCESS;
      
      } else {
        Index ++;
      }
    }  
  }  
        
  return EFI_NOT_FOUND;
}      

EFI_BATCH_STATEMENT *
SEnvGetJumpStmt (
  VOID
  )
{
  //
  // return value of global variable JumpStmt
  //
  return JumpStmt;
}

EFI_BATCH_STATEMENT *
SEnvBatchExtraStackTop (
  VOID
  )
/*++

Routine Description:

  Get the top element of the extra statement stack

Arguments:

Returns:
  Not NULL         The top element of the extra statement stack
  NULL             The function is not finished successfully    
--*/
{
  EFI_BATCH_STATEMENT                *Stmt;
  
  Stmt = CR (
           ExtraStmtStack.StmtList.Flink, 
           EFI_BATCH_STATEMENT, 
           Link,
           EFI_BATCH_STMT_SIGNATURE
           );
  
  return Stmt;  
}

EFI_BATCH_STATEMENT *
SEnvBatchStmtStackTop (
  VOID
  )
/*++

Routine Description:

  Get the top element of the current statement stack

Arguments:

Returns:
  Not NULL         The top element of the current statement stack
  NULL             The function is not finished successfully    
--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_STATEMENT                *Stmt;
  
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return NULL;
  }

  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link, 
             EFI_BATCH_SCRIPT_SIGNATURE
             );
  
  if (IsListEmpty (&Script->StmtStack.StmtList)) {
    return NULL;
  }

  Stmt = CR (
           Script->StmtStack.StmtList.Flink, 
           EFI_BATCH_STATEMENT, 
           Link,
           EFI_BATCH_STMT_SIGNATURE
           );
  
  return Stmt;
}

EFI_STATUS
SEnvBatchSetFilePosition ( 
  IN  UINT64                         NewPos
  )
/*++

Routine Description:

  Set the file position of the current script

Arguments:
  NewPos           New file position

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_BATCH_SCRIPT                   *Script;

  //
  // Find the top element in script stack, and set its FilePosition field
  //
  if (SEnvBatchIsActive ()) {
    Script = CR (
               ScriptStack.ScriptList.Flink, 
               EFI_BATCH_SCRIPT, 
               Link, 
               EFI_BATCH_SCRIPT_SIGNATURE
               );

    Script->FilePosition = NewPos;
    return EFI_SUCCESS; 
  }
  
  return EFI_NOT_FOUND;
}

EFI_STATUS
BatchGetFilePosition ( 
  OUT UINT64                         *FilePos
  )
/*++

Routine Description:

  Get the file position of current script

Arguments:
  FilePos          Output the file position of current script

Returns:
  EFI_SUCCESS      The function finished sucessfully

--*/
{
  EFI_BATCH_SCRIPT                   *Script;

  if (!FilePos) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Find the top element in script stack and return its FilePositon field
  //
  if (SEnvBatchIsActive ()) {
    Script = CR (
               ScriptStack.ScriptList.Flink, 
               EFI_BATCH_SCRIPT, 
               Link, 
               EFI_BATCH_SCRIPT_SIGNATURE
               );

    *FilePos = Script->FilePosition;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

VOID
SEnvInitBatch (
  VOID 
  )
/*++

Routine Description:

  Initializes global variables used for batch file processing.

Arguments:

Returns:

--*/
{
  //
  // Initialize global variables and the script stack.
  //
  LastError         = EFI_SUCCESS;
  ZeroMem (LastErrorBuf, LASTERROR_BUFSIZ);
  GotoIsActive      = FALSE;
  GotoFilePos       = (UINT64)0x00;
  EchoIsOn          = TRUE;
  OrigConIn         = ST->ConIn;
  OrigConOut        = ST->ConOut;
  
  ScriptStack.NestLevel = 0;
  InitializeListHead (&ScriptStack.ScriptList);
  
  ExtraStmtStack.NestLevel = 0;
  InitializeListHead (&ExtraStmtStack.StmtList);
}

BOOLEAN
SEnvBatchIsActive ( 
  VOID
  )
/*++

Routine Description:

  Returns whether any batch files are currently being processed.

Arguments:

Returns:

--*/
{
  //
  // The first task to execute a script is to push the current script into
  // the stack, so the empty or not of the script stack decides if batch is
  // active.
  // 
  return (BOOLEAN)(!IsListEmpty (&ScriptStack.ScriptList));
}


VOID
SEnvSetBatchAbort ( 
  VOID
  )
/*++

Routine Description:

  Sets a flag to notify the current batch script to exit.

Arguments:

Returns:

--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    Print (L"shell: Cannot execute script - %r\n", EFI_ABORTED);
    return;
  }
  
  //
  // Find the top element of script and set its BatchAbort field. This notifies
  // the current script to abort, but its caller will continue
  //
  Script = CR (ScriptStack.ScriptList.Flink, EFI_BATCH_SCRIPT, Link,
    EFI_BATCH_SCRIPT_SIGNATURE);
  
  Script->BatchAbort = TRUE;
  return;
}

VOID
SEnvSetAllBatchAbort ( 
  VOID
  )
/*++

Routine Description:

  Sets a flag to notify all the batch script to exit.

Arguments:

Returns:

--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  EFI_LIST_ENTRY                     *Link;
  
  if (!IsListEmpty (&ScriptStack.ScriptList)) {
    //
    // Go down through the script and set each element's BatchAbort field. This
    // notifies all nested scripts to abort.
    //
    for (Link = ScriptStack.ScriptList.Flink; 
         Link != &ScriptStack.ScriptList;
         Link = Link->Flink
      ) {
      Script = CR (Link, EFI_BATCH_SCRIPT, Link, EFI_BATCH_SCRIPT_SIGNATURE);
      Script->BatchAbort = TRUE;
    }
  }
  
  return;
}

BOOLEAN
SEnvGetBatchAbort ( 
  VOID
  )
/*++

Routine Description:

  Gets the flag which notifying the current batch script to exit.

Arguments:

Returns:

--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    Print (L"shell: Cannot execute script - %r\n", EFI_ABORTED);
    return FALSE;
  }
  
  //
  // Find the top element in script stack (current script) and return
  // its BatchAbort flag.
  //
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );
  
  return Script->BatchAbort;
}

VOID
SEnvBatchGetConsole ( 
  OUT EFI_SIMPLE_TEXT_IN_PROTOCOL    **ConIn,
  OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL   **ConOut
  )
/*++

Routine Description:

  Returns the Console I/O interface pointers.

Arguments:
  ConIn       The console input protocol
  ConOut      The console output protocol
  
Returns:

--*/
{
  //
  // return value of the 2 global variable
  //
  *ConIn  = OrigConIn;
  *ConOut = OrigConOut;
  return;
}


EFI_STATUS
SEnvBatchEchoCommand ( 
  IN  ENV_SHELL_INTERFACE            *Shell
  )
/*++

Routine Description:

  Echoes the given command to stdout.

Arguments:

Returns:
  EFI_SUCCESS      The function finished successfully
  
--*/
{
  UINTN                              Index;
  CHAR16                             *BatchFileName;
  EFI_STATUS                         Status;

  Status = EFI_SUCCESS;
  
  if (SEnvBatchIsActive () && EchoIsOn) {
    Print (L"%E");

    BatchFileName = NULL;
    Status = SEnvBatchFindArgument (0, &BatchFileName);
    if (EFI_ERROR (Status)) {
      Print (L"\n");
      return EFI_ABORTED;
    }
    
    //
    // Print "+"s according the nest level of current script
    //
    for (Index = 0; Index < ScriptStack.NestLevel - 1; Index ++) {
      Print (L"+");
    }
    
    Print (L"%s> ", BatchFileName);
    for (Index = 0; Index < Shell->ShellInt.Argc; Index ++) {
      Print (L"%s ", Shell->ShellInt.Argv[Index]);
    }
    
    for (Index = 0; Index < Shell->ShellInt.RedirArgc; Index ++) {
      Print (L"%s ", Shell->ShellInt.RedirArgv[Index]);
    }
    
    Print (L"\n");
    //
    // Switch output attribute back to normal, restore ST->ConOut
    //
    Print (L"%N");
  }
 
  return Status;
}


VOID
SEnvBatchSetEcho ( 
  IN  BOOLEAN                        Val
  )
/*++

Routine Description:

  Sets the echo flag to the specified value.

Arguments:
  Val              New value of "EchoIsOn"

Returns:

--*/
{
  //
  // Set value to global variable EchoIsOn
  //
  EchoIsOn = Val;
  return;
}


BOOLEAN
SEnvBatchGetEcho ( 
  VOID
  )
/*++

Routine Description:

  Returns the echo flag.

Arguments:

Returns:
  The value of "EchoIsOn"
  
--*/
{
  //
  // Get value of global variable EchoIsOn
  //
  return EchoIsOn;
}

EFI_STATUS
SEnvBatchSetCondition ( 
  IN  BOOLEAN                        Val
  )
/*++

Routine Description:

  Sets the condition flag of the current IF statement according to the 
  test result of IF conditon.

Arguments:
  Val              Test result of the IF condition of current IF statement.

Returns:
  EFI_SUCCESS      The function finished successfully

--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_STATEMENT                *Stmt;
  EFI_LIST_ENTRY                     *Link;
  EFI_BATCH_STATEMENT                *LowerStmt;
  BOOLEAN                            LowerCondition;

  if (!SEnvBatchIsActive ()) {
    return EFI_ABORTED;
  }
  
  //
  // Find the current statement in the current script
  //
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link, 
             EFI_BATCH_SCRIPT_SIGNATURE
             );
  
  if (IsListEmpty (&Script->StmtStack.StmtList)) {
    return EFI_ABORTED;
  }

  Link = Script->StmtStack.StmtList.Flink;
  Stmt = CR (Link, EFI_BATCH_STATEMENT, Link, EFI_BATCH_STMT_SIGNATURE);
  if (Stmt->StmtType != StmtIf) {
    return EFI_ABORTED;
  }
  
  //
  // Get the Condition flag of the inner most IF statment that wraps the 
  // current IF. If the condition of the wrapping IF statement is false, 
  // even if Val is TRUE, set the conditon flag to false, because in this 
  // situation, we should not execute commands inside this IF.
  //
  LowerCondition = TRUE;
  Link = Link->Flink;
  while (Link != &Script->StmtStack.StmtList) { 
    LowerStmt = CR (Link, EFI_BATCH_STATEMENT, Link, EFI_BATCH_STMT_SIGNATURE);
    if (LowerStmt->StmtType == StmtIf) {
      LowerCondition = LowerStmt->StmtInfo.IfInfo.Condition;
      break;
    }
    Link = Link->Flink;
  }
  
  Stmt->StmtInfo.IfInfo.Condition = (BOOLEAN)(LowerCondition && Val);    
  return EFI_SUCCESS;
}

BOOLEAN
SEnvBatchGetCondition ( 
  VOID
  )
/*++

Routine Description:

  Get the condition flag of the current statement stack.

Arguments:

Returns:
  Value of the condition flag
  
--*/
{
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_STATEMENT                *Stmt;
  EFI_LIST_ENTRY                     *StmtLink;

  if (SEnvBatchIsActive ()) {
    Script = CR (
               ScriptStack.ScriptList.Flink, 
               EFI_BATCH_SCRIPT, 
               Link, 
               EFI_BATCH_SCRIPT_SIGNATURE
               );
    
    //
    // If we are not in any statement, we should execute all commands we
    // meet, so treat as if condition is TRUE
    //
    if (IsListEmpty (&Script->StmtStack.StmtList)) {
      return TRUE;
    }

    //
    // Go down along the statement stack and looks for the 1st if statement.
    // The condition flag of this if statement is returned.
    //
    for (StmtLink = Script->StmtStack.StmtList.Flink; 
      StmtLink != &Script->StmtStack.StmtList; StmtLink = StmtLink->Flink) {
      Stmt = CR (
               StmtLink, 
               EFI_BATCH_STATEMENT, 
               Link, 
               EFI_BATCH_STMT_SIGNATURE
               );
    
      if (Stmt->StmtType != StmtIf) {
        continue;
      }
      return Stmt->StmtInfo.IfInfo.Condition;
    }
    return TRUE;
  }
  
  return FALSE;
}

UINTN
SEnvGetLineNumber ( 
  VOID
  )
/*++

Routine Description:

  Get the current line number of the current script

Arguments:

Returns:
  Current line number of the current script
  
--*/
{
  EFI_BATCH_SCRIPT                   *Script;

  if (IsListEmpty (&ScriptStack.ScriptList)) {
    Print (L"Internal error\n");
    return 0;
  }
  
  //
  // Find the top (current) script in stack and return its LineNumber
  // 
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link, 
             EFI_BATCH_SCRIPT_SIGNATURE
             );
      
  return Script->LineNumber;
}

VOID
SEnvSetLineNumber ( 
  UINTN                              LineNumber
  )
/*++

Routine Description:

  Get the current line number of the current script

Arguments:

Returns:
  Current line number of the current script
  
--*/
{
  EFI_BATCH_SCRIPT                   *Script;

  if (IsListEmpty (&ScriptStack.ScriptList)) {
    Print (L"Internal error\n");
    return;
  }
  
  //
  // Find the top (current) script in stack and return its LineNumber
  // 
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link, 
             EFI_BATCH_SCRIPT_SIGNATURE
             );
      
  Script->LineNumber = LineNumber;
}


VOID
SEnvBatchSetGotoActive ( 
  VOID 
  )
/*++

Routine Description:

  Sets the goto-is-active to TRUE and saves the current position
  of the active script file.

Arguments:

Returns:

--*/
{
  //
  // Set the global flag GotoIsActive and records the current file position
  //
  GotoIsActive = TRUE;
  BatchGetFilePosition (&GotoFilePos);
  return;
}

BOOLEAN
SEnvBatchGetGotoActive ( 
  VOID
  )
/*++

Routine Description:

  Get the goto-is-active flag.

Arguments:

Returns:
  The value of the goto-is-active flag

--*/
{
  //
  // return the value of global variable GotoIsActive
  //
  return GotoIsActive;
}

BOOLEAN
SEnvBatchVarIsLastError ( 
  IN CHAR16 *Name
  )
/*++

Routine Description:

  Checks to see if name of the variable name is "lasterror".

Arguments:
  Name             Name of the variable

Returns:
  TRUE             Variable name is "lasterror"
  FALSE            Variable name is not "lasterror"

--*/
{
  //
  // Check if the current variable name is "LastError"
  //
  return (BOOLEAN)(StriCmp (L"lasterror", Name) == 0);
}

CHAR16 *
SEnvBatchGetLastError ( 
  VOID 
  )
/*++

Routine Description:

  Returns a pointer to a string representation of the error value 
  returned by the last shell command.

Arguments:

Returns:
  Pointer to the lasterror string
  
--*/
{
  //
  // Convert the error code into string and return the string
  //
  ValueToHex (LastErrorBuf, (UINT64)LastError);
  return LastErrorBuf;
}

VOID
SEnvBatchSetRewind (
  IN  BOOLEAN                        Val
  )
/*++

Routine Description:

  Sets the Rewind flag to indicate if searching for goto target has passed the
  file end or not.

Arguments:
  New value of the Rewind flag

Returns:

--*/
{
  //
  // Set the global Rewind flag to indicate if searching for goto target has
  // passed file end or not.
  //
  Rewind = Val;
  return;
}


BOOLEAN
SEnvBatchGetRewind ( 
  VOID 
  )
/*++

Routine Description:

  Returns the value of the Rewind flag

Arguments:

Returns:
  The value of Rewind flag
--*/
{
  //
  // Get the global Rewind flag which indicates if searching for goto target
  // has passed file end or not.
  // 
  return Rewind;
}

STATIC EFI_STATUS
BatchIsAscii (
  IN  EFI_FILE_HANDLE                File, 
  OUT BOOLEAN                        *IsAscii
  )
/*++

Routine Description:

  Checks to see if the specified batch file is ASCII.

Arguments:
  File             The file handle of current active script
  IsAscii          Whether current active script is an ascii file

Returns:
  EFI_SUCCESS      The function finished successfully

--*/
{
  //
  // UNICODE byte-order-mark is two bytes
  //
  CHAR8                              Buffer8[2];  
  UINTN                              BufSize;
  EFI_STATUS                         Status;

  //
  //  Read the first two bytes to check for byte order mark
  //
  BufSize = sizeof (Buffer8);
  Status = File->Read (File, &BufSize, Buffer8);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = File->SetPosition (File, (UINT64)0);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  //  If we find a UNICODE byte order mark assume it is UNICODE,
  //  otherwise assume it is ASCII.  UNICODE byte order mark on
  //  IA little endian is first byte 0xff and second byte 0xfe
  //
  if ( (Buffer8[0] | (Buffer8[1] << 8)) == EFI_UNICODE_BYTE_ORDER_MARK ) {
    *IsAscii = FALSE;
  
  } else {
    *IsAscii = TRUE;
  }

  return Status;
}


STATIC EFI_STATUS
BatchGetLine (
  IN  EFI_FILE_HANDLE                File, 
  IN  BOOLEAN                        Ascii,
  IN  OUT UINT64                     *FilePosition,
  OUT UINTN                          *CmdLineSize,
  OUT CHAR16                         **CommandLine
  )
/*++

Routine Description:

  Reads the next line from the batch file, converting it from ASCII to 
  UNICODE if necessary.  If end of file is encountered then it returns 
  0 in the CmdLineLen parameter.

Arguments:

  File            The batch file handle which is being executed.
  Ascii           If the file is of Ascii format.
  CmdLineSize     Size of the next command line acturally read.
  CommandLine     Command line buffer which will hold the next line.    

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  EFI_STATUS                         Status;
  CHAR8                              Buffer8[COMMON_CMDLINE_LENGTH];
  CHAR16                             Buffer16[COMMON_CMDLINE_LENGTH];
  UINTN                              BufSize;
  UINTN                              ExtendSize;
  UINTN                              OldBufSize;
  UINTN                              CharSize;
  UINTN                              Index;
  UINTN                              OldIndex;
  BOOLEAN                            LineEnd;
  CHAR16                             *CmdBuf;
  
  //
  //  Check params
  //
  if (CommandLine == NULL || CmdLineSize == NULL || FilePosition == NULL ) {
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Make sure we don't overrun our stack-based array Buffer8
  //
  CharSize = Ascii ? sizeof(CHAR8) : sizeof(CHAR16);
  
  //
  //  Initialize OUT param
  //
  *CommandLine = NULL;
  *CmdLineSize = 0;

  //
  // If beginning of UNICODE file, move past the Byte-Order-Mark (2 bytes)
  //
  if (!Ascii && *FilePosition == (UINT64)0) {
    *FilePosition = (UINT64)2;
  }

  Status = File->SetPosition (File, *FilePosition);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // (1) Read a buffer-full from the file
  // (2) Locate the end of the 1st line in the buffer
  // (3) Convert first CRs or LFs to 0(string end mark)
  // (4) If no CR/LF found, reallocate memory and continue reading and parsing
  
  LineEnd    = FALSE;
  ExtendSize = (COMMON_CMDLINE_LENGTH) * sizeof(CHAR16);
  CmdBuf = *CommandLine = AllocateZeroPool (ExtendSize + sizeof(CHAR16));
  if (CmdBuf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  OldBufSize = ExtendSize + sizeof(CHAR16);
  OldIndex   = 0;
  
  while (!LineEnd) {
    BufSize = COMMON_CMDLINE_LENGTH * CharSize;
    Status = File->Read (File, &BufSize, Ascii ? Buffer8 : (CHAR8 *)Buffer16);
    if (EFI_ERROR (Status)) {
      FreePool (*CommandLine);
      *CommandLine = NULL;
      return Status;
    }
    
    if (BufSize == 0) {
      *CmdLineSize = (CmdBuf - *CommandLine) * sizeof(CHAR16);
      *FilePosition += (UINT64)*CmdLineSize;
      Status = File->SetPosition (File, *FilePosition);
      if (EFI_ERROR (Status)) {
        FreePool (*CommandLine);
        *CommandLine = NULL;
      }
      return Status;
    }

    for (Index = 0; Index < BufSize / CharSize; Index ++) {
      if (Ascii) {
        if (Buffer8[Index] == ASCII_LF || Buffer8[Index] == ASCII_CR) {
          CmdBuf[Index] = 0;
          *CmdLineSize = (OldIndex + Index + 1) * CharSize;

          if (Buffer8[Index+1] == ASCII_LF || Buffer8[Index+1] == ASCII_CR) {
            *CmdLineSize = (OldIndex + Index + 2) * CharSize;
          }
          LineEnd = TRUE;
          break;
        }
        CmdBuf[Index] = Buffer8[Index];
      } else {
        if (Buffer16[Index] == UNICODE_LF || Buffer16[Index] == UNICODE_CR) {
          CmdBuf[Index] = 0;
          *CmdLineSize = (OldIndex + Index + 1) * CharSize;
          
          if (Buffer16[Index+1] == UNICODE_LF || 
            Buffer16[Index+1] == UNICODE_CR) {
            *CmdLineSize = (OldIndex + Index + 2) * CharSize;
          }
          LineEnd = TRUE;
          break;
        }
        CmdBuf[Index] = Buffer16[Index];
      }
    }
     
    if (BufSize < COMMON_CMDLINE_LENGTH * CharSize && !LineEnd) {
      CmdBuf[Index] = 0;
      *CmdLineSize = (OldIndex + Index + 1) * CharSize;
      LineEnd = TRUE;
      break;
    }

    if (!LineEnd) {
      *CommandLine = ReallocatePool (
                       *CommandLine, 
                       OldBufSize, 
                       OldBufSize + ExtendSize
                       );
                       
      if (*CommandLine == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      
      ZeroMem (*CommandLine + OldBufSize / sizeof(CHAR16), ExtendSize);
      CmdBuf = *CommandLine + OldBufSize / sizeof(CHAR16) - 1;
      OldBufSize += ExtendSize;
      OldIndex += COMMON_CMDLINE_LENGTH;
    }
  }

  //
  //  Reset the file position to just after the command line
  //
  *FilePosition += (UINT64)*CmdLineSize;
  Status = File->SetPosition (File, *FilePosition);
  if (EFI_ERROR (Status)) {
    FreePool (*CommandLine);
    *CommandLine = NULL;
  }
  return Status;
}


EFI_STATUS
SEnvExecuteScript (
  IN ENV_SHELL_INTERFACE             *Shell,
  IN EFI_FILE_HANDLE                 File
  )
/*++

Routine Description:

  Execute the commands in the script file specified by the 
  file parameter.

Arguments:
  Shell:  shell interface of the caller
  File:   file handle to open script file

Returns:
  EFI_STATUS

--*/
{
  BOOLEAN                            EndOfFile;
  EFI_STATUS                         Status;
  UINTN                              BufSize;
  CHAR16                             *CommandLine;
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_STATEMENT                *Stmt;
  ENV_SHELL_INTERFACE                NewShell;
  UINTN                              GotoTargetStatus;
  UINTN                              SkippedIfCount;
  BOOLEAN                            EchoStateValid;
  BOOLEAN                            EchoState;
  EFI_STATUS                         ExecuteResult;
  BOOLEAN                            ChangeLastError;
  BOOLEAN                            ScriptPushed;

  //
  //  Initialize
  //
  Status         = EFI_SUCCESS;
  EndOfFile      = FALSE;
  BufSize        = 0;
  EchoStateValid = FALSE;
  EchoState      = FALSE;
  ZeroMem (&NewShell, sizeof(NewShell));
  GotoIsActive   = FALSE;
  ScriptPushed   = FALSE;
  SEnvInitTargetLabel();
  SEnvBatchSetRewind (FALSE);
  CommandLine    = NULL;

  //
  //  Check params
  //
  if (File == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (!SEnvBatchIsActive ()) {
    OldEchoIsOn = EchoIsOn;
  }
  
  //
  // Figure out if the file is ASCII or UNICODE.
  //
  Status = BatchIsAscii (File, &Shell->StdIn.Ascii);
  if (EFI_ERROR (Status)) {
    Print (L"shell: Cannot read from file - %r\n", Status);
    goto Done;
  }

  Status = BatchPushScriptStack (Shell);
  if (EFI_ERROR(Status)) {
    Print (L"shell: Cannot execute script - %r\n", Status);
    SEnvSetAllBatchAbort ();
    goto Done;
  }
  
  ScriptPushed = TRUE;
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );
  
  //
  // Iterate through the file, reading a line at a time and executing each
  // line as a shell command.  Nested shell scripts will come through 
  // this code path recursively.
  //
  EndOfFile = FALSE;
  SkippedIfCount = 0;

  while (1) {
    //
    // Read a command line from the file
    //    
    BufSize = MAX_CMDLINE;
    Script->LineNumber ++;
    
    if (CommandLine != NULL) {
      FreePool (CommandLine);
      CommandLine = NULL;
    }
    
    Status = BatchGetLine (File, Shell->StdIn.Ascii, &(Script->FilePosition),
      &BufSize, &CommandLine);

    if (EFI_ERROR (Status)) {
      Print (L"shell: Cannot read from file - %r\n", Status);
      SEnvSetBatchAbort ();
      goto Done;
    }

    //
    // No error and no chars means EOF
    // If we are in the middle of a GOTO then rewind to search for the
    //   label from the beginning of the file, otherwise we are done
    //   with this script.
    //
    if (BufSize == 0) {
      if (GotoIsActive) {
        SEnvBatchSetRewind (TRUE);
        Script->LineNumber = 0;
        Script->FilePosition = (UINT64)(0x00);
        Status = File->SetPosition (File, Script->FilePosition);
        if (EFI_ERROR (Status)) {
          Print (L"shell: Cannot execute script - %r", Status);
          SEnvSetBatchAbort ();
          goto Done;

        } else {
          continue;
        }

      } else {
        goto Done;
      }

    }

    //
    // Convert the command line to an arg list
    //
    Status = SEnvStringToArg (
               CommandLine, 
               TRUE, 
               &NewShell.ShellInt.Argv,
               &NewShell.ShellInt.Argc
               );
               
    if (EFI_ERROR(Status)) {
      SEnvSetBatchAbort ();
      goto Done;
    }

    //
    // Skip comments and blank lines
    //
    if (NewShell.ShellInt.Argc == 0) {
      SEnvFreeArgument (&NewShell.ShellInt.Argc, &NewShell.ShellInt.Argv);
      continue;
    }

    //
    // If a GOTO command is active, skip everything until we find 
    // the target label or until we determine it doesn't exist.
    //
    if (GotoIsActive) {
      //
      // Check if we have the right label or if we've searched 
      // the whole file
      //
      Status = SEnvCheckForGotoTarget (
                 NewShell.ShellInt.Argv[0],
                 GotoFilePos, 
                 Script->FilePosition, 
                 &GotoTargetStatus
                 );
      if (EFI_ERROR (Status)) {
        Print (L"shell: Cannot execute script - %r", Status);
        SEnvSetBatchAbort ();
        goto Done;
      }
  
      switch (GotoTargetStatus) {
        case GOTO_TARGET_FOUND:
          GotoIsActive = FALSE;
          SEnvBatchSetRewind (FALSE);
          SEnvFreeTargetLabel ();
          
          Status = SEnvPopUntilJumpStmt ();
          if (EFI_ERROR (Status)) {
            Print (L"shell: Cannot execute script - %r", Status);
            SEnvSetBatchAbort ();
            goto Done;
          }

          if (!SEnvBatchExtraStackEmpty ()) {
            Status = SEnvBatchConnectStmtStack ();
            if (EFI_ERROR (Status)) {
              if (Status == EFI_UNSUPPORTED) {
                Print (L"shell: Attempt to jump into a loop\n");
              
              } else {
                Print (L"shell: Cannot execute script - %r", Status);
              }
              SEnvSetBatchAbort ();
              goto Done;
            }
          }
          SEnvFreeArgument (&NewShell.ShellInt.Argc, &NewShell.ShellInt.Argv);
          continue;

        case GOTO_TARGET_NOT_FOUND:
          break;

        case GOTO_TARGET_DOESNT_EXIST:
          GotoIsActive = FALSE;
          Status = EFI_INVALID_PARAMETER;
          LastError = Status;
          SEnvPrintLabelNotFound ();
          SEnvFreeTargetLabel ();
          SEnvSetBatchAbort ();
          goto Done;

        default: {}
      }

    } else if ( NewShell.ShellInt.Argv[0][0] == L':' ) {
      //
      // Skip labels when no GOTO is active
      //
      SEnvFreeArgument (&NewShell.ShellInt.Argc, &NewShell.ShellInt.Argv);
      continue;
    }

    // 
    // Execute the command
    //
    ChangeLastError = FALSE;
    if (StriCmp (NewShell.ShellInt.Argv[0], L"if") != 0 && 
        StriCmp (NewShell.ShellInt.Argv[0], L"for") != 0 &&
        StriCmp (NewShell.ShellInt.Argv[0], L"endif") != 0 &&
        StriCmp (NewShell.ShellInt.Argv[0], L"endfor") != 0 &&
        StriCmp (NewShell.ShellInt.Argv[0], L"else") != 0
      ) {
      
      if (!SEnvBatchGetCondition () || SEnvBatchGetGotoActive ()) {
        SEnvFreeArgument (&NewShell.ShellInt.Argc, &NewShell.ShellInt.Argv);
        continue;
      }

      if (StriCmp (NewShell.ShellInt.Argv[0], L"goto") != 0) {
        ChangeLastError = TRUE;
      }

    } else if (!SEnvBatchGetCondition () || SEnvBatchGetGotoActive ()) {

      EchoStateValid = TRUE;
      EchoState = EchoIsOn;
      SEnvBatchSetEcho (FALSE);
    }

    ExecuteResult = SEnvDoExecute (
                      Shell->ShellInt.ImageHandle, 
                      CommandLine,
                      &NewShell, 
                      TRUE
                      );

    if (ChangeLastError) {
      LastError = ExecuteResult;
    }
    
    if (EchoStateValid) {
      EchoStateValid = FALSE;
      SEnvBatchSetEcho (EchoState);
    }
    
    if (SEnvGetBatchAbort()) {
      goto Done;
    }
  }

Done:
  //
  // Clean up
  //

  //
  // Free any potential remaining GOTO target label
  //
  GotoIsActive = FALSE;
  SEnvFreeTargetLabel();
  SEnvFreeArgument (&NewShell.ShellInt.Argc, &NewShell.ShellInt.Argv);
  if (CommandLine != NULL) {
    FreePool (CommandLine);
    CommandLine = NULL;
  }

  //
  // Close the script file
  //
  if (File) {
    File->Close( File );
  }

  Status = SEnvBatchPopStmtStack (ExtraStmtStack.NestLevel, TRUE);
  if (EFI_ERROR (Status)) {
    Print (L"shell: Cannot execute script - %r", Status);
    SEnvSetBatchAbort ();
  }

  if (ScriptPushed) {
    if (!SEnvGetBatchAbort()) {
      Script = CR (
                 ScriptStack.ScriptList.Flink, 
                 EFI_BATCH_SCRIPT, 
                 Link, 
                 EFI_BATCH_SCRIPT_SIGNATURE
                 );
                 
      if (!IsListEmpty (&Script->StmtStack.StmtList)) {
        Stmt = CR (
                 Script->StmtStack.StmtList.Flink, 
                 EFI_BATCH_STATEMENT,
                 Link, 
                 EFI_BATCH_STMT_SIGNATURE
                 );
                 
        Print (
          L"shell: %s has no corresponding %s\n", 
          Stmt->StmtType == StmtIf ? L"IF" : L"FOR",
          Stmt->StmtType == StmtIf ? L"ENDIF" : L"ENDFOR"
          );
          
        SEnvSetBatchAbort ();
      }
    }
    Status = BatchPopScriptStack (1);
    if (EFI_ERROR (Status)) {
      Print (L"shell: Cannot execute script - %r", Status);
      SEnvSetAllBatchAbort ();
    }
  }
  
  //
  // If we are returning to the interactive shell, then reset 
  // the batch-is-active flag
  //
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    EchoIsOn = OldEchoIsOn;
  }

  return Status;
}
