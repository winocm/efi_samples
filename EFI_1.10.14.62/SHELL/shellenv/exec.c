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

  exec.c
  
Abstract:

Revision History

--*/

#include "shelle.h"

extern EFI_LIST_ENTRY SEnvCurMapping;
extern EFI_LIST_ENTRY SEnvOrgFsDevicePaths;
extern EFI_LIST_ENTRY SEnvCurFsDevicePaths;
extern EFI_LIST_ENTRY SEnvOrgBlkDevicePaths;
extern EFI_LIST_ENTRY SEnvCurBlkDevicePaths;

typedef struct {
  CHAR16                       **Arg;
  UINTN                        ArgIndex;
  UINTN                        ArgCount;

  BOOLEAN                      Output;
  BOOLEAN                      Quote;
  UINTN                        AliasLevel;
  UINTN                        RecurseLevel;

  CHAR16                       *Buffer;
  UINTN                        BufferIndex;
  UINTN                        BufLength;
} PARSE_STATE;


typedef struct _SENV_OPEN_DIR {
  struct _SENV_OPEN_DIR        *Next;
  EFI_FILE_HANDLE              Handle;
} SENV_OPEN_DIR;

//
// Internal prototypes
//
EFI_STATUS
ShellParseStr (
  IN  CHAR16                   *Str,
  IN  OUT PARSE_STATE          *ParseState
  );

EFI_STATUS
ShellAppendBuffer (
  IN  PARSE_STATE              *ParseState,
  IN  UINTN                    Length,
  IN  CHAR16                   *String
  );

EFI_STATUS
SEnvDoExecute (
  IN  EFI_HANDLE               *ParentImageHandle,
  IN  CHAR16                   *CommandLine,
  IN  ENV_SHELL_INTERFACE      *Shell,
  IN  BOOLEAN                  Output
  );

VOID
SEnvLoadImage (
  IN  EFI_HANDLE               ParentImage,
  IN  CHAR16                   *IName,
  OUT EFI_HANDLE               *pImageHandle,
  OUT EFI_FILE_HANDLE          *pScriptsHandle
  );

//
//  Parser driver function
//
EFI_STATUS
SEnvStringToArg (
  IN  CHAR16                   *Str,
  IN  BOOLEAN                  Output,
  OUT CHAR16                   ***pArgv,
  OUT UINTN                    *pArgc
  )
/*++

Routine Description:
  Initialize the PARSE_STATE structure and call ShellParseStr to parse a 
  command line into several arguments.
  
Arguments:
  Str                          The command line string to be parsed
  Output                       If error message should be outputed to screen
  pArgv                        The result arguments
  pArgc                        The count of arguments
  
Returns:
  EFI_STATUS                   The function finished successfully
  
--*/
{
  PARSE_STATE                  ParseState;
  EFI_STATUS                   Status;

  //
  // Initialize a new state
  //
  ZeroMem (&ParseState, sizeof(ParseState));
  ParseState.Output = Output;
  ParseState.ArgCount = COMMON_ARG_COUNT;
  ParseState.Arg = AllocateZeroPool (COMMON_ARG_COUNT * sizeof(CHAR16 *));
  if (ParseState.Arg == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ParseState.BufLength = COMMON_ARG_LENGTH;
  ParseState.Buffer = AllocateZeroPool (COMMON_ARG_LENGTH * sizeof(CHAR16 *));
  if (ParseState.Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Parse the string
  //
  Status = ShellParseStr (Str, &ParseState);
  FreePool (ParseState.Buffer);

  *pArgv = ParseState.Arg;
  *pArgc = ParseState.ArgIndex;

  return Status;
}

VOID
ShellCopyStrSkipQuote (
  OUT CHAR16                   *Dst,
  IN  CHAR16                   *Src,
  IN  UINTN                    Length
  )
/*++

Routine Description:
  Copy a string from source to destination, all '"'s are skipped

Arguments:
  Dst              Destination string
  Src              Source string
  Length           Length of destination string

Returns:

--*/

{
  UINTN                        SrcIndex;
  UINTN                        DstIndex;
  
  SrcIndex = DstIndex = 0;
  while (DstIndex < Length) {
    //
    // In shell, '"' is used to delimit quotation so we skip all '"'s to get
    // the variable name.
    //
    if (Src[SrcIndex] != '"') {
      Dst[DstIndex] = Src[SrcIndex];
      DstIndex ++;
    }
    SrcIndex ++;
  }
}



EFI_STATUS
ShellParseStr (
  IN  CHAR16                   *Str,
  IN  OUT PARSE_STATE          *ParseState
  )
/*++

Routine Description:
  Parse a command line string into several arguments

Arguments:
  Str              Position to begin parse
  ParseState       The PARSE_STATE structure holding the current parse state

Returns:
  EFI_SUCCESS      The function finished successfully
  
--*/
{
  EFI_STATUS                   Status;
  CHAR16                       *Alias;
  CHAR16                       *SubstituteStr;
  CHAR16                       *OldSubstituteStr;
  BOOLEAN                      Literal; 
  BOOLEAN                      Comment;
  BOOLEAN                      IsVariable;
  UINTN                        ArgNo;
  CHAR16                       *VarName;
  UINTN                        VarNameLen;
  UINTN                        QuoteCount;

  ParseState->RecurseLevel += 1;
  if (ParseState->RecurseLevel > 5) {
    DEBUG ((EFI_D_VARIABLE, "shell: Recursive alias or macro\n"));
    if (ParseState->Output) {
      Print (L"shell: Recursive alias or macro\n");
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  SubstituteStr = NULL;
  VarNameLen = 0;

  while (*Str) {

    //
    // Skip leading white space
    //
    if (IsWhiteSpace(*Str)) {
      Str += 1;
      continue;
    }

    //
    // Pull this arg out of the string
    //
    ParseState->BufferIndex = 0;
    Literal = FALSE;
    Comment = FALSE;
    while (*Str) {

      //
      // If we have white space (including ',') and we are not in a quote,
      // move to the next word
      //
      if ((IsWhiteSpace (*Str)) && !ParseState->Quote) {
        break;
      }

      //
      // Check char
      //
      switch (*Str) {
      case '#':
        if (SEnvBatchIsActive()) {
          //
          //  Comment, discard the rest of the characters in the line
          //
          Comment = TRUE;
          while (*Str) {
            Str ++;
          }
          Str --;
        } else {
          ShellAppendBuffer (ParseState, 1, Str);
        }
        break;

      case '%':
        //
        // if in a script, try to find argument
        //
        IsVariable = FALSE;
        if (SEnvBatchIsActive () && IsDigit (Str[1])) {
          //
          // Found a script argument - substitute
          //
          ArgNo = Str[1] - '0';
          Status = SEnvBatchFindArgument (ArgNo, &SubstituteStr);
          if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
            Print (L"shell: Fail to find argument - %r\n", Status);
            SEnvSetBatchAbort ();
            goto Done;
          }

          if (!EFI_ERROR (Status)) {
            ShellAppendBuffer (ParseState, StrLen (SubstituteStr), 
              SubstituteStr);
            Str += 1;
            break;
          }
        }
        
        //
        // Try to find a shell enviroment variable
        //
        QuoteCount = 0;
        OldSubstituteStr = SubstituteStr;
        SubstituteStr = Str + 1;
        while (*SubstituteStr != '%' && *SubstituteStr != 0 &&
          (!IsWhiteSpace (*SubstituteStr) || ParseState->Quote)) {
          if (*SubstituteStr == '"') {
            ParseState->Quote = (BOOLEAN)(!ParseState->Quote);
            QuoteCount ++;
          }
          SubstituteStr ++;
        }
        
        if (*SubstituteStr == '%') {
          IsVariable = TRUE;
          VarNameLen = SubstituteStr - (Str + 1) - QuoteCount;
          VarName = AllocateZeroPool( (VarNameLen + 1) * sizeof(CHAR16) );
          if (!VarName) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
          }

          ShellCopyStrSkipQuote (VarName, Str+1, VarNameLen);
          VarName[VarNameLen] = (CHAR16)0x0000;
          //
          //  Check for special case "lasterror" variable
          //  Otherwise just get the matching environment variable
          //
          if (SEnvBatchIsActive () && SEnvBatchVarIsLastError (VarName)) {
            SubstituteStr = SEnvBatchGetLastError();
          } else {
            SubstituteStr = SEnvGetEnv (VarName);
          }
          FreePool (VarName);
           
          if (SubstituteStr) {
            //
            //  Insert the variable's value in the new arg - 
            //  the arg may include more than just the variable
            //
            ShellAppendBuffer (ParseState, StrLen (SubstituteStr), 
              SubstituteStr);
            Str += VarNameLen + 1 + QuoteCount;
            break;
          }
        }

        //
        // Try to find a for statement defined variable
        //      
        if (SEnvBatchIsActive () && IsAlpha (Str[1])) {
          //
          //  For loop index
          // 
          Status = SEnvBatchFindVariable (Str, &SubstituteStr);
          if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
            if (ParseState->Output) {
              Print (L"shell: Fail to find variable - %r\n");
            }
            goto Done;
          }

          if (!EFI_ERROR (Status) && SubstituteStr) {
            //
            //  Found a match
            //
            ShellAppendBuffer (ParseState, StrLen (SubstituteStr), 
              SubstituteStr);

            //
            // only advance one char - standard processing will get the 2nd char
            //
            Str += 1;
            break;   
          }
        }
        
        if (SEnvBatchIsActive () && IsDigit (Str[1])) {
          Str += 1;
          break;
        }
        
        if (IsVariable) {
          Str += VarNameLen + QuoteCount + 1;
        }
        break;

      case '^':
      
        //
        //  Literal, don't process aliases on this arg
        //
        if (Str[1]) {
          ShellAppendBuffer (ParseState, 1, &Str[1]);
          Str += 1;
          Literal = TRUE;
        }
        break;

      case '"':
        
        //
        //  Quoted string entry and exit
        //
        ParseState->Quote = (BOOLEAN)(!ParseState->Quote);
        break;

      default:
        if (!IsValidChar(*Str)) {
          if (ParseState->Output) {
            Print (L"shell: Invalid char '0x%x' in string\n", *Str);
          }
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
        ShellAppendBuffer (ParseState, 1, Str);
        break;
      }

      //
      // Next char
      //
      Str += 1;
    }

    //
    // If the new argument string is empty and we have encountered a 
    // comment, then skip it.  Otherwise we have a new arg
    //
    if ( Comment && ParseState->BufferIndex == 0 ) {
      break;
    }

    //
    // If it was composed with a literal, do not check to see if the arg has an
    // alias
    //
    Alias = NULL;
    ParseState->Buffer[ParseState->BufferIndex] = 0;
    if (!Literal  &&  !ParseState->AliasLevel  &&  ParseState->ArgIndex == 0) {
      Alias = SEnvGetAlias(ParseState->Buffer);
    }

    //
    // If there's an alias, parse it
    //
    if (Alias) {
      
      ParseState->AliasLevel += 1;
      Status = ShellParseStr (Alias, ParseState);
      ParseState->AliasLevel -= 1;

      if (EFI_ERROR(Status)) {
        goto Done;
      }

    } else {
      if (ParseState->ArgIndex == ParseState->ArgCount) {
        ParseState->Arg = ReallocatePool (ParseState->Arg, 
          ParseState->ArgCount * sizeof(CHAR16 *),
          (ParseState->ArgCount + COMMON_ARG_COUNT) * sizeof(CHAR16 *));
        if (ParseState->Arg == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          break;
        }
        ParseState->ArgCount += COMMON_ARG_COUNT;
      }
      //
      // Otherwise, copy the word to the arg array
      //
      ParseState->Arg[ParseState->ArgIndex] = StrDuplicate(ParseState->Buffer);
      if (!ParseState->Arg[ParseState->ArgIndex]) {
        Status = EFI_OUT_OF_RESOURCES;
        break;
      }

      ParseState->ArgIndex += 1;
    }
  }

  Status = EFI_SUCCESS;

Done:
  ParseState->RecurseLevel -= 1;
  if (EFI_ERROR(Status)) {
    
    //
    // Free all the args allocated
    //
    SEnvFreeArgument (&ParseState->ArgIndex, &ParseState->Arg);
  }

  return Status;
}

EFI_STATUS
ShellAppendBuffer (
  IN  PARSE_STATE              *ParseState,
  IN  UINTN                    Length,
  IN  CHAR16                   *String
  )
/*++

Routine Description:
  This function appends String to the end of buffer in ParseState. If the
  buffer is not large enough to hold the string, it will be expanded.
  
Arguments:
  Argc             Argument count
  Argv             The parsed arguments

Returns:

--*/
{
  UINTN                        Index;
  UINTN                        ExtendLength;
  
  Index = ParseState->BufferIndex;
  
  //
  // Check if current buffer can hold the string
  //
  if (Index + Length - 1 >= ParseState->BufLength - 1) {
    ExtendLength = Length > COMMON_ARG_LENGTH ? Length : COMMON_ARG_LENGTH;
    
    //
    // If cann't hold, allocate more memory
    //
    ParseState->Buffer = ReallocatePool (ParseState->Buffer, 
      ParseState->BufLength * sizeof (CHAR16 *),
      (ParseState->BufLength + ExtendLength) * sizeof (CHAR16 *));
    if (ParseState->Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    
    ZeroMem (ParseState->Buffer + ParseState->BufLength, 
      ExtendLength * sizeof (CHAR16 *));
    ParseState->BufLength += ExtendLength;
  }    

  //
  // Copy the string into the end of the buffer
  //
  CopyMem (&ParseState->Buffer[Index], String, Length * sizeof (CHAR16));
  ParseState->BufferIndex += Length;
  
  return EFI_SUCCESS;
}
  
VOID
SEnvFreeArgument (
  IN  UINTN                   *Argc,
  IN  CHAR16                  ***Argv
  )
/*++

Routine Description:
  This function frees the parsed arguments.
  
Arguments:
  Argc             Argument count
  Argv             The parsed arguments

Returns:

--*/
{
  UINT32                      Index;
  
  ASSERT (Argv);
  ASSERT (Argc);
  
  if (*Argv != NULL) {
    //
    // Free arguments according to the argument count
    //
    for (Index = 0; Index < *Argc; Index ++) {
      if ((*Argv)[Index] != NULL) {
        FreePool ((*Argv)[Index]);
        (*Argv)[Index] = NULL;
      }
    }
    FreePool (*Argv);
    *Argv = NULL;
  }
  *Argc = 0;
}

EFI_STATUS
SEnvRedirOutput (
  IN OUT ENV_SHELL_INTERFACE  *Shell,
  IN BOOLEAN                  Ascii,
  IN BOOLEAN                  Append,
  IN OUT UINTN                *NewArgc,
  IN OUT UINTN                *Index,
  OUT ENV_SHELL_REDIR_FILE    *Redir
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  CHAR16                      *FileName;
  EFI_STATUS                  Status;
  EFI_FILE_INFO               *Info;
  UINTN                       Size;
  CHAR16                      UnicodeMarker;
  UINT64                      FileMode;

  UnicodeMarker = EFI_UNICODE_BYTE_ORDER_MARK;

  //
  // Update args
  //
  if (!*NewArgc) {
    *NewArgc = *Index;
  }

  *Index += 1;
  if (*Index >= Shell->ShellInt.Argc) {
    return EFI_INVALID_PARAMETER;
  }

  if (Redir->Handle) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Open the output file
  //
  Redir->Ascii = Ascii;
  Redir->WriteError = EFI_SUCCESS;
  FileName = Shell->ShellInt.Argv[*Index];
  Redir->FilePath = SEnvNameToPath(FileName);

  if ( StriCmp (FileName, L"NUL") != 0) {
    if (Redir->FilePath) {
      FileMode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | ((Append)? 0 : EFI_FILE_MODE_CREATE);
      Redir->File = ShellOpenFilePath(Redir->FilePath, FileMode);
      if (Append && !Redir->File) {

        //
        // If file does not exist make a new one. And send us down the other path
        //
        FileMode |= EFI_FILE_MODE_CREATE;
        Redir->File = ShellOpenFilePath(Redir->FilePath, FileMode);
        Append = FALSE;
      }
    }

    if (!Redir->File) {
      Print(L"Could not open output file %hs\n", FileName);
      if (Redir->FilePath) {
        FreePool (Redir->FilePath);
      }
      Redir->FilePath = NULL;
      return EFI_INVALID_PARAMETER;
    }

    Info = LibFileInfo (Redir->File);
    ASSERT (Info);
    if (Append) {
      FreePool (Info);
      Size = sizeof(UnicodeMarker);
      Redir->File->Read (Redir->File, &Size, &UnicodeMarker);
      if ((UnicodeMarker == EFI_UNICODE_BYTE_ORDER_MARK) && Ascii) {
        Print(L"Could not Append Ascii to Unicode file %hs\n", FileName);
        if (Redir->FilePath) {
          FreePool (Redir->FilePath);
        }
        Redir->FilePath = NULL;
        return EFI_INVALID_PARAMETER;
      } else if ((UnicodeMarker != EFI_UNICODE_BYTE_ORDER_MARK) && !Ascii) {
        Print(L"Could not Append Unicode to Ascii file %hs\n", FileName);
        if (Redir->FilePath) {
          FreePool (Redir->FilePath);
        }
        Redir->FilePath = NULL;
        return EFI_INVALID_PARAMETER;
      }
  
      //
      // Seek to end of the file
      //
      Redir->File->SetPosition (Redir->File, (UINT64)-1);
    } else {
  
      //
      // Truncate the file
      //
      Info->FileSize = 0;
      Size = SIZE_OF_EFI_FILE_INFO + StrSize(Info->FileName);
      if (Redir->File->SetInfo) {
        Redir->File->SetInfo (Redir->File, &GenericFileInfo, Size, Info);
      } else {
        DEBUG ((EFI_D_ERROR, 
               "SEnvRedirOutput: SetInfo in file system driver not complete\n")
               );
      }
      FreePool (Info);

      if (!Ascii) {
        Size = sizeof(UnicodeMarker);
        Redir->File->Write(Redir->File, &Size, &UnicodeMarker);
      }
    }

    //
    // Allocate a new handle
    //
    CopyMem (&Redir->Out, &SEnvConToIo, sizeof(EFI_SIMPLE_TEXT_OUT_PROTOCOL));

  } else {
    CopyMem (&Redir->Out, &SEnvDummyConToIo, 
             sizeof(EFI_SIMPLE_TEXT_OUT_PROTOCOL));
    Redir->FilePath = EndDevicePath;
  }

  Status = LibInstallProtocolInterfaces (
          &Redir->Handle, 
          &gEfiSimpleTextOutProtocolGuid,       &Redir->Out,
          &gEfiDevicePathProtocolGuid,    Redir->FilePath,
          NULL
          );
  Redir->Signature = ENV_REDIR_SIGNATURE;
  ASSERT (!EFI_ERROR(Status));

  return EFI_SUCCESS;
}


EFI_STATUS
SEnvExecRedir (
  IN OUT ENV_SHELL_INTERFACE  *Shell
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN                   NewArgc;
  UINTN                   Index;
  UINTN                   RedirIndex;
  EFI_STATUS              Status;
  CHAR16                  *p;
  CHAR16                  LastChar;
  BOOLEAN                 Ascii;
  BOOLEAN                 Append;
  EFI_SYSTEM_TABLE        *SysTable;
  UINTN                   StringLen;
  BOOLEAN                 RedirStdOut;
  
  Status = EFI_SUCCESS;
  NewArgc = 0;
  SysTable = Shell->SystemTable;

  for (Index = 1; Index < Shell->ShellInt.Argc && !EFI_ERROR(Status); Index += 1) {
    p = Shell->ShellInt.Argv[Index];

    //
    // Trailing a or A means do ASCII default is Unicode
    //
    StringLen = StrLen(p);
    LastChar = p[StringLen - 1];
    Ascii =  (BOOLEAN)(((LastChar == 'a') || (LastChar == 'A')));
    
    //
    // Initializing Append to avoid a level 4 warning
    //
    Append = 0;

    RedirStdOut = FALSE;
    if (StrnCmp(p, L"2>", 2) == 0) {
      Status = SEnvRedirOutput (Shell, Ascii, FALSE, &NewArgc, 
                                &Index, &Shell->StdErr);
      SysTable->StdErr = &Shell->StdErr.Out;
      SysTable->StandardErrorHandle = Shell->StdErr.Handle;
      Shell->ShellInt.StdErr = Shell->StdErr.File;
    } else if (StrnCmp(p, L"1>", 2) == 0) {
      Append = (BOOLEAN)((p[2] == '>'));
      RedirStdOut = TRUE;
    } else if (*p == '>') {
      Append = (BOOLEAN)((p[1] == '>'));
      RedirStdOut = TRUE;
    }
    if (RedirStdOut) {
      Status = SEnvRedirOutput (Shell, Ascii, Append, &NewArgc, 
                                &Index, &Shell->StdOut);
      if (EFI_ERROR(Status)) {
        goto Done;
      }
      SysTable->ConOut = &Shell->StdOut.Out;
      SysTable->ConsoleOutHandle = Shell->StdOut.Handle;
      Shell->ShellInt.StdOut = Shell->StdOut.File;
    }
  }

  //
  //  Strip redirection args from arglist, saving in RedirArgv so they can be
  //  echoed in batch scripts.
  //
  if (NewArgc) {
    Shell->ShellInt.RedirArgc = Shell->ShellInt.Argc - (UINT32) NewArgc;
    Shell->ShellInt.RedirArgv = AllocateZeroPool (Shell->ShellInt.RedirArgc * sizeof(CHAR16 *));
    if ( !Shell->ShellInt.RedirArgv ) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
    RedirIndex = 0;
    for (Index = NewArgc; Index < Shell->ShellInt.Argc; Index += 1) {
      Shell->ShellInt.RedirArgv[RedirIndex++] = Shell->ShellInt.Argv[Index];
      Shell->ShellInt.Argv[Index] = NULL;
    }
    Shell->ShellInt.Argc = (UINT32) NewArgc;
  } else {
    Shell->ShellInt.RedirArgc = 0;
    Shell->ShellInt.RedirArgv = NULL;
  }

Done:
  return Status;
}


VOID
SEnvCloseRedir (
  IN OUT ENV_SHELL_REDIR_FILE    *Redir
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // Close redirection file
  //
  if (Redir->File) {
    Redir->File->Close (Redir->File);
    Redir->File = NULL;
  }
  
  //
  // Uninstall related protocol interfaces
  //
  if (Redir->Handle) {
    BS->UninstallProtocolInterface (Redir->Handle, 
                                    &gEfiSimpleTextOutProtocolGuid, 
                                    &Redir->Out);
    BS->UninstallProtocolInterface (Redir->Handle, 
                                    &gEfiSimpleTextInProtocolGuid, 
                                    &Redir->In);
    BS->UninstallProtocolInterface (Redir->Handle, 
                                    &gEfiDevicePathProtocolGuid, 
                                    Redir->FilePath);
    if (Redir->FilePath != EndDevicePath) {
      FreePool (Redir->FilePath);
    }
    Redir->Handle = NULL;
  }
}
    


EFI_STATUS
SEnvDoExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN ENV_SHELL_INTERFACE  *Shell,
  IN BOOLEAN              Output
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_SHELL_INTERFACE           *ParentShell;
  EFI_SYSTEM_TABLE              *ParentSystemTable;
  EFI_STATUS                    Status;
  UINTN                         Index;
  SHELLENV_INTERNAL_COMMAND     InternalCommand;
  EFI_HANDLE                    NewImage;
  EFI_FILE_HANDLE               Script;
  UINTN                         Index1;
  BOOLEAN                       ShowHelp;
  BOOLEAN                       ConsoleContextSaved;
  EFI_HANDLE                    SavedConsoleInHandle;
  EFI_HANDLE                    SavedConsoleOutHandle;
  EFI_HANDLE                    SavedStandardErrorHandle;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *SavedConIn;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SavedConOut;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SavedStdErr;
  EFI_HANDLE                    SavedRedirConsoleInHandle;
  EFI_HANDLE                    SavedRedirConsoleOutHandle;
  EFI_HANDLE                    SavedRedirStandardErrorHandle;

  //
  // Initializing Status to avoid a level 4 warning
  //
  Status                         = EFI_SUCCESS;
  ParentShell                    = NULL;
  ShowHelp                       = FALSE;
  ConsoleContextSaved            = FALSE;
  SavedConIn                     = NULL;
  SavedConOut                    = NULL;
  SavedStdErr                    = NULL;
  SavedConsoleInHandle           = NULL;
  SavedConsoleOutHandle          = NULL;
  SavedStandardErrorHandle       = NULL;
  SavedRedirConsoleInHandle      = NULL;
  SavedRedirConsoleOutHandle     = NULL;
  SavedRedirStandardErrorHandle  = NULL;

  //
  // Switch output attribute to normal
  //
  Print (L"%N");

  //
  //  Check that there is something to do
  //
  if (Shell->ShellInt.Argc < 1) {
    goto Done;
  }

  //
  // Assume some defaults
  //
  BS->HandleProtocol (ParentImageHandle, 
                      &gEfiLoadedImageProtocolGuid, 
                      (VOID*)&Shell->ShellInt.Info);
  Shell->ShellInt.ImageHandle = ParentImageHandle;
  Shell->ShellInt.StdIn  = &SEnvIOFromCon;
  Shell->ShellInt.StdOut = &SEnvIOFromCon;
  Shell->ShellInt.StdErr = &SEnvErrIOFromCon;

  //
  // Get parent's image stdout & stdin
  //
  Status = BS->HandleProtocol (ParentImageHandle, 
                               &ShellInterfaceProtocol, 
                               (VOID*)&ParentShell);
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  ParentSystemTable = ParentShell->Info->SystemTable;
  Shell->ShellInt.StdIn  = ParentShell->StdIn;
  Shell->ShellInt.StdOut = ParentShell->StdOut;
  Shell->ShellInt.StdErr = ParentShell->StdErr;

  Shell->SystemTable = Shell->ShellInt.Info->SystemTable;

  //
  // If the SystemTable is not NULL, then save the current console 
  // into local variables
  //
  if (Shell->SystemTable) {
    SavedConIn                = Shell->SystemTable->ConIn;
    SavedConOut               = Shell->SystemTable->ConOut;
    SavedStdErr               = Shell->SystemTable->StdErr;
    SavedConsoleInHandle      = Shell->SystemTable->ConsoleInHandle;
    SavedConsoleOutHandle     = Shell->SystemTable->ConsoleOutHandle;
    SavedStandardErrorHandle  = Shell->SystemTable->StandardErrorHandle;
    ConsoleContextSaved       = TRUE;
  }

  Status = SEnvBatchEchoCommand (Shell);
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  Status = SEnvExecRedir (Shell);
  SetCrc (&Shell->SystemTable->Hdr);

  SavedRedirConsoleInHandle      = Shell->SystemTable->ConsoleInHandle;
  SavedRedirConsoleOutHandle     = Shell->SystemTable->ConsoleOutHandle;
  SavedRedirStandardErrorHandle  = Shell->SystemTable->StandardErrorHandle;

  if (EFI_ERROR(Status)) {
    goto Done;
  }


  //
  // Handle special case of the internal "set default device command"
  // Is it one argument that ends with a ":"?
  //
  Index = StrLen(Shell->ShellInt.Argv[0]);
  if (Shell->ShellInt.Argc == 1 && Shell->ShellInt.Argv[0][0] != ':' && Shell->ShellInt.Argv[0][Index-1] == ':') {
    Status = SEnvSetCurrentDevice (Shell->ShellInt.Argv[0]);
    goto Done;
  }

  //
  // Attempt to dispatch it as an internal command
  //
  InternalCommand = SEnvGetCmdDispath(Shell->ShellInt.Argv[0]);
  if (InternalCommand) {

    for (Index1 = 1; Index1 < Shell->ShellInt.Argc; Index1++) {
      //
      // Having '-?' or '/?', show help information of this command
      //
      if ((StriCmp (Shell->ShellInt.Argv[Index1], L"-?") == 0) || (StriCmp (Shell->ShellInt.Argv[Index1], L"/?") == 0) ) {
        ShowHelp = TRUE;
        break;
      }

      //
      // Having '-b' or '/b', enable page break when output to ConOut
      //
      if (StriCmp (Shell->ShellInt.Argv[Index1], L"-b") == 0) {
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
      }
    }

    //
    // Push & replace the current shell info on the parent image handle.  
    // (note we are using the parent image's loaded image information structure)
    //
    BS->ReinstallProtocolInterface (ParentImageHandle, 
                                    &ShellInterfaceProtocol, 
                                    ParentShell, 
                                    &Shell->ShellInt);
    ParentShell->Info->SystemTable = Shell->SystemTable;

    InitializeShellApplication (ParentImageHandle, Shell->SystemTable);

    if (ShowHelp == TRUE) {
      PrintHelpInfo (Shell->ShellInt.Argv[0]);
    }
    else {
      //
      // Dispatch the command
      //
      Status = InternalCommand (ParentImageHandle, 
                                Shell->ShellInt.Info->SystemTable);
    }

    //
    // Restore the parent's image handle shell info
    //
    BS->ReinstallProtocolInterface (ParentImageHandle, 
                                    &ShellInterfaceProtocol, 
                                    &Shell->ShellInt, 
                                    ParentShell);
    ParentShell->Info->SystemTable = ParentSystemTable;
    InitializeShellApplication (ParentImageHandle, ParentSystemTable);
    goto Done;
  }

  //
  // Load the app, or open the script
  //
  SEnvLoadImage(ParentImageHandle, Shell->ShellInt.Argv[0], &NewImage, &Script);
  if (!NewImage  && !Script) {
    if ( Output ) {
      Print(L"'%es' not found\n", Shell->ShellInt.Argv[0]);
    }
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (NewImage) {
    CHAR16  *CurrentDir;
    CHAR16  *OptionsBuffer;
    UINT32  OptionsSize;

    //
    // Put the shell info on the handle
    //
    BS->HandleProtocol (NewImage, 
                        &gEfiLoadedImageProtocolGuid, 
                        (VOID*)&Shell->ShellInt.Info);
                        
    LibInstallProtocolInterfaces (&NewImage, 
                                  &ShellInterfaceProtocol, 
                                  &Shell->ShellInt, NULL);

    //
    // Create load options which may include command line and current
    // working directory
    //
    CurrentDir = SEnvGetCurDir(NULL);
    for (Index1 = OptionsSize = 0; Index1 < Shell->ShellInt.Argc; Index1++) {
      //
      // StrSize includes NULL which we need for space between args
      //
      OptionsSize += (UINT32)StrSize (Shell->ShellInt.Argv[Index1]);
    }
    
    OptionsSize += sizeof (CHAR16);

    if (CurrentDir) {
      OptionsSize += (UINT32)StrSize (CurrentDir); // StrSize includes NULL
    }
    OptionsBuffer = AllocateZeroPool (OptionsSize);

    if (OptionsBuffer) {
      //
      // Set the buffer before we manipulate it.
      //
      Shell->ShellInt.Info->LoadOptions = OptionsBuffer;
      Shell->ShellInt.Info->LoadOptionsSize = OptionsSize;

      //
      // Copy the command line and current working directory
      //
      for (Index1 = 0; Index1 < Shell->ShellInt.Argc; Index1++) {
        StrCat (OptionsBuffer, Shell->ShellInt.Argv[Index1]);
        StrCat (OptionsBuffer, L" ");
      }
      
      if (CurrentDir) {
        StrCpy (&OptionsBuffer[ StrLen (OptionsBuffer) + 1 ], CurrentDir);
      }
    } else {
      //
      // Fail semi-gracefully (i.e. no command line expansion)
      //
      Shell->ShellInt.Info->LoadOptions = CommandLine;
      Shell->ShellInt.Info->LoadOptionsSize = (UINT32) StrSize (CommandLine);
    }

    //
    // Pass a copy of the system table with new input & outputs
    //
    Shell->ShellInt.Info->SystemTable = Shell->SystemTable;

    //
    // If the image is an app start it, else abort it
    //
    if (Shell->ShellInt.Info->ImageCodeType == EfiLoaderCode) {
      InitializeShellApplication (ParentImageHandle, Shell->SystemTable);
      Status = BS->StartImage (NewImage, NULL, NULL);
    } else {
      Print (L"Image is not a application\n");
      BS->Exit(NewImage, EFI_INVALID_PARAMETER, 0, NULL);
      Status = EFI_INVALID_PARAMETER;
    }

    //
    // App has exited, remove our data from the image handle
    //
    if (OptionsBuffer) {
      FreePool (OptionsBuffer);
    }
    /////////////////////////////////////////
    //Appended at 2003-08-07
    if(CurrentDir){
      FreePool(CurrentDir);
    }
    ////////////////////////////////////////

    BS->UninstallProtocolInterface (NewImage, 
                                    &ShellInterfaceProtocol, 
                                    &Shell->ShellInt);
    InitializeShellApplication (ParentImageHandle, ParentSystemTable);

  } else if ( Script ) {

    //
    // Push & replace the current shell info on the parent image handle.
    // (note we are using the parent image's loaded image information structure)
    //
    BS->ReinstallProtocolInterface (ParentImageHandle, 
                                    &ShellInterfaceProtocol, 
                                    ParentShell, 
                                    &Shell->ShellInt);
    ParentShell->Info->SystemTable = Shell->SystemTable;

    Status = SEnvExecuteScript( Shell, Script );

    //
    // Restore the parent's image handle shell info
    //
    BS->ReinstallProtocolInterface (ParentImageHandle, 
                                    &ShellInterfaceProtocol, 
                                    &Shell->ShellInt, 
                                    ParentShell);
    ParentShell->Info->SystemTable = ParentSystemTable;
    InitializeShellApplication (ParentImageHandle, ParentSystemTable);
  }
  
Done:

  DisablePageBreak ();
  // SEnvBatchSetLastError( Status );

#ifdef EFI_DEBUG

  if (EFI_ERROR(Status)  &&  Output) {
    if (Status != -1) {
      //
      // Don't Print on a "Disconnect All" exit. The ConOut device may not exist
      //
      Print (L"Exit status code: %r\n", Status);
    }
  }
#endif

  //
  // Cleanup
  //
  if (Shell) {

    //
    // If there's an arg list, free it
    //
    SEnvFreeArgument (&Shell->ShellInt.Argc, &Shell->ShellInt.Argv);

    //
    //  If any redirection arguments were saved, free them
    //
    SEnvFreeArgument (&Shell->ShellInt.RedirArgc, &Shell->ShellInt.RedirArgv);

    //
    // Close any file redirection
    //
    SEnvCloseRedir(&Shell->StdOut);
    SEnvCloseRedir(&Shell->StdErr);
    SEnvCloseRedir(&Shell->StdIn);

    //
    // If the SystemTable is not NULL, then restore the current console into 
    // local variables
    //
    if (ConsoleContextSaved) {
      if (SavedRedirConsoleInHandle == Shell->SystemTable->ConsoleInHandle) {
        Shell->SystemTable->ConIn               = SavedConIn; 
        Shell->SystemTable->ConsoleInHandle     = SavedConsoleInHandle;
      }

      if (SavedRedirConsoleOutHandle == Shell->SystemTable->ConsoleOutHandle) {
        Shell->SystemTable->ConOut              = SavedConOut;
        Shell->SystemTable->ConsoleOutHandle    = SavedConsoleOutHandle;
      }

      if (SavedRedirStandardErrorHandle == Shell->SystemTable->StandardErrorHandle) {
        Shell->SystemTable->StdErr              = SavedStdErr;
        Shell->SystemTable->StandardErrorHandle = SavedStandardErrorHandle;
      }
      SetCrc (&Shell->SystemTable->Hdr);
    }
  }

  //
  // Switch output attribute to normal
  //
  if (Status != -1) {
    //
    // Don't Print on a "Disconnect All" exit. The ConOut device may not exist
    //
    Print (L"%N");
  }

  return Status;
}


EFI_STATUS
SEnvExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN BOOLEAN              Output
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  ENV_SHELL_INTERFACE     Shell;
  EFI_STATUS              Status;

  EFI_LIST_ENTRY          *Link;
  MAPPING_INFO            *MappingInfo;
  BOOLEAN                 Invalid, PrintHead;

  Status = EFI_SUCCESS;
  Invalid = FALSE;

  //
  // Make all file systems as un-accessed
  //
  for (Link = SEnvCurMapping.Flink; Link != &SEnvCurMapping; Link = Link->Flink) {
    MappingInfo = CR (Link, MAPPING_INFO, Link, MAPPING_INFO_SIGNATURE);
    MappingInfo->Accessed = FALSE;
  }
  
  //
  // Convert the command line to an arg list
  //
  ZeroMem (&Shell, sizeof(Shell));
  Status = SEnvStringToArg (CommandLine, Output, 
                            &Shell.ShellInt.Argv, &Shell.ShellInt.Argc );
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // Execute the command
  //
  Status = SEnvDoExecute( ParentImageHandle, CommandLine, &Shell, Output );
  if (EFI_ERROR(Status)) {
    goto Done;
  }

Done:
  //
  // If command is a file/dir related command,
  // display invalid file systems mappings used by this command
  //
  // SEnvCheckValidMappings ();
  
  PrintHead = TRUE;
  for (Link = SEnvCurMapping.Flink; Link != &SEnvCurMapping; Link = Link->Flink) {
    MappingInfo = CR (Link, MAPPING_INFO, Link, MAPPING_INFO_SIGNATURE);
    if ((MappingInfo->Accessed == TRUE) && (MappingInfo->Valid == FALSE)) {
      Invalid = TRUE;
      if (PrintHead) {
        Print (L"Invalid file system mapping on");
        PrintHead = FALSE;
      }
      Print (L" %hs", MappingInfo->MappedName);
    }
  }
  
  if (Invalid) {
    Print (L"\n\n");
  }
  return Status;
}


VOID
SEnvLoadImage (
  IN EFI_HANDLE           ParentImage,
  IN CHAR16               *IName,
  OUT EFI_HANDLE          *pImageHandle,
  OUT EFI_FILE_HANDLE     *pScriptHandle
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  CHAR16                      *Path;
  BOOLEAN                     PathNeedFree;
  CHAR16                      *Ptr1;
  CHAR16                      *Ptr2;
  CHAR16                      *PathName;
  CHAR16                      *OldPathName;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  FILEPATH_DEVICE_PATH        *FilePath;
  CHAR16                      c;
  EFI_HANDLE                  ImageHandle;
  EFI_STATUS                  Status;
  SENV_OPEN_DIR               *OpenDir;
  SENV_OPEN_DIR               *OpenDirHead;
  EFI_FILE_HANDLE             ScriptHandle;
  INTN                        INameLen, PathPos;
  
  PathName = NULL;
  DevicePath = NULL;
  ImageHandle = NULL;
  ScriptHandle = NULL;
  OpenDirHead = NULL;
  *pImageHandle = NULL;
  *pScriptHandle = NULL;
  PathNeedFree = FALSE;

  //
  // Check if IName contains path info
  //
  INameLen = StrLen (IName);
  if (INameLen == 0) {
    return;
  }
  
  Ptr1 = NULL;
  Ptr2 = NULL;
    
  for (PathPos = INameLen - 1; PathPos >= 0; PathPos--) {
    if (IName[PathPos] == ':' || IName[PathPos] == '\\') {
      if (PathPos != INameLen - 1) {
        Ptr1 = &IName[PathPos+1];
      }
      break;
    }
  }
  
  //
  // Processing :foo
  //
  if (PathPos == 0 && IName[PathPos] == ':') {
    return;
  }
  
  //
  // Processing foo: or foo:\
  //
  if (PathPos != -1 && Ptr1 == NULL) {
    return;
  }

  if (PathPos >= 0) {
    Path = AllocatePool ((PathPos+1+1) * sizeof (CHAR16));
    if (Path == NULL) {
      return;
    }
    PathNeedFree = TRUE;

    ZeroMem (Path, (PathPos+1+1) * sizeof (CHAR16));
    CopyMem (Path, IName, (PathPos+1) * sizeof (CHAR16));
    //
    // Processing foo:bar
    //
    if (Path[PathPos] == ':') {
      Path[PathPos] = 0;
      Ptr2 = ShellCurDir (Path);
      FreePool (Path);
      PathNeedFree = FALSE;
      if (Ptr2) {
        Path = Ptr2;
        PathNeedFree = TRUE;
      } else {
        return;
      }
    }
    IName = Ptr1;
    Ptr1 = StrDuplicate(Path);
    if (PathNeedFree) {
      FreePool (Path);
    }
  } else {
    //
    // Get the path variable 
    //
    Path = SEnvGetEnv (L"path");
    PathNeedFree = FALSE;
    if (!Path) {
      Path = SEnvGetCurDir (NULL);
      PathNeedFree = TRUE;
      // DEBUG ((EFI_D_VARIABLE, "SEnvLoadImage: no path variable\n"));
      // return ;
    /*
    //
    // BUGBUG: Do we need to add current path as the first in path list ?
    //
    } else {
      Ptr1 = SEnvGetCurDir (NULL);
      if (Ptr1) {
        Ptr2 = AllocatePool ((StrLen (Path) + StrLen (Ptr1)) * sizeof (CHAR16));
        CopyMem (Ptr2, Ptr1, StrLen (Ptr1) * sizeof (CHAR16));
        Ptr1 = Ptr2;
        Ptr2 += StrLen (SEnvGetCurDir (NULL));
        c = L';';
        CopyMem (Ptr2, &c,  sizeof (CHAR16));
        Ptr2 += 1;
        CopyMem (Ptr2, Path, StrLen (Path) * sizeof (CHAR16));
        Ptr2 = Ptr1;
        Path = Ptr2;
      }
    */
    }
    if (!Path) {
      return;
    }
    Ptr1 = StrDuplicate(Path);
    if (PathNeedFree) {
      FreePool (Path);
    }
  }

  if (!Path) {
    return;
  }
  
  Path = Ptr1;

  //
  // Search each path component
  // (using simple ';' as separator here - oh well)
  //
  c = *Path;
  for (Ptr1=Path; *Ptr1 && c; Ptr1=Ptr2+1) {
    for (Ptr2=Ptr1; *Ptr2 && *Ptr2 != ';'; Ptr2++) ;

    if (Ptr1 != Ptr2) {
      c = *Ptr2;
      
      //
      // Null terminate the path
      //
      *Ptr2 = 0;
      //
      // Normally, a reference to a \FOO.EFI would end up
      // having a "." prepended to it because the environment
      // always has a "." as the first directory to check.
      // and you would end up executing .\FOO.EFI which is
      // not desired.
      // If the target starts with a "\", then we need to
      // progress the IName pointer past the "\" and seed the
      // *Ptr1 which is the environment pointer which has the "."
      // with a "\"
      //
      if ( StrnCmp( L"\\", &IName[0], 1) == 0 ) {
        *Ptr1 = '\\';
        IName++;
      }

      //
      // Open the directory 
      //
      DevicePath = SEnvNameToPath(Ptr1);
      if (!DevicePath) {
        continue;
      }

      OpenDir = AllocateZeroPool (sizeof(SENV_OPEN_DIR));
      if (!OpenDir) {
        break;
      }

      OpenDir->Handle = ShellOpenFilePath(DevicePath, EFI_FILE_MODE_READ);
      OpenDir->Next = OpenDirHead;
      OpenDirHead = OpenDir;
      FreePool (DevicePath);
      DevicePath = NULL;
      if (!OpenDir->Handle) {
        continue;
      }

      if (StriCmp( L".nsh", &(IName[StrLen(IName)-4]) ) != 0) {
        //
        // Attempt to open it as an executable 
        //
        if (StriCmp( L".efi", &(IName[StrLen(IName)-4]) ) == 0) {
          PathName = (Ptr2[-1] == ':' || Ptr2[-1] == '\\') ? L"%s%s" : L"%s\\%s";
        } else {
          PathName = (Ptr2[-1] == ':' || Ptr2[-1] == '\\') ? L"%s%s.efi" : L"%s\\%s.efi";
        }
      
        PathName = PoolPrint(PathName, Ptr1, IName);
        if (!PathName) {
          break;
        }
  
        if (PathName[0] == '.' && PathName[1] == '\\') {
          OldPathName = PathName;
          PathName = PoolPrint(&PathName[2]);
          FreePool (OldPathName);
        }
  
        DevicePath = SEnvNameToPath(PathName);
        if (!DevicePath) {
          continue;
        }
  
        //
        // Attempt to load the image
        //
        Status = BS->LoadImage (FALSE, ParentImage, DevicePath, 
                                NULL, 0, &ImageHandle);
        if (!EFI_ERROR(Status)) {
          goto Done;
        }
  
        //
        // Try as a ".nsh" file
        //
        FreePool(DevicePath);
        FreePool(PathName);
        DevicePath = NULL;
        PathName = NULL;
      }
      
      if ( StriCmp( L".nsh", &(IName[StrLen(IName)-4]) ) == 0 ) {
        //
        //  User entered entire filename with .nsh extension
        //
        PathName = PoolPrint (L"%s", IName);
      } else {
        //
        //  User entered filename without .nsh extension
        //
        PathName = PoolPrint (L"%s.nsh", IName);
      }
      if (!PathName) {
        break;
      }

      DevicePath = SEnvFileNameToPath(PathName);
//      DevicePath = SEnvNameToPath(PathName);
      if (DevicePath) {
//        ASSERT (
//          DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && 
//          DevicePathSubType(DevicePath) == MEDIA_FILEPATH_DP
//          );

        FilePath = (FILEPATH_DEVICE_PATH *) DevicePath;
        
        Status = OpenDir->Handle->Open (
              OpenDir->Handle,
              &ScriptHandle,
              FilePath->PathName,
              EFI_FILE_MODE_READ,
              0
              );

        FreePool(DevicePath);
        DevicePath = NULL;

        if (!EFI_ERROR(Status)) {
          goto Done;
        }
      }

      //
      // BUGBUG
      //
      ScriptHandle = NULL;
    }    
    
    if (DevicePath) {
      FreePool (DevicePath);
      DevicePath = NULL;
    }

    if (PathName) {
      FreePool (PathName);
      PathName = NULL;
    }
  }

Done:
  while (OpenDirHead) {
    if (OpenDirHead->Handle) {
      OpenDirHead->Handle->Close (OpenDirHead->Handle);
    }
    OpenDir = OpenDirHead->Next;
    FreePool (OpenDirHead);
    OpenDirHead = OpenDir;
  }

  FreePool (Path);

  if (DevicePath) {
    FreePool (DevicePath);
    DevicePath = NULL;
  }

  if (PathName) {
    FreePool (PathName);
    PathName = NULL;
  }

  if (ImageHandle) {
    ASSERT (!ScriptHandle);
    *pImageHandle = ImageHandle;
  }

  if (ScriptHandle) {
    ASSERT (!ImageHandle);
    *pScriptHandle = ScriptHandle;
  }
}


EFI_STATUS
SEnvExit (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // BUGBUG: for now just use a "magic" return code to indicate EOF
  //
  //
  // All allocated resources for Shell will be freed in SEnvFreeResources (),
  // it will be called when Shell exits (In newshell\init.c).
  //
  return  (EFI_STATUS)-1;
}



VOID
SEnvFreeResources (
  VOID
)
/*++

Routine Description:
  Free all resources allocated in Shell

Arguments:

Returns:

--*/
{
  MAPPING_INFO    *MappingInfo;
  DEVICEPATH_INFO *DevicePathInfo;
  
  //
  // Remove allocated resources for mapping info check
  //
  while (!IsListEmpty (&SEnvCurMapping)) {
    MappingInfo = CR (SEnvCurMapping.Flink, MAPPING_INFO, 
                      Link, MAPPING_INFO_SIGNATURE);
    FreePool (MappingInfo->MappedName);
    FreePool (MappingInfo->DevicePath);
    RemoveEntryList (&MappingInfo->Link);
    FreePool (MappingInfo);
  }

  while (!IsListEmpty (&SEnvOrgFsDevicePaths)) {
    DevicePathInfo = CR (SEnvOrgFsDevicePaths.Flink, DEVICEPATH_INFO, Link, 
                          DEVICEPATH_INFO_SIGNATURE);
    FreePool (DevicePathInfo->DevicePath);
    RemoveEntryList (&DevicePathInfo->Link);
    FreePool (DevicePathInfo);
  }
  
  while (!IsListEmpty (&SEnvCurFsDevicePaths)) {
    DevicePathInfo = CR (SEnvCurFsDevicePaths.Flink, DEVICEPATH_INFO, Link, 
                          DEVICEPATH_INFO_SIGNATURE);
    FreePool (DevicePathInfo->DevicePath);
    RemoveEntryList (&DevicePathInfo->Link);
    FreePool (DevicePathInfo);
  }
  
  while (!IsListEmpty (&SEnvOrgBlkDevicePaths)) {
    DevicePathInfo = CR (SEnvOrgBlkDevicePaths.Flink, DEVICEPATH_INFO, Link, 
                          DEVICEPATH_INFO_SIGNATURE);
    FreePool (DevicePathInfo->DevicePath);
    RemoveEntryList (&DevicePathInfo->Link);
    FreePool (DevicePathInfo);
  }
  
  while (!IsListEmpty (&SEnvCurBlkDevicePaths)) {
    DevicePathInfo = CR (SEnvCurBlkDevicePaths.Flink, DEVICEPATH_INFO, Link, 
                          DEVICEPATH_INFO_SIGNATURE);
    FreePool (DevicePathInfo->DevicePath);
    RemoveEntryList (&DevicePathInfo->Link);
    FreePool (DevicePathInfo);
  }
}