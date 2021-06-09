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
    
      defio.h

Abstract:




Revision History

--*/

//
//
//

extern UINT64  RtVirtualIoBase;
extern UINT64  RtVirtualFlashBase;


#define IO_DEVICE_SIGNATURE     EFI_SIGNATURE_32('i','o','d','v')
typedef struct {
    UINTN                       Signature;
    UINTN                       MemBase;
    UINTN                       IoBase;

    FLOCK                       PciLock;
    UINTN                       PciAddress;
    UINTN                       PciData;

    EFI_DEVICE_IO_INTERFACE     Io;
    EFI_DEVICE_PATH             *DevicePath;    
} IO_DEVICE;

#define IO_DEVICE_FROM_THIS(a) CR(a, IO_DEVICE, Io, IO_DEVICE_SIGNATURE);
//
//
//

typedef union {
    UINT8       VOLATILE    *buf;
    UINT8       VOLATILE    *ui8;
    UINT16      VOLATILE    *ui16;
    UINT32      VOLATILE    *ui32;
    UINT64      VOLATILE    *ui64;
    UINTN       VOLATILE    ui;
} PTR;


EFI_STATUS
INTERNAL
DefMemoryRead (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   Address,
    IN UINTN                    Count,
    IN OUT VOID                 *Buffer
    );


EFI_STATUS
INTERNAL
DefMemoryWrite (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   Address,
    IN UINTN                    Count,
    IN OUT VOID                 *Buffer
    );

EFI_STATUS
RUNTIMEFUNCTION
RtDefIoRead (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    );

EFI_STATUS
RUNTIMEFUNCTION
RtDefIoWrite (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    );

EFI_STATUS
INTERNAL
DefIoRead (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    );

EFI_STATUS
INTERNAL
DefIoWrite (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    );

EFI_STATUS
INTERNAL
DefPciRead (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    );

EFI_STATUS
INTERNAL
DefPciWrite (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    );

EFI_STATUS
INTERNAL
ReadWritePciConfigSpace (
    IN  EFI_DEVICE_IO_INTERFACE     *This,
    IN  BOOLEAN                     Write,
    IN  EFI_IO_WIDTH                Width,
    IN  UINT64                      UserAddress,
    IN  UINTN                       Count,
    IN  OUT VOID                    *UserBuffer
    );

EFI_STATUS
PlInstallDefaultIoDevice (
    IN EFI_DEVICE_PATH          *DevicePath,
    IN UINTN                    MemBase,
    IN UINTN                    IoBase
    );

VOID
InitializePciRootBusList (
    IN OUT LIST_ENTRY   *ListHead
    );

VOID *
FindAcpiRsdPtr (
    VOID
    );

VOID *
FindSMBIOSPtr (
    VOID
    );

VOID *
FindMPSPtr (
    VOID
    );

//
// Globals
//

extern EFI_DEVICE_IO_INTERFACE      *GlobalIoFncs; 
extern LIST_ENTRY                   PciRootBusList;
extern EFI_DEVICE_PATH             *LegacyDevicePath;


