/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  fat.h

Abstract:

  Main header file for EFI FAT file system driver

Revision History

--*/

#ifndef _FAT_H_
#define _FAT_H_

#include "Efi.h"
#include "EfiDriverLib.h"
#include "fatfs.h"


//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (BlockIo)
#include EFI_PROTOCOL_DEFINITION (DiskIo)
#include EFI_PROTOCOL_DEFINITION (UnicodeCollation)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleFileSystem)

#include EFI_GUID_DEFINITION (GlobalVariable)

//
// The fat types we support
//
typedef enum {
  FAT12,
  FAT16,
  FAT32,
  FatUndefined
} FAT_VOLUME_TYPE;

//
// Cache
//

#define FAT_CACHE_SIZE              16
#define FAT_CACHE_PAGE_MAX_SIZE     (8 * 1024)

//
// Cache entry
//
typedef struct {
  BOOLEAN               Valid;
  BOOLEAN               Dirty;
  UINTN                 PageNo;
  UINTN                 Lru;
  UINT8                 *Data;
} CACHE_BUFFER;


//
// FAT volume
//
#define FAT_VOLUME_SIGNATURE    EFI_SIGNATURE_32('f','a','t','v')
typedef struct {
  UINTN                   Signature;

  EFI_HANDLE              Handle;
  BOOLEAN                 Valid;
  BOOLEAN                 DiskError;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   VolInterface;

  //
  // If opened, the parent handle and BlkIo interface
  //
  EFI_BLOCK_IO_PROTOCOL   *BlkIo;
  EFI_DISK_IO_PROTOCOL    *DiskIo;
  UINTN                   BlockSize;
  UINT64                  VolSize;
  UINT32                  MediaId;
  BOOLEAN                 ReadOnly;

  //
  // Unpacked Fat BPB info
  //
  UINTN                   SectorSize;
  UINTN                   SectorsPerCluster;
  UINTN                   ReservedSectors;
  UINTN                   NoFats;
  UINTN                   RootEntries;      // < FAT32, root dir is fixed size
  UINTN                   RootCluster;      // >= FAT32, root cluster chain head
  UINTN                   Sectors;
  UINTN                   Media;                 
  UINTN                   SectorsPerFat;         
  FAT_VOLUME_TYPE         FatType;

  
  //
  // Other, computed, values from fat bpb info
  // 
  UINT64                  FatPos;                 // Disk pos of fat tables
  UINTN                   FatSize;                // # of bytes in each fat
  UINT64                  RootPos;                // Disk pos of root directory
  UINT64                  FirstClusterPos;        // Disk pos of first cluster
  UINTN                   MaxCluster;             // Max cluster #
  UINTN                   ClusterSize;

  //
  // Current part of fat table that's present
  //
  UINT64                  FatEntryPos;         // Location of buffer
  UINTN                   FatEntrySize;        // Size of buffer
  UINT32                  FatEntryBuffer;      // The buffer
  
  FAT_INFO_SECTOR         FatInfoSector;       // Free cluster info
  UINTN                   FreeInfoPos;         // Pos with the free cluster info
  BOOLEAN                 FreeInfoValid;       // If free cluster info is valid

  //
  // info for marking the volume dirty or not
  //
  BOOLEAN                 FatDirty;         // If fat-entries have been updated
  UINT32                  DirtyValue;
  UINT32                  NotDirtyValue;

  //
  // The opened root file
  //
  struct _FAT_OFILE       *Root;

  //
  // New OFiles are added to this list so they
  // can be cleaned up if they aren't referenced.
  //
  EFI_LIST_ENTRY          CheckRef;
  
  CACHE_BUFFER            Cache[FAT_CACHE_SIZE]; 
  UINTN                   CachePageSize;

  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;

} FAT_VOLUME;


#define ASSERT_VOLUME_LOCKED(a)     ASSERT_LOCKED(&FatFsLock)

//
// FAT_IFILE - Each opened file instance
//
#define FAT_IFILE_SIGNATURE     EFI_SIGNATURE_32('f','a','t','i')
typedef struct {
  UINTN                   Signature;

  EFI_FILE                Handle;

  UINT64                  Position;
  BOOLEAN                 ReadOnly;

  struct _FAT_OFILE       *OFile;
  EFI_LIST_ENTRY          Link;

} FAT_IFILE;

#define IFILE_FROM_FHAND(a) CR(a, FAT_IFILE, Handle, FAT_IFILE_SIGNATURE)


//
// FAT_OFILE - Each opened file
//
typedef enum {
  IsEmpty,
  IsFile,
  IsDir,
  IsPreserve
} FAT_DIR_ENTRY_TYPE;

#define HASH_WORD_MASK        0xFF  //256 Hash entries
#define HASH_NODE_TERMINATOR  0xffffffff
#define HASH_NODE_BLOCK_UNIT  8

typedef struct {
  UINT32    Position;
} HASH_NODE;

typedef struct {
  HASH_NODE *HashNode;
  UINT32    CountOfNodes;         //Including Terminator
  UINT32    NodeBufferSize;       //In count of nodes
} HASH_ENTRY;

#define FAT_OFILE_SIGNATURE     EFI_SIGNATURE_32('f','a','t','o')
typedef struct _FAT_OFILE {
  UINTN                   Signature;
  FAT_VOLUME              *Vol;

  //
  // A permanant error code to return to all accesses to
  // this opened file
  //
  EFI_STATUS              Error;

  //
  // A list of the IFILE instances for this OFile
  //
  EFI_LIST_ENTRY          Opens;

  //
  // Compressed fat file info
  //
  union {
  FAT_DIRECTORY_ENTRY DirEntry;
  FAT_DIRECTORY_LFN   LfnEntry;
  } u;

  FAT_DIR_ENTRY_TYPE      DirType;
  UINTN                   DirCount;
  UINT64                  DirPosition;

  //
  // Uncompressed fat file info
  //
  CHAR16                  *FileString;
  CHAR8                   File8Dot3Name[11];
  UINT8                   CaseFlag;
  UINT64                  FileSize;
  UINTN                   FileCluster;
  UINTN                   FileCurrentCluster;
  UINTN                   FileLastCluster;
  UINT8                   Attributes;
  BOOLEAN                 IsFixedRootDir;
  BOOLEAN                 IsBlank;    

  EFI_TIME                CreateTime;
  EFI_TIME                LastAccess;
  EFI_TIME                LastModification;
  BOOLEAN                 PreserveLastMod;

  //
  // Dirty is set if there have been any updates to this file 
  //
  BOOLEAN                 Dirty;

  //
  // Set by an OFile SetPosition
  //
  UINT64                  Position;       // within file
  UINT64                  PosDisk;        // on the disk
  UINTN                   PosRem;         // remaining in this disk run
  
  //
  // Performance speed ups for directory
  //
  UINT64                  CurrentEntryPos;// hint for searching for sub entries
  UINT64                  FreeEntryPos;   // hint for searching for blank/empty
                                          // entries
  CHAR16                  *NameNotFound;  // the file name that is not found 
                                          // last time

  //
  // The opened parent, and currently opened child files
  //
  struct _FAT_OFILE       *Parent;
  EFI_LIST_ENTRY          ChildHead;
  EFI_LIST_ENTRY          ChildLink;
  
  HASH_ENTRY              *ChildHashTable;  // Hash table for children
  UINT32                  HashTopPosition;  // Highest dir entry pos the Hash 
                                            // table reaches
  HASH_ENTRY              *HashEntry1;      // This Ofile's Hash Entry for LFN
  HASH_ENTRY              *HashEntry2;      // This Ofile's Hash Entry for 8.3 
                                            // name

  //
  // Link in Vol->CheckRef
  //
  EFI_LIST_ENTRY          CheckLink;

} FAT_OFILE;


//
// Other Definitions
//

#define FAT_CHAR_UPPER      0x01
#define FAT_CHAR_LOWER      0x02
#define FAT_CHAR_SYMBOL     0x04
#define FAR_CHAR_LEGAL      0x40
#define FAT_CHAR_ILLEGAL    0x80

#define MAX_LFN_NUMBER      4096

#define EFI_FILE_STRING_SIZE        260
#define CHAR_FAT_VALID              0x01

//
// Unicode
//
typedef CHAR8                   LC_ISO_639_2;
#define LC_ISO_639_2_ENTRY_SIZE 3

//
// Used in FatDiskIo
// 
typedef enum {
  READ_DISK,
  WRITE_DISK
} DISK_IO_MODE;

#define FAT_ARRAY_TO_UINT16(a) ((((UINT8*)(a))[0]) | \
                           ((((UINT8*)(a))[1]) << 8))

#define FAT_ARRAY_TO_UINT32(a) ((((UINT8*)(a))[0]) | \
                           ((((UINT8*)(a))[1]) << 8) | \
                           ((((UINT8*)(a))[2]) << 16) | \
                           ((((UINT8*)(a))[3]) << 24))

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gFatDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gFatComponentName;

//
// Externs
//
extern EFI_LOCK     FatFsLock;
extern EFI_FILE     FATFileInterface;
extern EFI_UNICODE_COLLATION_PROTOCOL   *gUnicodeCollationInterface;


//
// Function Prototypes
//

EFI_STATUS
FatInitUnicodeCollationSupport (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  LC_ISO_639_2                   *LangCode,
  OUT EFI_UNICODE_COLLATION_PROTOCOL **UnicodeCollationInterface
  );

EFI_STATUS
FatOpenDevice (
  IN OUT FAT_VOLUME           *Vol
  );

EFI_STATUS
FatOpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE                        **File
  );

EFI_STATUS
FatOFileOpen (
  IN FAT_OFILE   *OFile,
  OUT FAT_IFILE  **NewIFile,
  IN CHAR16      *FileName,
  IN UINT64      OpenMode,
  IN UINT64      Attributes
  );

EFI_STATUS
EFIAPI
FatOpen (
  IN EFI_FILE   *FHand,
  OUT EFI_FILE  **NewHandle,
  IN CHAR16     *FileName,
  IN UINT64     OpenMode,
  IN UINT64     Attributes
  );

EFI_STATUS
EFIAPI
FatGetPosition (
  IN EFI_FILE  *FHand,
  OUT UINT64   *Position
  );

EFI_STATUS
EFIAPI
FatGetInfo (
  IN EFI_FILE   *FHand,
  IN EFI_GUID   *Type,
  IN OUT UINTN  *BufferSize,
  OUT VOID      *Buffer
  );

EFI_STATUS
EFIAPI
FatSetInfo (
  IN EFI_FILE  *FHand,
  IN EFI_GUID  *Type,
  IN UINTN     BufferSize,
  IN VOID      *Buffer
  );

EFI_STATUS
EFIAPI
FatFlush (
  IN EFI_FILE  *FHand
  );

EFI_STATUS
FatOFileFlush (
  IN FAT_OFILE    *OFile
  );

EFI_STATUS
EFIAPI
FatClose (
  IN EFI_FILE  *FHand
  );

EFI_STATUS
EFIAPI
FatDelete (
  IN EFI_FILE  *FHand
  );

EFI_STATUS
EFIAPI
FatSetPosition (
  IN EFI_FILE  *FHand,
  OUT UINT64   Position
  );

EFI_STATUS
EFIAPI
FatRead (
  IN EFI_FILE   *FHand,
  IN OUT UINTN  *BufferSize,
  OUT VOID      *Buffer
  );

EFI_STATUS
EFIAPI
FatWrite (
  IN EFI_FILE   *FHand,
  IN OUT UINTN  *BufferSize,
  OUT VOID      *Buffer
  );

EFI_STATUS
FatOFilePosition (
  IN FAT_OFILE  *OFile,
  IN UINT64     Position
  );

UINTN
FatSizeToClusters (
  IN UINT64  Size,
  IN UINTN   ClusterSize
  );

VOID
FatEfiTimeToFatTime (
  IN EFI_TIME        *ETime,
  OUT FAT_DATE_TIME  *FTime
  );

VOID
FatFatTimeToEfiTime (
  IN FAT_DATE_TIME  *FTime,
  OUT EFI_TIME      *ETime
  );

BOOLEAN
FatIsValidTime (
  IN EFI_TIME  *Time
  );

VOID
FatFatToStr (
  IN UINTN                            FatSize,
  IN CHAR8                            *Fat,
  OUT CHAR16                          *String
  );

BOOLEAN
FatStrToFat (
  IN CHAR16                           *String,
  IN UINTN                            FatSize,
  OUT CHAR8                           *Fat
  );

UINTN
FatStrToFatLen (
  IN CHAR16                           *String
  );
 
BOOLEAN FatLfnIsValid (
  CHAR16  *Name
  );

FAT_OFILE *
FatAllocateOFile (
  IN FAT_VOLUME           *Vol
  );
  
VOID
FatFreeOFile (
  IN FAT_OFILE  *OFile
  );

EFI_STATUS
FatSetVolumeDirty (
  IN FAT_VOLUME       *Vol,
  IN BOOLEAN          Dirty
  );

EFI_STATUS
FatDiskIo (
  IN FAT_VOLUME       *Vol,
  IN DISK_IO_MODE     IoMode,
  IN UINT64           Offset,
  IN UINTN            BufferSize,
  IN OUT VOID         *Buffer
  );

VOID
FatAcquireLock ();

EFI_STATUS
FatCleanupVolume (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *OFile OPTIONAL,
  IN EFI_STATUS           Status OPTIONAL
  );

VOID
FatReleaseLock ();

BOOLEAN
FatIsLocked ();

UINT8
FatDirEntryChecksum (
  FAT_DIRECTORY_ENTRY *Dir
  );

UINTN
FatNameToDirEntryCount (
  IN CHAR16           *Name,
  OUT UINT8           *CaseFlag
  );

CHAR16 *
FatGetNameComp (
  IN CHAR16    *Path,
  OUT CHAR16   *Name
  );

BOOLEAN
FatMatchFileName (
  IN FAT_OFILE        *Entry,
  IN CHAR16           *Name
  );

BOOLEAN
FatIsDotEntry (
  IN FAT_OFILE        *OFile
  );

EFI_STATUS
FatGenerate8Dot3Name (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *Parent,
  IN FAT_OFILE            *OFile
  );

//
// dirsup.c
//

VOID
FatPackDirEntry (
  IN FAT_OFILE        *OFile
  );

VOID
FatNameToStr (
  IN CHAR8            *Fatname,
  IN UINTN            Len,
  IN UINTN            LowerCase,
  IN CHAR16           *Str
  );

EFI_STATUS
FatUnpackDirEntry (
  IN FAT_OFILE        *OFile
  );

EFI_STATUS
FatNewDirEntry (
  IN FAT_OFILE            *OFile,
  IN UINTN                EntryCount,
  OUT FAT_OFILE           **NewOFile
  );

//
// dirent.c
//

EFI_STATUS
FatGetVolumeEntry (
  IN FAT_VOLUME           *Vol,
  IN CHAR16               *Name
  );

EFI_STATUS
FatSetVolumeEntry (
  IN FAT_VOLUME           *Vol,
  IN CHAR16               *Name
  );

EFI_STATUS
FatOpenEntry (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *Parent,
  IN UINT64               *Position,
  OUT FAT_OFILE           **pOFile
  );

EFI_STATUS
FatGetDirOFile (
  IN FAT_OFILE            *OFile,
  IN OUT UINT64           *Position,
  OUT FAT_OFILE           **pOFile
  );

EFI_STATUS
FatOFileWriteDir (
  IN FAT_OFILE            *OFile
  );

//
//
//

EFI_STATUS
FatAllocateIFile(
  IN FAT_OFILE            *OFile,
  OUT FAT_IFILE           **pIFile
  );

EFI_STATUS
FatIFileClose (
  FAT_IFILE           *IFile
  );

EFI_STATUS
FatGetFileInfo (
  IN FAT_VOLUME       *Vol,
  IN FAT_OFILE        *OFile,
  IN OUT UINTN        *BufferSize,
  OUT VOID            *Buffer    
  );

//
// fspace.c
//

EFI_STATUS
FatShrinkEof (
  IN FAT_OFILE            *OFile
  );

EFI_STATUS
FatGrowEof (
  IN FAT_OFILE            *OFile,
  IN UINT64               NewSize
  );

UINT64
FatDirSize (
  IN FAT_OFILE            *OFile
  );

UINTN
FatGetFatEntry(
  IN FAT_VOLUME           *Fat,
  IN UINTN                Index
  );

EFI_STATUS
FatSetFatEntry(
  IN FAT_VOLUME           *Fat,
  IN UINTN                Index,
  IN UINTN                Value
  );

UINTN
FatAllocateCluster(
  IN FAT_VOLUME   *Vol
  );

VOID
FatComputeFreeInfo (
  IN FAT_VOLUME *Vol
  );

EFI_STATUS
FatCheckVolumeRef (
  IN FAT_VOLUME   *Vol
  );

BOOLEAN
FatCheckOFileRef (
  IN FAT_OFILE   *OFile
  );

//
//
//

VOID
FatSetVolumeError (
  IN FAT_OFILE            *OFile,
  IN EFI_STATUS           Status
  );

EFI_STATUS
FatAllocateVolume (
  IN  EFI_HANDLE                Handle,
  IN  EFI_DISK_IO_PROTOCOL      *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL     *BlockIo
  );

EFI_STATUS
FatAbandonVolume (
  IN FAT_VOLUME       *Vol
  );

VOID
FatStrLwr (
  IN CHAR16   *Str
  );

VOID
FatStrUpr (
  IN CHAR16   *Str
  );

INTN
FatStriCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2
  );
 
//
// FAT volume read/write functions with cache mechanism.
//

EFI_STATUS
FatDiskIoReadVolume (
  IN FAT_VOLUME         *Vol,
  IN UINT64             Offset,
  IN UINTN              BufferSize,
  OUT VOID              *Buffer
  );

EFI_STATUS
FatDiskIoWriteVolume (
  IN FAT_VOLUME         *Vol,
  IN UINT64             Offset,
  IN UINTN              BufferSize,
  IN VOID               *Buffer
  );

EFI_STATUS
FatVolumeFlushCache (
  IN FAT_VOLUME         *Vol
  );

HASH_ENTRY*
FatInsertHashNode (
  FAT_OFILE     *OFile,
  CHAR8         *String,
  UINTN         StringSize,
  UINT32        Position
  );

VOID
FatRemoveHashNode (
  HASH_ENTRY    *Entry,
  UINT32        Position
  );
  
FAT_OFILE*
FatHashSearch (
  FAT_OFILE     *OFile,
  CHAR16        *Name, 
  UINT32        PreferedPosition
  );

EFI_STATUS
FatOFileWrite (
  IN FAT_OFILE             *OFile,
  IN UINT64                Position,
  IN OUT UINTN             *DataBufferSize,
  IN VOID                  *UserBuffer
  );
#endif //_FAT_H_
