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

  fatfs.h

Abstract:

  Definitions for on-disk FAT structures

Revision History

--*/

#ifndef _FATFS_H_
#define _FATFS_H_

#pragma pack(1)


//
// Boot Sector
//

#define FAT_BOOT_SECTOR_DIRTY            0x01

typedef struct {
  UINT8           Ia32Jump[3];
  CHAR8           OemId[8];

  UINT16          SectorSize;
  UINT8           SectorsPerCluster;
  UINT16          ReservedSectors;
  UINT8           NoFats;
  UINT16          RootEntries;            // < FAT32, root dir is fixed size
  UINT16          Sectors;
  UINT8           Media;                  // (ignored)
  UINT16          SectorsPerFat;          // < FAT32
  UINT16          SectorsPerTrack;        // (ignored)
  UINT16          Heads;                  // (ignored)
  UINT32          HiddenSectors;          // (ignored)
  UINT32          LargeSectors;           // => FAT32
  
  UINT8           PhysicalDriveNumber;    // (ignored)
  UINT8           CurrentHead;            // holds boot_sector_dirty bit
  UINT8           Signature;              // (ignored)

  CHAR8           Id[4];
  CHAR8           FatLabel[11];
  CHAR8           SystemId[8];

} FAT_BOOT_SECTOR;

typedef struct {
  UINT8           Ia32Jump[3];
  CHAR8           OemId[8];

  UINT16          SectorSize;
  UINT8           SectorsPerCluster;
  UINT16          ReservedSectors;
  UINT8           NoFats;
  UINT16          RootEntries;            // < FAT32, root dir is fixed size
  UINT16          Sectors;
  UINT8           Media;                  // (ignored)
  UINT16          SectorsPerFat;          // < FAT32
  UINT16          SectorsPerTrack;        // (ignored)
  UINT16          Heads;                  // (ignored)
  UINT32          HiddenSectors;          // (ignored)
  UINT32          LargeSectors;           // Used if Sectors==0

  UINT32          LargeSectorsPerFat;     // FAT32
  UINT16          ExtendedFlags;          // FAT32 (ignored)
  UINT16          FsVersion;              // FAT32 (ignored)
  UINT32          RootDirFirstCluster;    // FAT32
  UINT16          FsInfoSector;           // FAT32
  UINT16          BackupBootSector;       // FAT32
  UINT8           Reserved[12];           // FAT32 (ignored)

  UINT8           PhysicalDriveNumber;    // (ignored)
  UINT8           CurrentHead;            // holds boot_sector_dirty bit
  UINT8           Signature;              // (ignored)

  CHAR8           Id[4];
  CHAR8           FatLabel[11];
  CHAR8           SystemId[8];
} FAT_BOOT_SECTOR_EX;


//
// FAT Info Structure
//

typedef struct {
  UINT32          ClusterCount;       
  UINT32          NextCluster;        
} FAT_FREE_INFO;

#define FAT_INFO_SIGNATURE              0x41615252
typedef struct {
  UINT32          Signature;   
  UINT8           ExtraBootCode[480];
  UINT32          InfoBeginSignature;        
  FAT_FREE_INFO   FreeInfo;
  UINT8           Reserved[12];           
  UINT32          InfoEndSignature;     
} FAT_INFO_SECTOR;

#define FAT_INFO_BEGIN_SIGNATURE        0x61417272
#define FAT_INFO_END_SIGNATURE          0xAA550000


//
// FAT entry values
//

#define FAT_CLUSTER_SPECIAL     ((-1 & ~0xF) | 0x07)
#define FAT_CLUSTER_FREE        0
#define FAT_CLUSTER_RESERVED    (FAT_CLUSTER_SPECIAL)
#define FAT_CLUSTER_BAD         (FAT_CLUSTER_SPECIAL)
#define FAT_CLUSTER_LAST        (-1)

#define FAT_END_OF_FAT_CHAIN(Cluster) ((Cluster) > (FAT_CLUSTER_SPECIAL))


//
// Directory Entry
//

typedef struct {
  UINT16          Day:5;
  UINT16          Month:4;
  UINT16          Year:7;                 // From 1980
} FAT_DATE;

typedef struct {
  UINT16          DoubleSecond:5;
  UINT16          Minute:6;
  UINT16          Hour:5;
} FAT_TIME;

typedef struct {
  FAT_TIME        Time;
  FAT_DATE        Date;
} FAT_DATE_TIME;

typedef struct {
  CHAR8           FileName[11];           // 8.3 filename
  UINT8           Attributes;
  UINT8           CaseFlag;
  UINT8           CreateMillisecond;      // (creation milliseconds - ignored)
  FAT_DATE_TIME   FileCreateTime;
  FAT_DATE        FileLastAccess;
  UINT16          FileClusterHigh;        // >= FAT32
  FAT_DATE_TIME   FileModificationTime;
  UINT16          FileCluster;        
  UINT32          FileSize;
} FAT_DIRECTORY_ENTRY;

#define FAT_ATTRIBUTE_READ_ONLY         0x01
#define FAT_ATTRIBUTE_HIDDEN            0x02
#define FAT_ATTRIBUTE_SYSTEM            0x04
#define FAT_ATTRIBUTE_VOLUME_ID         0x08
#define FAT_ATTRIBUTE_DIRECTORY         0x10
#define FAT_ATTRIBUTE_ARCHIVE           0x20
#define FAT_ATTRIBUTE_DEVICE            0x40
#define FAT_ATTRIBUTE_LFN               0x0F

#define FAT_CASE_ONLY                   0x01    // internal flag
#define FAT_CASE_NAME_LOWER             0x08
#define FAT_CASE_EXT_LOWER              0x10

typedef struct {
  UINT8           Ordinal;
  CHAR8           Name1[10];          // (Really 5 chars, but not WCHAR aligned)
  UINT8           Attributes;
  UINT8           Type;
  UINT8           Checksum;
  CHAR16          Name2[6];
  UINT16          MustBeZero;
  CHAR16          Name3[2];
} FAT_DIRECTORY_LFN;

#define FAT_LFN_LAST                    0x40    // Ordinal field
#define MAX_LFN_ENTRIES                 20


#pragma pack()

#endif //_FATFS_H_
