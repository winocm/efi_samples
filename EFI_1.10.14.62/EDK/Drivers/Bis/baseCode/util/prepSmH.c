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
// prepSmH.C
//
// Description: 
//  Contains functions to prep and dispose of signed manifest handles.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bis_priv.h>


 //------------------------------------------------
 // PrepareSignedManifestHandle-
 //     Makes a raw signed manifest ready for verification ops.
 //     Import and instantiate via CSSM.
 //     
 // The caller passes a pointer to a BIS_SM struct which is
 // filled in with the appropriate CSSM info ( a Verifiable
 // Object handle) if the operation is a success.
 //
 // Returns: BIS_OK or CSSM_ERROR_REPORTED.
 // In the case of an error, the exact CSSM error code
 // will be stored in cssmInfo->lastErr.
 //

static CSSM_GUID nullGuid= {0,0,0,{0,0,0,0,0,0,0,0}};
    

BIS_STATUS
PrepareSignedManifestHandle(
     APP_CSSMINFO_PTR cssmInfo     //Struct filled in by PSD_Initialize.
    ,BIS_DATA_PTR     rawSM        //SM being prepared
    ,BIS_SM_PTR       prepedSMInfo //Struct passed by caller to be filled in.
    ,CSSM_STRING      signerInformationName //Name of signer section inside 
                                            //of rawSM bundle.
    )
{
    CSSM_VL_LOCATION    bundleLoc;
    BIS_STATUS          brc         = BIS_OK;

    //Setup
    cssmInfo->lastErr= 0;
    prepedSMInfo->sigInfo= NULL;
    prepedSMInfo->doInfoPtr=  NULL;
    prepedSMInfo->bundleUid=  &nullGuid;


    //Convert rawSM to the CSSM equivalent
    bundleLoc.MediaType=                 CSSM_VL_MEDIA_TYPE_MEMORY;
    bundleLoc.Location.MemoryRef.Data=   rawSM->data;
    bundleLoc.Location.MemoryRef.Length= rawSM->length;



    //New simplified manifest instantiation.
    prepedSMInfo->hVerifiableObj=
    CSSM_VL_InstantiateVoFromLocation(
         cssmInfo->hVL,                     //VLHandle,
         &bundleLoc,                        //VoBundleLocation,
         BIS_NULL,                          //VoIdentifier,
         signerInformationName);            //VoName
    if (prepedSMInfo->hVerifiableObj == CSSM_INVALID_HANDLE) 
    {
        brc= saveCssmErr( cssmInfo );
    }

    return brc;
}



 //------------------------------------------------
 // FreeSignedManifestHandle
 //     Release resources acquired by PrepareSignedManifestHandle
 //

BIS_STATUS
FreeSignedManifestHandle(
     APP_CSSMINFO_PTR cssmInfo      //Struct filled in by PSD_Initialize.
    ,BIS_SM_PTR       prepedSMInfo  //Struct containing SM info.
    )
{

    BIS_STATUS   brc= BIS_OK;
    CSSM_RETURN  rc;

    cssmInfo->lastErr= 0;


    //Free the signature info if present.
    if ( prepedSMInfo->sigInfo != NULL )
    {

        rc= CSSM_VL_FreeSignatureInfo( cssmInfo->hVL
        ,  prepedSMInfo->sigInfo );
        if (rc==CSSM_FAIL)
        {
            brc= saveCssmErr( cssmInfo );
        }

        prepedSMInfo->sigInfo= NULL;

    }


    // Free the data object location map if present.
    if (prepedSMInfo->doInfoPtr != NULL)
    {

        rc= CSSM_VL_FreeDoInfos( cssmInfo->hVL, prepedSMInfo->doInfoPtr, 1);
        if (rc==CSSM_FAIL && brc== BIS_OK )   //preserve 1st error from above.
        {
            brc= saveCssmErr( cssmInfo );
        }
        prepedSMInfo->doInfoPtr= NULL;
    }



    //Free the VerifiableObject and delete the bundle.
    if ( prepedSMInfo->hVerifiableObj )
    {
        rc= CSSM_VL_FreeVo(cssmInfo->hVL,  prepedSMInfo->hVerifiableObj);
        if (rc==CSSM_FAIL && brc== BIS_OK )   //preserve 1st error from above.
        {
            brc= saveCssmErr( cssmInfo );
        }
    }


    return brc;    
}


//eof
