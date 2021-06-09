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

    EfiKey.h

Abstract:

    Header file for USB Keyboard Driver's Data Structures

Revision History
++*/
#ifndef _USB_KB_H
#define _USB_KB_H


#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(UsbIo)
#include EFI_PROTOCOL_DEFINITION(ComponentName)
#include EFI_GUID_DEFINITION (HotPlugDevice)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(SimpleTextIn)


#include "usb.h"
#include "usblib.h"

#define MAX_KEY_ALLOWED 32

#define HZ                    1000*1000*10
#define USBKBD_REPEAT_DELAY   ((HZ)/2)
#define USBKBD_REPEAT_RATE    ((HZ)/50)

#define CLASS_HID           3
#define SUBCLASS_BOOT       1
#define PROTOCOL_KEYBOARD   1

#define BOOT_PROTOCOL     0
#define REPORT_PROTOCOL   1

typedef struct
{
  UINT8   Down;
  UINT8   KeyCode;
} USB_KEY;

typedef struct
{
  USB_KEY     buffer[MAX_KEY_ALLOWED + 1];
  UINT8   bHead;
  UINT8   bTail;
} USB_KB_BUFFER;

#define USB_KB_DEV_SIGNATURE  EFI_SIGNATURE_32('u','k','b','d')
typedef struct
{
  UINTN                           Signature;
  
  EFI_SIMPLE_TEXT_IN_PROTOCOL     SimpleInput;
  EFI_USB_IO_PROTOCOL             *UsbIo;
  
  EFI_USB_INTERFACE_DESCRIPTOR    InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR     IntEndpointDescriptor;
  
  USB_KB_BUFFER                   KeyboardBuffer;
  UINT8                           CtrlOn;
  UINT8                           AltOn;
  UINT8                           ShiftOn;
  UINT8                           NumLockOn;
  UINT8                           CapsOn;
  UINT8                           LastKeyCodeArray[8];
  UINT8                           CurKeyChar;
  
  UINT8                           RepeatKey;
  EFI_EVENT                       RepeatTimer;  
  
  EFI_UNICODE_STRING_TABLE        *ControllerNameTable;

} USB_KB_DEV;

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gUsbKeyboardDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gUsbKeyboardComponentName;

#define USB_KB_DEV_FROM_THIS(a) \
    CR(a, USB_KB_DEV, SimpleInput, USB_KB_DEV_SIGNATURE)

#define MOD_CONTROL_L 0x01
#define MOD_CONTROL_R 0x10
#define MOD_SHIFT_L   0x02
#define MOD_SHIFT_R   0x20
#define MOD_ALT_L   0x04
#define MOD_ALT_R   0x40
#define MOD_WIN_L   0x08
#define MOD_WIN_R   0x80

typedef struct
{
  UINT8 Mask;
  UINT8 Key;
} KB_MODIFIER;

#define USB_KEYCODE_MAX_MAKE    0x64


#define USBKBD_VALID_KEYCODE(key)   ((UINT8)(key) > 3)

typedef struct
{
  UINT8   NumLock : 1;
  UINT8   CapsLock : 1;
  UINT8   Resrvd : 6;
} LED_MAP;
#endif
