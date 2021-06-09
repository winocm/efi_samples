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
#include "salefi.h"
#include "SalProc.h"
#include "Int86.h"
#include "efiDevp.h"

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
    BOOLEAN                 CarryFlag;
    rArg                    Results = {-1,0,0,0};

    BbsDevicePath = (BBS_BBS_DEVICE_PATH *)DevicePath;
    Print (L"\n%HLegacy Boot%N called with device type %x\n", BbsDevicePath->DeviceType);
    
    //
    // For now read first 512 bytes of media to 0:7C00 and then do SAL callback
    // NOTE: Assuming block size is 512 bytes of media
    
    //
    // Setup the int13 call for reading 1 sector
    //

    if (BbsDevicePath->DeviceType == BBS_TYPE_HARDDRIVE) {
        Regs.h.DL = 0x80;   // 'C' hard drive
    } else if (BbsDevicePath->DeviceType == BBS_TYPE_FLOPPY) {
        Regs.h.DL = 0x00;   // 'A' floppy
    } else {
        return EFI_UNSUPPORTED;
    }

    Regs.h.AH = 0x02;       // function number
    Regs.h.AL = 0x01;       // 1 sector to read
    Regs.x.BX = 0x7c00;     // read to 0x7c00
    Regs.h.CH = 0x00;       // cylinder = 0
    Regs.h.CL = 0x01;       // read 1st sector
    Regs.x.ES = 0x0000;     // segment 0
    Regs.h.DH = 0x00;       // head = 0
    
    CarryFlag = TRUE;
    CarryFlag = Int86 (0x13, &Regs);

    if (!CarryFlag) {
        // read data to 0:7c00. Now call SAL to do the rest

        //
        // call SAL callback to set handoff state
        //

        RtSalCallBack(
            ID_SALCB_BOOTIA32OS, 0, 0, 0,
            0,0,0,0,
            &Results
        );

        if (Results.p0) {
            Print (L"\n%HLegacy Boot%N called with device type %x failed\n", BbsDevicePath->DeviceType);
        }
        return EFI_SUCCESS;
    } else {
        Print (L"\nCould not read from device %x \n", BbsDevicePath->DeviceType);
        return EFI_UNSUPPORTED;
    }

}
