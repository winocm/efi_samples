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
// core_updt_BOA.c
//
// Description:
//
//  Contains the core logic of the "Update Boot Object Authorization"
//  function implemented in terms of PSD CSSM cryptography services.
//
//  Called by BIS_UpdateBootObjectAuthorization( ... ) after coarse
//  grained parm checking is complete.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bis_priv.h>



BIS_STATUS
areTokensEqual( BIS_DATA_PTR       calculatedToken
                , CSSM_DATA_PTR    tokenFromUpdateManifest);

#if (1)
#define UBOATRACE(s)
#else
#define UBOATRACE(s){if (BIS_FLAG(TRACEFLAGS,TRACE_UBOA)){PUT_S(s);}}
#endif


BIS_STATUS
Core_UpdtBOA(
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_UBOA_PARMS   *parmBlock )
{
    BIS_DATA        RequestCredential= parmBlock->requestCredential;

    CSSM_STRING     updateTokenName= UPDATETOKEN_ATTR_NAME;
    CSSM_DATA       updateTokenValue; //token from update manifest.


    //Data Structure Used to attach zero length external data to manifest.
    char                  dummyData=0;
    CSSM_VL_DO_LMAP_ENTRY dummyMapEntry=
    {
        {0,0,0,{0,0,0,0,0,0,0,0}},
        UPDATE_PARMS_SECTION_NAME,
        {CSSM_VL_MEDIA_TYPE_MEMORY, {0,0}}
    };

    CSSM_VL_DO_LMAP       dummyExternalData;

    CSSM_STRING     parmID_Name=     PARMID_ATTR_NAME;
    CSSM_DATA       parmID_Value;

    CSSM_STRING     parmValue_Name=  PARMVALUE_ATTR_NAME;
    CSSM_DATA       parmValue_Value;

    UINT32          updateField;        //which platform parm is to be updated...
    UINT32          updateCounter;      //used to increment platform update
    CSSM_DATA       updateCounterLV;    // counter.

    BIS_STATUS      brc;
    BIS_SM          smInfo;
    BIS_BOOLEAN     freeSmInfo= BIS_FALSE;

    BIS_GBOAC_PARMS  gboacParms;
    BIS_GBOAUT_PARMS gboautParms;
    CSSM_DATA       authorityCertificate;   //cssm format
    BIS_BOOLEAN     platSpecificSecurityCheckPassed;

    CSSM_VL_VERIFICATION_HANDLE hVeri;

    // Prepare for early error-exit cleanup.
    freeSmInfo             = BIS_FALSE;
    gboacParms.certificate = BIS_NULL;

    dummyExternalData.NumberOfMapEntries = 1;
    dummyExternalData.MapEntries = &dummyMapEntry;

    //
    // BUGBUG - remove warning 4 - init'd to zero
    //
    updateField = 0;
    parmValue_Value.Length = 0;

    // *** Prepare for manifest verification:
    UBOATRACE("UBOA40 ");

    brc= PrepareSignedManifestHandle(
        cssmInfo                    //Struct filled in by PSD_Initialize.
        ,&RequestCredential         //SM being prepared.
        ,&smInfo                    //SMINFO tobe filled in.
        ,UPDATE_MANIFEST_SIGINFO_NAME  //Name of signer information.
        );
    if ( brc != BIS_OK ) {
        goto ERROR_EXIT;
    }
    freeSmInfo= BIS_TRUE;



    // *** Retrieve source of authority
    UBOATRACE("UBOA50 ");
    gboacParms.sizeOfStruct= sizeof(gboacParms);
    gboacParms.appHandle=    parmBlock->appHandle;
    gboacParms.certificate=  BIS_NULL;

    brc= BIS_GetBootObjectAuthorizationCertificate( &gboacParms );

    #if (COMPILE_SELFTEST_CODE == 1)
    // Conditional code to skip authority check for test purposes
    if (BIS_FLAG(BEHAVFLAGS,BEHAV_ALLOW_ANY_UPDATE)) {
        // simulate having no configured authority certificate
        brc = BIS_BOA_CERT_NOTFOUND;
    }
    #endif // COMPILE_SELFTEST_CODE

    //Handle errors
    if (brc != BIS_OK && brc != BIS_BOA_CERT_NOTFOUND)
    {
        goto ERROR_EXIT;
    }

    //Boot Object Auth Cert was retrieved, cast into CSSM format.
    if ( brc == BIS_OK )
    {
        UBOATRACE("UBOA60 ");
        authorityCertificate.Data=   gboacParms.certificate->data;
        authorityCertificate.Length= gboacParms.certificate->length;
    }

    //No Boot Object Authorization Certificate is configured,
    //Perform Platform specific security check and use manifest
    //signer certificate as authority_cert.
    else // (brc == BIS_BOA_CERT_NOTFOUND)
    {
        BIS_DATA  temp_signer;

        UBOATRACE("UBOA60 ");

        //Use the Signer Certificate of the input manifest
        //as the authority certificate.
        UBOATRACE("UBOA70 ");
        brc= GetSignerOfManifest( cssmInfo, &smInfo );
        if (brc!=BIS_OK){
            goto ERROR_EXIT;
        }

        //Make stack variable contain cert data info.
        authorityCertificate.Data =
            smInfo.sigInfo->SignerCertGroup->CertList->Data;
        authorityCertificate.Length =
            smInfo.sigInfo->SignerCertGroup->CertList->Length;
        temp_signer.data   = authorityCertificate.Data;
        temp_signer.length = authorityCertificate.Length;

        #if (COMPILE_SELFTEST_CODE == 1)
        // Conditional code to skip authority check for test purposes
        if (BIS_FLAG(BEHAVFLAGS,BEHAV_ALLOW_ANY_UPDATE)) {
            platSpecificSecurityCheckPassed = BIS_TRUE;
        }
        else {
        #endif  // COMPILE_SELFTEST_CODE

        // Perform   external   authorization   check,   passing  the  signer's
        // certificate.
        brc = CallAuthorization(
            BISOP_UpdateBootObjectAuthorization,  // opCode
            & RequestCredential,                  // credentials
            & temp_signer,                        // credentialsSigner
            NULL,                                 // dataObject
            0,                                    // reserved
            & platSpecificSecurityCheckPassed     // isAuthorized
            );
        if (BIS_OK != brc) {
            brc = BIS_SECURITY_FAILURE;
            goto ERROR_EXIT;
        }


        #if (COMPILE_SELFTEST_CODE == 1)
        } // if flags is "allow" else
        #endif  // COMPILE_SELFTEST_CODE

        //Bail out if security check failed.
        if ( !platSpecificSecurityCheckPassed )
        {
            brc= BIS_SECURITY_FAILURE;
            goto ERROR_EXIT;
        }
    } // else BIS_BOA_CERT_NOTFOUND

    //Replace dummy zero-length external data in manifest with
    //memory reference.
    UBOATRACE("UBOA80 ");
    dummyMapEntry.MapEntry.Location.MemoryRef.Data= &dummyData;
    dummyMapEntry.VoBundleIdentifier= *smInfo.bundleUid;
    brc= CSSM_VL_SetDoLMapEntries(
            cssmInfo->hVL,
            smInfo.hVerifiableObj,
            &dummyExternalData);
    if ( brc != CSSM_OK)
    {
        brc= BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }


    // ***Combined integrity and authorization check of the input signed manifest
    // and the signer certificate.
    UBOATRACE("UBOA90 ");
    hVeri= CSSM_VL_VerifyRootCredentialsDataAndContainment(
        cssmInfo->hVL,              // Verification Lib handle
        smInfo.hVerifiableObj,      // Signed manifest handle.
        &authorityCertificate,      // BOA Cert or SM Cert
        0, NULL,                    //Default CSP.
        0, NULL);                   //Not checking pointers into data objects.
    if ( hVeri == CSSM_INVALID_HANDLE)
    {
        saveCssmErr( cssmInfo );    //save actual fail code.
        brc= BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }


    // *** Check the "ParameterSet" section in the signed Manifest. ***
    // It should contain a specific guid value.
    UBOATRACE("UBOA100 ");
    if ( !CheckParmsetValue(cssmInfo, &smInfo ) )
    {
        brc= BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }


    // *** Replay check ***
    // Check the signed manifest update token against, the
    // platforms value for it.
    // Get the "ParameterSetToken" value from the manifest.
    UBOATRACE("UBOA110 ");
    brc= GetDataObjectByName(cssmInfo
            ,&smInfo                    //SM to obtain token from.
            ,updateTokenName          //ObjName
            ,&updateTokenValue        //ObjValue [OUT]
            ,BIS_TRUE );                //do base 64 decode.

    if (brc!=BIS_OK){
        brc= BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }

    // Get platform update token.
    UBOATRACE("UBOA120 ");
    gboautParms.sizeOfStruct= sizeof(gboautParms);
    gboautParms.appHandle=    parmBlock->appHandle;
    gboautParms.updateToken=  BIS_NULL;
    brc= BIS_GetBootObjectAuthorizationUpdateToken( &gboautParms );
    if (brc!=BIS_OK){
        MEM_free( cssmInfo->appInfo, updateTokenValue.Data);
        goto ERROR_EXIT;
    }

    // Compare platform update token value against UpdateToken value
    // in signed manifest.
    UBOATRACE("UBOA130 ");
    brc= areTokensEqual( gboautParms.updateToken, &updateTokenValue );

    #if (COMPILE_SELFTEST_CODE == 1)
    // Conditional code to skip token check for test purposes
    if (BIS_FLAG(BEHAVFLAGS,BEHAV_ALLOW_ANY_UPDATE)) {
        brc = BIS_OK;
    }
    #endif

    // Free resources no longer needed.
    MEM_free( cssmInfo->appInfo, updateTokenValue.Data);
    MEM_free( cssmInfo->appInfo, gboautParms.updateToken );

    // Bail if tokens are not equal.
    if ( brc != BIS_OK)
    {
        brc= BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }



    // *** Update Platform Parameter***
    //Get the Name of the Platform parameter that is be be updated.
    // Eg. retrive the value of the parm named: "ParameterId" .
    // It should be  "BootObjectAuthorizationCertificate" or
    // "BootAuthorizationCheckFlag"

    UBOATRACE("UBOA140 ");
    brc= GetDataObjectByName(cssmInfo
            ,&smInfo                    //SM to obtain ParmId from.
            ,parmID_Name              //ObjName
            ,&parmID_Value            //ObjValue [OUT]
            ,BIS_TRUE );                //base 64 decode.


    // If 'ParameterId' was retrieved ...
    if ( brc == BIS_OK )
    {
        //Check the ParmeterId value for validity...
        if ( 0 == BIS_strncmp(parmID_Value.Data
                             , BOAC_PARMID, sizeof(BOAC_PARMID)-1 ) )
        {
            UBOATRACE("UBOA150 ");
            updateField= UPDATE_CERT;
        }
        else if ( 0 == BIS_strncmp(parmID_Value.Data
        , BOACF_PARMID, sizeof(BOACF_PARMID)-1 ) )
        {
            UBOATRACE("UBOA160 ");
            updateField= UPDATE_FLAG;
        }
        else {
            brc= BIS_SECURITY_FAILURE;
        }

        //free memory occupied by base64 decode value.
        MEM_free( cssmInfo->appInfo, parmID_Value.Data);

    }

    // If 'ParameterId' was retrieved and is valid ...
    UBOATRACE("UBOA170 ");
    if ( brc == BIS_OK )
    {
        //Get the Value of the Platform parameter that is be be updated:
        // (Manifest data object name is: "ParameterValue" ).
        // It is the value of the platform cert or the check flag.
        UBOATRACE("UBOA180 ");

        brc= GetDataObjectByName(cssmInfo
            ,&smInfo                    //SM to obtain ParmId from.
            ,parmValue_Name           //ObjName
            ,&parmValue_Value         //ObjValue [OUT]
            ,BIS_TRUE );                //do base 64 decode.

    }

    //If any problem with parmid of value, bail out.
    if (brc != BIS_OK)
    {
        brc= BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }

    //When updating  the check flag, data length must be 1.
    if (updateField == UPDATE_FLAG && parmValue_Value.Length != 1)
    {
        brc= BIS_SECURITY_FAILURE;
        goto ERROR_EXIT;
    }

    //else when updating  the cert do some verifcation on the cert.
    //(Don't consider null certs, Length == 0).
    else if (updateField == UPDATE_CERT && parmValue_Value.Length > 0)
    {
        UINT32    newCertChk;

        newCertChk= CSSM_VL_SelfVerifyCertificate(
                cssmInfo->hVL,  /* VLHandle */
                &parmValue_Value);        /* Certificate */

        if ( ISL_OK != newCertChk )
        {
            brc= BIS_SECURITY_FAILURE;
            goto ERROR_EXIT;
        }

    }


    //Update requested parm in NonVolatile Storage
    UBOATRACE("UBOA190 ");
    brc= (BIS_STATUS)UpdateBoaInNVM( appInfo, updateField, &parmValue_Value);
    if (brc != BIS_OK)
    {
        goto ERROR_EXIT;
    }



    // ***Increment update counter in NVM***
    //Attempt to read the update counter from NVM.
    UBOATRACE("UBOA200 ");
    brc= (BIS_STATUS)NVM_Read( appInfo->nvmHandle
        , (UINT8 *)&updateCounter
        , GET_NVM_FIELD_SIZE( bootObjectAuthorizationUpdateCount )
        , GET_NVM_FIELD_OFFSET( bootObjectAuthorizationUpdateCount )
        );


    //Update token in NonVolatile Storage (unless an error has occured).
    if (brc == BIS_OK )
    {
        UBOATRACE("UBOA210 ");

        //Incr counter and put into CSSM_DATA form.
        updateCounter += 1;
        updateCounterLV.Data=   (UINT8*)&updateCounter;
        updateCounterLV.Length= sizeof(updateCounter);

        //Update in NVM.
        brc= (BIS_STATUS)UpdateBoaInNVM( appInfo
        , UPDATE_COUNTER, &updateCounterLV);
    }

    // Get a copy of the platform update token for the caller.
    // (unless an error has occured).
    
    	// MEMFREEBUG description: the memory allocated for the update token
    	// in the following call will be freed in the call to 
    	// FreeSignedManifestHandle (FSMH) just below? It is believed that FSMH
    	// is freeing a stale pointer to memory that has been reallocated to
    	// the update token. Something about the legacy BIS implementation 
    	// masks this behavior.
    
    UBOATRACE("UBOA220 ");
    if (brc == BIS_OK)
    {
        gboautParms.sizeOfStruct= sizeof(gboautParms);
        gboautParms.appHandle=    parmBlock->appHandle;
        brc= BIS_GetBootObjectAuthorizationUpdateToken( &gboautParms );
        parmBlock->newUpdateToken = gboautParms.updateToken;
    }

ERROR_EXIT:
    if (freeSmInfo){
        FreeSignedManifestHandle(cssmInfo, &smInfo);
    }
    

    if ( gboacParms.certificate !=  BIS_NULL )
    {
        MEM_free( cssmInfo->appInfo, gboacParms.certificate);
    }

    return (parmBlock->returnValue= brc);
}



//-------------------------------------------------------------
// CheckParmsetValue
//
// Checks that the signedManfest pointed to by 'smInfo' contains
// a data object name "ParameterSet" and that it's value is
// equal to the value of the #define symbol
// BOOT_OBJECT_AUTHORIZATION_PARMSET_GUIDVALUE.
// returns:
//      BIS_BOOLEAN true if OK.
//

BIS_STATUS
CheckParmsetValue(
     APP_CSSMINFO_PTR           cssmInfo
    ,BIS_SM_PTR                 smInfo)
{
    BIS_STATUS      brc;
    BIS_BOOLEAN     result;


    CSSM_STRING     parmSetName= PARMSET_ATTR_NAME;
    CSSM_DATA       parmSetValue; //guid from update manifest.

    CSSM_GUID       bootObjectAuthorizationSetGUID=
                        BOOT_OBJECT_AUTHORIZATION_PARMSET_GUIDVALUE;
    CSSM_GUID_PTR   valueFromSm;


    //Get the "ParameterSet" data object from the SM.
    brc= GetDataObjectByName(cssmInfo
            ,smInfo                    //SM to obtain token from.
            ,parmSetName             //ObjName
            ,&parmSetValue           //ObjValue [OUT]
            ,BIS_TRUE );               //do base 64 decode.

    if (brc!=BIS_OK){
        return BIS_FALSE;
    }

    //check that the parmSetValue is the correct
    // length for a guid.
    if ( parmSetValue.Length != sizeof(CSSM_GUID) )
    {
        result= BIS_FALSE;
    }
    else {
        //Cast pointer to guid pointer and compare against expected guid.
        valueFromSm= (CSSM_GUID_PTR)parmSetValue.Data;
        result= EfiCompareGuid( (EFI_GUID *)valueFromSm, (EFI_GUID *)&bootObjectAuthorizationSetGUID);
    }

    //Free memory obtained by GetDataObjectByValue.
    MEM_free( cssmInfo->appInfo, parmSetValue.Data );

    return result;
}



    //-------------------------------------------------------------
    // Update Boot Object Authorization Field in NonVolatile Memory.
    //

EFI_STATUS
UpdateBoaInNVM(
      BIS_APPINFO_PTR appInfo
    , UINT32          updateField
    , CSSM_DATA_PTR   fieldValue)
{
    EFI_STATUS  status;
    BIS_BOOLEAN checkFlag;
    UINT8       *pCheckFlag;

    switch (updateField)
    {

    case UPDATE_FLAG:      //Update the bootAuthCheckFlag.
        pCheckFlag= (UINT8*)fieldValue->Data;   //promote byte to uint32.
        checkFlag=  *pCheckFlag;
        status= NVM_Write( appInfo->nvmHandle
            , (UINT8 *)&checkFlag
            , sizeof(BIS_BOOLEAN)
            , GET_NVM_FIELD_OFFSET( bootObjectAuthorizationCheckFlag ) );
        break;

    case UPDATE_CERT:       //Update the bootAuth Certificate.
        status= NVM_Write( appInfo->nvmHandle
            , (UINT8 *)&fieldValue->Length
            , sizeof(UINT32)
            , GET_NVM_FIELD_OFFSET( byteLengthOfBOACert ) );

        if ( status == BIS_OK ) {
            status= NVM_Write( appInfo->nvmHandle
            , (UINT8 *)fieldValue->Data
            , fieldValue->Length
            , GET_NVM_FIELD_OFFSET( bootObjectAuthorizationCertificate ));
        }
        break;


    case UPDATE_COUNTER:      //Update the bootAuth update token.
        status= NVM_Write( appInfo->nvmHandle
            , (UINT8 *)fieldValue->Data
            , sizeof(UINT32)
            , GET_NVM_FIELD_OFFSET( bootObjectAuthorizationUpdateCount ) );
        break;

    default:
        status= BIS_BAD_PARM;

    } //switch

    return status;
}


    //---------------------------------------------------------
    // areTokensEqual - compares a freshly calculated update
    //      token to the token extracted from the update
    //      request manifest.
    //
    //      returns: BIS_OK if two tokens are identical.
    //          and  BIS_SECURITY_FAILURE if not.

BIS_STATUS
areTokensEqual( BIS_DATA_PTR       calculatedToken
                , CSSM_DATA_PTR    tokenFromUpdateManifest)
{
    BIS_STATUS  result= BIS_OK;
    UINT32      l1, l2;
    UINT8       *d1, *d2;

    //Check length for equality.
    l1= calculatedToken->length;
    l2= tokenFromUpdateManifest->Length;
    if ( l1 != l2 )
    {
        result= BIS_SECURITY_FAILURE;
    }

    //Check data for equality
    else {

        d1= calculatedToken->data;
        d2= tokenFromUpdateManifest->Data;

        //Compare 'l1' bytes of data.
        for ( ; l1>0 ; --l1)
        {
            if (*d1 != *d2){
                result= BIS_SECURITY_FAILURE;
                break;
            }

            d1+=1;
            d2+=1;
        }

    }

    return result;



}



//eof
