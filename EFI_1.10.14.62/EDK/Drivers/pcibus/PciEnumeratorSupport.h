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

  PciEnumeratorSupport.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#ifndef _EFI_PCI_ENUMERATOR_SUPPORT_H
#define _EFI_PCI_ENUMERATOR_SUPPORT_H

EFI_STATUS
PciPciDeviceInfoCollector (
  IN PCI_IO_DEVICE                    *Bridge,
  IN UINT8                            StartBusNumber
);


EFI_STATUS 
PciDevicePresent(
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo,
  PCI_TYPE00                          *Pci,
  UINT8                               Bus,
  UINT8                               Device,
  UINT8                               Func
);

EFI_STATUS
PciEnumeratorLight (
  IN EFI_HANDLE                       Controller
);

EFI_STATUS
PciGetBusRange (
  IN     EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Descriptors,
  OUT    UINT16                             *MinBus, 
  OUT    UINT16                             *MaxBus,
  OUT    UINT16                             *BusRange
);

#endif

