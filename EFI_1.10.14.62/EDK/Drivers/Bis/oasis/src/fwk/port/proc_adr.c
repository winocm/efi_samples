/*-----------------------------------------------------------------------
 *      File:   proc_adr.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *
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

#include "cssm.h"
#include "cssmport.h"
#include "guids.h"

#define NUM_ADDINS       3

extern CSSM_RETURN CSSMAPI F77DDF91_FF0D_11d1_9CB7_00A0C9708162_AddInAuthenticate
                        (const char* cssmCredentialPath, const char* cssmSection,
                         const char* AppCredential, const char* AppSection);
extern CSSM_RETURN CSSMAPI F38A8023_BDB1_11d1_93C8_00A0C93C3211_AddInAuthenticate
                        (const char* cssmCredentialPath, const char* cssmSection,
                         const char* AppCredential, const char* AppSection);
extern CSSM_RETURN CSSMAPI B2CC8C10_F00B_11d1_878B_00A0C91A2629_AddInAuthenticate
                        (const char* cssmCredentialPath, const char* cssmSection,
                         const char* AppCredential, const char* AppSection);

static ADD_IN_AUTH_TBL AddInAuthPtrTbl[NUM_ADDINS] =
{
    {intel_preos_csm_bis_guid_def,
        F77DDF91_FF0D_11d1_9CB7_00A0C9708162_AddInAuthenticate},
    {intel_preos_x509v3_clm_guid_def,
        F38A8023_BDB1_11d1_93C8_00A0C93C3211_AddInAuthenticate},
    {intel_preos_SMv2_vl_guid_def,
        B2CC8C10_F00B_11d1_878B_00A0C91A2629_AddInAuthenticate}
};

/* Dynamic Library Loading routines */
/*-----------------------------------------------------------------------------
 * Name: cssm_GetAddinAuthenticateFuncPtr
 *
 * Description:  Returns the address of the specified function
 * 
 * Parameters: 
 * pGuid (input)    :  GUID of the addin to get the address for
 *
 * Returns:
 * Address of the specified function
 * NULL if an error occurs
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ADDIN_AUTH_FUNC_PTR cssm_GetAddinAuthenticateFuncPtr(CSSM_GUID_PTR Guid)
{
    sint32 x;
    for(x = 0; x < NUM_ADDINS; x++)
    {
        if (!cssm_memcmp(Guid,&AddInAuthPtrTbl[x].guid, sizeof(CSSM_GUID)))
        {
#pragma warning (disable:4054 4055)
            return (ADDIN_AUTH_FUNC_PTR)((void *) AddInAuthPtrTbl[x].procaddress);
#pragma warning (default:4054 4055)
        }
    }

    return NULL;
}
