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

  BOT.h

Abstract:

--*/
#ifndef _BOT_H
#define _BOT_H

#include "usb.h"
#include EFI_PROTOCOL_DEFINITION (UsbIo)
#include EFI_PROTOCOL_DEFINITION (UsbAtapi)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#pragma pack(1)

//
//Bulk Only device protocol
//
typedef struct {
  UINT32 dCBWSignature ;
  UINT32 dCBWTag ;
  UINT32 dCBWDataTransferLength ;
  UINT8  bmCBWFlags ;
  UINT8  bCBWLUN ;
  UINT8  bCBWCBLength ;
  UINT8  CBWCB[16] ;
} CBW ;

typedef struct {
  UINT32 dCSWSignature ;
  UINT32 dCSWTag ;
  UINT32 dCSWDataResidue ;
  UINT8  bCSWStatus ;
  UINT8  Filler[18];
} CSW ;

#pragma pack()

#define USB_BOT_DEVICE_SIGNATURE   EFI_SIGNATURE_32('u','b','o','t')

typedef struct {
  UINTN                         Signature;

  EFI_USB_ATAPI_PROTOCOL        UsbAtapiProtocol;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_INTERFACE_DESCRIPTOR  *InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   *BulkInEndpointDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   *BulkOutEndpointDescriptor;
  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
} USB_BOT_DEVICE;

#define USB_BOT_DEVICE_FROM_THIS(a) \
    CR(a, USB_BOT_DEVICE, UsbAtapiProtocol, USB_BOT_DEVICE_SIGNATURE)

//
// Status code, see Usb Bot device spec
//
#define CSWSIG 0x53425355
#define CBWSIG 0x43425355


//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gUsbBotDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gUsbBotComponentName;


#endif
