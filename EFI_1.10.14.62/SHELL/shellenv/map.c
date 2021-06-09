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

  map.c
  
Abstract:

  Shell environment short device name mapping information management

Revision History

--*/

#include "shelle.h"

//
// Remember current mapping
//
extern EFI_LIST_ENTRY SEnvCurMapping;

extern EFI_LIST_ENTRY SEnvMap;

STATIC CHAR16 *SEnvCurDevice;

BOOLEAN MappingModified = FALSE;

STATIC 
BOOLEAN 
IsValidName (
  IN CHAR16           *Name
  );

//
//
//
VOID
SEnvInitMap (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // The mapping data is read in from the variable init.
  //

  //
  // Init the default map device
  //
  SEnvCurDevice = StrDuplicate(L"none");
}


CHAR16 *
SEnvGetDefaultMapping (
  IN EFI_HANDLE           ImageHandle
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
  EFI_STATUS                Status;
  EFI_LIST_ENTRY            *Head;
  EFI_LIST_ENTRY            *Link;
  VARIABLE_ID               *Var;
  EFI_HANDLE                  Handle;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;

  Status = BS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, 
                               (VOID*)&LoadedImage);
  
  if (EFI_ERROR(Status) || LoadedImage==NULL) {
    return NULL;
  }

  //
  // Walk through the SEnvMap linked list to get correspond mapping
  //
  Head = &SEnvMap;
  for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)Var->u.Str;
    Status = BS->LocateDevicePath (&gEfiDevicePathProtocolGuid,
                                   &DevicePath,&Handle);
    if (!EFI_ERROR(Status) && Handle!=NULL) {
      if (LoadedImage->DeviceHandle == Handle) {
        return(Var->Name);
      }
    }
  }

  return NULL;
}


VOID
SEnvDumpMapping(
  IN UINTN            SLen,
  IN BOOLEAN          Verbose,
  IN VARIABLE_ID      *Var
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  CHAR16                      *Ptr;
  EFI_DEVICE_PATH_PROTOCOL    *DPath;
  EFI_STATUS                  Status;
  EFI_HANDLE                  DeviceHandle;

  Ptr = DevicePathToStr ((EFI_DEVICE_PATH_PROTOCOL *) Var->u.Str);
  Print (L"  %h-.*s : %s\n", SLen, Var->Name, Ptr);

  if (Verbose) {
    
    //
    // lookup handle for this mapping
    //
    DPath = (EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value;
    Status = BS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &DPath, 
                                   &DeviceHandle);
    if (EFI_ERROR(Status)) {
      Print (L"%*s= Handle for this mapping not found\n", SLen+3);
    } else {
      Print (L"%*s= Handle", SLen + 3, L"");
      SEnvDHProt (FALSE, FALSE, 0, DeviceHandle, "eng");
    }

    //
    // print current directory for this mapping
    //
    Print (L"%*s> %s\n\n", SLen+3, L"", Var->CurDir ? Var->CurDir : L"\\");
  }
  
  FreePool (Ptr);
}


EFI_STATUS
SEnvCmdMap (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "map" command

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY              *Link;
  EFI_LIST_ENTRY              *Head;
  VARIABLE_ID                 *Var;
  VARIABLE_ID                 *Found;
  CHAR16                      *Name;
  CHAR16                      *Value;    
  UINTN                       SLen;
  UINTN                       Len;
  UINTN                       Size;
  UINTN                       DataSize;
  BOOLEAN                     Delete;
  BOOLEAN                     Verbose;
  BOOLEAN                     Remap;
  EFI_STATUS                  Status;
  UINTN                       Index;
  CHAR16                      *Ptr;
  EFI_HANDLE                  Handle;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;

  InitializeShellApplication (ImageHandle, SystemTable);
  Head = &SEnvMap;

  //
  // Initialize variables to avoid a level 4 warning
  //
  Var = NULL;

  Name = NULL;
  Value = NULL;
  Delete = FALSE;
  Verbose = FALSE;
  Remap = FALSE;
  Status = EFI_SUCCESS;
  Found = NULL;

  //
  // Crack arguments
  //
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'd':
      case 'D':
        Delete = TRUE;
        break;

      case 'v':
      case 'V':
        Verbose = TRUE;
        break;

      case 'r':
      case 'R':
        Remap = TRUE;
        break;

      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;
          
      default:
        Print (L"map: Unknown flag %hs\n", Ptr);
        return EFI_INVALID_PARAMETER;
      }
      continue;
    }

    if (!Name) {
      Name = Ptr;
      if (! IsValidName (Name)) {
        Print (L"map: Invalid sname %hs\n", Name);
        return EFI_INVALID_PARAMETER;
      }
      continue;
    }

    if (!Value) {
      Value = Ptr;
      continue;
    }

    Print (L"map: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  if (Delete && Value) {
    Print (L"map: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  //
  // Process
  //
  if (Remap && !Value && !Delete) {
    Status = SEnvReloadDefaults (ImageHandle, SystemTable, &MappingModified);
    Remap = FALSE;
  }

  if (Value || Verbose) {
    SEnvLoadHandleTable ();

    if (Verbose) {
      SEnvLoadHandleProtocolInfo (&gEfiDevicePathProtocolGuid);
    }
  }

  AcquireLock (&SEnvLock);

  SLen = 0;
  for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    Len = StrLen(Var->Name);
    if (Len > SLen) {
      SLen = Len;
    }
  }

  if (!Name) {
    Print (L"%EDevice mapping table%N\n");
    for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      SEnvDumpMapping(SLen, Verbose, Var);
    }

  } else {
    //
    // Find the specified value
    //
    for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      if (StriCmp(Var->Name, Name) == 0) {
        Found = Var;
        break;
      }
    }

    if (Found && Delete) {
      //
      // Remove it from the store
      //
      Status = RT->SetVariable (Found->Name, &SEnvMapId, 0, 0, NULL);
      MappingModified = TRUE;
    } else if (Value) {
      //
      // Find the handle in question
      //
      Handle = SEnvHandleFromStr(Value);
      if (!Handle) {
        Print (L"map: Handle %hd not found\n", Handle);
        Status = EFI_NOT_FOUND;   
        goto Done;
      }

      //
      // Get the handle's device path
      //
      DevicePath = DevicePathFromHandle(Handle);
      if (!DevicePath) {
        Print (L"map: Device path of handle %hd not found\n", Handle);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      DataSize = DevicePathSize(DevicePath);

      //
      // Add it to the store
      //
      Status = RT->SetVariable (
              Found ? Found->Name : Name, 
              &SEnvMapId,
              EFI_VARIABLE_BOOTSERVICE_ACCESS,
              DataSize,
              DevicePath
              );
      
      MappingModified = TRUE;

      if (!EFI_ERROR(Status)) {
        //
        // Make a new in memory copy
        //
        Size = sizeof(VARIABLE_ID) + StrSize(Name) + DataSize;
        Var  = AllocateZeroPool (Size);

        Var->Signature = VARIABLE_SIGNATURE;
        Var->u.Value = ((UINT8 *) Var) + sizeof(VARIABLE_ID);
        Var->Name = (CHAR16*) (Var->u.Value + DataSize);
        Var->ValueSize = DataSize;
        CopyMem (Var->u.Value, DevicePath, DataSize);
        StrCpy (Var->Name, Found ? Found->Name : Name);
        InsertTailList (Head, &Var->Link);
      }
    } else {
      if (Found) {
        SEnvDumpMapping(SLen, Verbose, Var);
      } else {
        Print (L"map: Cannot find handle %hs \n", Name);
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

Done:
  ReleaseLock (&SEnvLock);
  SEnvFreeHandleTable ();
  
  //
  // Get current mapping info for whole system
  //
  SEnvGetCurMappings ();
  
  return Status;
}


VARIABLE_ID *
SEnvMapDeviceFromName (
  IN OUT CHAR16   **pPath
  )
/*++

Routine Description:

  Check the Path for a device name, and updates the path to point after
  the device name.  If no device name is found, the current default is used.

Arguments:

Returns:

--*/
{
  CHAR16          *Path;
  CHAR16          *Ptr;
  CHAR16          *MappedName;
  CHAR16          c;
  VARIABLE_ID     *Var;
  EFI_LIST_ENTRY  *Link;
  EFI_LIST_ENTRY  *Link1;

  MAPPING_INFO    *MappingInfo;
  BOOLEAN         Found;
  
  ASSERT_LOCKED (&SEnvLock);

  Var = NULL;
  Path = *pPath;

  //
  // Check for a device name terminator
  //
  for (Ptr = Path; *Ptr && *Ptr != ':' && *Ptr != '\\'; Ptr++) {
    ;
  }

  //
  // Move Ptr to the last ':' in contiguous ':'s
  //
  while (*Ptr == ':' && *(Ptr+1) == ':') {
    Ptr ++;
  }
  
  //
  // Use either the passed in name or the current device name setting
  //
  MappedName = *Ptr == ':' ? Path : SEnvCurDevice;
  
  //
  // Null terminate the string in Path just in case that is the one we 
  // are using
  //
  c = *Ptr;
  *Ptr = 0;
  
  //
  // Find current mapping
  //
  Found = FALSE;
  MappingInfo = NULL;
  for (Link = SEnvCurMapping.Flink; Link != &SEnvCurMapping; Link = Link->Flink) {
    MappingInfo = CR (Link, MAPPING_INFO, Link, MAPPING_INFO_SIGNATURE);
    if (StriCmp (MappingInfo->MappedName, MappedName) == 0) {
      Found = TRUE;
      break;
    }
  }
  
  if (!Found) {
    return NULL;
  }
  
  //
  // Ignore mapping info checking for the handle that only contains filesystem
  // protocol but no blkio on it.
  //
  if (MappingInfo->MediaId != 0xFFFFFFFF) {
    SEnvCheckValidMappings (MappingInfo);
  }

  //
  // Find the mapping for the device
  //
  for (Link=SEnvMap.Flink; Link != &SEnvMap; Link=Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp(Var->Name, MappedName) == 0) {
      for (Link1 = SEnvCurMapping.Flink; Link1 != &SEnvCurMapping; Link1 = Link1->Flink) {
        MappingInfo = CR (Link1, MAPPING_INFO, Link, MAPPING_INFO_SIGNATURE);
        if (StriCmp (MappingInfo->MappedName, MappedName) == 0) {
          MappingInfo->Accessed = TRUE;
          if (!MappingInfo->Valid) {
            return NULL;
          }
          break;
        }
      }
      break;
    }
  }

  //
  // Restore the path 
  //
  *Ptr = c;

  //
  // If the mapped device was not found, return NULL
  //
  if (Link == &SEnvMap) {
    DEBUG((EFI_D_VARIABLE, "SEnvNameToPath: Mapping for '%es' not found\n", 
          Path));
    return NULL;
  }

  //
  // If we found it as part of the path, skip the path over it
  //
  if (MappedName == Path) {
    *pPath = Ptr + 1;
  }

  //
  // Return the target mapping
  //
  return Var;
}


EFI_DEVICE_PATH_PROTOCOL *
SEnvIFileNameToPath (
  IN CHAR16               *Path
  )
/*++

Routine Description:

  Builds a device path from the filename string.  Note that the
  device name must already be stripped off of the file name string

Arguments:

Returns:

--*/
{
  CHAR16                      *LPath, *ps;
  BOOLEAN                     UseLPath;
  EFI_DEVICE_PATH_PROTOCOL    *DPath, *Node, *NewPath;
  CHAR16                      Buffer[MAX_ARG_LENGTH];
  UINTN                       Index;

  ASSERT_LOCKED (&SEnvLock);

  DPath = NULL;

  //
  // If no path, return the root
  //
  if (!*Path) {
    DPath = FileDevicePath(NULL, L"\\");
  }

  //
  // Build a file path for the name component(s)
  //
  while (*Path) {
    Index = 0;
    LPath = NULL;
    UseLPath = FALSE;

    ps = Path;
    while (*ps) {
      //
      // if buffer has run out, just handle to LPath
      //
//    if (Index > MAX_ARG_LENGTH-2  || *ps == '#') {
      if (Index > MAX_ARG_LENGTH-2 ) {
        UseLPath = TRUE;
        break;
      }

      /*
      if (*ps == '^') {
        if (ps[1]) {
          ps += 1;
          Buffer[Index++] = *ps;
        }
        ps += 1;
        continue;
      }
      */

      if (*ps == '\\') {
        LPath = ps;
      }

      Buffer[Index++] = *ps;
      ps += 1;
    }

    if (UseLPath) {
      Index = LPath ? LPath - Path : 0;
      ps = Path + Index;
    }

    //
    // If we have part of a path name, append it to the device path
    //
    if (Index) {
      Buffer[Index] = 0;
      Node = FileDevicePath(NULL, Buffer);
      NewPath = AppendDevicePath (DPath, Node);
      FreePool (Node);
      if (DPath) {
        FreePool (DPath);
      }
      DPath = NewPath;
    }

    if (*ps == 0) {
      break;
    }

    Path = ps + 1;
  }

  return DPath;
}


EFI_DEVICE_PATH_PROTOCOL *
SEnvFileNameToPath (
  IN CHAR16               *Path
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL         *FilePath;

  AcquireLock (&SEnvLock);
  FilePath = SEnvIFileNameToPath (Path);
  ReleaseLock (&SEnvLock);
  return FilePath;
}


EFI_DEVICE_PATH_PROTOCOL *
SEnvINameToPath (
  IN CHAR16               *Path
  )
/*++

Routine Description:

  Convert a file system style name to an file path    

Arguments:

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL    *DPath, *FPath, *RPath, *FilePath;
  VARIABLE_ID                 *Var;
  BOOLEAN                     FreeDPath;
  
  DPath = NULL;
  RPath = NULL;
  FPath = NULL;
  FilePath = NULL;
  FreeDPath = FALSE;

  ASSERT_LOCKED (&SEnvLock);

  //
  // Get the device for the name, and advance past the device name
  //
  Var = SEnvMapDeviceFromName (&Path);
  if (!Var) {
    DEBUG((EFI_D_VARIABLE, "SEnvNameToPath: mapped device not found\n"));
    goto Done;
  }

  //
  // Start the file path with this mapping
  //
  DPath = (EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value;

  //
  // If the path is relative, append the current dir of the device to the dpath
  //
  if (*Path != '\\') {
    RPath = SEnvIFileNameToPath (Var->CurDir ? Var->CurDir : L"\\");
    DPath = AppendDevicePath (DPath, RPath);
    FreeDPath = TRUE;
  }
  
  //
  // Build a file path for the rest of the name string
  //
  FPath = SEnvIFileNameToPath (Path);

  //
  // Append the 2 paths
  //
  FilePath = AppendDevicePath(DPath, FPath);

Done:
  if (DPath && FreeDPath) {
    FreePool (DPath);
  }

  if (RPath) {
    FreePool (RPath);
  }

  if (FPath) {
    FreePool (FPath);
  }

  return FilePath;
}


EFI_DEVICE_PATH_PROTOCOL *
SEnvNameToPath (
  IN CHAR16                   *Path
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL    *DPath;

  AcquireLock (&SEnvLock);
  DPath = SEnvINameToPath (Path);
  ReleaseLock (&SEnvLock);

  return DPath;
}


EFI_STATUS
SEnvCmdCd (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL    *FilePath;
  EFI_STATUS                  Status;
  EFI_FILE_HANDLE             OpenDir;
  CHAR16                      *Dir;
  CHAR16                      *CurDir;
  VARIABLE_ID                 *Var;
  EFI_FILE_INFO               *FileInfo;


  InitializeShellApplication (ImageHandle, SystemTable);
  FilePath = NULL;

  //
  // If no arguments, print the current directory
  //
  if (SI->Argc == 1) {
    Dir = SEnvGetCurDir(NULL);
    if (Dir) {
      Print (L"%s\n", Dir);
      FreePool (Dir);
      return  EFI_SUCCESS;
    } else {
      Print (L"cd: Current directory not specified\n");
      return EFI_NOT_FOUND;
    }
  }

  AcquireLock (&SEnvLock);

  //
  // If more then 1 argument, syntax
  //
  if (SI->Argc > 2) {
    Print (L"cd: Too many arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Find the target device
  //
  Dir = SI->Argv[1];
  Var = SEnvMapDeviceFromName (&Dir);
  if (!Var) {
    Print (L"cd: Cannot find mapped device\n");
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // If there's no path specified, print the current path for the device
  //
  if (*Dir == 0) {
    Print (L"%s\n", Var->CurDir ? Var->CurDir : L"\\");
    Status = EFI_SUCCESS;
    goto Done;
  }

  //
  // Build a file path for the argument
  //
  FilePath = SEnvINameToPath (SI->Argv[1]);
  if (!FilePath) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // Open the target directory
  //
  OpenDir = ShellOpenFilePath(FilePath, EFI_FILE_MODE_READ);

  if (!OpenDir) {
    Print (L"cd: Target directory not found\n");
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // Get information on the file path that was opened.
  //
  FileInfo = LibFileInfo(OpenDir);
  if (FileInfo == NULL) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // Verify that the file opened is a directory.
  //
  if (!(FileInfo->Attribute & EFI_FILE_DIRECTORY)) {
    Print (L"cd: Target is not a directory\n");
    FreePool (FileInfo);
    OpenDir->Close (OpenDir);
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  FreePool (FileInfo);

  CurDir = SEnvFileHandleToFileName(OpenDir);
  OpenDir->Close (OpenDir);
  
  //
  // If we have a new path, update the device
  //
  if (CurDir) {
    if (Var->CurDir) {
      FreePool(Var->CurDir);
    }
    Var->CurDir = CurDir;

  } else {

    Print (L"cd: Cannot change directory\n");

  }

  Status = EFI_SUCCESS;

Done:
  ReleaseLock (&SEnvLock);

  if (FilePath) {
    FreePool (FilePath);
  }

  return Status;
}


CHAR16 *
SEnvGetCurDir (
  IN CHAR16       *DeviceName OPTIONAL    
  )
/*++

Routine Description:

  N.B. results are allocated in pool

Arguments:

Returns:

--*/
{
  CHAR16          *Dir;
  EFI_LIST_ENTRY  *Link;
  VARIABLE_ID     *Var;

  Dir = NULL;
  if (!DeviceName) {
    DeviceName = SEnvCurDevice;
  }

  //
  // Walk through SEnvMap linked list to get acquired device name
  //
  AcquireLock (&SEnvLock);
  for (Link=SEnvMap.Flink; Link != &SEnvMap; Link=Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp(Var->Name, DeviceName) == 0) {
      Dir = PoolPrint (L"%s:%s", Var->Name, Var->CurDir ? Var->CurDir : L"\\");
      break;
    }
  }

  ReleaseLock (&SEnvLock);
  return Dir;
}


EFI_STATUS
SEnvSetCurrentDevice (
  IN CHAR16       *Name
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  VARIABLE_ID     *Var;
  EFI_LIST_ENTRY  *Link;
  EFI_STATUS      Status;
  UINTN           Len;
  CHAR16          *NewName;
  CHAR16          c;


  Len = StrLen(Name);
  if (Len < 1) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If the name ends with a ":" strip it off
  //
  Len -= 1;
  c = Name[Len];
  if (c == ':') {
    Name[Len] = 0;
  }

  Status = EFI_NO_MAPPING;
  AcquireLock (&SEnvLock);

  for (Link=SEnvMap.Flink; Link != &SEnvMap; Link=Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp(Var->Name, Name) == 0) {
      NewName = StrDuplicate(Name);
      if (NewName) {
        FreePool (SEnvCurDevice);
        SEnvCurDevice = NewName;
      }
      Status = EFI_SUCCESS;
      break;
    }
  }

  ReleaseLock (&SEnvLock);

  //
  // Restore the name
  //
  Name[Len] = c;
  return Status;
}


VOID
SEnvGetCurMappings (
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY                  *Link;
  MAPPING_INFO                    *MappingInfo;
  EFI_STATUS                      Status;

  UINTN                           Index;

  UINTN                           NoHandles;
  EFI_HANDLE                      *Handles;
  
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *MappedDevicePath;
  UINTN                           DevPathSize;
  EFI_BLOCK_IO_PROTOCOL           *BlkIo;

  VARIABLE_ID                     *Var;
  
  //
  // Remove previous mapping info
  //
  MappingInfo = NULL;
  
  while (!IsListEmpty (&SEnvCurMapping)) {
    MappingInfo = CR (SEnvCurMapping.Flink, MAPPING_INFO, 
                      Link, MAPPING_INFO_SIGNATURE);
    FreePool (MappingInfo->MappedName);
    FreePool (MappingInfo->DevicePath);
    RemoveEntryList (&MappingInfo->Link);
    FreePool (MappingInfo);
  }
  
  Status = LibLocateHandle (
                            ByProtocol, 
                            &gEfiSimpleFileSystemProtocolGuid, 
                            NULL, 
                            &NoHandles, 
                            &Handles
                            );
                            
  if (!NoHandles) {
    return;
  }
  
  for (Index = 0; Index < NoHandles; Index++) {
    DevicePath = DevicePathFromHandle (Handles[Index]);
    if (!DevicePath) {
      continue;
    }
    
    DevPathSize = DevicePathSize (DevicePath);

    //
    // Find the mapping for the device
    //
    for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      MappedDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap(Var->Name);
      if (CompareMem (DevicePath, MappedDevicePath, DevPathSize) == 0) {
        MappingInfo = AllocateZeroPool (sizeof (MAPPING_INFO));
        MappingInfo->DevicePath = AllocateZeroPool (DevPathSize);
        CopyMem (MappingInfo->DevicePath, DevicePath, DevPathSize);
        MappingInfo->MappedName = AllocateZeroPool (sizeof (CHAR16) * (StrLen (Var->Name) + 1));
        StrCpy (MappingInfo->MappedName, Var->Name);
        Status = BS->HandleProtocol (Handles[Index], &gEfiBlockIoProtocolGuid, 
                                     (VOID **)&BlkIo);
        if (!EFI_ERROR(Status)) {
          MappingInfo->MediaId = BlkIo->Media->MediaId;
        } else {
          //
          // In most cases, the handle that contains filesystem protocol also
          // have blkio protocol on it. But there will be another case that
          // the handle that contains filesystem protocol does not have
          // blkio protocol on it. Here just assign MappingInfo->MediaId to
          // 0xFFFFFFFF for this situation.
          //
          MappingInfo->MediaId = 0xFFFFFFFF;
        }
        MappingInfo->Valid = TRUE;
        MappingInfo->Accessed = FALSE;
        MappingInfo->Signature = MAPPING_INFO_SIGNATURE;
        InsertTailList (&SEnvCurMapping, &MappingInfo->Link);
      }
    }
  }
  FreePool (Handles);
}


VOID
SEnvCheckValidMappings (
  MAPPING_INFO *CurMappingInfo
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY                  *Link;
  MAPPING_INFO                    *MappingInfo;
  EFI_STATUS                      Status;

  UINTN                           Index;

  UINTN                           NoHandles;
  EFI_HANDLE                      *Handles;

  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  UINTN                           DevPathSize;
  EFI_BLOCK_IO_PROTOCOL           *BlkIo;

  DevPathSize = DevicePathSize (CurMappingInfo->DevicePath);
  
  //
  // Set current mapping info to be invalid
  //
  for (Link = SEnvCurMapping.Flink; Link != &SEnvCurMapping; Link = Link->Flink) {
    MappingInfo = CR (Link, MAPPING_INFO, Link, MAPPING_INFO_SIGNATURE);
    if (CompareMem (CurMappingInfo->DevicePath, MappingInfo->DevicePath, DevPathSize) == 0) {
      MappingInfo->Valid = FALSE;
    }
  }
  
  Status = LibLocateHandle (
                            ByProtocol, 
                            //&gEfiSimpleFileSystemProtocolGuid, 
                            &gEfiBlockIoProtocolGuid,
                            NULL, 
                            &NoHandles, 
                            &Handles
                            );
                            
  if (!NoHandles) {
    return;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    Status = BS->HandleProtocol (Handles[Index], &gEfiBlockIoProtocolGuid, 
                                 (VOID **)&BlkIo);
    if (!EFI_ERROR (Status)) {
      DevicePath = DevicePathFromHandle (Handles[Index]);
      DevPathSize = DevicePathSize (CurMappingInfo->DevicePath);
      //
      // We only check media that has no logical partition on it,
      // which means floppy disk, etc.
      //
      if (!BlkIo->Media->LogicalPartition) {
        VOID *Buffer;
        //
        // We only check the block represent current mapping
        //
        if (CompareMem (CurMappingInfo->DevicePath, DevicePath, DevPathSize) == 0) {
          UINT32 MediaId;
          BOOLEAN MediaPresent;
          Buffer = AllocatePool (BlkIo->Media->BlockSize);
          MediaId = BlkIo->Media->MediaId;
          MediaPresent = BlkIo->Media->MediaPresent;
          BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 0, 
                             BlkIo->Media->BlockSize, Buffer);
          FreePool (Buffer);
          if (MediaPresent != BlkIo->Media->MediaPresent || MediaId != BlkIo->Media->MediaId) {
            if (BlkIo->Media->MediaPresent) {
              BS->ReinstallProtocolInterface (Handles[Index], 
                                              &gEfiBlockIoProtocolGuid, 
                                              BlkIo, BlkIo);
            }
          }
        }
      }
      //
      // Set corresponding mapping info to be valid
      //
      for (Link = SEnvCurMapping.Flink; Link != &SEnvCurMapping; Link = Link->Flink) {
        MappingInfo = CR (Link, MAPPING_INFO, Link, MAPPING_INFO_SIGNATURE);
        if (CompareMem (MappingInfo->DevicePath, DevicePath, DevPathSize) == 0) {
          MappingInfo->Valid = TRUE;
        }
      }
    }
  }
  FreePool (Handles);
}

STATIC 
BOOLEAN 
IsValidName (
  IN CHAR16           *Name
  )
/*++

Routine Description:
  
  To check if the Name is valid name for mapping.
  
Arguments:

  Name    - the name
  
Returns:
  TRUE    - the name is valid
  FALSE   - the name is invalid

--*/
{
  CHAR16  *Ptr;

  //
  // forbid special chars inside name
  //  
  for (Ptr = Name; *Ptr; Ptr += 1) {
    if (*Ptr < 0x20  ||
        *Ptr == '\"' ||
        *Ptr == '*'  ||
        *Ptr == '/'  ||
        *Ptr == ':'  ||
        *Ptr == '<'  ||
        *Ptr == '>'  ||
        *Ptr == '?'  ||
        *Ptr == '\\' ||
        *Ptr == '|' ) {
      return FALSE;
    }
  }
  return TRUE;
}
