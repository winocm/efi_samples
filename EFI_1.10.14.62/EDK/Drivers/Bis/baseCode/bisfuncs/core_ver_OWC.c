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
// core_ver_OWC.c
//
// Description: 
// 
//  Contains the core logic of the "Verify Object With Credentials"
//  implemented in terms of PSD CSSM cryptography services.
//
//  Called by BIS_VerifyObjectWithCredential( ... ) after coarse
//  grained parm checking is complete.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bis_priv.h>



BIS_STATUS 
Core_VerifyObjectWithCredential( 
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_VOWC_PARMS   *parmBlock )
{
    BIS_DATA          Credentials= parmBlock->credentials;
    BIS_DATA          DataObject=  parmBlock->dataObject;
    BIS_DATA          SectionName= parmBlock->sectionName;
    BIS_DATA          AuthorityCertificate= parmBlock->authorityCertificate;
    CSSM_DATA         authorityCert;

    BIS_BOOLEAN_PTR   IsVerified=  &parmBlock->isVerified;       
    BIS_SM            credentialSmInfo;
    BIS_STATUS        brc;

    CSSM_VL_DO_LMAP         doMap;
    CSSM_VL_DO_LMAP_ENTRY   doMapEnt;
    CSSM_RETURN             rc;


    // *** Prepare the "Credentials" manifest for use.
    *IsVerified= BIS_FALSE;
    brc= PrepareSignedManifestHandle(
        cssmInfo                 //Struct filled in by PSD_Initialize.
        ,&Credentials             //SM being prepared.
        ,&credentialSmInfo       //SMINFO tobe filled in.
        ,VERIFIABLE_OBJECT_SIGINFO_NAME //Name of signer info.
        );
    if ( brc != BIS_OK ) 
    { 
        return (parmBlock->returnValue= brc); 
    }

    
    // *** Attach "DataObject" parm to "Credentials" manifest.

    //Construct a "do" map and a map entry describing the
    // DataObject parameter.
    doMap.NumberOfMapEntries= 1;
    doMap.MapEntries=         &doMapEnt;
    EfiCopyMem( (UINT8*) &doMapEnt.VoBundleIdentifier
            , (UINT8*) credentialSmInfo.bundleUid, sizeof(CSSM_GUID) );
    EfiCopyMem( (UINT8*) &doMapEnt.JoinName, SectionName.data, SectionName.length);
    doMapEnt.JoinName[SectionName.length]= 0; //terminal with null.
    doMapEnt.MapEntry.MediaType= CSSM_VL_MEDIA_TYPE_MEMORY;
    doMapEnt.MapEntry.Location.MemoryRef.Length= DataObject.length;
    doMapEnt.MapEntry.Location.MemoryRef.Data=   DataObject.data;

    //Attach the dataObject to the credentials manifest.
    rc= CSSM_VL_SetDoLMapEntries( cssmInfo->hVL, credentialSmInfo.hVerifiableObj, &doMap);
    if (rc==CSSM_FAIL)
    {
        brc= saveCssmErr( cssmInfo );
        FreeSignedManifestHandle(cssmInfo, &credentialSmInfo);
        return  brc;
    }
    
    
    //Prepare authority certificate, use the passed in OR
    // use the signer of the manifest.
    if (AuthorityCertificate.data !=NULL)
    {
        //Make stack variable contain cert data info.
        authorityCert.Data=   AuthorityCertificate.data;
        authorityCert.Length= AuthorityCertificate.length;
    }
    else {

        //Use the Signer Certificate of the credentials manifest
        //as the authority certificate.
        brc= GetSignerOfManifest( cssmInfo, &credentialSmInfo );
        if (brc!=BIS_OK)
        {
            FreeSignedManifestHandle(cssmInfo, &credentialSmInfo);
            return (parmBlock->returnValue= brc);
        }

        //Make stack variable contain cert data info.
        authorityCert.Data=   credentialSmInfo.sigInfo->SignerCertGroup->CertList->Data;
        authorityCert.Length= credentialSmInfo.sigInfo->SignerCertGroup->CertList->Length;

    }


    //  ***Combined integrity and authorization check.
    rc= CSSM_VL_VerifyRootCredentialsDataAndContainment(
        cssmInfo->hVL,                      // Verification Lib handle
        credentialSmInfo.hVerifiableObj,    // Credential & dataobject manifest
        &authorityCert,             //
        0, NULL,                    //Default CSP.
        0, NULL);                   //Not checking pointers into data objects.
    if ( rc == CSSM_INVALID_HANDLE)
    {
        saveCssmErr( cssmInfo );    //save actual fail code.
        parmBlock->returnValue= BIS_SECURITY_FAILURE;
    }
    else{
        *IsVerified= BIS_TRUE;
        parmBlock->returnValue= BIS_OK;
    }

    FreeSignedManifestHandle(cssmInfo, &credentialSmInfo);
    return parmBlock->returnValue;
}


//eof
