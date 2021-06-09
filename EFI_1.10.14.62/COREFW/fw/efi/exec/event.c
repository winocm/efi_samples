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
#include "..\hand\hand.h"

//
// Internal prototypes
//

//
// Event functions
//


VOID
InitializeEvent (
    VOID
    )
/*++

Routine Description:

    Initializes "event" support

Arguments:

    None
    
Returns:

    None

--*/
{
    UINTN        Index;


    for (Index=0; Index <= TPL_HIGH_LEVEL; Index++) {
        InitializeListHead (&EventQueue[Index]);
    }

    for (Index=0; Index < EVT_EFI_SIGNAL_MAX; Index++) {
        InitializeListHead (&EventSignalQueue[Index]);
    }

    InitializeListHead (&RuntimeEventList);
    InitializeLock (&EventQueueLock, TPL_HIGH_LEVEL);
    EventPending = 0;
}

#pragma RUNTIME_CODE(RtEventVirtualAddressFixup)
VOID
RUNTIMEFUNCTION
RtEventVirtualAddressFixup(
    VOID
    )
{
    EFI_CONVERT_POINTER     ConvertPointer;
    LIST_ENTRY              *Link;
    IEVENT                  *Event;
    UINTN                   Index;

    ConvertPointer = RT->ConvertPointer;

    //
    // Fix any notify function on any runtime events
    //

    for (Link=RuntimeEventList.Flink; Link != &RuntimeEventList; Link=Link->Flink) {
        Event = CR(Link, IEVENT, RuntimeLink, EVENT_SIGNATURE);

        if (Event->NotifyFunction) {
            ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Event->NotifyFunction);
        }

        if (Event->Type & EVT_RUNTIME_CONTEXT) {
            ConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &Event->NotifyContext);
        }
    }

    //
    // Fix list heads for queue & dispatching events
    //

    for (Index=0; Index < TPL_HIGH_LEVEL; Index++) {
        RtConvertList (EFI_INTERNAL_PTR, &EventQueue[Index]);
    }

    //
    // Fix the runtime event list
    //

    RtConvertList (EFI_INTERNAL_PTR, &RuntimeEventList);

    //
    // Fix the enable/disable interrupt thunk pointer
    //

    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &EfiSetInterruptState);

    //
    // EventSignalQueue is not needed anymore (since SetVirtualAddressMap has been called).
    // EfiTimerList is not needed at runtime.
    //
}


VOID
DispatchEventNotifies (
    IN EFI_TPL      Priority
    )
/*++

Routine Description:

    Dispatches all pending events.  
    Must be called at the TPL_NOTIFY

Arguments:

    None
    
Returns:

    None

--*/
{
    IEVENT          *Event;
    LIST_ENTRY      *Head;
    
    AcquireLock (&EventQueueLock);
    ASSERT(EventQueueLock.OwnerTpl == Priority);
    Head = &EventQueue[Priority];

    //
    // Dispatch all the pending notifications
    //

    while (!IsListEmpty(Head)) {
        
        Event = CR(Head->Flink, IEVENT, NotifyLink, EVENT_SIGNATURE);
        RemoveEntryList (&Event->NotifyLink);

        Event->NotifyLink.Flink = NULL;
        Event->SignalCount = 0;

        ReleaseLock (&EventQueueLock);
        
        //
        // Notify this event
        //

        Event->NotifyFunction (Event, Event->NotifyContext);

        //
        // Check for next pending event
        //

        AcquireLock (&EventQueueLock);
    }

    EventPending &= ~(1 << Priority);
    ReleaseLock (&EventQueueLock);
}


VOID
STATIC
NotifyEvent (
    IEVENT      *Event
    )
/*++

Routine Description:

    Queues the event's notification function to fire

Arguments:

    Event       - The Event to notify
    
Returns:

    None

--*/
{

    //
    // Event database must be locked
    //

    ASSERT_LOCKED (&EventQueueLock);

    //
    // If the event is queued somewhere, remove it
    //

    if (Event->NotifyLink.Flink) {
        RemoveEntryList (&Event->NotifyLink);
        Event->NotifyLink.Flink = NULL;
    }

    // 
    // Queue the event to the pending notification list
    //

    InsertTailList (&EventQueue[Event->NotifyTpl], &Event->NotifyLink);
    EventPending |= 1 << Event->NotifyTpl;
}


#pragma RUNTIME_CODE(RtNotifySignalList)
VOID
RUNTIMEFUNCTION
RtNotifySignalList (
    IN UINTN                SignalType
    )
/*++

Routine Description:

    Signals all events on the requested list

Arguments:

    SignalType      - The list to signal
    
Returns:

    None

--*/
{
    LIST_ENTRY              *Link, *Head;
    IEVENT                  *Event;

    SignalType = (SignalType & EVT_EFI_SIGNAL_MASK) - 1;
    ASSERT (SignalType < EVT_EFI_SIGNAL_MAX);

    AcquireLock (&EventQueueLock);

    Head = &EventSignalQueue[SignalType];
    for (Link = Head->Flink; Link != Head; Link = Link->Flink) {
        Event = CR(Link, IEVENT, SignalLink, EVENT_SIGNATURE);
        NotifyEvent (Event);
    }

    ReleaseLock (&EventQueueLock);
}


EFI_STATUS
BOOTSERVICE
CreateEvent (
    IN UINT32                   Type,
    IN EFI_TPL                  NotifyTpl,
    IN EFI_EVENT_NOTIFY         NotifyFunction,
    IN VOID                     *NotifyContext,
    OUT EFI_EVENT               *pEvent
    )
/*++

Routine Description:

    Create & initialize a new event structure

Arguments:

    Type                - The type of event structure to create

    NotifyType          - The task priority level of the NotifyFunction

    NotifyFunction      - The function to notify when the event is signalled

    NotifyContext       - The notification function's context value

Returns:

    *pEvent the new event structure

--*/
{
    IEVENT          *Event;
    EFI_STATUS      Status;
    
    if (pEvent == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Check to make sure no reserved flags are set
    //

    if (Type & ~(EVT_TIMER |
                 EVT_RUNTIME |
                 EVT_RUNTIME_CONTEXT |
                 EVT_NOTIFY_WAIT |
                 EVT_NOTIFY_SIGNAL |
                 EVT_SIGNAL_EXIT_BOOT_SERVICES |
                 EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE) ) {

        return EFI_INVALID_PARAMETER;
    }
    
    // Check that EVT_SIGNAL_EXIT_BOOT_SERVICES is exclusive, none of the other EVT_* can be used with it. 
    if (((Type & EVT_SIGNAL_EXIT_BOOT_SERVICES) == EVT_SIGNAL_EXIT_BOOT_SERVICES) &&
        (Type != EVT_SIGNAL_EXIT_BOOT_SERVICES)) {
      return EFI_INVALID_PARAMETER;
    }
  
    // Check that EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE is exclusive, none of the other EVT_* can be used with it. 
    if (((Type & EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE) == EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE) &&
        (Type != EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE)) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Is there's a notify type?
    //

    if (Type & (EVT_NOTIFY_WAIT | EVT_NOTIFY_SIGNAL)) {

        //
        // Can't supply both notify wait & notify signal
        //

        if ((Type & (EVT_NOTIFY_WAIT | EVT_NOTIFY_SIGNAL)) == (EVT_NOTIFY_WAIT | EVT_NOTIFY_SIGNAL)) {
            return EFI_INVALID_PARAMETER;
        }

        //
        // Check for an invalid NotifyFunction or NotifyTpl
        //

        if (!NotifyFunction || NotifyTpl <= TPL_APPLICATION || !VALID_TPL(NotifyTpl)) {
            return EFI_INVALID_PARAMETER;
        }

    } else {

        //
        // No notification needed, zero ignored values
        //

        NotifyTpl = 0;
        NotifyFunction = NULL;
        NotifyContext = NULL;
    }

    //
    // Allcoate and initialize a new event structure.
    //

    Status = BSAllocatePool (
                (Type & EVT_RUNTIME) ? EfiRuntimeServicesData: EfiBootServicesData,
                sizeof(IEVENT),
                &Event
                );

    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    ZeroMem (Event, sizeof(IEVENT));

    Event->Signature = EVENT_SIGNATURE;
    Event->Type = Type;
    
    Event->NotifyTpl = NotifyTpl;
    Event->NotifyFunction = NotifyFunction;
    Event->NotifyContext = NotifyContext;

    
    AcquireLock (&EventQueueLock);

    //
    // If the event is a runtime event, keep it on a special list.
    //

    if (Type & EVT_RUNTIME) {
        InsertTailList (&RuntimeEventList, &Event->RuntimeLink);
    }

    //
    // Is there an efi signal type?
    //

    Type = Type & EVT_EFI_SIGNAL_MASK;
    if (Type) {
        
        //
        // Put the event on the efi signal queue based on it's type
        //

        Type = Type - 1;
        ASSERT (Type < EVT_EFI_SIGNAL_MAX);

        InsertHeadList (&EventSignalQueue[Type], &Event->SignalLink);
    }

    ReleaseLock (&EventQueueLock);
    
    //
    // Done
    //

    *pEvent = Event;
    return EFI_SUCCESS;
}

EFI_STATUS
BOOTSERVICE
SignalEvent (
    IN EFI_EVENT    UserEvent
    )
/*++

Routine Description:

    Signals the event.  Queues the event to be notified if needed
    
Arguments:

    Event       - Event to signal
    
Returns:

    Status code

--*/
{
    IEVENT          *Event;

    Event = UserEvent;

    if (Event == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (Event->Signature != EVENT_SIGNATURE) {
      return EFI_INVALID_PARAMETER;
    }

    AcquireLock (&EventQueueLock);

    //
    // If the event is not already signalled, do so
    //

    if (!Event->SignalCount) {
        Event->SignalCount = Event->SignalCount + 1;

        //
        // If signalling type is a notify function, queue it
        //

        if (Event->Type & EVT_NOTIFY_SIGNAL) {
            NotifyEvent (Event);
        }
    }

    ReleaseLock (&EventQueueLock);
    return EFI_SUCCESS;
}

EFI_STATUS
BOOTSERVICE
CheckEvent (
    IN EFI_EVENT        UserEvent
    )
/*++

Routine Description:

    Check the status of an event
    
Arguments:

    Event       - Event to check
    
Returns:

    Status code

--*/

{
    IEVENT      *Event;
    EFI_STATUS  Status;

    Event = UserEvent;

    if (Event == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (Event->Signature != EVENT_SIGNATURE) {
      return EFI_INVALID_PARAMETER;
    }

    if (Event->Type & EVT_NOTIFY_SIGNAL) {
        return EFI_INVALID_PARAMETER;
    }

    Status = EFI_NOT_READY;

    if (!Event->SignalCount && (Event->Type & EVT_NOTIFY_WAIT)) {

        //
        // Queue the wait notify function
        //

        AcquireLock (&EventQueueLock);
        if (!Event->SignalCount) {
            NotifyEvent (Event);
        }
        ReleaseLock (&EventQueueLock);
    }

    //
    // If the even looks signalled, get the lock and clear it
    //

    if (Event->SignalCount) {
        AcquireLock (&EventQueueLock);

        if (Event->SignalCount) {
            Event->SignalCount = 0;
            Status = EFI_SUCCESS;
        }

        ReleaseLock (&EventQueueLock);
    }

    return Status;
}

EFI_STATUS
BOOTSERVICE
WaitForEvent (
    IN UINTN        NumberOfEvents,
    IN EFI_EVENT    *UserEvents,
    OUT UINTN       *UserIndex
    )
/*++

Routine Description:

    Stops execution until an event is signaled.
    
Arguments:

    NumberOfEvents  - The number of events in the UserEvents array

    UserEvents      - The array of events to wait on.

    UserIndex       - Pointer to the index of the event that satisfied the wait condition
    
Returns:

    Status code

--*/

{
    EFI_STATUS      Status;
    UINTN           Index;

    //
    // Can only WaitForEvent at TPL_APPLICATION
    //

    if (CurrentTPL() != TPL_APPLICATION) {
        //ASSERT (CurrentTPL() == TPL_APPLICATION);
        return EFI_UNSUPPORTED;
    }

    for(;;) {
        
        for(Index = 0; Index < NumberOfEvents; Index++) {

            Status = CheckEvent(UserEvents[Index]);

            if (!EFI_ERROR(Status)) {
                *UserIndex = Index;
                return EFI_SUCCESS;
            }

            if (Status != EFI_NOT_READY) {
                *UserIndex = -1;
                return Status;
            }
        }

        PL->IdleLoop(TRUE);
    }
}

EFI_STATUS
BOOTSERVICE
CloseEvent (
    IN EFI_EVENT        UserEvent
    )
/*++

Routine Description:

    Frees the event structure
    
Arguments:

    Event       - Event to close
    
Returns:

    Status code

--*/

{
    IEVENT      *Event;

    Event = UserEvent;

    if (Event == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (Event->Signature != EVENT_SIGNATURE) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // If it's a timer event, make sure it's not pending
    //

    if (Event->Type & EVT_TIMER) {
        SetTimer (Event, TimerCancel, 0);
    }

    AcquireLock (&EventQueueLock);

    //
    // If the event is queued somewhere, remove it
    //

    if (Event->RuntimeLink.Flink) {
        RemoveEntryList (&Event->RuntimeLink);
    }

    if (Event->NotifyLink.Flink) {
        RemoveEntryList (&Event->NotifyLink);
    }

    if (Event->SignalLink.Flink) {
        RemoveEntryList (&Event->SignalLink);
    }

    ReleaseLock (&EventQueueLock);

    //
    // If the event is registered on a protocol notify, then remove it from the protocol database
    //

    UnregisterProtocolNotify (Event);

    FreePool(Event);

    return EFI_SUCCESS;
}
