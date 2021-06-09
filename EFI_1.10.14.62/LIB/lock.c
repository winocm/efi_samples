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

    lock.c

Abstract:

    Implements FLOCK



Revision History

--*/


#include "lib.h"


VOID
InitializeLock (
    IN OUT FLOCK    *Lock,
    IN EFI_TPL      Priority
    )
/*++

Routine Description:

    Initialize a basic mutual exclusion lock.   Each lock
    provides mutual exclusion access at it's task priority
    level.  Since there is no-premption (at any TPL) or
    multiprocessor support, acquiring the lock only consists
    of raising to the locks TPL.

    Note on a debug build the lock is acquired and released
    to help ensure proper usage.
    
Arguments:

    Lock        - The FLOCK structure to initialize

    Priority    - The task priority level of the lock

    
Returns:

    An initialized F Lock structure.

--*/
{
    Lock->Tpl = Priority;
    Lock->OwnerTpl = 0;
    Lock->Lock = 0;
}


VOID
AcquireLock (
    IN FLOCK    *Lock
    )
/*++

Routine Description:

    Raising to the task priority level of the mutual exclusion
    lock, and then acquires ownership of the lock.
    
Arguments:

    Lock        - The lock to acquire
    
Returns:

    Lock owned

--*/
{
    RtAcquireLock (Lock);
}


VOID
ReleaseLock (
    IN FLOCK    *Lock
    )
/*++

Routine Description:

    Releases ownership of the mutual exclusion lock, and
    restores the previous task priority level.
    
Arguments:

    Lock        - The lock to release
    
Returns:

    Lock unowned

--*/
{
    RtReleaseLock (Lock);
}
