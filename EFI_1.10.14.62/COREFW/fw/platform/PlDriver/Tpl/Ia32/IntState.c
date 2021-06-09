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

    IntState.c

Abstract: 

    Mils pl interfaces that are IA-32 specific


Revision History

--*/

#include "efi.h"
#include "efilib.h"

BOOLEAN
PlSetInterruptState (
    IN BOOLEAN      Enable
    )
/*++

Routine Description:

    This routine sets the interrupt state (psr.i)

Arguments:

    Interrupt state to set

Returns:
    old interrupt state
   

--*/
{
    UINT32  OldEflags;
    
    __asm   pushfd
    __asm   pop         eax
    __asm   mov         OldEflags, eax
    
    if (Enable)
        __asm   sti
    else
        __asm   cli

    return ((OldEflags & (1 << 9)) != 0);
}
