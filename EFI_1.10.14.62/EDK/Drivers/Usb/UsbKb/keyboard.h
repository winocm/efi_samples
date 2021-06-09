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

    Keyboard.h

Abstract:

    Function prototype for USB Keyboard Driver

Revision History
++*/

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "efikey.h"
#include "usblib.h"


BOOLEAN
IsUSBKeyboard(
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  );


EFI_STATUS
InitUSBKeyboard(
  IN USB_KB_DEV   *UsbKeyboardDevice
  );


EFI_STATUS
KeyboardHandler(
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  );


EFI_STATUS
USBParseKey(
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  );


EFI_STATUS
USBKeyCodeToEFIScanCode(
  IN  USB_KB_DEV      *UsbKeyboardDevice, 
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  );


EFI_STATUS
InitUSBKeyBuffer(
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  );

BOOLEAN
IsUSBKeyboardBufferEmpty(
  IN  USB_KB_BUFFER   KeyboardBuffer
  );


BOOLEAN
IsUSBKeyboardBufferFull(
  IN  USB_KB_BUFFER   KeyboardBuffer
  );


EFI_STATUS
InsertKeyCode(
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  IN      UINT8         Key,
  IN      UINT8         Down
  );

EFI_STATUS
RemoveKeyCode(
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  );

VOID
USBKeyboardRepeatHandler(
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  );
  
EFI_STATUS
SetKeyLED(
  IN  USB_KB_DEV    *UsbKeyboardDevice
  );  
  
#endif
