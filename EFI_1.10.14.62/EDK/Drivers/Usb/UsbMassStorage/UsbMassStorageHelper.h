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

    UsbMassStorageHelper.h

Abstract:

    Function prototype for USB Mass Storage Driver

Revision History
++*/
#ifndef _USB_FLPHLP_H
#define _USB_FLPHLP_H

#include "UsbMassStorage.h"

EFI_STATUS
USBFloppyIdentify (                                                    
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  );

EFI_STATUS
USBFloppyPacketCommand(
  USB_FLOPPY_DEV            *UsbFloppyDevice,
  VOID                      *Command,
  UINT8                     CommandSize,
  VOID                      *DataBuffer,
  UINT32                    BufferLength,
  EFI_USB_DATA_DIRECTION    Direction,
  UINT16                    TimeOutInMilliSeconds
  );

EFI_STATUS
USBFloppyInquiry(
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  OUT   USB_INQUIRY_DATA  **Idata
  );

EFI_STATUS
USBFloppyRead10(
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  IN    VOID              *Buffer, 
  IN    EFI_LBA           Lba, 
  IN    UINTN             NumberOfBlocks
  );


EFI_STATUS 
USBFloppyReadFormatCapacity (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  );

EFI_STATUS  
UsbFloppyRequestSense ( 
  IN  USB_FLOPPY_DEV  *UsbFloppyDevice,
  OUT UINTN           *SenseCounts
  );


EFI_STATUS                                                         
UsbFloppyTestUnitReady (                                                  
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  );



EFI_STATUS
USBFloppyWrite10(
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  IN    VOID              *Buffer, 
  IN    EFI_LBA           Lba, 
  IN    UINTN             NumberOfBlocks
  );

EFI_STATUS 
UsbFloppyDetectMedia (
  IN  USB_FLOPPY_DEV  *UsbFloppyDevice,
  OUT BOOLEAN         *MediaChange
  );

EFI_STATUS
UsbFloppyModeSense5 (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  );

EFI_STATUS
UsbFloppyModeSense1C (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  );
  
#endif  
