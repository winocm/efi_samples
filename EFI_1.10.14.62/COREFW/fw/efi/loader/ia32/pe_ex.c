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

    pe_ex.c

Abstract:

    stub calls mirroring ia64 directory

Revision History

--*/

#include "..\loader.h"
#include "pe.h"


//
// Declare runtime functions
//

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(LoadPeRelocate_Ex)
#endif

#define ALIGN_POINTER(p,s)  ((VOID *) (p + ((s - ((UINTN)p)) & (s-1))))

STATIC
EFI_STATUS
RUNTIMEFUNCTION
LoadPeRelocate_Ex (
    IN UINT16      *Reloc,
    IN OUT CHAR8   *Fixup, 
    IN OUT CHAR8   **FixupData,
    IN UINT64      Adjust
    )
{
    EFI_STATUS                  Status;

    DEBUG((D_LOAD|D_ERROR, "PeRelocate_Ex: unknown fixed type\n"));

    Status = EFI_UNSUPPORTED;

    return Status;
}

STATIC
EFI_STATUS
RUNTIMEFUNCTION INTERNAL
ConvertPeImage_Ex (
    IN UINT16      *Reloc,
    IN OUT CHAR8   *Fixup, 
    IN OUT CHAR8   **FixupData,
    IN UINT64      Adjust
    )
// Reapply fixups to move an image to a new address
{
    EFI_STATUS                  Status;

    DEBUG((D_LOAD|D_ERROR, "ConvertPeImage_Ex: unknown fixed type\n"));

    Status = EFI_UNSUPPORTED;

    return Status;

}





