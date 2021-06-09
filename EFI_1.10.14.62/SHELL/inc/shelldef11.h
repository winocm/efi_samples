/*++

Copyright (c) 2001 - 2002 Intel Corporation

Module Name:

    shell11def.h

Abstract:

    Defines for Shell1.1 migration

Revision History

--*/

#ifndef _SHELL_DEF_11_H_
#define _SHELL_DEF_11_H_

#include "efivar.h"
#include "legacyboot.h"

//
// Inject a break point in the code to assist debugging.. Only for 32 bit environment
// until debugger support is added to 64 bit environment
// For EFI64, just put in a halt loop.
//
#if EFI32
#define EFI_BREAKPOINT()  __asm { int 3 }
#else
#define EFI_BREAKPOINT()  while(TRUE)
#endif

#include EFI_GUID_DEFINITION (ConsoleInDevice)
#include EFI_GUID_DEFINITION (ConsoleOutDevice)
#include EFI_GUID_DEFINITION (GlobalVariable)
#include EFI_GUID_DEFINITION (Gpt)
#include EFI_GUID_DEFINITION (pcansi)
#include EFI_GUID_DEFINITION (PrimaryConsoleInDevice)
#include EFI_GUID_DEFINITION (PrimaryConsoleOutDevice)
#include EFI_GUID_DEFINITION (PrimaryStandardErrorDevice)
#include EFI_GUID_DEFINITION (SalSystemTable)
#include EFI_GUID_DEFINITION (Smbios)
#include EFI_GUID_DEFINITION (StandardErrorDevice)

//
//
//

//
//
//
#ifndef EFI_LIST_ENTRY
  #define EFI_LIST_ENTRY LIST_ENTRY
#endif

#ifndef ForwardLink
  #define ForwardLink Flink
#endif

#ifndef BackwardLink
  #define BackwardLink Blink
#endif

//
// Protocol
//
#ifndef EFI_DEVICE_PATH_PROTOCOL
  #define EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH
#endif

#ifndef EFI_LOADED_IMAGE_PROTOCOL
  #define EFI_LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE
#endif

#ifndef EFI_SIMPLE_TEXT_OUT_PROTOCOL
  #define EFI_SIMPLE_TEXT_OUT_PROTOCOL SIMPLE_TEXT_OUTPUT_INTERFACE
#endif

#ifndef EFI_SIMPLE_TEXT_IN_PROTOCOL
  #define EFI_SIMPLE_TEXT_IN_PROTOCOL SIMPLE_INPUT_INTERFACE
#endif

#ifndef EFI_BLOCK_IO_PROTOCOL
  #define EFI_BLOCK_IO_PROTOCOL EFI_BLOCK_IO
#endif

#ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
  #define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_FILE_IO_INTERFACE
#endif

#ifndef EFI_DEVICE_IO_PROTOCOL
  #define EFI_DEVICE_IO_PROTOCOL EFI_DEVICE_IO_INTERFACE
#endif

//
// Debug info
//
#ifndef EFI_D_ERROR
  #define EFI_D_ERROR D_ERROR
#endif

#ifndef EFI_D_INIT
  #define EFI_D_INIT D_INIT
#endif

#ifndef EFI_D_VARIABLE
  #define EFI_D_VARIABLE D_VARIABLE
#endif

#ifndef EFI_D_WARN
  #define EFI_D_WARN D_WARN
#endif

#ifndef EFI_D_INFO
  #define EFI_D_INFO D_INFO
#endif

#ifndef EFI_D_LOAD
  #define EFI_D_LOAD D_LOAD
#endif

#ifndef EFI_D_EVENT
  #define EFI_D_EVENT D_EVENT
#endif

#ifndef EFI_D_POOL
  #define EFI_D_POOL D_POOL
#endif

#ifndef EFI_D_PAGE
  #define EFI_D_PAGE D_PAGE
#endif

#ifndef EFI_D_NET
  #define EFI_D_NET D_NET
#endif

#ifndef EFI_D_FS
  #define EFI_D_FS D_FS
#endif

#ifndef EFI_D_BLKIO
  #define EFI_D_BLKIO D_BLKIO
#endif

#ifndef EFI_D_UNDI
  #define EFI_D_UNDI D_UNDI
#endif

#ifndef EFI_D_LOADFILE
  #define EFI_D_LOADFILE D_LOADFILE
#endif

#ifndef EFI_D_BM
  #define EFI_D_BM D_BM
#endif

//
// gGUID name
//
#ifndef gEfiLoadedImageProtocolGuid
  #define gEfiLoadedImageProtocolGuid LoadedImageProtocol
#endif

#ifndef gEfiSimpleFileSystemProtocolGuid
  #define gEfiSimpleFileSystemProtocolGuid FileSystemProtocol
#endif

#ifndef gEfiDriverBindingProtocolGuid
  #define gEfiDriverBindingProtocolGuid DriverBindingProtocol
#endif

#ifndef gEfiDevicePathProtocolGuid
  #define gEfiDevicePathProtocolGuid DevicePathProtocol
#endif

#ifndef gEfiSimpleTextOutProtocolGuid
  #define gEfiSimpleTextOutProtocolGuid TextOutProtocol
#endif

#ifndef gEfiSimpleTextInProtocolGuid
  #define gEfiSimpleTextInProtocolGuid TextInProtocol
#endif

#ifndef gEfiBlockIoProtocolGuid
  #define gEfiBlockIoProtocolGuid BlockIoProtocol
#endif

#ifndef gEfiFileSystemInfoGuid
  #define gEfiFileSystemInfoGuid FileSystemInfo
#endif

#ifndef gEfiSalSystemTableGuid
  #define gEfiSalSystemTableGuid SalSystemTableGuid
#endif

#ifndef gEfiAcpiTableGuid
  #define gEfiAcpiTableGuid AcpiTableGuid
#endif

#ifndef gEfiAcpi20TableGuid
  #define gEfiAcpi20TableGuid Acpi20TableGuid
#endif

#ifndef gEfiMpsTableGuid
  #define gEfiMpsTableGuid MpsTableGuid
#endif

#ifndef gEfiSMBIOSTableGuid
  #define gEfiSMBIOSTableGuid SMBIOSTableGuid
#endif

#ifndef gEfiDeviceIoProtocolGuid
  #define gEfiDeviceIoProtocolGuid DeviceIoProtocol
#endif

//
// GUID
//
#ifndef EFI_DEVICE_IO_PROTOCOL_GUID
  #define EFI_DEVICE_IO_PROTOCOL_GUID DEVICE_IO_PROTOCOL
#endif

#ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
  #define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID SIMPLE_FILE_SYSTEM_PROTOCOL
#endif

#ifndef EFI_DISK_IO_PROTOCOL_GUID
  #define EFI_DISK_IO_PROTOCOL_GUID DISK_IO_PROTOCOL
#endif

#ifndef EFI_BLOCK_IO_PROTOCOL_GUID
  #define EFI_BLOCK_IO_PROTOCOL_GUID BLOCK_IO_PROTOCOL
#endif

#ifndef EFI_SIMPLE_TEXT_IN_PROTOCOL_GUID
  #define EFI_SIMPLE_TEXT_IN_PROTOCOL_GUID SIMPLE_TEXT_INPUT_PROTOCOL
#endif

#ifndef EFI_SIMPLE_TEXT_OUT_PROTOCOL_GUID
  #define EFI_SIMPLE_TEXT_OUT_PROTOCOL_GUID SIMPLE_TEXT_OUTPUT_PROTOCOL
#endif

#ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
  #define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID SIMPLE_FILE_SYSTEM_PROTOCOL
#endif

#ifndef LOAD_FILE_PROTOCOL_GUID
  #define LOAD_FILE_PROTOCOL_GUID LOAD_FILE_PROTOCOL
#endif

#ifndef EFI_LOADED_IMAGE_PROTOCOL_GUID
  #define EFI_LOADED_IMAGE_PROTOCOL_GUID LOADED_IMAGE_PROTOCOL
#endif

#ifndef EFI_VARIABLE_STORE_PROTOCOL_GUID
  #define EFI_VARIABLE_STORE_PROTOCOL_GUID VARIABLE_STORE_PROTOCOL
#endif

#ifndef EFI_UNICODE_COLLATION_PROTOCOL_GUID
  #define EFI_UNICODE_COLLATION_PROTOCOL_GUID UNICODE_COLLATION_PROTOCOL
#endif

#ifndef EFI_LEGACY_BOOT_PROTOCOL_GUID
  #define EFI_LEGACY_BOOT_PROTOCOL_GUID LEGACY_BOOT_PROTOCOL
#endif

#ifndef EFI_SERIAL_IO_PROTOCOL_GUID
  #define EFI_SERIAL_IO_PROTOCOL_GUID SERIAL_IO_PROTOCOL
#endif

#ifndef EFI_PXE_BASE_CODE_PROTOCOL_GUID
  #define EFI_PXE_BASE_CODE_PROTOCOL_GUID EFI_PXE_BASE_CODE_PROTOCOL
#endif

#ifndef EFI_SIMPLE_NETWORK_PROTOCOL_GUID
  #define EFI_SIMPLE_NETWORK_PROTOCOL_GUID EFI_SIMPLE_NETWORK_PROTOCOL
#endif

#ifndef EFI_VGA_CLASS_DRIVER_PROTOCOL_GUID
  #define EFI_VGA_CLASS_DRIVER_PROTOCOL_GUID VGA_CLASS_DRIVER_PROTOCOL
#endif

#ifndef EFI_PXE_BASE_CODE_PROTOCOL_GUID
  #define EFI_PXE_BASE_CODE_PROTOCOL_GUID EFI_PXE_BASE_CODE_PROTOCOL
#endif

#ifndef EFI_GLOBAL_VARIABLE_GUID
  #define EFI_GLOBAL_VARIABLE_GUID EFI_GLOBAL_VARIABLE
#endif

#ifndef EFI_FILE_SYSTEM_INFO_ID_GUID
  #define EFI_FILE_SYSTEM_INFO_ID_GUID EFI_FILE_INFO_ID
#endif

#ifndef EFI_DEVICE_PATH_PROTOCOL_GUID
  #define EFI_DEVICE_PATH_PROTOCOL_GUID DEVICE_PATH_PROTOCOL
#endif

//
// Shell environment
//
#ifndef EFI_SIMPLE_TEXT_OUTPUT_MODE
  #define EFI_SIMPLE_TEXT_OUTPUT_MODE SIMPLE_TEXT_OUTPUT_MODE
#endif

#ifndef EFI_UNICODE_BYTE_ORDER_MARK
  #define EFI_UNICODE_BYTE_ORDER_MARK UNICODE_BYTE_ORDER_MARK
#endif

//
// TPL
//
#ifndef EFI_TPL_APPLICATION
  #define EFI_TPL_APPLICATION TPL_APPLICATION
#endif

#ifndef EFI_TPL_NOTIFY
  #define EFI_TPL_NOTIFY TPL_NOTIFY
#endif

//
// Protocol, Guid definition
//

//#include EFI_PROTOCOL_DEFINITION (BlockIo)
#include EFI_PROTOCOL_DEFINITION (BusSpecificDriverOverride)
#include EFI_PROTOCOL_DEFINITION (DebugPort)
#include EFI_PROTOCOL_DEFINITION (DebugSupport)
#include EFI_PROTOCOL_DEFINITION (Decompress)
//#include EFI_PROTOCOL_DEFINITION (DeviceIo)
//#include EFI_PROTOCOL_DEFINITION (DevicePath)
//#include EFI_PROTOCOL_DEFINITION (DiskIo)
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)
#include EFI_PROTOCOL_DEFINITION (Ebc)
//#include EFI_PROTOCOL_DEFINITION (EfiNetworkInterfaceIdentifier)
//#include EFI_PROTOCOL_DEFINITION (LegacyBoot)
//#include EFI_PROTOCOL_DEFINITION (LoadedImage)
//#include EFI_PROTOCOL_DEFINITION (LoadFile)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION (PlatformDriverOverride)
#include EFI_PROTOCOL_DEFINITION (PxeBaseCode)
//#include EFI_PROTOCOL_DEFINITION (SerialIo)
//#include EFI_PROTOCOL_DEFINITION (SimpleFileSystem)
#include EFI_PROTOCOL_DEFINITION (SimplePointer)
//#include EFI_PROTOCOL_DEFINITION (SimpleNetwork)
//#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)
//#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)
//#include EFI_PROTOCOL_DEFINITION (UnicodeCollation)
#include EFI_PROTOCOL_DEFINITION (ScsiPassThru)
#include EFI_PROTOCOL_DEFINITION (UsbIo)
#include EFI_PROTOCOL_DEFINITION (UsbHostController)
#include EFI_PROTOCOL_DEFINITION (usbAtapi)
#include EFI_PROTOCOL_DEFINITION (WinNtThunk)
#include EFI_PROTOCOL_DEFINITION (WinNtIo)
#include EFI_PROTOCOL_DEFINITION (Bis)
#include EFI_PROTOCOL_DEFINITION (IsaAcpi)
#include EFI_PROTOCOL_DEFINITION (IsaIo)
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (UgaIo)
#include EFI_PROTOCOL_DEFINITION (VgaMiniPort)
#endif // _SHELL_DEF_11_H_
