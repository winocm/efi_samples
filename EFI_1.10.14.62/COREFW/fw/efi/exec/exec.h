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

    event.h

Abstract:

    



Revision History

--*/


#include "efifw.h"


extern UINTN EventPending;
#define VALID_TPL(a)            ((a) <= TPL_HIGH_LEVEL)

//
// EFI_EVENT
//

#define EVENT_SIGNATURE         EFI_SIGNATURE_32('e','v','n','t')
typedef struct {
    INTN                    Signature;
    UINT32                  Type;
    UINT32                  SignalCount;

    //
    // Entry if the event is registered to be signalled
    //

    LIST_ENTRY              SignalLink;

    //
    // Notification information for this event
    //

    EFI_TPL                 NotifyTpl;
    EFI_EVENT_NOTIFY        NotifyFunction;
    VOID                    *NotifyContext;
    LIST_ENTRY              NotifyLink;         
    
    //
    // A list of all runtime events
    //

    LIST_ENTRY              RuntimeLink;

    //
    // Information by event type
    //

    union {
   
        //
        // For timer events
        //

        struct {
            LIST_ENTRY      Link;
            UINT64          TriggerTime;
            UINT64          Period;
        } Timer;
    } ;

} IEVENT;    

//
// Internal prototypes
//

VOID
DispatchEventNotifies (
    IN EFI_TPL      Priority
    );

UINTN
HighestSetBit (
    IN UINTN         i
    );


BOOLEAN
INTERNAL RUNTIMESERVICE
GetInterruptState (
    VOID               
    );

BOOLEAN
INTERNAL STATIC
SetInterruptStateThunk (
    IN BOOLEAN      Enable
    );

//
// Internal Global data
//

extern INTERNAL FLOCK EventQueueLock;
extern INTERNAL UINTN EventPending;
extern INTERNAL LIST_ENTRY EventQueue[];
extern INTERNAL UINTN EventPending;
extern INTERNAL LIST_ENTRY EventSignalQueue[];
extern INTERNAL LIST_ENTRY RuntimeEventList;
extern INTERNAL EFI_TPL TPL;
extern INTERNAL UINT8 HSB[];

extern INTERNAL UINT64 EfiMtc;
extern INTERNAL EFI_EVENT EfiMtcEvent;
extern INTERNAL CHAR16 *EfiMtcName;
extern INTERNAL EFI_GUID EfiMtcGuid;
