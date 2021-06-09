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

    lib.h

Abstract:

    EFI library header files



Revision History

--*/


#include "efi.h"
#include "efilib.h"
#include "EfiRtLib.h"

//
// Include non architectural protocols
//
#include "efivar.h"
#include "legacyBoot.h"
#include "intload.h"
#include "VgaClass.h"
#include "EfiConSplit.h"
#include "AdapterDebug.h"

#include "EfiGpt.h"
#include "LibSmbios.h"

#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(LoadedImage)
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION(DriverDiagnostics)
#include EFI_PROTOCOL_DEFINITION(ComponentName)
#include EFI_PROTOCOL_DEFINITION(BusSpecificDriverOverride)
#include EFI_PROTOCOL_DEFINITION(PlatformDriverOverride)
#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION(PciIo)

//
// Prototypes
//

VOID
InitializeGuid (
    VOID
    );

INTN
LibStubStriCmp (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *S1,
    IN CHAR16                           *S2
    );

BOOLEAN
LibStubMetaiMatch (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *String,
    IN CHAR16                           *Pattern
    );

VOID
LibStubStrLwrUpr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *Str
    );

BOOLEAN
LibMatchDevicePaths (
    IN  EFI_DEVICE_PATH *Multi,
    IN  EFI_DEVICE_PATH *Single
    );

EFI_DEVICE_PATH *
LibDuplicateDevicePathInstance (
    IN EFI_DEVICE_PATH  *DevPath
    );


//
// Globals
//
extern BOOLEAN                          LibInitialized;
extern BOOLEAN                          LibFwInstance;
extern SIMPLE_TEXT_OUTPUT_INTERFACE     *LibRuntimeDebugOut;
extern EFI_UNICODE_COLLATION_INTERFACE  *UnicodeInterface;
extern EFI_UNICODE_COLLATION_INTERFACE  LibStubUnicodeInterface;
extern EFI_RAISE_TPL                    LibRuntimeRaiseTPL;
extern EFI_RESTORE_TPL                  LibRuntimeRestoreTPL;
