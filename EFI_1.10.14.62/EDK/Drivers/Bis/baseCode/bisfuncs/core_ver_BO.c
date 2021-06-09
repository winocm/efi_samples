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
// core_ver_BOA.c
//
// Description: 
// 
//  Contains the core logic of the "Verify Boot Object"
//  implemented in terms of PSD CSSM cryptography services.
//
//  Called by BIS_VerifyBootObject( ... ) after coarse
//  grained parm checking is complete.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bis_priv.h>




BIS_STATUS 
Core_VerifyBootObject( 
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_VBO_PARMS   *parmBlock )
{
    //Stack based references to parameter block contents.
    BIS_DATA          bootObjectSignedManifest= parmBlock->credentials;
    BIS_DATA          bootObject=               parmBlock->dataObject;
    BIS_BOOLEAN       *IsVerified=              &(parmBlock->isVerified); 
             
    //Used in call to BIS_GetBootObjectAuthorizationCheckFlag()
    BIS_GBOACF_PARMS  getChkFlagParms;
    BIS_BOOLEAN       checkIsReqd;

    //Used in call to BIS_GetBootObjectAuthorizationCertificate()
    BIS_GBOAC_PARMS   gboacParms;
    BIS_DATA          bootAuthorizationCert;          
           
    //Used to extract signer info from bootObj Manifest.
    BIS_BOOLEAN       useCertFromManifest;
    BIS_SM            smInfo;
    BIS_BOOLEAN       freeSmInfo= BIS_FALSE;

    //Parm bundle for calling BIS_VerifyObjectWithCredentials.
    BIS_VOWC_PARMS    verObjWCredParms;
    BIS_DATA          sectionName;
    
    UINT8             sectNameStringValue[]= BOOT_OBJECT_SECTION_NAME;
    BIS_BOOLEAN       needPlatformSpecificAuthorityCheck;
    BIS_BOOLEAN       platformSpecificAuthorizationCheckPassed;
    BIS_STATUS        brc;

    //
    // BUGBUG - init'd to remove warning 4
    //
    bootAuthorizationCert.length = 0;

    //Assume failure and set up for error exit
    *IsVerified = BIS_FALSE;
    gboacParms.certificate = BIS_NULL;
    
    //Setup and call BIS_GetBootObjectAuthorityCheckFlag.
    getChkFlagParms.sizeOfStruct=    sizeof(getChkFlagParms);
    getChkFlagParms.appHandle=       parmBlock->appHandle;
    brc= BIS_GetBootObjectAuthorizationCheckFlag( &getChkFlagParms );
    if ( brc !=BIS_OK )
    {
        brc = BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }
    checkIsReqd= getChkFlagParms.checkIsRequired;


    //If check is not required and no credentials were supplied
    //in the parm block, set [out] parm to IsVerified= TRUE and return.
    if ( !checkIsReqd &&  bootObjectSignedManifest.data == BIS_NULL)
    {
        *IsVerified= BIS_TRUE;
        return (parmBlock->returnValue= BIS_OK);
    }

    //If check is required and no credentails were supplied
    //in the parm block, set [out] parm to IsVerified= FALSE and return error
    if ( checkIsReqd && bootObjectSignedManifest.data == BIS_NULL)
    {
        *IsVerified= BIS_FALSE;
        brc = BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }


    //If check is required do some setup...
    if ( checkIsReqd )
    {
        // Retrieve platform Boot Object Authorization Certificate
        gboacParms.sizeOfStruct= sizeof(gboacParms);
        gboacParms.appHandle=    parmBlock->appHandle;
        gboacParms.certificate = BIS_NULL;
        brc= BIS_GetBootObjectAuthorizationCertificate( &gboacParms );
        if (brc != BIS_OK && brc != BIS_BOA_CERT_NOTFOUND)
        {
            brc = BIS_SECURITY_FAILURE;
            goto ERROR_EXIT;
        }

        if (brc == BIS_OK) {
            // We have a platform certificate, use it for authority
            bootAuthorizationCert.data=   (gboacParms.certificate)->data;
            bootAuthorizationCert.length= (gboacParms.certificate)->length;
            useCertFromManifest                = BIS_FALSE;
            needPlatformSpecificAuthorityCheck = BIS_FALSE;
        } else {
            // No platform certificate, use manifest and ask for authority
            useCertFromManifest                = BIS_TRUE;
            needPlatformSpecificAuthorityCheck = BIS_TRUE;
        }
    }  // if (checkIsRequired)


    //
    //  Check is not required BUT manifest was supplied
    //  so use the certificate from the manifest for integrity
    //  verification and don't ask for authority check
    else 
    {
        useCertFromManifest                    = BIS_TRUE;
        needPlatformSpecificAuthorityCheck     = BIS_FALSE;
    } // if check is required else
    

    //If No platform cert is configured, 
    //Extract authority Cert from manifest.
    if ( useCertFromManifest ) {
        // *** Prepare for manifest verification:
        brc= PrepareSignedManifestHandle(
            cssmInfo                    //Struct filled in by PSD_Initialize.
            ,&bootObjectSignedManifest  //SM being prepared.
            ,&smInfo                    //SMINFO tobe filled in.
            ,VERIFIABLE_OBJECT_SIGINFO_NAME //Name of signer info.
            );
        if ( brc != BIS_OK ) {
            brc = BIS_SECURITY_FAILURE;
            goto ERROR_EXIT;
        }
        freeSmInfo= BIS_TRUE;
        
        //Get Signer Certificate of the input manifest
        brc= GetSignerOfManifest( cssmInfo, &smInfo );
        if (brc!=BIS_OK) {
            brc = BIS_SECURITY_FAILURE;
            goto ERROR_EXIT;
        }
        
        //Make stack variable reference cert data info.
        bootAuthorizationCert.data   =
            smInfo.sigInfo->SignerCertGroup->CertList->Data;
        bootAuthorizationCert.length =
            smInfo.sigInfo->SignerCertGroup->CertList->Length;
        
        // And check the authority of this source if needed
        if (needPlatformSpecificAuthorityCheck) {
            // Perform  external  authorization  check,  passing  the  signer's
            // certificate.
            brc = CallAuthorization(
                BISOP_VerifyBootObject,                    // opCode
                & bootObjectSignedManifest,                // credentials
                & bootAuthorizationCert,                   // credentialsSigner
                & bootObject,                              // dataObject
                0,                                         // reserved
                & platformSpecificAuthorizationCheckPassed // isAuthorized
                );
            if (BIS_OK != brc) {
                brc = BIS_SECURITY_FAILURE;
                goto ERROR_EXIT;
            }

            if (! platformSpecificAuthorizationCheckPassed) {
                // Bail out with a security error
                brc = BIS_SECURITY_FAILURE;
                goto ERROR_EXIT;
            }

        } // if need platform-specific authority check
        
    } // if using certificate from manifest


    //Setup and call BIS_VerifyObjectWithCredential to finish.
    verObjWCredParms.sizeOfStruct=     sizeof(verObjWCredParms);
    verObjWCredParms.appHandle=        parmBlock->appHandle;

    verObjWCredParms.credentials=      bootObjectSignedManifest;
    verObjWCredParms.dataObject=       bootObject;
    verObjWCredParms.authorityCertificate= bootAuthorizationCert;

    sectionName.data=                  &sectNameStringValue[0];
    sectionName.length=                sizeof(sectNameStringValue)-1; 
                                       // -1 omits ending null.
    verObjWCredParms.sectionName=      sectionName;
    verObjWCredParms.isVerified =      BIS_FALSE;

    brc= BIS_VerifyObjectWithCredential( &verObjWCredParms );

    *IsVerified = verObjWCredParms.isVerified;

    //Cleanup
ERROR_EXIT:    
    if (gboacParms.certificate != BIS_NULL){
        MEM_free( appInfo, gboacParms.certificate );
    }
    if (freeSmInfo) {
        FreeSignedManifestHandle(cssmInfo, &smInfo);
    }

    return (parmBlock->returnValue= brc);
}


//eof
