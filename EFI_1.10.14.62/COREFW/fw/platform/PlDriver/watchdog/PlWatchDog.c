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

    PlWatchdog.c

Abstract: 

    Watchdog timer platform code.

    This code uses the system timer to simulate a hardware-based watchdog timerout.

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlWatchdog.h"

STATIC VOID
WatchdogTimerExpiration (
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
    WATCHDOG_TIMER_CONTEXT *WdtContext;

    WdtContext = (WATCHDOG_TIMER_CONTEXT *) Context;

    DEBUG ((D_ERROR, "WatchDog timer has expired.  Now resetting the system\n"));  

    RT->ResetSystem (
                    EfiResetCold,
                    EFI_TIMEOUT,
                    WdtContext->Size,
                    WdtContext->Data
                    );
}


EFI_STATUS
PlInitWatchdogTimer (
    VOID
    )
{
    EFI_STATUS    Status;

    Status = BS->CreateEvent(EVT_TIMER | EVT_NOTIFY_SIGNAL, 
                             TPL_CALLBACK, 
                             WatchdogTimerExpiration, 
                             &WatchdogTimerContext, 
                             &WatchdogTimerEvent
                             );	

    return Status;
}


EFI_STATUS
PlSetWatchdogTimer (
    IN UINTN                    Timeout,
    IN UINT64                   WatchdogCode,
    IN UINTN                    DataSize,
    IN CHAR16                   *WatchdogData OPTIONAL
    )
{
    EFI_STATUS Status;

    Status = EFI_SUCCESS;     
    WatchdogTimerContext.Size = DataSize;

    if (DataSize > 0) {
        WatchdogTimerContext.Data = WatchdogData;
    } else {
        WatchdogTimerContext.Data = NULL;
    }

    if (Timeout == 0) {
        Status = BS->SetTimer (WatchdogTimerEvent, 
                               TimerCancel, 
                               0x0000
                               );
    } else {
        Status = BS->SetTimer (WatchdogTimerEvent, 
                               TimerRelative, 
                               Timeout * TIMER_CALIBRATE_PER_SECOND
                              );
    }

    return Status;
}

