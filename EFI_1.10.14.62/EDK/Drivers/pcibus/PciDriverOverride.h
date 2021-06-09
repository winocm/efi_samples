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

  PciDriverOverride.h
  
Abstract:

  

Revision History

--*/

#ifndef _EFI_PCI_DRIVER_OVERRRIDE_H
#define _EFI_PCI_DRIVER_OVERRRIDE_H

#include EFI_PROTOCOL_DEFINITION(BusSpecificDriverOverride)


#define DRIVER_OVERRIDE_SIGNATURE   EFI_SIGNATURE_32 ('d','r','o','v')

typedef struct _PCI_DRIVER_OVERRIDE_LIST {
  UINT32                         Signature;
  EFI_LIST_ENTRY                 Link ;
  EFI_HANDLE                     DriverImageHandle;
} PCI_DRIVER_OVERRIDE_LIST;


#define DRIVER_OVERRIDE_FROM_LINK(a) \
  CR (a, PCI_DRIVER_OVERRIDE_LIST, Link, DRIVER_OVERRIDE_SIGNATURE)


EFI_STATUS
InitializePciDriverOverrideInstance (
  PCI_IO_DEVICE  *PciIoDevice
);

EFI_STATUS
AddDriver(
  IN PCI_IO_DEVICE     *PciIoDevice,
  IN EFI_HANDLE        DriverImageHandle
);


#endif
