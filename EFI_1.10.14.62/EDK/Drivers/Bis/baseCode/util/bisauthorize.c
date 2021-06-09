///////////////////////////////////////////////////////////////////////////////
//		               INTEL CONFIDENTIAL	
/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/
//
// The  source  code contained or described herein and all documents related to
// the source code ("Material") are owned by Intel Corporation or its suppliers
// or  licensors.   Title to the Material remains with Intel Corporation or its
// suppliers   and   licensors.    The  Material  contains  trade  secrets  and
// proprietary  and  confidential  information  of  Intel  or its suppliers and
// licensors.   The  Material  is  protected  by  worldwide copyright and trade
// secret  laws  and  treaty  provisions.  No part of the Material may be used,
// copied,  reproduced,  modified,  published,  uploaded,  posted, transmitted,
// distributed,  or  disclosed in any way without Intel's prior express written
// permission.
//
// No  license  under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of  the Materials, either expressly, by implication, inducement, estoppel or
// otherwise.   Any  license  under  such  intellectual property rights must be
// express and approved by Intel in writing.
//
//  Purpose:
//      BIS  functions  may  require  an  authorization decision.  Usually this
//      authorization   decision  is  based  on  a  Boot  Object  Authorization
//      Certificate.  When there is none as yet configured in the platform, BIS
//      does a callback to a BIOS-supplied function that does the authorization
//      decision in a platform-specific way.  This module stores the pointer to
//      the  callback  function  and  takes  care of calling it on demand.  The
//      public  interface to this module consists of an initialization function
//      and a call-through function.
//
///////////////////////////////////////////////////////////////////////////////



#include <bis_priv.h>
#include <bisBaseCode.h>
//#include <efibis.h>



// TBD********* what are we really doing for asserts?
#if ! defined(BIS_ASSERT)
#if defined(DOING_BIS_ASSERT)
void
BisAssertDisplayer(
    unsigned char *  expression,
    unsigned char *  file,
    UINT32               line
    )
{
    // for now, this is just a convenient place to put a breakpoint since there
    // is no other good universal display capability.
    int foo;

    // Give  the debugger an executable code line to settle on while the formal
    // parameters are in context.
    foo = 0;
} // BisAssertDisplayer
#define BIS_ASSERT(expr)    \
if (! (expr) ) {            \
    BisAssertDisplayer(     \
        #expr,              \
        __FILE__,           \
        __LINE__            \
        );                  \
}
#else // not DOING_BIS_ASSERT
#define BIS_ASSERT(expr)
#endif // DOING_BIS_ASSERT
#endif // defined BIS_ASSERT


///////////////////////////////////////////////////////////////////////////////
//  Function Name: CallAuthorization
//
//  Description:
//      This function is the same as the AUTHORIZATION_FN function pointer type
//      defined  in  the  BIS  Developers  Guide  except  that it takes care of
//      filling  in the "passBack" parameter from a stored value and also takes
//      care  of  calling  the  BIOS-supplied  function  from a stored function
//      pointer.  Internal BIS functions that need to call the AUTHORIZATION_FN
//      should call through this procedure.
//
//      The   procedure   AuthorizationInit  must  be  called  to  do  one-time
//      initialization before calling this function.
//
//  Parameters:       
//      See BIS Developers Guide
//
//  Returns:       
//      See BIS Developers Guide
//
///////////////////////////////////////////////////////////////////////////////
BIS_STATUS
CallAuthorization(
    IN   UINT32           opCode,
    IN   BIS_DATA_PTR     credentials,
    IN   BIS_DATA_PTR     credentialsSigner,
    IN   BIS_DATA_PTR     dataObject,
    IN   BIS_DATA_PTR     reserved,
    OUT  BIS_BOOLEAN *    isAuthorized
    )
{
    EFI_STATUS                  to_return;
    BIS_DATA                    clean_data_object;
    BIS_DATA_PTR                clean_data_ptr;
    EFI_BIS_CALLING_FUNCTION    CallingFunction;
    BISBC_INSTANCEDATA 		    *instanceData=	NULL;   
    
    // check initialization consistency
    BIS_ASSERT(ExternalAuthorization != BIS_NULL);

    // check parameter consistency
    BIS_ASSERT(
        (opCode == BISOP_VerifyBootObject) ||
        (opCode == BISOP_UpdateBootObjectAuthorization));
    BIS_ASSERT(
        (credentials != BIS_NULL) &&
        (credentials->data != BIS_NULL) &&
        (credentials->length != 0));
    BIS_ASSERT(
        (credentialsSigner != BIS_NULL) &&
        (credentialsSigner->data != BIS_NULL) &&
        (credentialsSigner->length != 0));
    BIS_ASSERT(reserved == 0);
    BIS_ASSERT(isAuthorized != BIS_NULL);

    // Manufacture  a  safely structured data object pointer to pass through to
    // the external function.
    if (opCode == BISOP_VerifyBootObject) {
        BIS_ASSERT(dataObject != BIS_NULL);
        clean_data_object = * dataObject;
        if ((clean_data_object.data == BIS_NULL) ||
            (clean_data_object.length == 0)) {
            // Make  sure  it  has  a  safe  pointer and a zero length, because
            // callers  have  varying  ways  of  expressing  a zero-length data
            // object that may confuse an external function down the line.
            clean_data_object.data = (UINT8* ) & clean_data_object;
            clean_data_object.length = 0;
        }
        clean_data_ptr = & clean_data_object;
    }
    else {
        clean_data_ptr = BIS_NULL;
    }

    if (opCode == BISOP_VerifyBootObject)
    {
        CallingFunction = BisCallingFunctionVerify;
    }
    else
    {
        CallingFunction = BisCallingFunctionUpdate;
    }

    // call through to external function
    instanceData = getInstanceData( BISBC );

    to_return = instanceData->Authorize->CheckCredentials(
        instanceData->Authorize,                // This
        CallingFunction,                        // CallingFunction
        (EFI_BIS_DATA*)credentials,             // Credentials
        (EFI_BIS_DATA*)credentialsSigner,       // CredentialsSigner
        (EFI_BIS_DATA*)clean_data_ptr,          // DataObject
        0,                                      // Reserved
        (BOOLEAN*)isAuthorized                  // IsAuthorized
        );

    // Consistency check that the external function worked.
    BIS_ASSERT(EFI_SUCCESS == to_return);

    return mapEfiToBis(to_return);
    
} // CallAuthorization



