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

  SerialMouse.h
  
Abstract:
  Private Data definition for Serial Mouse driver

--*/

#ifndef _SERIAL_MOUSE_H
#define _SERIAL_MOUSE_H

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (SerialIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (SimplePointer)

#define SERIAL_MOUSE_PRIVATE_DATA_SIGNATURE   EFI_SIGNATURE_32('s','e','r','m')

typedef struct {
  UINTN                        Signature;
  EFI_HANDLE                   Handle;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;
  EFI_SIMPLE_POINTER_PROTOCOL  SimplePointer;
  EFI_SIMPLE_POINTER_MODE      SimplePointerMode;
  EFI_SIMPLE_POINTER_STATE     SimplePointerState;
  BOOLEAN                      StateChanged;
  EFI_SERIAL_IO_PROTOCOL       *SerialIo;
  EFI_SERIAL_IO_MODE           OriginalSerialIoMode;
  BOOLEAN                      ControlValid;
  UINT32                       Control;
  EFI_SERIAL_IO_MODE           SerialIoMode;
  EFI_EVENT                    TimeEvent;
  EFI_UNICODE_STRING_TABLE     *ControllerNameTable;
} SERIAL_MOUSE_PRIVATE_DATA;

#define SERIAL_MOUSE_PRIVATE_DATA_FROM_THIS(a) CR(a, SERIAL_MOUSE_PRIVATE_DATA, SimplePointer, SERIAL_MOUSE_PRIVATE_DATA_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gSerialMouseDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gSerialMouseComponentName;

//
// Microsoft* Serial Mouse constants
//
// * Other names and brands may be claimed as the property of others.
//
#define	MS_PACKET_LENGTH    3
#define MS_SYNC_MASK        0x40
#define MS_READ_BYTE_ONE    0
#define MS_READ_BYTE_TWO    1
#define MS_READ_BYTE_THREE  2
#define MS_PROCESS_PACKET   3

#define IS_MS_SYNC_BYTE(Byte) (Byte & MS_SYNC_MASK)

#endif
