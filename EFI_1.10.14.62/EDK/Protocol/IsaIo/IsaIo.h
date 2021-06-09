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

    IsaIo.h
    
Abstract:

    EFI ISA I/O Protocol

Revision History

--*/

#ifndef _EFI_ISA_IO_H
#define _EFI_ISA_IO_H

#include EFI_PROTOCOL_DEFINITION(IsaAcpi)

//
// Global ID for the ISA I/O Protocol
//

#define EFI_ISA_IO_PROTOCOL_GUID \
  { 0x7ee2bd44, 0x3da0, 0x11d4, 0x9a, 0x38, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

EFI_INTERFACE_DECL(_EFI_ISA_IO_PROTOCOL);

//
// Prototypes for the ISA I/O Protocol
//

typedef enum {
  EfiIsaIoWidthUint8,
  EfiIsaIoWidthUint16,
  EfiIsaIoWidthUint32,
  EfiIsaIoWidthReserved,
  EfiIsaIoWidthFifoUint8,
  EfiIsaIoWidthFifoUint16,
  EfiIsaIoWidthFifoUint32,
  EfiIsaIoWidthFifoReserved,
  EfiIsaIoWidthFillUint8,
  EfiIsaIoWidthFillUint16,
  EfiIsaIoWidthFillUint32,
  EfiIsaIoWidthFillReserved,
  EfiIsaIoWidthMaximum
} EFI_ISA_IO_PROTOCOL_WIDTH;

//
// Attributes for common buffer allocations
//
#define EFI_ISA_IO_ATTRIBUTE_MEMORY_WRITE_COMBINE  0x080    // Map a memory range so write are combined
#define EFI_ISA_IO_ATTRIBUTE_MEMORY_CACHED         0x800    // Map a memory range so all r/w accesses are cached
#define EFI_ISA_IO_ATTRIBUTE_MEMORY_DISABLE        0x1000   // Disable a memory range 

//
// Channel attribute for DMA operations
//
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_COMPATIBLE  0x001
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_A           0x002
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_B           0x004
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_C           0x008
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_8           0x010
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_16          0x020
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SINGLE_MODE       0x040
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_DEMAND_MODE       0x080
#define EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_AUTO_INITIALIZE   0x100

typedef enum {
  EfiIsaIoOperationBusMasterRead,
  EfiIsaIoOperationBusMasterWrite,
  EfiIsaIoOperationBusMasterCommonBuffer,
  EfiIsaIoOperationSlaveRead,
  EfiIsaIoOperationSlaveWrite,
  EfiIsaIoOperationMaximum
} EFI_ISA_IO_PROTOCOL_OPERATION;

typedef
EFI_STATUS
(EFIAPI *EFI_ISA_IO_PROTOCOL_IO_MEM) (
  IN     struct _EFI_ISA_IO_PROTOCOL  *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH    Width,
  IN     UINT32                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  );

typedef struct {
  EFI_ISA_IO_PROTOCOL_IO_MEM  Read;
  EFI_ISA_IO_PROTOCOL_IO_MEM  Write;
} EFI_ISA_IO_PROTOCOL_ACCESS;

typedef
EFI_STATUS
(EFIAPI *EFI_ISA_IO_PROTOCOL_COPY_MEM) (
  IN     struct _EFI_ISA_IO_PROTOCOL  *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH    Width,
  IN     UINT32                       DestOffset,
  IN     UINT32                       SrcOffset,
  IN     UINTN                        Count
  );

typedef
EFI_STATUS
(EFIAPI *EFI_ISA_IO_PROTOCOL_MAP) (
  IN     struct _EFI_ISA_IO_PROTOCOL    *This,
  IN     EFI_ISA_IO_PROTOCOL_OPERATION  Operation,
  IN     UINT8                          ChannelNumber      OPTIONAL,
  IN     UINT32                         ChannelAttributes,
  IN     VOID                           *HostAddress,
  IN OUT UINTN                          *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
  OUT    VOID                           **Mapping
  );

typedef
EFI_STATUS
(EFIAPI *EFI_ISA_IO_PROTOCOL_UNMAP) (
  IN  struct _EFI_ISA_IO_PROTOCOL  *This,
  IN  VOID                         *Mapping
  );

typedef
EFI_STATUS
(EFIAPI *EFI_ISA_IO_PROTOCOL_ALLOCATE_BUFFER) (
  IN  struct _EFI_ISA_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE            Type,
  IN  EFI_MEMORY_TYPE              MemoryType,
  IN  UINTN                        Pages,
  OUT VOID                         **HostAddress,
  IN  UINT64                       Attributes
  );

typedef
EFI_STATUS
(EFIAPI *EFI_ISA_IO_PROTOCOL_FREE_BUFFER) (
  IN  struct _EFI_ISA_IO_PROTOCOL  *This,
  IN  UINTN                        Pages,
  IN  VOID                         *HostAddress
  );

typedef
EFI_STATUS
(EFIAPI *EFI_ISA_IO_PROTOCOL_FLUSH) (
  IN  struct _EFI_ISA_IO_PROTOCOL  *This
  );

//
// Interface structure for the ISA I/O Protocol
//
typedef struct _EFI_ISA_IO_PROTOCOL {
  EFI_ISA_IO_PROTOCOL_ACCESS           Mem;
  EFI_ISA_IO_PROTOCOL_ACCESS           Io;
  EFI_ISA_IO_PROTOCOL_COPY_MEM         CopyMem;
  EFI_ISA_IO_PROTOCOL_MAP              Map;
  EFI_ISA_IO_PROTOCOL_UNMAP            Unmap;
  EFI_ISA_IO_PROTOCOL_ALLOCATE_BUFFER  AllocateBuffer;
  EFI_ISA_IO_PROTOCOL_FREE_BUFFER      FreeBuffer;
  EFI_ISA_IO_PROTOCOL_FLUSH            Flush;
  EFI_ISA_ACPI_RESOURCE_LIST           *ResourceList;
  UINT32                               RomSize;
  VOID                                 *RomImage;
} EFI_ISA_IO_PROTOCOL;

extern EFI_GUID gEfiIsaIoProtocolGuid;

#endif
