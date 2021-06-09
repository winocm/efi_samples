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

    8259.c

Abstract: 

    8259 specific functions.

Revision History

--*/


#include "efi.h"
#include "efilib.h"
#include "8259.h"
#include "..\PlIntCtrl.h"
#include "EfiLdrHandoff.h"

#define IVT_REDIRECT_BASE ((VOID *)(pic1RedirBase * 4))

static UINT8 pic1RedirBase = 0x68;

INT_CTRL_MASK
BOOTSERVICE
PlGetIntrCtrlMask(
    )
/*++
Routine Description:

Reads the INT_CTRL_MASK from the interrupt controller and returns it.  The
definition of INT_CTRL_MASK may vary depending on the interrupt controller type.
For a dual 8259 system, INT_CTRL_MASK is simply a UINT16 where the low byte is
the master pic mask and the high byte is the slave PIC mask.
--*/
{
    INT_CTRL_MASK ReturnValue;

    __asm
    {
        xor     eax, eax
        in      al, 0xa1
        shl     eax, 8
        in      al, 0x21
        mov     ReturnValue, ax
    }
    return ReturnValue;
}
    
VOID
BOOTSERVICE
PlSetIntrCtrlMask(
    IN INT_CTRL_MASK Mask
    )
/*++
Routine Description:

Sets the requested mask in the interrupt controller.

Arguments:
    Mask - The mask to set.
        For a dual 8259 system, INT_CTRL_MASK is simply a UINT16 where the low
        byte is the master pic mask and the high byte is the slave PIC mask.
--*/
{
    __asm
    {
        xor     eax, eax
        mov     ax, Mask
        out     0x21, al
        shr     ax, 8
        out     0xa1, al
    }
}
    
VOID
BOOTSERVICE
PlMapIrqToVect(
    INT_CTRLR_MODE Mode
    )
/*++
Routine Description:

Sets up the mapping of hardware IRQ to INT number in the interrupt controller.
Also sets up real mode IVT to redirect INT 0x68 - 0x6f to INT 0x08 - 0x0f so
real mode interrupts will function during a BIOS thunk without having to
completely reconfigure the interrupt controller (only the mask must be saved and
restored for each think).  This call is made once during EFI init, and once if a
legacy boot via INT 19 is done.

BEWARE: Fixing the redirection range to 0x68-0x6F does not generally work. Many
        video cards use INT6D for their purposes, and other issues may also
        exist with other vectors. A somewhat safer mechanism is to discover a
        block of unused vectors dynamically.

Arguments:
    Mode = INT_CTRLR_EFIMODE: IRQ 0-15 = INT 0x68-0x6f; IRQ 8-15 = INT 0x70-0x78
           INT_CTRLR_BIOSMODE: IRQ 0-7 = INT 0x08-0x0f; IRQ 8-15 = INT 0x70-0x78
--*/
{
    static INT_CTRLR_MODE CurrentMode = INT_CTRLR_BIOSMODE;
    static UINT16 SavedVectors [8][2];  // 8 vectors of IVT to save
    static const UINT16 NewVectors [8][2] = // redirection - addresses fixed in start.asm
        {{0xfe0, EFILDR_BASE_SEGMENT},
         {0xfe3, EFILDR_BASE_SEGMENT},
         {0xfe6, EFILDR_BASE_SEGMENT},                                
         {0xfe9, EFILDR_BASE_SEGMENT},                                
         {0xfec, EFILDR_BASE_SEGMENT},                                
         {0xfef, EFILDR_BASE_SEGMENT},                                
         {0xff2, EFILDR_BASE_SEGMENT},                                
         {0xff5, EFILDR_BASE_SEGMENT}};
                                         
    if (Mode == INT_CTRLR_EFIMODE && CurrentMode == INT_CTRLR_BIOSMODE)         // Switch to EFI intr ctrl config
    {
        // Initialize master PIC
        __asm
        {
            mov     al, 0x11
            out     0x20, al                // ICW1 (bit 4 set)... Slave attached
            mov     al, pic1RedirBase
            out     0x21, al                // ICW2 - IRQ0 == INT 0x68
            mov     al, 0x04
            out     0x21, al                // ICW3 - slave connected to pin 2
            mov     al, 0x01
            out     0x21, al                // ICW4 - 8086 mode
        }
        // save real mode vectors 68h to 6fh to be restored later...
        CopyMem ((VOID *) SavedVectors, IVT_REDIRECT_BASE, 32);
        // redirect real mode vectors 68h to 6fh to 8h - 0fh
        CopyMem (IVT_REDIRECT_BASE, (VOID *) NewVectors, 32);
    }
    else if (Mode == INT_CTRLR_BIOSMODE && CurrentMode == INT_CTRLR_EFIMODE)    // Switch to BIOS intr ctrl config
    {
        // Initialize master PIC
        __asm
        {
            mov     al, 0x11
            out     0x20, al                // ICW1 (bit 4 set)... Slave attached
            mov     al, 0x8
            out     0x21, al                // ICW2 - IRQ0 == INT 0x08
            mov     al, 0x04
            out     0x21, al                // ICW3 - slave connected to pin 2
            mov     al, 0x01
            out     0x21, al                // ICW4 - 8086 mode
        }

        // restore real mode vectors 68h to 6fh
        CopyMem (IVT_REDIRECT_BASE, (VOID *) SavedVectors, 32);
    }
    CurrentMode = Mode;         // save new current mode
}

VOID
PlEnableTimerInterrupt(
    VOID
    )
{
//    outp(0x21, inp(0x21) & ~1);   // clear bit 0 in master PIC mask to enable timer interrupt
    __asm
    {
        in      al, 0x21
        and     al, 0xfe
        out     0x21, al
    }
}    
    
VOID
PlDisableTimerInterrupt(
    VOID
    )
{
//    outp(0x21, inp(0x21) | 1);    // set bit 0 in master PIC mask to enable timer interrupt
    __asm
    {
        in      al, 0x21
        or      al, 1
        out     0x21, al
    }
}    

VOID
PlEOI(
    VOID
    )
{
    __asm
    {
        mov     al, 0x20
        out     0xa0, al
        out     0x20, al
    }
}

UINT8
PlGetVectorFromIrq(
    UINT8 irq
    )
{
    if (irq < 8)
        return pic1RedirBase + irq;
    if (irq < 16)
        return 0x70 + irq;
    return 0;
}
