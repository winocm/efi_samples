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

  DeviceIo.h

Abstract:

  Device IO protocol as defined in the EFI 1.0 specification.

  Device IO is used to abstract hardware access to devices. It includes
  memory mapped IO, IO, PCI Config space, and DMA.

 
--*/

#ifndef _DEVICE_IO_H_
#define _DEVICE_IO_H_

#define EFI_DEVICE_IO_PROTOCOL_GUID \
  { 0xaf6ac311, 0x84c3, 0x11d2, 0x8e, 0x3c, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

EFI_INTERFACE_DECL(_EFI_DEVICE_IO_PROTOCOL);

typedef enum {
  IO_UINT8,
  IO_UINT16,
  IO_UINT32,
  IO_UINT64,
  MMIO_COPY_UINT8,
  MMIO_COPY_UINT16,
  MMIO_COPY_UINT32,
  MMIO_COPY_UINT64
} EFI_IO_WIDTH;

typedef
EFI_STATUS
(EFIAPI *EFI_DEVICE_IO) (
  IN struct _EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH                   Width,
  IN UINT64                         Address,
  IN UINTN                          Count,
  IN OUT VOID                       *Buffer
  );

typedef struct {
  EFI_DEVICE_IO                   Read;
  EFI_DEVICE_IO                   Write;
} EFI_IO_ACCESS;

typedef 
EFI_STATUS
(EFIAPI *EFI_PCI_DEVICE_PATH) (
  IN struct _EFI_DEVICE_IO_PROTOCOL   *This,
  IN UINT64                           Address,
  IN OUT EFI_DEVICE_PATH_PROTOCOL     **PciDevicePath
  );

typedef enum {
  EfiBusMasterRead,
  EfiBusMasterWrite,
  EfiBusMasterCommonBuffer
} EFI_IO_OPERATION_TYPE;

typedef
EFI_STATUS
(EFIAPI *EFI_IO_MAP) (
  IN struct _EFI_DEVICE_IO_PROTOCOL   *This,
  IN EFI_IO_OPERATION_TYPE            Operation,
  IN EFI_PHYSICAL_ADDRESS             *HostAddress,
  IN OUT UINTN                        *NumberOfBytes,
  OUT EFI_PHYSICAL_ADDRESS            *DeviceAddress,
  OUT VOID                            **Mapping
  );

typedef
EFI_STATUS
(EFIAPI *EFI_IO_UNMAP) (
  IN struct _EFI_DEVICE_IO_PROTOCOL   *This,
  IN VOID                             *Mapping
  );

typedef
EFI_STATUS
(EFIAPI *EFI_IO_ALLOCATE_BUFFER) (
  IN struct _EFI_DEVICE_IO_PROTOCOL   *This,
  IN EFI_ALLOCATE_TYPE                Type,
  IN EFI_MEMORY_TYPE                  MemoryType,
  IN UINTN                            Pages,
  IN OUT EFI_PHYSICAL_ADDRESS         *HostAddress
  );

typedef
EFI_STATUS
(EFIAPI *EFI_IO_FLUSH) (
  IN struct _EFI_DEVICE_IO_PROTOCOL  *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_IO_FREE_BUFFER) (
  IN struct _EFI_DEVICE_IO_PROTOCOL   *This,
  IN UINTN                            Pages,
  IN EFI_PHYSICAL_ADDRESS             HostAddress
  );

typedef struct _EFI_DEVICE_IO_PROTOCOL {
  EFI_IO_ACCESS                       Mem;
  EFI_IO_ACCESS                       Io;
  EFI_IO_ACCESS                       Pci;
  EFI_IO_MAP                          Map;
  EFI_PCI_DEVICE_PATH                 PciDevicePath;
  EFI_IO_UNMAP                        Unmap;
  EFI_IO_ALLOCATE_BUFFER              AllocateBuffer;
  EFI_IO_FLUSH                        Flush;
  EFI_IO_FREE_BUFFER                  FreeBuffer;
} EFI_DEVICE_IO_PROTOCOL;


extern EFI_GUID gEfiDeviceIoProtocolGuid;

#endif
