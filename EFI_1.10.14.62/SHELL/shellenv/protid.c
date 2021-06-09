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

  protid.c

Abstract:

  Shell environment protocol id information management



Revision History

--*/

#include "shelle.h"
#include "intload.h"

extern EFI_LIST_ENTRY SEnvOrgFsDevicePaths;
extern EFI_LIST_ENTRY SEnvCurFsDevicePaths;
extern EFI_LIST_ENTRY SEnvOrgBlkDevicePaths;
extern EFI_LIST_ENTRY SEnvCurBlkDevicePaths;
extern EFI_LIST_ENTRY SEnvCurMapping;

#define PROTOCOL_INFO_SIGNATURE EFI_SIGNATURE_32('s','p','i','n')

typedef struct {
  UINTN                       Signature;
  EFI_LIST_ENTRY              Link;

  //
  // parsing info for the protocol
  //
  EFI_GUID                    ProtocolId;
  CHAR16                      *IdString;
  SHELLENV_DUMP_PROTOCOL_INFO DumpToken;
  SHELLENV_DUMP_PROTOCOL_INFO DumpInfo;

  //
  // database info on which handles are supporting this protocol
  //
  UINTN                       NoHandles;
  EFI_HANDLE                  *Handles;

} PROTOCOL_INFO;

struct {
  CHAR16                      *IdString;
  SHELLENV_DUMP_PROTOCOL_INFO DumpInfo;
  SHELLENV_DUMP_PROTOCOL_INFO DumpToken;
  EFI_GUID                    ProtocolId;
} SEnvInternalProtocolInfo[] = {
  L"DevIo",               NULL,                 NULL,           EFI_DEVICE_IO_PROTOCOL_GUID,
  L"Fs",                  NULL,                 NULL,           EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
  L"DiskIo",              NULL,                 NULL,           EFI_DISK_IO_PROTOCOL_GUID,
  L"BlkIo",               SEnvBlkIo,            NULL,           EFI_BLOCK_IO_PROTOCOL_GUID,
  L"Txtin",               NULL,                 NULL,           EFI_SIMPLE_TEXT_IN_PROTOCOL_GUID,
  L"Txtout",              SEnvTextOut,          NULL,           EFI_SIMPLE_TEXT_OUT_PROTOCOL_GUID,
  L"Load",                NULL,                 NULL,           LOAD_FILE_PROTOCOL_GUID,
  L"Image",               SEnvImage,            SEnvImageTok,   EFI_LOADED_IMAGE_PROTOCOL_GUID,
  L"Varstore",            NULL,                 NULL,           EFI_VARIABLE_STORE_PROTOCOL_GUID,
  L"UnicodeCollation",    NULL,                 NULL,           EFI_UNICODE_COLLATION_PROTOCOL_GUID,
  L"LegacyBoot",          NULL,                 NULL,           EFI_LEGACY_BOOT_PROTOCOL_GUID,
  L"SerialIo",            NULL,                 NULL,           EFI_SERIAL_IO_PROTOCOL_GUID,
  L"Pxebc",               NULL,                 NULL,           EFI_PXE_BASE_CODE_PROTOCOL_GUID,
  L"Net",                 NULL,                 NULL,           EFI_SIMPLE_NETWORK_PROTOCOL,
  L"ShellInt",            NULL,                 NULL,           SHELL_INTERFACE_PROTOCOL,
  L"SEnv",                NULL,                 NULL,           ENVIRONMENT_VARIABLE_ID,
  L"ShellProtId",         NULL,                 NULL,           PROTOCOL_ID_ID,
  L"ShellDevPathMap",     NULL,                 NULL,           DEVICE_PATH_MAPPING_ID,
  L"ShellAlias",          NULL,                 NULL,           ALIAS_ID,
  L"G0",                  NULL,                 NULL,           { 0,0,0,0,0,0,0,0,0,0,0 },
  L"Efi",                 NULL,                 NULL,           EFI_GLOBAL_VARIABLE_GUID,
  L"GenFileInfo",         NULL,                 NULL,           EFI_FILE_INFO_ID,
  L"FileSysInfo",         NULL,                 NULL,           EFI_FILE_SYSTEM_INFO_ID_GUID,
  L"PcAnsi",              NULL,                 NULL,           DEVICE_PATH_MESSAGING_PC_ANSI,
  L"Vt100",               NULL,                 NULL,           DEVICE_PATH_MESSAGING_VT_100,
  L"Vt100+",              NULL,                 NULL,           DEVICE_PATH_MESSAGING_VT_100_PLUS,
  L"VtUtf8",              NULL,                 NULL,           DEVICE_PATH_MESSAGING_VT_UTF8,
  L"ESP",                 NULL,                 NULL,           EFI_PART_TYPE_EFI_SYSTEM_PART_GUID,
  L"GPT MBR",             NULL,                 NULL,           EFI_PART_TYPE_LEGACY_MBR_GUID,
  L"BIS",                 NULL,                 NULL,           EFI_BIS_PROTOCOL_GUID,
  L"DriverBinding",       NULL,                 NULL,           EFI_DRIVER_BINDING_PROTOCOL_GUID,
  L"ComponentName",       NULL,                 NULL,           EFI_COMPONENT_NAME_PROTOCOL_GUID,
  L"Configuration",       NULL,                 NULL,           EFI_DRIVER_CONFIGURATION_PROTOCOL_GUID,
  L"Diagnostics",         NULL,                 NULL,           EFI_DRIVER_DIAGNOSTICS_PROTOCOL_GUID,
  L"PciRootBridgeIo",     SEnvPciRootBridgeIo,  NULL,           EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID,
  L"PciIo",               SEnvPciIo,            NULL,           EFI_PCI_IO_PROTOCOL_GUID,
  L"SimplePointer",       NULL,                 NULL,           EFI_SIMPLE_POINTER_PROTOCOL_GUID,
  L"ConIn",               NULL,                 NULL,           EFI_CONSOLE_IN_DEVICE_GUID,
  L"ConOut",              NULL,                 NULL,           EFI_CONSOLE_OUT_DEVICE_GUID,
  L"StdErr",              NULL,                 NULL,           EFI_STANDARD_ERROR_DEVICE_GUID,
  L"DebugPort",           NULL,                 NULL,           EFI_DEBUGPORT_PROTOCOL_GUID,
  L"DebugSupport",        SEnvDebugSupport,     NULL,           EFI_DEBUG_SUPPORT_PROTOCOL_GUID,
  L"Decompress",          NULL,                 NULL,           EFI_DECOMPRESS_PROTOCOL_GUID,
  L"Ebc",                 NULL,                 NULL,           EFI_EBC_INTERPRETER_PROTOCOL_GUID,
  L"ScsiPassThru",        NULL,                 NULL,           EFI_SCSI_PASS_THRU_PROTOCOL_GUID,
  L"UsbHostController",   NULL,                 NULL,           EFI_USB_HC_PROTOCOL_GUID,
  L"UsbIo",               NULL,                 NULL,           EFI_USB_IO_PROTOCOL_GUID,
  L"UsbAtapi",            NULL,                 NULL,           EFI_USB_ATAPI_PROTOCOL_GUID,
  L"Nii",                 NULL,                 NULL,           EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL,
  L"BusSpecificDriverOverride", SEnvBusSpecificDriverOverride, NULL, EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL_GUID,
  L"PlatformDriverOverride", NULL,              NULL,           EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL_GUID,
  L"WinNtThunk",          NULL,                 NULL,           EFI_WIN_NT_THUNK_PROTOCOL_GUID,
  L"WinNtIo",             NULL,                 NULL,           EFI_WIN_NT_IO_PROTOCOL_GUID,
  L"PrimaryConIn",        NULL,                 NULL,           EFI_PRIMARY_CONSOLE_IN_DEVICE_GUID,
  L"PrimaryConOut",       NULL,                 NULL,           EFI_PRIMARY_CONSOLE_OUT_DEVICE_GUID,
  L"PrimaryStdErr",       NULL,                 NULL,           EFI_PRIMARY_STANDARD_ERROR_DEVICE_GUID,
  L"IsaAcpi",             NULL,                 NULL,           EFI_ISA_ACPI_PROTOCOL_GUID,
  L"IsaIo",               SEnvIsaIo,            NULL,           EFI_ISA_IO_PROTOCOL_GUID,
  L"UgaDraw",             NULL,                 NULL,           EFI_UGA_DRAW_PROTOCOL_GUID,
  L"UgaIo",               NULL,                 NULL,           EFI_UGA_IO_PROTOCOL_GUID,
  L"VgaMiniPort",         NULL,                 NULL,           EFI_VGA_MINI_PORT_PROTOCOL_GUID,
  L"Unknown Device",      NULL,                 NULL,           UNKNOWN_DEVICE_GUID,

//
// Keep device path last
//
  L"Dpath",               SEnvDPath,            SEnvDPathTok,   EFI_DEVICE_PATH_PROTOCOL_GUID,

  NULL
} ;

//
// SEnvProtocolInfo - A list of all known protocol info structures
//
EFI_LIST_ENTRY  SEnvProtocolInfo;

//
//
//
VOID
SEnvInitProtocolInfo (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // Initialize SEnvProtocolInfo linked list
  //
  InitializeListHead (&SEnvProtocolInfo);
}


VOID
SEnvLoadInternalProtInfo (
  VOID
  )
/*++

Routine Description:

  Initialize internal protocol handlers.

Arguments:

Returns:

--*/
{
  UINTN               Index;

  //
  // Walk through the SEnvInternalProtocolInfo[] array
  // add each protocol info to a linked list
  //
  for (Index = 0; SEnvInternalProtocolInfo[Index].IdString; Index += 1) {
    SEnvAddProtocol (
      &SEnvInternalProtocolInfo[Index].ProtocolId,
      SEnvInternalProtocolInfo[Index].DumpToken,
      SEnvInternalProtocolInfo[Index].DumpInfo,
      SEnvInternalProtocolInfo[Index].IdString
      );
  }
}


PROTOCOL_INFO *
SEnvGetProtById (
  IN EFI_GUID         *Protocol,
  IN BOOLEAN          GenId
  )
/*++

Routine Description:

  Locate a protocol handle by guid.

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO       *Prot;
  EFI_LIST_ENTRY      *Link;
  UINTN               LastId;
  UINTN               Id;
  CHAR16              s[40];

  ASSERT_LOCKED(&SEnvGuidLock);

  //
  // Find the protocol entry for this id
  //
  LastId = 0;
  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    if (CompareGuid(&Prot->ProtocolId, Protocol) == 0) {
      return Prot;
    }

    if (Prot->IdString[0] == 'g') {
      Id = Atoi(Prot->IdString+1);
      LastId = Id > LastId ? Id : LastId;
    }
  }

  //
  // If the protocol id is not found, generate a string for it if needed
  //
  Prot = NULL;
  if (GenId) {
    SPrint (s, sizeof(s), L"g%d", LastId+1);
    Prot = AllocateZeroPool (sizeof(PROTOCOL_INFO));
    if (Prot) {
      Prot->Signature = PROTOCOL_INFO_SIGNATURE;
      CopyMem (&Prot->ProtocolId, Protocol, sizeof(EFI_GUID));
      Prot->IdString = StrDuplicate(s);
      InsertTailList (&SEnvProtocolInfo, &Prot->Link);
    }
  }

  return Prot;
}


PROTOCOL_INFO *
SEnvGetProtByStr (
  IN CHAR16           *Str
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO       *Prot;
  EFI_LIST_ENTRY      *Link;
  UINTN               Index;
  EFI_GUID            Guid;
  CHAR16              c;
  CHAR16              *Ptr;

  ASSERT_LOCKED(&SEnvGuidLock);

  //
  // Search for short name match
  //
  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    if (StriCmp(Prot->IdString, Str) == 0) {
      return Prot;
    }
  }

  //
  // Convert Str to guid and then match
  //
  if (StrLen(Str) == 36  &&  Str[9] == '-'  &&  Str[19] == '-'  && Str[24] == '-') {
    Guid.Data1 = (UINT32) xtoi(Str+0);
    Guid.Data2 = (UINT16) xtoi(Str+10);
    Guid.Data3 = (UINT16) xtoi(Str+15);
    for (Index = 0; Index < 8; Index++) {
      Ptr = Str+25+Index*2;
      c = Ptr[3];
      Ptr[3] = 0;
      Guid.Data4[Index] = (UINT8) xtoi(Ptr);
      Ptr[3] = c;
    }

    for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
      Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
      if (CompareGuid(&Prot->ProtocolId, &Guid) == 0) {
        return Prot;
      }
    }
  }

  return NULL;
}


EFI_STATUS
SEnvIGetProtID (
  IN CHAR16           *Str,
  OUT EFI_GUID        *ProtId
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO       *Prot;
  EFI_STATUS          Status;

  AcquireLock (&SEnvGuidLock);

  Status = EFI_NOT_FOUND;
  CopyMem (ProtId, &NullGuid, sizeof(EFI_GUID));

  //
  // Get protocol id by string
  //
  Prot = SEnvGetProtByStr (Str);
  if (Prot) {
    CopyMem (ProtId, &Prot->ProtocolId, sizeof(EFI_GUID));
    Status = EFI_SUCCESS;
  }

  ReleaseLock (&SEnvGuidLock);

  return Status;
}


VOID
SEnvAddProtocol (
  IN EFI_GUID                     *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpInfo OPTIONAL,
  IN CHAR16                       *IdString
  )
/*++

Routine Description:

  Published interface to add protocol handlers.

Arguments:

Returns:

--*/
{
  SEnvIAddProtocol (TRUE, Protocol, DumpToken, DumpInfo, IdString);
}


VOID
SEnvIAddProtocol (
  IN BOOLEAN                      SaveId,
  IN EFI_GUID                     *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO  DumpInfo OPTIONAL,
  IN CHAR16                       *IdString
  )
/*++

Routine Description:

  Internal interface to add protocol handlers.

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO       *Prot;
  BOOLEAN             StoreInfo;
  CHAR16              *ObsoleteName;

  ObsoleteName = NULL;
  StoreInfo = FALSE;

  AcquireLock (&SEnvGuidLock);

  //
  // Get the current protocol info
  //
  Prot = SEnvGetProtById (Protocol, FALSE);

  if (Prot) {
    //
    // If the name has changed, delete the old variable
    //
    if (StriCmp (Prot->IdString, IdString)) {
      ObsoleteName = Prot->IdString;
      StoreInfo = TRUE;
    } else {
      FreePool (Prot->IdString);
    }
    Prot->IdString = NULL;
  } else {
    //
    // Allocate new protocol info
    //
    Prot = AllocateZeroPool (sizeof(PROTOCOL_INFO));
    Prot->Signature = PROTOCOL_INFO_SIGNATURE;
    StoreInfo = TRUE;
  }

  //
  // Apply any updates to the protocol info
  //
  if (Prot) {
    CopyMem (&Prot->ProtocolId, Protocol, sizeof(EFI_GUID));
    Prot->IdString = StrDuplicate(IdString);
    Prot->DumpToken = DumpToken;
    Prot->DumpInfo = DumpInfo;

    if (Prot->Link.Flink) {
      RemoveEntryList (&Prot->Link);
    }
    InsertTailList (&SEnvProtocolInfo, &Prot->Link);
  }

  ReleaseLock (&SEnvGuidLock);

  //
  // If the name changed, delete the old name
  //
  if (ObsoleteName) {
    RT->SetVariable (ObsoleteName, &SEnvProtId, 0, 0, NULL);
    FreePool (ObsoleteName);
  }

  //
  // Store the protocol idstring to a variable
  //
  if (Prot && StoreInfo  && SaveId) {
    RT->SetVariable (
      Prot->IdString,
      &SEnvProtId,
      EFI_VARIABLE_BOOTSERVICE_ACCESS, // | EFI_VARIABLE_NON_VOLATILE,
      sizeof(EFI_GUID),
      &Prot->ProtocolId
      );
  }
}


VOID
SEnvLoadHandleProtocolInfo (
  IN EFI_GUID         *SkipProtocol
  )
/*++

Routine Description:

  Code to load the internal handle cross-reference info for each protocol.

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO       *Prot;
  EFI_LIST_ENTRY      *Link;

  AcquireLock (&SEnvGuidLock);

  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    //
    // Load internal handle cross-reference info for each protocol
    //
    if (!SkipProtocol || CompareGuid(SkipProtocol, &Prot->ProtocolId) != 0) {
      LibLocateHandle (
        ByProtocol,
        &Prot->ProtocolId,
        NULL,
        &Prot->NoHandles,
        &Prot->Handles
        );
    }
  }

  ReleaseLock (&SEnvGuidLock);
}


VOID
SEnvFreeHandleProtocolInfo (
  VOID
  )
/*++

Routine Description:

  Free the internal handle cross-reference protocol info.

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO       *Prot;
  EFI_LIST_ENTRY      *Link;

  AcquireLock (&SEnvGuidLock);

  //
  // Free all protocol handle info
  //
  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);

    if (Prot->NoHandles) {
      FreePool (Prot->Handles);
      Prot->Handles = NULL;
      Prot->NoHandles = 0;
    }
  }

  ReleaseLock (&SEnvGuidLock);
}


CHAR16 *
SEnvIGetProtocol (
  IN EFI_GUID     *ProtocolId,
  IN BOOLEAN      GenId
  )
/*++

Routine Description:

  Published interface to lookup a protocol id string.

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO   *Prot;
  CHAR16          *Id;

  ASSERT_LOCKED (&SEnvGuidLock);
  Prot = SEnvGetProtById(ProtocolId, GenId);
  Id = Prot ? Prot->IdString : NULL;
  return Id;
}


CHAR16 *
SEnvGetProtocol (
  IN EFI_GUID     *ProtocolId,
  IN BOOLEAN      GenId
  )
/*++

Routine Description:

  Published interface to lookup a protocol id string.

Arguments:

Returns:

--*/
{
  CHAR16          *Id;

  AcquireLock (&SEnvGuidLock);
  Id = SEnvIGetProtocol(ProtocolId, GenId);
  ReleaseLock (&SEnvGuidLock);
  return Id;
}

EFI_STATUS
SEnvCmdProt (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "guid" command.

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO       *Prot;
  EFI_LIST_ENTRY      *Link;
  UINTN               Len;
  UINTN               SLen;
  CHAR16              *Ptr;
  UINTN               Index;

  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Initializing variable to avoid level 4 warning
  //
  for (Index = 1;Index < SI->Argc; Index++) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;
      default :
        Print (L"guid: Unknown flag %hs\n",Ptr);
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  AcquireLock (&SEnvGuidLock);

  //
  // Find the protocol entry for this id
  //

  SLen = 0;
  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    Len = StrLen(Prot->IdString);
    if (StrLen(Prot->IdString) > SLen) {
      SLen = Len;
    }
  }

  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);

    //
    // Can't use Lib function to dump the guid as it may lookup the 
    // "short name" for it
    //

    //
    // BUGBUG : Have to release and reacquire the lock for output redirection
    //          of this command to work properly. Otherwise, we get an ASSERT
    //          from RaiseTPL().
    //
    ReleaseLock (&SEnvGuidLock);

    Print(L"  %h-.*s : %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x  %c\n",
          SLen,
          Prot->IdString,
          Prot->ProtocolId.Data1,
          Prot->ProtocolId.Data2,
          Prot->ProtocolId.Data3,
          Prot->ProtocolId.Data4[0],
          Prot->ProtocolId.Data4[1],
          Prot->ProtocolId.Data4[2],
          Prot->ProtocolId.Data4[3],
          Prot->ProtocolId.Data4[4],
          Prot->ProtocolId.Data4[5],
          Prot->ProtocolId.Data4[6],
          Prot->ProtocolId.Data4[7],
          (Prot->DumpToken || Prot->DumpInfo) ? L'*' : L' '
          );

    AcquireLock (&SEnvGuidLock);
  }

  ReleaseLock (&SEnvGuidLock);
  return EFI_SUCCESS;
}


UINTN
SEnvGetHandleNumber (
  IN  EFI_HANDLE  Handle
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN  HandleNumber;

  for (HandleNumber = 0; HandleNumber < SEnvNoHandles; HandleNumber++) {
    if (SEnvHandles[HandleNumber] == Handle) {
      break;
    }
  }
  if (HandleNumber >= SEnvNoHandles) {
    return 0;
  }
  return (HandleNumber + 1);
}

EFI_STATUS
GetDriverName (
  EFI_HANDLE  DriverBindingHandle,
  UINT8       *Language,
  BOOLEAN     ImageName,
  CHAR16      **DriverName
  )

{
  EFI_STATUS                   Status;
  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding;
  EFI_LOADED_IMAGE_PROTOCOL    *Image;
  EFI_COMPONENT_NAME_PROTOCOL  *ComponentName;

  *DriverName = NULL;

  Status = BS->OpenProtocol(
                 DriverBindingHandle,
                 &gEfiDriverBindingProtocolGuid,
                 (VOID **)&DriverBinding, 
                 NULL, 
                 NULL, 
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (ImageName) {
    Status = BS->OpenProtocol(
                   DriverBinding->ImageHandle,
                   &gEfiLoadedImageProtocolGuid,
                   (VOID **)&Image, 
                   NULL, 
                   NULL, 
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
    if (!EFI_ERROR (Status)) {
      *DriverName = DevicePathToStr (Image->FilePath);
    }
  } else {
    Status = BS->OpenProtocol(
                   DriverBindingHandle,
                   &gEfiComponentNameProtocolGuid,
                   (VOID **)&ComponentName, 
                   NULL, 
                   NULL, 
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
    if (!EFI_ERROR (Status)) {
      Status = ComponentName->GetDriverName (
                                ComponentName, 
                                Language, 
                                DriverName
                                );
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetDeviceName (
  EFI_HANDLE  DeviceHandle,
  BOOLEAN     UseComponentName,
  BOOLEAN     UseDevicePath,
  CHAR8       *Language,
  CHAR16      **BestDeviceName,
  EFI_STATUS  *ConfigurationStatus,
  EFI_STATUS  *DiagnosticsStatus,
  BOOLEAN     Display,
  UINTN       Indent
  )

{
  EFI_STATUS                        Status;
  UINTN                             HandleIndex;
  UINTN                             ParentDriverIndex;
  UINTN                             ChildIndex;
  UINTN                             DriverBindingHandleCount;
  EFI_HANDLE                        *DriverBindingHandleBuffer;
  UINTN                             ParentControllerHandleCount;
  EFI_HANDLE                        *ParentControllerHandleBuffer;
  UINTN                             ParentDriverBindingHandleCount;
  EFI_HANDLE                        *ParentDriverBindingHandleBuffer;
  UINTN                             ChildControllerHandleCount;
  EFI_HANDLE                        *ChildControllerHandleBuffer;
  UINTN                             ChildHandleCount;
  EFI_HANDLE                        *ChildHandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  CHAR16                            *ControllerName;
  EFI_COMPONENT_NAME_PROTOCOL       *ComponentName;
  UINTN                             Index;
  BOOLEAN                           First;

  *BestDeviceName = NULL;
  *ConfigurationStatus = EFI_NOT_FOUND;
  *DiagnosticsStatus   = EFI_NOT_FOUND;
  First                = TRUE;

  //----------2003-08-08-----------------------------
  DriverBindingHandleBuffer    = NULL;
  ParentControllerHandleBuffer = NULL;
  ChildControllerHandleBuffer   = NULL;
  //-------------------------------------------------

  Status = LibGetManagingDriverBindingHandles (
             DeviceHandle,
             &DriverBindingHandleCount,
             &DriverBindingHandleBuffer
             );

  Status = LibGetParentControllerHandles (
             DeviceHandle,
             &ParentControllerHandleCount,
             &ParentControllerHandleBuffer
             );

  Status = LibGetChildControllerHandles (
             DeviceHandle,
             &ChildControllerHandleCount,
             &ChildControllerHandleBuffer
             );

  for (HandleIndex = 0; HandleIndex < DriverBindingHandleCount; HandleIndex++) {

    Status = BS->OpenProtocol (
                   DriverBindingHandleBuffer[HandleIndex],
                   &gEfiDriverConfigurationProtocolGuid,
                   NULL,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                   );
    if (!EFI_ERROR (Status)) {
      *ConfigurationStatus = EFI_SUCCESS;
    }

    Status = BS->OpenProtocol (
                   DriverBindingHandleBuffer[HandleIndex],
                   &gEfiDriverDiagnosticsProtocolGuid,
                   NULL,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                   );
    if (!EFI_ERROR (Status)) {
      *DiagnosticsStatus = EFI_SUCCESS;
    }

    Status = BS->OpenProtocol (
                   DriverBindingHandleBuffer[HandleIndex],
                   &gEfiComponentNameProtocolGuid,
                   (VOID **)&ComponentName,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = ComponentName->GetControllerName (
                              ComponentName, 
                              DeviceHandle, 
                              NULL, 
                              Language, 
                              &ControllerName
                              );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (Display) {
      if (!First) {
        Print (L"\n");
        for (Index = 0; Index < Indent; Index++) {
          Print (L" ");
        }
      }
      Print (L"%hs",ControllerName);
      First = FALSE;
    }

    if (UseComponentName) {
      if (*BestDeviceName == NULL) {
        //*BestDeviceName = ControllerName;
        //----2003-08-08-----------------------------------
		//*BestDeviceName to be freed 
		//If not duplicate string, caller(SEnvCmdDevices) don't know whether he need to free this string
        *BestDeviceName = StrDuplicate(ControllerName);
		//-------------------------------------------------

      }
    }
  }

  for (HandleIndex = 0; HandleIndex < ParentControllerHandleCount; HandleIndex++) {

    Status = LibGetManagingDriverBindingHandles (
               ParentControllerHandleBuffer[HandleIndex],
               &ParentDriverBindingHandleCount,
               &ParentDriverBindingHandleBuffer
               );
    if (EFI_ERROR (Status)) {
      continue;
    }

    for (ParentDriverIndex = 0; ParentDriverIndex < ParentDriverBindingHandleCount; ParentDriverIndex++) {

      Status = LibGetManagedChildControllerHandles (
                 ParentDriverBindingHandleBuffer[ParentDriverIndex],
                 ParentControllerHandleBuffer[HandleIndex],
                 &ChildHandleCount,
                 &ChildHandleBuffer
                 );
      if (EFI_ERROR (Status)) {
        continue;
      }

      for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {
        if (ChildHandleBuffer[ChildIndex] == DeviceHandle) {

          Status = BS->OpenProtocol (
                         ParentDriverBindingHandleBuffer[ParentDriverIndex],
                         &gEfiDriverConfigurationProtocolGuid,
                         NULL,
                         NULL,
                         NULL,
                         EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                         );
          if (!EFI_ERROR (Status)) {
            *ConfigurationStatus = EFI_SUCCESS;
          }

          Status = BS->OpenProtocol (
                         ParentDriverBindingHandleBuffer[ParentDriverIndex],
                         &gEfiDriverDiagnosticsProtocolGuid,
                         NULL,
                         NULL,
                         NULL,
                         EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                         );
          if (!EFI_ERROR (Status)) {
            *DiagnosticsStatus = EFI_SUCCESS;
          }

          Status = BS->OpenProtocol (
                         ParentDriverBindingHandleBuffer[ParentDriverIndex],
                         &gEfiComponentNameProtocolGuid,
                         (VOID **)&ComponentName,
                         NULL,
                         NULL,
                         EFI_OPEN_PROTOCOL_GET_PROTOCOL
                         );
          if (EFI_ERROR (Status)) {
            continue;
          }

          Status = ComponentName->GetControllerName (
                                    ComponentName, 
                                    ParentControllerHandleBuffer[HandleIndex],
                                    DeviceHandle, 
                                    Language, 
                                    &ControllerName
                                    );
          if (EFI_ERROR (Status)) {
            continue;
          }

          if (Display) {
            if (!First) {
              Print (L"\n");
              for (Index = 0; Index < Indent; Index++) {
                Print (L" ");
              }
            }
            Print (L"%hs",ControllerName);
            First = FALSE;
          }

          if (UseComponentName) {
            if (*BestDeviceName == NULL) {
			  /**BestDeviceName = ControllerName;*/
			  //----2003-08-08-----------------------------------
			  //*BestDeviceName to be freed 
			  //If not duplicate string, caller(SEnvCmdDevices) don't know whether he need to free this string
              *BestDeviceName = StrDuplicate(ControllerName);
			  //-------------------------------------------------
            }   
          }
        }
      }
      BS->FreePool (ChildHandleBuffer);
    }
    BS->FreePool(ParentDriverBindingHandleBuffer);
  }

  if (UseDevicePath) {
    if (*BestDeviceName == NULL) {
      Status = BS->OpenProtocol (
                     DeviceHandle,
                     &gEfiDevicePathProtocolGuid,
                     (VOID **)&DevicePath,
                     NULL,
                     NULL,
                     EFI_OPEN_PROTOCOL_GET_PROTOCOL
                     );

      if (!EFI_ERROR (Status)) {
        *BestDeviceName = DevicePathToStr(DevicePath);
      }
    }
  }
  //-------------2003-08-08---------------------------------
  if(DriverBindingHandleBuffer){
	BS->FreePool(DriverBindingHandleBuffer);
  }
  if(ParentControllerHandleBuffer){
  	BS->FreePool(ParentControllerHandleBuffer);
  }
  if(ChildControllerHandleBuffer){
      BS->FreePool(ChildControllerHandleBuffer);
  }
  //--------------------------------------------------------

  return EFI_SUCCESS;
}

EFI_STATUS
DisplayDriverModelHandle (
  IN EFI_HANDLE  Handle,
  IN BOOLEAN     BestName,
  IN CHAR8       *Language
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                        Status;
  EFI_STATUS                        ConfigurationStatus;
  EFI_STATUS                        DiagnosticsStatus;
  UINTN                             DriverBindingHandleCount;
  EFI_HANDLE                        *DriverBindingHandleBuffer;
  UINTN                             ParentControllerHandleCount;
  EFI_HANDLE                        *ParentControllerHandleBuffer;
  UINTN                             ChildControllerHandleCount;
  EFI_HANDLE                        *ChildControllerHandleBuffer;
  CHAR16                            *BestDeviceName;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  UINTN                             Index;
  CHAR16                            *DriverName;
  EFI_DRIVER_BINDING_PROTOCOL       *DriverBinding;
  EFI_COMPONENT_NAME_PROTOCOL       *ComponentName;
  UINTN                             NumberOfChildren;
  UINTN                             HandleIndex;
  UINTN                             ControllerHandleCount;
  EFI_HANDLE                        *ControllerHandleBuffer;
  UINTN                             ChildIndex;
  BOOLEAN                           Image;

  //
  // See if Handle is a device handle and display its details.
  //
  DriverBindingHandleBuffer = NULL;
  Status = LibGetManagingDriverBindingHandles (
             Handle,
             &DriverBindingHandleCount,
             &DriverBindingHandleBuffer
             );

  ParentControllerHandleBuffer = NULL;
  Status = LibGetParentControllerHandles (
             Handle,
             &ParentControllerHandleCount,
             &ParentControllerHandleBuffer
             );

  ChildControllerHandleBuffer = NULL;
  Status = LibGetChildControllerHandles (
             Handle,
             &ChildControllerHandleCount,
             &ChildControllerHandleBuffer
             );

  if (DriverBindingHandleCount > 0 || ParentControllerHandleCount > 0 || ChildControllerHandleCount > 0) {

    DevicePath = NULL;
    //----------2003-08-08-------------------
    BestDeviceName = NULL;
    //---------------------------------------
    Status = BS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid, &DevicePath);

    Print (L"\n");
    Print (L"     Controller Name    : ");

    Status = GetDeviceName (
               Handle,
               TRUE,
               FALSE,
               Language,
               &BestDeviceName,
               &ConfigurationStatus,
               &DiagnosticsStatus,
               TRUE,
               26
               );

    if (BestDeviceName == NULL) {
      Print (L"%H<UNKNOWN>%N");
    }
    Print (L"\n");

    if (DevicePath != NULL) {
      Print (L"     Device Path        : %hs\n", DevicePathToStr (DevicePath));
    } else {
      Print (L"     Device Path        : %H<NONE>%N\n");
    }
  
    Print (L"     Controller Type    : ");
    if (ParentControllerHandleCount == 0) {
      Print (L"%HROOT%N\n");
    } else if (ChildControllerHandleCount > 0) {
      Print (L"%HBUS%N\n");
    } else {
      Print (L"%HDEVICE%N\n");
    }

    if (!EFI_ERROR (ConfigurationStatus)) {
      Print (L"     Configuration      : %HYES%N\n");
    } else {
      Print (L"     Configuration      : %HNO%N\n");
    }

    if (!EFI_ERROR (DiagnosticsStatus)) {
      Print (L"     Diagnostics        : %HYES%N\n");
    } else {
      Print (L"     Diagnostics        : %HNO%N\n");
    }

    if (DriverBindingHandleCount != 0) {
      Print (L"     Managed by :\n");
      for (Index = 0; Index < DriverBindingHandleCount; Index++) {
        Image = FALSE;
        Status = GetDriverName (
                   DriverBindingHandleBuffer[Index],
                   Language,
                   FALSE,
                   &DriverName
                   );
        if (DriverName == NULL) {
          Status = GetDriverName (
                     DriverBindingHandleBuffer[Index],
                     Language,
                     TRUE,
                     &DriverName
                     );
          if (!EFI_ERROR (Status)) {
            Image = TRUE;
          }
        }
        if (DriverName == NULL) {
          DriverName = L"<UNKNOWN>";
        }
        if (Image) {
          Print (L"       Drv[%h02x] : Image(%hs)\n", 
                 SEnvGetHandleNumber (DriverBindingHandleBuffer[Index]), 
                 DriverName);
        } else {
          Print (L"       Drv[%h02x] : %hs\n", 
                 SEnvGetHandleNumber (DriverBindingHandleBuffer[Index]),
                 DriverName);
        }   
      }
    } else {
      Print (L"     Managed by         : %H<NONE>%N\n");
    }

    if (ParentControllerHandleCount != 0) {
      Print (L"     Parent Controllers :\n");
      for (Index = 0; Index < ParentControllerHandleCount; Index++) {
        Status = GetDeviceName (
                   ParentControllerHandleBuffer[Index],
                   BestName,
                   TRUE,
                   Language,
                   &BestDeviceName,
                   &ConfigurationStatus,
                   &DiagnosticsStatus,
                   FALSE,
                   0
                   );
        if (BestDeviceName == NULL) {
          BestDeviceName = StrDuplicate(L"<UNKNOWN>");
        }
        Print (L"       Parent[%h02x] : %hs\n", 
               SEnvGetHandleNumber (ParentControllerHandleBuffer[Index]), 
               BestDeviceName);
 	    FreePool(BestDeviceName);
      }
    } else {
      Print (L"     Parent Controllers : %H<NONE>%N\n");
    }

    if (ChildControllerHandleCount != 0) {
      Print (L"     Child Controllers  :\n");
      for (Index = 0; Index < ChildControllerHandleCount; Index++) {
        Status = GetDeviceName (
                   ChildControllerHandleBuffer[Index],
                   BestName,
                   TRUE,
                   Language,
                   &BestDeviceName,
                   &ConfigurationStatus,
                   &DiagnosticsStatus,
                   FALSE,
                   0
                   );
        if (BestDeviceName == NULL) {
          BestDeviceName = StrDuplicate(L"<UNKNOWN>");
        }
        Print (L"       Child[%h02x] : %hs\n", 
               SEnvGetHandleNumber (ChildControllerHandleBuffer[Index]), 
               BestDeviceName);
	    FreePool(BestDeviceName);
      }
    } else {
      Print (L"     Child Controllers  : %H<NONE>%N\n");
    }
  }
  Status = EFI_SUCCESS;

  if (DriverBindingHandleBuffer) {
    BS->FreePool (DriverBindingHandleBuffer);
  }
  if (ParentControllerHandleBuffer) {
    BS->FreePool (ParentControllerHandleBuffer);
  }
  if (ChildControllerHandleBuffer) {
    BS->FreePool (ChildControllerHandleBuffer);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // See if Handle is a driver binding handle and display its details.
  //

  Status = BS->OpenProtocol (
                 Handle,
                 &gEfiDriverBindingProtocolGuid,
                 (VOID **)&DriverBinding,
                 NULL,
                 NULL,
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  ComponentName = NULL;
  Status = BS->OpenProtocol (
                 Handle,
                 &gEfiComponentNameProtocolGuid,
                 (VOID **)&ComponentName,
                 NULL,
                 NULL,
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );

  DiagnosticsStatus = BS->OpenProtocol (
                            Handle,
                            &gEfiDriverDiagnosticsProtocolGuid,
                            NULL,
                            NULL,
                            NULL,
                            EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                            );

  ConfigurationStatus = BS->OpenProtocol (
                              Handle,
                              &gEfiDriverConfigurationProtocolGuid,
                              NULL,
                              NULL,
                              NULL,
                              EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                              );

  NumberOfChildren = 0;
  ControllerHandleBuffer = NULL;
  Status = LibGetManagedControllerHandles (
             Handle,
             &ControllerHandleCount,
             &ControllerHandleBuffer
             );
  if (ControllerHandleCount > 0) {
    for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
      Status = LibGetManagedChildControllerHandles (
                 Handle,
                 ControllerHandleBuffer[HandleIndex],
                 &ChildControllerHandleCount,
                 NULL
                 );
      NumberOfChildren += ChildControllerHandleCount;
    }
  }

  Status = GetDriverName (
             Handle,
             Language,
             FALSE,
             &DriverName
             );
  Print (L"\n");
  if (DriverName != NULL) {
    Print (L"     Driver Name    : %hs\n", DriverName);
  } else {
    Print (L"     Driver Name    : %H<NONE>%N\n");
  }

  Status = GetDriverName (
             Handle,
             Language,
             TRUE,
             &DriverName
             );
  if (DriverName != NULL) {
    Print (L"     Image Name     : %hs\n", DriverName);
  } else {
    Print (L"     Image Name     : %H<NONE>%N\n");
  }

  Print (L"     Driver Version : %h08x\n", DriverBinding->Version);

  if (NumberOfChildren > 0) {
    Print (L"     Driver Type    : %HBUS%N\n");    
  } else if (ControllerHandleCount > 0) {
    Print (L"     Driver Type    : %HDEVICE%N\n");
  } else {
    Print (L"     Driver Type    : %H<UNKNOWN>%N\n");
  }

  if (!EFI_ERROR (ConfigurationStatus)) {
    Print (L"     Configuration  : %HYES%N\n");
  } else {
    Print (L"     Configuration  : %HNO%N\n");
  }

  if (!EFI_ERROR (DiagnosticsStatus)) {
    Print (L"     Diagnostics    : %HYES%N\n");
  } else {
    Print (L"     Diagnostics    : %HNO%N\n");
  }

  if (ControllerHandleCount == 0) {
    Print (L"     Managing       : %H<NONE>%N\n");
  } else {
    Print (L"     Managing       :\n");
    for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
      Status = GetDeviceName (
                 ControllerHandleBuffer[HandleIndex],
                 BestName,
                 TRUE,
                 Language,
                 &BestDeviceName,
                 &ConfigurationStatus,
                 &DiagnosticsStatus,
                 FALSE,
                 0
                 );
      if (BestDeviceName == NULL) {
          BestDeviceName = StrDuplicate(L"<UNKNOWN>");
      }
      Print (L"       Ctrl[%h02x] : %hs\n", 
             SEnvGetHandleNumber (ControllerHandleBuffer[HandleIndex]), 
             BestDeviceName);

      FreePool(BestDeviceName);
      BestDeviceName = NULL;

      Status = LibGetManagedChildControllerHandles (
                 Handle,
                 ControllerHandleBuffer[HandleIndex],
                 &ChildControllerHandleCount,
                 &ChildControllerHandleBuffer
                 );
      if (!EFI_ERROR (Status)) {
        for (ChildIndex = 0; ChildIndex < ChildControllerHandleCount; ChildIndex++) {
          Status = GetDeviceName (
                     ChildControllerHandleBuffer[ChildIndex],
                     BestName,
                     TRUE,
                     Language,
                     &BestDeviceName,
                     &ConfigurationStatus,
                     &DiagnosticsStatus,
                     FALSE,
                     0
                     );
          if (BestDeviceName == NULL) {
            BestDeviceName = StrDuplicate(L"<UNKNOWN>");
          }
          Print (L"         Child[%h02x] : %hs\n", 
                 SEnvGetHandleNumber (ChildControllerHandleBuffer[ChildIndex]),
                 BestDeviceName);
	      FreePool(BestDeviceName);
        }
        BS->FreePool (ChildControllerHandleBuffer);
      }
    }
    BS->FreePool (ControllerHandleBuffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDHProt (
  IN BOOLEAN          Verbose,
  IN BOOLEAN          DriverModel,
  IN UINTN            HandleNo,
  IN EFI_HANDLE       Handle,
  IN CHAR8            *Language
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO               *Prot;
  EFI_LIST_ENTRY              *Link;
  VOID                        *Interface;
  UINTN                       Index;
  UINTN                       Index1;
  EFI_STATUS                  Status;
  SHELLENV_DUMP_PROTOCOL_INFO Dump;
  EFI_GUID                    **ProtocolBuffer;
  UINTN                       ProtocolBufferCount;

  if (!HandleNo) {
    HandleNo = SEnvGetHandleNumber (Handle);
  }

  //
  // Get protocol info by handle
  //
  ProtocolBuffer = NULL;
  Status = BS->ProtocolsPerHandle (Handle, &ProtocolBuffer, 
                                   &ProtocolBufferCount);

  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }
  
  if (Verbose) {
    Print (L"%NHandle %h02x (%hX)\n", HandleNo, Handle);
  } else {
    Print (L"%N %h2x: ", HandleNo, Handle);
  }

  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    for (Index = 0; Index < Prot->NoHandles; Index++) {
      //
      // If this handle supports this protocol, dump it
      //
      if (Prot->Handles[Index] == Handle) {
        Dump = Verbose ? Prot->DumpInfo : Prot->DumpToken;
        Status = BS->HandleProtocol (Handle, &Prot->ProtocolId, &Interface);
        if (!EFI_ERROR (Status)) {
          if (Verbose) {
            for (Index1 = 0; Index1 < ProtocolBufferCount; Index1++) {
              if (ProtocolBuffer[Index1] != NULL) {
                if (CompareGuid (ProtocolBuffer[Index1], &Prot->ProtocolId) == 0) {
                  ProtocolBuffer[Index1] = NULL;
                }
              }
            }

            //
            // Dump verbose info
            //

            Print (L"   %hs (%X) ", Prot->IdString, Interface);
            if (Dump != NULL) {
              Dump (Handle, Interface);
            }
            Print (L"\n");
          } else {
            if (Dump != NULL) {
              Dump (Handle, Interface);
            }  else {
              Print (L"%hs ", Prot->IdString);
            }
          }
        }
      }
    }
  }

  if (DriverModel) {
    Status = DisplayDriverModelHandle (Handle, TRUE, Language);
    if (EFI_ERROR (Status)) {
      goto Done;
    }
  }

  //
  // Dump verbose info
  //
  if (Verbose) {
    for (Index1 = 0; Index1 < ProtocolBufferCount; Index1++) {
      if (ProtocolBuffer[Index1] != NULL) {
        Status = BS->HandleProtocol (Handle, ProtocolBuffer[Index1], &Interface);
        if (!EFI_ERROR (Status)) {
          Print (L"   %hg (%08x)\n", ProtocolBuffer[Index1], Interface);
        }
      }
    }

    Print (L"%N");
  } else {
    Print (L"%N\n");
  }
  Status = EFI_SUCCESS;

Done:
  if (ProtocolBuffer != NULL) {
    BS->FreePool (ProtocolBuffer);
  }

  return Status;
}

EFI_STATUS
SEnvCmdDH (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "DH" command

Arguments:

Returns:

--*/
{
  BOOLEAN             ByProtocol;
  CHAR16              *Arg;
  CHAR16              *Ptr;
  EFI_STATUS          Status;
  UINTN               Index;
  PROTOCOL_INFO       *Prot;
  BOOLEAN             Verbose;
  BOOLEAN             DriverModel;
  UINTN               StringIndex;
  CHAR8               *Language;

  //
  // Initialize
  //
  InitializeShellApplication (ImageHandle, SystemTable);

  Arg = NULL;
  ByProtocol = FALSE;

  Language = LibGetVariable (VarLanguage, &EfiGlobalVariable);
  if (Language == NULL) {
    Language = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  //
  // Crack args
  //
  Verbose     = FALSE;
  DriverModel = FALSE;
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'p':
      case 'P':
        ByProtocol = TRUE;
        break;

      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case 'v' :
      case 'V' :
        Verbose = TRUE;
        break;

      case 'd' :
      case 'D' :
        DriverModel = TRUE;
        break;

      case 'l' :
      case 'L' :
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            Language[StringIndex] = (CHAR8)Ptr[StringIndex + 2];
          }
          Language [StringIndex] = 0;
        }
        break;

      default:
        Print (L"dh: Unkown flag %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
      continue;
    }

    if (!Arg) {
      Arg = Ptr;
      continue;
    }

    Print (L"dh: Too many arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  //
  // Load handle & protocol info tables
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  if (Arg) {
    if (ByProtocol) {
      AcquireLock (&SEnvGuidLock);
      Prot = SEnvGetProtByStr (Arg);
      ReleaseLock (&SEnvGuidLock);

      if (Prot) {
        //
        // Dump the handles on this protocol
        //
        Print(L"%NHandle dump by protocol '%s'\n", Prot->IdString);
        for (Index = 0; Index < Prot->NoHandles; Index++) {
          Status = SEnvDHProt (Verbose, DriverModel, 0, Prot->Handles[Index], Language);
          if (EFI_ERROR (Status)) {
            goto Done;
          }

        }
      } else {
        Print(L"dh: Protocol '%s' not found\n", Arg);
      }
    } else {

      //
      // Dump 1 handle
      //
      Index = SEnvHandleNoFromStr(Arg) - 1;
      if (Index > SEnvNoHandles) {
        Print (L"dh: Invalid handle number %hs\n", Arg);
      } else {
        if (DriverModel) {
          Status = SEnvDHProt (Verbose, DriverModel, Index+1, 
                               SEnvHandles[Index], Language);
        } else {
          Status = SEnvDHProt (TRUE, DriverModel, Index+1, 
                               SEnvHandles[Index], Language);
        }
        if (EFI_ERROR (Status)) {
          goto Done;
        }
      }
    }
  } else {
    //
    // Dump all handles
    //
    Print(L"%NHandle dump\n");
    for (Index = 0; Index < SEnvNoHandles; Index++) {
      Status = SEnvDHProt (Verbose, DriverModel, Index+1, 
                           SEnvHandles[Index], Language);
      if (EFI_ERROR (Status)) {
        goto Done;
      }
    }
  }
  Status = EFI_SUCCESS;

  ///////////////////////////////////////////////////////////
  //Remember to free memory previous allated. 
  //Appended at 2003-08-07
  if(Language != NULL){
	  FreePool(Language);
  }
  ///////////////////////////////////////////////////////////

Done:
  SEnvFreeHandleTable ();
  return Status;
}

EFI_STATUS
SEnvDeviceTree (
  IN EFI_HANDLE       ControllerHandle,
  IN UINTN            Level,
  IN BOOLEAN          RootOnly,
  IN BOOLEAN          BestName,
  IN CHAR8            *Language
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                   Status;
  EFI_LOADED_IMAGE_PROTOCOL    *Image;
  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding;
  UINT32                       ControllerHandleIndex;
  UINTN                        HandleCount;
  EFI_HANDLE                   *HandleBuffer;
  UINT32                       *HandleType;
  UINTN                        HandleIndex;
  UINTN                        ChildIndex;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;
  UINTN                        ChildHandleCount;
  EFI_HANDLE                   *ChildHandleBuffer;
  UINT32                       *ChildHandleType;
  UINTN                        Index;
  BOOLEAN                      Root;
  EFI_STATUS                   ConfigurationStatus;
  EFI_STATUS                   DiagnosticsStatus;
  CHAR16                       *DeviceName;

  Status = BS->OpenProtocol(
                 ControllerHandle,
                 &gEfiDriverBindingProtocolGuid,
                 (VOID **)&DriverBinding, 
                 NULL, 
                 NULL, 
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = BS->OpenProtocol(
                 ControllerHandle,
                 &gEfiLoadedImageProtocolGuid,
                 (VOID **)&Image, 
                 NULL, 
                 NULL, 
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = LibScanHandleDatabase (
             NULL,
             NULL,
             ControllerHandle, 
             &ControllerHandleIndex, 
             &HandleCount, 
             &HandleBuffer, 
             &HandleType
             );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (RootOnly) {
    Root = TRUE;
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
        Root = FALSE;
      }
    }

    Status = BS->OpenProtocol(
                   ControllerHandle, 
                   &gEfiDevicePathProtocolGuid,
                   (VOID **)&DevicePath, 
                   NULL, 
                   NULL, 
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
    if (EFI_ERROR (Status)) {
      Root = FALSE;
    }

    if (!Root) {
      return EFI_SUCCESS;
    }
  }

  //
  // Display the handle specified by ControllerHandle
  //
  for (Index = 0; Index < Level; Index++) {
    Print(L"  ");
  }
  Print (L"Ctrl[%h02x] ",SEnvHandleToNumber (ControllerHandle));

  Status = GetDeviceName (
             ControllerHandle,
             BestName,
             TRUE,
             Language,
             &DeviceName,
             &ConfigurationStatus,
             &DiagnosticsStatus,
             FALSE,
             0
             );
  if (DeviceName != NULL) {
    Print (L"%s\n", DeviceName);
  } else {
    Print(L"<UNKNOWN>\n");
  }

  //
  // Print the list of drivers that are managing this controller
  //
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE && 
        HandleType[HandleIndex] & EFI_HANDLE_TYPE_DEVICE_DRIVER            ) {

      DriverBinding = NULL;
      Status = BS->OpenProtocol(
                     HandleBuffer[HandleIndex],
                     &gEfiDriverBindingProtocolGuid,
                     (VOID **)&DriverBinding, 
                     NULL, 
                     NULL, 
                     EFI_OPEN_PROTOCOL_GET_PROTOCOL
                     );

      Status = LibScanHandleDatabase (
                 HandleBuffer[HandleIndex], 
                 NULL, 
                 ControllerHandle, 
                 &ControllerHandleIndex, 
                 &ChildHandleCount, 
                 &ChildHandleBuffer, 
                 &ChildHandleType
                 );

      if (!EFI_ERROR (Status)) {
        for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {
          if (ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_CHILD_HANDLE  &&
              ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_DEVICE_HANDLE ) {

            Status = SEnvDeviceTree(
                       ChildHandleBuffer[ChildIndex], 
                       Level+1, 
                       FALSE,
                       BestName,
                       Language
                       );
            if (EFI_ERROR (Status)) {
              return Status;
            }
          }
        }

        BS->FreePool(ChildHandleBuffer);
        BS->FreePool(ChildHandleType);
      }
    }
  }

  BS->FreePool (HandleBuffer);
  BS->FreePool (HandleType);

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvCmdDeviceTree (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )

{
  CHAR16              *Arg;
  CHAR16              *Ptr;
  CHAR8               *Language;
  BOOLEAN             BestName;
  EFI_STATUS          Status;
  UINTN               Index;
  UINTN               StringIndex;

  //
  // Initialize
  //
  InitializeShellApplication (ImageHandle, SystemTable);

  Language = LibGetVariable (VarLanguage, &EfiGlobalVariable);
  if (Language == NULL) {
    Language = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  Arg = NULL;

  //
  // Crack args
  //
  BestName = TRUE;
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'l' :
      case 'L' :
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            Language[StringIndex] = (CHAR8)Ptr[StringIndex + 2];
          }
          Language [StringIndex] = 0;
        }
        break;

      case 'd' :
      case 'D' :
        BestName = FALSE;
        break;

      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      default:
        Print (L"devtree: Unkown flag %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
      continue;
    }

    if (!Arg) {
      Arg = Ptr;
      continue;
    }

    Print (L"devtree: Too many arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Load handle & protocol info tables
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  if (Arg) {
    //
    // Dump 1 handle
    //
    Index = SEnvHandleNoFromStr(Arg) - 1;
    if (Index > SEnvNoHandles) {
      Print (L"devtree: Invalid handle number %hs\n", Arg);
    } else {
      Print(L"%NDevice Tree\n");
      SEnvDeviceTree(
        SEnvHandles[Index], 
        1, 
        FALSE,
        BestName,
        Language
        );
    }
  } else {
    //
    // Dump all handles
    //
    Print(L"%NDevice Tree\n");
    for (Index = 0; Index < SEnvNoHandles; Index++) {
      SEnvDeviceTree(
        SEnvHandles[Index], 
        1, 
        TRUE,
        BestName,
        Language
        );
    }
  }
  Status = EFI_SUCCESS;

Done:
  SEnvFreeHandleTable ();
  return Status;
}

EFI_STATUS
SEnvCmdDriverConfigurationProcessActionRequired (
  EFI_HANDLE                                DriverImageHandle,
  EFI_HANDLE                                ControllerHandle,
  EFI_HANDLE                                ChildHandle,
  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired
  )

{
  CHAR16      ReturnStr[2];
  EFI_HANDLE  ContextOverride[2];

  switch (ActionRequired) {
  case EfiDriverConfigurationActionNone:
    Print (L"None\n");
    break;
  case EfiDriverConfigurationActionStopController:
    Print (L"Stop Controller\n");
    Print (L"\nPress [ENTER] to stop controller");
    Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
    BS->DisconnectController (ControllerHandle, DriverImageHandle, ChildHandle);
    Print (L"Controller stopped\n");
    break;
  case EfiDriverConfigurationActionRestartController:
    Print (L"Restart Controller\n");
    Print (L"\nPress [ENTER] to restart controller");
    Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
    BS->DisconnectController (ControllerHandle, DriverImageHandle, ChildHandle);
    ContextOverride[0] = DriverImageHandle;
    ContextOverride[1] = NULL;
    BS->ConnectController (ControllerHandle, ContextOverride, NULL, TRUE);
    Print (L"Controller restarted\n");
    break;
  case EfiDriverConfigurationActionRestartPlatform:
    Print (L"Restart Platform\n");
    Print (L"\nPress [ENTER] to restart platform");
    Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
    RT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
    break;
  default:
    Print (L"Unknown\n");
    break;
  }
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvCmdDriverConfiguration (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                                Status;
  CHAR16                                    *Ptr;
  UINTN                                     HandleNumber;
  EFI_HANDLE                                DriverImageHandle;
  EFI_HANDLE                                DeviceHandle;
  EFI_HANDLE                                ChildHandle;
  UINTN                                     StringIndex;
  UINTN                                     Index;
  CHAR8                                     *Language;
  CHAR8                                     *SupportedLanguages;
  UINTN                                     DriverImageHandleCount;
  EFI_HANDLE                                *DriverImageHandleBuffer;
  UINTN                                     HandleCount;
  EFI_HANDLE                                *HandleBuffer;
  UINT32                                    *HandleType;
  UINTN                                     HandleIndex;
  UINTN                                     ChildIndex;
  UINTN                                     ChildHandleCount;
  EFI_HANDLE                                *ChildHandleBuffer;
  UINT32                                    *ChildHandleType;
  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired;
  EFI_DRIVER_CONFIGURATION_PROTOCOL         *DriverConfiguration;
  BOOLEAN                                   ForceDefaults;
  UINT32                                    DefaultType;
  BOOLEAN                                   ValidateOptions;
  BOOLEAN                                   SetOptions;
  BOOLEAN                                   AllChildren;
  BOOLEAN                                   DriverHandleBufferAlloc;
                                            

  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  DriverImageHandle      = NULL;
  DeviceHandle           = NULL;
  ChildHandle            = NULL;
  ForceDefaults          = FALSE;
  DefaultType            = 0;
  ValidateOptions        = FALSE;
  SetOptions             = FALSE;
  AllChildren            = FALSE;
  DriverHandleBufferAlloc= FALSE;

  Language = LibGetVariable (VarLanguage, &EfiGlobalVariable);
  if (Language == NULL) {
    Language = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'l' :
      case 'L' :
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            Language[StringIndex] = (CHAR8)Ptr[StringIndex + 2];
          }
          Language [StringIndex] = 0;
        }
        break;

      case 'c' :
      case 'C' :
        AllChildren = TRUE;
        break;

      case 'f' :
      case 'F' :
        if (ValidateOptions || SetOptions) {
          Print (L"drvcfg: Invalid flag combination\n");
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
        ForceDefaults = TRUE;
        DefaultType = (UINT32) (xtoi (Ptr+2));
        break;

      case 'v' :
      case 'V' :
        if (ForceDefaults || SetOptions) {
          Print (L"drvcfg: Invalid flag combination\n");
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
        ValidateOptions = TRUE;
        break;

      case 's' :
      case 'S' :
        if (ForceDefaults || ValidateOptions) {
          Print (L"drvcfg: Invalid flag combination\n");
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
        SetOptions = TRUE;
        break;

      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      default:
        Print (L"drvcfg: Unkown flag %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      HandleNumber = SEnvHandleNoFromStr (Ptr);
      if (HandleNumber == 0) {
        Print (L"drvcfg: Invalid handle %s\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      if (DriverImageHandle == NULL) {
        DriverImageHandle = SEnvHandles[HandleNumber - 1];
      } else if (DeviceHandle== NULL) {
        DeviceHandle = SEnvHandles[HandleNumber - 1];
      } else if (ChildHandle== NULL) {
        ChildHandle = SEnvHandles[HandleNumber - 1];
      } else {
        Print (L"drvcfg: Too many arguments\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  }

  if (ChildHandle == NULL && AllChildren) {
    SetOptions = FALSE;
  }

  if (ForceDefaults) {
    Print (L"Force Default Configuration to DefaultType %08x\n", DefaultType);
  } else if (ValidateOptions) {
    Print (L"Validate Configuration Options\n");
  } else if (SetOptions) {
    Print (L"Set Configuration Options\n");
  } else {
    Print (L"Configurable Components\n");
  }

  //
  // Display all handles that support being configured
  //
  if (DriverImageHandle == NULL) {
    Status = LibLocateHandle (
               ByProtocol,
               &gEfiDriverConfigurationProtocolGuid,
               NULL,
               &DriverImageHandleCount,
               &DriverImageHandleBuffer
               );
    if (EFI_ERROR (Status)) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
	DriverHandleBufferAlloc = TRUE;
  } else {
    DriverImageHandleCount  = 1;
    DriverImageHandleBuffer = &DriverImageHandle;
	DriverHandleBufferAlloc = FALSE;
  }
                                   
  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    Status = BS->OpenProtocol (
                   DriverImageHandleBuffer[Index],
                   &gEfiDriverConfigurationProtocolGuid,
                   (VOID **)&DriverConfiguration,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
    if (EFI_ERROR (Status)) {
      Print (L"drvcfg: Driver image handle [%h02x] does not support Driver Configuration Protocol\n", SEnvGetHandleNumber (DriverImageHandleBuffer[Index]));
      continue;
    }

    Status = EFI_NOT_FOUND;
    for (SupportedLanguages = DriverConfiguration->SupportedLanguages; SupportedLanguages[0] != 0; SupportedLanguages += 3) {
      if (CompareMem (SupportedLanguages, Language, 3) == 0) {
        Status = EFI_SUCCESS;
      }
    }
    if (EFI_ERROR (Status)) {
      Print (L"drvcfg: Driver image handle [%h02x] does not support language %a\n", 
             SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), Language);
      continue;
    }

    Status = LibScanHandleDatabase (
               DriverImageHandleBuffer[Index], 
               NULL,
               NULL, 
               NULL, 
               &HandleCount, 
               &HandleBuffer, 
               &HandleType
               );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (SetOptions && DeviceHandle == NULL) {
      ST->ConOut->ClearScreen(ST->ConOut);
      Status = DriverConfiguration->SetOptions (
                                      DriverConfiguration,
                                      NULL,
                                      NULL,
                                      Language,
                                      &ActionRequired
                                      );
      ST->ConOut->ClearScreen(ST->ConOut);
      Print(
        L"  Drv[%h02x]  Ctrl[ALL]  Lang[%ha]", 
        SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), 
        DriverConfiguration->SupportedLanguages
        );
      if (!EFI_ERROR (Status)) {
        Print (L" - Options set.  Action Required is ");

        for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

          if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) == EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {

            SEnvCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );

          }
        }

      } else {
        Print (L" - Options not set. Status = %r\n", Status);
      }
      continue;
    }

    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

      if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) != EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {
        continue;
      }

      if (DeviceHandle != NULL && DeviceHandle != HandleBuffer[HandleIndex]) {
        continue;
      }

      if (ChildHandle == NULL) {
        Print(
          L"  Drv[%h02x]  Ctrl[%h02x]  Lang[%ha]", 
          SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), 
          SEnvGetHandleNumber (HandleBuffer[HandleIndex]),
          DriverConfiguration->SupportedLanguages
          );

        if (ForceDefaults) {
          Status = DriverConfiguration->ForceDefaults (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL,
                                          DefaultType,
                                          &ActionRequired
                                          );

          if (!EFI_ERROR (Status)) {
            Print (L" - Defaults forced.  Action Required is ");

            SEnvCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );
          } else {
            Print (L" - Force of defaults failed.  Status = %r\n",Status);
          }
        } else if (ValidateOptions) {
          Status = DriverConfiguration->OptionsValid (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL
                                          );

          if (!EFI_ERROR (Status)) {
            Print (L" - Options valid\n");
          } else {
            Print (L" - Options not valid. Status = %r\n", Status);
          }
        } else if (SetOptions) {
          ST->ConOut->ClearScreen(ST->ConOut);
          Status = DriverConfiguration->SetOptions (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL,
                                          Language,
                                          &ActionRequired
                                          );
          ST->ConOut->ClearScreen(ST->ConOut);
          Print(
            L"  Drv[%h02x]  Ctrl[%h02x]  Lang[%ha]", 
            SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), 
            SEnvGetHandleNumber (HandleBuffer[HandleIndex]),
            DriverConfiguration->SupportedLanguages
            );
          if (!EFI_ERROR (Status)) {
            Print (L" - Options set.  Action Required is ");

            SEnvCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );

          } else {
            Print (L" - Options not set. Status = %r\n", Status);
          }
        } else {
          Print (L"\n");
        }
      }

      if (ChildHandle == NULL && !AllChildren) {
        continue;
      }

      Status = LibScanHandleDatabase (
                 DriverImageHandleBuffer[Index], 
                 NULL, 
                 HandleBuffer[HandleIndex], 
                 NULL, 
                 &ChildHandleCount, 
                 &ChildHandleBuffer, 
                 &ChildHandleType
                 );
      if (EFI_ERROR (Status)) {
        continue;
      }

      for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {

        if ((ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_CHILD_HANDLE) != EFI_HANDLE_TYPE_CHILD_HANDLE) {
          continue;
        }

        if (ChildHandle != NULL && ChildHandle != ChildHandleBuffer[ChildIndex]) {
          continue;
        }

        Print(
          L"  Drv[%h02x]  Ctrl[%h02x]  Child[%h02x]  Lang[%ha]", 
          SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), 
          SEnvGetHandleNumber (HandleBuffer[HandleIndex]),
          SEnvGetHandleNumber (ChildHandleBuffer[ChildIndex]),
          DriverConfiguration->SupportedLanguages
          );

        if (ForceDefaults) {
          Status = DriverConfiguration->ForceDefaults (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex],
                                          DefaultType,
                                          &ActionRequired
                                          );

          if (!EFI_ERROR (Status)) {
            Print (L" - Defaults forced.  Action Required is ");

            SEnvCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              ChildHandleBuffer[ChildIndex],
              ActionRequired
              );

          } else {
            Print (L" - Force of defaults failed.  Status = %r\n",Status);
          }
        } else if (ValidateOptions) {
          Status = DriverConfiguration->OptionsValid (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex]
                                          );

          if (!EFI_ERROR (Status)) {
            Print (L" - Options valid\n");
          } else {
            Print (L" - Options not valid. Status = %r\n", Status);
          }
        } else if (SetOptions) {
          ST->ConOut->ClearScreen(ST->ConOut);
          Status = DriverConfiguration->SetOptions (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex],
                                          Language,
                                          &ActionRequired
                                          );
          ST->ConOut->ClearScreen(ST->ConOut);
          Print(
            L"  Drv[%h02x]  Ctrl[%h02x]  Child[%h02x]  Lang[%ha]", 
            SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), 
            SEnvGetHandleNumber (HandleBuffer[HandleIndex]),
            SEnvGetHandleNumber (ChildHandleBuffer[ChildIndex]),
            DriverConfiguration->SupportedLanguages
            );
          if (!EFI_ERROR (Status)) {
            Print (L" - Options set.  Action Required is ");

            SEnvCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              ChildHandleBuffer[ChildIndex],
              ActionRequired
              );

          } else {
            Print (L" - Options not set. Status = %r\n", Status);
          }
        } else {
          Print (L"\n");
        }
      }
      BS->FreePool (ChildHandleBuffer);
      BS->FreePool (ChildHandleType);
    }
    BS->FreePool (HandleBuffer);
    BS->FreePool (HandleType);
  }

Done:
  //------------------2003-08-11------------------------------------
  if(DriverHandleBufferAlloc){
      FreePool (DriverImageHandleBuffer);
  }
  //----------------------------------------------------------------
  FreePool (Language);
  SEnvFreeHandleTable ();
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvCmdDriverRunDiagnostics (
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL   *DriverDiagnostics,
  EFI_HANDLE                        ControllerHandle,
  EFI_HANDLE                        ChildHandle,
  EFI_DRIVER_DIAGNOSTIC_TYPE        DiagnosticType,
  CHAR8                             *Language
  )

{
  EFI_STATUS  Status;
  EFI_GUID    *ErrorType;
  UINTN       BufferSize;
  CHAR16      *Buffer;

  ErrorType  = NULL;
  BufferSize = 0;
  Buffer     = NULL;
  Status = DriverDiagnostics->RunDiagnostics (
                                DriverDiagnostics,
                                ControllerHandle,
                                ChildHandle,
                                DiagnosticType,
                                Language,
                                &ErrorType,
                                &BufferSize,
                                &Buffer
                                );

  if (!EFI_ERROR (Status)) {
    Print (L" - %HPASSED%N\n");
  } else {
    Print (L" - %HFAILED%N with %r\n", Status);
  }
  if (ErrorType != NULL) {
    Print (L"    ErrorType = %g\n", ErrorType);
  }
  if (BufferSize > 0) {
    Print (L"    BufferSize = %d\n", BufferSize);
    Print (L"    Buffer = %s\n",Buffer);
    DumpHex (2, 0, BufferSize, Buffer);
    BS->FreePool(Buffer);
  }
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvCmdDriverDiagnostics (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                        Status;
  CHAR16                            *Ptr;
  UINTN                             HandleNumber;
  EFI_HANDLE                        DriverImageHandle;
  EFI_HANDLE                        DeviceHandle;
  EFI_HANDLE                        ChildHandle;
  UINTN                             StringIndex;
  UINTN                             Index;
  CHAR8                             *Language;
  CHAR8                             *SupportedLanguages;
  EFI_DRIVER_DIAGNOSTIC_TYPE        DiagnosticType;
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL   *DriverDiagnostics;
  UINTN                             DriverImageHandleCount;
  EFI_HANDLE                        *DriverImageHandleBuffer;
  BOOLEAN                           RunDiagnostics;
  BOOLEAN                           DiagnoseAllChildren;
  UINTN                             HandleCount;
  EFI_HANDLE                        *HandleBuffer;
  UINT32                            *HandleType;
  UINTN                             HandleIndex;
  UINTN                             ChildIndex;
  UINTN                             ChildHandleCount;
  EFI_HANDLE                        *ChildHandleBuffer;
  UINT32                            *ChildHandleType;
  BOOLEAN                           DriverHandleBufferAlloc;

  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  DriverImageHandle      = NULL;
  DeviceHandle           = NULL;
  ChildHandle            = NULL;
  RunDiagnostics         = FALSE;
  DiagnosticType         = EfiDriverDiagnosticTypeStandard;
  DiagnoseAllChildren    = FALSE;
  DriverHandleBufferAlloc= FALSE;

  Language = LibGetVariable (VarLanguage, &EfiGlobalVariable);
  if (Language == NULL) {
    Language = AllocatePool(4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'l' :
      case 'L' :
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            Language[StringIndex] = (CHAR8)Ptr[StringIndex + 2];
          }
          Language [StringIndex] = 0;
        }
        break;

      case 'c' :
      case 'C' :
        DiagnoseAllChildren = TRUE;
        break;

      case 's' :
      case 'S' :
        DiagnosticType = EfiDriverDiagnosticTypeStandard;
        RunDiagnostics = TRUE;
        break;

      case 'e' :
      case 'E' :
        DiagnosticType = EfiDriverDiagnosticTypeExtended;
        RunDiagnostics = TRUE;
        break;

      case 'm' :
      case 'M' :
        DiagnosticType = EfiDriverDiagnosticTypeManufacturing;
        RunDiagnostics = TRUE;
        break;

      default:
        Print (L"drvdiag: Unkown flag %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      HandleNumber = SEnvHandleNoFromStr (Ptr);
      if (HandleNumber == 0) {
        Print (L"drvdiag: Invalid handle %s\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      if (DriverImageHandle == NULL) {
        DriverImageHandle = SEnvHandles[HandleNumber - 1];
      } else if (DeviceHandle== NULL) {
        DeviceHandle = SEnvHandles[HandleNumber - 1];
      } else if (ChildHandle== NULL) {
        ChildHandle = SEnvHandles[HandleNumber - 1];
      } else {
        Print (L"drvdiag: Too many arguments\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  }

  if (RunDiagnostics) {
    Print (L"Run Diagnostics\n");
  } else {
    Print (L"Available Diagnostics\n");
  }

  //
  // Display all handles that support being diagnosed
  //
  if (DriverImageHandle == NULL) {
    Status = LibLocateHandle (
               ByProtocol,
               &gEfiDriverDiagnosticsProtocolGuid,
               NULL,
               &DriverImageHandleCount,
               &DriverImageHandleBuffer
               );
    if (EFI_ERROR (Status)) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
	//-----------------2003-08-08-------------------------
	DriverHandleBufferAlloc = TRUE;
	//----------------------------------------------------
  } else {
    DriverImageHandleCount  = 1;
    DriverImageHandleBuffer = &DriverImageHandle;
	//-----------------2003-08-08-------------------------
	DriverHandleBufferAlloc = FALSE;
	//----------------------------------------------------
  }
                                   
  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    Status = BS->OpenProtocol (
                   DriverImageHandleBuffer[Index],
                   &gEfiDriverDiagnosticsProtocolGuid,
                   (VOID **)&DriverDiagnostics,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
    if (EFI_ERROR (Status)) {
      Print (L"drvdiag: Driver image handle [%h02x] does not support Driver Diagnostics Protocol\n", SEnvGetHandleNumber (DriverImageHandleBuffer[Index]));
      continue;
    }

    Status = EFI_NOT_FOUND;
    for (SupportedLanguages = DriverDiagnostics->SupportedLanguages; SupportedLanguages[0] != 0; SupportedLanguages += 3) {
      if (CompareMem (SupportedLanguages, Language, 3) == 0) {
        Status = EFI_SUCCESS;
      }
    }
    if (EFI_ERROR (Status)) {
      Print (L"drvdiag: Driver image handle [%h02x] does not support language %a\n", 
             SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), Language);
      continue;
    }

    Status = LibScanHandleDatabase (
               DriverImageHandleBuffer[Index], 
               NULL,
               NULL, 
               NULL, 
               &HandleCount, 
               &HandleBuffer, 
               &HandleType
               );
    if (EFI_ERROR (Status)) {
      continue;
    }

    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

      if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) != EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {
        continue;
      }

      if (DeviceHandle != NULL && DeviceHandle != HandleBuffer[HandleIndex]) {
        continue;
      }

      if (ChildHandle == NULL) {
        Print(
          L"  Drv[%h02x]  Ctrl[%h02x]  Lang[%ha]", 
          SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), 
          SEnvGetHandleNumber (HandleBuffer[HandleIndex]),
          DriverDiagnostics->SupportedLanguages
          );

        if (RunDiagnostics) {

          SEnvCmdDriverRunDiagnostics (
            DriverDiagnostics,
            HandleBuffer[HandleIndex],
            NULL,
            DiagnosticType,
            Language
            );

        } else {
          Print (L"\n");
        }
      }

      if (ChildHandle == NULL && !DiagnoseAllChildren) {
        continue;
      }

      Status = LibScanHandleDatabase (
                 DriverImageHandleBuffer[Index], 
                 NULL, 
                 HandleBuffer[HandleIndex], 
                 NULL, 
                 &ChildHandleCount, 
                 &ChildHandleBuffer, 
                 &ChildHandleType
                 );
      if (EFI_ERROR (Status)) {
        continue;
      }

      for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {

        if ((ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_CHILD_HANDLE) != EFI_HANDLE_TYPE_CHILD_HANDLE) {
          continue;
        }

        if (ChildHandle != NULL && ChildHandle != ChildHandleBuffer[ChildIndex]) {
          continue;
        }

        Print(
          L"  Drv[%h02x]  Ctrl[%h02x]  Child[%h02x]  Lang[%ha]", 
          SEnvGetHandleNumber (DriverImageHandleBuffer[Index]), 
          SEnvGetHandleNumber (HandleBuffer[HandleIndex]),
          SEnvGetHandleNumber (ChildHandleBuffer[ChildIndex]),
          DriverDiagnostics->SupportedLanguages
          );

        if (RunDiagnostics) {

          SEnvCmdDriverRunDiagnostics (
            DriverDiagnostics,
            HandleBuffer[HandleIndex],
            ChildHandleBuffer[ChildIndex],
            DiagnosticType,
            Language
            );

        } else {
          Print (L"\n");
        }
      }
      BS->FreePool (ChildHandleBuffer);
      BS->FreePool (ChildHandleType);
    }
    BS->FreePool (HandleBuffer);
    BS->FreePool (HandleType);
  }

Done:
  if(DriverHandleBufferAlloc){
	  FreePool(DriverImageHandleBuffer);
	  }
  FreePool (Language);
  SEnvFreeHandleTable ();
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvCmdDevices (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                        Status;
  EFI_STATUS                        ConfigurationStatus;
  EFI_STATUS                        DiagnosticsStatus;
  CHAR16                            *Ptr;
  UINTN                             StringIndex;
  UINTN                             Index;
  CHAR8                             *Language;
  UINTN                             DeviceHandleCount;
  EFI_HANDLE                        *DeviceHandleBuffer;
  UINTN                             DriverBindingHandleCount;
  EFI_HANDLE                        *DriverBindingHandleBuffer;
  UINTN                             ParentControllerHandleCount;
  EFI_HANDLE                        *ParentControllerHandleBuffer;
  UINTN                             ChildControllerHandleCount;
  EFI_HANDLE                        *ChildControllerHandleBuffer;
  CHAR16                            *BestDeviceName;

  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  Language = LibGetVariable (VarLanguage, &EfiGlobalVariable);
  if (Language == NULL) {
    Language = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'l' :
      case 'L' :
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            Language[StringIndex] = (CHAR8)Ptr[StringIndex + 2];
          }
          Language [StringIndex] = 0;
        }
        break;

      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      default:
        Print (L"devices: Unkown flag %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      Print (L"devices: Unkown parameter %hs\n", Ptr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // Display all device handles
  //
  Status = LibLocateHandle (
             AllHandles,
             NULL,
             NULL,
             &DeviceHandleCount,
             &DeviceHandleBuffer
             );
  if (EFI_ERROR (Status)) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  Print (L"%HC  T   D%N\n");
  Print (L"%HT  Y C I%N\n");
  Print (L"%HR  P F A%N\n");
  Print (L"%HL  E G G #P #D #C Device Name%N\n");
  Print (L"%H== = = = == == == =============================================================%N\n");

  for (Index = 0; Index < DeviceHandleCount; Index++) {

    Status = GetDeviceName (
               DeviceHandleBuffer[Index],
               TRUE,
               TRUE,
               Language,
               &BestDeviceName,
               &ConfigurationStatus,
               &DiagnosticsStatus,
               FALSE,
               0
               );

    //
    // Retrieve the best name for DeviceHandleBuffer[Index].  Also determine if
    // the device support being configured, or having diagnostics run on it.
    // Also determine the number of parents controllers, the number of managing
    // drivers, and the number of children.
    //
    Status = LibGetManagingDriverBindingHandles (
               DeviceHandleBuffer[Index],
               &DriverBindingHandleCount,
               &DriverBindingHandleBuffer
               );

    Status = LibGetParentControllerHandles (
               DeviceHandleBuffer[Index],
               &ParentControllerHandleCount,
               &ParentControllerHandleBuffer
               );

    Status = LibGetChildControllerHandles (
               DeviceHandleBuffer[Index],
               &ChildControllerHandleCount,
               &ChildControllerHandleBuffer
               );

    if (DriverBindingHandleCount > 0 || ParentControllerHandleCount > 0 || ChildControllerHandleCount > 0) {
      Print (L"%h02x", SEnvGetHandleNumber (DeviceHandleBuffer[Index]));

      if (ParentControllerHandleCount == 0) {
        Print (L" R");
      } else if (ChildControllerHandleCount > 0) {
        Print (L" B");
      } else {
        Print (L" D");
      }

      if (!EFI_ERROR (ConfigurationStatus)) {
        Print (L" X");
      } else {
        Print (L" -");
      }

      if (!EFI_ERROR (DiagnosticsStatus)) {
        Print (L" X");
      } else {
        Print (L" -");
      }

      if (ParentControllerHandleCount == 0) {
        Print (L"  -");
      } else {
        Print (L" %2d", ParentControllerHandleCount);
      }
      if (DriverBindingHandleCount == 0) {
        Print (L"  -");
      } else {
        Print (L" %2d", DriverBindingHandleCount);
      }
      if (ChildControllerHandleCount == 0) {
        Print (L"  -");
      } else {
        Print (L" %2d", ChildControllerHandleCount);
      }

      if (BestDeviceName == NULL) {
        Print (L" <UNKNOWN>\n");
      } else {
        Print (L" %s\n", BestDeviceName);
      }
    }

    if (DriverBindingHandleBuffer) {
      BS->FreePool (DriverBindingHandleBuffer);
    }
    if (ParentControllerHandleBuffer) {
      BS->FreePool (ParentControllerHandleBuffer);
    }
    if (ChildControllerHandleBuffer) {
      BS->FreePool (ChildControllerHandleBuffer);
    }
    //--------------2003-08-08---------------------
    if(BestDeviceName){
      BS->FreePool(BestDeviceName);
    }
	//---------------------------------------------
  }

  BS->FreePool (DeviceHandleBuffer);

Done:
  FreePool (Language);
  SEnvFreeHandleTable ();
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvCmdDrivers (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                        Status;
  EFI_STATUS                        DiagnosticsStatus;
  EFI_STATUS                        ConfigurationStatus;
  CHAR16                            *Ptr;
  UINTN                             StringIndex;
  UINTN                             Index;
  CHAR8                             *Language;
  UINTN                             DriverImageHandleCount;
  EFI_HANDLE                        *DriverImageHandleBuffer;
  UINTN                             HandleIndex;
  EFI_DRIVER_BINDING_PROTOCOL       *DriverBinding;
  EFI_LOADED_IMAGE_PROTOCOL         *LoadedImage;
  EFI_COMPONENT_NAME_PROTOCOL       *ComponentName;
  CHAR16                            *DriverName;
  CHAR16                            FormattedDriverName[40];
  UINTN                             NumberOfChildren;
  UINTN                             ControllerHandleCount;
  EFI_HANDLE                        *ControllerHandleBuffer;
  UINTN                             ChildControllerHandleCount;
  CHAR8                             *PdbFileName;

  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  Language = LibGetVariable (VarLanguage, &EfiGlobalVariable);
  if (Language == NULL) {
    Language = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'l' :
      case 'L' :
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            Language[StringIndex] = (CHAR8)Ptr[StringIndex + 2];
          }
          Language [StringIndex] = 0;
        }
        break;

      case 'b' :
      case 'B' :
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      default:
        Print (L"drivers: Unkown flag %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      Print (L"drivers: Unkown parameter %hs\n", Ptr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // Display all driver handles
  //
  Status = LibLocateHandle (
             ByProtocol,
             &gEfiDriverBindingProtocolGuid,
             NULL,
             &DriverImageHandleCount,
             &DriverImageHandleBuffer
             );
  if (EFI_ERROR (Status)) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  Print (L"%H            T   D%N\n");
  Print (L"%HD           Y C I%N\n");
  Print (L"%HR           P F A%N\n");
  Print (L"%HV  VERSION  E G G #D #C DRIVER NAME                         IMAGE NAME%N\n");
  Print (L"%H== ======== = = = == == =================================== ===================%N\n");
                                   
  for (Index = 0; Index < DriverImageHandleCount; Index++) {

    Status = BS->OpenProtocol (
                   DriverImageHandleBuffer[Index],
                   &gEfiDriverBindingProtocolGuid,
                   (VOID **)&DriverBinding,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
    if (EFI_ERROR (Status)) {
      continue;
    }

    LoadedImage = NULL;
    Status = BS->OpenProtocol (
                   DriverBinding->ImageHandle,
                   &gEfiLoadedImageProtocolGuid,
                   (VOID **)&LoadedImage,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );

    ComponentName = NULL;
    Status = BS->OpenProtocol (
                   DriverImageHandleBuffer[Index],
                   &gEfiComponentNameProtocolGuid,
                   (VOID **)&ComponentName,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );

    DiagnosticsStatus = BS->OpenProtocol (
                              DriverImageHandleBuffer[Index],
                              &gEfiDriverDiagnosticsProtocolGuid,
                              NULL,
                              NULL,
                              NULL,
                              EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                              );

    ConfigurationStatus = BS->OpenProtocol (
                                DriverImageHandleBuffer[Index],
                                &gEfiDriverConfigurationProtocolGuid,
                                NULL,
                                NULL,
                                NULL,
                                EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                                );

    NumberOfChildren = 0;
    Status = LibGetManagedControllerHandles (
               DriverImageHandleBuffer[Index],
               &ControllerHandleCount,
               &ControllerHandleBuffer
               );
    if (!EFI_ERROR (Status)) {
      for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
        Status = LibGetManagedChildControllerHandles (
                   DriverImageHandleBuffer[Index],
                   ControllerHandleBuffer[HandleIndex],
                   &ChildControllerHandleCount,
                   NULL
                   );
        NumberOfChildren += ChildControllerHandleCount;
      }
      BS->FreePool (ControllerHandleBuffer);
    }

    Print (L"%h02x %08x", 
           SEnvGetHandleNumber (DriverImageHandleBuffer[Index]),
           DriverBinding->Version);

    if (NumberOfChildren > 0) {
      Print (L" B");    
    } else if (ControllerHandleCount > 0) {
      Print (L" D");
    } else {
      Print (L" ?");
    }

    if (!EFI_ERROR (ConfigurationStatus)) {
      Print (L" X");
    } else {
      Print (L" -");
    }

    if (!EFI_ERROR (DiagnosticsStatus)) {
      Print (L" X");
    } else {
      Print (L" -");
    }

    if (ControllerHandleCount == 0) {
      Print (L"  -");
    } else {
      Print(L" %2d", ControllerHandleCount);
    }

    if (NumberOfChildren == 0) {
      Print (L"  -");
    } else {
      Print(L" %2d", NumberOfChildren);
    }

    DriverName = L"<UNKNOWN>";
    if (ComponentName != NULL) {
      Status = ComponentName->GetDriverName (
                                ComponentName, 
                                Language, 
                                &DriverName
                                );
      if (EFI_ERROR (Status)) {
        DriverName = L"<UNKNOWN>";
      }
    }
    for (StringIndex = 0; StringIndex < StrLen(DriverName) && StringIndex < 35; StringIndex++) {
      FormattedDriverName[StringIndex] = DriverName[StringIndex];
    }
    for (StringIndex = StrLen(DriverName); StringIndex < 35; StringIndex++) {
      FormattedDriverName[StringIndex] = L' ';
    }
    FormattedDriverName[35] = 0;
    Print(L" %s ", FormattedDriverName);

    if (LoadedImage == NULL) {
      Print (L"<UNKNOWN>\n");
    } else {
      PdbFileName = GetPdbPath (LoadedImage->ImageBase);
      if (PdbFileName != NULL) {
        PrintShortPdbFileName (PdbFileName, 19);
        Print (L"\n");
      } else {
        DriverName = DevicePathToStr (LoadedImage->FilePath);
        if (StrLen (DriverName) > 19) {
          DriverName[19] = 0;
        }
        Print (L" %s\n", DriverName);
		FreePool(DriverName);
      }
    }
  }

  //----------2003-08-08-------------------------
  FreePool(DriverImageHandleBuffer);
  //---------------------------------------------
Done:
  FreePool (Language);
  SEnvFreeHandleTable ();
  return EFI_SUCCESS;
}

extern EFI_LIST_ENTRY SEnvMap;
extern EFI_LIST_ENTRY SEnvEnv;
extern EFI_LIST_ENTRY SEnvAlias;

EFI_STATUS
SEnvLoadDefaults (
  IN EFI_HANDLE           Image,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY          DefCmds;
  POOL_PRINT              Path;
  DEFAULT_CMD             *Cmd;
  PROTOCOL_INFO           *ProtFs;
  PROTOCOL_INFO           *ProtBlkIo;
  UINTN                   Index;
  UINTN                   HandleNo;
  CHAR16                  *DefaultMapping;

  InitializeShellApplication (Image, SystemTable);

  ZeroMem (&Path, sizeof(Path));

  //
  // If we have some settings, use those
  //
  if (!IsListEmpty(&SEnvMap) && !IsListEmpty(&SEnvEnv) && !IsListEmpty(&SEnvAlias)) {
    goto Done;
  }

  //
  // There are no settings, build some defaults
  //
  InitializeListHead (&DefCmds);

  AcquireLock (&SEnvLock);
  SEnvLoadHandleTable();
  SEnvLoadHandleProtocolInfo (NULL);
  AcquireLock (&SEnvGuidLock);
  ProtFs = SEnvGetProtByStr(L"fs");
  ProtBlkIo = SEnvGetProtByStr(L"blkio");
  ReleaseLock (&SEnvGuidLock);

  //
  // Run all the devices that support a File System and add a default
  // mapping and path setting for each device
  //
  CatPrint (&Path, L"set -v path .");
  if (ProtFs != NULL) {
    for (Index = 0; Index < ProtFs->NoHandles; Index++) {
      for (HandleNo = 0; HandleNo < SEnvNoHandles; HandleNo++) {
        if (SEnvHandles[HandleNo] == ProtFs->Handles[Index]) {
          break;
        }
      }
      HandleNo += 1;
  
      Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
      Cmd->Line = Cmd->Buffer;
      Cmd->Signature = DEFAULT_CMD_SIGNATURE;
      SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map fs%x %x", Index, HandleNo);
      InsertTailList(&DefCmds, &Cmd->Link);
  
      //
      // Append this device to the path
      //
      CatPrint (&Path, L";fs%x:\\efi\\tools;fs%x:\\efi\\boot;fs%x:\\;", Index, Index, Index);
    }
  }

  //
  // Run all the devices that support a BlockIo and add a default
  // mapping for the device
  //
  if (ProtBlkIo != NULL) {
    for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
      for (HandleNo = 0; HandleNo < SEnvNoHandles; HandleNo++) {
        if (SEnvHandles[HandleNo] == ProtBlkIo->Handles[Index]) {
          break;
        }
      }
      HandleNo += 1;
  
      Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
      Cmd->Line = Cmd->Buffer;
      Cmd->Signature = DEFAULT_CMD_SIGNATURE;
      SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map blk%x %x", Index, HandleNo);
      InsertTailList(&DefCmds, &Cmd->Link);
    }
  }

  //
  // Release handle table resources & lock
  //
  SEnvFreeHandleTable();
  ReleaseLock (&SEnvLock);

  //
  // Execute all the queue commands
  //
  while (!IsListEmpty(&DefCmds)) {
    Cmd = CR (DefCmds.Flink, DEFAULT_CMD, Link, DEFAULT_CMD_SIGNATURE);
    SEnvExecute (Image, Cmd->Line, TRUE);
    RemoveEntryList (&Cmd->Link);
    FreePool (Cmd);
  }

  SEnvExecute (Image, Path.str, TRUE);
  SEnvExecute (Image, L"alias -v dir ls", TRUE);
  SEnvExecute (Image, L"alias -v md mkdir", TRUE);
  SEnvExecute (Image, L"alias -v rd rm", TRUE);
  SEnvExecute (Image, L"alias -v del rm", TRUE);
  SEnvExecute (Image, L"alias -v copy cp", TRUE);

Done:
  //
  // Change the current device to default mapping
  //
  DefaultMapping = SEnvGetDefaultMapping(Image);

  if (DefaultMapping!=NULL) {
    ZeroMem (&Path, sizeof(Path));
    CatPrint(&Path,L"%s:",DefaultMapping);
    SEnvExecute (Image, Path.str, TRUE);
  }

  if (Path.str) {
    FreePool (Path.str);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
SEnvReloadDefaults (
  IN EFI_HANDLE           Image,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN OUT BOOLEAN          *MappingModified
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY          DefCmds;
  POOL_PRINT              Path;
  DEFAULT_CMD             *Cmd;
  PROTOCOL_INFO           *ProtFs;
  PROTOCOL_INFO           *ProtBlkIo;
  UINTN                   Index;
  UINTN                   HandleNo;
  BOOLEAN                 EchoStatus;
  BOOLEAN                 Same;

  EFI_LIST_ENTRY          *Link;
  EFI_LIST_ENTRY          *Head;
  MAPPING_INFO            *MappingInfo;
  VARIABLE_ID             *Var;

  InitializeShellApplication (Image, SystemTable);

  Head = &SEnvMap;

  //
  // There are no settings, build some defaults
  //
  InitializeListHead (&DefCmds);
  ZeroMem (&Path, sizeof(Path));

  AcquireLock (&SEnvLock);
  SEnvLoadHandleTable();
  SEnvLoadHandleProtocolInfo (NULL);

  AcquireLock (&SEnvGuidLock);
  ProtBlkIo = SEnvGetProtByStr(L"blkio");
  ReleaseLock (&SEnvGuidLock);

  //
  // Check for media change for every block io
  //
  for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
    EFI_STATUS Status;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;

    Status = BS->HandleProtocol (ProtBlkIo->Handles[Index], 
                                 &gEfiBlockIoProtocolGuid, (VOID **)&BlkIo);
    if (!EFI_ERROR(Status)) {
      BlkIo->FlushBlocks (BlkIo);
    }
  }

  for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
    EFI_STATUS Status;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    VOID *Buffer;

    Status = BS->HandleProtocol (ProtBlkIo->Handles[Index],
                                 &gEfiBlockIoProtocolGuid, (VOID **)&BlkIo);
    if (!EFI_ERROR(Status)) {
      if (!BlkIo->Media->LogicalPartition) {
        BOOLEAN MediaPresent;
        UINT32 MediaId;
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
        MediaPresent = BlkIo->Media->MediaPresent;
        MediaId = BlkIo->Media->MediaId;
        Buffer = AllocatePool (BlkIo->Media->BlockSize);
        BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 0, 
                           BlkIo->Media->BlockSize, Buffer);
        FreePool (Buffer);
        FileSystem = NULL;
        //
        // If the handle contains both blockio protocol and file system
        // protocol, mostly it is a floppy. If only contains blockio protocol,
        // this block is raw block, need partition driver to detect partition
        // blocks. In this case, we need to reinstall that blockio protocol
        // interfaces to let partition driver detect media and install
        // partition blocks on that.
        //
        Status = BS->HandleProtocol (ProtBlkIo->Handles[Index], 
                                     &gEfiSimpleFileSystemProtocolGuid, 
                                     (VOID **)&FileSystem);

        BS->ReinstallProtocolInterface (ProtBlkIo->Handles[Index], 
                                        &gEfiBlockIoProtocolGuid, 
                                        BlkIo, BlkIo);
      }
    }
  }

  //
  // New code, try to make mapping list sequence unchanged if current device 
  // paths and original device paths are the same
  //
  Same = FALSE;

  if (!MappingModified) {
    SEnvGetDevicePathList (&SEnvCurFsDevicePaths, &gEfiSimpleFileSystemProtocolGuid);
    SEnvGetDevicePathList (&SEnvCurBlkDevicePaths, &gEfiBlockIoProtocolGuid);

    //
    // Compare original & current devicepath list for both blkio and fs
    //
    Same = SEnvCompareDevicePathList (&SEnvOrgFsDevicePaths, 
                                      &SEnvCurFsDevicePaths);
    if (Same) {
      Same = SEnvCompareDevicePathList (&SEnvOrgBlkDevicePaths, 
                                        &SEnvCurBlkDevicePaths);
    }
  }

  MappingModified = FALSE;

  if (Same) {
    //
    // Since all devicepath for blkio and fs are the same
    // we don't need to do remapping
    //

    //
    // Set path
    //
    SEnvLoadHandleTable();
    SEnvLoadHandleProtocolInfo (NULL);
    AcquireLock (&SEnvGuidLock);
    ProtFs = SEnvGetProtByStr(L"fs");
    ReleaseLock (&SEnvGuidLock);

    CatPrint (&Path, L"set -v path .");
    if (ProtFs != NULL) {
      for (Index = 0; Index < ProtFs->NoHandles; Index++) {
        //
        // Append this device to the path
        //
        CatPrint (&Path, L";fs%x:\\efi\\tools;fs%x:\\efi\\boot;fs%x:\\", 
                  Index, Index, Index);
      }
    }

    //
    // make all mapping info be valid and not-accessed
    //
    for (Link = SEnvCurMapping.Flink; Link != &SEnvCurMapping; Link = Link->Flink) {
      MappingInfo = CR (Link, MAPPING_INFO, Link, MAPPING_INFO_SIGNATURE);
      MappingInfo->Accessed = FALSE;
      MappingInfo->Valid = TRUE;
    }
  } else {
    //
    // Remove old SEnvMap Info
    //
    for (Link = Head->Flink; Link != Head;) {
      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      RT->SetVariable (Var->Name, &SEnvMapId, 0, 0, NULL);
      Link = Link->Flink;
      RemoveEntryList (&Var->Link);
      FreePool (Var);
    }

    //
    // Load handle table and handle protocol info so that we can get
    // the updated info if media changed
    //
    SEnvLoadHandleTable();
    SEnvLoadHandleProtocolInfo (NULL);
    AcquireLock (&SEnvGuidLock);
    ProtFs = SEnvGetProtByStr(L"fs");
    ProtBlkIo = SEnvGetProtByStr(L"blkio");
    ReleaseLock (&SEnvGuidLock);

    //
    // Run all the devices that support a File System and add a default
    // mapping and path setting for each device
    //
    CatPrint (&Path, L"set -v path .");
    if (ProtFs != NULL) {
      for (Index = 0; Index < ProtFs->NoHandles; Index++) {
        for (HandleNo = 0; HandleNo < SEnvNoHandles; HandleNo++) {
          if (SEnvHandles[HandleNo] == ProtFs->Handles[Index]) {
            break;
          }
        }
        HandleNo += 1;
  
        Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
        Cmd->Line = Cmd->Buffer;
        Cmd->Signature = DEFAULT_CMD_SIGNATURE;
        SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map fs%x %x", Index, HandleNo);
        InsertTailList(&DefCmds, &Cmd->Link);
  
        //
        // Append this device to the path
        //
        CatPrint (&Path, L";fs%x:\\efi\\tools;fs%x:\\efi\\boot;fs%x:\\", 
                  Index, Index, Index);
      }
    }

    //
    // Run all the devices that support a BlockIo and add a default
    // mapping for the device
    //
    if (ProtBlkIo != NULL) {
      for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
        for (HandleNo = 0; HandleNo < SEnvNoHandles; HandleNo++) {
          if (SEnvHandles[HandleNo] == ProtBlkIo->Handles[Index]) {
            break;
          }
        }
        HandleNo += 1;
  
        Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
        Cmd->Line = Cmd->Buffer;
        Cmd->Signature = DEFAULT_CMD_SIGNATURE;
        SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map blk%x %x", Index, HandleNo);
        InsertTailList(&DefCmds, &Cmd->Link);
      }
    }
  }

  //
  // Release handle table resources & lock
  //
  SEnvFreeHandleTable();
  ReleaseLock (&SEnvLock);

  //
  // Set Echo Status to off, to avoid junk output if the it is executed in 
  // script file
  //
  EchoStatus = SEnvBatchGetEcho();
  SEnvBatchSetEcho(FALSE);

  //
  // Execute all the queue commands
  //
  while (!IsListEmpty(&DefCmds)) {
    Cmd = CR (DefCmds.Flink, DEFAULT_CMD, Link, DEFAULT_CMD_SIGNATURE);
    SEnvExecute (Image, Cmd->Line, TRUE);
    RemoveEntryList (&Cmd->Link);
    FreePool (Cmd);
  }

  if (Path.str) {
    SEnvExecute (Image, Path.str, TRUE);
  }

  //
  // Restore original echo status
  //
  SEnvBatchSetEcho(EchoStatus);

  if (Path.str) {
    FreePool (Path.str);
  }

  //
  // Remember current device paths
  //
  SEnvGetDevicePathList (&SEnvOrgFsDevicePaths, &gEfiSimpleFileSystemProtocolGuid);
  SEnvGetDevicePathList (&SEnvOrgBlkDevicePaths, &gEfiBlockIoProtocolGuid);

  return EFI_SUCCESS;
}


EFI_STATUS
SEnvCmdUnload (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "Unload" command

Arguments:

Returns:

--*/
{
  BOOLEAN                      Prompt;
  BOOLEAN                      Verbose;
  CHAR16                       *Arg;
  CHAR16                       *Ptr;
  EFI_STATUS                   Status;
  UINTN                        Index;
  CHAR16                       ReturnStr[2];
  EFI_LOADED_IMAGE_PROTOCOL    *ImageInfo;
  EFI_HANDLE                   TargetHandle;

  //
  // Initializing variable to avoid level 4 warning
  //
  Status = EFI_SUCCESS;

  //
  // Initialize
  //
  InitializeShellApplication (ImageHandle, SystemTable);

  Arg = NULL;
  Prompt = TRUE;
  Verbose = FALSE;

  //
  // Crack args
  //
  if (SI->Argc < 2) {
    Print(L"unload: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'n':
      case 'N':
        Prompt = FALSE;
        break;

      case 'v':
      case 'V':
        Verbose = TRUE;
        break;

      default:
        Print (L"unload: Unkown flag %hs\n", Ptr);
        return EFI_INVALID_PARAMETER;
      }
      continue;
    }

    if (!Arg) {
      Arg = Ptr;
      continue;
    }

    Print (L"unload: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  //
  // Load handle & protocol info tables
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  Index = SEnvHandleNoFromStr(Arg) - 1;

  if (Index > SEnvNoHandles) {
    Print(L"unload: Invalid handle number %hs\n", Arg);
  } else {
    TargetHandle = SEnvHandles[Index];
    SEnvDHProt (Verbose, FALSE, Index+1, TargetHandle, "eng");

    //
    // Make sure it is an image handle to a protocol
    //
    Status = BS->HandleProtocol (TargetHandle, 
                                 &gEfiLoadedImageProtocolGuid, 
                                 (VOID**)&ImageInfo);
    if (EFI_ERROR(Status)) {
      Print (L"unload: Locate LoadedImageProtocel error - %r\n", Status);
    } else if( (ImageInfo->ImageCodeType != EfiBootServicesCode) &&
               (ImageInfo->ImageCodeType != EfiRuntimeServicesCode)) {
      Print(L"unload: Handle index does not specify a protocol image\n");
    } else {
      //
      // Ask permission if needed
      //
      if (Prompt) {
        Input (L"Unload protocol image (y/n)? ", 
               ReturnStr, 
               sizeof(ReturnStr)/sizeof(CHAR16));
        Print(L"\n");
        if (ReturnStr[0] == L'y' || ReturnStr[0] == L'Y') {
          Status = BS->UnloadImage (TargetHandle);
          if (EFI_ERROR(Status)) {
            Print(L"unload: Unload protocol image error - %r\n", Status);
          }
        } else {
          return EFI_ABORTED;
        }
      }
    }
  }
  return Status;
}


EFI_STATUS
SEnvCmdOpenInfo (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
/*++

Routine Description:
  Connect DriverHandle [DeviceHandle]

Arguments:

Returns:

--*/
{
  EFI_STATUS                          Status;
  UINTN                               HandleIndex;
  EFI_HANDLE                          DumpHandle;
  PROTOCOL_INFO                       *Prot;
  EFI_LIST_ENTRY                      *Link;
  UINTN                               Index;
  UINTN                               Count;
  VOID                                *Interface;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenBuffer;
  UINTN                               OpenBufferCount;
  EFI_DRIVER_BINDING_PROTOCOL         *DriverBinding;

  InitializeShellApplication (ImageHandle, SystemTable);

  if (SI->Argc <= 1) {
    Print (L"OpenInfo: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }
  if (SI->Argc > 3) {
    Print (L"OpenInfo: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  HandleIndex = SEnvHandleNoFromStr (SI->Argv[1]);
  if (HandleIndex == 0) {
    Print (L"OpenInfo: Handle %hs out of range\n", SI->Argv[1]);
    return EFI_INVALID_PARAMETER;
  }
  DumpHandle = SEnvHandles[HandleIndex - 1];

  Print (L"%NHandle %h02x (%hX)\n", HandleIndex, DumpHandle);

  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    for (Index = 0; Index < Prot->NoHandles; Index++) {
      if (Prot->Handles[Index] == DumpHandle) {
        //
        // This handle supports this protocol, dump it
        //
        Print (L"%hs \n", Prot->IdString);
        Status = BS->OpenProtocolInformation (DumpHandle, &Prot->ProtocolId, 
                                              &OpenBuffer, &OpenBufferCount);
        if (!EFI_ERROR (Status)) {
          for (Count = 0; Count < OpenBufferCount; Count++) {
            //
            // Print Image info.
            //
            if (SEnvHandleToNumber (OpenBuffer[Count].ControllerHandle) == 0) {
              Print (L"  Drv[%h02x] Ctrl[  ] Cnt(%h02x) ",
                SEnvHandleToNumber (OpenBuffer[Count].AgentHandle),
                OpenBuffer[Count].OpenCount
                );
            } else {
              Print (L"  Drv[%h02x] Ctrl[%h02x] Cnt(%h02x) ",
                SEnvHandleToNumber (OpenBuffer[Count].AgentHandle),
                SEnvHandleToNumber (OpenBuffer[Count].ControllerHandle),
                OpenBuffer[Count].OpenCount
                );
            }
            
            //
            // Handle different conditions
            //
            switch (OpenBuffer[Count].Attributes) {
            case EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL :
              Print(L"%hs", L"HandProt ");
              break;
            case EFI_OPEN_PROTOCOL_GET_PROTOCOL :
              Print(L"%hs", L"GetProt  ");
              break;
            case EFI_OPEN_PROTOCOL_TEST_PROTOCOL :
              Print(L"%hs", L"TestProt ");
              break;
            case EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER :
              Print(L"%hs", L"Child    ");
              break;
            case EFI_OPEN_PROTOCOL_BY_DRIVER :
              Print(L"%hs", L"Driver   ");
              break;
            case EFI_OPEN_PROTOCOL_EXCLUSIVE :
              Print(L"%hs", L"Exclusive");
              break;
            case EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE :
              Print(L"%hs", L"DriverEx ");
              break;
            default :
              Print(L"%hs", L"Unknown  ");
              break;
            }
            
            //
            // Dump image token info
            //
            Status = BS->HandleProtocol (
                           OpenBuffer[Count].AgentHandle, 
                           &gEfiDriverBindingProtocolGuid, 
                           (VOID **)&DriverBinding
                           );
            if (!EFI_ERROR (Status)) {
              Status = BS->HandleProtocol (
                             DriverBinding->ImageHandle, 
                             &gEfiLoadedImageProtocolGuid, 
                             (VOID **)&Interface
                             );
              if (!EFI_ERROR (Status)) {
                SEnvImageTok (DriverBinding->ImageHandle, Interface);
              }
            }

            Print(L"\n");
          }
          BS->FreePool (OpenBuffer);
        }
      }
    }
  }

  SEnvFreeHandleTable ();
  return EFI_SUCCESS;
}


VOID
SEnvGetDevicePathList (
  EFI_LIST_ENTRY    *ListHead,
  EFI_GUID          *Protocol
  )
/*++

Routine Description:
  Get current device paths by specified protocol

Arguments:

Returns:

--*/
{
  EFI_STATUS                      Status;

  DEVICEPATH_INFO                 *DevicePathInfo;

  UINTN                           Index;
  UINTN                           NoHandles;
  EFI_HANDLE                      *Handles;

  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  UINTN                           DevPathSize;

  //
  // Remove previous device path info
  //
  while (!IsListEmpty (ListHead)) {
    DevicePathInfo = (DEVICEPATH_INFO *)(CR (ListHead->Flink, DEVICEPATH_INFO,
                                         Link, DEVICEPATH_INFO_SIGNATURE));
    FreePool (DevicePathInfo->DevicePath);
    RemoveEntryList (&DevicePathInfo->Link);
    FreePool (DevicePathInfo);
  }

  //
  // Establish current device path info list
  //
  Status = LibLocateHandle (
                            ByProtocol,
                            Protocol,
                            NULL,
                            &NoHandles,
                            &Handles
                            );

  if (NoHandles) {
    for (Index = 0; Index < NoHandles; Index++) {
      DevicePath = DevicePathFromHandle (Handles[Index]);
      if (!DevicePath) {
        continue;
      }

      DevPathSize = DevicePathSize (DevicePath);

      //
      // Copy device path info to the device path list
      //
      DevicePathInfo = AllocateZeroPool (sizeof (DEVICEPATH_INFO));
      DevicePathInfo->DevicePath = AllocateZeroPool (DevPathSize);
      CopyMem (DevicePathInfo->DevicePath, DevicePath, DevPathSize);
      DevicePathInfo->Found = FALSE;
      DevicePathInfo->Signature = DEVICEPATH_INFO_SIGNATURE;
      InsertTailList (ListHead, &DevicePathInfo->Link);
    }
  }
}


BOOLEAN
SEnvCompareDevicePathList (
  EFI_LIST_ENTRY    *ListHead1,
  EFI_LIST_ENTRY    *ListHead2
  )
/*++

Routine Description:
  Compare devicepath list

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY  *Link1;
  EFI_LIST_ENTRY  *Link2;
  
  DEVICEPATH_INFO *DevicePathInfo1;
  DEVICEPATH_INFO *DevicePathInfo2;

  UINTN           DevicePathSize1;
  UINTN           DevicePathSize2;

  if (IsListEmpty (ListHead1) || IsListEmpty (ListHead2)) {
    return FALSE;
  }

  //
  // Compare two device path lists
  //
  for (Link1 = ListHead1->Flink; Link1 != ListHead1; Link1 = Link1->Flink) {
    DevicePathInfo1 = CR (Link1, DEVICEPATH_INFO, Link, DEVICEPATH_INFO_SIGNATURE);
    DevicePathSize1 = DevicePathSize (DevicePathInfo1->DevicePath);
    for (Link2 = ListHead2->Flink; Link2 != ListHead2; Link2 = Link2->Flink) {
      DevicePathInfo2 = CR (Link2, DEVICEPATH_INFO, Link, DEVICEPATH_INFO_SIGNATURE);
      DevicePathSize2 = DevicePathSize (DevicePathInfo2->DevicePath);
      if (DevicePathSize1 != DevicePathSize2) {
        continue;
      }
      if (CompareMem (DevicePathInfo1->DevicePath, DevicePathInfo2->DevicePath, DevicePathSize1) == 0) {
        DevicePathInfo1->Found = TRUE;
        DevicePathInfo2->Found = TRUE;
        break;
      }
    }
    if (DevicePathInfo1->Found == FALSE) {
      //
      // Devicepath node in devicepath1 is not in devicepath2 list,
      // don't need to continue comparing
      //
      return FALSE;
    }
  }

  //
  // All devicepath nodes in devicepath1 list are in devicepath2 list,
  // so we just need to find if there are new device path in devicepath2 list
  //
  for (Link2 = ListHead2->Flink; Link2 != ListHead2; Link2 = Link2->Flink) {
    DevicePathInfo2 = CR (Link2, DEVICEPATH_INFO, Link, 
                          DEVICEPATH_INFO_SIGNATURE);
    if (DevicePathInfo2->Found == FALSE) {
      return FALSE;
    }
  }
  return TRUE;
}
