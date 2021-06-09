///////////////////////////////////////////////////////////////////////////////
//
// This software is provided "as is" with no warranties, express or
// implied, including but not limited to any implied warranty of
// merchantability, fitness for a particular purpose, or freedom from
// infringement.
//
// Intel Corporation may have patents or pending patent applications,
// trademarks, copyrights, or other intellectual property rights that
// relate to this software.  The furnishing of this document does not
// provide any license, express or implied, by estoppel or otherwise,
// to any such patents, trademarks, copyrights, or other intellectual
// property rights.
//
// This software is furnished under license and may only be used or
// copied in accordance with the terms of the license. Except as
// permitted by such license, no part of this software may be reproduced,
// stored in a retrieval system, or transmitted in any form or by any
// means without the express written consent of Intel Corporation.
//
// Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
// This software and associated documentation (if any) is furnished
// under a license and may only be used or copied in accordance
// with the terms of the license. Except as permitted by such
// license, no part of this software or documentation may be
// reproduced, stored in a retrieval system, or transmitted in any
// form or by any means without the express written consent of
// Intel Corporation.
//
//

#ifndef _PSD_H_
#define _PSD_H_

//************************************************************************************************//
// PSD.H
//
// Description:
//  Describe structure and functions specific to BIS' use of CSSM.
//
//************************************************************************************************//



#undef  WIN32          //We do OASIS compiles in WIN32.
#define OASIS          //This def causes CSSMAPI to be NULL allowing us
                       // to control the calling convention.
#define CSSM_BIS
#include <cssm.h>
#include <intel/guids.h>
#define WIN32

#define TBD(s) message("TBD*****" s)

        //
        // APP_CSSMINFO - contains an applications CSSM handles
        //      and memory allocate for CSSM to manage.
        //
typedef struct _BIS_APPINFO *BIS_APPINFO_PTR;

typedef struct _APP_CSSMINFO
{
    CSSM_MODULE_HANDLE hVL;           //handle to verification module.
    CSSM_MODULE_HANDLE hCSP;          //handle to crypto service module.
    UINT32             lastErr;       //bis stores CSSM errors here.
    BIS_APPINFO_PTR    appInfo;       //parent app
}
APP_CSSMINFO, *APP_CSSMINFO_PTR;




     //
     // BIS_SM - contains an info about a signed manifest
     //      once it has been processed by PrepareSignedManifest.
     //

typedef struct _BIS_SM
{
    CSSM_VOBUNDLE_UID_PTR       bundleUid;
    CSSM_VO_HANDLE              hVerifiableObj;
    CSSM_VL_SIGNATURE_INFO_PTR  sigInfo;
    CSSM_VL_DO_INFO_PTR         doInfoPtr;
}
BIS_SM, *BIS_SM_PTR;



typedef struct _BisBaseCodeInstanceData BISBC_INSTANCEDATA;

BIS_STATUS
PSD_InitCSSM( BISBC_INSTANCEDATA *bisGlobal );

EFI_STATUS
PSD_InitializeApp(
    BIS_APPINFO_PTR  appInfo    //The calling app's instance data.
    ,APP_CSSMINFO_PTR cssmInfo   //Struct passed by caller to be filled in.
    );

BIS_STATUS
PSD_Shutdown( APP_CSSMINFO_PTR cssmInfo ); //The calling app's cssm data.


        //** MAIN PSD WRAPPER FUNCTIONS ***
BIS_STATUS
Core_UpdtBOA(
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_UBOA_PARMS   *parmBlock );

BIS_STATUS
Core_VerifyBootObject(
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_VBO_PARMS   *parmBlock );

BIS_STATUS
Core_VerifyObjectWithCredential(
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_VOWC_PARMS   *parmBlock );


UINT32
core_GetBootObjectAuthorizationUpdateToken(
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_GBOAUT_PARMS *parmBlock
    ,UINT32            updateCount
    ,CSSM_GUID_PTR     platformGuid);

        //** PSD SERVICE WRAPPER FUNCTIONS ***

BIS_STATUS
    saveCssmErr( APP_CSSMINFO_PTR cssmInfo );


BIS_STATUS
PrepareSignedManifestHandle(
     APP_CSSMINFO_PTR cssmInfo     //Struct filled in by PSD_Initialize.
    ,BIS_DATA_PTR     rawSM        //SM begin prepared
    ,BIS_SM_PTR       prepedSMInfo //Struct passed by caller to be filled in.
    ,CSSM_STRING      signerInformationName //signer info section name.
    );

BIS_STATUS
FreeSignedManifestHandle(
     APP_CSSMINFO_PTR cssmInfo      //Struct filled in by PSD_Initialize.
    ,BIS_SM_PTR       prepedSMInfo     //Struct
    );


BIS_STATUS
GetSignerOfManifest(
     APP_CSSMINFO_PTR           cssmInfo
    ,BIS_SM_PTR                 smInfo  //SM to obtain signature from.
     );

BIS_STATUS
GetDataObjectByName(
     APP_CSSMINFO_PTR           cssmInfo
    ,BIS_SM_PTR                 smInfo       //SM to obtain signature from.
    ,CSSM_STRING                objName
    ,CSSM_DATA_PTR              requestedObj //[OUT]
    ,BIS_BOOLEAN                decodeFromBase64
    );


BIS_STATUS
CheckParmsetValue( APP_CSSMINFO_PTR cssmInfo, BIS_SM_PTR smInfo);


BIS_BOOLEAN
compareCssmGuids(  CSSM_GUID_PTR a,  CSSM_GUID_PTR b );

         // ** Values used for 'updateField' parm when calling UpdateBoaInNVM().
#define  UPDATE_CERT  (1)    //Boot Object Authorization Cert
#define  UPDATE_FLAG  (2)    //Boot Object Check Flag
#define  UPDATE_COUNTER (3)  //Platform's Update Counter.

EFI_STATUS
UpdateBoaInNVM(BIS_APPINFO_PTR appInfo, UINT32 updateField
    , CSSM_DATA_PTR fieldValue);


UINT32 *
GetCssmErr( BIS_APPLICATION_HANDLE  AppHandle );


//Memory function wrappers used by OASIS/CSSM/PSD
void *malloc_cssm(uint32 Size, void *AllocRef);
void  free_cssm(void *MemPtr, void *AllocRef);
void *realloc_cssm(void *MemPtr, uint32 Size, void *AllocRef);
void *calloc_cssm(uint32 Num, uint32 Size, void *AllocRef);


//ISL types.
typedef struct isl_data {
	uint32 Length;
	uint8  *Data;
} ISL_DATA, *ISL_DATA_PTR;

typedef enum isl_status {
    ISL_OK = 0,
    ISL_FAIL = -1
} ISL_STATUS;

ISL_STATUS IslUtil_Base64Decode(
	ISL_DATA_PTR pEncodedData,
	ISL_DATA_PTR pDecodedData);

uint32 IslUtil_Base64DecodeSize(ISL_DATA_PTR pEncodedData);

#endif

//History. This file cloned from smbios bis file:
//Archive: /SMA/Src/inc/psd.h
//Revision: 25
//Date: 10/07/98 12:47p
//eof

