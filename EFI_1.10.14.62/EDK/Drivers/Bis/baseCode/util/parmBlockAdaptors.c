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

    parmBlockAdaptors.c

Abstract:


	  The core_xxxxx.c modules make calls to low level bis functions using
	  the single parmblock pointer interface. The functions in this module
	  provide adaptors that translate that style of call into the new EFI BIS
	  function intferface. This allows us to avoid a source branch or
	  a lot of #ifdef code in the core modules.


Revision History

--*/


#include <bisBaseCode.h>


UINT32
BIS_GetBootObjectAuthorizationCertificate( BIS_GBOAC_PARMS *parmBlock )
{
	EFI_STATUS estatus;

	estatus= EFI_BIS_GetBootObjectAuthorizationCertificate(
		parmBlock->appHandle,      				   // AppHandle From Initialize().
		(EFI_BIS_DATA**)&parmBlock->certificate	   // Pointer to certificate
		);

	return parmBlock->returnValue= mapEfiToBis( estatus );
}



UINT32
BIS_GetBootObjectAuthorizationCheckFlag( BIS_GBOACF_PARMS *parmBlock )
{

	EFI_STATUS estatus;
	BOOLEAN    checkRequired;


	estatus= EFI_BIS_GetBootObjectAuthorizationCheckFlag(
		parmBlock->appHandle,		// AppHandle From Initialize().
		&checkRequired				// Value of check flag.
		);

    if (EFI_SUCCESS == estatus)
    {
        parmBlock->checkIsRequired= checkRequired;
    }

	parmBlock->returnValue= mapEfiToBis( estatus );
    return parmBlock->returnValue;
}



UINT32
BIS_GetBootObjectAuthorizationUpdateToken( BIS_GBOAUT_PARMS *parmBlock )
{

	EFI_STATUS estatus;

	estatus= EFI_BIS_GetBootObjectAuthorizationUpdateToken(
		parmBlock->appHandle,      				  // AppHandle From Initialize().
		(EFI_BIS_DATA**)&parmBlock->updateToken   // Value of update token.
		);

	return parmBlock->returnValue= mapEfiToBis( estatus );
}



UINT32
BIS_VerifyObjectWithCredential( BIS_VOWC_PARMS *parmBlock )
{
	EFI_STATUS estatus;
	BOOLEAN    obIsVerified;

	estatus= EFI_BIS_VerifyObjectWithCredential(
		parmBlock->appHandle,      				  		// AppHandle From Initialize().
		(EFI_BIS_DATA*)&parmBlock->credentials,  		//  Verification signed manifest.
		(EFI_BIS_DATA*)&parmBlock->dataObject,   		//  Boot object to verify.
		(EFI_BIS_DATA*)&parmBlock->sectionName,  		//  Name of credential section to use.
		(EFI_BIS_DATA*)&parmBlock->authorityCertificate,  // Certificate for credentials.
		&obIsVerified							   		  // Result of verifcation.
		);

    if (EFI_SUCCESS == estatus)
    {
        parmBlock->isVerified = obIsVerified;
    }

	parmBlock->returnValue= mapEfiToBis( estatus );
	return parmBlock->returnValue;

}

//eof
