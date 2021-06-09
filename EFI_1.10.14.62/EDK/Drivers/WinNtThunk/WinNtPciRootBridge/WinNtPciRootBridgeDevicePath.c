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

  WinNtPciRootBridgeDevicePath.c
    
Abstract:

  EFI WinNt Emulated PCI Root Bridge Controller

Revision History

--*/

#include "WinNtPciRootBridge.h"

//
// Static device path declarations for this driver.
//

typedef struct {
  ACPI_HID_DEVICE_PATH          AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL      EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;

static EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath = {
  {
    ACPI_DEVICE_PATH,
    ACPI_DP,
    (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
    (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8),
    EISA_PNP_ID(0x0A03),
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH,
    0
  }
};

EFI_STATUS
WinNtRootBridgeDevicePathConstructor (
	IN	EFI_DEVICE_PATH_PROTOCOL	**Protocol
  )
/*++

Routine Description:

    Construct the device path protocol

Arguments:

    Protocol - protocol to initialize
    
Returns:

    None

--*/
{
  WIN_NT_PCI_ROOT_BRIDGE_INSTANCE *mPrivateData;

  mPrivateData = DRIVER_INSTANCE_FROM_DEVICE_PATH_THIS(Protocol);

  mPrivateData->DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath;

  return EFI_SUCCESS;
}