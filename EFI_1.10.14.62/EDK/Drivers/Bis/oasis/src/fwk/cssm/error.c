/*-----------------------------------------------------------------------------
 *      File:   error.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------------
 */
/*
 * INTEL CONFIDENTIAL 
 * This file, software, or program is supplied under the terms of a 
 * license agreement or nondisclosure agreement with Intel Corporation 
 * and may not be copied or disclosed except in accordance with the 
 * terms of that agreement. This file, software, or program contains 
 * copyrighted material and/or trade secret information of Intel 
 * Corporation, and must be treated as such. Intel reserves all rights 
 * in this material, except as the license agreement or nondisclosure 
 * agreement specifically indicate. 
 */ 
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software is subject to the U.S. Export Administration Regulations 
 * and other U.S. law, and may not be exported or re-exported to certain 
 * countries (currently Afghanistan (Taliban-controlled areas), Cuba, Iran, 
 * Iraq, Libya, North Korea, Serbia (except Kosovo), Sudan and Syria) or to 
 * persons or entities prohibited from receiving U.S. exports (including Denied 
 * Parties, Specially Designated Nationals, and entities on the Bureau of 
 * Export Administration Entity List or involved with missile technology or 
 * nuclear, chemical or biological weapons).
 */ 
/*
 * This file contains the functions that are contained in the Error portion
 * of the CSSM exported functions.
 */

#include "cssm.h"
#include "cssmport.h"
#include "internal.h"

extern CSSM_ERROR GlobalErr;

#define port_SetError CSSM_SetError

/*-----------------------------------------------------------------------------
 * Name: CSSM_GetError
 *
 * Description:  
 * Gets the currently set error information
 * 
 * Parameters: 
 * None
 *
 * Returns:
 * A pointer to the error information
 * NULL if CSSM_Init was never called 
 * 
 * Error Codes:
 * None available
 *---------------------------------------------------------------------------*/
CSSM_ERROR_PTR CSSMAPI CSSM_GetError(void)
{
    /* make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return NULL;

    return(&GlobalErr);
}

/*-----------------------------------------------------------------------------
 * Name: CSSM_SetError
 *
 * Description:  
 * Sets the error information
 * 
 * Parameters: 
 * guid (input)  : The GUID of the module setting the error
 * error (input) : The error code to set
 *
 * Returns:
 * CSSM_OK if the function was successful. 
 * CSSM_FAIL if an error condition occurred. 
 * 
 * Error Codes:
 * CSSM_INVALID_POINTER if the input guid is NULL
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_SetError(CSSM_GUID_PTR guid, uint32 error)
{
    /* make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return CSSM_FAIL;

    if (!guid)
    {
        GlobalErr.error = CSSM_INVALID_POINTER;
        cssm_memcpy(&GlobalErr.guid, &CssmGUID, sizeof(CSSM_GUID));
        return CSSM_FAIL;
    }

    GlobalErr.error = error;
    cssm_memcpy(&GlobalErr.guid, guid, sizeof(CSSM_GUID));    

    return CSSM_OK;
}

/*-----------------------------------------------------------------------------
 * Name: CSSM_ClearError
 *
 * Description:  
 * Clear the error information
 * 
 * Parameters: 
 * None
 *
 * Returns:
 * None
 *
 * Error Codes:
 * None available
 *---------------------------------------------------------------------------*/
void CSSMAPI CSSM_ClearError(void)
{
    /* make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return;

    GlobalErr.error = CSSM_OK;
    cssm_memset(&GlobalErr.guid, 0, sizeof(CSSM_GUID));
}
