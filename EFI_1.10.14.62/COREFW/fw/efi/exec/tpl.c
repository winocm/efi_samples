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

    tpl.c

Abstract:

    Task priority function    



Revision History

--*/

#include "exec.h"

//
// Declare runtime functions
//

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(SetInterruptStateThunk)
#pragma RUNTIME_CODE(HighestSetBit)
#pragma RUNTIME_CODE(CurrentTPL)
#pragma RUNTIME_CODE(RaiseTPL)
#pragma RUNTIME_CODE(RestoreTPL)
#endif


//
//
//


BOOLEAN
INTERNAL STATIC
SetInterruptStateThunk (
    IN BOOLEAN      Enable
    )
{
    return Enable ^ TRUE;
}

//
// Return the highest set bit
//

UINTN
STATIC
HighestSetBit (
    IN UINTN     i
    )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
    if (i & 0xFFFF0000) {
        if (i & 0xFF000000) {
            return HSB[i >> 24] + 24;
        } else {
            return HSB[i >> 16] + 16;
        }
    } else {
        if (i & 0xFF00) {
            return HSB[i >> 8] + 8;
        } else {
            return HSB[i];
        }
    }
}


//
// Current Task Priority Level
//

EFI_TPL
CurrentTPL (
    VOID
    )
/*++

Routine Description:

    Returns the current task priority level.

    N.B. Having interrupts disabled is equivalent to TPL_HIGH_LEVEL

Arguments:

    None.
    
Returns:

    Returns the current task priority level

--*/
{
    return TPL;
}


EFI_TPL
BOOTSERVICE
RaiseTPL (
    IN EFI_TPL      NewTpl
    )
/*++

Routine Description:

    Raise the task priority level to the new level.
    High level is implemented by disabling processor interrupts.

Arguments:

    NewTpl      - New task priority level
    
Returns:

    The previous task priority level

--*/
{
    EFI_TPL     OldTpl;
    BOOLEAN     PreviousState;

    OldTpl = TPL;
    ASSERT (OldTpl <= NewTpl);
    ASSERT (VALID_TPL (NewTpl));

    //
    // If raising to high level, disable interrupts
    //
    if (NewTpl >= TPL_HIGH_LEVEL  &&  OldTpl < TPL_HIGH_LEVEL) {
        PreviousState = EfiSetInterruptState (FALSE);
        // ASSERT (PreviousState);
    }

    //
    // Set the new value
    //
    TPL = NewTpl;

    return OldTpl;
}


VOID
BOOTSERVICE
RestoreTPL (
    IN EFI_TPL NewTpl
    )
/*++

Routine Description:

    Lowers the task priority to the previous value.   If the new 
    priority unmasks events at a higher priority, they are dispatched.

Arguments:

    NewTpl  - New, lower, task priority
    
Returns:

    None

--*/
{
    EFI_TPL     OldTpl;

    OldTpl = CurrentTPL();
    ASSERT (NewTpl <= OldTpl);
    ASSERT (VALID_TPL (NewTpl));

    //
    // If lowering below HIGH_LEVEL, make sure
    // interrupts are enabled
    //
    if (OldTpl >= TPL_HIGH_LEVEL  &&  NewTpl < TPL_HIGH_LEVEL) {
       TPL = TPL_HIGH_LEVEL;  
    }

    //
    // Dispatch any pending events
    //
    while ((-2 << NewTpl) & EventPending) {
        TPL = HighestSetBit(EventPending);
        if (TPL < TPL_HIGH_LEVEL) {
            EfiSetInterruptState (TRUE);
        }
        DispatchEventNotifies (TPL);
    }

    //
    // Set the new value
    //
    TPL = NewTpl;

    //
    // If lowering below HIGH_LEVEL, make sure
    // interrupts are enabled
    //
    if (TPL < TPL_HIGH_LEVEL) {
       EfiSetInterruptState (TRUE);
    }
}
