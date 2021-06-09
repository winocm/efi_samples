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

    VgaClass.h
    
Abstract: 
    

Revision History
--*/

#ifndef _VGA_CLASS_H
#define _VGA_CLASS_H

#include "Efi.h"
#include "EfiDriverLib.h"
#include "Pci22.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)

// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (VgaMiniPort)

//
// PCI VGA MiniPort Device Structure
//
#define PCI_VGA_MINI_PORT_DEV_SIGNATURE   EFI_SIGNATURE_32('P','V','M','P')

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  EFI_VGA_MINI_PORT_PROTOCOL    VgaMiniPort;
  EFI_PCI_IO_PROTOCOL           *PciIo;
} PCI_VGA_MINI_PORT_DEV;

#define PCI_VGA_MINI_PORT_DEV_FROM_THIS(a) CR(a, PCI_VGA_MINI_PORT_DEV, VgaMiniPort, PCI_VGA_MINI_PORT_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gPciVgaMiniPortDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPciVgaMiniPortComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
PciVgaMiniPortDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
PciVgaMiniPortDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
PciVgaMiniPortDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );
  
//
// VGA Mini Port Protocol functions
//
EFI_STATUS 
EFIAPI
PciVgaMiniPortSetMode (
  IN  EFI_VGA_MINI_PORT_PROTOCOL  *This,
  IN  UINTN                       ModeNumber
  );

#endif
