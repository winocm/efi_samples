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

//************************************************************************************************//
// NVM.H
//
// Description: High-Level Non-volatile Memory Functions.
//
//      The functions described in this .h file are used by BIS
//      to manage it's platform dependent persistant storage.
//
//      Functions are implemented in terms of the NVI.H pointers
//      passed to BIS_main.
//
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#ifndef _NVM_H_
#define _NVM_H_

//This constant will be compiled in only one module internal to BIS.
#ifdef BIS_GEN_SCS_CONST
char scsConst_nvm_h[]=

#endif
        //NVM_AREA_ID is used to uniquely identify
        // a storage area in NonVolatileMemory.
        //It's format is compatible with GUIDs.

typedef struct _NVM_AREA_ID
{
    UINT32 f1;
    UINT16 f2, f3;
    UINT8  f4[8];
}
NVM_AREA_ID;

typedef void*   NVM_AREA_HANDLE;

         //-------------------//
         // BIS' NVM_AREA ID  //
#ifndef LINUX_BIS
#define  NVM_BIS_AREA_ID    \
{ 0xe3f2ed60, 0xf688, 0x11d1, { 0x8d, 0xeb, 0x0, 0xa0, 0xc9, 0x85, 0x85, 0x80 } }
#endif

        //----------------------//
        // BIS NVM AREA LAYOUT  //

typedef struct _BIS_NVM
{
    UINT32
     truncatedDigest;               //1st 4 bytes of a SHA1 digest of
                                    //the entire NVM area.
    UINT32
     bootObjectAuthorizationCheckFlag;

    UINT32
     bootObjectAuthorizationUpdateCount;

    UINT32                  //Actual byte length of following array.
     byteLengthOfBOACert;

    UINT8                   //Place holder. Previous field contains actual length.
     bootObjectAuthorizationCertificate[4];

}
BIS_NVM, *BIS_NVM_PTR;


#define GET_NVM_FIELD_OFFSET( anNVMFieldName )           \
        ( ( (UINTN)&( (BIS_NVM_PTR)0)->anNVMFieldName) )

#define GET_NVM_FIELD_SIZE(  anNVMFieldName )            \
        ( sizeof( ( (BIS_NVM_PTR)0)->anNVMFieldName) )


typedef struct _BIS_APPINFO BIS_APPINFO;
typedef struct _BIS_APPINFO *BIS_APPINFO_PTR;

BIS_STATUS NVM_Init(  );


NVM_AREA_HANDLE
    NVM_Open( BIS_APPINFO *appInfo
    , NVM_AREA_ID *memAreaToOpen
    , UINT32 *areaLength);



EFI_STATUS
    NVM_Read( NVM_AREA_HANDLE areaHandle
    , UINT8 *data
    , UINT32 dataLength
    , UINT32 offset );

EFI_STATUS
    NVM_Write( NVM_AREA_HANDLE areaHandle
    , UINT8 *data
    , UINT32 dataLength
    , UINT32 offset );

EFI_STATUS
    NVM_Close( BIS_APPINFO *appInfo, NVM_AREA_HANDLE areaHandle );

//private methods:
BIS_BOOLEAN checkForBisGUID( NVM_AREA_ID *callerGUID );
EFI_STATUS  NVM_CheckParms(  NVM_AREA_HANDLE areaHandle
                , UINT8 *data , UINT32 dataLength , UINT32 offset );
EFI_STATUS digestNVM( BIS_APPINFO_PTR  appInfo, UINT32 *truncDigestOut);


#endif

//History:
//branched from file...
// Archive: /SMA/Src/bisEFI/EfiSample/corefw/drivers/bis/inc/nvm.h
// Revision: 1
// Date: 7/20/00 2:32p
//eof

