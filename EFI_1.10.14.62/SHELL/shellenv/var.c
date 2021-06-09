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

  var.c
  
Abstract:

  Shell environment variable management

Revision History

--*/

#include "shelle.h"

//
// The different variable categories
//
EFI_LIST_ENTRY  SEnvEnv;
EFI_LIST_ENTRY  SEnvMap;
EFI_LIST_ENTRY  SEnvAlias;

VOID
SEnvInitVariables (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  CHAR16              *Name;
  CHAR16              *Data;
  UINTN               BufferSize;
  UINTN               NameSize, DataSize;
  EFI_GUID            Id;
  EFI_LIST_ENTRY      *ListHead;
  VARIABLE_ID         *Var;
  EFI_STATUS          Status;
  BOOLEAN             IsString;
  UINT32              Attributes;
  UINTN               Size;

  //
  // Initialize the different variable lists
  //

  InitializeListHead (&SEnvEnv);
  InitializeListHead (&SEnvMap);
  InitializeListHead (&SEnvAlias);

  BufferSize = 1024;
  Name = AllocatePool (BufferSize);
  Data = AllocatePool (BufferSize);
  ASSERT(Name && Data); 

  Var = NULL;
  
  //
  // Read all the variables in the system and collect ours
  //
  Name[0] = 0;
  for (; ;) {
    NameSize = BufferSize;
    Status = RT->GetNextVariableName (&NameSize, Name, &Id);
    if (EFI_ERROR(Status)) {
      break;
    }

    //
    // See if it's a shellenv variable
    //
    ListHead = NULL;
    IsString = FALSE;
    if (CompareGuid (&Id, &SEnvEnvId) == 0) {
      ListHead = &SEnvEnv;
      IsString = TRUE;
    }

    if (CompareGuid (&Id, &SEnvMapId) == 0) {
      ListHead = &SEnvMap;
    }

    if (CompareGuid (&Id, &SEnvAliasId) == 0) {
      ListHead = &SEnvAlias;
      IsString = TRUE;
    }

    if (ListHead) {
      DataSize = BufferSize;
      Status = RT->GetVariable (Name, &Id, &Attributes, &DataSize, Data);

      if (!EFI_ERROR(Status)) {

        //
        // Add this value
        //
        Size = sizeof(VARIABLE_ID) + StrSize(Name) + DataSize;
        Var  = AllocateZeroPool (Size);

        Var->Signature = VARIABLE_SIGNATURE;
        Var->u.Value = ((UINT8 *) Var) + sizeof(VARIABLE_ID);
        Var->Name = (CHAR16*) (Var->u.Value + DataSize);
        Var->ValueSize = DataSize;
        CopyMem (Var->u.Value, Data, DataSize);
        CopyMem (Var->Name, Name, NameSize);

        if( Attributes & EFI_VARIABLE_NON_VOLATILE ) {
          Var->Flags = NON_VOL ; 
        }
        else {
          Var->Flags = VOL ; 
        }

        InsertTailList (ListHead, &Var->Link);
      }
    }

    //
    // If this is a protocol entry, add it
    //
    if (CompareGuid (&Id, &SEnvProtId) == 0) {

      DataSize = BufferSize;
      Status = RT->GetVariable (Name, &Id, &Attributes, &DataSize, Data);

      if (!EFI_ERROR(Status)  && DataSize == sizeof (EFI_GUID)) {
        SEnvIAddProtocol (FALSE, (EFI_GUID *) Data, NULL, NULL, Name);
      } else {
        DEBUG ((EFI_D_INIT|EFI_D_WARN, 
              "SEnvInitVariables: skipping bogus protocol id %s\n", Var->Name));
        RT->SetVariable (Name, &SEnvProtId, 0, 0, NULL);
      }
    }
  }

  FreePool (Name);
  FreePool (Data);
}


CHAR16 *
SEnvIGetStr (
  IN CHAR16           *Name,
  IN EFI_LIST_ENTRY   *Head
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY      *Link;
  VARIABLE_ID         *Var;
  CHAR16              *Value;

  AcquireLock (&SEnvLock);

  //
  // Walk through linked list to find corresponding Var
  //
  Value = NULL;
  for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
    Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp (Var->Name, Name) == 0) {
      Value = Var->u.Str;
      break;
    }
  }

  ReleaseLock (&SEnvLock);
  return Value;
}


CHAR16 *
SEnvGetMap (
  IN CHAR16           *Name
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return SEnvIGetStr (Name, &SEnvMap);
}


CHAR16 *
SEnvGetEnv (
  IN CHAR16           *Name
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return SEnvIGetStr (Name, &SEnvEnv);
}


CHAR16 *
SEnvGetAlias (
  IN CHAR16           *Name
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return SEnvIGetStr (Name, &SEnvAlias);
}


VOID
SEnvSortVarList (
  IN EFI_LIST_ENTRY           *Head
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  ASSERT_LOCKED(&SEnvLock);

  return ;
}


EFI_STATUS
SEnvCmdSA (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable,
  IN EFI_LIST_ENTRY           *Head,
  IN EFI_GUID                 *Guid,
  IN CHAR16                   *CmdName
  )
/*++

Routine Description:

  Code for shell "set" & "alias" command

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY              *Link;
  VARIABLE_ID                 *Var;
  VARIABLE_ID                 *Found;
  CHAR16                      *Name;
  CHAR16                      *Value;    
  UINTN                       Size, SLen, Len;
  BOOLEAN                     Delete;    
  EFI_STATUS                  Status;
  UINTN                       Index;
  CHAR16                      *p;
  BOOLEAN                     Volatile;    

  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Initializing variable to avoid level 4 warning
  //
  Name = NULL;
  Value = NULL;
  Delete = FALSE;
  Status = EFI_SUCCESS;
  Found = NULL;
  Volatile = FALSE;
  Var = NULL;

  //
  // Crack arguments
  //
  for (Index = 1; Index < SI->Argc; Index += 1) {
    p = SI->Argv[Index];
  if (*p == 0) {
      return EFI_INVALID_PARAMETER;
  }
    if (*p == '-') {
      switch (p[1]) {
      case 'd':
      case 'D':
        Delete = TRUE;
        break;

      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case 'v' :
      case 'V' :
        Volatile = TRUE;
        break;

      default:
        Print (L"%s: Unknown flag %hs\n", CmdName, p);
        return EFI_INVALID_PARAMETER;
      }
      continue;
    }

    if (!Name) {
      if (SEnvGetCmdDispath (p) && (StriCmp (CmdName, L"alias") == 0)) {
        Print (L"%s: %hs is an internal command\n", CmdName, p);
        return EFI_INVALID_PARAMETER;
      }
      else {
        Name = p;
        continue;
      }
    }

    if (!Value) {
      Value = p;
      continue;
    }

    Print (L"%s: Too many arguments\n", CmdName);
    return EFI_INVALID_PARAMETER;
  }

  if (Delete && Value) {
    Print (L"%s: Too many arguments\n", CmdName);
    return EFI_INVALID_PARAMETER;
  }

  //
  // Process
  //
  AcquireLock (&SEnvLock);

  if (!Name) {
    //
    // dump the list
    //
    SEnvSortVarList (Head);

    SLen = 0;
    for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
      Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      Len = StrLen(Var->Name);
      if (Len > SLen) {
        SLen = Len;
      }
    }

    for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
      Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      if( Var->Flags == VOL ) {
        Print(L"  * %h-.*s : %s\n", SLen, Var->Name, Var->u.Str);
      }
      else {
        Print(L"    %h-.*s : %s\n", SLen, Var->Name, Var->u.Str);
      }
    }

  } else {
    //
    // Find the specified value
    //
    for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
      Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      if (StriCmp(Var->Name, Name) == 0) {
        Found = Var;
        break;
      }
    }

    if (Found && Delete) {
      //
      // Remove it from the store
      //
      Status = RT->SetVariable (Found->Name, Guid, 0, 0, NULL);
      if (Status == EFI_NOT_FOUND) {
        Print (L"%s: %hs not found\n", CmdName, Found->Name);
      }
    } else if (Value) {
      //
      // Add it to the store
      //
      if( Found && ( ( Volatile && ( Found->Flags == NON_VOL ) ) ||
          ( !Volatile && ( Found->Flags == VOL ) ) ) )
      {
        if( Found->Flags == NON_VOL ) {
          Print (L"%s: %hs already exists as a non-volatile variable\n", 
                 CmdName, Found->Name ) ;
        }
        else {
          Print (L"%s: %hs already exists as a volatile variable\n", 
                 CmdName, Found->Name ) ;
        }
        Found = NULL;
        Status = EFI_ACCESS_DENIED;
      }
      else
      {
        Status = RT->SetVariable (
          Found ? Found->Name : Name,
          Guid, 
          EFI_VARIABLE_BOOTSERVICE_ACCESS | ( Volatile ? 0 : EFI_VARIABLE_NON_VOLATILE ),
          StrSize(Value), 
          Value
          );

        if (!EFI_ERROR(Status)) {
          //
          // Make a new in memory copy
          //
          Size = sizeof(VARIABLE_ID) + StrSize(Name) + StrSize(Value);
          Var  = AllocateZeroPool (Size);

          Var->Signature = VARIABLE_SIGNATURE;
          Var->u.Value = ((UINT8 *) Var) + sizeof(VARIABLE_ID);
          Var->Name = (CHAR16*) (Var->u.Value + StrSize(Value));
          Var->ValueSize = StrSize(Value);
          StrCpy (Var->u.Str, Value);
          StrCpy (Var->Name, Found ? Found->Name : Name);

          if( Volatile ) {
            Var->Flags = VOL ; 
          }
          else {
            Var->Flags = NON_VOL ; 
          }

          InsertTailList (Head, &Var->Link);
        }
      }

    } else {

      if (Found) {
        Print(L"  %hs : %s\n", Var->Name, Var->u.Str);
      } else {
        Print(L"%s: %hs not found\n", CmdName, Name);
      }

      Found = NULL;
    }

    //
    // Remove the old in memory copy if there was one
    //
    if (Found) {
      RemoveEntryList (&Found->Link);
      FreePool (Found);
    }
  }

  ReleaseLock (&SEnvLock);
  return Status;
}


EFI_STATUS
SEnvCmdSet (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal shell "set" command

Arguments:

Returns:

--*/
{
  //
  // Process 'set' command
  //
  return SEnvCmdSA (ImageHandle, SystemTable, &SEnvEnv, &SEnvEnvId, L"set");
}


EFI_STATUS
SEnvCmdAlias (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal shell "alias" command

Arguments:

Returns:

--*/
{
  //
  // Process 'alias' command
  //
  return SEnvCmdSA (ImageHandle, SystemTable, 
                    &SEnvAlias, &SEnvAliasId, L"alias");
}
