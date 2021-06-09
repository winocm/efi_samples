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

    emul.h

Abstract:

    Emulation application



Revision History

--*/

#define EFI_NT_EMUL     1

#include "windows.h"
#include "winioctl.h"

#include "efi.h"
#include "efilib.h"
#include "efivar.h"
#include "emulenv.h"

#include "stdio.h"
#include "memory.h"
#include "process.h"

#define _EFI_WIN_NT_H_

//
//
//

EFI_STATUS
PlInitializeWinNtThunkTable (
  );

EFI_STATUS
PlInstallDefaultIoDevice (
    IN EFI_DEVICE_PATH          *DevicePath,
    IN UINTN                    MemBase,
    IN UINTN                    IoBase
    );

VOID
PlInitConsole (
    IN CHAR16           *PipeName,
    OUT EFI_HANDLE      *Handle
    );

VOID
EFIFirmwareBanner (
    VOID
    );

VOID
PlPrintLogonBanner (
    VOID
    );

EFI_STATUS
PlEmulateLoad (
    IN CHAR16                   *InternalName,
    IN OUT EFI_LOADED_IMAGE     *ImageInfo,
    OUT EFI_IMAGE_ENTRY_POINT   *ImageEntryPoint
    );

VOID
PlSystemTimeToEfiTime (
    IN SYSTEMTIME       *SystemTime,
    OUT EFI_TIME        *Time
    );

EFI_STATUS
PlGetTime (
    IN EFI_TIME                 *Time,
    IN EFI_TIME_CAPABILITIES    *Capabilities OPTIONAL
    );

EFI_STATUS
RUNTIMEFUNCTION
PlTimeFieldsValid (
    IN EFI_TIME *Time
    );


EFI_STATUS
PlSetTime (
    IN EFI_TIME                 *Time
    );

EFI_STATUS
PlSetWakeupTime (
    IN BOOLEAN                      Enable,
    IN EFI_TIME                     *Time
    );

EFI_STATUS
PlGetWakeupTime (
    OUT BOOLEAN                     *Enabled,
    OUT BOOLEAN                     *Pending,
    OUT EFI_TIME                    *Time
    );

VOID
PlInitNvVarStoreEmul (
    IN UINTN    Attribute,
    IN UINTN    BankSize,
    IN UINTN    NoBanks
    );

EFI_STATUS
PlInitializeUnicodeStringDevice(
    VOID                                  
    );

//
// Globals
//

extern EFI_PLATFORM_TABLE   PlTable;
extern EFI_FIRMWARE_TABLE   *FW;
extern EFI_DEVICE_PATH      BiosRootDevicePath[];
extern DWORD NtLastTick;
extern BOOLEAN NtInterruptState;
