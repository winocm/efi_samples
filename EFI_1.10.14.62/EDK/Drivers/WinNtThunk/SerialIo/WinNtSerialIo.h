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

  WinNtSerialIo.h

Abstract:


--*/

#ifndef _WIN_NT_SERIAL_IO_
#define _WIN_NT_SERIAL_IO_

#include "EfiWinNt.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (WinNtIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SerialIo)

#define WIN_NT_SERIAL_IO_PRIVATE_DATA_SIGNATURE   EFI_SIGNATURE_32('N','T','s','i')
typedef struct {
  UINT64                            Signature;

  //
  // Protocol data for the new handle we are going to add
  //
  EFI_HANDLE                        Handle;
  EFI_SERIAL_IO_PROTOCOL            SerialIo;
  EFI_SERIAL_IO_MODE                SerialIoMode;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;

  //
  // Private Data
  //
  EFI_HANDLE                        ControllerHandle;
  EFI_DEVICE_PATH_PROTOCOL          *ParentDevicePath;
  UART_DEVICE_PATH                  UartDevicePath;
  EFI_WIN_NT_THUNK_PROTOCOL         *WinNtThunk;

  EFI_UNICODE_STRING_TABLE          *ControllerNameTable;

  //
  // Private NT type Data;
  //
  HANDLE                            NtHandle;
  DCB                               NtDCB;
  DWORD                             NtError;
  COMSTAT                           NtComStatus;
 
} WIN_NT_SERIAL_IO_PRIVATE_DATA;

#define WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, WIN_NT_SERIAL_IO_PRIVATE_DATA, SerialIo, WIN_NT_SERIAL_IO_PRIVATE_DATA_SIGNATURE)

//
// Global Protocol Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gWinNtSerialIoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gWinNtSerialIoComponentName;

//
// Macros to convert EFI serial types to NT serial types.
//

#define SERIAL_TIMEOUT_DEFAULT  (1000 * 1000)       // one second
#define SERIAL_BAUD_DEFAULT     115200
#define SERIAL_FIFO_DEFAULT     14
#define SERIAL_DATABITS_DEFAULT 8
#define SERIAL_PARITY_DEFAULT   DefaultParity
#define SERIAL_STOPBITS_DEFAULT DefaultStopBits

#define SERIAL_CONTROL_MASK     (EFI_SERIAL_CLEAR_TO_SEND       | \
                                 EFI_SERIAL_DATA_SET_READY      | \
                                 EFI_SERIAL_RING_INDICATE       | \
                                 EFI_SERIAL_CARRIER_DETECT      | \
                                 EFI_SERIAL_REQUEST_TO_SEND     | \
                                 EFI_SERIAL_DATA_TERMINAL_READY | \
                                 EFI_SERIAL_INPUT_BUFFER_EMPTY)

#define ConvertBaud2Nt(x)     (DWORD)x
#define ConvertData2Nt(x)     (BYTE)x

#define ConvertParity2Nt(x)              \
    (BYTE) (                             \
    x == DefaultParity ? NOPARITY    :   \
    x == NoParity      ? NOPARITY    :   \
    x == EvenParity    ? EVENPARITY  :   \
    x == OddParity     ? ODDPARITY   :   \
    x == MarkParity    ? MARKPARITY  :   \
    x == SpaceParity   ? SPACEPARITY : 0 \
    )

#define ConvertStop2Nt(x)                 \
    (BYTE) (                                \
    x == DefaultParity   ? ONESTOPBIT   :   \
    x == OneFiveStopBits ? ONE5STOPBITS :   \
    x == TwoStopBits     ? TWOSTOPBITS  : 0 \
    )

#define ConvertTime2Nt(x)    ((x) / 1000)

//
//
//
#define SERIAL_PORT_MAX_BAUD_RATE          115400   // 115200 baud with rounding errors

//
// Function Prototypes
//

EFI_STATUS
InitializeWinNtSerialIo (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  );

STATIC
EFI_STATUS
EFIAPI
WinNtSerialIoDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
WinNtSerialIoDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
WinNtSerialIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  );

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoReset (
  IN EFI_SERIAL_IO_PROTOCOL *This
  );

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT64                 BaudRate,
  IN UINT32                 ReceiveFifoDepth,
  IN UINT32                 Timeout,
  IN EFI_PARITY_TYPE        Parity,
  IN UINT8                  DataBits,
  IN EFI_STOP_BITS_TYPE     StopBits
  );

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoSetControl (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT32                 Control
  );

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoGetControl (
  IN  EFI_SERIAL_IO_PROTOCOL  *This,
  OUT UINT32                  *Control
  );

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoWrite (
  IN EFI_SERIAL_IO_PROTOCOL   *This,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  );

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoRead (
  IN  EFI_SERIAL_IO_PROTOCOL  *This,
  IN  OUT UINTN               *BufferSize,
  OUT VOID                    *Buffer
  );

#endif
