/*-----------------------------------------------------------------------
 *      File:   tal_glob.c
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
 * This file contains the functions to handle global variables for the csp.
 */

#include "cssmport.h"
#include "tal_glob.h"
#include "tal_defs.h"

CSSM_API_MEMORY_FUNCS tal_mem_funcs = 
  {cssm_malloc, cssm_free, cssm_realloc, cssm_calloc, NULL};

CSSM_GUID        tal_guid = {0,0,0,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};
CSSM_GUID_PTR	 tal_guid_ptr = &tal_guid;

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                         CSSM API functions                               */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* Wrapper around CSSM_SetError */
CSSM_RETURN TAL_SetError(uint32 error)	/* per-thread error code */
{
    return CSSM_SetError(tal_guid_ptr, error);
}

/* Wrapper around CSSM_GetError */
CSSM_ERROR_PTR TAL_GetError()
{
    return (CSSM_ERROR_PTR)CSSM_GetError();
}

/* Wrapper around CSSM_ClearError */
void TAL_ClearError()
{
        CSSM_ClearError();
}

