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

  WinNtPciRootBridge.h

Abstract:

  EFI WinNt Emulated PCI Root Bridge Controller

--*/

#ifndef _WIN_NT_PCI_ROOT_BRIDGE_H_
#define _WIN_NT_PCI_ROOT_BRIDGE_H_

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)

//
// Driver Instance Data Prototypes
//

#define WIN_NT_PCI_ROOT_BRIDGE_SIGNATURE  EFI_SIGNATURE_32('h', '2', 'p', 'b')

typedef struct {
  UINT32                           Signature;
  EFI_HANDLE                       Handle;
                                    
  UINTN                            MemBase;     // Offsets host to bus memory addr.
  UINTN                            IoBase;      // Offsets host to bus io addr.
                                    
  UINT64                           MemLimit;    // Max allowable memory access
  UINT64                           IoLimit;     // Max allowable io access
                                    
  EFI_LOCK                         PciLock;
  UINTN                            PciAddress;
  UINTN                            PciData;

  EFI_DEVICE_PATH_PROTOCOL         *DevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  Io;

} WIN_NT_PCI_ROOT_BRIDGE_INSTANCE;

//
// Driver Instance Data Macros
//

#define DRIVER_INSTANCE_FROM_DEVICE_PATH_THIS(a) \
  CR(a, WIN_NT_PCI_ROOT_BRIDGE_INSTANCE, DevicePath, WIN_NT_PCI_ROOT_BRIDGE_SIGNATURE)

#define DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(a) \
  CR(a, WIN_NT_PCI_ROOT_BRIDGE_INSTANCE, Io, WIN_NT_PCI_ROOT_BRIDGE_SIGNATURE)

//
// Driver Protocol Constructor Prototypes
//

typedef struct {
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation;
  UINTN                                      NumberOfBytes;
  UINTN                                      NumberOfPages;
  EFI_PHYSICAL_ADDRESS                       HostAddress;
  EFI_PHYSICAL_ADDRESS                       MappedHostAddress;
} MAP_INFO;

EFI_STATUS
WinNtRootBridgeDevicePathConstructor (
  IN EFI_DEVICE_PATH_PROTOCOL **Protocol
  );

EFI_STATUS
WinNtRootBridgeIoConstructor (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *Protocol
  );

//
// Driver entry point prototype
//
EFI_STATUS
InitializeWinNtPciRootBridge (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );

#endif
