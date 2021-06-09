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

  Gpt.h
  
Abstract:

  Data Structures required for detecting GPT Partitions
  
Revision History

--*/

#ifndef _GPT_H_
#define _GPT_H_

#pragma pack(1)

#define PRIMARY_PART_HEADER_LBA         1

#define EFI_PTAB_HEADER_ID  "EFI PART"

//
// EFI Partition Attributes
//
#define EFI_PART_REQUIRED_TO_FUNCTION   0x0000000000000001

//
// GPT Partition Table Header
//
typedef struct {
    EFI_TABLE_HEADER    Header;
    EFI_LBA             MyLBA;
    EFI_LBA             AlternateLBA;
    EFI_LBA             FirstUsableLBA;
    EFI_LBA             LastUsableLBA;
    EFI_GUID            DiskGUID;
    EFI_LBA             PartitionEntryLBA;
    UINT32              NumberOfPartitionEntries;
    UINT32              SizeOfPartitionEntry;
    UINT32              PartitionEntryArrayCRC32;
} EFI_PARTITION_TABLE_HEADER;

//
// GPT Partition Entry
//
typedef struct {
    EFI_GUID    PartitionTypeGUID;
    EFI_GUID    UniquePartitionGUID;
    EFI_LBA     StartingLBA;
    EFI_LBA     EndingLBA;
    UINT64      Attributes;
    CHAR16      PartitionName[36];
} EFI_PARTITION_ENTRY;

//
// GPT Partition Entry Status
//
typedef struct {
    BOOLEAN     OutOfRange;
    BOOLEAN     Overlap;
} EFI_PARTITION_ENTRY_STATUS;

#pragma pack()

#endif
