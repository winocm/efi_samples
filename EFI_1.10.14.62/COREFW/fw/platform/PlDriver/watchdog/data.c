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

    data.c

Abstract: 

    Watchdog timer global data

    The fact that the context record is made global is that we cannot allocate pool
    from the driver initialization because of the fact that an EFI_OUT_OF_RESOURCES
    is not allowed to be returned from the service call (per the spec.).  As such,
    use this ugly artifice.
    
Revision History

--*/
#include "efi.h"
#include "efilib.h"
#include "PlWatchdog.h"

EFI_EVENT               WatchdogTimerEvent;
WATCHDOG_TIMER_CONTEXT  WatchdogTimerContext;

