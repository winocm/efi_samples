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

  BiosKeyboard.c
    
Abstract:

  ConsoleOut Routines that speak VGA.

Revision History

--*/

#include "BiosKeyboard.h"

#define CHAR_SCANCODE   0xe0
#define CHAR_ESC        0x1b

//
// EFI Driver Binding Protocol Instance
//
//   This driver has a version value of 0xfffffff0.  This is the
//   highest possible priority for a driver.  This is done on purpose to 
//   prevent any other drivers from managing the keyboard device.  
//
EFI_DRIVER_BINDING_PROTOCOL gBiosKeyboardDriverBinding = {
  BiosKeyboardDriverBindingSupported,
  BiosKeyboardDriverBindingStart,
  BiosKeyboardDriverBindingStop,
  0xfffffff0,
  NULL,
  NULL
};  

//
// Private worker functions
//
VOID 
EFIAPI
BiosKeyboardWaitForKey (
    IN  EFI_EVENT  Event,
    IN  VOID       *Context
    );
    
EFI_STATUS
BiosKeyboardCheckForKey (
    IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This 
    );

UINT16
ConvertToEFIScanCode(
  IN  CHAR16  KeyChar, 
  IN  UINT16  ScanCode
  );

//
// Driver Entry Point
//  

EFI_DRIVER_ENTRY_POINT(BiosKeyboardDriverEntryPoint)
    
EFI_STATUS
BiosKeyboardDriverEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  ) 
/*++
  
  Routine Description:
    Driver Entry Point.
        
  Arguments:
    (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)
  
  Returns:
    EFI_STATUS
--*/                
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gBiosKeyboardDriverBinding,
           ImageHandle,
           &gBiosKeyboardComponentName,
           NULL,
           NULL
           );
} 

//
// EFI Driver Binding Protocol Functions
//

EFI_STATUS
BiosKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Supported.
    
  Arguments:
    (Standard DriverBinding Protocol Supported() function)
    
  Returns:
    EFI_STATUS
  
--*/                
{
  EFI_STATUS           Status;
  EFI_ISA_IO_PROTOCOL  *IsaIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,           
                  &gEfiIsaIoProtocolGuid, 
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Use the ISA I/O Protocol to see if Controller is the Keyboard controller
  //
  if (IsaIo->ResourceList->Device.HID != EISA_PNP_ID(0x303) || IsaIo->ResourceList->Device.UID != 0) {
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
         Controller,  
         &gEfiIsaIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );

  return Status;
}   

EFI_STATUS
BiosKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Install VGA Mini Port Protocol onto VGA device handles
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
    
--*/                
{
  EFI_STATUS            Status;
  EFI_ISA_IO_PROTOCOL   *IsaIo;
  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate;
#ifndef SOFT_SDV
  IA32_RegisterSet_t    Regs;  
  BOOLEAN               CarryFlag;
#endif

  BiosKeyboardPrivate = NULL;

  //
  // Open the IO Abstraction(s) needed 
  //
  Status = gBS->OpenProtocol (
                  Controller,           
                  &gEfiIsaIoProtocolGuid, 
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
      
  //
  // Allocate the private device structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(BIOS_KEYBOARD_DEV),
                  &BiosKeyboardPrivate
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EfiZeroMem (BiosKeyboardPrivate, sizeof (BIOS_KEYBOARD_DEV)) ;
  
  //
  // Initialize the private device structure
  //
  BiosKeyboardPrivate->Signature = BIOS_KEYBOARD_DEV_SIGNATURE;
  BiosKeyboardPrivate->Handle    = Controller;

  BiosKeyboardPrivate->SimpleTextIn.Reset             = BiosKeyboardReset;
  BiosKeyboardPrivate->SimpleTextIn.ReadKeyStroke     = BiosKeyboardReadKeyStroke;

  BiosKeyboardPrivate->ExtendedKeyboard = FALSE;

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent (
        EFI_EVENT_NOTIFY_WAIT,
        EFI_TPL_NOTIFY,
        BiosKeyboardWaitForKey,
        &(BiosKeyboardPrivate->SimpleTextIn),
        &((BiosKeyboardPrivate->SimpleTextIn).WaitForKey)
        );
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // Reset the keyboard device
  //
  Status = BiosKeyboardPrivate->SimpleTextIn.Reset (&BiosKeyboardPrivate->SimpleTextIn, FALSE);
  if (EFI_ERROR(Status)) {
    goto Done;
  }

#ifndef SOFT_SDV
  //
  // Get Configuration
  //
  Regs.h.AH = 0xc0;
  CarryFlag = Int86(0x15, &Regs);
  if (CarryFlag == FALSE) {
    //
    // Check bit 6 of Feature Byte 2.  
    // If it is set, then Int 16 Func 09 is supported
    //
    if (*(UINT8 *)( (Regs.x.ES << 4) + Regs.x.BX + 0x06) & 0x40) {
      //
      // Get Keyboard Functionality
      //
      Regs.h.AH = 0x09;
      CarryFlag = Int86(0x16, &Regs);
      if (CarryFlag == FALSE) {
        //
        // Check bit 5 of AH.
        // If it is set, then INT 16 Finc 10-12 are supported.
        //
        if (Regs.h.AL & 0x40) {
          //
          // Set the flag to use INT 16 Func 10-12
          //
          BiosKeyboardPrivate->ExtendedKeyboard = TRUE;
        }
      }
    }
  }
#endif

  //
  // Install protocol interfaces for the keyboard device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleTextInProtocolGuid, &BiosKeyboardPrivate->SimpleTextIn,
                  NULL
                  );

Done:
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent((BiosKeyboardPrivate->SimpleTextIn).WaitForKey);
    
    gBS->FreePool(BiosKeyboardPrivate);
    
    gBS->CloseProtocol (
                Controller,
                &gEfiIsaIoProtocolGuid,
                This->DriverBindingHandle,
                Controller
                );
  }
  return Status;
}

EFI_STATUS
BiosKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop.
  
  Arguments:
    (Standard DriverBinding Protocol Stop() function)
  
  Returns:
    EFI_STATUS
  
--*/                
{
  EFI_STATUS                   Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL  *SimpleTextIn;
  BIOS_KEYBOARD_DEV            *BiosKeyboardPrivate;

  //
  // Disable Keyboard
  //

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  (VOID **)&SimpleTextIn,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (SimpleTextIn);
  
  //
  // Uninstall the Simple TextIn Protocol
  //
  
  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid, &BiosKeyboardPrivate->SimpleTextIn
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Release the IsaIo protocol on the controller handle
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  //
  // Free other resources
  //
  gBS->CloseEvent((BiosKeyboardPrivate->SimpleTextIn).WaitForKey);
  gBS->FreePool(BiosKeyboardPrivate);

  return EFI_SUCCESS;
}

//
// EFI Simple Text In Protocol Functions
//
EFI_STATUS 
EFIAPI
BiosKeyboardReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  IN  BOOLEAN                      ExtendedVerification
  )
{
  EFI_INPUT_KEY               Key;

  //
  // No reset exists for keyboard, so flush out keyboard buffer
  //
  while (!EFI_ERROR (BiosKeyboardReadKeyStroke (This, &Key)));

  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI 
BiosKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  OUT EFI_INPUT_KEY                *Key
  )
{
  BIOS_KEYBOARD_DEV   *BiosKeyboardPrivate;
  UINT16              ScanCode;
  UINT16              KeyChar;
  IA32_RegisterSet_t  Regs;  
  EFI_STATUS          Status;

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  //
  // If there's no key, just return
  //
  Status = BiosKeyboardCheckForKey (This);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Read the key
  //
  if (BiosKeyboardPrivate->ExtendedKeyboard) {
    Regs.h.AH = 0x10;
  } else {
    Regs.h.AH = 0x00;
  }
  Int86(0x16, &Regs);
  
  ScanCode = Regs.h.AH;
  KeyChar = (CHAR16)Regs.h.AL;

  if (KeyChar == CHAR_NULL || KeyChar == CHAR_SCANCODE || KeyChar == CHAR_ESC) {
    Key->ScanCode    = ConvertToEFIScanCode(KeyChar, ScanCode);
    Key->UnicodeChar = CHAR_NULL;
  } else {
    Key->ScanCode    = 0;
    Key->UnicodeChar = KeyChar;
  }

  return EFI_SUCCESS;
}

VOID 
EFIAPI
BiosKeyboardWaitForKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  //
  // Someone is waiting on the keyboard event, if there's
  // a key pending, signal the event
  //

  if (!EFI_ERROR (BiosKeyboardCheckForKey (Context))) {
    gBS->SignalEvent (Event);
  }
}

EFI_STATUS 
EFIAPI 
BiosKeyboardCheckForKey (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This 
  )
{   
  BIOS_KEYBOARD_DEV   *BiosKeyboardPrivate;
  IA32_RegisterSet_t  Regs;
  BOOLEAN             CarryFlag;

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  if (BiosKeyboardPrivate->ExtendedKeyboard) {
    Regs.h.AH = 0x11;
  } else {
    Regs.h.AH = 0x01;
  }
  CarryFlag = Int86(0x16, &Regs);

  return (Regs.x.Flags.ZF) ? EFI_NOT_READY : EFI_SUCCESS;
}

//
// Private worker functions
//
UINT16
ConvertToEFIScanCode(
  IN  CHAR16  KeyChar, 
  IN  UINT16  ScanCode
  )
{
  UINT16   EfiScanCode;

  if (KeyChar == CHAR_ESC) {
    EfiScanCode = SCAN_ESC;
  } else if (KeyChar == 0x00 || KeyChar == 0xe0) {
    //
    // Movement & Function Keys
    //
    switch (ScanCode) {
    case 0x48 :
      EfiScanCode = SCAN_UP; 
      break;
    case 0x50 :
      EfiScanCode = SCAN_DOWN; 
      break;
    case 0x4d :
      EfiScanCode = SCAN_RIGHT; 
      break;
    case 0x4b :
      EfiScanCode = SCAN_LEFT; 
      break;
    case 0x47 :
      EfiScanCode = SCAN_HOME; 
      break;
    case 0x4F :
      EfiScanCode = SCAN_END; 
      break;
    case 0x52 :
      EfiScanCode = SCAN_INSERT; 
      break;
    case 0x53 :
      EfiScanCode = SCAN_DELETE; 
      break;
    case 0x49 :
      EfiScanCode = SCAN_PAGE_UP; 
      break;
    case 0x51 :
      EfiScanCode = SCAN_PAGE_DOWN; 
      break;
    //
    // Function Keys are only valid if KeyChar == 0x00
    //  This function does not require KeyChar to be 0x00
    //
    case 0x3b :
      EfiScanCode = SCAN_F1; 
      break;
    case 0x3c :
      EfiScanCode = SCAN_F2; 
      break;
    case 0x3d :
      EfiScanCode = SCAN_F3; 
      break;
    case 0x3e :
      EfiScanCode = SCAN_F4; 
      break;
    case 0x3f :
      EfiScanCode = SCAN_F5; 
      break;
    case 0x40 :
      EfiScanCode = SCAN_F6; 
      break;
    case 0x41 :
      EfiScanCode = SCAN_F7; 
      break;
    case 0x42 :
      EfiScanCode = SCAN_F8; 
      break;
    case 0x43 :
      EfiScanCode = SCAN_F9; 
      break;
    case 0x44 :
      EfiScanCode = SCAN_F10; 
      break;
    default:
      EfiScanCode = SCAN_NULL; 
      break;
    }
  } else {
    return SCAN_NULL;
  }
  return EfiScanCode;
}
