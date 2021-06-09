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

  BiosVgaMiniPort.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_VGA_MINI_PORT_H
#define _BIOS_VGA_MINI_PORT_H

#include "Efi.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "Int86.h"

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
// BIOS VGA MiniPort Device Structure
//
#define BIOS_VGA_MINI_PORT_DEV_SIGNATURE   EFI_SIGNATURE_32('B','V','M','P')

typedef struct {
  UINTN                       Signature;
  EFI_HANDLE                  Handle;
  EFI_VGA_MINI_PORT_PROTOCOL  VgaMiniPort;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  BOOLEAN                     Int86Available;
} BIOS_VGA_MINI_PORT_DEV;

#define BIOS_VGA_MINI_PORT_DEV_FROM_THIS(a) CR(a, BIOS_VGA_MINI_PORT_DEV, VgaMiniPort, BIOS_VGA_MINI_PORT_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gBiosVgaMiniPortDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gBiosVgaMiniPortComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
BiosVgaMiniPortDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosVgaMiniPortDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosVgaMiniPortDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );
  
//
// BIOS VGA Mini Port Protocol functions
//
EFI_STATUS 
EFIAPI
BiosVgaMiniPortSetMode (
  IN  EFI_VGA_MINI_PORT_PROTOCOL  *This,
  IN  UINTN                       ModeNumber
  );

#endif
