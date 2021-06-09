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

  PciRootBridgeIo.c

Abstract:

  PCI Root Bridge I/O protocol as defined in the EFI 1.1 specification.

  PCI Root Bridge I/O protocol is used by PCI Bus Driver to perform PCI Memory, PCI I/O, 
  and PCI Configuration cycles on a PCI Root Bridge. It also provides services to perform 
  defferent types of bus mastering DMA

--*/

#include "Efi.h"

#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)

EFI_GUID gEfiPciRootBridgeIoProtocolGuid = EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiPciRootBridgeIoProtocolGuid, "PciRootBridgeIo Protocol", "EFI 1.1 Pci Root Bridge IO Protocol");
