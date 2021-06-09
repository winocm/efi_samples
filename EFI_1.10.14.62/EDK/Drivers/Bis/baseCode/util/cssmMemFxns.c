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

//


//************************************************************************************************//
//
//  cssmMemFxns.c -
//
//      This file contains functions that map CSSM sytle
//      memory functions into BIS memory functions.
//
//      void *malloc_cssm(uint32 Size, void *AllocRef);
//      void  free_cssm(void *MemPtr, void *AllocRef);
//      void *realloc_cssm(void *MemPtr, uint32 Size, void *AllocRef);
//      void *calloc_cssm(uint32 Num, uint32 Size, void *AllocRef);
//
//      "realloc_cssm" is implemented in terms of primative BIS mem
//      funcs as there is nothing analogous already in place.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bisBaseCode.h>

void UpdateCssmMemUsage(char *m, BIS_BOOLEAN alloc);

// ==================================================== //

void
*malloc_cssm(uint32 Size, void *AllocRef)
{
    UINT8 *m;

    //Size 0 should return a valid pointer for compatibility
    //with MS runtime. The cssm code depends on this. We set
    //the size to 4 to everything below us will be happy.
    if (Size == 0)
    {
        Size= 4;
    }

    //Check parms and bail if bogus.
    if (AllocRef==BIS_NULL)
    {
        #if (COMPILE_SELFTEST_CODE==1)
        if (BIS_FLAG(MMFLAGS,MM_CSSM_FUNCTS))
        {
            PUT_SX("malloc_cssm parm err. allocRef=", (UINT32)AllocRef);
            PUT_SDN(" size=", Size);
        }
        #endif

        return BIS_NULL;
    }


    //Call our function.
    m= MEM_malloc( (BIS_APPINFO *)AllocRef, Size );

    #if (COMPILE_SELFTEST_CODE==1)
    if (BIS_FLAG(MMFLAGS,MM_CSSM_FUNCTS))
    {
        PUT_SD("malloc_cssm size", Size);
        PUT_SXN("  @ ", (UINT32)m);
    }
    UpdateCssmMemUsage(m,BIS_TRUE);
    #endif

    return m;
}

// ==================================================== //

void
free_cssm(void *MemPtr, void *AllocRef)
{
    //Check parms and bail if bogus.
    if (MemPtr  ==BIS_NULL
    ||  AllocRef==BIS_NULL)
    {
        #if (COMPILE_SELFTEST_CODE==1)
        if (BIS_FLAG(MMFLAGS,MM_CSSM_FUNCTS))
        {
            PUT_SX("free_cssm parm err. AllocRef=", (UINT32)AllocRef);
            PUT_SXN(" MemPtr=", (UINT32)MemPtr);
        }
        #endif

        return;
    }


    #if (COMPILE_SELFTEST_CODE==1)
    UpdateCssmMemUsage(MemPtr, BIS_FALSE);
    #endif


    //Call our function.
    MEM_free( (BIS_APPINFO *)AllocRef, MemPtr );


    #if (COMPILE_SELFTEST_CODE==1)
    if (BIS_FLAG(MMFLAGS,MM_CSSM_FUNCTS))
    {
        PUT_SXN("free_cssm MemPtr=", (UINT32)MemPtr);
    }
    #endif

}

// ==================================================== //

void *
realloc_cssm(void *MemPtr, uint32 Size, void *AllocRef)
{
    char   *newMem;

    //Check parms and bail if bogus.
    if (MemPtr  ==BIS_NULL
    ||  AllocRef==BIS_NULL
    ||  Size    == 0)
    {
        return BIS_NULL;
    }


    //Call our function.
    newMem= MEM_malloc( (BIS_APPINFO *)AllocRef, Size );
    if (newMem == BIS_NULL){
        return NULL;
    }

    #if (COMPILE_SELFTEST_CODE==1)
    UpdateCssmMemUsage(newMem,BIS_TRUE);
    #endif

    //Copy old data to new area.
    //Note we are using NEW size here and will potentially copy memory
    //that doesnt belong to us. This is because we don't know how long
    //the original area is.
    EfiCopyMem( newMem, MemPtr, Size );

    #if (COMPILE_SELFTEST_CODE==1)
    UpdateCssmMemUsage(MemPtr,BIS_FALSE);
    #endif

    //Free Old area.
    MEM_free( (BIS_APPINFO *)AllocRef, MemPtr );


    return newMem;

}

// ==================================================== //
void *
calloc_cssm(uint32 Num, uint32 Size, void *AllocRef)
{
    UINT8 *mem;

    //Check parms and bail if bogus.
    if (Num     == 0
    ||  AllocRef== BIS_NULL
    ||  Size    == 0)
    {
        #if (COMPILE_SELFTEST_CODE==1)
        if (BIS_FLAG(MMFLAGS,MM_CSSM_FUNCTS))
        {
            PUT_SD("calloc_cssm parm error size=", Size);
            PUT_SD(" num=", Num);
            PUT_SXN("  allocRef=", (UINT32)AllocRef);
        }
        #endif

        return BIS_NULL;
    }

    //Call our function.
    mem= MEM_malloc( (BIS_APPINFO *)AllocRef, Size*Num );


    //Zero memory. CSSM depends on calloc doing it!
    if (mem!=NULL)
    {
		EfiZeroMem( mem, Size*Num );
    }

    #if (COMPILE_SELFTEST_CODE==1)
    if (BIS_FLAG(MMFLAGS,MM_CSSM_FUNCTS))
    {
        PUT_SD("calloc_cssm size=", Size);
        PUT_SXN("  @ ", (UINT32)mem);
    }
    UpdateCssmMemUsage(mem, BIS_TRUE);
    #endif

    return mem;
}


// eof

