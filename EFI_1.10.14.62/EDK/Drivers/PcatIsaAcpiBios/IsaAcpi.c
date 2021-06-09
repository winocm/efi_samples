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

  IsaAcpi.c
    
Abstract: 
  
    ISA ACPI Protocol Implementation

Revision History

--*/

#include "PcatIsaAcpi.h"

//
// Platform specific data for the ISA devices that are present.in the platform
//

//
// COM 1 UART Controller
//
static EFI_ISA_ACPI_RESOURCE mPcatIsaAcpiCom1DeviceResources[] = {
  {EfiIsaAcpiResourceIo,        0, 0x3f8, 0x3ff},
  {EfiIsaAcpiResourceInterrupt, 0, 4,     0},
  {EfiIsaAcpiResourceEndOfList, 0, 0,     0}
};

//
// COM 2 UART Controller
//
static EFI_ISA_ACPI_RESOURCE mPcatIsaAcpiCom2DeviceResources[] = {
  {EfiIsaAcpiResourceIo,        0, 0x2f8, 0x2ff},
  {EfiIsaAcpiResourceInterrupt, 0, 3,     0},
  {EfiIsaAcpiResourceEndOfList, 0, 0,     0}
};

//
// PS/2 Keyboard Controller
//
static EFI_ISA_ACPI_RESOURCE  mPcatIsaAcpiPs2KeyboardDeviceResources[] = {
  {EfiIsaAcpiResourceIo,        0, 0x60, 0x64},
  {EfiIsaAcpiResourceInterrupt, 0, 1,     0},
  {EfiIsaAcpiResourceEndOfList, 0, 0,     0}
};

//
// Table of ISA Controllers
//
EFI_ISA_ACPI_RESOURCE_LIST gPcatIsaAcpiDeviceList[] = {
  {{EISA_PNP_ID(0x501), 0}, mPcatIsaAcpiCom1DeviceResources        }, // COM 1 UART Controller
  {{EISA_PNP_ID(0x501), 1}, mPcatIsaAcpiCom2DeviceResources        }, // COM 2 UART Controller
  {{EISA_PNP_ID(0x303), 0}, mPcatIsaAcpiPs2KeyboardDeviceResources }, // PS/2 Keyboard Controller
  {{0,                  0}, NULL                                   }  // End if ISA Controllers
};

//
// ISA ACPI Protocol Functions
//
VOID
IsaDeviceLookup (
  IN  EFI_ISA_ACPI_DEVICE_ID      *Device,
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **IsaAcpiDevice,
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **NextIsaAcpiDevice
  )
/*++

Routine Description:
  Enumerate the ISA devices on the ISA bus

Arguments:

Returns:

--*/
{
  UINTN  Index;

  *IsaAcpiDevice = NULL;
  if (NextIsaAcpiDevice != NULL) {
    *NextIsaAcpiDevice = NULL;
  }
  if (Device == NULL) {
    Index = 0;
  } else {
    for(Index = 0; gPcatIsaAcpiDeviceList[Index].ResourceItem != NULL; Index++) {
      if (Device->HID == gPcatIsaAcpiDeviceList[Index].Device.HID && 
          Device->UID == gPcatIsaAcpiDeviceList[Index].Device.UID    ) {
        break;
      }
    }
    if (gPcatIsaAcpiDeviceList[Index].ResourceItem == NULL) {
      return;
    }
    *IsaAcpiDevice = &(gPcatIsaAcpiDeviceList[Index]);
    Index++;
  }
  if (gPcatIsaAcpiDeviceList[Index].ResourceItem != NULL && NextIsaAcpiDevice != NULL) {
    *NextIsaAcpiDevice = &(gPcatIsaAcpiDeviceList[Index]);
  }
}

EFI_STATUS
IsaDeviceEnumerate (
  IN  EFI_ISA_ACPI_PROTOCOL   *This,
  OUT EFI_ISA_ACPI_DEVICE_ID  **Device
  )
/*++

Routine Description:
  Enumerate the ISA devices on the ISA bus

Arguments:

Returns:

--*/
{
  EFI_ISA_ACPI_RESOURCE_LIST  *IsaAcpiDevice;
  EFI_ISA_ACPI_RESOURCE_LIST  *NextIsaAcpiDevice;

  IsaDeviceLookup (*Device, &IsaAcpiDevice, &NextIsaAcpiDevice);
  if (NextIsaAcpiDevice == NULL) {
    return EFI_NOT_FOUND;
  }
  *Device = &(NextIsaAcpiDevice->Device);
  return EFI_SUCCESS;
}

EFI_STATUS
IsaDeviceSetPower (
  IN EFI_ISA_ACPI_PROTOCOL   *This,
  IN EFI_ISA_ACPI_DEVICE_ID  *Device,
  IN BOOLEAN                 OnOff
  )
/*++

Routine Description:
  Set ISA device power 

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
} 

EFI_STATUS
IsaGetCurrentResource (
  IN  EFI_ISA_ACPI_PROTOCOL       *This,
  IN  EFI_ISA_ACPI_DEVICE_ID      *Device,  
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **ResourceList
  )
/*++

Routine Description:
  Get current Resource of the specific ISA device

Arguments:

Returns:

--*/
{
  IsaDeviceLookup (Device, ResourceList, NULL);
  if (*ResourceList == NULL) {
    return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
IsaGetPossibleResource (
  IN  EFI_ISA_ACPI_PROTOCOL       *This,
  IN  EFI_ISA_ACPI_DEVICE_ID      *Device,  
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **ResourceList
  )
/*++

Routine Description:

Arguments:

Returns:

--*/ 
{
  return EFI_SUCCESS;
}

EFI_STATUS
IsaSetResource (
  IN EFI_ISA_ACPI_PROTOCOL       *This,
  IN EFI_ISA_ACPI_DEVICE_ID      *Device,  
  IN EFI_ISA_ACPI_RESOURCE_LIST  *ResourceList
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}
        
EFI_STATUS
IsaEnableDevice (
  IN EFI_ISA_ACPI_PROTOCOL   *This,
  IN EFI_ISA_ACPI_DEVICE_ID  *Device,
  IN BOOLEAN                 Enable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;  
}

EFI_STATUS
IsaInitDevice (
  IN EFI_ISA_ACPI_PROTOCOL   *This,
  IN EFI_ISA_ACPI_DEVICE_ID  *Device
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}


EFI_STATUS
IsaInterfaceInit (
  IN EFI_ISA_ACPI_PROTOCOL  *This
)  
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}  
