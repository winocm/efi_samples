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

    interrupt.c

Abstract: 

    CPU architecuture specific routines for managing interrupts.

--*/

#include "efi.h"

VOID
InstallInterruptHandler(
    UINT16 Vector,
    void (*Handler)(void)
    )
{
    __asm {
        pushfd                              // save eflags
        cli                                 // turn off interrupts
        sub     esp, 6                      // open some space on the stack
        mov     edi, esp
        sidt    es:[edi]                    // get fword address of IDT
        mov     edi, es:[edi+2]             // move offset of IDT into EDI
        add     esp, 6                      // correct stack
        movzx   eax, Vector                 // Get vector number
        shl     eax, 3                      // multiply by 8 to get offset
        add     edi, eax                    // add to IDT base to get entry
        mov     eax, Handler                // load new address into IDT entry
        mov     word ptr es:[edi], ax       // write bits 15..0 of offset
        shr     eax, 16                     // use ax to copy 31..16 to descriptors
        mov     word ptr es:[edi+6], ax     // write bits 31..16 of offset
        popfd                               // restore flags (possible enabling interrupts)
    }
}
