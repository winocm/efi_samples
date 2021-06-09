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

    LegacyBoot.c
    
Abstract:

    Platform code to support Legacy boot.



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "LegacyBoot.h"
#include "PlLegacyBoot.h"
#include "Int86.h"
#include "efiDevp.h"
#include "PlTpl.h"
#include "PlIntCtrl.h"

STATIC EFI_STATUS
LegacyBoot(
    IN  EFI_DEVICE_PATH     *DevicePath
    );

static LEGACY_BOOT_INTERFACE   GlobalLegacyBoot;

EFI_STATUS
PlInitializeLegacyBoot (
    VOID
    )
{
    EFI_STATUS              Status;
    EFI_HANDLE              DummyHandle;

    DummyHandle = 0;
    GlobalLegacyBoot.BootIt = LegacyBoot;
    Status = LibInstallProtocolInterfaces (
                &DummyHandle,
                &LegacyBootProtocol,    &GlobalLegacyBoot,
                NULL
                );

    return Status;
}

STATIC
EFI_STATUS
LegacyBoot (
    IN EFI_DEVICE_PATH      *DevicePath
    )
{
    //
    // replace me with callout to platform code
    //
    IN  IA32_RegisterSet_t  Regs;
    BBS_BBS_DEVICE_PATH     *BbsDevicePath;
    BOOLEAN             SavedInterruptState;

    BbsDevicePath = (BBS_BBS_DEVICE_PATH *)DevicePath;
    Print (L"\n%HLegacy Boot%N called with device type %x\n", BbsDevicePath->DeviceType);
    
    //
    // Should call optional Protected mode BBS interface to set next boot.
    //
    
    SavedInterruptState = PlSetInterruptState(FALSE);   // insure interrupts are turned off before messing with interrupt controller
    PlMapIrqToVect(INT_CTRLR_BIOSMODE);                 // re-vector interrupt controller and restore IVT
    Int86 (0x19, &Regs);                                // Do INT 19
    PlMapIrqToVect(INT_CTRLR_EFIMODE);                  // Boot failed so set it back up for EFI
    PlSetInterruptState(SavedInterruptState);           // Restore interrupt flag
    
    return EFI_SUCCESS;
}
