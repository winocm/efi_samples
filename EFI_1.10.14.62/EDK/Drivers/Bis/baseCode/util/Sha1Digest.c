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
// sha1Digest
//
// Description: create a sha1 digest of one of more CSSM_DATA items.
//      The output in provided
// 
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//



#include <BIS_PRIV.H>




//--------------------------------------------------------------------------------//
//  sha1Digest - compute a SHA1 digest of one or more input buffers.
//
//  PARMS:  
//    BIS_APPINFO_PTR  appInfo      - appInfo structure
//    CSSM_DATA        dataBufs[]   - array of 1 or more data bufs to digest.
//    UINT32           nbrDataBufs  - nbr elements in array.
//    BIS_DATA_PTR     *digestOut   - [OUT] pointer to digest area, which
//                                    must be MEM_free'd eventually.
//
//
//  returns: BIS_OK on success.
//----------------------------------------------------------------------------------//



EFI_STATUS 
sha1Digest( 
     BIS_APPINFO_PTR  appInfo
    ,CSSM_DATA        dataBufs[]
    ,UINT32           nbrDataBufs
    ,BIS_DATA_PTR     *digestOut
    )
{
    #define SHA1_DIGEST_SIZE_IN_BYTES 20

    BIS_DATA_PTR     sha1DigestBisData= BIS_NULL;
    CSSM_DATA        sha1Digest;

    APP_CSSMINFO_PTR cssmInfo=  appInfo->pCssmInfo;
    CSSM_CC_HANDLE   hCC=       CSSM_INVALID_HANDLE;
    EFI_STATUS       rc=        EFI_SUCCESS;
    
    
    //Set output value as if failed.
    *digestOut= BIS_NULL;

    //Create structure to hold the Digest
    sha1DigestBisData= 
    MEM_allocBisData( appInfo, SHA1_DIGEST_SIZE_IN_BYTES );
    if ( sha1DigestBisData == BIS_NULL)
    {
        rc= EFI_OUT_OF_RESOURCES;
    }


    //Continue if alloc OK.
    if (rc==EFI_SUCCESS)
    {
        //Put Structure into CSSM format.
        sha1Digest.Length= sha1DigestBisData->length;
        sha1Digest.Data=   sha1DigestBisData->data;

    
        //Prep to do digest.
        hCC= CSSM_CSP_CreateDigestContext(cssmInfo->hCSP, CSSM_ALGID_SHA1);
        if (hCC==CSSM_INVALID_HANDLE)
        {
            rc=  EFI_DEVICE_ERROR;
        }


        //Continue if Created OK.
        if ( rc == EFI_SUCCESS)
        {
            //Create a SHA_1 digest of input data items.
            rc= CSSM_DigestData(hCC, dataBufs, nbrDataBufs, &sha1Digest); 
            if (rc != CSSM_OK)
            {
                saveCssmErr( appInfo->pCssmInfo );
                rc= EFI_DEVICE_ERROR;
            }

        }

    }
    
    
    //Free the digest if errors have occured    
    if ( rc != EFI_SUCCESS && sha1DigestBisData != BIS_NULL)
    {
        MEM_free(appInfo, sha1DigestBisData );
    }
    else    //Pass digest back to caller.
    {
        *digestOut= sha1DigestBisData;
    }


    //Free cssm handle if valid
    if ( hCC != CSSM_INVALID_HANDLE )
    {
        CSSM_DeleteContext(hCC);
    }

    return  rc;

}


//eof
