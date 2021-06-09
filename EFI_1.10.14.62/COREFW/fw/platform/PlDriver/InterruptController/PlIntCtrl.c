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

    PlIntCtrl.c

Abstract: 

    Platform functions for accessing/modifying the interrupt controller.

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlIntCtrl.h"
#include "8259.h"

VOID
BOOTSERVICE
PlSetupInterruptControllerMask (
    IN INT_CTRLR_MODE Mode
    )
/*++

Routine Description:

This routine saves the current interrupt controller mask, and sets the interrupt
controller mask up for the new mode.  The mask for the opposite mode is saved in
private static data.

This call is made once during EFI initialization, and twice during each BIOS
thunk.

Interrupts must be disabled when PlSetupInterruptControllerMask is called.

It is assumed that the first time this function is called, the interrupt
controller mask has been configured by the BIOS

Arguments:

    Mode        INT_CTRLR_EFIMODE == setup for EFI
                INT_CTRLR_BIOSMODE == setup for BIOS

--*/
{
    static INT_CTRLR_MODE CurrentMode = INT_CTRLR_BIOSMODE;
    static INT_CTRL_MASK 
        EfiMask = DEFAULT_EFI_MASK, 
        BiosMask;
    
    if (Mode == INT_CTRLR_EFIMODE && CurrentMode == INT_CTRLR_BIOSMODE)         // Switch to EFI intr ctrl config
    {
        BiosMask = PlGetIntrCtrlMask();         // save BIOS mask
        PlSetIntrCtrlMask(EfiMask);             // Restore EfiMask
    }
    else if (Mode == INT_CTRLR_BIOSMODE && CurrentMode == INT_CTRLR_EFIMODE)    // Switch to BIOS intr ctrl config
    {

        EfiMask = PlGetIntrCtrlMask();          // save EFI mask
        PlSetIntrCtrlMask(BiosMask);            // Restore BIOS mask
    }
    CurrentMode = Mode;         // save new current mode
}


VOID
BOOTSERVICE
PlSetupInterruptController (
    IN INT_CTRLR_MODE Mode
    )
/*++

Routine Description:

This routine simply sets up the interrupt controller to function with EFI.  It is
called once during EFI initialization, and once again later to restore BIOS settings
if a legacy boot via INT 19 is invoked.

The mode parameter is not checked, so care should be taken to insure the interrupt
controller is not already in the mode the caller is trying to set.  If this happens,
corruption of private data (interrupt masks, IVT entries, etc.) could result.

Interrupts must be disabled when PlSetupInterruptController is called.

Arguments:

    Mode        INT_CTRLR_EFIMODE == setup for EFI
                INT_CTRLR_BIOSMODE == setup for BIOS

--*/
{
    PlMapIrqToVect(Mode);                   // Program 8259 vectors for selected mode
    PlSetupInterruptControllerMask(Mode);
}

