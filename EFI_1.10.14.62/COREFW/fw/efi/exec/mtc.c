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

    mtc.c

Abstract:

    Monotonic counter

Revision History

--*/

#include "exec.h"

//
// Prototypes
//

EFI_STATUS
STATIC RUNTIMEFUNCTION
EfiMtcEventHandler (
    IN EFI_EVENT                Event,
    IN VOID                     *Context
    );

//
// Declare runtime functions
//

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(GetNextHighMonotonicCount)
#pragma RUNTIME_CODE(EfiMtcEventHandler)
#endif

//
//
//


VOID
InitializeMonotonicCount (
    VOID
    )
{
    UINT32              HighCount;
    UINTN               BufferSize;
    EFI_STATUS          Status;

    //
    // Initialize event to handle overflows
    //

    Status = CreateEvent (
                EVT_NOTIFY_SIGNAL,
                TPL_CALLBACK,
                EfiMtcEventHandler,
                NULL,
                &EfiMtcEvent
                );

    ASSERT (!EFI_ERROR(Status));

    //
    // Read the last high part
    //

    BufferSize = sizeof(HighCount);
    Status = RT->GetVariable (
                EfiMtcName,
                &EfiMtcGuid,
                NULL,
                &BufferSize,
                &HighCount
                );

    if (EFI_ERROR(Status)) {
        HighCount = 0;
    }

    //
    // Set the current value
    //

    EfiMtc = LShiftU64(HighCount, 32);
    EfiMtcEventHandler(NULL, NULL);
}



EFI_STATUS
BOOTSERVICE
GetNextMonotonicCount (
    OUT UINT64                  *Count
    )
{
    UINT64                      PreviousMtc;
    UINT32                      i;

    //
    //If the Count parameter is NULL return EFI_INVALID_PARAMETER
    //
    if(!Count){
        return EFI_INVALID_PARAMETER;
    }
    
    //
    // If the MTC hasn't been initalized return nothing
    //

    if (!EfiMtc) {
        ASSERT (EfiMtc);
        *Count = 0;
        return EFI_NOT_READY;
    }

    //
    // To cheap to code excahnge-add - just use a lock for now
    //

    AcquireLock (&EventQueueLock);
    PreviousMtc = EfiMtc;
    EfiMtc = EfiMtc + 1;
    ReleaseLock (&EventQueueLock);

    //
    // Return the new count
    //

    *Count = PreviousMtc;

    //
    // If the MSB bit of the low part toggled, then signal that the high
    // part needs updated now
    //

    i = ((UINT32) EfiMtc) ^ ((UINT32) PreviousMtc);
    if (i & 0x80000000) {
        SignalEvent (EfiMtcEvent);                     
    }

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
RUNTIMEFUNCTION
EfiMtcEventHandler (
    IN EFI_EVENT                Event,
    IN VOID                     *Context
    )
{
    UINT32                      HighCount;
    EFI_STATUS                  Status;

    //
    // To cheap to code compare-exchange - just use a lock for now
    //

    AcquireLock (&EventQueueLock);
    HighCount = (UINT32) RShiftU64(EfiMtc, 32) + 1;
    EfiMtc = LShiftU64(HighCount, 32);
    ReleaseLock (&EventQueueLock);

    //
    // Update the NvRam store to match the new high part
    //

    Status = RT->SetVariable (
                EfiMtcName,
                &EfiMtcGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(HighCount),
                &HighCount
                );

    return Status;
}


EFI_STATUS
RUNTIMEFUNCTION
GetNextHighMonotonicCount (
    OUT UINT32                  *HighCount
    )
{
    EFI_STATUS                  Status;


    if (!EfiAtRuntime) {
        return EFI_UNSUPPORTED;
    }

    if (HighCount == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Update high part
    //

    Status = EfiMtcEventHandler (NULL, NULL);

    //
    // Return the current high part
    //

    *HighCount = (UINT32) RShiftU64(EfiMtc, 32);
    return Status;
}
