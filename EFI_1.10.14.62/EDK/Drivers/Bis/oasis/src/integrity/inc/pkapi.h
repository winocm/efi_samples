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
  
  pkapi.h

Abstract:


Revision History

--*/
#ifndef _PKAPI_H_
#define _PKAPI_H_

//#pragma pack(1)

#include "pkdef.h"
#ifdef WIN32
#include "windows.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

PK_ARCHIVE_PTR
pk_LoadArchive(
        ISL_MEMORY_CONTEXT_PTR  pArchive,
        ISL_CONST_DATA                  Filename );

ISL_STATUS
pk_RecycleArchive(
        PK_ARCHIVE_PTR  pArchive );

ISL_STATUS
pk_FindFile(
        PK_ARCHIVE_PTR          pPKArchive,
        ISL_CONST_DATA          Filename,
        ISL_CONST_DATA_PTR      Filedata );

PK_ITERATOR_PTR
pk_CreateFileEnumerator(
        PK_ARCHIVE_PTR          pPKArchive);

ISL_STATUS
pk_GetNextFile(
        PK_ITERATOR_PTR         pIterator,
        ISL_CONST_DATA_PTR      Filename,
        ISL_CONST_DATA_PTR      Filedata );

ISL_STATUS
pk_RecycleFileEnumerator(
        PK_ITERATOR_PTR         pIterator );

PK_ARCHIVE_PTR
pk_InitializeFromMemory(
        ISL_MEMORY_CONTEXT_PTR  pMem,
        ISL_CONST_DATA                  Fileimage );

#ifndef EISL
/* Here are the archive creation functions */

PK_NEW_ARCHIVE_PTR
pk_CreateNewArchive(
        ISL_MEMORY_CONTEXT_PTR pArchive );

/* NOTE: Filenames must be relative
   filenames to the current directory when adding
   files to an archive */

ISL_STATUS
pk_AddFileToNewArchive(
        PK_NEW_ARCHIVE_PTR      pNewArchive,
        ISL_CONST_DATA          Filename,
        ISL_CONST_DATA          Filedata );     /* If Filedata is NULL (length 0), assume file
                                                                           is on disk and fetch it. */

/* NOTE: The Filename to write out the archive
   does not have to be a relative pathname */

ISL_STATUS
pk_WriteNewArchive(
        PK_NEW_ARCHIVE_PTR      pNewArchive,
        ISL_CONST_DATA          FilenameToWrite );

ISL_STATUS
pk_RecycleNewArchive(
        PK_NEW_ARCHIVE_PTR      pNewArchive );

#endif
#ifdef __cplusplus
}
#endif
#endif
