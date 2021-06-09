/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  Module Name:模块名字

    UsbMouse.h                为什么用.h，不用.c？

  Abstract:

--*/

#ifndef _USB_MOUSE_H
#define _USB_MOUSE_H

#include "usb.h"
#include EFI_PROTOCOL_DEFINITION (UsbIo)
#include EFI_PROTOCOL_DEFINITION (SimplePointer)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define CLASS_HID           3
#define SUBCLASS_BOOT       1
#define PROTOCOL_MOUSE      2

#define BOOT_PROTOCOL       0
#define REPORT_PROTOCOL     1

#define USB_MOUSE_DEV_SIGNATURE   EFI_SIGNATURE_32('u','m','o','u')

typedef struct
{
  BOOLEAN                         ButtonDetected;
  UINT8                           ButtonMinIndex;
  UINT8                           ButtonMaxIndex;
  UINT8                           Reserved;
} PRIVATE_DATA;

typedef struct
{
  UINTN                           Signature;
  EFI_USB_IO_PROTOCOL             *UsbIo;
  EFI_USB_INTERFACE_DESCRIPTOR    *InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR     *IntEndpointDescriptor;
  UINT8                           NumberOfButtons;
  INT32                           XLogicMax;
  INT32                           XLogicMin;
  INT32                           YLogicMax;
  INT32                           YLogicMin;
  EFI_SIMPLE_POINTER_PROTOCOL     SimplePointerProtocol;
  EFI_SIMPLE_POINTER_STATE        State;
  EFI_SIMPLE_POINTER_MODE         Mode;
  BOOLEAN                         StateChanged;
  PRIVATE_DATA                    PrivateData;
  EFI_UNICODE_STRING_TABLE        *ControllerNameTable;
} USB_MOUSE_DEV;

#define USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(a) \
    CR(a, USB_MOUSE_DEV, SimplePointerProtocol, USB_MOUSE_DEV_SIGNATURE)

//
// Global Variables全局变量，其他函数可以使用
//
extern EFI_DRIVER_BINDING_PROTOCOL gUsbMouseDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gUsbMouseComponentName;
    
#endif
