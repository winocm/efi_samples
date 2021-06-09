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

Abstract:




Revision History

--*/

#include "exec.h"


//
// These globals are runtime globals
//

#pragma BEGIN_RUNTIME_DATA()

//
// EfiSetInterruptState - function to enable/disable interrupts
//

INTERNAL PL_SET_INTERRUPT_STATE EfiSetInterruptState = SetInterruptStateThunk;

//
// TPL - Task priority level
//

INTERNAL EFI_TPL  TPL = TPL_APPLICATION;

//
// HSB - Highest set bit conversion array
//

INTERNAL UINT8 HSB[256] = {
    0,  0,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
    5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

//
// EventQueueLock - Protects the event queus
//

INTERNAL FLOCK EventQueueLock = {0};

//
// EventQueue - A list of event's to notify for each priority level
// EventPending - A bitmask of the EventQueues that are pending
//

INTERNAL LIST_ENTRY EventQueue[TPL_HIGH_LEVEL+1] = {0};
INTERNAL UINTN EventPending = 0;

//
// EventSignalQueue - A list of events to signal when the EFI operation occurs
//

INTERNAL LIST_ENTRY EventSignalQueue[EVT_EFI_SIGNAL_MAX] = {0};

//
// RuntimeEventList - A list of all runtime events
//

INTERNAL LIST_ENTRY RuntimeEventList = {0};

//
// EfiMtc - The current Monotonic count value
//

INTERNAL UINT64 EfiMtc = 0;

//
// EfiMtcEvent - Event to use to update the Mtc's high part when wrapping
//

INTERNAL EFI_EVENT EfiMtcEvent = {0};

//
// EfiMtcName - Variable name of the MTC value
// EfiMtcGuid - Guid of the MTC value
//

INTERNAL CHAR16 *EfiMtcName = L"MTC";
INTERNAL EFI_GUID EfiMtcGuid = { 0xeb704011, 0x1402, 0x11d3, 0x8e, 0x77, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b };
