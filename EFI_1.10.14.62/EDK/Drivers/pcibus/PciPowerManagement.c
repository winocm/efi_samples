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

  PciPowerManagement.c
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#include "Pcibus.h"

BOOLEAN
PciCapabilitySupport (
  IN PCI_IO_DEVICE  *PciIoDevice
);

EFI_STATUS
LocatePMCapabilityRegBlock (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT8        *Offset
);


BOOLEAN
PciCapabilitySupport (
  IN PCI_IO_DEVICE  *PciIoDevice
)
/*++

Routine Description:

Arguments:

Returns:
  
  None

--*/
{

  if (PciIoDevice->Pci.Hdr.Status & EFI_PCI_STATUS_CAPABILITY) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
LocatePMCapabilityRegBlock (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT8        *Offset
)
/*++

Routine Description:

Arguments:

Returns:
  
  None

--*/
{
  UINT8  CapabilityPtr;
  UINT16 CapabilityEntry;
  UINT8  CapabilityID;

  if (!PciCapabilitySupport (PciIoDevice)) {
    return EFI_UNSUPPORTED;
  }

  CapabilityPtr = 0;
  if (IS_CARDBUS_BRIDGE (&PciIoDevice->Pci)) {
    
    PciIoDevice->PciIo.Pci.Read (&PciIoDevice->PciIo, 
                                 EfiPciIoWidthUint8,
                                 EFI_PCI_CARDBUS_BRIDGE_CAPABILITY_PTR,
                                 1,
                                 &CapabilityPtr
                                );
  } else {

    PciIoDevice->PciIo.Pci.Read (&PciIoDevice->PciIo, 
                                 EfiPciIoWidthUint8,
                                 EFI_PCI_CAPABILITY_PTR,
                                 1,
                                 &CapabilityPtr
                                );
  }


  
  
  while (CapabilityPtr > 0x3F) {

    PciIoDevice->PciIo.Pci.Read (&PciIoDevice->PciIo, 
                                 EfiPciIoWidthUint16,
                                 CapabilityPtr,
                                 1,
                                 &CapabilityEntry
                                );

    CapabilityID = (UINT8) CapabilityEntry;
    
    if (CapabilityID == EFI_PCI_POWER_MANAGEMENT_CAPID) {
      *Offset = CapabilityPtr;
      return EFI_SUCCESS;
    }
    
    CapabilityPtr = (UINT8) (CapabilityEntry >> 8);
  }

  return EFI_NOT_FOUND;
}


EFI_STATUS
ResetPowerManagementFeature (
  IN PCI_IO_DEVICE *PciIoDevice
)
/*++

Routine Description:

  This function is intended to turn off PWE assertion and
  put the device to D0 state if the device supports
  PCI Power Management.

Arguments:

Returns:
  
  None

--*/
{
  EFI_STATUS Status;
  UINT8      PowerManagementRegBlock;
  UINT16     PMCSR;

  PowerManagementRegBlock = 0;

  Status = LocatePMCapabilityRegBlock (
                                        PciIoDevice,
                                        &PowerManagementRegBlock
                                      );

  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Turn off the PWE assertion and put the device into D0 State
  //
  PMCSR = 0x8000;

  //
  // Write PMCSR
  //
  PciIoDevice->PciIo.Pci.Write (
                                &PciIoDevice->PciIo,
                                EfiPciIoWidthUint16,
                                PowerManagementRegBlock + 4,
                                1,
                                &PMCSR
                              );

  return EFI_SUCCESS;
}

                                












         






