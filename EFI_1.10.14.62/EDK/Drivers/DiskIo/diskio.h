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

  DiskIo.h
  
Abstract:
  Private Data definition for Disk IO driver

--*/

#ifndef _DISK_IO_H
#define _DISK_IO_H

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (BlockIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DiskIo)


#define DISK_IO_PRIVATE_DATA_SIGNATURE   EFI_SIGNATURE_32('d','s','k','I')

#define DATA_BUFFER_BLOCK_NUM    (64)

typedef struct {
  UINTN                   Signature;
  EFI_DISK_IO_PROTOCOL    DiskIo;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  UINT32                  BlockSize;
} DISK_IO_PRIVATE_DATA;

#define DISK_IO_PRIVATE_DATA_FROM_THIS(a) CR(a, DISK_IO_PRIVATE_DATA, DiskIo, DISK_IO_PRIVATE_DATA_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gDiskIoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gDiskIoComponentName;

#endif
