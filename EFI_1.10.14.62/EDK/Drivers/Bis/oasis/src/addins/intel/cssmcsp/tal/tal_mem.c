/*-----------------------------------------------------------------------
 *      File:   tal_mem.c
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
 * This file contains the memory functions for the csp.
 */

#include "cssmport.h"
#include "tal_defs.h"
#include "tal_mem.h"
#include "tal_glob.h"

extern CSSM_SPI_MEMORY_FUNCS  csp_MemoryFunctions;
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                       Extern memory functions                            */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: TAL_Malloc, TAL_Calloc, TAL_Realloc, TAL_Free
 *
 *Description:
 * These functions decide which memory spacce to locate. If the Handle is 
 * CSP_HANDLE_SELF(0), than allocate the memory in its own space. If the Handle 
 * is not 0, than request CSSM to allocate the memory in the caller's area.
 *
 *Parameters: 
 * Handle(input)- Add-in Handles.
 *---------------------------------------------------------------------------*/
void* TAL_Malloc(CSSM_HANDLE Handle, uint32 size)
{
    if (Handle == CSP_HANDLE_SELF)
        return cssm_malloc(size, NULL);
    return TAL_AppMalloc(Handle, size);
}

void* TAL_Calloc(CSSM_HANDLE Handle, uint32 num, uint32 size)
{
    if (Handle == CSP_HANDLE_SELF)
        return cssm_calloc(num, size, NULL);
    return TAL_AppCalloc(Handle, num, size);
}

void* TAL_Realloc(CSSM_HANDLE Handle, void* pBuf, uint32 size)
{
    if (Handle == CSP_HANDLE_SELF)
        return cssm_realloc(pBuf, size, NULL);
    return TAL_AppRealloc(Handle, pBuf, size);
}

void TAL_Free(CSSM_HANDLE Handle, void* pBuf)
{
    if (Handle == CSP_HANDLE_SELF)
    {
        cssm_free(pBuf, NULL);
        return;
    }
    TAL_AppFree(Handle, pBuf);
}

/*----------------------------------------------------------------------
 *Name: TAL_AppMalloc(...)
 *
 *Description:
 *  Makes an upcall to CSSM to use application's malloc function.
 *
 *Parameters:
 *  AddInHandle(input) - CSSM managed CSP handle is used for up calls to 
 *                       memory functions
 *  size(input) - Bytes to allocate
 *
 *Returns:
 *    void*: the requisted space in caller's space.
 *    NULL:	 no space is allocated
 *----------------------------------------------------------------------*/
void* TAL_AppMalloc(CSSM_HANDLE AddInHandle, uint32 size)
{
    void *pBuf = NULL;
    pBuf = csp_MemoryFunctions.malloc_func(AddInHandle, size);
    return pBuf;
}

/*----------------------------------------------------------------------
 *Name: TAL_AppCalloc(...)
 *
 *Description:
 *  Makes an upcall to CSSM to use application's calloc function.
 *
 *Parameters:
 *  AddInHandle(input) - CSSM managed CSP handle is used for up calls to 
 *                       memory functions
 *  size(input) - Bytes to allocate
 *  num(input) - Length in bytes of each element
 *
 *Returns:
 *    void*: the requisted space in caller's space.
 *    NULL:	 no space is allocated
 *----------------------------------------------------------------------*/
void* TAL_AppCalloc(CSSM_HANDLE AddInHandle, uint32 num, uint32 size)
{
    void *pBuf = NULL;
    pBuf = csp_MemoryFunctions.calloc_func(AddInHandle, num, size);
    return pBuf;
}

/*----------------------------------------------------------------------
 *Name: TAL_AppRealloc(...)
 *
 *Description:
 *  Makes an upcall to CSSM to use application's Realloc function.
 *
 *Parameters:
 *  AddInHandle(input) - CSSM managed CSP handle is used for up calls to 
 *                       memory functions
 *  size(input) - New size in bytes
 *  pBuf(input) - Pointer to previously allocated memory block
 *
 *Returns:
 *    void*: the requisted space in caller's space.
 *    NULL:	 no space is allocated
 *----------------------------------------------------------------------*/
void* TAL_AppRealloc(CSSM_HANDLE AddInHandle, void* pBuf, uint32 size)
{
    void *pBuf1 = NULL;
    if (pBuf)
        pBuf1 = csp_MemoryFunctions.realloc_func(AddInHandle, pBuf, size);
    else
        pBuf1 = csp_MemoryFunctions.realloc_func(AddInHandle, pBuf1, size);
    
    return pBuf1;
}

/*----------------------------------------------------------------------
 *Name: TAL_AppFree(...)
 *
 *Description:
 *  Makes an upcall to CSSM to use application's memory free function.
 *
 *Parameters:
 *  AddInHandle(input) - CSSM managed CSP handle is used for up calls to 
 *                       memory functions
 *  pBuf(input) - Previously allocated memory block to be freed
 *
 *Returns: none
 *----------------------------------------------------------------------*/
void TAL_AppFree(CSSM_HANDLE AddInHandle, void* pBuf)
{
    csp_MemoryFunctions.free_func(AddInHandle, pBuf);
}

/****************************************************************************/
