/*-----------------------------------------------------------------------
 *      File:   heap.c
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

extern CSSM_API_MEMORY_FUNCS CssmMemFuncs;

void * cssm_malloc (uint32  mem_size, void* allocRef)
{ allocRef; return CssmMemFuncs.malloc_func(mem_size, CssmMemFuncs.AllocRef); }

void * cssm_calloc (uint32  num_elem, uint32  num_bytes, void* allocRef)
{ allocRef; return CssmMemFuncs.calloc_func(num_elem, num_bytes, CssmMemFuncs.AllocRef); }

void cssm_free (void *mem_ptr, void* allocRef)
{ allocRef; CssmMemFuncs.free_func(mem_ptr, CssmMemFuncs.AllocRef); }

void * cssm_realloc (void *old_ptr, uint32  num_bytes, void* allocRef)
{ allocRef; return CssmMemFuncs.realloc_func(old_ptr, num_bytes, CssmMemFuncs.AllocRef); }



#if 0
/* Portable heap for OASIS */
#if defined (_DEBUG)
#define DEBUGPADSIZE 4
static uint8 _PadFill    = 0xFD;   /* fill outer boundries with this */
static uint8 _DeadFill   = 0xDD;   /* fill free objects with this */
static uint8 _CleanFill  = 0xCD;   /* fill new objects with this */
#endif

#define HEAP_AVAILABLE		 0x0001
#define HEAP_ALLOCATED		 0x0002
#define HEAP_DELETED		 0x0004
#define HEAP_RECLAIMED       0x0008

#define HEAP_GRANULARITY     0x0010  /* Memory allocation granularity */
#define HEAP_GRANSHIFT       0x0004  /* used inplace of Mults and Divs */
#define HEAP_NOLINK          0xFFFFFF

typedef struct key_HEAPHEADER
{
    uint32				Flags    :  8; /* Allocation flags      */
    uint32				FreeLink : 24; /* in granularity terms  */
    
    uint32              Tailsize :  8; /* in bytes              */
    uint32				Size     : 24; /* in granularity terms  */

#if defined (_DEBUG)
	uint8                  pad[DEBUGPADSIZE];
#endif
	/* Followed by your actual memory */
	/* followed by another padding section in debug */
} _HEAPHEADER;

typedef struct key_HEAP_REF
{
    uint32       HeapSize;
    _HEAPHEADER *FirstBlock;
    _HEAPHEADER *FirstFree;
    _HEAPHEADER *HeapBottom;
    sint32        AllocCount;
    sint32        FailOnCount;
    uint32        HighWater;
    uint32        CurrentAlloc;
} _HEAP_REF;

#define DATABLOCK(Block) ((unsigned char *)((_HEAPHEADER *)Block + 1))
#define HEADERBLOCK(Data) (((_HEAPHEADER *)Data)-1)
void cssm_heap_terminate(void *allocRef);

/*-----------------------------------------------------------------------------
 * Global heap reference for the default case
 *---------------------------------------------------------------------------*/
static _HEAP_REF *gpHeap = (_HEAP_REF *)NULL; 

/*-----------------------------------------------------------------------------
 * Name: cssm_malloc
 *
 * Description: Allocates memory on the heap. 
 * 
 * Parameters: 
 *		mem_size : Size of memory requested
 *		allocRef : Reference to a heap (See cssm_heap_init)
 *
 * Returns:
 *      Pointer to allocated block is Successful
 *      NULL if a block of mem_size size is not available
 *
 * Notes : if _DEBUG is defined the malloc routine initializes memory
 *         with CD (a big odd number) in the data area and pads a guard zone
 *         with FD (a big odd number) both before and after the data area.
 *         cssm_free checks the guard zone to see if is has been destroyed
 *         in which case an INT 3 is executed to break in the debugger.
 *         if _DEBUG is not defined there is NO guard zone. Only the
 *         heaps intrinsic granularity buffers memory area. That means if
 *         you hit a guard zone in debug mode you may destroy your heap in
 *         non debug mode.  Don't do that;
 * Error Codes: None.
 *---------------------------------------------------------------------------*/
void * cssm_malloc (uint32  mem_size, void* allocRef)
{
    _HEAPHEADER *RetBlock = (_HEAPHEADER *)NULL;    /* Define the Return block  */
    _HEAPHEADER *RemBlock = (_HEAPHEADER *)NULL;    /* Define the remaining blk */
    _HEAPHEADER *PrevBlock = (_HEAPHEADER *)NULL;   /* Define the remaining blk */
    _HEAP_REF   *HeapRef = (_HEAP_REF *)NULL; 
    uint32      GranAdjustedSize = 0;               /* Granularity size         */

    HeapRef = (allocRef == NULL) ? gpHeap : (_HEAP_REF *)allocRef;    

    if (HeapRef != NULL && mem_size > 0)            /* filter bogus calls       */
    {                                              

        GranAdjustedSize = GranAdjustSize(mem_size);

#if defined (TEST_ALLOW_ALLOC_FAIL)
        HeapRef->AllocCount++;
        HeapRef->CurrentAlloc += GranAdjustedSize;
        HeapRef->HighWater = (HeapRef->HighWater <  HeapRef->CurrentAlloc) ? HeapRef->CurrentAlloc : HeapRef->HighWater;

        if ((HeapRef->FailOnCount < 0 && HeapRef->AllocCount == -HeapRef->FailOnCount ) ||
            (HeapRef->FailOnCount > 0 && HeapRef->AllocCount >= HeapRef->FailOnCount))
        {
            return NULL;
        }
#endif

        /* Starting at first free block find a block to satisfy request */
        
        RetBlock = HeapRef->FirstFree;              /* start with FREE block    */
        while ( RetBlock != NULL &&                 /* Have we passed the heap? */
                (RetBlock->Size << HEAP_GRANSHIFT) < GranAdjustedSize ) 
        {
            /* hang on the the previous block for linking */
            PrevBlock = RetBlock;
            if (RetBlock->FreeLink > 0)             /* try next block           */
                RetBlock = (_HEAPHEADER *)((uint8 *)HeapRef + 
                                           (RetBlock->FreeLink << HEAP_GRANSHIFT)); 
            else
                RetBlock = NULL;                    /* found the bottom         */
        }
        
        if (RetBlock != NULL)        /* if we got a block we must initialize it */
        {
            uint32 RemainingBlock;
            uint32 MinBlockLeft;
            uint32 PostPad;

            /* determine if we want to split the block or leave it alone */
            MinBlockLeft = ((sizeof(_HEAPHEADER) + HEAP_GRANULARITY
#           if defined (_DEBUG)                     
                                    + DEBUGPADSIZE
#           endif                
                                   ) >> HEAP_GRANSHIFT) + 1;

            RemainingBlock = RetBlock->Size  - (GranAdjustedSize >> HEAP_GRANSHIFT);
            RetBlock->Flags = HEAP_ALLOCATED;   /* Mark as allocated */
            if (RemainingBlock > MinBlockLeft)
            {   
                /* divide the block in two blocks */
                RemBlock = (_HEAPHEADER *)((uint8 *)RetBlock + GranAdjustedSize);
                RemBlock->Size = RemainingBlock;        /* set the size         */
                RemBlock->Flags = HEAP_AVAILABLE;       /* Mark as available    */
                RemBlock->FreeLink = RetBlock->FreeLink;/* Link Free space      */
                RetBlock->FreeLink = 0;                 /* Unlink free block    */
                RetBlock->Size = GranAdjustedSize >> HEAP_GRANSHIFT;/* adust return blk */

                if (RetBlock == HeapRef->FirstFree)     /* Was this the first free block*/
                    HeapRef->FirstFree = RemBlock;      /* point to remaining block     */
                else if (PrevBlock != NULL)             /* then adjust the free link    */
                {
                    PrevBlock->FreeLink = ((uint8 *)RemBlock - (uint8 *)HeapRef ) >> HEAP_GRANSHIFT;
                }
            }
            else
            {   /* use the whole block as is, don't split it */
                if (RetBlock == HeapRef->FirstFree)
                {
                    HeapRef->FirstFree = (_HEAPHEADER *)((RetBlock->FreeLink != 0) ? 
                        ((uint8 *)HeapRef + (RetBlock->FreeLink << HEAP_GRANSHIFT)) :
                        NULL);
                }
                else if (PrevBlock != NULL)
                {
                    PrevBlock->FreeLink = RetBlock->FreeLink;
                    RetBlock->FreeLink  = 0;
                }
            }

            /* calculate the padding area beyond the data up to the granularity */
            PostPad = (RetBlock->Size << HEAP_GRANSHIFT)-(mem_size + sizeof(_HEAPHEADER));
            RetBlock->Tailsize = PostPad;
#           if defined (_DEBUG)
            {
                uint8 *pPad;
                uint32 x; 
                pPad = RetBlock->pad;
                for (x = 0; x < DEBUGPADSIZE; x++)
                {
                    *pPad++    = _PadFill;             /* init guard zone */
                }
                for (x = 0; x < mem_size; x++)
                {
                    *pPad++    = _CleanFill;           /* initialize to something big and odd */
                }
                for (x = 0; x < PostPad; x++)
                {
                    *pPad++    = _PadFill;             /* init postpad guard zone */
                }
            }
#           endif

        }
    }   
    return (RetBlock != NULL) ? DATABLOCK(RetBlock) : NULL;
}

/*-----------------------------------------------------------------------------
 * Name: cssm_calloc
 *
 * Description:  Allocates a memory block of the requested size and
 *               initializes all elements to zero
 * 
 * Parameters: 
 * num_elem (input)       : Number of elements to allocate
 * num_bytes (input)      : Size of each element to allocate
 * allocRef               : Reference to the heap in which memory is allocated
 *
 * Returns:
 * Pointer to allocated memory. 
 * NULL if an error condition occurred. 
 * 
 * Error Codes:
 * CSSM_CALLOC_FAILED
 *---------------------------------------------------------------------------*/

void * cssm_calloc (uint32  num_elem, uint32  num_bytes, void* allocRef)
{
    void *ptr = NULL;                           /* Return pointer */
    uint32 totalmemory = num_elem * num_bytes;  /* memory to allocate and initialize */
    uint32 *Temp1;                              /* init a DWORD at a time pointer */
    uint8 *Temp2;                               /* init the last few bytes      */
    uint32 x;                                   /* index counter */

    ptr = cssm_malloc(totalmemory,allocRef);    /* allocate the memory */
    if (ptr != NULL)                            /* got the memory? */
    {
        Temp1 = (uint32 *)ptr;                  /* Init using 32bits at a time */
        for (x = 0; x < (totalmemory >> 2); x++)  
        {
            *Temp1++ = 0;
        }
        Temp2 = (uint8 *) Temp1;                /* finish using 8 bits */
        for (x = 0; x < (totalmemory & 0x03); x++)
        {
            *Temp2++ = 0;
        }

    }
    return(ptr);
}

/*-----------------------------------------------------------------------------
 * Name: cssm_free
 *
 * Description:  Frees an allocated memory block
 * 
 * Parameters: 
 * mem_ptr (input)       : Pointer to allocated memory
 * allocRef (input)      : Reference to the heap where the memory was allocated
 *
 * Returns: void
 * 
 * Notes : if _DEBUG is defined the malloc routine initializes memory
 *         with CD (a big odd number) in the data area and pads a guard zone
 *         with FD (a big odd number) both before and after the data area.
 *         cssm_free checks the guard zone to see if is has been destroyed
 *         in which case an INT 3 is executed to break in the debugger.
 *         if _DEBUG is not defined there is NO guard zone. Only the
 *         heaps intrinsic granularity buffers memory area. That means if
 *         you hit a guard zone in debug mode you may destroy your heap in
 *         non debug mode.  Don't do that;
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/

void cssm_free (void *mem_ptr, void* allocRef)
{
    _HEAP_REF *HeapRef = (_HEAP_REF *)NULL; 
    _HEAPHEADER *hdr = HEADERBLOCK(mem_ptr);

    HeapRef = (allocRef == NULL) ? gpHeap : (_HEAP_REF *)allocRef;    

    if (HeapRef != NULL &&                          /* Heap init was done */
        !(mem_ptr == 0 || (uint32)mem_ptr & 1) &&   /* likely to be legal */
        (hdr->Flags & HEAP_ALLOCATED))              /* was allocated      */
    {
#       if defined (_DEBUG)
        {
            _HEAPHEADER *SearchBlock = (_HEAPHEADER *)NULL;   /* Define the remaining blk */
            uint8 *postpad;
            uint8 *pad;
            uint32 x;
            uint32	AlignFirstBlock;

            /* Walk the real heap to find the pointer before validation   */
            /* This prevents someone from screwing up the heap using free */
            /* but only during debug mode */
         	AlignFirstBlock = (uint32)((uint8 *)HeapRef + sizeof(_HEAP_REF));
            AlignFirstBlock =  (AlignFirstBlock % HEAP_GRANULARITY) ? 
                            HEAP_GRANULARITY - AlignFirstBlock % HEAP_GRANULARITY :
                            0;
    
            SearchBlock = (_HEAPHEADER *) ((uint8 *)HeapRef + sizeof(_HEAP_REF) + 
                                                    AlignFirstBlock);
           
            while (SearchBlock != hdr && 
                   SearchBlock != NULL && 
                   SearchBlock < HeapRef->HeapBottom && 
                   SearchBlock->Size != 0)
            {
                SearchBlock = (_HEAPHEADER *)((uint8 *)SearchBlock + SearchBlock->Size * HEAP_GRANULARITY);
            }
            
            if (SearchBlock != hdr)
            {   /* the memory your are freeing is not a valid block in the heap */
                /* if you get here someone is doing something bad               */
                
                
                // There will be no asm code in the efi implementation..
                //_asm int 3; /* Debug Break: I couldn't find this in the heap anywhere */
                return;  

            }
            else
            {
                /* Check the guard zone for pointers writing past there area                */
                postpad = (uint8 *)hdr + (hdr->Size << HEAP_GRANSHIFT) - (hdr->Tailsize);
                pad = hdr->pad;
                for (x = 0; x < DEBUGPADSIZE; x++)
                {
                    if (*pad != _PadFill || *postpad != _PadFill)
                    {
                        // There will be no asm code in the efi implementation
                        
                        //_asm int 3; /*  Debug Break: Someone wrote into the area before the data BAD EVIL SOMEONE */
                        return;
                    }
                    pad++;
                    postpad++;
                }
                /* Fill memory with odd garbage */
                for (x = 0; x < (hdr->Size << HEAP_GRANSHIFT) - sizeof(_HEAPHEADER); x++)
                {
                    *pad++ = _DeadFill;
                }

            }
        }
#       endif
        hdr->Flags = HEAP_DELETED | HEAP_AVAILABLE;
#if defined (TEST_ALLOW_ALLOC_FAIL)
        HeapRef->CurrentAlloc -= hdr->Size  << HEAP_GRANSHIFT;
#endif
        _HeapReclaim(hdr,allocRef);                     /* Defragment if necessary */
    }
#   if defined (_DEBUG)
    /* this causes the system to break when freeing invalid memory */
    /* you will hit this point if you free NULL, an odd address or */
    /* a non-allocated heap entry. This happens only in debug      */
    else
    {
#   if defined (_CATCH_NULL_FREE)
#   else
        if (mem_ptr != 0)               /* this prevents NULL frees from hitting the break */
#   endif
            // There will be no asm code in the efi implementation
            //_asm int 3;
    }
#endif

}

/*-----------------------------------------------------------------------------
 * Name: cssm_realloc
 *
 * Description:  Re-allocates a memory block of the requested size
 * 
 * Parameters: 
 * old_ptr (input)        : Pointer to old memory block
 * num_bytes (input)      : Size of memory block to allocate
 * allocRef (input)       : Reference to heap where memory to be reallocated
 *                          is currently allocated
 *
 * Returns:
 * Pointer to allocated memory. 
 * NULL if an error condition occurred or memory was freed, see comments. 
 * 
 * Error Codes:
 * CSSM_REALLOC_FAILED
 *
 * Comments: 
 *  If the pointer passed in is NULL, cssm_realloc behaves like cssm_malloc. 
 *  The return value is NULL if the size is zero and the buffer argument is 
 *  not NULL, or if there is not enough available memory to expand the block 
 *  to the given size. In the first case, the original block is freed. In the 
 *  second, the original block is unchanged.
 *---------------------------------------------------------------------------*/
void * cssm_realloc (void *old_ptr, uint32  num_bytes, void* allocRef)
{
    void *new_ptr = NULL;
    if (old_ptr == NULL)    /* This is just a malloc call */
    {
        new_ptr = cssm_malloc(num_bytes,allocRef);
    }
    else
    {
        _HEAP_REF *HeapRef = (_HEAP_REF *)NULL;
        _HEAPHEADER *hdr;
        uint32 OldBlockSize;
        uint32 NewBlockSize;

        NewBlockSize = GranAdjustSize(num_bytes);   /* calc new size */
        hdr = HEADERBLOCK(old_ptr);                 /* point to header */
        OldBlockSize = (hdr->Size << HEAP_GRANSHIFT);
    
        HeapRef = (allocRef == NULL) ? gpHeap : (_HEAP_REF *)allocRef;    

        /* Realloc Same */
        if (NewBlockSize == OldBlockSize)
        {
            new_ptr = old_ptr;
#           if defined (_DEBUG)             /* initialize new data area */
            {                               /* The memsize may be different even though */
                uint8 *PostPadData;         /* the block size is the same */
                uint32 PostPad;
                uint32 x;

                PostPadData = (uint8 *)hdr + (hdr->Size << HEAP_GRANSHIFT);
                PostPad = (hdr->Size << HEAP_GRANSHIFT)-(num_bytes + sizeof(_HEAPHEADER));
                if (hdr->Tailsize <= PostPad)       /* Alloc getting smaller or the same*/
                {
                    PostPadData -= PostPad;                         /* point to padding */
                    for (x = 0; x < PostPad; x++)
                    {
                        *PostPadData++    = _PadFill;               /* init postpad area*/
                    }
                }
                else
                {
                    PostPadData -= hdr->Tailsize;                   /* point to New Mem */
                    for (x = 0; x < hdr->Tailsize - PostPad; x++)
                    {
                        *PostPadData++    = _CleanFill;             /* init new memory  */
                    }

                    for (x = 0; x < PostPad; x++)
                    {
                        *PostPadData++    = _PadFill;              /* init postpad area */
                    }

                }

                hdr->Tailsize = PostPad;;
            }
#           endif
        } 
        /* Realloc Bigger */
        else if (NewBlockSize > OldBlockSize)
        {
            new_ptr = cssm_malloc(num_bytes, allocRef);
            if (new_ptr != NULL)
            {
                cssm_memcpy(new_ptr,old_ptr,OldBlockSize - (sizeof(_HEAPHEADER) + hdr->Tailsize));
                cssm_free(old_ptr,allocRef);
            }
        }
        /* Realloc Smaller */
        else if (NewBlockSize < OldBlockSize)
        {
            new_ptr = cssm_malloc(num_bytes, allocRef);
            if (new_ptr != NULL)
            {
                cssm_memcpy(new_ptr,old_ptr,num_bytes);
                cssm_free(old_ptr,allocRef);
            }
        }

    }
    return (new_ptr);
}

#endif
