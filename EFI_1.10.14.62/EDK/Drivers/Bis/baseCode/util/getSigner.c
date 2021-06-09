/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

*/

//************************************************************************************************//
// getSigner.C
//
// Description: 
//  Contains GetSignerOfManifest( ) function.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bis_priv.h>



//------------------------------------------------
//  GetSignerOfManifest
//
//  - gets the first signature from the signed manifest
//  refered to by the BIS_SM_PTR. 
//
//  
//  Returns:
//  BIS_OK and Stores a  CSSM_VL_SIGNATURE_INFO_PTR in BIS_SM_PTR->sigInfo. 
//  and on success.
//
//  Returns: 
//  CSSM_ERROR_REPORTED in the case of an error. The exact CSSM error code
//  will be stored in cssmInfo->lastErr.
// 
//     

BIS_STATUS
GetSignerOfManifest(
     APP_CSSMINFO_PTR           cssmInfo
    ,BIS_SM_PTR                 smInfo  //SM to obtain signature from.
     )
{
    BIS_STATUS   brc= BIS_OK;
    CSSM_RETURN  rc;
    CSSM_HANDLE  iterHandle;

    cssmInfo->lastErr= 0;
    smInfo->sigInfo=
    CSSM_VL_GetFirstSignatureInfo( cssmInfo->hVL
        , smInfo->hVerifiableObj, &iterHandle );
    if (smInfo->sigInfo == NULL)
    {
            brc= saveCssmErr( cssmInfo );
    }
    else
    {

        rc= CSSM_VL_AbortScan(cssmInfo->hVL, iterHandle);
        if (rc==CSSM_FAIL)
        {
            brc= saveCssmErr( cssmInfo );
        }
         
    }
    
    return brc;      
}


//eof
