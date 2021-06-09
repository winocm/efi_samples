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

  bcfg.c
  
Abstract:

  Shell command "bcfg"

  Boot time driver config

Revision History

--*/

#include "shell.h"

#define MAX_ENV_SIZE    1024


#define BCFG_NONE       0
#define BCFG_DUMP       1
#define BCFG_MOVE       2
#define BCFG_REMOVE     3
#define BCFG_ADD        4    
#define BCFG_USAGE      5


typedef struct {
  UINT32                    Attributes;
  UINT16                    FilePathListLength;
  CHAR16                    *Description;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  VOID                      *LoadOptions;
  UINTN                     LoadOptionsSize;
  CHAR16                    *FilePathStr;
} BCFG_LOAD_OPTION;

//
//
//

EFI_STATUS
InitializeBCfg (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );


VOID
DumpFileInfo (
  IN SHELL_FILE_ARG          *Arg
  );


VOID
BCfgDumpBootList (
  IN CHAR16       *BootOrder,
  IN CHAR16       *BootOption
  );

BCFG_LOAD_OPTION *
BCfgParseLoadOption (
  UINT8               *Data,
  UINTN               DataSize
  );

VOID
BCfgFreeLoadOption (
  BCFG_LOAD_OPTION    *Option
  );

VOID
BCfgSetOperation (
  UINTN               *OldOper,
  UINTN               NewOper
  );

VOID
BCfgUsage (
  VOID
  );

VOID
BCfgRemove (
  IN UINTN            Position
  );

VOID
BCfgMove (
  IN UINTN            Src,
  IN UINTN            Dest
  );

VOID
BCfgAdd (
  IN UINTN            Position,
  IN CHAR16           *File,
  IN CHAR16           *Desc
  );



//
//
//

BOOLEAN     BCfgVerbose = FALSE;

//
// Selected list
//

CHAR16      *BCfgSelOrder;
CHAR16      *BCfgSelOption;
CHAR16      *BCfgSelName;
UINT32      BCfgAttributes;
BOOLEAN     BCfgUsePath;
BOOLEAN     BCfgUseHandle;
BOOLEAN     BCfgUseHandleBlindly;
//
// Scratch memory
//

UINTN       BCfgOrderCount;
UINT16      *BCfgOrder;
UINT8       *BCfgData;

//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeBCfg)
#endif

EFI_STATUS
InitializeBCfg (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  CHAR16                  **Argv;
  UINTN                   Argc;
  EFI_STATUS              Status;
  UINTN                   Index, BufferSize;
  UINTN                   No1, No2;
  CHAR16                  *p, *File, *Desc;
  UINTN                   Oper;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeBCfg,
    L"bcfg",  // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif
  //
  // We are not being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  InitializeShellApplication (ImageHandle, SystemTable);
  Argv = SI->Argv;
  Argc = SI->Argc;

  BCfgVerbose = FALSE;
  BCfgSelName = NULL;
  BCfgSelOrder = NULL;
  BCfgOrderCount = 0;
  BCfgUsePath = FALSE;
  BCfgUseHandle = FALSE;

  No1 = 0;
  No2 = 0;
  File = NULL;
  Desc = NULL;

  BCfgOrder = AllocatePool(MAX_ENV_SIZE + 32);
  BCfgData  = AllocatePool(MAX_ENV_SIZE + 32);
    
  //
  // Scan args for flags
  //
  Oper = BCFG_NONE;
  for (Index = 1; Index < Argc; Index += 1) {
    p = Argv[Index];
    if (StriCmp(p, L"?") == 0) {
      BCfgSetOperation (&Oper, BCFG_USAGE);
    } else if (StriCmp(p, L"driver") == 0) {
      BCfgSelOrder = VarDriverOrder;
      BCfgSelOption = VarDriverOption;
      BCfgSelName = L"boot driver";
    } else if (StriCmp(p, L"boot") == 0) {
      BCfgSelOrder = VarBootOrder;
      BCfgSelOption = VarBootOption;
      BCfgSelName = L"boot option";
    } else if (StriCmp(p, L"dump") == 0) {
      BCfgSetOperation (&Oper, BCFG_DUMP);
    } else if (StriCmp(p, L"v") == 0) {
      BCfgVerbose = TRUE;
    } else if (StriCmp(p, L"rm") == 0) {
      Index += 1;
      if (Index < Argc) {
        No1 = xtoi(Argv[Index]);
      }

      BCfgSetOperation (&Oper, BCFG_REMOVE);

    } else if (StriCmp(p, L"mv") == 0) {
      Index += 1;
      if (Index < Argc) {
        No1 = xtoi(Argv[Index]);
      }

      Index += 1;
      if (Index < Argc) {
        No2 = xtoi(Argv[Index]);
      }

      BCfgSetOperation (&Oper, BCFG_MOVE);

    } else if (StriCmp(p, L"add") == 0 
            || StriCmp(p, L"addp") == 0
            || StriCmp(p, L"addh") == 0) {
      Index += 1;
      if (Index < Argc) {
        No1 = xtoi(Argv[Index]);
      }
      Index += 1;
      if (Index < Argc) {
        File = Argv[Index];
      }
      Index += 1;
      if (Index < Argc) {
        Desc = Argv[Index];
      }

      if (StriCmp(p, L"addp") == 0) {               
        BCfgUsePath = TRUE;
      } else if (StriCmp(p, L"addh") == 0) {
        BCfgUseHandle = TRUE;
    }
      
      BCfgSetOperation (&Oper, BCFG_ADD);

    } else {
      Print (L"bcfg: Unknown argument '%hs'\n", p);
      Oper = BCFG_USAGE;
      break;
    }
  }

  if (BCfgSelOrder) {
    //
    // Read the boot order var
    //
    BufferSize = MAX_ENV_SIZE;
    Status = RT->GetVariable (
          BCfgSelOrder, 
          &EfiGlobalVariable,
          &BCfgAttributes,
          &BufferSize,
          BCfgOrder
          );

    if (EFI_ERROR(Status)) {
      BufferSize = 0;
      BCfgAttributes = EFI_VARIABLE_NON_VOLATILE | 
                       EFI_VARIABLE_BOOTSERVICE_ACCESS;
      if (CompareMem(BCfgSelOrder,VarBootOrder, sizeof(VarBootOrder)) == 0) {
        BCfgAttributes = BCfgAttributes | EFI_VARIABLE_RUNTIME_ACCESS;
      }
    }

    BCfgOrderCount = BufferSize / sizeof(UINT16);
  }

  if (Oper == BCFG_NONE) {
    Oper = BCFG_USAGE;
  }
  
  if (Oper != BCFG_USAGE && !BCfgSelName) {
    Print (L"bcfg: \'Driver\' or \'boot\' should be supplied\n");
    Oper = BCFG_NONE;
  }

  switch (Oper) {
  case BCFG_NONE:
    break;

  case BCFG_USAGE:
    BCfgUsage();
    break;

  case BCFG_DUMP:
    Print (L"The %s list is:\n", BCfgSelName);
    BCfgDumpBootList (BCfgSelOrder, BCfgSelOption);
    break;

  case BCFG_ADD:
    BCfgAdd (No1, File, Desc);
    break;

  case BCFG_MOVE:
    BCfgMove (No1, No2);
    break;

  case BCFG_REMOVE:
    BCfgRemove (No1);
    break;
  }

  //
  // Done
  //

  if (BCfgOrder) {
    FreePool (BCfgOrder);
  }

  if (BCfgData) {
    FreePool (BCfgData);
  }

  return EFI_SUCCESS;
}

VOID
BCfgSetOperation (
  UINTN           *OldOper,
  UINTN           NewOper
  )
{
  //
  // 
  //
  if (*OldOper != BCFG_NONE && *OldOper != BCFG_USAGE) {
    Print (L"bcfg: Cannot specified multiple operation at a time\n");
    *OldOper = BCFG_USAGE;
  }
  *OldOper = NewOper;
}


VOID
BCfgUsage (
  VOID
  )
{
  //
  // help information
  //
  Print (L"bcfg driver|boot [dump [-v]][add # file \"desc\"][rm #] [mv # #]\n");
  Print (L"  driver  selects boot driver list\n");
  Print (L"  boot    selects boot option list\n");
  Print (L"  dump    dumps selected list\n");
  Print (L"  -v      dumps verbose (includes load options)\n");
  Print (L"  add     add 'file' with 'desc' at position #\n");
  Print (L"  addp    add 'file' with 'desc' at position #.Use hard drive path\n");
  Print (L"  addh    add 'handle' with 'desc' at position #.Use Handle\n");
  Print (L"  rm      remove #\n");
  Print (L"  mv      move # to #\n");
}


VOID
BCfgAdd (
  IN UINTN                  Position,
  IN CHAR16                 *File,
  IN CHAR16                 *Desc
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath, *FilePath, *FileNode, *DevPath;
  CHAR16                    *Str, *p;
  UINT8                     *p8;
  SHELL_FILE_ARG            *Arg;
  EFI_LIST_ENTRY            FileList;
  CHAR16                    OptionStr[40];
  UINTN                     DescSize, FilePathSize;
  BOOLEAN                   Found;
  UINTN                     Target, Index;
  EFI_HANDLE                *Handles;
  UINTN                     HandlesNo;
  EFI_HANDLE                CurHandle;
  UINTN                     CurHandleNo;
  UINTN                     DriverBindingHandleCount;
  EFI_HANDLE                *DriverBindingHandleBuffer;
  UINTN                     ParentControllerHandleCount;
  EFI_HANDLE                *ParentControllerHandleBuffer;
  UINTN                     ChildControllerHandleCount;
  EFI_HANDLE                *ChildControllerHandleBuffer;

  Str = NULL;
  FilePath = NULL;
  FileNode = NULL;
  InitializeListHead (&FileList);
  DriverBindingHandleBuffer = NULL;
  ParentControllerHandleBuffer = NULL;
  ChildControllerHandleBuffer = NULL;
  Handles = NULL;

  Status = EFI_SUCCESS;

  if (Position < 1) {
    Position = 1;
  }

  Position = Position - 1;

  if (Position > BCfgOrderCount) {
    Position = BCfgOrderCount;
  }

  if (!File || !Desc) {
    Print (L"bcfg: Missing argument for 'add' operation\n");
    Print (L"bcfg: Driver|boot add # file \"desc\"\n");
    goto Done;
  }


  if (BCfgUseHandle) {
    LibLocateHandle (AllHandles, NULL, NULL, &HandlesNo, &Handles);
    CurHandleNo = xtoi(File) - 1;
    if ((CurHandleNo < 0) || (CurHandleNo > HandlesNo - 1)) {
      Print(L"bcfg: Handle number out of range\n");
      goto Done;
    }
  CurHandle = Handles[CurHandleNo];

  //
  //Make sure that the handle should point to a real controller
  //
    Status = LibGetManagingDriverBindingHandles (
               CurHandle,
               &DriverBindingHandleCount,
               &DriverBindingHandleBuffer);

    Status = LibGetParentControllerHandles (
               CurHandle,
               &ParentControllerHandleCount,
               &ParentControllerHandleBuffer);

    Status = LibGetChildControllerHandles (
               CurHandle,
               &ChildControllerHandleCount,
               &ChildControllerHandleBuffer);

    if (DriverBindingHandleCount > 0 
          || ParentControllerHandleCount > 0 
          || ChildControllerHandleCount > 0) {
      FilePath = NULL;
      Status = BS->HandleProtocol (
                 CurHandle, 
                 &gEfiDevicePathProtocolGuid,
                 &FilePath);
    }
    if (EFI_ERROR (Status)) {
      Print (L"The Handle didn't contain a device path\n");
      goto Done;
    }
  }
  else {
    //
    // Get file info
    //
    ShellFileMetaArg (File, &FileList);

    //
    // If filename expanded to multiple names, fail
    //
    if (FileList.Flink->Flink != &FileList) {
      Print (L"bcfg: Too many source files\n");
      goto Done;
    }

    Arg = CR(FileList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status = Arg->Status;
    if (EFI_ERROR(Status)) {
      Print (L"bcfg: File %hs - %r\n", Arg->FileName, Status);
      goto Done;
    }

    //
    // Build FilePath to the filename
    //

    //
    // split full name at device string
    //
    for(p=Arg->FullName; *p && *p != ':'; p++) ;

    if (!*p) {
      Print (L"bcfg: Unsupported file name '%hs'\n", Arg->FullName);
      Status = EFI_UNSUPPORTED;
      goto Done;
    }

    //
    // get the device path 
    //
    *p = 0;
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap(Arg->FullName);
    if (!DevicePath) {
      Print (L"bcfg: No device path for %s\n", Arg->FullName);
      Status = EFI_UNSUPPORTED;
      goto Done;
    }

    if (BCfgUsePath) {
      DevPath = DevicePath;
      while (!IsDevicePathEnd(DevPath)) {
        if ((DevicePathType(DevPath) == MEDIA_DEVICE_PATH) &&
          (DevicePathSubType(DevPath) == MEDIA_HARDDRIVE_DP)) {

          //
          // If we find it use it in stead
          //
          DevicePath = DevPath;
          break;
        }
        DevPath = NextDevicePathNode(DevPath);      
      }
    }

    //
    // append the file 
    //
    FileNode = FileDevicePath(NULL, p+1);
    FilePath = AppendDevicePath(DevicePath, FileNode);
  }

  
  //
  // Find a free target ,a brute force implementation
  //
  Found = FALSE;
  for (Target=1; Target < 0xFFFF; Target += 1) {
    Found = TRUE;
    for (Index=0; Index < BCfgOrderCount; Index += 1) {
      if (BCfgOrder[Index] == Target) {
        Found = FALSE;
        break;
      }
    }

    if (Found) {
      break;
    }
  }

  if (Target == 0xFFFF) {
    Print (L"bcfg: Failed to find available variable name\n");
    goto Done;
  }

  Print (L"Target = %x\n", Target);

  //
  // Add the option
  //
  DescSize = StrSize(Desc);
  FilePathSize = DevicePathSize(FilePath);

  p8 = BCfgData;
  *((UINT32 *) p8) = LOAD_OPTION_ACTIVE;      // Attributes
  p8 += sizeof (UINT32);

  *((UINT16 *) p8) = (UINT16)FilePathSize;    // FilePathListLength
  p8 += sizeof (UINT16);

  CopyMem (p8, Desc, DescSize);
  p8 += DescSize;
  CopyMem (p8, FilePath, FilePathSize);

  SPrint (OptionStr, sizeof(OptionStr), BCfgSelOption, Target);
  Status = RT->SetVariable (
        OptionStr,
        &EfiGlobalVariable,
        BCfgAttributes,
        sizeof(UINT32) + sizeof(UINT16) + DescSize + FilePathSize,
        BCfgData
        );

  if (EFI_ERROR(Status)) {
    Print (L"bcfg: Failed to add %hs - %hr\n", OptionStr, Status);
    goto Done;
  }

  //
  // Insert target into order list
  //
  BCfgOrderCount += 1;
  for (Index=BCfgOrderCount-1; Index > Position; Index -= 1) {
    BCfgOrder[Index] = BCfgOrder[Index-1];
  }

  BCfgOrder[Position] = (UINT16) Target;
  Status = RT->SetVariable (
          BCfgSelOrder, 
          &EfiGlobalVariable, 
          BCfgAttributes,
          BCfgOrderCount * sizeof(UINT16),
          BCfgOrder
          );

  if (EFI_ERROR(Status)) {
    Print (L"bcfg: Failed to update %hs - %hr\n", BCfgSelOrder, Status);
    goto Done;
  }

  //
  // Done
  //
  Print (L"bcfg: Add %s as %x\n", BCfgSelName, Position+1);

Done:
  if (FileNode) {
    FreePool (FileNode);
  }

//
//If always Free FilePath, will free devicepath in system when use "addh"
//

  if (FilePath && !BCfgUseHandle) {
    FreePool (FilePath);
  }

  if (Str) {
    FreePool(Str);
  }

  if (DriverBindingHandleBuffer) {
    FreePool (DriverBindingHandleBuffer);
  }

  if (ParentControllerHandleBuffer) {
    FreePool (ParentControllerHandleBuffer);
  }

  if (ChildControllerHandleBuffer) {
    FreePool (ChildControllerHandleBuffer);
  }

  if (Handles) {
    FreePool (Handles);
  }

  ShellFreeFileList (&FileList);
}


VOID
BCfgRemove (
  IN UINTN            Position
  )
{
  CHAR16              OptionStr[40];
  EFI_STATUS          Status;
  UINTN               Index;
  UINT16              Target;


  if (Position < 1 || Position > BCfgOrderCount) {
    Print (L"bcfg: %hx not removed.  Value is out of range\n", Position);
    return ;
  }

  Target = BCfgOrder[Position-1];

  //
  // remove from order list
  //
  BCfgOrderCount = BCfgOrderCount - 1;
  for (Index=Position-1; Index < BCfgOrderCount; Index += 1) {
    BCfgOrder[Index] = BCfgOrder[Index+1];
  }

  LibDeleteVariable (BCfgSelOrder, &EfiGlobalVariable);

  Status = RT->SetVariable (
          BCfgSelOrder, 
          &EfiGlobalVariable, 
          BCfgAttributes,
          BCfgOrderCount * sizeof(UINT16),
          BCfgOrder
          );

  //
  // Remove the option
  //
  SPrint (OptionStr, sizeof(OptionStr), BCfgSelOption, Target);
  RT->SetVariable (OptionStr, &EfiGlobalVariable, BCfgAttributes, 0, NULL);

  //
  // Done
  //
  if (EFI_ERROR(Status)) {
    Print (L"bcfg: Failed to remove - %hr\n", Status);
  } else {
    Print (L"bcfg: %s %x removed\n", BCfgSelName, Position);
  }
}

VOID
BCfgMove (
  IN UINTN            Src,
  IN UINTN            Dest
  )
{
  UINT16              Target;
  UINTN               Index;
  EFI_STATUS          Status;

  if (Src < 1 || Src > BCfgOrderCount) {
    Print (L"bcfg: %hd not moved.  Value is out of range\n", Src);
    return ;
  }

  if (Dest < 1) {
    Dest = 1;
  }

  if (Dest > BCfgOrderCount) {
    Dest = BCfgOrderCount;
  }

  //
  //
  //
  Src = Src - 1;
  Dest = Dest - 1;
  Target = BCfgOrder[Src];

  //
  // Remove the item
  //
  for (Index=Src; Index < BCfgOrderCount-1; Index += 1) {
    BCfgOrder[Index] = BCfgOrder[Index+1];
  }

  //
  // Insert it
  //
  for (Index=BCfgOrderCount-1; Index > Dest; Index -= 1) {
    BCfgOrder[Index] = BCfgOrder[Index-1];
  }

  BCfgOrder[Dest] = Target;

  //
  // Update the order
  //
  Status = RT->SetVariable (
          BCfgSelOrder, 
          &EfiGlobalVariable, 
          BCfgAttributes,
          BCfgOrderCount * sizeof(UINT16),
          BCfgOrder
          );

  //
  // Done
  //
  if (EFI_ERROR(Status)) {
    Print (L"bcfg: Failed to move option - %hr\n", Status);
  } else {
    Print (L"bcfg: %s %x moved to %x\n", BCfgSelName, Src+1, Dest+1);
  }
}

VOID
BCfgDumpBootList (
  IN CHAR16           *BootOrder,
  IN CHAR16           *BootOption
  )
{
  EFI_STATUS          Status;
  UINTN               DataSize;
  UINT32              Attributes;
  CHAR16              OptionStr[40];
  BCFG_LOAD_OPTION    *Option;
  UINTN               Index;

  for (Index=0; Index < BCfgOrderCount; Index++) {
    SPrint (OptionStr, sizeof(OptionStr), BootOption, BCfgOrder[Index]);
    DataSize = MAX_ENV_SIZE;
    //
    // Call runtime service
    //
    Status = RT->GetVariable (
          OptionStr,
          &EfiGlobalVariable,
          &Attributes,
          &DataSize,
          BCfgData
          );

    //
    // Display
    //
    Print (L"%02x. ", Index+1);
    if (!EFI_ERROR(Status)) {

      Option = BCfgParseLoadOption ((UINT8 *) BCfgData, DataSize);
      if (!Option) {
        Print (L"%Hcould not parse option%N\n");
        continue;
      }

      Print (L"%s %H\"%ns\"%s%N\n", 
            Option->FilePathStr, 
            Option->Description, 
            Option->LoadOptionsSize ? L" OPT" : L""
            );

      BCfgFreeLoadOption (Option);

    } else {
      Print (L"%hr\n", Status);
    }
  }
}


BCFG_LOAD_OPTION *
BCfgParseLoadOption (
  UINT8                     *Data,
  UINTN                     DataSize
  )
{
  BCFG_LOAD_OPTION          *Option;
  BOOLEAN                   Valid;
  UINT8                     *End;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathNode;

  Valid = FALSE;
  Option = AllocateZeroPool(sizeof(BCFG_LOAD_OPTION));

  //
  // Parse the load option into the Option structure
  //
  if (DataSize < 10) {
    goto Done;
  }

  //
  // First 32 bits are the load option attributes
  //
  CopyMem (&Option->Attributes, Data, sizeof(UINT32));
  Data += sizeof(UINT32);
  DataSize -= sizeof(UINT32);

  //
  // Next comes the FilePathListLength
  //
  CopyMem (&Option->FilePathListLength, Data, sizeof(UINT16));
  Data += sizeof(UINT16);
  DataSize -= sizeof(UINT16);

  //
  // Next is a null terminated string
  //
  Option->Description = AllocatePool(DataSize);
  CopyMem (Option->Description, Data, DataSize);

  //
  // find the string terminator
  //
  Data = (UINT8 *) Option->Description;
  End = Data + DataSize;
  while (*((CHAR16 *) Data)) {
    if (Data > End - sizeof(CHAR16) - 1) {
      goto Done;
    }
    Data += sizeof(UINT16);
  }
  Data += sizeof(UINT16);
  DataSize = End - Data;

  //
  // Next is the file path
  //
  Option->FilePath = AllocatePool (DataSize);
  CopyMem (Option->FilePath, Data, DataSize);

  //
  // find the end of path terminator
  //
  DevicePathNode = (EFI_DEVICE_PATH_PROTOCOL *) Data;
  while (!IsDevicePathEnd (DevicePathNode)) {
    DevicePathNode = NextDevicePathNode (DevicePathNode);
    if ((UINT8 *) DevicePathNode > End - sizeof(EFI_DEVICE_PATH_PROTOCOL)) {
      goto Done;
    }
  }

//    Data = ((UINT8 *) DevicePathNode) + sizeof(EFI_DEVICE_PATH_PROTOCOL);
  Data += Option->FilePathListLength; 
  if (Data > End) {
    goto Done;
  }

  DataSize = End - Data;

  //
  // Next is the load options
  //
  if (DataSize) {
    Option->LoadOptions = Data;
    Option->LoadOptionsSize = DataSize;
  }

  //
  // Expand the FilePath to a string
  //
  Option->FilePathStr = DevicePathToStr(Option->FilePath);
  Valid = TRUE;

Done:
  if (!Valid && Option) {
    BCfgFreeLoadOption (Option);
    Option = NULL;
  }

  return Option;
}


VOID
BCfgFreeLoadOption (
  BCFG_LOAD_OPTION    *Option
  )
{
  //
  // release resources
  //
  
  if (Option->Description) {
    FreePool (Option->Description);
  }

  if (Option->FilePath) {
    FreePool (Option->FilePath);
  }

  if (Option->FilePathStr) {
    FreePool (Option->FilePathStr);
  }

  FreePool (Option);
}
