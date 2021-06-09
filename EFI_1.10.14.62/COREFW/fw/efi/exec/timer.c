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

    event.c

Abstract:

    EFI Event support
    


Revision History

--*/


#include "exec.h"

//
// Internal prototypes
//

UINT64
STATIC
CurrentSystemTime (
    VOID
    );

VOID
INTERNAL
CheckTimers (
    IN EFI_EVENT    Event,
    IN VOID         *Context
    );

VOID
STATIC
InsertEventTimer (
    IN IEVENT       *Event
    );

//
// Declare runtime functions
//

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(FwTimerTick)
#endif

//
// Internal data
//

STATIC LIST_ENTRY   EfiTimerList;
STATIC FLOCK        EfiTimerListLock;
STATIC EFI_EVENT    EfiCheckTimerEvent;

STATIC FLOCK        EfiSystemTimeLock;
STATIC UINT64       EfiSystemTime;

//
// Timer functions
//

VOID
InitializeTimer (
    VOID
    )
/*++

Routine Description:

    Initializes timer support

Arguments:

    None
    
Returns:

    None

--*/
{
    EFI_STATUS      Status;

    Status = CreateEvent(
                EVT_NOTIFY_SIGNAL,
                TPL_HIGH_LEVEL-1,
                CheckTimers,
                NULL,
                &EfiCheckTimerEvent
                );
    ASSERT (!EFI_ERROR(Status));

    EfiSystemTime = 0;
    InitializeListHead (&EfiTimerList);
    InitializeLock (&EfiTimerListLock, TPL_HIGH_LEVEL-1);
    InitializeLock (&EfiSystemTimeLock, TPL_HIGH_LEVEL);
}

UINT64
STATIC
CurrentSystemTime (
    VOID
    )
/*++

Routine Description:

    Returns the current system time

Arguments:

    None
    
Returns:

    Returns the current system time

--*/
{
    UINT64          SystemTime;

    // Just use a lock..
    AcquireLock (&EfiSystemTimeLock);
    SystemTime = EfiSystemTime;
    ReleaseLock (&EfiSystemTimeLock);
    return SystemTime;
}



VOID
BOOTSERVICE
FwTimerTick (
    IN UINTN        Duration
    )
/*++

Routine Description:

    Called by the platform code to process a tick.

Arguments:

    Duration    - The number of 100ns elasped since the last call to TimerTick
    
Returns:

    None

--*/
{
    IEVENT          *Event;

    //
    // Check runtiem flag in case there are ticks while exiting boot services
    //

    if (!EfiAtRuntime) {

        AcquireLock (&EfiSystemTimeLock);

        //
        // Update the system time
        //

        EfiSystemTime += Duration;

        //
        // If the head of the list is expired, fire the timer event
        // to process it
        //

        if (!IsListEmpty(&EfiTimerList)) {
            Event = CR(EfiTimerList.Flink, IEVENT, Timer.Link, EVENT_SIGNATURE);

            if (Event->Timer.TriggerTime <= EfiSystemTime) {
                SignalEvent (EfiCheckTimerEvent);
            }
        }

        ReleaseLock (&EfiSystemTimeLock);
    }
}


VOID
INTERNAL
CheckTimers (
    IN EFI_EVENT            CheckEvent,
    IN VOID                 *Context
    )
/*++

Routine Description:

    Checks the sorted timer list against the current system time.
    Signals any expired event timer.


Arguments:

    Event       - Not used

    Context     - Not used

Returns:

    None

--*/
{
    UINT64                  SystemTime;
    IEVENT                  *Event;

    //
    // Check the timer database for expired timers
    //

    AcquireLock (&EfiTimerListLock);
    SystemTime = CurrentSystemTime();

    while (!IsListEmpty(&EfiTimerList)) {
        Event = CR(EfiTimerList.Flink, IEVENT, Timer.Link, EVENT_SIGNATURE);

        //
        // If this timer is not expired, then we're done
        //

        if (Event->Timer.TriggerTime > SystemTime) {
            break;
        }

        //
        // Remove this timer from the timer queue
        //

        RemoveEntryList (&Event->Timer.Link);
        Event->Timer.Link.Flink = NULL;

        //
        // Signal it
        //

        SignalEvent (Event);

        //
        // If this is a periodic timer, set it
        //

        if (Event->Timer.Period) {

            //
            // Compute the timers new trigger time
            //

            Event->Timer.TriggerTime = Event->Timer.TriggerTime + Event->Timer.Period;

            //
            // If that's before now, then reset the timer to start from now
            //

            if (Event->Timer.TriggerTime <= SystemTime) {
                Event->Timer.TriggerTime = SystemTime;
                SignalEvent (EfiCheckTimerEvent);
            }

            //
            // Add the timer
            //

            InsertEventTimer (Event);
        }
    }

    ReleaseLock (&EfiTimerListLock);
}

VOID
STATIC
InsertEventTimer (
    IN IEVENT       *Event
    )
/*++

Routine Description:

    Inserts the

Arguments:

    Event       - Not used

    Context     - Not used

Returns:

    None

--*/
{
    UINT64          TriggerTime;
    LIST_ENTRY      *Link;
    IEVENT          *Event2;

    ASSERT_LOCKED (&EfiTimerListLock);

    //
    // Get the timer's trigger time
    //

    TriggerTime = Event->Timer.TriggerTime;

    //
    // Insert the timer into the timer database in assending sorted order
    //

    for (Link=EfiTimerList.Flink; Link  != &EfiTimerList; Link = Link->Flink) {
        Event2 = CR(Link, IEVENT, Timer.Link, EVENT_SIGNATURE);

        if (Event2->Timer.TriggerTime > TriggerTime) {
            break;
        }
    }

    InsertTailList (Link, &Event->Timer.Link);
}


EFI_STATUS
BOOTSERVICE
SetTimer (
    IN EFI_EVENT            UserEvent,
    IN EFI_TIMER_DELAY      Type,
    IN UINT64               TriggerTime
    )
{
    IEVENT      *Event;

    Event = UserEvent;

    if (Event == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (Event->Signature != EVENT_SIGNATURE) {
      return EFI_INVALID_PARAMETER;
    }

    if (Type >= TimerTypeMax  || !(Event->Type & EVT_TIMER)) {
        return EFI_INVALID_PARAMETER;
    }

    AcquireLock (&EfiTimerListLock);

    //
    // If the timer is queued to the timer database, remove it
    //

    if (Event->Timer.Link.Flink) {
        RemoveEntryList (&Event->Timer.Link);
        Event->Timer.Link.Flink = NULL;
    }

    Event->Timer.TriggerTime = 0;
    Event->Timer.Period = 0;

    if (Type != TimerCancel) {

        if (Type == TimerPeriodic) {
            Event->Timer.Period = TriggerTime;
        }

        Event->Timer.TriggerTime = CurrentSystemTime() + TriggerTime;
        InsertEventTimer (Event);

        if (!TriggerTime) {
            SignalEvent (EfiCheckTimerEvent);
        }
    }

    ReleaseLock (&EfiTimerListLock);
    return EFI_SUCCESS;
}
