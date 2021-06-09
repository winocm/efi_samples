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

  PciDeviceSupport.h
  
Abstract:

  

Revision History

--*/

#ifndef _EFI_PCI_DEVICE_SUPPORT_H
#define _EFI_PCI_DEVICE_SUPPORT_H

EFI_STATUS
InitializePciDevicePool(
  VOID
);

EFI_STATUS 
InsertPciDevice (
  PCI_IO_DEVICE *Bridge,
  PCI_IO_DEVICE *PciDeviceNode 
);

EFI_STATUS 
DestroyPciDeviceTree ( 
   IN PCI_IO_DEVICE *Bridge 
);

EFI_STATUS 
DestroyRootBridgeByHandle (
   EFI_HANDLE Controller
);

EFI_STATUS 
RegisterPciDevice ( 
  IN  EFI_HANDLE                     Controller,
  IN  PCI_IO_DEVICE                  *PciIoDevice,
  OUT EFI_HANDLE                     *Handle
);


EFI_STATUS 
DeRegisterPciDevice ( 
  IN  EFI_HANDLE                     Controller,
  IN  EFI_HANDLE                     Handle
);

EFI_STATUS 
StartPciDevices ( 
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
);

PCI_IO_DEVICE *
CreateRootBridge (
  IN EFI_HANDLE RootBridgeHandle 
);

PCI_IO_DEVICE* 
GetRootBridgeByHandle (
   EFI_HANDLE RootBridgeHandle
);

EFI_STATUS 
InsertRootBridge (
  PCI_IO_DEVICE *RootBridge
);

EFI_STATUS 
DestroyRootBridge ( 
   IN PCI_IO_DEVICE *RootBridge 
);

BOOLEAN 
RootBridgeExisted( 
  IN EFI_HANDLE RootBridgeHandle 
);

BOOLEAN 
PciDeviceExisted( 
  IN PCI_IO_DEVICE    *Bridge,
  IN PCI_IO_DEVICE    *PciIoDevice
);

PCI_IO_DEVICE*
ActiveVGADeviceOnTheRootBridge(
  IN PCI_IO_DEVICE        *RootBridge
);

PCI_IO_DEVICE*
ActiveVGADeviceOnTheSameSegment(
  IN PCI_IO_DEVICE        *VgaDevice
);
#endif

