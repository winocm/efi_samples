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

    PcatIsaAcpi.h
    
Abstract:

    EFI PCAT ISA ACPI Driver for a Generic PC Platform

Revision History

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#include "Pci22.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (IsaAcpi)

//
// PCAT ISA ACPI device private data structure
//
#define PCAT_ISA_ACPI_DEV_SIGNATURE  EFI_SIGNATURE_32('L','P','C','D')

typedef struct {
  UINTN                  Signature;
  EFI_HANDLE             Handle;    
  EFI_ISA_ACPI_PROTOCOL  IsaAcpi;
  EFI_PCI_IO_PROTOCOL    *PciIo;
} PCAT_ISA_ACPI_DEV;

#define PCAT_ISA_ACPI_DEV_FROM_THIS(a) CR(a, PCAT_ISA_ACPI_DEV, IsaAcpi, PCAT_ISA_ACPI_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gPcatIsaAcpiDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPcatIsaAcpiComponentName;

//
// Prototypes for Driver model protocol interface
//
EFI_STATUS
PcatIsaAcpiDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
PcatIsaAcpiDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
PcatIsaAcpiDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

//
// Prototypes for the ISA ACPI protocol interface
//
EFI_STATUS
IsaDeviceEnumerate (
  IN  EFI_ISA_ACPI_PROTOCOL   *This,
  OUT EFI_ISA_ACPI_DEVICE_ID  **Device
  );

EFI_STATUS
IsaDeviceSetPower (
  IN EFI_ISA_ACPI_PROTOCOL   *This,
  IN EFI_ISA_ACPI_DEVICE_ID  *Device,
  IN BOOLEAN                 OnOff
  );
  
EFI_STATUS
IsaGetCurrentResource (
  IN  EFI_ISA_ACPI_PROTOCOL       *This,
  IN  EFI_ISA_ACPI_DEVICE_ID      *Device,
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **ResourceList
  );
  
EFI_STATUS
IsaGetPossibleResource (
  IN  EFI_ISA_ACPI_PROTOCOL       *This,
  IN  EFI_ISA_ACPI_DEVICE_ID      *Device,  
  OUT EFI_ISA_ACPI_RESOURCE_LIST  **ResourceList
  );
  
EFI_STATUS
IsaSetResource (
  IN EFI_ISA_ACPI_PROTOCOL       *This,
  IN EFI_ISA_ACPI_DEVICE_ID      *Device,
  IN EFI_ISA_ACPI_RESOURCE_LIST  *ResourceList
  );
  
EFI_STATUS
IsaEnableDevice (
  IN EFI_ISA_ACPI_PROTOCOL   *This,
  IN EFI_ISA_ACPI_DEVICE_ID  *Device,
  IN BOOLEAN                 Enable
  );

EFI_STATUS
IsaInitDevice (
  IN EFI_ISA_ACPI_PROTOCOL   *This,
  IN EFI_ISA_ACPI_DEVICE_ID  *Device
  );
  
EFI_STATUS
IsaInterfaceInit (
  IN EFI_ISA_ACPI_PROTOCOL  *This
);  
