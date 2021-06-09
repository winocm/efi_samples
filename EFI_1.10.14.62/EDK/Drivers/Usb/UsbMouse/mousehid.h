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

  MouseHid.h

Abstract:

--*/

#ifndef __MOUSE_HID_H
#define __MOUSE_HID_H

#include "hid.h"
#include "usbmouse.h"

//
// HID Item general structure
//
typedef struct _hid_item 
{
  UINT16  Format;
  UINT8   Size;
  UINT8   Type;
  UINT8   Tag;
  union 
  {
      UINT8   U8;
      UINT16  U16;
      UINT32  U32;
      INT8    I8;
      INT16   I16;
      INT32   I32;
      UINT8   *LongData;
  } Data;
} HID_ITEM;

typedef struct _hid_global {
  UINT16  UsagePage;
  INT32   LogicMin;
  INT32   LogicMax;
  INT32   PhysicalMin;
  INT32   PhysicalMax;
  UINT16  UnitExp;
  UINT16  UINT;
  UINT16  ReportId;
  UINT16  ReportSize;
  UINT16  ReportCount;
} HID_GLOBAL;

typedef struct _hid_local {
  UINT16  Usage[16]; /* usage array */
  UINT16  UsageIndex;
  UINT16  UsageMin;
} HID_LOCAL;

typedef struct _hid_collection {
  UINT16  Type;
  UINT16  Usage;
} HID_COLLECTION;

typedef struct _hid_parser {
  HID_GLOBAL      Global;
  HID_GLOBAL      GlobalStack[8];
  UINT32          GlobalStackPtr;
  HID_LOCAL       Local;
  HID_COLLECTION  CollectionStack[8];
  UINT32          CollectionStackPtr;
} HID_PARSER;

EFI_STATUS
ParseMouseReportDescriptor (
  IN  USB_MOUSE_DEV   *UsbMouse,
  IN  UINT8           *ReportDescriptor,
  IN  UINTN           ReportSize
  );

#endif
