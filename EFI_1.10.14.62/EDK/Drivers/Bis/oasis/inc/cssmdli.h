/*-----------------------------------------------------------------------
 *      File:   CSSMDLI.H
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------
 */
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software is subject to the U.S. Export Administration Regulations 
 * and other U.S. law, and may not be exported or re-exported to certain 
 * countries (currently Afghanistan (Taliban-controlled areas), Cuba, Iran, 
 * Iraq, Libya, North Korea, Serbia (except Kosovo), Sudan and Syria) or to 
 * persons or entities prohibited from receiving U.S. exports (including Denied 
 * Parties, Specially Designated Nationals, and entities on the Bureau of 
 * Export Administration Entity List or involved with missile technology or 
 * nuclear, chemical or biological weapons).
 */ 

#ifndef _CSSMDLI_H
#define _CSSMDLI_H

#include "cssmtype.h"
//#include "cssmspi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cssm_spi_dl_funcs {
        CSSM_DB_HANDLE (CSSMAPI *DbOpen) (
                       CSSM_DL_HANDLE DLHandle,
                       const char *DbName,
                                           const CSSM_NET_ADDRESS_PTR DbLocation,
                       const CSSM_DB_ACCESS_TYPE_PTR AccessRequest,
                       const CSSM_USER_AUTHENTICATION_PTR UserAuthentication,
                       const void *OpenParameters);


    CSSM_RETURN (CSSMAPI *DbClose) (CSSM_DL_DB_HANDLE DLDBHandle);

    CSSM_DB_HANDLE (CSSMAPI *DbCreate) (
                      CSSM_DL_HANDLE DLHandle,
                      const char *DbName,
                                          const CSSM_NET_ADDRESS_PTR DbLocation,
                      const CSSM_DBINFO_PTR DBInfo,
                      const CSSM_DB_ACCESS_TYPE_PTR AccessRequest,
                      const CSSM_USER_AUTHENTICATION_PTR UserAuthentication,
                      const void *OpenParameters);

    CSSM_RETURN (CSSMAPI *DbDelete) (
                     CSSM_DL_HANDLE DLHandle,
                     const char *DbName,
                                         const CSSM_NET_ADDRESS_PTR DbLocation,
                     const CSSM_USER_AUTHENTICATION_PTR UserAuthentication);

         CSSM_RETURN (CSSMAPI *Authenticate) (
                      CSSM_DL_DB_HANDLE DLDBHandle,
                      const CSSM_DB_ACCESS_TYPE_PTR AccessRequest,
                      const CSSM_USER_AUTHENTICATION_PTR UserAuthentication);


    CSSM_DB_UNIQUE_RECORD_PTR (CSSMAPI *DataInsert) (
                           CSSM_DL_DB_HANDLE DLDBHandle,
                           const CSSM_DB_RECORDTYPE RecordType,
                           const CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes,
                           const CSSM_DATA_PTR Data);

    CSSM_RETURN (CSSMAPI *DataDelete) (
                       CSSM_DL_DB_HANDLE DLDBHandle,
                       const CSSM_DB_UNIQUE_RECORD_PTR UniqueRecordIdentifier);

        CSSM_RETURN (CSSMAPI *DataModify) (
                                                        CSSM_DL_DB_HANDLE DLDBHandle,
                                                        const CSSM_DB_RECORDTYPE RecordType,
                                                        CSSM_DB_UNIQUE_RECORD_PTR  UniqueRecordIdentifier,
                                                        CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR  AttributesToBeModified,
                                                        CSSM_DATA_PTR DataToBeModified);


    CSSM_DB_UNIQUE_RECORD_PTR (CSSMAPI *DataGetFirst) (
                       CSSM_DL_DB_HANDLE DLDBHandle,
                       const CSSM_QUERY_PTR Query,
                       CSSM_HANDLE_PTR  ResultsHandle,
                       CSSM_BOOL  *EndOfDataStore,
                       CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes,
                       CSSM_DATA_PTR  Data);

    CSSM_DB_UNIQUE_RECORD_PTR (CSSMAPI *DataGetNext) (
                                  CSSM_DL_DB_HANDLE DLDBHandle,
                                  CSSM_HANDLE ResultsHandle,
                                  CSSM_BOOL *EndOfDataStore,
                                  CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes,
                                  CSSM_DATA_PTR Data);

    CSSM_RETURN (CSSMAPI *DataAbortQuery) (CSSM_DL_DB_HANDLE DLDBHandle,
                                      CSSM_HANDLE ResultsHandle);

        CSSM_RETURN (CSSMAPI *DataGetFromUniqueRecordId) (
                                                                        CSSM_DL_DB_HANDLE DLDBHandle,
                                                                        const CSSM_DB_UNIQUE_RECORD_PTR UniqueRecord,
                                                                        CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes,
                                                                        CSSM_DATA_PTR  Data);

    CSSM_RETURN (CSSMAPI *FreeUniqueRecord) (
                                       CSSM_DL_DB_HANDLE DLDBHandle,
                                       CSSM_DB_UNIQUE_RECORD_PTR UniqueRecord);

    void * (CSSMAPI *PassThrough) (CSSM_DL_DB_HANDLE DLHandle,
                                   uint32 PassThroughId,
                                   const void * InputParams);
}CSSM_SPI_DL_FUNCS, *CSSM_SPI_DL_FUNCS_PTR;

#ifdef __cplusplus
}
#endif

#endif
