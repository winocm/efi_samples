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

    timer.c

Abstract:

    Handles timer interface between SAL and EFI


Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlTimer.h"
#include "salefi.h"
#include "salproc.h"
#include "Int86.h"

EFI_STATUS
PlTimer(
    IN UINT8  State,
    IN UINTN  Period,
    IN PLABEL *ProcLabel
    )
/*++

Routine Description:

    Registers a timer callback with SAL and starts timer tick or
    Deregisters and cancels timer


Arguments:
    State to indicate register or deregister
    Period in msec

Returns:

    EFI_STATUS

--*/
{
    rArg                Return={-1,0,0,0};
    EFI_STATUS          Status;

    
    Status = EFI_SUCCESS;

    if (State == (UINT8)ID_SALCB_TIMER_START) { 

        ASSERT (Period);

        //
        // register timer with SAL
        //

        SalCallBack(
            ID_SALCB_TIMER, ID_SALCB_TIMER_START, Period,
            (UINT64)(ProcLabel->ProcEntryPoint), (UINT64)(ProcLabel->GP),
            0,0,0,
            &Return
            );

        if(Return.p0) {
            Status = EFI_UNSUPPORTED;
            DEBUG((D_ERROR, "PlTimer: Timer could not be started: %08x\n", Status));
        }

    }
    else {
        //
        // cancel timer
        //

        SalCallBack(
            ID_SALCB_TIMER, ID_SALCB_TIMER_CANCEL,
            0,0,0,0,0,0,
            &Return
            );

        if(Return.p0) {
            Status = EFI_UNSUPPORTED;
            DEBUG((D_ERROR, "PlTimer: Timer could not be cancelled: %08x\n", Status));
        }

    }
    
    return (Status);
}

