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
#include EFI_PROTOCOL_DEFINITION (VgaMiniPort)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)

// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)

//
// VGA specific registers
//
#define CRTC_CURSOR_START         0xA
#define CRTC_CURSOR_END           0xB

#define CRTC_CURSOR_LOCATION_HIGH 0xE
#define CRTC_CURSOR_LOCATION_LOW  0xF

#define EFI_MAX_ATTRIBUTE   0x7F

//
// VGA Class Device Structure
//
#define VGA_CLASS_DEV_SIGNATURE   EFI_SIGNATURE_32('V','G','A','C')

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  SimpleTextOut;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   SimpleTextOutputMode;
  EFI_VGA_MINI_PORT_PROTOCOL    *VgaMiniPort;
  EFI_PCI_IO_PROTOCOL           *PciIo;
} VGA_CLASS_DEV;

#define VGA_CLASS_DEV_FROM_THIS(a) CR(a, VGA_CLASS_DEV, SimpleTextOut, VGA_CLASS_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gVgaClassDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gVgaClassComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
VgaClassDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
VgaClassDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
VgaClassDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );
  
//
// Simple Text Output Protocol functions
//
EFI_STATUS 
EFIAPI
VgaClassReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL        *This,
  IN  BOOLEAN                             ExtendedVerification
  );

EFI_STATUS 
EFIAPI
VgaClassOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  );

EFI_STATUS 
EFIAPI
VgaClassTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  );

EFI_STATUS 
EFIAPI
VgaClassClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This
  );

EFI_STATUS 
EFIAPI
VgaClassSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Attribute
  );

EFI_STATUS 
EFIAPI
VgaClassSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  );

EFI_STATUS 
EFIAPI
VgaClassEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         Visible
  );

EFI_STATUS 
EFIAPI
VgaClassQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  );

EFI_STATUS 
EFIAPI
VgaClassSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber
  );

#endif
