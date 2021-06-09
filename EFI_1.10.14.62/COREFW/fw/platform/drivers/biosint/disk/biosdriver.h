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

    BiosDriver.h

Abstract:

    Bios Platform Include File



Revision History

--*/

#include "efifw.h"
#include "edd.h"
#include "int86.h"
#include "pci22.h"

VOID
BuildEDD30DevicePath (
    IN  BIOS_LEGACY_DRIVE   *Drive,
    IN  EFI_DEVICE_PATH     **DevPath
    );

UINTN
BiosGetNumberOfDiskettes (
    VOID
    );

UINTN
BiosGetNumberOfHardDrives (
    VOID
    );

BOOLEAN
BiosInitBlkIo (
    IN  BIOS_BLK_IO_DEV     *Dev
    );

EFI_STATUS
EDD30BiosReadBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );

EFI_STATUS
EDD30BiosWriteBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );

EFI_STATUS 
BiosBlkIoFlushBlocks (
    IN  EFI_BLOCK_IO  *This
    );

EFI_STATUS 
BiosBlkIoReset (
    IN  EFI_BLOCK_IO        *This,
    IN  BOOLEAN             ExtendedVerification
    );

EFI_STATUS
EDD11BiosReadBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );

EFI_STATUS
EDD11BiosWriteBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );

EFI_STATUS
BiosReadLegacyDrive (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );

EFI_STATUS
BiosWriteLegacyDrive (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );
