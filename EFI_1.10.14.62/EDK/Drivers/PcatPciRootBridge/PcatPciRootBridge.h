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

  PcatPciRootBridge.h

Abstract:

  The driver for the host to pci bridge (root bridge).

--*/

#ifndef _PCAT_PCI_ROOT_BRIDGE_H_
#define _PCAT_PCI_ROOT_BRIDGE_H_

#include "Efi.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "Acpi.h"

//
// Driver Consumes GUIDs
//
#include EFI_GUID_DEFINITION(PciOptionRomTable)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)

//
// Driver Instance Data Prototypes
//
#define PCAT_PCI_ROOT_BRIDGE_SIGNATURE  EFI_SIGNATURE_32('p', 'c', 'r', 'b')

typedef struct {
  UINT32                            Signature;
  EFI_HANDLE                        Handle;
                                    
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   Io;
                                    
  UINT32                            RootBridgeNumber;
  UINT32                            PrimaryBus;
  UINT32                            SubordinateBus;
                                     
  UINT64                            MemBase;     // Offsets host to bus memory addr.
  UINT64                            MemLimit;    // Max allowable memory access
                                    
  UINT64                            IoBase;      // Offsets host to bus io addr.
  UINT64                            IoLimit;     // Max allowable io access
                                    
  UINT64                            PciAddress;
  UINT64                            PciData;
                                    
  UINT64                            PhysicalMemoryBase;
  UINT64                            PhysicalIoBase;
                                     
  EFI_LOCK                          PciLock;
                                    
  UINT64                            Attributes;
                                    
  UINT64                            Mem32Base;
  UINT64                            Mem32Limit;
  UINT64                            Pmem32Base;
  UINT64                            Pmem32Limit;
  UINT64                            Mem64Base;
  UINT64                            Mem64Limit;
  UINT64                            Pmem64Base;
  UINT64                            Pmem64Limit;

  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Configuration;

  EFI_LIST_ENTRY                    MapInfo;
} PCAT_PCI_ROOT_BRIDGE_INSTANCE;

//
// Driver Instance Data Macros
//
#define DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(a) \
  CR(a, PCAT_PCI_ROOT_BRIDGE_INSTANCE, Io, PCAT_PCI_ROOT_BRIDGE_SIGNATURE)

//
// Private data types
//
typedef union {
  UINT8   VOLATILE  *buf;
  UINT8   VOLATILE  *ui8;
  UINT16  VOLATILE  *ui16;
  UINT32  VOLATILE  *ui32;
  UINT64  VOLATILE  *ui64;
  UINTN   VOLATILE  ui;
} PTR;

typedef struct {
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation;
  UINTN                                      NumberOfBytes;
  UINTN                                      NumberOfPages;
  EFI_PHYSICAL_ADDRESS                       HostAddress;
  EFI_PHYSICAL_ADDRESS                       MappedHostAddress;
} MAP_INFO;

typedef struct {
  EFI_LIST_ENTRY Link;
  MAP_INFO * Map;  
} MAP_INFO_INSTANCE;

typedef
VOID
(*EFI_PCI_BUS_SCAN_CALLBACK) (
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *IoDev,
  UINT16                           MinBus,
  UINT16                           MaxBus,
  UINT16                           MinDevice,
  UINT16                           MaxDevice,
  UINT16                           MinFunc,
  UINT16                           MaxFunc,
  UINT16                           Bus,
  UINT16                           Device,
  UINT16                           Func,
  IN VOID                          *Context
  );

typedef struct {
  UINT16                    *CommandRegisterBuffer;
  UINT32                    PpbMemoryWindow;     
} PCAT_PCI_ROOT_BRIDGE_SCAN_FOR_ROM_CONTEXT;

//
// Driver Protocol Constructor Prototypes
//
EFI_STATUS 
ConstructConfiguration(
  IN OUT PCAT_PCI_ROOT_BRIDGE_INSTANCE  *PrivateData
  );

EFI_STATUS
PcatPciRootBridgeParseBars (
  IN PCAT_PCI_ROOT_BRIDGE_INSTANCE  *PrivateData,
  IN UINT16                         Command,
  IN UINTN                          Bus,
  IN UINTN                          Device,
  IN UINTN                          Function
  );

EFI_STATUS
ScanPciRootBridgeForRoms(
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *IoDev
  );

EFI_STATUS
PcatRootBridgeDevicePathConstructor (
  IN EFI_DEVICE_PATH_PROTOCOL  **Protocol,
  IN UINTN                     RootBridgeNumber
  );

EFI_STATUS
PcatRootBridgeIoConstructor (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *Protocol,
  IN UINTN                            SegmentNumber
  );

EFI_STATUS
PcatRootBridgeIoGetIoPortMapping (
  OUT EFI_PHYSICAL_ADDRESS  *IoPortMapping,
  OUT EFI_PHYSICAL_ADDRESS  *MemoryPortMapping
  );

EFI_STATUS
PcatRootBridgeIoPciRW (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN BOOLEAN                                Write,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 UserAddress,
  IN UINTN                                  Count,
  IN OUT VOID                               *UserBuffer
  );

//
// Driver entry point prototype
//
EFI_STATUS
InitializePcatPciRootBridge (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );

#endif
