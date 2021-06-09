/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    efibis_basecode.c

Abstract:

    Public EFI/BIS functions.


Revision History

--*/

#include <efi.h>
#include <efidriverlib.h>
#include EFI_PROTOCOL_DEFINITION (Bis)
#include <bisBaseCode.h>

static EFI_GUID 		 BISBaseCodeProto=	EFI_BIS_PROTOCOL_GUID;
EFI_BIS_PROTOCOL     *BISBC= NULL;



	//
	// EBDP2DB - Assign EFI_BIS_DATA* to BIS_DATA.
	//		Conversion is safe even if null EFI_BIS_DATA* is presented.
	//
void
EBDP2BD( EFI_BIS_DATA *ebdp,  BIS_DATA* bd)
{
	if ( ebdp == NULL ){
		bd->length= 0;
		bd->data=   0;
	}
	else {
		bd->length= ebdp->Length;
		bd->data=   ebdp->Data;
	}
}

	//
 	// getInstanceData -
 	//		parm:    EFI_BIS_PROTOCOL pointer
	//		returns: address of BISBC_INSTANCEDATA that contains the input parm.
	//				 
BISBC_INSTANCEDATA*
getInstanceData( EFI_BIS_PROTOCOL *bisInterface )
{
	BISBC_INSTANCEDATA *result= NULL;	
	
	if ( bisInterface != NULL )
	{
		result= CR(bisInterface, BISBC_INSTANCEDATA, bisInterface, BBCIDSIG);
		
		//structure at computed address should contain structure length
		//and contain signature.
		if ( result->sizeOfThis != sizeof(BISBC_INSTANCEDATA)
		||   result->Signature  != BBCIDSIG )
		{
			result= NULL;
		}
						
	}
	
	return result;

}


	//
	// EFI_BIS_Initialize()
	//

EFI_STATUS
EFI_BIS_Initialize(
    IN     EFI_BIS_PROTOCOL        *This,             //this
    OUT    BIS_APPLICATION_HANDLE  *AppHandle,        //Application handle.
    IN OUT EFI_BIS_VERSION         *InterfaceVersion, //ver needed/available.
    IN     EFI_BIS_DATA            *TargetAddress     //Address of BIS platform.
    )
{
	BIS_INIT_PARMS 	   initParms;
	BIS_INIT_PARMS	   *parmBlock= &initParms;
    BIS_APPINFO_PTR    appInfo;
    NVM_AREA_ID        bisNvmAreaId= NVM_BIS_AREA_ID;
    UINT32             maxSize;
    BIS_STATUS         addOK;
	BISBC_INSTANCEDATA *bisGlobal;
	COLLECTION		   *appInfoTrackingCollection;

	//Print(L"Bis_Initialize called\n");

	if ( This == NULL || AppHandle == NULL
	||   InterfaceVersion == NULL  )
	{
		return EFI_DEVICE_ERROR;
	}


	//
	// Do parm block validation. Almost verbatim from BIS legacy client.
	//
	bisGlobal=  getInstanceData( This );
	if ( bisGlobal == NULL )
	{
		return EFI_DEVICE_ERROR;
	}
	
	//Only one BIS user at a time.
	if ( bisGlobal->bisInUse == BIS_TRUE ){
	    return EFI_OUT_OF_RESOURCES;
    }
	
		
	appInfoTrackingCollection= bisGlobal->appInfoTrackingCollection;

	//Convert EFI_BIS parms to BIS parm block.
	EfiZeroMem( &initParms, sizeof(initParms) );
	initParms.sizeOfStruct= 	sizeof(initParms);
	initParms.interfaceVersion.major= InterfaceVersion->Major;
	initParms.interfaceVersion.minor= InterfaceVersion->Minor;

  EBDP2BD( TargetAddress,  &initParms.targetAddress);


    //Version negotiation. As of Sept. 98 there is only one
    //version. If you ask for anything else, we fail you.
    //In the future we'll need to determine if the requested
    //version is compatible with the available version.
    if (parmBlock->interfaceVersion.major != BIS_CURRENT_VERSION_MAJOR)
    {
        //pass back actual numbers.
        InterfaceVersion->Major= BIS_CURRENT_VERSION_MAJOR;
        InterfaceVersion->Minor= BIS_CURRENT_VERSION_MINOR;

        return EFI_INCOMPATIBLE_VERSION;
    }

    //pass back actual minor version.
    InterfaceVersion->Minor= BIS_CURRENT_VERSION_MINOR;


    //Allocate and initialize appInfo structure and CSSM structure.
    parmBlock->appHandle= BIS_NULL;
    appInfo= (BIS_APPINFO_PTR) x_malloc( sizeof(BIS_APPINFO) + sizeof(APP_CSSMINFO));
    if (appInfo)
    {

        appInfo->SizeOfStruct= sizeof(BIS_APPINFO);

        //Add new appinfo struct to BIS' collection.
        addOK= COL_AddElement( appInfoTrackingCollection, (UINTN)appInfo );

        //create the apps memory tracking collection.
        if (addOK)
        {
            appInfo->memoryAllocations= COL_New( BIS_INIT_COLLECTION_SIZE
            ,  BIS_INIT_COLLECTION_INCR );
        }

        //Bail on memory allocation failures.
        if (addOK == BIS_FALSE || appInfo->memoryAllocations == NULL )
        {
            gBS->FreePool( appInfo );
            return EFI_OUT_OF_RESOURCES;
        }

        //Make appInfo->pCssmInfo point to it's piece of the allocation;
        appInfo->pCssmInfo= (APP_CSSMINFO_PTR) ( appInfo + 1);

		//Point to same instance data that interface structure points to.
		appInfo->efiInstanceData= bisGlobal;
    }
    else
    {
        return EFI_OUT_OF_RESOURCES;
    }

    //return the appinfo struct address.
    parmBlock->appHandle= (BIS_APPLICATION_HANDLE)appInfo;


    //Initialize the crypto and manifest service subsys.
    parmBlock->returnValue = (UINT32)PSD_InitializeApp( appInfo, appInfo->pCssmInfo );
    if ( parmBlock->returnValue != BIS_OK )
    {
        COL_Destroy( appInfo->memoryAllocations );
        gBS->FreePool( appInfo );
        return parmBlock->returnValue;
    }


    //Open our NVM (non volatile mem area).
    appInfo->nvmHandle= NVM_Open( appInfo, &bisNvmAreaId, &maxSize);
    if ( appInfo->nvmHandle == BIS_NULL)
    {

        //this not currently fatal.
        return EFI_DEVICE_ERROR;
    }


	bisGlobal->bisInUse=   BIS_TRUE;
	*AppHandle= parmBlock->appHandle;
    return EFI_SUCCESS;

}	//End EFI_BIS_Initialize



	//
	// EFI_BIS_FREE()
	//

EFI_STATUS
EFI_BIS_Free(
    IN BIS_APPLICATION_HANDLE  AppHandle,      //From Initialize( ).
    IN EFI_BIS_DATA            *ToFree         //[in]  BIS_DATA being freed.
	)
{
	EFI_STATUS      rc= EFI_SUCCESS;
  BIS_APPINFO_PTR appInfo;


    //Extract BIS_APPINFO_PTR
    appInfo= CAST_APP_HANDLE(AppHandle);

    //Check apphandle validity
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo)))
    {
        rc= BIS_BAD_APPHANDLE;
    }

	if ( ToFree == NULL )
	{
		return EFI_INVALID_PARAMETER;
	}

    //Free memory and remove from tracking collection in 'appInfo'.
	if ( rc == EFI_SUCCESS )
	{
	    if ( !MEM_free( appInfo, ToFree ) )
	    {
	        rc= EFI_INVALID_PARAMETER;
	    }
	}

    //success.
	return rc;

}


	//
	//	 EFI_BIS_Shutdown()
	//

EFI_STATUS
EFI_BIS_Shutdown(
    IN BIS_APPLICATION_HANDLE  AppHandle       // From Initialize( ).
	)
{
	BIS_SHUTDOWN_PARMS shutParms;

    BIS_APPINFO_PTR  appInfo;
    COLLECTION      *col;
    UINTN            colIx;
    UINTN            element;
    void            *memptr;
    EFI_STATUS       rv;

	BISBC_INSTANCEDATA  *bisGlobal;
	COLLECTION			*appInfoTrackingCollection;


	//Shutdown logic is almost verbatin from BIS legacy implementation.

    //Extract BIS_APPINFO_PTR
    appInfo= 	CAST_APP_HANDLE(AppHandle);


    //Check apphandle validity
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return EFI_NO_MAPPING;
    }

	bisGlobal=  				appInfo->efiInstanceData;
	appInfoTrackingCollection=  bisGlobal->appInfoTrackingCollection;


	EfiZeroMem( &shutParms, sizeof(shutParms) );
	shutParms.sizeOfStruct= sizeof(shutParms);
	shutParms.appHandle=    AppHandle;


    //Close the non-volative memory
    rv= NVM_Close( appInfo, appInfo->nvmHandle );
    if ( rv != EFI_SUCCESS)
    {
    	bisGlobal->bisInUse= BIS_FALSE;
      return rv;
    }


    //Close CSSM subsystem.
    PSD_Shutdown( appInfo->pCssmInfo );


    //Interate over the collection appInfo->memoryAllocations,
    //Free any memory that this app failed to free for itself
    col= appInfo->memoryAllocations;


    //Processing highest to lowest index, avoids compaction in COLLECTION.
    colIx= COL_GetCount( col );
    while ( colIx )       //NOTE: while condition contains an assignement!
    {
        colIx -= 1;             //adjust for zero origin index.

        if ( COL_ElementAt( col, colIx, &element ) )
        {
            memptr= (void*)element;
            gBS->FreePool( memptr );                //free the memory
            COL_RemoveElementAt( col, colIx);   //delete collection element.
        }

        colIx= COL_GetCount( col );
    }

    // Delete the collection object
    COL_Destroy( col );


    //Destroy appinfo object and remove from BIS' collection.
    gBS->FreePool( appInfo );
    COL_RemoveElement( appInfoTrackingCollection, (UINTN)appInfo );

   	bisGlobal->bisInUse= BIS_FALSE;
    return EFI_SUCCESS;

}


	//
	// EFI_BIS_GetBootObjectAuthorizationCertificate()
	//


EFI_STATUS
EFI_BIS_GetBootObjectAuthorizationCertificate(
    IN  BIS_APPLICATION_HANDLE  AppHandle,      // From Initialize( ).
    OUT EFI_BIS_DATA            **Certificate   // Pointer to certificate.
	)
{
	BIS_GBOAC_PARMS 	gboacParms;
	BIS_GBOAC_PARMS 	*parmBlock= &gboacParms;

  BIS_APPINFO_PTR 	appInfo;
  BIS_DATA_PTR    	boaCert;
  EFI_STATUS      	status;
	EFI_STATUS			persistSt;
  UINT32          	certLength;
  UINT32          	flashLength;
	BISBC_INSTANCEDATA  *bisGlobal;


    //Extract BIS_APPINFO_PTR
    appInfo= CAST_APP_HANDLE(AppHandle);

    //Check parms.
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return EFI_NO_MAPPING;
    }
	if ( Certificate == NULL )
	{
		return EFI_INVALID_PARAMETER;
	}

	bisGlobal=  appInfo->efiInstanceData;

	EfiZeroMem( &gboacParms, sizeof(gboacParms) );
	gboacParms.sizeOfStruct= sizeof(gboacParms);
	gboacParms.appHandle=    AppHandle;

    //Attempt to read the certificate length field from NVM.
    status= NVM_Read( appInfo->nvmHandle
        , (UINT8 *)&certLength
        , sizeof(certLength)
        , GET_NVM_FIELD_OFFSET( byteLengthOfBOACert )
        );
    if ( status != EFI_SUCCESS ) {
        return status;
    }
    else if ( certLength == 0 ){
        return EFI_NOT_FOUND;
    }

    //Is certLength reasonable? If too big for available area
    //assume that the plat cert is not configured.
    persistSt= bisGlobal->Persist->GetLength(
					bisGlobal->Persist, &flashLength, BIS_NULL );

	if ( persistSt != EFI_SUCCESS ){
		return EFI_NOT_FOUND;
	}

    flashLength= flashLength - sizeof(BIS_BOOLEAN); //updateCheckFlag size
    flashLength= flashLength
			   - GET_NVM_FIELD_SIZE( bootObjectAuthorizationUpdateCount );
    flashLength= flashLength - sizeof(certLength);

    if ( certLength > flashLength)
    {
        return EFI_NOT_FOUND;
    }


    //Create structure to hold the certificate.
    boaCert= parmBlock->certificate=
    MEM_allocBisData( appInfo, certLength );
    if ( boaCert == BIS_NULL) {
        return EFI_OUT_OF_RESOURCES;
    }


    //Attempt to read the certificate from NVM.
    status= NVM_Read( appInfo->nvmHandle
        , (UINT8 *)boaCert->data
        , certLength
        , GET_NVM_FIELD_OFFSET( bootObjectAuthorizationCertificate )
        );

    //Delete the data block if read failed.
    if (status != EFI_SUCCESS){
        MEM_free( appInfo, boaCert);
        parmBlock->certificate=  BIS_NULL;
    }

	  *Certificate= (EFI_BIS_DATA*)parmBlock->certificate;
    return status;

}


	//
	//	EFI_BIS_VerifyBootObject( )
	//

EFI_STATUS
EFI_BIS_VerifyBootObject(
    IN  BIS_APPLICATION_HANDLE AppHandle,    // From Initialize( ).
    IN  EFI_BIS_DATA           *Credentials, // Verification signed manifest.
    IN  EFI_BIS_DATA           *DataObject,  // Boot object to verify.
    OUT BOOLEAN                *IsVerified   // Result of verifcation.
	)
{
	BIS_VBO_PARMS   vboParms;
	BIS_STATUS      rc;
    BIS_APPINFO_PTR appInfo;




    //Extract BIS_APPINFO_PTR
    appInfo= CAST_APP_HANDLE(AppHandle);

    //Check apphandle validity
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return EFI_NO_MAPPING;
    }

	if ( Credentials == NULL || DataObject == NULL || IsVerified == NULL)
	{
		return EFI_INVALID_PARAMETER;
	}

	//Setup Parm Block
	EfiZeroMem( &vboParms, sizeof(vboParms) );
	vboParms.sizeOfStruct= sizeof(vboParms);
	vboParms.appHandle=	   AppHandle;

	EBDP2BD( Credentials,  &vboParms.credentials);
	EBDP2BD( DataObject,  &vboParms.dataObject);

    rc= Core_VerifyBootObject( appInfo, appInfo->pCssmInfo, &vboParms );
	*IsVerified= (BOOLEAN)((vboParms.isVerified == BIS_TRUE) ? TRUE:FALSE);

	return mapBisToEfi(rc);

}



	//
	// EFI_BIS_GetBootObjectAuthorizationCheckFlag()
	//

EFI_STATUS
EFI_BIS_GetBootObjectAuthorizationCheckFlag(
    IN  BIS_APPLICATION_HANDLE  AppHandle,        // From Initialize( ).
    OUT BOOLEAN                 *CheckIsRequired  // Value of check flag.
	)
{
  BIS_APPINFO_PTR   appInfo;
  EFI_STATUS        status;
	BIS_BOOLEAN		  checkIsRequired;




    //Extract BIS_APPINFO_PTR
    appInfo= CAST_APP_HANDLE(AppHandle);

    //Check apphandle validity
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return EFI_NO_MAPPING;
    }

	if ( CheckIsRequired == NULL){
		return EFI_INVALID_PARAMETER;
	}

    //Attempt to read the check flag from NVM.
    status= NVM_Read( appInfo->nvmHandle
        , (UINT8 *)&checkIsRequired
        , sizeof(BIS_BOOLEAN)
        , GET_NVM_FIELD_OFFSET( bootObjectAuthorizationCheckFlag )
        );

	if (status == BIS_OK){
		*CheckIsRequired= (BOOLEAN)((checkIsRequired)?TRUE:FALSE);
	}
  return status;
}


	//
	// EFI_BIS_GetBootObjectAuthorizationUpdateToken()
	//

EFI_STATUS
EFI_BIS_GetBootObjectAuthorizationUpdateToken(
    IN  BIS_APPLICATION_HANDLE  AppHandle,      // From Initialize( ).
    OUT EFI_BIS_DATA            **UpdateToken   // Value of update token.
	)
{
	BIS_GBOAUT_PARMS gboautParms;
    BIS_APPINFO_PTR appInfo;
    EFI_STATUS      status;
    UINT32          updateCounter;
    CSSM_GUID       systemGuid;
    EFI_STATUS      finalStatus;


    //Extract BIS_APPINFO_PTR
    appInfo= CAST_APP_HANDLE( AppHandle );

    //Check apphandle validity
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return EFI_NO_MAPPING;
    }

	if ( UpdateToken == NULL){
		return EFI_INVALID_PARAMETER;
	}

    //Attempt to read the update counter from NVM.
    status= NVM_Read( appInfo->nvmHandle
        , (UINT8 *)&updateCounter
        , GET_NVM_FIELD_SIZE( bootObjectAuthorizationUpdateCount )
        , GET_NVM_FIELD_OFFSET( bootObjectAuthorizationUpdateCount )
        );

    //The the platform guid from SMM
    if (status == EFI_SUCCESS)
    {
        //get the platform guid
        status= GetSystemGuid( &systemGuid );
    }

    //Call the "core" function to calc the update token
    if (status == EFI_SUCCESS)
    {
		EfiZeroMem( &gboautParms, sizeof(gboautParms) );
		gboautParms.sizeOfStruct= sizeof(gboautParms);
		gboautParms.appHandle=	  AppHandle;

        status=
        core_GetBootObjectAuthorizationUpdateToken(
             appInfo
            ,appInfo->pCssmInfo
            ,&gboautParms
            ,updateCounter
            ,&systemGuid);
            
           
  	*UpdateToken= (EFI_BIS_DATA*)gboautParms.updateToken;
    }


    //Prepare to exit with error.
    if (status != EFI_SUCCESS)
    {
        if (status != BIS_GETGUID_ERROR){	//preserve this error code
    			status= EFI_DEVICE_ERROR;
		    }
        gboautParms.updateToken= BIS_NULL;
      	*UpdateToken= (EFI_BIS_DATA*)gboautParms.updateToken;
    }

	finalStatus= mapBisToEfi((BIS_STATUS)status);
	

	
    return finalStatus;
}



//
//	EFI_BIS_UpdateBootObjectAuthorization(
//

EFI_STATUS
EFI_BIS_UpdateBootObjectAuthorization(
    IN  BIS_APPLICATION_HANDLE AppHandle,          // From Initialize( ).
    IN  EFI_BIS_DATA           *RequestCredential, // Update Request Manifest.
    OUT EFI_BIS_DATA           **NewUpdateToken    // Next update token.
	)
{
	BIS_UBOA_PARMS uboaParms;

    BIS_APPINFO_PTR appInfo;
    UINT32          rc;


    appInfo= CAST_APP_HANDLE(AppHandle);
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return EFI_NO_MAPPING;
    }

	if ( RequestCredential == NULL
	||   NewUpdateToken    == NULL )
	{
		return EFI_DEVICE_ERROR;
	}

    if (RequestCredential->Data   == BIS_NULL
    ||  RequestCredential->Length == 0 )
    {
        return EFI_INVALID_PARAMETER;
    }

	//Create parm struct.
	EfiZeroMem( &uboaParms, sizeof(uboaParms) );
	uboaParms.sizeOfStruct=			sizeof(uboaParms);
	uboaParms.appHandle=			AppHandle;
	EBDP2BD( RequestCredential, &uboaParms.requestCredential);



    //Call the update "core" function.
    rc= Core_UpdtBOA( appInfo, appInfo->pCssmInfo, &uboaParms);

	*NewUpdateToken= (EFI_BIS_DATA*)uboaParms.newUpdateToken;
    return mapBisToEfi(rc);
}



	//
	//	EFI_BIS_VerifyObjectWithCredential( )
	//

EFI_STATUS
EFI_BIS_VerifyObjectWithCredential(
    IN  BIS_APPLICATION_HANDLE AppHandle,     //  From Initialize( ).
    IN  EFI_BIS_DATA           *Credentials,  //  Verification signed manifest.
    IN  EFI_BIS_DATA           *DataObject,   //  Boot object to verify.
    IN  EFI_BIS_DATA           *SectionName,  //  Name of credential section to use.
    IN  EFI_BIS_DATA           *AuthorityCertificate,  // Certificate for credentials.
    OUT BOOLEAN                *IsVerified    // Result of verifcation.
	)
{
	BIS_VOWC_PARMS  vowcParms;
	BIS_VOWC_PARMS  *parmBlock= &vowcParms;
    BIS_APPINFO_PTR appInfo;
	BIS_STATUS      status;

    appInfo= CAST_APP_HANDLE(AppHandle);
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return (parmBlock->returnValue= BIS_BAD_APPHANDLE);
    }


	EfiZeroMem( &vowcParms, sizeof(vowcParms) );
	vowcParms.sizeOfStruct= 		sizeof(vowcParms);
	vowcParms.appHandle=			AppHandle;
	EBDP2BD( Credentials, 			&vowcParms.credentials );
	EBDP2BD( DataObject,			&vowcParms.dataObject );
	EBDP2BD( SectionName, 			&vowcParms.sectionName );
	EBDP2BD( AuthorityCertificate, 	&vowcParms.authorityCertificate );

    // Verify there is a object to be verified is non null,
    //   the section name is non null,
    //   and the credentials are non NULL
    if (parmBlock->dataObject.data    == BIS_NULL ||
        parmBlock->sectionName.data   == BIS_NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    // Credentials can NOT be null, return security error if they are
    if (parmBlock->credentials.data == BIS_NULL ||
        parmBlock->credentials.length == 0)
    {
        return EFI_SECURITY_VIOLATION;
    }

    //Authority certificate may be Null but, if it isn't,
    // the length must be > 0
    if (parmBlock->authorityCertificate.data   != BIS_NULL
    &&  parmBlock->authorityCertificate.length == 0 )
    {
        return EFI_SECURITY_VIOLATION;
    }


    status=Core_VerifyObjectWithCredential(
		appInfo, appInfo->pCssmInfo, parmBlock );

	*IsVerified=  (BOOLEAN)((parmBlock->isVerified)?TRUE:FALSE);
	return mapBisToEfi(status);
}


	//
	// EFI_BIS_GetSignatureInfo(
	//

EFI_STATUS
EFI_BIS_GetSignatureInfo(
    IN  BIS_APPLICATION_HANDLE  AppHandle,      //  From Initialize( ).
    OUT EFI_BIS_DATA            **SignatureInfo // Signature info struct.
	)
{
	BIS_GSI_PARMS  			gsiParms;
	BIS_GSI_PARMS  			*parmBlock= &gsiParms;
    BIS_APPINFO_PTR         appInfo;
    BIS_SIGNATURE_INFO_PTR  sigInfo;
    EFI_STATUS              rc= EFI_SUCCESS;
    BIS_GBOAC_PARMS         gboacParms;
    EFI_STATUS              efiStatus = EFI_SUCCESS;


    //Extract BIS_APPINFO_PTR
    appInfo= CAST_APP_HANDLE(AppHandle);

    //Check apphandle validity
    if ( appInfo == BIS_NULL  || (!IS_APPINFO_VALID(appInfo))){
        return EFI_NO_MAPPING;
    }
	if ( SignatureInfo == NULL){
		return EFI_DEVICE_ERROR;
	}

	EfiZeroMem( &gsiParms, sizeof(gsiParms) );
	gsiParms.sizeOfStruct=	sizeof( gsiParms );
	gsiParms.appHandle=		AppHandle;

    //Create structure to hold the SigInfo
    //FOR NOW there is only one entry.
    parmBlock->signatureInfo=  MEM_allocBisData( appInfo,  sizeof(BIS_SIGNATURE_INFO)*1 );
    if ( parmBlock->signatureInfo == BIS_NULL)
    {
        rc= BIS_MEMALLOC_FAILED;
    }

    else
    {
        //Get signature Info pointer and fill in structure.
        sigInfo=  (BIS_SIGNATURE_INFO*)gsiParms.signatureInfo->data;
        sigInfo->algorithmID=   BIS_ALG_DSA;
        sigInfo->keyLength=     1024;


        //Get the platform Boot Obj Auth Cert
		efiStatus=EFI_BIS_GetBootObjectAuthorizationCertificate(
    		AppHandle,      	 					 // From Initialize( ).
    		(EFI_BIS_DATA**)&gboacParms.certificate   // Pointer to certificate.
			);

        //Certificate Exists, create short hash from it.
        if (efiStatus==EFI_SUCCESS)
        {

            CSSM_DATA    csmBufs[1];
            BIS_DATA_PTR digest;

            csmBufs[0].Data=   gboacParms.certificate->data;
            csmBufs[0].Length= gboacParms.certificate->length;

            rc= sha1Digest( appInfo, csmBufs, 1, &digest);
            if (rc==BIS_OK)
            {
                //extract 1st 32bits of digest
                sigInfo->certificateID=(*(UINT32*)digest->data);
                sigInfo->certificateID &= BIS_CERT_ID_MASK;

                //Free the complete digest.
                MEM_free( appInfo, digest);
            }

            //Probable out of memory. sha1Digest failed.
            //Release the SigInfo structure.
            else
            {
                MEM_free( appInfo, parmBlock->signatureInfo);
                parmBlock->signatureInfo= BIS_NULL;
            }

            //Free the certificate
            MEM_free( appInfo, gboacParms.certificate);


        }   /*certificate exists*/

        else //No BOAC was found, fill in the predefined value
        {    //for the certificate hash.

            sigInfo->certificateID= BIS_CERT_ID_DSA;
            rc= BIS_OK;
        }


    } /*end else*/


	if ( rc == BIS_OK ){
		*SignatureInfo= (EFI_BIS_DATA*)parmBlock->signatureInfo;
	}

    return mapBisToEfi((BIS_STATUS)rc);

}



	//
	// EFIBIS_BaseCodeModuleInit(
	//


EFI_STATUS
EFIBIS_BaseCodeModuleInit(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{

  EFI_STATUS              	Status= 	EFI_SUCCESS;
  BISBC_INSTANCEDATA 			*instanceData=	NULL;
  EFI_BIS_PROTOCOL   		*bisI_F=		NULL;
	EFI_STATUS					locateStatus;
	EFI_HANDLE					deviceHandle= 0;

  EfiInitializeDriverLib (ImageHandle, SystemTable);
    

	//Allocate space for the modules instance data.
  instanceData = x_malloc(sizeof(BISBC_INSTANCEDATA));

	if ( instanceData == NULL )
	{
		Status= EFI_OUT_OF_RESOURCES;
	}
	//Print(L"EFIBIS_BaseCodeModuleInit AllocZeroPool= %d\n", Status);


	//Allocate our interface struct.
	if ( Status == EFI_SUCCESS )
	{
		instanceData->sizeOfThis= sizeof(BISBC_INSTANCEDATA);
		instanceData->Signature=  BBCIDSIG;
		instanceData->bisInUse=   BIS_FALSE;

		bisI_F= &instanceData->bisInterface;

		//Init our interface struct.
		bisI_F->Revision=   EFI_BIS_PROTOCOL_REVISION;
		bisI_F->Initialize=	EFI_BIS_Initialize;
		bisI_F->Shutdown= 	EFI_BIS_Shutdown;
		bisI_F->Free= 		EFI_BIS_Free;

		bisI_F->GetBootObjectAuthorizationCertificate=
							EFI_BIS_GetBootObjectAuthorizationCertificate;
		bisI_F->GetBootObjectAuthorizationCheckFlag=
							EFI_BIS_GetBootObjectAuthorizationCheckFlag;
		bisI_F->GetBootObjectAuthorizationUpdateToken=
							EFI_BIS_GetBootObjectAuthorizationUpdateToken;
		bisI_F->GetSignatureInfo=
							EFI_BIS_GetSignatureInfo;
		bisI_F->UpdateBootObjectAuthorization=
							EFI_BIS_UpdateBootObjectAuthorization;
		bisI_F->VerifyBootObject=
							EFI_BIS_VerifyBootObject;
		bisI_F->VerifyObjectWithCredential=
							EFI_BIS_VerifyObjectWithCredential;

		//Init the a global pointer to our interface.
		BISBC= bisI_F;

	}

	//Locate bis modules
	if ( Status == EFI_SUCCESS )
	{

		locateStatus= EFIBIS_InitPersistModule(&instanceData->Persist);
		if (locateStatus != EFI_SUCCESS ){
			Status= EFI_LOAD_ERROR;
		}
		//Print(L"EFIBIS_BaseCodeModuleInit EFIBIS_InitPersistModule= %d\n", locateStatus);
	
		locateStatus= EFIBIS_InitAuthFxnModule( &instanceData->Authorize );

    if (locateStatus != EFI_SUCCESS ){
			Status= EFI_LOAD_ERROR;
		}
		//Print(L"EFIBIS_BaseCodeModuleInit EFIBIS_InitAuthFxnModule= %d\n", locateStatus);


	}

	//Publish our interface
    if ( Status == EFI_SUCCESS)
    {
		Status =  gBS->InstallProtocolInterface(
                &deviceHandle,			//address of new handle
                &BISBaseCodeProto,		//protocol
                EFI_NATIVE_INTERFACE,
                bisI_F					//interface
                );

		//Print(L"EFIBIS_BaseCodeModuleInit LibInstallProtocolInterfaces= %d\n", Status);
	}



	//BIS SPECIFIC INIT
	if ( Status == EFI_SUCCESS )
	{
    	//Init the collection object used to track BIS_APPINFO objects.
    	instanceData->appInfoTrackingCollection= COL_New( APPINFO_COLLECTION_SIZE
        , APPINFO_COLLECTION_INCR);
        if ( instanceData->appInfoTrackingCollection == NULL){
			Status= EFI_OUT_OF_RESOURCES;
		}
		
		//Print(L"EFIBIS_BaseCodeModuleInit COL_New= %d\n", Status);

	}


	if ( Status == EFI_SUCCESS )
	{
		//Non Volative Memory Initialization
		NVM_Init();
		//Print(L"EFIBIS_BaseCodeModuleInit NVM_Init called\n");

		//Do one time CSSM init.
		PSD_InitCSSM( instanceData );
		//Print(L"EFIBIS_BaseCodeModuleInit PDS_InitCSSM called\n");
	}


	//Free memory if we are going to fail for EFI reasons
	if ( Status != EFI_SUCCESS )
	{
		if ( instanceData != NULL){
			gBS->FreePool( instanceData );
		}

	//	if ( bisI_F != NULL){
	//		gBS->FreePool( bisI_F );
	//	}
	}


	//Print(L"EFIBIS_BaseCodeModuleInit final status= %d\n", Status);

	#if 0
	if ( Status != EFI_SUCCESS){
	    DEBUG(( D_ERROR, "EFIBIS_BaseCodeModuleInit error= %d\n", Status));
		BREAKPOINT();
	}
	#endif
	
	

    return Status;
}



// ----------------------------------------------- //
// The following macro generates the entrypoints needed to load this module
// as a DLL in the NT emulation environent.
//

EFI_DRIVER_ENTRY_POINT(EFIBIS_BaseCodeModuleInit);
