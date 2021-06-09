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

    bsdata.c

Abstract:

    Firmware boot service global data



Revision History

--*/

#include "efifw.h"


//
// EFI's system table
// EFI's boot service table
//
extern INTERNAL EFI_BOOT_SERVICES       BootServices;
   

//
// BootServices - EFI firmware boot services table
//

INTERNAL 
EFI_BOOT_SERVICES BootServices = {
    {
        EFI_BOOT_SERVICES_SIGNATURE,
        EFI_BOOT_SERVICES_REVISION,
        sizeof (EFI_BOOT_SERVICES),
        0
    },

    RaiseTPL,
    RestoreTPL,

    BootServiceAllocatePages,
    FreePages,
    GetMemoryMap,
    BootServiceAllocatePool,
    BSFreePool,

    CreateEvent,
    SetTimer,
    WaitForEvent,    
    SignalEvent,
    CloseEvent,
    CheckEvent,

    InstallProtocolInterface,
    ReinstallProtocolInterface,
    UninstallProtocolInterface,
    HandleProtocol,
    NULL,
    RegisterProtocolNotify,
    LocateHandle,
    LocateDevicePath,
    InstallConfigurationTable,

    LoadImage,
    StartImage,
    Exit,
    BSUnloadImage,
    ExitBootServices,

    GetNextMonotonicCount,
    NULL,                           // Stall
    NULL,                           // SetWatchdogTimer 

    ConnectController,
    DisconnectController,

    OpenProtocol,
    CloseProtocol,
    OpenProtocolInformation,

    ProtocolsPerHandle,
    LocateHandleBuffer,
    LocateProtocol,    

    InstallMultipleProtocolInterfaces,
    UninstallMultipleProtocolInterfaces,

    RtCalculateCrc32,
    EfiCoreCopyMem,
    EfiCoreSetMem
};

//
// FirmwareTable - EFI firmware table, exported to the environment emulator
//


INTERNAL
EFI_FIRMWARE_TABLE FirmwareTable = {

    FwMemoryMapInstalled,
    FwNvStoreInstalled,
    
    FwAddMemoryDescriptor,

    FwTimerTick,
    FwLoadInternal
};

//
//
//

EFI_PLATFORM_TABLE *PL;

