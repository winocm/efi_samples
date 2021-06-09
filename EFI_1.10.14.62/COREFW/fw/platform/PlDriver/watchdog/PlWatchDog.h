#ifndef _PLWATCHDOG_H
#define _PLWATCHDOG_H
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

    PlWatchdog.h

Abstract: 

    Watchdog timer platform code.


Revision History

--*/

#define TIMER_CALIBRATE_PER_SECOND 10000000

EFI_STATUS
PlInitWatchdogTimer (
    VOID
    );

EFI_STATUS
PlSetWatchdogTimer (
    IN UINTN                    Timeout,
    IN UINT64                   WatchdogCode,
    IN UINTN                    DataSize,
    IN CHAR16                   *WatchdogData OPTIONAL
    );

typedef struct {
    UINTN                       Size;
    UINT16                      *Data;
} WATCHDOG_TIMER_CONTEXT;

    
extern EFI_EVENT               WatchdogTimerEvent;
extern WATCHDOG_TIMER_CONTEXT  WatchdogTimerContext;

#endif