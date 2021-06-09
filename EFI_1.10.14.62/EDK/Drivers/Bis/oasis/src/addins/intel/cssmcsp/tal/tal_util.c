/*-----------------------------------------------------------------------
 *      File:   tal_util.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------
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
 * This file is part of the Token Adaptation Layer (TAL) source code base.
 * The TAL code makes it easier for CSP venders to develop CSPs that plug
 * into the Intel CDSA infrastructure.
 * This file contains the token adaptation layer utility functions.
 */

#include "cssmerr.h"
#include "tal_util.h"
#include "tal_glob.h"
#include "tal_mem.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Output buffer functions                         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*----------------------------------------------------------------------------
 *Name: TAL_MakeOutBuffer
 *
 *Description:
 * This function makes sure the output buffer exists and is big enough.
 * Allocates space in CSPHandle area, if the output buffer does not exist, 
 *
 *Parameters: 
 * CSPHandle(input)- CSSM managed CSP handle is used for up calls to memory 
 *                   functions
 * OutData_ptr(output) - Pointer to the output buffer
 * NeedSize(input) - Requested size of the output buffer
 *
 *Returns:
 * CSSM_OK - call is successful
 * CSSM_FAIL - call failed
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_INVALID_DATA_POINTER
 *	 CSSM_CSP_MEMORY_ERROR
 *   CSSM_CSP_ERR_OUTBUF_LENGTH
 *--------------------------------------------------------------------------*/
CSSM_RETURN	TAL_MakeOutBuffer(CSSM_CSP_HANDLE CSPHandle,
                              CSSM_DATA_PTR OutData_ptr,
							  uint32 NeedSize)
{
    if (OutData_ptr == NULL) 
    {
        TAL_SetError(CSSM_CSP_INVALID_DATA_POINTER);
        return CSSM_FAIL;
    }
    if (NeedSize == 0)
        return CSSM_OK;

    if (OutData_ptr->Data == NULL)
    {
        OutData_ptr->Data = (uint8*)TAL_Malloc(CSPHandle, NeedSize);
        if (OutData_ptr->Data == NULL)
        {
            TAL_SetError(CSSM_CSP_MEMORY_ERROR);
            return CSSM_FAIL;
        }
        OutData_ptr->Length = NeedSize;
        return CSSM_OK;
    }
    else if (OutData_ptr->Length < NeedSize)
    {
        TAL_SetError(CSSM_CSP_ERR_OUTBUF_LENGTH);
        return CSSM_FAIL;
    }
    return CSSM_OK;
}

/*----------------------------------------------------------------------------
 *Name: TAL_ValidateInDataParam
 *
 *Description:
 * This function verifies the input parameters. The following are the rules.
 *
 *      InBufs_ptr->Length    InBufs_ptr->Data	
 *      ------------------------------------------------------------------
 *           0              NULL            0 length input
 *           0              NOT NULL        0 length input
 *          >0              NULL            Error case
 *          >0              NOT NULL        Pass in buf to crypto function
 *
 *Parameters: 
 * InBufs_ptr(input) - Input buffers
 * InBufCount(input) - Input buffer count
 *
 *Returns:
 * CSSM_OK - call is successful
 * CSSM_FAIL - call failed
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_INVALID_DATA_COUNT
 *	 CSSM_CSP_INVALID_DATA_POINTER
 *   CSSM_CSP_INVALID_DATA
 *--------------------------------------------------------------------------*/
CSSM_RETURN TAL_ValidateInDataParam(const CSSM_DATA_PTR InBufs_ptr,
                                    uint32 InBufCount)
{
    uint32          i = 0;
    CSSM_DATA_PTR	pDataBuf = InBufs_ptr;

    if (InBufCount == 0)
    {
        TAL_SetError(CSSM_CSP_INVALID_DATA_COUNT);
        return CSSM_FAIL;
    }
    for (i=0; i<InBufCount; i++)
    {
        if (pDataBuf == NULL)
        {
            TAL_SetError(CSSM_CSP_INVALID_DATA_POINTER);
            return CSSM_FAIL;
        }
        if ((pDataBuf->Length != 0) && (pDataBuf->Data == NULL))
        {
            TAL_SetError(CSSM_CSP_INVALID_DATA);
            return CSSM_FAIL;
        }
        pDataBuf++; 
    }
    return CSSM_OK;
}

/*----------------------------------------------------------------------------
 *Name: TAL_ValidateOutDataParam
 *
 *Description:
 * This function verifies the output parameters. The following are the rules.
 *
 *      OutBufs_ptr->Length    OutBufs_ptr->Data                                    
 *      ----------------------------------------------------------------------      
 *           0              NULL            Allocate return buf and set size  
 *           0              NOT NULL        Error case                        
 *          >0              NULL            Error case                        
 *          >0              NOT NULL        Pass out buf to crypto function   
 *
 *Parameters: 
 * OutBufs_ptr (input) - output buffers
 * OutBufCount (input) - output buffer count
 *
 *Returns:
 * CSSM_OK - call is successful
 * CSSM_FAIL - call failed
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_INVALID_DATA_COUNT
 *	 CSSM_CSP_INVALID_DATA_POINTER
 *   CSSM_CSP_INVALID_DATA
 *--------------------------------------------------------------------------*/
CSSM_RETURN TAL_ValidateOutDataParam(const CSSM_DATA_PTR OutBufs_ptr,
                                     uint32 OutBufCount)
{
    uint32          i = 0;
    CSSM_DATA_PTR	pDataBuf = OutBufs_ptr;

    if (OutBufCount == 0)
    {
        TAL_SetError(CSSM_CSP_INVALID_DATA_COUNT);
        return CSSM_FAIL;
    }
    for (i=0; i<OutBufCount; i++)
    {
        if (pDataBuf == NULL)
        {
            TAL_SetError(CSSM_CSP_INVALID_DATA_POINTER);
            return CSSM_FAIL;
        }
        if (((pDataBuf->Length == 0) && (pDataBuf->Data != NULL)) ||
            ((pDataBuf->Length != 0) && (pDataBuf->Data == NULL)))
        {
            TAL_SetError(CSSM_CSP_INVALID_DATA);
            return CSSM_FAIL;
        }
        pDataBuf++; 
    }
    return CSSM_OK;
}
/*----------------------------------------------------------------------------
 *Name: TAL_ValidateInKeyParam
 *
 *Description:
 * This function verifies the input key parameter. The following are the rules.
 *
 *      Key_ptr->KeyData.	Key_ptr->KeyData.                                                        
 *      KeyBlobLength       KeyBlob                                   
 *	----------------------------------------------------------------------      
 *           0              NULL            Allocate return buf and set size  
 *           0              NOT NULL        Error case                        
 *          >0              NULL            Error case                        
 *          >0              NOT NULL        Pass out buf to crypto function   
 *
 *Parameters: 
 * Key_ptr(input) - key parameter
 *
 *Returns:
 * CSSM_OK - call is successful
 * CSSM_FAIL - call failed
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_INVALID_KEY_POINTER
 *	 CSSM_CSP_INVALID_KEY
 *--------------------------------------------------------------------------*/
CSSM_RETURN TAL_ValidateInKeyParam(const CSSM_KEY_PTR Key_ptr)
{
    if (Key_ptr == NULL)
    {
        TAL_SetError(CSSM_CSP_INVALID_KEY_POINTER);
        return CSSM_FAIL;
    }
    if ((Key_ptr->KeyData.Length == 0) || (Key_ptr->KeyData.Data == NULL) )
    {
        TAL_SetError(CSSM_CSP_INVALID_KEY);
        return CSSM_FAIL;
    }
    return CSSM_OK;
}

