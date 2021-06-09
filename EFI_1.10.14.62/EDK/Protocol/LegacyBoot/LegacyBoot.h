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
  
    LegacyBoot.h

Abstract:

    EFI support for legacy boot



Revision History

--*/

#ifndef _LEGACY_BOOT_INCLUDE_
#define _LEGACY_BOOT_INCLUDE_

#define EFI_LEGACY_BOOT_PROTOCOL_GUID \
    { 0x376e5eb2, 0x30e4, 0x11d3, { 0xba, 0xe5, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } }

EFI_INTERFACE_DECL(_EFI_LEGACY_BOOT_PROTOCOL);

#pragma pack(1)
//
// BBS 1.01 (See Appendix A) IPL and BCV Table Entry Data structure.
//  Seg:Off pointers have been converted to EFI pointers in this data structure
//  This is the structure that also maps to the EFI device path for the boot selection
//
typedef struct {
    UINT16  DeviceType;
    UINT16  StatusFlag;
    UINT32  Reserved;
    VOID    *BootHandler;   // Not an EFI entry point
    CHAR8   *DescString;
} BBS_TABLE_ENTRY;
#pragma pack()

typedef
EFI_STATUS
(EFIAPI *EFI_LEGACY_BOOT_CALL) (
    IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
    );


//
// BBS support functions
//  PnP Call numbers and BiosSelector hidden in implementation
//

typedef enum {
    IplRelative,
    BcvRelative
} BBS_TYPE;


//
// == PnP Function 0x60 then BbsVersion == 0x0101 if this call fails then BbsVersion == 0x0000
//

//
// == PnP Function 0x61
//
typedef
EFI_STATUS
(EFIAPI *EFI_GET_DEVICE_COUNT) (
    IN  struct _EFI_LEGACY_BOOT_PROTOCOL   *This,
    IN  BBS_TYPE        *TableType,
    OUT UINTN           *DeviceCount,
    OUT UINTN           *MaxCount
    );

//
// == PnP Function 0x62
//
typedef
EFI_STATUS
(EFIAPI *EFI_GET_PRIORITY_AND_TABLE) (
    IN  struct _EFI_LEGACY_BOOT_PROTOCOL   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize, // MaxCount * sizeof(UINT8)
    OUT     UINTN       *Priority,
    IN OUT  UINTN       *TableSize,    // MaxCount * sizeof(BBS_TABLE_ENTRY)
    OUT BBS_TABLE_ENTRY *TableEntrySize
    );

//
// == PnP Function 0x63
//
typedef
EFI_STATUS
(EFIAPI *EFI_SET_PRIORITY) (
    IN  struct _EFI_LEGACY_BOOT_PROTOCOL   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize,
    OUT     UINTN       *Priority
    );

typedef struct _EFI_LEGACY_BOOT_PROTOCOL {
    EFI_LEGACY_BOOT_CALL        BootIt;

    //
    // New functions to allow BBS booting to be configured from EFI
    //
    UINTN                       BbsVersion;     // Currently 0x0101
    EFI_GET_DEVICE_COUNT        GetDeviceCount;
    EFI_GET_PRIORITY_AND_TABLE  GetPriorityAndTable;
    EFI_SET_PRIORITY            SetPriority;   
} EFI_LEGACY_BOOT_PROTOCOL;

EFI_STATUS
PlInitializeLegacyBoot (
    VOID
    );

#endif
