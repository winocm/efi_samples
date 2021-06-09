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

    imem.h

Abstract:




Revision History

--*/


#include "efifw.h"


//
// MEMORY_MAP_ENTRY
//

#define MEMORY_MAP_SIGNATURE   EFI_SIGNATURE_32('m','m','a','p')
typedef struct {
    UINTN                   Signature;
    LIST_ENTRY              Link;
    BOOLEAN                 FromPool;

    EFI_MEMORY_TYPE         Type;
    UINT64                  Start;
    UINT64                  End;

    UINT64                  VirtualStart;
    UINT64                  Attribute;
} MEMORY_MAP;

//
// Internal prototypes
//

INTERNAL
VOID
InitializePool (
    VOID
    );

INTERNAL
VOID *
AllocatePoolPages (
    IN EFI_MEMORY_TYPE          PoolType,
    IN UINTN                    NoPages,
    IN UINTN                    Alignment
    );

INTERNAL
VOID
FreePoolPages (
    IN EFI_PHYSICAL_ADDRESS     Memory,
    IN UINTN                    NoPages
    );

INTERNAL
VOID *
AllocatePoolI (
    IN EFI_MEMORY_TYPE          PoolType,
    IN UINTN                    Size
    );

INTERNAL
EFI_STATUS
FreePoolI (
    IN VOID                     *Buffer
    );

//
// Internal Global data
//

extern FLOCK        MemoryLock; 
extern LIST_ENTRY   MemoryMap;
extern MEMORY_MAP   *MemoryLastConvert;