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

  BiosKeyboard.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_KEYBOARD_H
#define _BIOS_KEYBOARD_H

#include "Efi.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "Int86.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (IsaIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)

//
// BIOS Keyboard Device Structure
//
#define BIOS_KEYBOARD_DEV_SIGNATURE   EFI_SIGNATURE_32('B','K','B','D')

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   SimpleTextIn;
  BOOLEAN                       ExtendedKeyboard;
} BIOS_KEYBOARD_DEV;

#define BIOS_KEYBOARD_DEV_FROM_THIS(a) CR(a, BIOS_KEYBOARD_DEV, SimpleTextIn, BIOS_KEYBOARD_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gBiosKeyboardDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gBiosKeyboardComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
BiosKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );
  
//
// Simple Text Input Protocol functions
//
EFI_STATUS 
EFIAPI 
BiosKeyboardReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  IN  BOOLEAN                      ExtendedVerification
  );

EFI_STATUS 
EFIAPI 
BiosKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  OUT EFI_INPUT_KEY                *Key
  );

#endif
