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

    initmem.c

Abstract:

    EFI Memory management initialization 



Revision History

--*/

#include "imem.h"

#define POOL_FREE_SIGNATURE     EFI_SIGNATURE_32('p','f','r','0')
typedef struct {
    UINT32          Signature;
    UINT32          Index;
    LIST_ENTRY      Link;
} POOL_FREE;


#define POOL_HEAD_SIGNATURE     EFI_SIGNATURE_16('p','h')
typedef struct {
    UINT16          Signature;
    INT32           Type;
    UINT8           Reserved;
    UINT32          Size;
    CHAR8           Data[1];
} POOL_HEAD;

#define SIZE_OF_POOL_HEAD EFI_FIELD_OFFSET(POOL_HEAD,Data)

#define POOL_TAIL_SIGNATURE     EFI_SIGNATURE_32('p','t','a','l')
typedef struct {
    UINT32          Signature;
    UINT32          Size;
} POOL_TAIL;


#define POOL_SHIFT  7

#define POOL_OVERHEAD   (SIZE_OF_POOL_HEAD + sizeof(POOL_TAIL))

#define HEAD_TO_TAIL(a)   \
    ((POOL_TAIL *) (((CHAR8 *) (a)) + (a)->Size - sizeof(POOL_TAIL)));


#define SIZE_TO_LIST(a)     ((a) >> POOL_SHIFT)
#define LIST_TO_SIZE(a)     ((a+1) << POOL_SHIFT)

#ifdef EFI64
#define DEFAULT_PAGE_ALLOCATION     (EFI_PAGE_SIZE * 2)
#else
#define DEFAULT_PAGE_ALLOCATION     (EFI_PAGE_SIZE)
#endif

#define MAX_POOL_LIST       SIZE_TO_LIST(DEFAULT_PAGE_ALLOCATION)

//
// Globals
//

#define POOL_SIGNATURE  EFI_SIGNATURE_32('p','l','s','t')
typedef struct {
    INTN            Signature;
    UINTN           Used;
    EFI_MEMORY_TYPE MemoryType;
    LIST_ENTRY      FreeList[MAX_POOL_LIST];
    LIST_ENTRY      Link;
} POOL; 


POOL        PoolHead[EfiMaxMemoryType];
LIST_ENTRY  PoolHeadList;

//
//
//

INTERNAL
VOID
InitializePool (
    VOID
    )
/*++

Routine Description:

    Called to initialize the pool

Arguments:

    none

Returns:

    none

--*/
{
    UINTN       Type;
    UINTN       Index;

    for (Type=0; Type < EfiMaxMemoryType; Type++) {
        PoolHead[Type].Signature  = 0;
        PoolHead[Type].Used       = 0;
        PoolHead[Type].MemoryType = Type;
        for (Index=0; Index < MAX_POOL_LIST; Index++) {
            InitializeListHead (&PoolHead[Type].FreeList[Index]);
        }
    }
    InitializeListHead (&PoolHeadList);
}

POOL *
LookupPoolHead (
  IN EFI_MEMORY_TYPE  MemoryType
  )

{
  LIST_ENTRY  *Link;
  POOL        *Pool;
  UINTN       Index;

  if (MemoryType >= 0 && MemoryType < EfiMaxMemoryType) {
    return &PoolHead[MemoryType];
  }

  if (MemoryType < 0) {

    for (Link = PoolHeadList.Flink; Link != &PoolHeadList; Link = Link->Flink) {
      Pool = CR(Link, POOL, Link, POOL_SIGNATURE);
      if (Pool->MemoryType == MemoryType) {
        return Pool;
      }
    }

    Pool = AllocatePoolI (EfiBootServicesData, sizeof (POOL));
    if (Pool == NULL) {
      return NULL;
    }

    Pool->Signature = POOL_SIGNATURE;
    Pool->Used      = 0;
    Pool->MemoryType = MemoryType;
    for (Index=0; Index < MAX_POOL_LIST; Index++) {
      InitializeListHead (&Pool->FreeList[Index]);
    }

    InsertHeadList (&PoolHeadList, &Pool->Link);

    return Pool;
  }

  return NULL;
}

EFI_STATUS
BOOTSERVICE
BootServiceAllocatePool (
    IN EFI_MEMORY_TYPE  PoolType,
    IN UINTN            Size,
    OUT VOID            **Buffer
    )
/*++

Routine Description:

    Allocate pool of a particular type.

Arguments:

    PoolType        - Type of pool to allocate

    Size            - The amount of pool to allocate

    Buffer          - The address to return a pointer to the allocated pool.

Returns:

    Status

--*/
{
    if (PoolType >= EfiMaxMemoryType && PoolType <= 0x7fffffff) {
        return EFI_INVALID_PARAMETER;
    }

    return BSAllocatePool(PoolType,Size,Buffer);
}


EFI_STATUS
BSAllocatePool (
    IN EFI_MEMORY_TYPE  PoolType,
    IN UINTN            Size,
    OUT VOID            **Buffer
    )
/*++

Routine Description:

    Allocate pool of a particular type.

Arguments:

    PoolType        - Type of pool to allocate

    Size            - The amount of pool to allocate

    Buffer          - The address to return a pointer to the allocated pool.

Returns:

    Status

--*/
{
    //
    // If we're at runtime, fail it
    //

    if (EfiAtRuntime) {
        return EFI_UNSUPPORTED;
    }
    
    //
    // If size is too large, fail it
    //

    if (Size > 0xffffff00) {
        *Buffer = NULL;
        return EFI_UNSUPPORTED;
    }

    //
    // Acquire the memory lock and make the allocation
    //

    AcquireLock (&MemoryLock);
    *Buffer = AllocatePoolI (PoolType, Size);
    ReleaseLock (&MemoryLock);
    return *Buffer ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}


INTERNAL
VOID *
AllocatePoolI (
    IN EFI_MEMORY_TYPE  PoolType,
    IN UINTN            Size
    )
/*++

Routine Description:

    Internal function to allocate pool of a particular type.

    N.B. Caller must have the memory lock held


Arguments:

    PoolType        - Type of pool to allocate

    Size            - The amount of pool to allocate

Returns:

    The allocate pool, or NULL

--*/
{
    POOL            *Pool;
    POOL_FREE       *Free;
    POOL_HEAD       *Head;
    POOL_TAIL       *Tail;
    CHAR8           *NewPage;
    VOID            *Buffer;
    UINTN            Index, FSize, offset, Adjustment;
    UINTN           NoPages;

    ASSERT_LOCKED(&MemoryLock);

    //
    // Adjust the size by the pool header & tailer overhead
    //
    
    //
    //  Adjusting the Size to be of proper alignment so that
    //  we don't get an unaligned access fault later when
    //  pool_Tail is being initialized
    //
    ALIGN_VARIABLE(Size, Adjustment);

    Size += POOL_OVERHEAD;
    Index = SIZE_TO_LIST(Size);
    Pool = LookupPoolHead (PoolType);
    if (Pool == NULL) {
      return NULL;
    }
    Head = NULL;

    //
    // If allocation is over max size, just allocate pages for the request
    // (slow)
    //

    if (Index >= MAX_POOL_LIST) {
        NoPages = EFI_SIZE_TO_PAGES(Size) + EFI_SIZE_TO_PAGES (DEFAULT_PAGE_ALLOCATION) - 1;
        NoPages &= ~(EFI_SIZE_TO_PAGES (DEFAULT_PAGE_ALLOCATION) - 1);
        Head = AllocatePoolPages (PoolType, NoPages, DEFAULT_PAGE_ALLOCATION);
        goto Done;
    }


    //
    // If there's no free pool in the proper list size, go get some more pages
    //

    if (IsListEmpty(&Pool->FreeList[Index])) {

        //
        // Get another page
        //

        NewPage = AllocatePoolPages(PoolType, EFI_SIZE_TO_PAGES (DEFAULT_PAGE_ALLOCATION), DEFAULT_PAGE_ALLOCATION);
        if (!NewPage) {
            goto Done;
        }

        //
        // Carve up new page into free pool blocks
        //

        offset = 0;
        while (offset < DEFAULT_PAGE_ALLOCATION) {
            ASSERT (Index < MAX_POOL_LIST);
            FSize = LIST_TO_SIZE(Index);

            while (offset + FSize <= DEFAULT_PAGE_ALLOCATION) {

                Free = (POOL_FREE *) &NewPage[offset];                
                Free->Signature = POOL_FREE_SIGNATURE;
                Free->Index     = (UINT32)Index;
                InsertHeadList (&Pool->FreeList[Index], &Free->Link);
                offset += FSize;
            }

            Index -= 1;
        }

        ASSERT(offset == DEFAULT_PAGE_ALLOCATION);
        Index = SIZE_TO_LIST(Size);
    }

    //
    // Remove entry from free pool list
    //

    Free = CR(Pool->FreeList[Index].Flink, POOL_FREE, Link, POOL_FREE_SIGNATURE);
    RemoveEntryList (&Free->Link);

    Head = (POOL_HEAD *) Free;

Done:
    Buffer = NULL;

    if (Head) {
        //
        // If we have a pool buffer, fill in the header & tail info
        //

        Head->Signature = POOL_HEAD_SIGNATURE;
        Head->Size = (UINT32) Size;
        Head->Type = (INT32) PoolType;
        Tail = HEAD_TO_TAIL(Head);
        Tail->Signature = POOL_TAIL_SIGNATURE;
        Tail->Size = (UINT32) Size;
        Buffer = Head->Data;
        DBGSETMEM (Buffer, Size - POOL_OVERHEAD);

        DEBUG((D_POOL, "AllcPool: Type %x, Addr %x (len %x) %,d\n", 
                            PoolType, 
                            Buffer, 
                            Size - POOL_OVERHEAD, 
                            Pool->Used
                            ));

        //
        // Account the allocation
        //

        Pool->Used += Size;

    } else {
        DEBUG((D_ERROR, "AllocatePool: failed to allocate %d bytes\n", Size));
    }

    return Buffer;
}
    

EFI_STATUS
BOOTSERVICE
BSFreePool (
    IN VOID            *Buffer
    )
/*++

Routine Description:

    Frees pool

Arguments:

    Buffer          - The allocated pool entry to free

Returns:

    Status

--*/
{
    EFI_STATUS Status;

    //
    // If we're at runtime, then return.
    //

    if (EfiAtRuntime) {
        return EFI_UNSUPPORTED;
    }

    AcquireLock (&MemoryLock);
    Status = FreePoolI (Buffer);
    ReleaseLock (&MemoryLock);
    return Status;
}

INTERNAL
EFI_STATUS
FreePoolI (
    IN VOID             *Buffer
    )
/*++

Routine Description:

    Internal function to free a pool entry.

    N.B. Caller must have the memory lock held


Arguments:

    Buffer          - The allocated pool entry to free

Returns:

    None

--*/
{
    POOL                *Pool;
    POOL_HEAD           *Head;
    POOL_TAIL           *Tail;
    POOL_FREE           *Free;
    UINTN                Index;
    UINTN                NoPages;
    UINTN                Size;
    CHAR8               *NewPage;
    UINTN                FSize;
    UINTN                offset;
    BOOLEAN              AllFree;

    //
    // Get the head & tail of the pool entry
    //

    if (Buffer==NULL){
	return EFI_INVALID_PARAMETER;
    }

    Head = CR(Buffer, POOL_HEAD, Data, POOL_HEAD_SIGNATURE);

    if (Head->Signature != POOL_HEAD_SIGNATURE) {
        return(EFI_INVALID_PARAMETER);
    }

    Tail = HEAD_TO_TAIL(Head);

    //
    // Debug
    //

    ASSERT (Tail->Signature == POOL_TAIL_SIGNATURE);
    ASSERT (Head->Size == Tail->Size);
    ASSERT_LOCKED (&MemoryLock);

    if (Tail->Signature != POOL_TAIL_SIGNATURE) {
        return(EFI_INVALID_PARAMETER);
    }

    if (Head->Size != Tail->Size) {
        return(EFI_INVALID_PARAMETER);
    }

    //
    // Determine the pool type and account for it
    //

    Size = Head->Size;
    Pool = LookupPoolHead (Head->Type);
    if (Pool == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    Pool->Used -= Size;
    DEBUG((D_POOL, "FreePool: %x (len %x) %,d\n", Head->Data, Head->Size - POOL_OVERHEAD, Pool->Used));

    //
    // Determine the pool list 
    //

    Index = SIZE_TO_LIST(Size);
    DBGSETMEM (Head, Size);

    //
    // If it's not on the list, it must be pool pages
    //

    if (Index >= MAX_POOL_LIST) {

        //
        // Return the memory pages back to free memory
        //

        NoPages = EFI_SIZE_TO_PAGES(Size) + EFI_SIZE_TO_PAGES (DEFAULT_PAGE_ALLOCATION) - 1;
        NoPages &= ~(EFI_SIZE_TO_PAGES (DEFAULT_PAGE_ALLOCATION) - 1);

        FreePoolPages ((EFI_PHYSICAL_ADDRESS) (UINTN)Head, NoPages);

    } else {

        //
        // Put the pool entry onto the free pool list
        //

        Free = (POOL_FREE *) Head;    
        Free->Signature = POOL_FREE_SIGNATURE;
        Free->Index     = (UINT32)Index;
        InsertHeadList (&Pool->FreeList[Index], &Free->Link);

        //
        // See if all the pool entries in the same page as Free are freed pool entries
        //

        NewPage = (CHAR8 *)((UINTN)Free & ~((DEFAULT_PAGE_ALLOCATION) -1));
        Free = (POOL_FREE *) &NewPage[0];                
        if (Free->Signature == POOL_FREE_SIGNATURE) {

            Index = Free->Index;

            AllFree = TRUE;
            offset = 0;
            while (offset < DEFAULT_PAGE_ALLOCATION) {
                FSize = LIST_TO_SIZE(Index);
                while (offset + FSize <= DEFAULT_PAGE_ALLOCATION) {
                    Free = (POOL_FREE *) &NewPage[offset];                
                    if (Free->Signature != POOL_FREE_SIGNATURE) {
                      AllFree = FALSE;
                    }
                    offset += FSize;
                }
                Index -= 1;
            }

            if (AllFree) {

                //
                // All of the pool entries in the same page as Free are free pool entries
                // Remove all of these pool entries from the free loop lists.
                //

                Free = (POOL_FREE *) &NewPage[0];                
                Index = Free->Index;
                offset = 0;
                while (offset < DEFAULT_PAGE_ALLOCATION) {
                    FSize = LIST_TO_SIZE(Index);
                    while (offset + FSize <= DEFAULT_PAGE_ALLOCATION) {
                        Free = (POOL_FREE *) &NewPage[offset];                
                        RemoveEntryList (&Free->Link);
                        offset += FSize;
                    }
                    Index -= 1;
                }

                //
                // Free the page
                //

                FreePoolPages ((EFI_PHYSICAL_ADDRESS)(UINTN) NewPage, EFI_SIZE_TO_PAGES (DEFAULT_PAGE_ALLOCATION));

            }
        }

    }

    //
    // If this is an OS specific memory type, then check to see if the last 
    // portion of that memory type has been freed.  If it has, then free the
    // list entry for that memory type
    //
    if (Pool->MemoryType < 0 && Pool->Used == 0) {
      RemoveEntryList (&Pool->Link);
      FreePoolI(Pool);
    }

    return EFI_SUCCESS;
}
