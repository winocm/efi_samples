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
// PSD_INIT.C
//
// Description:
//  Contains functions to init and shutdown OASIS aka CSSM (lite) aka PSD.
//
//************************************************************************************************//

#include <bisBaseCode.h>
//#include <cssmMemFxns.h>


    //--------------------------------------------------------//
    // PSD_InitCSSM - one time CSMM initialization.
    //--------------------------------------------------------//
BIS_APPINFO_PTR cssmInitsAppInfoStruct;

BIS_STATUS
PSD_InitCSSM( BISBC_INSTANCEDATA *bisGlobal )
{
	CSSM_VERSION      Version;
    CSSM_RETURN       rc;

    CSSM_MEMORY_FUNCS memFuncs=
    {
        malloc_cssm,
        free_cssm,
        realloc_cssm,
        calloc_cssm,
        BIS_NULL
    };

    BIS_BOOLEAN  addOK;
    COLLECTION  *appInfoTrackingCollection= bisGlobal->appInfoTrackingCollection;

    //Create a skeletal AppInfo struct for use
    //by our CSSM memory function wrappers.
    cssmInitsAppInfoStruct= (BIS_APPINFO_PTR)
        x_malloc( sizeof(BIS_APPINFO) + sizeof(APP_CSSMINFO));
    if (cssmInitsAppInfoStruct==NULL)
    {
        return BIS_INIT_FAILURE;
    }

    cssmInitsAppInfoStruct->SizeOfStruct= sizeof(BIS_APPINFO);
    memFuncs.AllocRef=                   cssmInitsAppInfoStruct;

    //Add new appinfo struct to BIS' collection.
    addOK= COL_AddElement( appInfoTrackingCollection
    , (UINTN)cssmInitsAppInfoStruct );

    //Create a collection to track memory allocated with
    //CSSMs appInfo struct.
    if (!addOK)return 2;
    cssmInitsAppInfoStruct->memoryAllocations=
        COL_New( BIS_INIT_COLLECTION_SIZE,  BIS_INIT_COLLECTION_INCR );

    //Make appInfo->pCssmInfo point to it's piece of the allocation;
    cssmInitsAppInfoStruct->pCssmInfo=
        (APP_CSSMINFO_PTR) ( cssmInitsAppInfoStruct + 1);


	/* Initialize CSSM */
	Version.Major = 2;
	Version.Minor = 0;

    //
    //  Init CSSM.
    //
    memFuncs.AllocRef= (void*)cssmInitsAppInfoStruct;
	rc= CSSM_Init(&Version, &memFuncs);

    return rc;

}


    //--------------------------------------------------------//
    // PSD_InitializeApp - this is called for each BIS client //
    //--------------------------------------------------------//

EFI_STATUS
PSD_InitializeApp(
        BIS_APPINFO_PTR  appInfo    //The calling app's instance data.
       ,APP_CSSMINFO_PTR cssmInfo   //Struct passed by caller to be filled in.
)
{
	CSSM_VERSION      Version;
    CSSM_MEMORY_FUNCS memFuncs=
    {
        malloc_cssm,
        free_cssm,
        realloc_cssm,
        calloc_cssm,
        BIS_NULL
    };



	/* Initialize System */
	Version.Major = 2;
	Version.Minor = 0;

    //Set to known value.
    cssmInfo->hVL=       CSSM_INVALID_HANDLE;
    cssmInfo->hCSP=       CSSM_INVALID_HANDLE;

    cssmInfo->lastErr=   0;
    cssmInfo->appInfo=   appInfo;
    memFuncs.AllocRef= (void*)appInfo;



    //
    //  Attach Verifiable Object Library Module.
    //
    cssmInfo->hVL=
    CSSM_ModuleAttach(                      // attach vsl
		&intel_preos_SMv2_vlm_guid_2_0_0,   // const CSSM_GUID_PTR GUID,
		&Version,                           // CSSM_VERSION_PTR
		&memFuncs,                          // CSSM_API_MEMORY_FUNCS_PTR MemoryFuncs,
		CSSM_SERVICE_VL,                    // SubserviceID,
		0,                                      //SubserviceFlags,
		0,                                      //Application
        NULL,                                   //Notification Callback
        NULL,                                   //AppFileName
        NULL,	                                //AppPath
        NULL);	                                //Reserved

	if (cssmInfo->hVL == CSSM_INVALID_HANDLE)
    {
        saveCssmErr( cssmInfo );
        PSD_Shutdown( cssmInfo );
        return EFI_DEVICE_ERROR;
    }



    //
    //  Attach Crypto service.
    //
    cssmInfo->hCSP =
    CSSM_ModuleAttach(                   // /* attach csp */
		&intel_preos_csm_bis_guid_2_0_0,        //CSSM_GUID_PTR GUID,
		&Version,                               //CSSM_VERSION_PTR Version,
		&memFuncs,                              //CSSM_API_MEMORY_FUNCS_PTR MemoryFuncs,
		CSSM_SERVICE_CSP,                       //SubserviceID,
		0,                                      //SubserviceFlags,
		0,                                      //Application
        NULL,                                   //Notification Callback
        NULL,                                   //AppFileName
        NULL,	                                //AppPath
        NULL);	                                //Reserved

	if (cssmInfo->hCSP == CSSM_INVALID_HANDLE)
    {
        saveCssmErr( cssmInfo );
        PSD_Shutdown( cssmInfo );
        return EFI_DEVICE_ERROR;
    }


    return EFI_SUCCESS;
}






    //-----------------//
    // PSD_Shutdown    //
    //-----------------//

BIS_STATUS
PSD_Shutdown( APP_CSSMINFO_PTR cssmInfo )      //The calling app's cssm data.
{
	if (cssmInfo->hVL  != CSSM_INVALID_HANDLE) CSSM_ModuleDetach(cssmInfo->hVL);
    if (cssmInfo->hCSP != CSSM_INVALID_HANDLE) CSSM_ModuleDetach(cssmInfo->hCSP);

	return BIS_TRUE;
}




    //-------------------//
    //  saveCssmErr - get last error from CSSM and
    //      save it in the APP_CSSMINFO structure provided
    //

BIS_STATUS
saveCssmErr( APP_CSSMINFO_PTR cssmInfo )
{
    CSSM_ERROR_PTR lastErr;

    lastErr=  CSSM_GetError();
    cssmInfo->lastErr= lastErr->error;
    return BIS_SECURITY_FAILURE;
}


//HISTORY: this file derived from smbios bis file...
// Archive: /SMA/Src/bis/Psd/psd_init.c
// Revision: 21
// Date: 10/04/99 12:14p

//eof
