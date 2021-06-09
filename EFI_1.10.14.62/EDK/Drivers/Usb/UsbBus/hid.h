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

    hid.h
    
  Abstract:

    Constants & structure definitions for Usb HID 

  Revision History

--*/

#ifndef _HID_H
#define _HID_H

#include "usb.h"
#include EFI_PROTOCOL_DEFINITION(UsbIo)

//
// HID constants definition, see HID rev1.0
//

//
// HID report item format
//
#define HID_ITEM_FORMAT_SHORT 0
#define HID_ITEM_FORMAT_LONG  1

//
// Special tag indicating long items
//
#define HID_ITEM_TAG_LONG 15

//
// HID report descriptor item type (prefix bit 2,3)
//
#define HID_ITEM_TYPE_MAIN          0
#define HID_ITEM_TYPE_GLOBAL        1
#define HID_ITEM_TYPE_LOCAL         2
#define HID_ITEM_TYPE_RESERVED      3

//
// HID report descriptor main item tags
//
#define HID_MAIN_ITEM_TAG_INPUT             8
#define HID_MAIN_ITEM_TAG_OUTPUT            9
#define HID_MAIN_ITEM_TAG_FEATURE           11
#define HID_MAIN_ITEM_TAG_BEGIN_COLLECTION  10
#define HID_MAIN_ITEM_TAG_END_COLLECTION    12

//
// HID report descriptor main item contents
//
#define HID_MAIN_ITEM_CONSTANT              0x001
#define HID_MAIN_ITEM_VARIABLE              0x002
#define HID_MAIN_ITEM_RELATIVE              0x004
#define HID_MAIN_ITEM_WRAP                  0x008 
#define HID_MAIN_ITEM_NONLINEAR             0x010
#define HID_MAIN_ITEM_NO_PREFERRED          0x020
#define HID_MAIN_ITEM_NULL_STATE            0x040
#define HID_MAIN_ITEM_VOLATILE              0x080
#define HID_MAIN_ITEM_BUFFERED_BYTE         0x100

//
// HID report descriptor collection item types
//
#define HID_COLLECTION_PHYSICAL             0
#define HID_COLLECTION_APPLICATION          1
#define HID_COLLECTION_LOGICAL              2

//
// HID report descriptor global item tags
//
#define HID_GLOBAL_ITEM_TAG_USAGE_PAGE        0
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM   1
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM   2
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM  3
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM  4
#define HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT     5
#define HID_GLOBAL_ITEM_TAG_UNIT              6
#define HID_GLOBAL_ITEM_TAG_REPORT_SIZE       7
#define HID_GLOBAL_ITEM_TAG_REPORT_ID         8
#define HID_GLOBAL_ITEM_TAG_REPORT_COUNT      9
#define HID_GLOBAL_ITEM_TAG_PUSH              10
#define HID_GLOBAL_ITEM_TAG_POP               11

//
// HID report descriptor local item tags
//
#define HID_LOCAL_ITEM_TAG_USAGE              0
#define HID_LOCAL_ITEM_TAG_USAGE_MINIMUM      1
#define HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM      2
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX   3
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MINIMUM 4
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MAXIMUM 5
#define HID_LOCAL_ITEM_TAG_STRING_INDEX       7
#define HID_LOCAL_ITEM_TAG_STRING_MINIMUM     8
#define HID_LOCAL_ITEM_TAG_STRING_MAXIMUM     9
#define HID_LOCAL_ITEM_TAG_DELIMITER          10

//
// HID usage tables
//
#define HID_USAGE_PAGE    0xffff0000

#define HID_UP_GENDESK    0x00010000
#define HID_UP_KEYBOARD   0x00070000
#define HID_UP_LED        0x00080000
#define HID_UP_BUTTON     0x00090000
#define HID_UP_CONSUMER   0x000c0000
#define HID_UP_DIGITIZER  0x000d0000
#define HID_UP_PID        0x000f0000

#define HID_USAGE         0x0000ffff

#define HID_GD_POINTER    0x00010001
#define HID_GD_MOUSE      0x00010002
#define HID_GD_JOYSTICK   0x00010004
#define HID_GD_GAMEPAD    0x00010005
#define HID_GD_HATSWITCH  0x00010039

//
// HID report types
//
#define HID_INPUT_REPORT    1
#define HID_OUTPUT_REPORT   2
#define HID_FEATURE_REPORT  3

//
// HID device quirks.
//
#define HID_QUIRK_INVERT  0x01
#define HID_QUIRK_NOTOUCH 0x02

//
// HID class protocol request
//
#define EFI_USB_GET_REPORT_REQUEST      0x01
#define EFI_USB_GET_IDLE_REQUEST        0x02
#define EFI_USB_GET_PROTOCOL_REQUEST    0x03
#define EFI_USB_SET_REPORT_REQUEST      0x09
#define EFI_USB_SET_IDLE_REQUEST        0x0a
#define EFI_USB_SET_PROTOCOL_REQUEST    0x0b

#pragma pack(1)

//
// Descriptor header for Report/Physical Descriptors
//
typedef struct hid_class_descriptor{
  UINT8   DescriptorType;
  UINT16  DescriptorLength;
} EFI_USB_HID_CLASS_DESCRIPTOR;

typedef struct hid_descriptor {
  UINT8                         Length;
  UINT8                         DescriptorType;
  UINT16                        BcdHID;
  UINT8                         CountryCode;
  UINT8                         NumDescriptors;
  EFI_USB_HID_CLASS_DESCRIPTOR  HidClassDesc[1];
} EFI_USB_HID_DESCRIPTOR;

#pragma pack()

EFI_STATUS 
UsbGetHidDescriptor (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo,
  IN  UINT8                     InterfaceNum,
  OUT EFI_USB_HID_DESCRIPTOR    *HidDescriptor
);

EFI_STATUS
UsbGetReportDescriptor (
  IN  EFI_USB_IO_PROTOCOL *UsbIo,
  IN  UINT8               InterfaceNum,
  IN  UINT16              DescriptorSize,
  OUT UINT8               *DescriptorBuffer
);

EFI_STATUS
UsbGetProtocolRequest(
  IN  EFI_USB_IO_PROTOCOL   *UsbIo,
  IN  UINT8                 Interface,
  IN  UINT8                 *Protocol
  );


EFI_STATUS
UsbSetProtocolRequest(
  IN  EFI_USB_IO_PROTOCOL   *UsbIo,
  IN  UINT8                 Interface,
  IN  UINT8                 Protocol
  );


EFI_STATUS
UsbSetIdleRequest(
  IN  EFI_USB_IO_PROTOCOL   *UsbIo,
  IN  UINT8                 Interface,
  IN  UINT8                 ReportId,
  IN  UINT8                 Duration
  );

EFI_STATUS
UsbGetIdleRequest(
  IN  EFI_USB_IO_PROTOCOL *UsbIo,
  IN  UINT8               Interface,
  IN  UINT8               ReportId,
  OUT UINT8               *Duration
  );

EFI_STATUS
UsbSetReportRequest(
  IN  EFI_USB_IO_PROTOCOL   *UsbIo,
  IN  UINT8                 Interface,
  IN  UINT8                 ReportId,
  IN  UINT8                 ReportType,
  IN  UINT16                ReportLen,
  IN  UINT8                 *Report
  );

EFI_STATUS
UsbGetReportRequest(
  IN  EFI_USB_IO_PROTOCOL   *UsbIo,
  IN  UINT8                 Interface,
  IN  UINT8                 ReportId,
  IN  UINT8                 ReportType,
  IN  UINT16                ReportLen,
  IN  UINT8                 *Report
  );

#endif
