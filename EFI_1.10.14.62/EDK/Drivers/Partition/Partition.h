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

  Partition.h
  
Abstract:

  Partition driver that produces logical BlockIo devices from a physical 
  BlockIo device. The logical BlockIo devices are based on the format
  of the raw block devices media. Currently "El Torito CD-ROM", Legacy 
  MBR, and GPT partition schemes are supported.

Revision History

--*/

#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (BlockIo)
#include EFI_PROTOCOL_DEFINITION (DiskIo)

//
// Driver Consumed Guids
//
#include EFI_GUID_DEFINITION (Gpt)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

//
// Partition private data
//
#define PARTITION_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32('P','a','r','t')
typedef struct {
  UINT64                        Signature;

  EFI_HANDLE                    Handle;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_BLOCK_IO_PROTOCOL         BlockIo;
  EFI_BLOCK_IO_MEDIA            Media;

  EFI_DISK_IO_PROTOCOL          *DiskIo;
  EFI_BLOCK_IO_PROTOCOL         *ParentBlockIo;
  UINT64                        Start;
  UINT64                        End;
  UINT32                        BlockSize;

  EFI_GUID                      *EspGuid;

} PARTITION_PRIVATE_DATA;

#define PARTITION_DEVICE_FROM_BLOCK_IO_THIS(a) \
  CR(a, PARTITION_PRIVATE_DATA, BlockIo, PARTITION_PRIVATE_DATA_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gPartitionDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPartitionComponentName;

//
// Extract INT32 from char array
//
#define UNPACK_INT32(a) (INT32)( (((UINT8 *) a)[0] <<  0) |    \
                                 (((UINT8 *) a)[1] <<  8) |    \
                                 (((UINT8 *) a)[2] << 16) |    \
                                 (((UINT8 *) a)[3] << 24) )

//
// Extract UINT32 from char array
//
#define UNPACK_UINT32(a) (UINT32)( (((UINT8 *) a)[0] <<  0) |    \
                                   (((UINT8 *) a)[1] <<  8) |    \
                                   (((UINT8 *) a)[2] << 16) |    \
                                   (((UINT8 *) a)[3] << 24) )

EFI_STATUS
PartitionInstallChildHandle (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ParentHandle,
  IN  EFI_DISK_IO_PROTOCOL         *ParentDiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *ParentBlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *ParentDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN  UINT64                       Start,
  IN  UINT64                       End,
  IN  UINT32                       BlockSize,
  IN  BOOLEAN                      InstallEspGuid
  );

BOOLEAN
PartitionInstallGptChildHandles (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DISK_IO_PROTOCOL         *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  );

BOOLEAN
PartitionInstallElToritoChildHandles (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DISK_IO_PROTOCOL         *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  );

BOOLEAN
PartitionInstallMbrChildHandles (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DISK_IO_PROTOCOL         *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  );

#endif
