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

    UsbMassStorage.h

Abstract:

    Header file for USB Mass Storage Driver's Data Structures

Revision History
++*/

#ifndef _USB_FLP_H
#define _USB_FLP_H


#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(UsbIo)
#include EFI_PROTOCOL_DEFINITION(UsbAtapi)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(BlockIo)


#include "usb.h"
#include "usblib.h"

#include "UsbMassStorageData.h"


#define CLASS_MASSTORAGE        8
#define SUBCLASS_UFI        4
#define SUBCLASS_8070              5
#define PROTOCOL_BOT        0x50
#define PROTOCOL_CBI0       0
#define PROTOCOL_CBI1       1

#define USBFLOPPY           1
#define USBFLOPPY2          2   // for those that use ReadCapacity(0x25) command to retrieve media capacity
#define USBCDROM            3

#define USB_FLOPPY_DEV_SIGNATURE  EFI_SIGNATURE_32('u','f','l','p')

typedef struct
{
  UINTN                     Signature;
  
  EFI_HANDLE                Handle;
  EFI_BLOCK_IO_PROTOCOL     BlkIo;
  EFI_BLOCK_IO_MEDIA        BlkMedia;
  EFI_USB_ATAPI_PROTOCOL    *AtapiProtocol;
  
  
  REQUEST_SENSE_DATA        *SenseData;
  UINT8                     SenseDataNumber;
  UINT8                     DeviceType;
  
} USB_FLOPPY_DEV;

#define USB_FLOPPY_DEV_FROM_THIS(a) \
    CR(a, USB_FLOPPY_DEV, BlkIo, USB_FLOPPY_DEV_SIGNATURE)

#endif
