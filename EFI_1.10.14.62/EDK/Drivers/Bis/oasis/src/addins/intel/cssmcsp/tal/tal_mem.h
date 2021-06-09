/*-----------------------------------------------------------------------
 *      File:   tal_mem.h
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
 * This is the header file for TAL memory functions.
 */

#ifndef	_TAL_MEM_H
#define	_TAL_MEM_H

#include "cssmtype.h"

/****************************************************************************/
/* extern functions                                                         */
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern void*    TAL_Malloc(CSSM_HANDLE Handle, uint32 size);
extern void*    TAL_Calloc(CSSM_HANDLE Handle, uint32 num, uint32 size);
extern void*    TAL_Realloc(CSSM_HANDLE Handle, void* pBuf, uint32 size);
extern void	    TAL_Free(CSSM_HANDLE Handle, void* pBuf);

extern void*    TAL_AppMalloc(CSSM_HANDLE AddInHandle, uint32 size);
extern void*    TAL_AppCalloc(CSSM_HANDLE AddInHandle, uint32 num, uint32 size);
extern void*    TAL_AppRealloc(CSSM_HANDLE AddInHandle, void* pBuf, uint32 size);
extern void	    TAL_AppFree(CSSM_HANDLE AddInHandle, void* pBuf);
                            
#ifdef __cplusplus
}
#endif
/****************************************************************************/

#endif /*_TAL_MEM_H*/
