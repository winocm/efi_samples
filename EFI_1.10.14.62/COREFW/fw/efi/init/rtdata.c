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

    rtdata.c

Abstract:

    Firmware runtime global data



Revision History

--*/

#include "efifw.h"

extern INTERNAL EFI_BOOT_SERVICES       BootServices;
extern INTERNAL EFI_RUNTIME_SERVICES    RuntimeServices;

//
// These globals are runtime globals
//
// N.B. The Microsoft C compiler will only put the data in the
// right data section if it is explicitly initialized..
//

#pragma BEGIN_RUNTIME_DATA()

//
// SystemTable - EFI system table
//

INTERNAL 
EFI_SYSTEM_TABLE SystemTable = {
    {
        EFI_SYSTEM_TABLE_SIGNATURE,
        EFI_SYSTEM_TABLE_REVISION,
        sizeof (EFI_SYSTEM_TABLE),
        0,
    },
    NULL,                           // FirmwareVendor
    0,                              // FirmwareRevision

    NULL,                           // ConsoleInHandle
    NULL,                           // ConIn

    NULL,                           // ConsoleOutHandle
    NULL,                           // ConOut

    NULL,                           // StandardErrorHandle
    NULL,                           // StdErr

    &RuntimeServices,               // RunTimeServices Table
    &BootServices,                  // BootServices Table

    0,                              // NumberOfTableEntries
    NULL                            // ConfigurationTable
} ;


INTERNAL EFI_RUNTIME_SERVICES     RuntimeServices = { 
    {
        EFI_RUNTIME_SERVICES_SIGNATURE,
        EFI_RUNTIME_SERVICES_REVISION,
        sizeof (EFI_RUNTIME_SERVICES),
        0
    },

    NULL,                   // GetTime
    NULL,                   // SetTime
    NULL,                   // GetWakeupTime
    NULL,                   // SetWakeupTime

    RtSetVirtualAddressMap,
    RtConvertPointer,

    RtGetVariable,       
    RtGetNextVariableName,
    RtSetVariable,

    GetNextHighMonotonicCount,
    NULL                    // ResetSystem
};
    
//
// EfiAtRuntime - indicates if ExitBootServices has been performed
//

BOOLEAN EfiAtRuntime = FALSE;

//
// EfiVirtualMode - indicates that SetVirtualAddressMap has been performed
//

BOOLEAN EfiVirtualMode = FALSE;
