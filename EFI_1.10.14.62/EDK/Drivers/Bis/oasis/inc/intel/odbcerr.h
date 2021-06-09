/*-----------------------------------------------------------------------
 *      File:   dlERR.H
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

#ifndef _ODBCERR_H
#define _ODBCERR_H

/* #define CSSM_DL_NO_DATASOURCES				   CSSM_DL_PRIVATE_ERROR + 1 moved to cssmerr.h 9/19/97 MEP */
/* #define CSSM_DL_DATASOURCE_CREATE_FAILED	       CSSM_DL_PRIVATE_ERROR + 2 redundant with CSSM_DL_DB_CREATE_FAIL 9/19/97 MEP */
/* #define CSSM_DL_INVALID_DATASTORE_NAME		   CSSM_DL_PRIVATE_ERROR + 3 moved to cssmerr.h 9/19/97 MEP */
/* #define CSSM_DL_DB_CREATEDATASTORE_FAILED	   CSSM_DL_PRIVATE_ERROR + 4 redundant with CSSM_DL_DB_CREATE_FAIL 9/19/97 MEP */
/* #define CSSM_DL_NOT_CERTIFICATE_DATABASE	    CSSM_DL_PRIVATE_ERROR + 5 removed cert specific 2/17/98 GBM */
/* #define CSSM_DL_DB_OPEN_CERTIFICATES_TABLE_FAIL	CSSM_DL_PRIVATE_ERROR + 6 removed cert specific 2/17/98 GBM */
/* #define CSSM_DL_INVALID_EXTENSION_PTR		   CSSM_DL_PRIVATE_ERROR + 7 removed 9/19/97 MEP */
/* #define CSSM_DL_DB_OPEN_EXTENSIONS_TABLE_FAIL   CSSM_DL_PRIVATE_ERROR + 8 removed 9/19/97 MEP */
/* #define CSSM_DL_DB_OPEN_SIGNATURES_TABLE_FAIL   CSSM_DL_PRIVATE_ERROR + 9 removed 9/19/97 MEP */
/* #define CSSM_DL_NO_EXTENSION				       CSSM_DL_PRIVATE_ERROR + 10 removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_SELECTION_PREDICATE_PTR	CSSM_DL_PRIVATE_ERROR + 12 redundant with CSSM_DL_INVALID_SELECTION_PRED_POINTER 12/31/97 MEP */
/* #define CSSM_DL_DB_BEGIN_OF_RECORDS             CSSM_DL_PRIVATE_ERROR + 14 removed 9/19/97 MEP */
/* #define CSSM_DL_DB_GET_FAIL						CSSM_DL_PRIVATE_ERROR + 15 redundant with CSSM_DL_CRL_GET_NEXT_FAIL 9/19/97 MEP */
/* #define CSSM_DL_INVALID_RESULT_HANDLE           CSSM_DL_PRIVATE_ERROR + 16 moved to cssmerr.h 9/19/97 MEP */
/* #define CSSM_DL_BAD_PASSTHRU_INPUT              CSSM_DL_PRIVATE_ERROR + 17 removed 9/19/97 MEP */
/* #define CSSM_DL_PASSTHROUGHID_NOT_SUPPORTED     CSSM_DL_PRIVATE_ERROR + 18 removed 9/19/97 MEP */
/* #define CSSM_DL_DB_CONFIGDATASTORE_FAILED       CSSM_DL_PRIVATE_ERROR + 19 removed 9/19/97 MEP */
/* #define CSSM_DL_DB_REMOVEDATASTORE_FAILED       CSSM_DL_PRIVATE_ERROR + 20 redundant with CSSM_DL_DB_DELETE_FAIL 9/19/97 MEP */
/* #define CSSM_DL_DB_OPEN_CRL_TABLE_FAIL			CSSM_DL_PRIVATE_ERROR + 21  removed recordtype specific 2/17/98 GBM */
/* #define CSSM_DL_DB_GETFIRSTCRL_EXCEPTION		CSSM_DL_PRIVATE_ERROR + 22 redundant with CSSM_DL_CRL_GET_FIRST_FAIL 9/19/97 MEP */
#define CSSM_DL_DB_CREATE_MUTEX_FAIL			   CSSM_DL_PRIVATE_ERROR + 25
#define CSSM_DL_COULD_NOT_LOCK_MUTEX			   CSSM_DL_PRIVATE_ERROR + 23
#define CSSM_DL_LOCK_MUTEX_FAIL                 CSSM_DL_PRIVATE_ERROR + 23
#define CSSM_DL_DB_CLOSE_MUTEX_FAIL             CSSM_DL_PRIVATE_ERROR + 27
#define CSSM_DL_DB_COULD_NOT_INIT_ODBC			   CSSM_DL_PRIVATE_ERROR + 24
#define CSSM_DL_DB_ODBC_INIT_FAIL               CSSM_DL_PRIVATE_ERROR + 24
#define CSSM_DL_DB_ODBC_FREE_FAIL               CSSM_DL_PRIVATE_ERROR + 26
/* #define CSSM_DL_DB_ISSUER_SUBJECT_NOT_FOUND     CSSM_DL_PRIVATE_ERROR + 28 removed 9/19/97 MEP */
/* #define CSSM_DL_DB_OPEN_DBFIELDS_TABLE_FAIL		CSSM_DL_PRIVATE_ERROR + 29 outdated 2/17/98 GBM */
/* #define CSSM_DL_DB_INDEX_ERROR						CSSM_DL_PRIVATE_ERROR + 30 remove 2/17/98 GBM */
#define CSSM_DL_DB_CREATE_INDEX_FAIL				CSSM_DL_PRIVATE_ERROR + 31
/* #define CSSM_DL_FUNCTION_NOT_SUPPORTED          CSSM_DL_PRIVATE_ERROR + 32 removed 9/19/97 MEP */
/* #define CSSM_DL_CERT_INDEX_VALUE_NOT_FOUND		CSSM_DL_PRIVATE_ERROR + 34 removed recordtype specific 2/17/98 GBM */
/* #define CSSM_DL_UNSUPPORTED_INDEX_NUM			CSSM_DL_PRIVATE_ERROR + 35 redundant with CSSM_DL_INVALID_NUMBER_OF_INDEXES 10/15/97 MEP */
/* #define CSSM_DL_INVALID_DBINFO					CSSM_DL_PRIVATE_ERROR + 33 redundant with CSSM_DL_INVALID_DB_INFO 12/2/97 MEP */
#define CSSM_DL_INVALID_OID_PTR                 CSSM_DL_PRIVATE_ERROR + 36

/* Release 1.2 */
/* #define CSSM_DL_INVALID_NUMBER_OF_RECORD_TYPES	CSSM_DL_PRIVATE_ERROR + 37 outdated 2/17/98 GBM */
/* #define CSSM_DL_INVALID_NUMBER_OF_INDEXES			CSSM_DL_PRIVATE_ERROR + 38 outdated 2/17/98 GBM */

/* Release 2.0 */
#define CSSM_DL_CANNOT_DELETE_ODBC_FILE         CSSM_DL_PRIVATE_ERROR + 39
#define CSSM_DL_ODBC_ENTRY_ALREADY_DELETED      CSSM_DL_PRIVATE_ERROR + 40
#define CSSM_DL_INTERNAL_POINTER_INVALID        CSSM_DL_PRIVATE_ERROR + 41
#define CSSM_DL_INVALID_ODBC_STATE              CSSM_DL_PRIVATE_ERROR + 42
#define CSSM_DL_INDEXES_DUPLICATED_IN_MULTIPLE_RECORDS CSSM_DL_PRIVATE_ERROR + 43
#endif /* _ODBCERR_H */
