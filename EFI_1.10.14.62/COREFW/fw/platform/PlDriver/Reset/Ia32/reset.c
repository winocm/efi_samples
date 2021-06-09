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

    reset.c

Abstract: 

    Miss Runtime Pl interfaces that are IA-32 specific


Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "plreset.h"
#include "PlDefio.h"

#pragma RUNTIME_CODE(RtPlResetSystem)
STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlResetSystem (
    IN EFI_RESET_TYPE   ResetType,
    IN EFI_STATUS       ResetStatus,
    IN UINTN            DataSize,
    IN CHAR16           *ResetData OPTIONAL
    )
/*++

Routine Description:

    This routine handles reset system calls

Arguments:

    Reset type
    Reason for reset
    Size of data pointed to by ResetData
    Context Data

Returns:
 
    None

--*/
{
    UINT8  Value;

    //
    // Do a cold reset by asserting the reset signal on the keyboard controller.
    //

    Value = 0xfe;
    RtDefIoWrite (NULL, IO_UINT8, 0x64, 1, &Value);

    //
    // Halt until the system resets.
    //

    for(;;);

    //
    // Should never get here.
    //

    return EFI_SUCCESS;
}

