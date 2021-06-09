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

Abstract:




Revision History

--*/

#include "imem.h"

#define EFI_DEFAULT_PAGE_ALLOCATION_ALIGNMENT  (EFI_PAGE_SIZE)

#ifdef EFI64
#define EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT  (EFI_PAGE_SIZE * 2)
#else
#define EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT  (EFI_PAGE_SIZE)
#endif

//
// MemoryMap - the curernt memory map
//

UINTN           MemoryMapKey = 0;

//
// MapStack - space to use as temp storage to build new map descriptors
// MapDepth - depth of new descriptor stack
//

#define MAX_MAP_DEPTH   6
UINTN           MapDepth = 0;
MEMORY_MAP      MapStack[MAX_MAP_DEPTH];
UINTN           FreeMapStack = 0;

//
// Watermarks to cooalese like memory types (IA-32 values)
//
#define EFI_RUNTIME_SERVICES_CODE_RESERVE (0x010000 * sizeof(UINTN))
#define EFI_RUNTIME_SERVICES_DATA_RESERVE (0x030000 * sizeof(UINTN))
#define EFI_BOOT_SERVICES_CODE_RESERVE    (0x080000 * sizeof(UINTN))
#define EFI_BOOT_SERVICES_DATA_RESERVE    (0x140000 * sizeof(UINTN))

UINT64 EfiWatermarkRuntimeServicesCode = MAX_ADDRESS;
UINT64 EfiWatermarkRuntimeServicesData = MAX_ADDRESS;
UINT64 EfiWatermarkBootServicesCode    = MAX_ADDRESS;
UINT64 EfiWatermarkBootServicesData    = MAX_ADDRESS;
UINT64 EfiWatermarkDefault             = MAX_ADDRESS;

//
// Internal prototypes
//

STATIC
VOID
AddRange (
    IN EFI_MEMORY_TYPE              Type,
    IN EFI_PHYSICAL_ADDRESS         Start,
    IN EFI_PHYSICAL_ADDRESS         End,
    IN UINT64                       Attribute
    );

    
STATIC
VOID
FreeMemoryMapStack (
    VOID
    );

STATIC
EFI_STATUS
ConvertPages (
    IN BOOLEAN              LoadConvert,
    IN UINT64               Start,
    IN UINT64               NoPages,
    IN EFI_MEMORY_TYPE      NewType
    );

STATIC
UINT64
FindFreePages (
    IN UINT64               MaxAddress,
    IN UINT64               NoPages,
    IN EFI_MEMORY_TYPE      NewType,
    IN UINTN                Alignment
    );

STATIC
VOID
RemoveMemoryMapEntry (
    MEMORY_MAP          *Entry
    );

//
//
//


VOID
InitializeMemoryMap (
    VOID
    )
/*++

Routine Description:

    Initialize memory subsystem

Arguments:

    None

Returns:

    None

--*/
{
    InitializeLock (&MemoryLock, TPL_NOTIFY);
    InitializeListHead (&MemoryMap);
    InitializePool ();
    MemoryMapKey = 0;
    MapDepth = 0;
    FreeMapStack = 0;
}

VOID
InitializeMemoryMapWatermarks (
  VOID
  )

{
  EFI_STATUS  BootServicesCodeStatus;
  EFI_STATUS  BootServicesDataStatus;
  EFI_STATUS  RuntimeServicesCodeStatus;
  EFI_STATUS  RuntimeServicesDataStatus;
  UINTN       Count;

  RuntimeServicesDataStatus = AllocatePages (
                                AllocateMaxAddress,
                                EfiRuntimeServicesData,
                                EFI_RUNTIME_SERVICES_DATA_RESERVE >> EFI_PAGE_SHIFT,
                                &EfiWatermarkRuntimeServicesData
                                );

  RuntimeServicesCodeStatus = AllocatePages (
                                AllocateMaxAddress,
                                EfiRuntimeServicesCode,
                                EFI_RUNTIME_SERVICES_CODE_RESERVE >> EFI_PAGE_SHIFT,
                                &EfiWatermarkRuntimeServicesCode
                                );

  BootServicesCodeStatus = AllocatePages (
                             AllocateMaxAddress,
                             EfiBootServicesCode,
                             EFI_BOOT_SERVICES_CODE_RESERVE >> EFI_PAGE_SHIFT,
                             &EfiWatermarkBootServicesCode
                             );

  BootServicesDataStatus = AllocatePages (
                             AllocateMaxAddress,
                             EfiBootServicesData,
                             EFI_BOOT_SERVICES_DATA_RESERVE >> EFI_PAGE_SHIFT,
                             &EfiWatermarkBootServicesData
                             );

  Count = 0;
  if (!EFI_ERROR (RuntimeServicesCodeStatus)) {
    FreePages (EfiWatermarkRuntimeServicesCode, EFI_RUNTIME_SERVICES_CODE_RESERVE >> EFI_PAGE_SHIFT);
    EfiWatermarkRuntimeServicesCode += (EFI_RUNTIME_SERVICES_CODE_RESERVE - 1);
    Count++;
  }
  if (!EFI_ERROR (RuntimeServicesDataStatus)) {
    FreePages (EfiWatermarkRuntimeServicesData, EFI_RUNTIME_SERVICES_DATA_RESERVE >> EFI_PAGE_SHIFT);
    EfiWatermarkRuntimeServicesData += (EFI_RUNTIME_SERVICES_DATA_RESERVE - 1);
    Count++;
  }
  if (!EFI_ERROR (BootServicesCodeStatus)) {
    FreePages (EfiWatermarkBootServicesCode, EFI_BOOT_SERVICES_CODE_RESERVE >> EFI_PAGE_SHIFT);
    EfiWatermarkBootServicesCode += (EFI_BOOT_SERVICES_CODE_RESERVE - 1);
    Count++;
  }
  if (!EFI_ERROR (BootServicesDataStatus)) {
    FreePages (EfiWatermarkBootServicesData, EFI_BOOT_SERVICES_DATA_RESERVE >> EFI_PAGE_SHIFT);
    EfiWatermarkDefault = EfiWatermarkBootServicesData - 1;
    EfiWatermarkBootServicesData += (EFI_BOOT_SERVICES_DATA_RESERVE - 1);
    Count++;
  }

  if (Count != 4) {
    EfiWatermarkRuntimeServicesCode = MAX_ADDRESS;
    EfiWatermarkRuntimeServicesData = MAX_ADDRESS;
    EfiWatermarkBootServicesCode = MAX_ADDRESS;
    EfiWatermarkBootServicesData = MAX_ADDRESS;
    EfiWatermarkDefault = MAX_ADDRESS;
  }
}
    
VOID
FwAddMemoryDescriptor (
    IN EFI_MEMORY_TYPE          Type,
    IN EFI_PHYSICAL_ADDRESS     Start,
    IN UINT64                   NoPages,
    IN UINT64                   Attribute
    )
/*++

Routine Description:

    Called to initialize the memory map and add descriptors to
    the current descriptor list.

    N.B. The first descriptor that is added must be general usable
    memory as the addition allocates heap.

Arguments:

    Type        - The type of memory to add

    Start       - The starting address in the memory range.
                  Must be page aligned.

    NoPages     - The number of pages in the range

    Attribute   - Attributes of the memory to add

Returns:

    None.  The range is added to the memroy map.

--*/
{
    EFI_PHYSICAL_ADDRESS            End;

    AcquireLock (&MemoryLock);
    End = Start + LShiftU64(NoPages, EFI_PAGE_SHIFT) - 1;
    AddRange (Type, Start, End, Attribute);
    FreeMemoryMapStack ();
    ReleaseLock (&MemoryLock);
}


STATIC
VOID
AddRange (
    IN EFI_MEMORY_TYPE              Type,
    IN EFI_PHYSICAL_ADDRESS         Start,
    IN EFI_PHYSICAL_ADDRESS         End,
    IN UINT64                       Attribute
    )
/*++

Routine Description:

    Internal function.  Adds a ranges to the memory map.  The
    range must not already exist in the map.

Arguments:

    Type        - The type of memory range to add

    Start       - The starting address in the memory range
                  Must be paged aligned.

    End         - The last address in the range
                  Must be the last byte of a page.

    Attribute   - The attributes of the memory range to add

Returns:

    None.  The range is added to the memroy map.

--*/
{
    LIST_ENTRY          *Link;
    MEMORY_MAP          *Entry;
    MEMORY_MAP          *Entry1;
    BOOLEAN             Merge;

    ASSERT ((Start & EFI_PAGE_MASK) == 0);
    ASSERT (End > Start) ;

    ASSERT_LOCKED (&MemoryLock);
    DEBUG((D_PAGE, "AddRange: %lx-%lx to %d\n", Start, End, Type));

    //
    // Memory map being altered
    //

    MemoryMapKey += 1;
    
    //
    // Look for adjoining memory descriptor
    //
    // Two memory descriptors can only be merged if they have the same Type
    // and the same Attribute.
    //

    Merge = FALSE;
    Link = MemoryMap.Flink;
    while (Link != &MemoryMap) {
        Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
        Link  = Link->Flink;

        if (Entry->Type != Type) {
            continue;
        }

        if (Entry->Attribute != Attribute) {
            continue;
        }

        if (Entry->End + 1 == Start) {
            Entry->End = End;
            Merge = TRUE;

        } 
        else if (Entry->Start == End + 1) {
            Entry->Start = Start;
            Merge = TRUE;
        }

        if (Merge) {
            Link = MemoryMap.Flink;
            while (Link != &MemoryMap) {
                Entry1 = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
                Link  = Link->Flink;

                if (Entry->Type != Entry1->Type) {
                  continue;
                }
                if (Entry->Attribute != Entry1->Attribute) {
                  continue;
                }
                if (Entry->End + 1 == Entry1->Start) {
                  Entry1->Start = Entry->Start;
                  RemoveMemoryMapEntry (Entry);
                  return;
                } else if (Entry->Start == Entry1->End + 1) {
                  Entry1->End = Entry->End;
                  RemoveMemoryMapEntry (Entry);
                  return;
                }
            }
            return;
        }
    }

    //
    // Add descriptor 
    //

    MapStack[MapDepth].Signature = MEMORY_MAP_SIGNATURE;
    MapStack[MapDepth].FromPool = FALSE;
    MapStack[MapDepth].Type = Type;
    MapStack[MapDepth].Start = Start;
    MapStack[MapDepth].End = End;
    MapStack[MapDepth].VirtualStart = 0;
    MapStack[MapDepth].Attribute = Attribute;
    InsertTailList (&MemoryMap, &MapStack[MapDepth].Link);

    MapDepth += 1;
    ASSERT (MapDepth < MAX_MAP_DEPTH);
    return;
}


STATIC
VOID
FreeMemoryMapStack (
    VOID
    )
/*++

Routine Description:

    Internal function.  Moves any memory descriptors that are on the
    temporary descriptor stack to heap.

Arguments:

    None.

Returns:

    None.

--*/
{
    MEMORY_MAP      *Entry, *Entry2;
    LIST_ENTRY      *Link2;

    ASSERT_LOCKED (&MemoryLock);

    //
    // If already freeing the map stack, then return
    //

    if (FreeMapStack) {
        return ;
    }

    //
    // Move the temporary memory descriptor stack into pool
    //

    FreeMapStack += 1;

    while (MapDepth) {

        //
        // Allocate memory for a entry
        //

        Entry = AllocatePoolI (EfiRuntimeServicesData, sizeof(MEMORY_MAP));

        //
        // Update to proper entry
        //

        MapDepth -= 1;

        if (MapStack[MapDepth].Link.Flink) {
            //
            // Move this entry to general pool
            //

            RemoveEntryList (&MapStack[MapDepth].Link);
            MapStack[MapDepth].Link.Flink = NULL;

            *Entry = MapStack[MapDepth];
            Entry->FromPool = TRUE;

            //
            // Find insertion location
            //

            for (Link2 = MemoryMap.Flink; Link2 != &MemoryMap; Link2 = Link2->Flink) {
                Entry2 = CR(Link2, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
                if (Entry2->FromPool && Entry2->Start > Entry->Start) {
                    break;
                }
            }

            InsertTailList (Link2, &Entry->Link);

        } else {

            //
            // It was removed, don't move it
            //

            FreePoolI (Entry);

        }
    }


    FreeMapStack -= 1;
}

STATIC
VOID
RemoveMemoryMapEntry (
    MEMORY_MAP          *Entry
    )
/*++

Routine Description:

    Internal function.  Removes a descrptor entry.

Arguments:

    Entry       - The entry to remove

Returns:

    None.

--*/
{
    RemoveEntryList (&Entry->Link);
    Entry->Link.Flink = NULL;

    if (Entry->FromPool) {
        FreePoolI (Entry);
    }
}


STATIC
EFI_STATUS
ConvertPages (
    IN BOOLEAN              LoadConvert,
    IN UINT64               Start,
    IN UINT64               NoPages,
    IN EFI_MEMORY_TYPE      NewType
    )
/*++

Routine Description:

    Internal function.  Converts a memory range to the specified type.
    The range must exist in the memory map.

Arguments:

    LoadConvert     - Signifies that the range is being converted on behalf
                      of the loader

    Start           - The first address of the range.
                      Must be page aligned.

    NoPages         - The number of pages to convert

    NewType         - The new type for the memory range

Returns:

    Status.

--*/
{

    UINT64                  NoBytes, End, RangeEnd, Attribute;
    LIST_ENTRY              *Link;
    MEMORY_MAP              *Entry;

    NoBytes = LShiftU64(NoPages, EFI_PAGE_SHIFT);
    End = Start + NoBytes - 1;

    ASSERT (!EfiAtRuntime);
    ASSERT (NoPages);
    ASSERT ((Start & EFI_PAGE_MASK) == 0);
    ASSERT (End > Start) ;
    ASSERT_LOCKED (&MemoryLock);

    if (!NoPages || (Start & EFI_PAGE_MASK) || (Start > Start + NoBytes)) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Convert the entire range
    //

    while (Start < End) {

        //
        // Find the entry that the covers the range
        //

        for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
            Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);

            if (Entry->Start <= Start && Entry->End > Start) {
                break;
            }
        }

        if (Link == &MemoryMap) {
            DEBUG((D_ERROR, "ConvertPages: failed to find range %lx - %lx\n", Start, End));
            return EFI_NOT_FOUND;
        }

        //
        // Convert range to the end, or to the end of the descriptor
        // if that's all we've got
        //

        RangeEnd = End;
        if (Entry->End < End) {
            RangeEnd = Entry->End;
        }

        DEBUG((D_PAGE, "ConvertRange: %lx-%lx to %d\n", Start, RangeEnd, NewType));

        //
        // Debug code - verify conversion is allowed
        //

        if (!LoadConvert && 
            !(NewType == EfiConventionalMemory ? 1 : 0) ^ (Entry->Type == EfiConventionalMemory ? 1 : 0)) {
            DEBUG((D_ERROR, "ConvertPages: Incompatible memory types\n"));
            return EFI_NOT_FOUND;
        }    

        //
        // Memory map being altered
        //

        MemoryMapKey += 1;

        //
        // Pull range out of descritpr
        //

        if (Entry->Start == Start) {

            // clip start
            Entry->Start = RangeEnd + 1;

        } else if (Entry->End == RangeEnd) {

            // clip end
            Entry->End = Start - 1;

        } else {

            // pull it out of the center. clip current

            // add a new one
            MapStack[MapDepth].Signature = MEMORY_MAP_SIGNATURE;
            MapStack[MapDepth].FromPool = FALSE;
            MapStack[MapDepth].Type = Entry->Type;
            MapStack[MapDepth].Start = RangeEnd+1;
            MapStack[MapDepth].End = Entry->End;

            //
            // Inherit Attribute from the Memory Descriptor that is being clipped
            //

            MapStack[MapDepth].Attribute = Entry->Attribute;

            Entry->End = Start - 1;
            ASSERT (Entry->Start < Entry->End);

            Entry = &MapStack[MapDepth];
            InsertTailList (&MemoryMap, &Entry->Link);

            MapDepth += 1;
            ASSERT (MapDepth < MAX_MAP_DEPTH);
        }

        //
        // The new range inherits the same Attribute as the Entry it is being cut out of.
        //

        Attribute = Entry->Attribute;

        //
        // If the descriptor is empty, then remove it from the map
        //

        if (Entry->Start == Entry->End + 1) {
            RemoveMemoryMapEntry (Entry);
            Entry = NULL;
        }
        
        //
        // Add our new range in
        //

        AddRange (NewType, Start, RangeEnd, Attribute);

        //
        // Move any map descriptor stack to general pool
        //

        FreeMemoryMapStack ();

        //
        // Bump the starting address, and convert the next range
        //

        Start = RangeEnd + 1;
    }

    //
    // Coverted the whole range, done
    //

    return EFI_SUCCESS;
}

STATIC
UINT64
FindFreePagesI (
    IN UINT64           MaxAddress,
    IN UINT64           NoPages,
    IN EFI_MEMORY_TYPE  NewType,
    IN UINTN            Alignment
    )
/*++

Routine Description:

    Internal function.  Finds a consecutive free page range below
    the requested address

Arguments:

    MaxAddress          - The address that the range must be below

    NoPages             - Number of pages needed

    NewType             - The type of memory the range is going to be turned into

Returns:

    The base address of the range, or 0 if the range was not found.

--*/
{
    UINT64              NoBytes, Target;
    UINT64              DescStart, DescEnd, DescNoBytes;
    LIST_ENTRY          *Link;
    MEMORY_MAP          *Entry;

    if (MaxAddress < EFI_PAGE_MASK) {
    	return 0;
    }
    
    if ((MaxAddress & EFI_PAGE_MASK) != EFI_PAGE_MASK) {
    									// if MaxAddress is not aligned to the end of a page
       	MaxAddress -= (EFI_PAGE_MASK + 1);	// Change MaxAddress to be 1 page lower
        MaxAddress &= ~EFI_PAGE_MASK;		// set MaxAddress to a page boundary
        MaxAddress |= EFI_PAGE_MASK;		// set MaxAddress to end of the page
    }

    if (!NoPages) {
        return 0;
    }

    NoBytes = LShiftU64(NoPages, EFI_PAGE_SHIFT);
    Target = 0;

    for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
        Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
    
        //
        // If it's not a free entry, don't bother with it
        //

        if (Entry->Type != EfiConventionalMemory) {
            continue;
        }

        DescStart = Entry->Start;
        DescEnd = Entry->End;

        //
        // If desc is past max allowed address, skip it
        //

        if (DescStart >= MaxAddress) {
            continue;
        }

        //
        // If desc ends past max allowed address, clip the end
        //

        if (DescEnd >= MaxAddress) {
            DescEnd = MaxAddress;
        }

        DescEnd = ((DescEnd + 1) & (~(Alignment - 1))) - 1;

        //
        // Compute the number of bytes we can used from this 
        // descriptor, and see it's enough to statisfy the request
        //

//        DescNoBytes = Entry->End - Entry->Start + 1;
        DescNoBytes = DescEnd - DescStart + 1;

        if (DescNoBytes >= NoBytes) {

            //
            // If this is the best match so far remember it
            //

        
            if (DescEnd > Target) {
                Target = DescEnd;
            }
        }
    }                    

    //
    // If this is a grow down, adjust target to be the allocation base
    //

    Target -= NoBytes - 1;

    //
    // If we didn't find a match, return 0
    //

    if (Target & EFI_PAGE_MASK) {
        Target = 0;
    }

    return Target;
}

STATIC
UINT64
FindFreePages (
    IN UINT64           MaxAddress,
    IN UINT64           NoPages,
    IN EFI_MEMORY_TYPE  NewType,
    IN UINTN            Alignment
    )
/*++

Routine Description:

    Internal function.  Finds a consecutive free page range below
    the requested address

Arguments:

    MaxAddress          - The address that the range must be below

    NoPages             - Number of pages needed

    NewType             - The type of memory the range is going to be turned into

Returns:

    The base address of the range, or 0 if the range was not found.

--*/
{
    UINT64  NewMaxAddress;
    UINT64  Start;

    NewMaxAddress = MaxAddress;

    switch (NewType) {
    case EfiRuntimeServicesCode:
      if (MaxAddress > EfiWatermarkRuntimeServicesCode) {
        NewMaxAddress = EfiWatermarkRuntimeServicesCode;
      }
      break;
    case EfiRuntimeServicesData:
      if (MaxAddress > EfiWatermarkRuntimeServicesData) {
        NewMaxAddress = EfiWatermarkRuntimeServicesData;
      }
      break;
    case EfiBootServicesCode:
      if (MaxAddress > EfiWatermarkBootServicesCode) {
        NewMaxAddress = EfiWatermarkBootServicesCode;
      }
      break;
    case EfiBootServicesData:
      if (MaxAddress > EfiWatermarkBootServicesData) {
        NewMaxAddress = EfiWatermarkBootServicesData;
      }
      break;
    case EfiLoaderCode:
    case EfiLoaderData:
    case EfiACPIReclaimMemory:
    case EfiACPIMemoryNVS:
    case EfiReservedMemoryType:
      if (MaxAddress > EfiWatermarkDefault) {
        NewMaxAddress = EfiWatermarkDefault;
      }
      break;
    }

    Start = FindFreePagesI (NewMaxAddress, NoPages, NewType, Alignment);
    if (!Start) {
      Start = FindFreePagesI (MaxAddress, NoPages, NewType, Alignment);
    }

    return Start;
}

EFI_STATUS
BOOTSERVICE
BootServiceAllocatePages (
    IN EFI_ALLOCATE_TYPE        Type,
    IN EFI_MEMORY_TYPE          MemoryType,
    IN UINTN                    NoPages,
    OUT EFI_PHYSICAL_ADDRESS    *Memory
    )
/*++

Routine Description:

    Allocates pages from the memory map.

Arguments:

    Type            - The type of allocation to perform

    MemoryType      - The type of memory to turn the allocated pages into.

    NoPages         - The number of pages to allocate

    Memory          - A pointer to recieve the base allocated memory address

Returns:

    Status.  On success, Memory is filled in with the base address allocated

--*/
{
    if (MemoryType >= EfiMaxMemoryType && MemoryType <= 0x7fffffff) {
        return EFI_INVALID_PARAMETER;
    }

    return AllocatePages(Type,MemoryType,NoPages,Memory);
}

EFI_STATUS
AllocatePages (
    IN EFI_ALLOCATE_TYPE        Type,
    IN EFI_MEMORY_TYPE          MemoryType,
    IN UINTN                    NoPages,
    OUT EFI_PHYSICAL_ADDRESS    *Memory
    )
/*++

Routine Description:

    Allocates pages from the memory map.

Arguments:

    Type            - The type of allocation to perform

    MemoryType      - The type of memory to turn the allocated pages into.

    NoPages         - The number of pages to allocate

    Memory          - A pointer to recieve the base allocated memory address

Returns:

    Status.  On success, Memory is filled in with the base address allocated

--*/
{
    EFI_STATUS              Status;
    UINT64                  Start, MaxAddress;
    UINTN                   Alignment;

    if (Type < AllocateAnyPages || Type >= MaxAllocateType) {
        return EFI_INVALID_PARAMETER;
    }

    if (EfiAtRuntime) {
        return EFI_UNSUPPORTED;
    }

    Alignment = EFI_DEFAULT_PAGE_ALLOCATION_ALIGNMENT;

    if  (MemoryType == EfiACPIReclaimMemory   ||
         MemoryType == EfiACPIMemoryNVS       ||
         MemoryType == EfiRuntimeServicesCode ||
         MemoryType == EfiRuntimeServicesData    ) {

      Alignment = EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT;
    }

    if (Type == AllocateAddress) {
      if ((*Memory & (Alignment - 1)) != 0) {
        return EFI_INVALID_PARAMETER;
      }
    }

    NoPages += EFI_SIZE_TO_PAGES (Alignment) - 1;
    NoPages &= ~(EFI_SIZE_TO_PAGES (Alignment) - 1);

    //
    // If this is for below a particular address, then 
    //

    Start = *Memory;
    MaxAddress = MAX_ADDRESS;
    if (Type == AllocateMaxAddress) {
        MaxAddress = Start;
    }

    AcquireLock (&MemoryLock);
    
    //
    // If not a specific address, then find an address to allocate
    //

    if (Type != AllocateAddress) {
        Start = FindFreePages(MaxAddress, NoPages, MemoryType, Alignment);
        if (!Start) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }
    }

    //
    // Convert pages from FreeMemory to the requested type
    //

    Status = ConvertPages (FALSE, Start, NoPages, MemoryType);

Done:
    ReleaseLock (&MemoryLock);

    if (!EFI_ERROR(Status)) {
        *Memory = Start;
    }

    return Status;
}


EFI_STATUS 
BOOTSERVICE
FreePages (
    IN EFI_PHYSICAL_ADDRESS     Memory,
    IN UINTN                    NoPages
    )
/*++

Routine Description:

    Frees previous allocated pages.

Arguments:

    Memory          - Base address of memory being freed

    NoPages         - The number of pages to free

Returns:

    Status.

--*/
{
    EFI_STATUS          Status;
    LIST_ENTRY          *Link;
    MEMORY_MAP          *Entry;
    UINTN               Alignment;

    if (EfiAtRuntime) {
        return EFI_UNSUPPORTED;
    }

    //
    // Free the range
    //
    AcquireLock (&MemoryLock);

    //
    // Find the entry that the covers the range
    //
    for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
        Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
        if (Entry->Start <= Memory && Entry->End > Memory) {
            break;
        }
    }
    if (Link == &MemoryMap) {
      ReleaseLock (&MemoryLock);
      return EFI_NOT_FOUND;
    }

    Alignment = EFI_DEFAULT_PAGE_ALLOCATION_ALIGNMENT;

    if  (Entry->Type == EfiACPIReclaimMemory   ||
         Entry->Type == EfiACPIMemoryNVS       ||
         Entry->Type == EfiRuntimeServicesCode ||
         Entry->Type == EfiRuntimeServicesData    ) {

      Alignment = EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT;

    }

    if ((Memory & (Alignment - 1)) != 0) {
      ReleaseLock (&MemoryLock);
      return EFI_INVALID_PARAMETER;
    }

    NoPages += EFI_SIZE_TO_PAGES (Alignment) - 1;
    NoPages &= ~(EFI_SIZE_TO_PAGES (Alignment) - 1);


    //
    //check if the NoPages is valid
    //
    if(!NoPages || NoPages > RShiftU64(Entry->End - Entry->Start + 1, EFI_PAGE_SHIFT)){
        ReleaseLock(&MemoryLock);
        return EFI_INVALID_PARAMETER;
    }

    Status = ConvertPages (FALSE, Memory, NoPages, EfiConventionalMemory);
    ReleaseLock (&MemoryLock);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Destroy the contents
    //

    if (Memory < MAX_ADDRESS) {
        DBGSETMEM ((VOID *) Memory, NoPages << EFI_PAGE_SHIFT);
    }
    
    return Status;
}


EFI_STATUS
BOOTSERVICE
GetMemoryMap (
    IN OUT UINTN                    *MemoryMapSize,
    IN OUT EFI_MEMORY_DESCRIPTOR    *Desc,
    OUT UINTN                       *MapKey,
    OUT UINTN                       *DescriptorSize,
    OUT UINT32                      *DescriptorVersion
    )
/*++

Routine Description:

    Returns the current memory map.

Arguments:

    MemoryMapSize   - On input the sizeof of Desc
                      On output the amount of data return in Desc

    Desc            - The buffer to return the memory map in

    MapKey          - The address to return the current mapkey in

Returns:

    Status.
    On success, Desc & MemoryMapSize is completed with the current memory map.
    On buffer too small, MemoryMapSize is completed with the buffer size needed.

--*/
{
    UINTN                       BufferSize;
    EFI_STATUS                  Status;
    LIST_ENTRY                  *Link;
    MEMORY_MAP                  *Entry;
    UINTN                       Size;

    if (MemoryMapSize == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Size = sizeof(EFI_MEMORY_DESCRIPTOR);

    //
    // Make sure Size != sizeof(EFI_MEMORY_DESCRIPTOR). This will
    //  prevent people from having pointer math bugs in thier code.
    //  now you have to use *DescriptorSize to make things work.
    //
    Size += sizeof(UINT64) - (Size % sizeof(UINT64));

    if (DescriptorSize != NULL) {
      *DescriptorSize = Size;
    }

    if (DescriptorVersion != NULL) {
      *DescriptorVersion = EFI_MEMORY_DESCRIPTOR_VERSION;
    }

    AcquireLock (&MemoryLock);

    //
    // Compute the buffer size needed to fit the entire map
    //

    BufferSize = 0;
    for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
        BufferSize += Size;
    }

    if (*MemoryMapSize < BufferSize) {
        Status = EFI_BUFFER_TOO_SMALL;
        goto Done;
    }

    if (Desc == NULL) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }


    //
    // Build the map
    //

    ZeroMem (Desc, Size);
    for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
        Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
        ASSERT (!Entry->VirtualStart);

        Desc->Type = Entry->Type;
        Desc->PhysicalStart = Entry->Start;
        Desc->VirtualStart = Entry->VirtualStart;
        Desc->NumberOfPages = RShiftU64(Entry->End - Entry->Start + 1, EFI_PAGE_SHIFT);
    
        switch (Entry->Type) {
            case EfiReservedMemoryType:
            case EfiUnusableMemory:
            case EfiACPIReclaimMemory:
            case EfiACPIMemoryNVS:
            case EfiMemoryMappedIO:
            case EfiMemoryMappedIOPortSpace:
            case EfiLoaderCode:
            case EfiLoaderData:
            case EfiBootServicesCode:
            case EfiBootServicesData:
            case EfiConventionalMemory:
                Desc->Attribute = Entry->Attribute;
                break;

            case EfiRuntimeServicesCode:
            case EfiRuntimeServicesData:
            case EfiPalCode:
                Desc->Attribute = Entry->Attribute | EFI_MEMORY_RUNTIME;
                break;

            default:
                Desc->Attribute = Entry->Attribute;
                break;
        }
        
        Desc = NextMemoryDescriptor(Desc, Size);
    }


    //
    //
    //

    if (MapKey) {
        *MapKey = MemoryMapKey;
    }

    Status = EFI_SUCCESS;

Done:
    ReleaseLock (&MemoryLock);
    *MemoryMapSize = BufferSize;
    return Status;
}



INTERNAL
VOID *
AllocatePoolPages (
    IN EFI_MEMORY_TYPE          PoolType,
    IN UINTN                    NoPages,
    IN UINTN                    Alignment
    )
/*++

Routine Description:

    Internal function.  Used by the pool functions to allocate pages
    to back pool allocation requests.

Arguments:

    PoolType        - The type of memory for the new pool pages

    NoPages         - No of pages to allocate

Returns:

    The allocated memory, or NULL.

--*/
{
    UINT64                      Start;
    EFI_STATUS                  Status;

    ASSERT (!EfiAtRuntime);

    //
    // Find the pages to convert
    //

    Start = FindFreePages (MAX_ADDRESS, NoPages, PoolType, Alignment);

    //
    // Convert it to boot services data
    //

    if (Start) {
        Status = ConvertPages (FALSE, Start, NoPages, PoolType);
        if (EFI_ERROR(Status)) {
            Start = 0;
        }
    }

    if (!Start) {
        DEBUG((D_ERROR, "AllocatePoolPages: failed to allocate %d pages\n", NoPages));
    }

    return (VOID *) Start;
}

INTERNAL
VOID
FreePoolPages (
    IN EFI_PHYSICAL_ADDRESS     Memory,
    IN UINTN                    NoPages
    )
/*++

Routine Description:

    Internal function.  Frees pool pages allocated via AllocatePoolPages()

Arguments:

    Memory          - The base address to free

    NoPages         - The number of pages to free


Returns:

    None

--*/
{
    ConvertPages (FALSE, Memory, NoPages, EfiConventionalMemory);
}


EFI_STATUS
SetCodeSection (
    IN EFI_PHYSICAL_ADDRESS     Base,
    IN UINTN                    NoPages,
    IN EFI_MEMORY_TYPE          MemoryType
    )
// Used by the loader to set sections to code
/*++

Routine Description:

    Used by the loader to convert code pages to code memory
    types after any fixups have been applied

Arguments:

    Base            - The base address to change the type of

    NoPages         - The number of pages to change the type of

    MemoryType      - The new memory type for the range

Returns:

    Status

--*/
{
    EFI_STATUS                  Status;


    AcquireLock (&MemoryLock);
    Status = ConvertPages (TRUE, Base, NoPages, MemoryType);
    ReleaseLock (&MemoryLock);
    return Status;
}


EFI_STATUS
TerminateMemoryMap (
    IN UINTN                    MapKey
    )
{
    EFI_STATUS                  Status;
    LIST_ENTRY                  *Link;
    MEMORY_MAP                  *Entry;

    Status = EFI_SUCCESS;

    AcquireLock (&MemoryLock);

    if (MapKey == MemoryMapKey) {

        //
        // Make sure the memory map is following all the construction rules
        // This is the last chance we will be able to display any messages on
        // the  console devices.
        //

        for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
          Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
          if (Entry->Attribute & EFI_MEMORY_RUNTIME) { 
            if (Entry->Type == EfiACPIReclaimMemory || Entry->Type == EfiACPIMemoryNVS) {
              DEBUG((D_ERROR, "ExitBootServices: ACPI memory entry has RUNTIME attribute set.\n"));
              ReleaseLock (&MemoryLock);
              return EFI_INVALID_PARAMETER;
            }
            if (Entry->Start & (EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT - 1)) {
              DEBUG((D_ERROR, "ExitBootServices: A RUNTIME memory entry is not on a proper alignment.\n"));
              ReleaseLock (&MemoryLock);
              return EFI_INVALID_PARAMETER;
            }
            if ((Entry->End + 1) & (EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT - 1)) {
              DEBUG((D_ERROR, "ExitBootServices: A RUNTIME memory entry is not on a proper alignment.\n"));
              ReleaseLock (&MemoryLock);
              return EFI_INVALID_PARAMETER;
            }
          }
        }

        //
        // No going back now.  Lock memory map from changing
        //

        EfiAtRuntime = TRUE;

        //
        // The map key they gave us matches what we expect. Fall through and
        // return success. In an ideal world we would clear out all of
        // EfiBootServicesCode and EfiBootServicesData. However this function
        // is not the last one called by ExitBootServices(), so we have to
        // preserve the memory contents.
        //

    } else {

        Status = EFI_INVALID_PARAMETER;

    }

    ReleaseLock (&MemoryLock);
    return Status;
}
