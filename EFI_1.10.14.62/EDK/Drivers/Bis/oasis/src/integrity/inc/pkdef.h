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
  
  pkdef.h

Abstract:


Revision History

--*/
#ifndef _PKDEF_H_
#define _PKDEF_H_

#define LOCAL_FILE_HEADER_SIGNATURE 0x04034b50
#define CENTRAL_FILE_HEADER_SIGNATURE 0x02014b50
#define END_OF_CENTRAL_DIR_SIGNATURE 0x06054b50

#include "isltype.h"
//#include "isl_internal.h"

#pragma pack(1)


/* Basic Types */
#ifdef EFI64
typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef          __int16 sint16;
typedef unsigned __int32 uint32;
typedef          __int32 sint32;
#else
/* Basic Types */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned int uint32;
typedef int sint32;
#endif

typedef struct pk_local_file_header {
    uint16      VersionNeededToExtract;
    uint16      GeneralPurposeBitFlag;
    uint16      CompressionMethod;
    uint16      LastModFileTime;
    uint16      LastModFileDate;
    uint32      crc32;
    uint32      CompressedSize;
    uint32      UncompressedSize;
    uint16      FilenameLength;
    uint16      ExtraFieldLength;
} PK_LOCAL_FILE_HEADER, *PK_LOCAL_FILE_HEADER_PTR;

typedef struct ISL_DATA_descriptor {
        uint32  crc32;
        uint32  CompressedSize;
        uint32  UncompressedSize;
} ISL_DATA_DESCRIPTOR, *ISL_DATA_DESCRIPTOR_PTR;

typedef struct pk_central_file_header {
    uint16      VersionMadeBy;
        uint16  VersionNeededToExtract;
    uint16      GeneralPurposeBitFlag;
    uint16      CompressionMethod;
    uint16      LastModFileTime;
    uint16      LastModFileDate;
    uint32      crc32;
    uint32      CompressedSize;
    uint32      UncompressedSize;
    uint16      FilenameLength;
    uint16      ExtraFieldLength;
    uint16      FileCommentLength;
    uint16      DiskNumberStart;
    uint16      InternalFileAttributes;
    uint32      ExternalFileAttributes;
    uint32      RelativeOffsetOfLocalHeader;
} PK_CENTRAL_FILE_HEADER, *PK_CENTRAL_FILE_HEADER_PTR;

typedef struct pk_central_dir_end_header {
    uint16      ThisDiskNumber;
        uint16  StartDiskNumber;
        uint16  TotalCentralDirEntriesOnDisk;
        uint16  TotalCentralDirEntries;
        uint32  SizeOfCentralDir;
        uint32  CentralDirOffsetFromStartingDisk;
        uint16  ZipfileCommentLength;
} PK_CENTRAL_DIR_END_HEADER, *PK_CENTRAL_DIR_END_HEADER_PTR;

typedef struct pk_central_dir_end {
        PK_CENTRAL_DIR_END_HEADER_PTR   Header;
        ISL_CONST_DATA                                  ZipFileComment;
} PK_CENTRAL_DIR_END, *PK_CENTRAL_DIR_END_PTR;

typedef struct pk_local_file {
        PK_LOCAL_FILE_HEADER_PTR        Header;
        ISL_CONST_DATA                          Filename;
        ISL_CONST_DATA                          ExtraField;
        ISL_CONST_DATA                          Filedata;
        ISL_DATA_DESCRIPTOR_PTR         DataDescriptor;
        struct pk_local_file*           Next;
} PK_LOCAL_FILE, *PK_LOCAL_FILE_PTR;

typedef struct pk_central_file {
        PK_CENTRAL_FILE_HEADER_PTR      Header;
        ISL_CONST_DATA                          Filename;
        ISL_CONST_DATA                          ExtraField;
        ISL_CONST_DATA                          FileComment;
        struct pk_central_file*         Next;
} PK_CENTRAL_FILE, *PK_CENTRAL_FILE_PTR;

typedef struct pk_archive {
        ISL_MEMORY_CONTEXT_PTR          pMem;           /* For memory context in EISL */
        ISL_CONST_DATA                          Archive;
        PK_LOCAL_FILE_PTR                       LocalFiles;
        PK_CENTRAL_FILE_PTR                     CentralFiles;
        PK_CENTRAL_DIR_END_PTR          DirEnd;
} PK_ARCHIVE, *PK_ARCHIVE_PTR;

typedef struct pk_iterator {
        PK_ARCHIVE_PTR                          PKArchivePtr;
        PK_LOCAL_FILE_PTR                       LocalFilePtr;
} PK_ITERATOR, *PK_ITERATOR_PTR;

typedef struct pk_new_file {
        ISL_DATA                                        Filename;
        ISL_DATA                                        FileImage;
        uint16                                  Date;
        uint16                                  Time;
        uint32                                  crc;
    uint32                                      RelativeOffsetOfLocalHeader;
        struct pk_new_file*             Next;
} PK_NEW_FILE, *PK_NEW_FILE_PTR;

typedef struct pk_new_archive {
        ISL_ARCHIVE_CONTEXT_PTR         pMem;
        PK_NEW_FILE_PTR                         Files;
} PK_NEW_ARCHIVE, *PK_NEW_ARCHIVE_PTR;

#pragma pack()

#endif
