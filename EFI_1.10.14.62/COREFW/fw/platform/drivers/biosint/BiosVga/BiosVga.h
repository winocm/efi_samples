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

  BiosVga.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_VGA_H
#define _BIOS_VGA_H

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
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)

#define EFI_MAX_ATTRIBUTE   0xff

//
// BIOS VGA Device Structure
//
#define BIOS_VGA_DEV_SIGNATURE   EFI_SIGNATURE_32('B','V','G','A')

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  SimpleTextOut;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   SimpleTextOutputMode;
  EFI_PHYSICAL_ADDRESS          BufferPhysicalAddress;
  UINT8                         *Buffer;
} BIOS_VGA_DEV;

#define BIOS_VGA_DEV_FROM_THIS(a) CR(a, BIOS_VGA_DEV, SimpleTextOut, BIOS_VGA_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gBiosVgaDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gBiosVgaComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
BiosVgaDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosVgaDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosVgaDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );
  
//
// Simple Text Output Protocol functions
//
EFI_STATUS 
EFIAPI
BiosVgaReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         ExtendedVerification
  );

EFI_STATUS 
EFIAPI
BiosVgaOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  );

EFI_STATUS 
EFIAPI
BiosVgaTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  );

EFI_STATUS 
EFIAPI
BiosVgaClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This
  );

EFI_STATUS 
EFIAPI
BiosVgaSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Attribute
  );

EFI_STATUS 
EFIAPI
BiosVgaSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  );

EFI_STATUS 
EFIAPI
BiosVgaEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         Visible
  );

EFI_STATUS 
EFIAPI
BiosVgaQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  );

EFI_STATUS 
EFIAPI
BiosVgaSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber
  );

#endif
