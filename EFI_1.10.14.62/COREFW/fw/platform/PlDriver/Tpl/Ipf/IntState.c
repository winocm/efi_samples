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

    plmisc.c

Abstract: 

    Mils pl interfaces that are IA-64 specific


Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "SalEfi.h"
#include "SalProc.h"

STATIC BOOLEAN   SalEmulateInterruptState = FALSE;

BOOLEAN
PlSetInterruptState (
    IN BOOLEAN      Enable
    )
/*++

Routine Description:

    This routine sets the interrupt state (psr.i)

Arguments:

    Interrupt state to set

Returns:
   

--*/
{
    BOOLEAN         PreviousState;
    rArg            Results = {-1,0,0,0} ;

    //
    // save the previous state
    //

    PreviousState = SalEmulateInterruptState;

    //
    // call SAL callback to set state
    //

    SalCallBack(
        ID_SALCB_INTERRUPT_STATE, (UINTN) Enable,
        0,0,0,
        0,0,0,
        &Results
        );

    //
    // If callback succeeded then cache the state
    //

    if(!Results.p0) {
        SalEmulateInterruptState = Enable;
    }
   
    return PreviousState;
}
