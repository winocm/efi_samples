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

  ls.c

Abstract:

  EFI shell command "ls"

Revision History

--*/

#include "shell.h"

//
// Global Variables
//
BOOLEAN mLsAttributes;
BOOLEAN mLsAttribA;
BOOLEAN mLsAttribH;
BOOLEAN mLsAttribS;
BOOLEAN mLsAttribR;
BOOLEAN mLsAttribD;
BOOLEAN mLsRecursive;

UINT64  mLsTotalSize;
UINT64  mLsTotalFiles;
UINT64  mLsTotalDirs;

//
// Function declarations
//
EFI_STATUS
InitializeLS (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
LsList(
  IN EFI_HANDLE           ParentHandle,
  IN CHAR16               *ParentPath,
  IN CHAR16               *Pattern
  );

CHAR16* CleanupAsFatLfn (
  CHAR16  *Name
  );

//
// Entry Point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeLS)
#endif

EFI_STATUS
InitializeLS (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_UNSUPPORTED         - Protocols unsupported
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error

--*/
{
  EFI_STATUS              Status;
  CHAR16                  **Argv;
  UINTN                   Argc;
  CHAR16                  *PtrOne;
  CHAR16                  *PtrTwo;
  UINTN                   Index;
  UINTN                   Index1;
  UINTN                   Index2;
  UINTN                   Pos;
  EFI_LIST_ENTRY          DirList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  BOOLEAN                 FileArgEncountered;
  CHAR16                  *Pattern;
  BOOLEAN                 LeadingBlanks;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeLS,
    L"ls",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  //Local variable initializations
  //
  Argv = SI->Argv;
  Argc = SI->Argc;
  PtrOne = NULL;
  PtrTwo = NULL;
  Index = 0;
  InitializeListHead (&DirList);
  Link = NULL;
  Arg = NULL;
  FileArgEncountered = FALSE;
  Pattern = NULL;
  LeadingBlanks = FALSE;
  
  //
  //Global variable initializations
  //
  mLsAttributes = FALSE;
  mLsAttribA = mLsAttribH = mLsAttribS = mLsAttribR = mLsAttribD = FALSE;
  mLsRecursive = FALSE;
  mLsTotalSize = 0;
  mLsTotalFiles = 0;
  mLsTotalDirs = 0;

  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"ls: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // Check flags
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (Argv[Index][0] == '-' &&
      (Argv[Index][1] == 'B'||Argv[Index][1] == 'b')) {

      //
      // -b (PageBreak)
      //
      EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
    } else if (Argv[Index][0] == '-' &&
      (Argv[Index][1] == 'R' || Argv[Index][1] == 'r')) {

      //
      // -r (Recursive)
      //
      mLsRecursive = TRUE;
    } else if (Argv[Index][0] == '-' &&
      (Argv[Index][1] == 'A' || Argv[Index][1] == 'a')) {

      //
      // -a (Attribute)
      //
      mLsAttributes = TRUE;
      for (PtrOne = &Argv[Index][2]; *PtrOne; PtrOne++) {
        switch (*PtrOne) {
          case 'A':
          case 'a':
            mLsAttribA = TRUE;
            break;
            
          case 'H':
          case 'h':
            mLsAttribH = TRUE;
            break;
            
          case 'S':
          case 's':
            mLsAttribS = TRUE;
            break;
            
          case 'R':
          case 'r':
            mLsAttribR = TRUE;
            break;
            
          case 'D':
          case 'd':
            mLsAttribD = TRUE;
            break;
            
          default:
            Print (L"ls: Invalid file attribute '%hc'\n", *PtrOne);
            Status = EFI_INVALID_PARAMETER;
            goto Done;
        }
      }

      //
      // No attribute specified, default to all on
      //
      if (PtrOne == &Argv[Index][2]) {
        mLsAttribA = mLsAttribH = mLsAttribS = mLsAttribR = mLsAttribD = TRUE;
      }
    } else if (Argv[Index][0] == '-') {

      //
      // unknown flags
      //
      Print (L"ls: Unknown flag '%hs'\n", Argv[Index]);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {

      //
      // treat as file argument
      //

      //
      // we only fetch the first file argument
      //
      if (!FileArgEncountered) {
        FileArgEncountered = TRUE;

        //
        // build the pattern string
        //
        if (StrLen(Argv[Index]) == 0) {
          Pattern = NULL;
        } else {
          PtrTwo = Argv[Index];
          LeadingBlanks = TRUE;
          for (PtrOne = Argv[Index];
            PtrOne < &Argv[Index][StrLen(Argv[Index])];
            PtrOne ++) {
            if ((*PtrOne)==' ' && LeadingBlanks) {
              PtrTwo = PtrOne + 1;
            } else {
              LeadingBlanks = FALSE;
              if ((*PtrOne)=='\\' || (*PtrOne)==':') {
                PtrTwo = PtrOne + 1;
              }
            }                             
          }

          //
          // Filter redundant '*' in pattern
          //
          Index1 = 0;
          Index2 = 0;
          Pos = 0;
          while (PtrTwo[Index1] != '\0') {
            Pos = Index1;
            Index2 = Index1 + 1;
            if (PtrTwo[Pos] == '*') {
              Pos++;
              while (PtrTwo[Pos] == '*' && PtrTwo[Pos] != '\0') {
                Pos ++;
              }
              
              PtrTwo[Index2] = PtrTwo[Pos];
              while (PtrTwo[Index2] != '\0') {
                Index2 ++;
                Pos ++;
                PtrTwo[Index2] = PtrTwo[Pos];
              }
            }
            Index1++;
          }

          //
          // Strip off trailing blanks in pattern
          //
          
          for (Index1 = StrLen (PtrTwo) - 1;
               Index1 >= 0 && PtrTwo[Index1] == ' ';
               Index1 --) {
            PtrTwo[Index1] = 0;
          }
          
          Pattern = StrDuplicate(PtrTwo);
        }

        Status = ShellFileMetaArg (Argv[Index], &DirList);
        if (EFI_ERROR(Status)) {
          Print (L"ls: Cannot open %hs - %r\n", Argv[Index], Status);
          goto Done;
        }

        //
        // Even file not found, can't be an empty list
        //
        if (IsListEmpty(&DirList)) {
          Status = EFI_NOT_FOUND;
          Print(L"ls: Cannot open %hs - %r\n", Argv[Index], Status);
          goto Done;
        }
      } else {
        Print (L"ls: Too many arguments\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  }

  //
  // patch up with Pattern
  //
  if (!Pattern) {
    Pattern = StrDuplicate(L"");
  }

  //
  // If no file arguments are supplied, then the current directory
  //
  if (!FileArgEncountered) {
    Status = ShellFileMetaArg(L".", &DirList);
    if (EFI_ERROR(Status)) {
      Print (L"ls: Cannot open current directory - %r\n", Status);
      goto Done;
    }
    if (IsListEmpty(&DirList)) {
      Status = EFI_NOT_FOUND;
      Print(L"ls: Cannot open current directory -%r\n", Status);
      goto Done;
    }

    Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (EFI_ERROR(Arg->Status)) {
      Print(L"ls: Cannot open current directory - %r\n",Arg->Status);
      Status = Arg->Status;
      goto Done;
    }
  }

  //
  // no wildcards and is a directory
  //
  if (DirList.Flink->Flink == &DirList) {
    Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    if (Arg->Status == EFI_SUCCESS &&
      Arg->Info && (Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
      for (PtrOne = Pattern; *PtrOne; PtrOne ++) {
        if ((*PtrOne) == '*' ||
          (*PtrOne) == '?' ||
          (*PtrOne) == '[' ||
          (*PtrOne) == ']') {
          break;
        }
      }

      if (!(*PtrOne))
      {
        Status = LsList(Arg->Handle, Arg->FullName, L"*");
        goto Final;
      }
    }
  }

  //
  // Call LsList
  //
  Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link,
    SHELL_FILE_ARG_SIGNATURE);

  if (Arg->Parent) {
    Status = LsList(Arg->Parent, Arg->ParentName, Pattern);
  } else {
    Print(L"ls: Cannot open directory %hs - %r\n", 
          Arg->FullName, Arg->Status);
    Status = Arg->Status;
    goto Final;
  }

Final:
  if (Status == EFI_ABORTED) {
    goto Done;
  }
  //
  // Display Final Summary for recursive
  //
  if (mLsRecursive) {
    Print (L"  Total Summary:\n");
    Print (L"%11,ld File(s) %11,ld bytes\n", mLsTotalFiles, mLsTotalSize);
    Print (L"%11,ld Dir(s)\n", mLsTotalDirs);
  }

Done:
  //
  //Free resources
  //
  if (Pattern) {
    FreePool (Pattern);
  }
  ShellFreeFileList(&DirList);

  return Status;
}


EFI_STATUS
LsList(
  IN EFI_FILE_HANDLE   ParentHandle,
  IN CHAR16            *ParentPath,
  IN CHAR16            *Pattern
  )
/*++

Routine Description:

  Performs list operation on the directory indicated by ParentHandle.
  Use Pattern as the search string to find matches to be listed.
  Recursive search is performed if required

Arguments:

  ParentHandle    The handle of the parent directory
  ParentPath      The path of the parent directory
  Pattern         Search string

Returns:

  EFI_SUCCESS

--*/
{
  EFI_STATUS          Status;
  EFI_FILE_INFO       *Info;
  UINTN               BufSize;
  EFI_FILE_HANDLE     ThisHandle;
  UINT64              TotalSize;
  UINT64              TotalFiles;
  UINT64              TotalDirs;
  CHAR16              *FullPath;
  BOOLEAN             Something;

  //
  //local variable initializations
  //
  Status = EFI_SUCCESS;
  ThisHandle = NULL;
  Info = NULL;
  BufSize = SIZE_OF_EFI_FILE_INFO + 1024;// Large enough
  TotalSize = 0;
  TotalFiles = 0;
  TotalDirs = 0;
  FullPath = NULL;
  Something = FALSE;

  Info = AllocatePool (BufSize);
  if (!Info) {
    Print (L"ls: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto ListDone;
  }

  //
  //get parent directory info
  //
  Status = ParentHandle->GetInfo(ParentHandle, &GenericFileInfo, &BufSize, Info);
  if (EFI_ERROR(Status)) {
    Print (L"ls: Cannot get info of directory %hs\n", ParentPath);
    goto ListDone;
  }

  //
  //is it a directory?
  //
  if (!(Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print (L"ls: Cannot operate on a file %hs\n", ParentPath);
    Status = EFI_INVALID_PARAMETER;
    goto ListDone;
  }

  //
  //find all match items in the parent directory for the first time
  //to determine if this dir need to be displayed in Recursive case
  //
  Status = ParentHandle->SetPosition(ParentHandle, 0);
  if (EFI_ERROR(Status)) {
    Print (L"ls: Cannot access directory %hs\n", ParentPath);
    goto ListDone;
  }

  //
  //loop
  //
  for (;;) {

    //
    //reuse BufSize and Info
    //
    BufSize = SIZE_OF_EFI_FILE_INFO + 1024;
    Status = ParentHandle->Read(ParentHandle, &BufSize, Info);
    if (EFI_ERROR(Status)) {
      Print(L"ls: Cannot read from directory %hs - %r\n",
        ParentPath, Status);
      goto ListDone;
    }

    //
    //no more entries
    //
    if (BufSize == 0) {
      break;
    }

    //
    //if file name matches, and attributes right
    //
    if (
      (MetaiMatch(Info->FileName, Pattern) && !mLsAttributes &&
       !(Info->Attribute & EFI_FILE_HIDDEN) &&
       !(Info->Attribute & EFI_FILE_SYSTEM)
      )
      ||
      MetaiMatch(Info->FileName, Pattern) && mLsAttributes &&
      (Info->Attribute & (mLsAttribA ? EFI_FILE_ARCHIVE : 0) ||
       Info->Attribute & (mLsAttribH ? EFI_FILE_HIDDEN : 0) ||
       Info->Attribute & (mLsAttribR ? EFI_FILE_READ_ONLY : 0) ||
       Info->Attribute & (mLsAttribS ? EFI_FILE_SYSTEM : 0) ||
       Info->Attribute & (mLsAttribD ? EFI_FILE_DIRECTORY : 0))
       ) {
      Something = TRUE;
    }
  }

  if (Something || !mLsRecursive) {
    Print (L"Directory of: %hs\n",ParentPath);
    Print (L"\n");

    //
    //list all match items in the parent directory
    //
    Status = ParentHandle->SetPosition (ParentHandle, 0);
    if (EFI_ERROR(Status)) {
      Print (L"ls: Cannot access directory %hs\n", ParentPath);
      goto ListDone;
    }

    //
    //loop
    //
    for (;;) {

      //
      //reuse BufSize and Info
      //
      BufSize = SIZE_OF_EFI_FILE_INFO + 1024;
      Status = ParentHandle->Read(ParentHandle, &BufSize, Info);
      if (EFI_ERROR(Status)) {
        Print(L"ls: Cannot read from directory %hs - %r\n",
          ParentPath, Status);
        goto ListDone;
      }

      //
      //no more entries
      //
      if (BufSize == 0) {
        break;
      }

      //
      //if file name matches, and attributes right, display it
      //
      if (
        (MetaiMatch(Info->FileName, Pattern) && !mLsAttributes &&
        !(Info->Attribute & EFI_FILE_HIDDEN) &&
        !(Info->Attribute & EFI_FILE_SYSTEM)
        )
        ||
        MetaiMatch(Info->FileName, Pattern) && mLsAttributes &&
        (Info->Attribute & (mLsAttribA ? EFI_FILE_ARCHIVE : 0) ||
        Info->Attribute & (mLsAttribH ? EFI_FILE_HIDDEN : 0) ||
        Info->Attribute & (mLsAttribR ? EFI_FILE_READ_ONLY : 0) ||
        Info->Attribute & (mLsAttribS ? EFI_FILE_SYSTEM : 0) ||
        Info->Attribute & (mLsAttribD ? EFI_FILE_DIRECTORY : 0))
        ) {
        if (Info->Attribute & EFI_FILE_DIRECTORY) {
          TotalDirs ++;
          mLsTotalDirs ++;
        }
        else {
          TotalFiles ++;
          TotalSize += Info->FileSize;
          mLsTotalFiles ++;
          mLsTotalSize += Info->FileSize;
        }

        //
        //LsDumpFileInfo(Info);
        //
        Print (L"  %t %s %c  %11,ld  ",
              &Info->ModificationTime,
              Info->Attribute & EFI_FILE_DIRECTORY ? L"<DIR>" : L"     ",
              Info->Attribute & EFI_FILE_READ_ONLY ? 'r' : ' ',
              Info->FileSize
              );
      
        Print (L"%s\n", Info->FileName);
      }
    }

    //
    //Display subTotal
    //
    if (TotalFiles == 0 && TotalDirs == 0) {
      Print (L"File Not Found\n");
      Status = EFI_NOT_FOUND;
    }
    else {
      Print (L"%11,ld File(s) %11,ld bytes\n", TotalFiles, TotalSize);
      Print (L"%11,ld Dir(s)\n", TotalDirs);
      Print (L"\n");
    }

  }

  //
  //if recursive, for each dir within the parent directory,
  //call myself
  //
  if (mLsRecursive) {
    Status = ParentHandle->SetPosition(ParentHandle, 0);
    if (EFI_ERROR(Status)) {
      Print (L"ls: Cannot access directory %hs\n", ParentPath);
      goto ListDone;
    }

    //
    //loop
    //
    for (;;) {

      //
      //reuse BufSize and Info
      //
      BufSize = SIZE_OF_EFI_FILE_INFO + 1024;
      Status = ParentHandle->Read(ParentHandle, &BufSize, Info);
      if (EFI_ERROR(Status)) {
        Print(L"ls: Cannot read from directory %hs - %r\n",
          ParentPath, Status);
        goto ListDone;
      }

      //
      //no more entries
      //
      if (BufSize == 0) {
        break;
      }

      //
      //if it's a directory, open it and recursive
      //
      if (Info->Attribute & EFI_FILE_DIRECTORY &&
        StriCmp(Info->FileName, L".") != 0 &&
        StriCmp(Info->FileName, L"..") != 0) {

        //
        //Compose a full path for this dir
        //
        if (FullPath) {
          FreePool(FullPath);
        }

        FullPath = AllocatePool (StrSize(ParentPath)
          + 1 + StrSize(Info->FileName) + 2);
        if (!FullPath) {
          Print (L"ls: Out of memory\n");
          Status = EFI_OUT_OF_RESOURCES;
          goto ListDone;
        }

        StrCpy (FullPath, ParentPath);
        if (FullPath[StrLen(FullPath)-1] != '\\') {
          StrCat (FullPath, L"\\");
        }
        StrCat (FullPath, Info->FileName);

        Status = ParentHandle->Open(ParentHandle,
                &ThisHandle,
                Info->FileName,
                EFI_FILE_MODE_READ,
                0);
        if (EFI_ERROR(Status)) {
          Print (L"ls: Cannot open %hs - %r\n",
            FullPath, Status);
          goto ListDone;
        }

        Status = LsList(ThisHandle, FullPath, Pattern);
        if (Status == EFI_ABORTED) {
          goto ListDone;
        }

        ThisHandle->Close(ThisHandle);
      }
    }
  }//end of mLsRecursive

ListDone:
  if (FullPath) {
    FreePool(FullPath);
  }

  if (Info) {
    FreePool(Info);
  }


  return Status;
}

CHAR16* CleanupAsFatLfn (
  CHAR16  *Name
  )
{  
  CHAR16  *p1, *p2;
  
  //
  // Strip off starting/trailing spaces and trailing periods, as FAT spec.
  //
  for (p1 = Name; *p1 && *p1 == ' '; p1 ++) {
    ;
  }  
  
  p2 = Name;
  
  while (*p1) {
    *p2 = *p1;
    p1 ++;
    p2 ++;
  }
  
  *p2 = 0;
  
  for (p1 = Name + StrLen(Name) - 1; 
      p1 >= Name && (*p1 == ' ' || *p1 == '.');
      p1 --) {
    ;
  }
  
  *(p1 + 1) = 0;

  return Name;
}
