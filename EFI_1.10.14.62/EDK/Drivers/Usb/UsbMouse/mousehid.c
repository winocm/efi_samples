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

  Mousehid.c

Abstract:
  Parse mouse hid descriptor

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#include EFI_PROTOCOL_DEFINITION(SimplePointer)
#include "hid.h"
#include "mousehid.h"

//
// Get an item from report descriptor
//
static 
UINT8 *GetNextItem(
  IN  UINT8   *StartPos, 
  IN  UINT8   *EndPos,
  OUT HID_ITEM *HidItem
  )
{
  UINT8   Temp;

  if((EndPos - StartPos) <= 0) {
    return NULL;
  }
  
  Temp = *StartPos;
  StartPos++;
  
  HidItem->Type = (UINT8)((Temp >> 2) & 0x03); //bit 2,3
  HidItem->Tag = (UINT8)((Temp >> 4) & 0x0F);  //bit 4-7

  if(HidItem->Tag == HID_ITEM_TAG_LONG) 
  {
  //
  // Long Items are not supported by HID rev1.0,
  // although we try to parse it.
  //
    HidItem->Format = HID_ITEM_FORMAT_LONG;

    if ((EndPos - StartPos) >= 2) 
    {
      HidItem->Size = *StartPos++;
      HidItem->Tag  = *StartPos++;

      if((EndPos - StartPos) >= HidItem->Size) 
      {
        HidItem->Data.LongData = StartPos;
        StartPos += HidItem->Size;
        return StartPos;
      }
    }
  } 
  else 
  {
    HidItem->Format = HID_ITEM_FORMAT_SHORT;
    HidItem->Size = (UINT8)(Temp & 0x03); //bit 0, 1

    switch (HidItem->Size) 
    {
    case 0:
      //
      // No data
      //
      return StartPos;

    case 1: 
      //
      // One byte data
      //
      if ((EndPos - StartPos) >= 1) 
      {
        HidItem->Data.U8 = *StartPos++;
        return StartPos;
      }

    case 2: 
      //
      // Two byte data
      //
      if ((EndPos - StartPos) >= 2) 
      {
        EfiCopyMem (&HidItem->Data.U16, StartPos, sizeof (UINT16));
        StartPos += 2;
        return StartPos;
      }

    case 3: 
      //
      // 4 byte data, adjust size
      //
      HidItem->Size++;
      if ((EndPos - StartPos) >= 4) 
      {
        EfiCopyMem (&HidItem->Data.U32, StartPos, sizeof (UINT32));
        StartPos += 4;
        return StartPos;
      }
    }
  }
  return NULL;
}

static 
UINT32 
GetItemData(
  IN  HID_ITEM *HidItem
  )
{
  //
  // Get Data from HID_ITEM structure
  //
  switch (HidItem->Size) {
    case 1: return HidItem->Data.U8;
    case 2: return HidItem->Data.U16;
    case 4: return HidItem->Data.U32;
  }
  return 0;
}


static 
VOID
ParseLocalItem(
  IN  USB_MOUSE_DEV   *UsbMouse,
  IN  HID_ITEM        *LocalItem
  )
{
  UINT32 Data;

  if (LocalItem->Size == 0) 
  {
    //
    // No expected data for local item
    //
    return;
  }

  Data = GetItemData(LocalItem);

  switch (LocalItem->Tag) 
  {
  case HID_LOCAL_ITEM_TAG_DELIMITER:

    //
    // we don't support delimiter here
    //
    return;

  case HID_LOCAL_ITEM_TAG_USAGE:
    return;

  case HID_LOCAL_ITEM_TAG_USAGE_MINIMUM:
    if(UsbMouse->PrivateData.ButtonDetected) {
      UsbMouse->PrivateData.ButtonMinIndex = (UINT8)Data;
    }
    return;

  case HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM:
    {
      if(UsbMouse->PrivateData.ButtonDetected) {
        UsbMouse->PrivateData.ButtonMaxIndex = (UINT8)Data;
      }
      return;
    }
  }
}

static 
VOID
ParseGlobalItem(
  IN  USB_MOUSE_DEV   *UsbMouse,
  IN  HID_ITEM        *GlobalItem
  )
{
  
  switch (GlobalItem->Tag) 
  {
  case HID_GLOBAL_ITEM_TAG_USAGE_PAGE:
    {
      UINT8  UsagePage;

      UsagePage = (UINT8)GetItemData(GlobalItem);

      //
      // We only care Button Page here
      //
      if(UsagePage == 0x09) //Button Page
      {
        UsbMouse->PrivateData.ButtonDetected = TRUE;
        return;
      }
      break;
    }

  }
}

static
VOID
ParseMainItem(
  IN  USB_MOUSE_DEV   *UsbMouse,
  IN  HID_ITEM        *MainItem
  )
{
    //
    // we don't care any main items, just skip
    //
    return;
}

static
void
ParseHidItem(
  IN  USB_MOUSE_DEV   *UsbMouse,
  IN  HID_ITEM        *HidItem
  )
{
  switch(HidItem->Type)
  {
  case HID_ITEM_TYPE_MAIN:
    //
    // For Main Item, parse main item
    //
    ParseMainItem(UsbMouse, HidItem);
    break;

  case HID_ITEM_TYPE_GLOBAL:
    //
    // For global Item, parse global item
    //
    ParseGlobalItem(UsbMouse, HidItem);
    break;

  case HID_ITEM_TYPE_LOCAL:
    //
    // For Local Item, parse local item
    //
    ParseLocalItem(UsbMouse, HidItem);
    break;
  }
}


//
// A simple parse just read some field we are interested in
//
EFI_STATUS
ParseMouseReportDescriptor (
  IN  USB_MOUSE_DEV   *UsbMouse,
  IN  UINT8           *ReportDescriptor,
  IN  UINTN           ReportSize
  )
{
  UINT8       *DescriptorEnd;
  UINT8       *ptr;
  HID_ITEM    HidItem;

  DescriptorEnd = ReportDescriptor + ReportSize;

  ptr = GetNextItem(ReportDescriptor, DescriptorEnd, &HidItem);

  while (ptr != NULL) 
  {
    if (HidItem.Format != HID_ITEM_FORMAT_SHORT) 
    {
      //
      // Long Format Item is not supported at current HID revision
      //
      return EFI_DEVICE_ERROR;
    }

    ParseHidItem(UsbMouse, &HidItem);

    ptr = GetNextItem(ptr, DescriptorEnd, &HidItem);
  }

  UsbMouse->NumberOfButtons = (UINT8)(UsbMouse->PrivateData.ButtonMaxIndex - UsbMouse->PrivateData.ButtonMinIndex + 1);
  UsbMouse->XLogicMax = UsbMouse->YLogicMax = 127;
  UsbMouse->XLogicMin = UsbMouse->YLogicMin = -127;

  return EFI_SUCCESS;
}

