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

    InternalShell.c
    
Abstract:

    Code to support launch of linked in Shell

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Plshell.h"
#include "IntLoad.h"

STATIC
EFI_STATUS
LoadInternalShellLoadFile (
    IN struct _EFI_LOAD_FILE_INTERFACE  *This,
    IN EFI_DEVICE_PATH                  *FilePath,
    IN BOOLEAN                          BootPolicy,
    IN OUT UINTN                        *BufferSize,
    IN VOID                             *Buffer OPTIONAL
    );



#define INTERNAL_SHELL_INSTANCE_STRUCTURE   'sisi'
typedef struct {
    UINTN           Signature;
    EFI_HANDLE      Handle;
    EFI_LOAD_FILE   LoadFile;
} INTERNAL_SHELL_INSTANCE;


#define UI_SHELL_NAME       L"EFI Shell [Built-in]"
#define UI_SHELL_NAME_SIZE  21

#pragma pack(1)
typedef struct {
    VENDOR_DEVICE_PATH  VendorDevicePath;
    EFI_DEVICE_PATH     EndDevicePath;
} INTERNAL_SHELL_DEVICE_PATH;

typedef struct {
    UINT32                      Attribute;
    UINT32                      FilePathListLength;
    CHAR16                      Name[UI_SHELL_NAME_SIZE];
    INTERNAL_SHELL_DEVICE_PATH  DevicePath;
} INTERNAL_SHELL_BOOT_OPTION;
#pragma pack()  

//
// UI String.Data
//
STATIC UI_STRING_ENTRY    InternalShellUiEntry[] = {
    { LanguageCodeEnglish, UI_SHELL_NAME },
    { NULL, NULL },
    { LanguageCodeEnglish, L"Intel EPG Lion BIOS Team - NO EXTRA CHARGE" },
    { NULL, NULL },
    { NULL, NULL }
};

STATIC UI_INTERFACE    InternalShellUi = { EFI_UI_VERSION, InternalShellUiEntry };

//
// Device Path Data
//
INTERNAL_SHELL_DEVICE_PATH InternalShellDevicePath = {
    {{ HARDWARE_DEVICE_PATH, HW_VENDOR_DP, sizeof(VENDOR_DEVICE_PATH) }, 
        INTERNAL_SHELL_GUID },
    { END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, sizeof(EFI_DEVICE_PATH) }
};




EFI_STATUS
PlInitializeInternalLoad (
    VOID
    )
{
    EFI_STATUS              Status;
    INTERNAL_SHELL_INSTANCE *Dev;

    Dev = AllocateZeroPool (sizeof(INTERNAL_SHELL_INSTANCE));
    Dev->Signature = INTERNAL_SHELL_INSTANCE_STRUCTURE;
    Dev->LoadFile = LoadInternalShellLoadFile;
    
    Status = LibInstallProtocolInterfaces (
                &Dev->Handle,
                &LoadFileProtocol,      &Dev->LoadFile,
                &UiProtocol,            &InternalShellUi,
                &DevicePathProtocol,    &InternalShellDevicePath,
                &InternalShellProtocol, NULL,
                NULL
                );

    return Status;
}

STATIC
EFI_STATUS
LoadInternalShellLoadFile (
    IN struct _EFI_LOAD_FILE_INTERFACE  *This,
    IN EFI_DEVICE_PATH                  *FilePath,
    IN BOOLEAN                          BootPolicy,
    IN OUT UINTN                        *BufferSize,
    IN VOID                             *Buffer OPTIONAL
    )
{
    if (*BufferSize < sizeof(VOID *)) {
        //
        // The core will call with a buffer size of NULL so it can find
        //  out how big the image is, so lets give them an answer.
        //  They will call back a second time to load the image.
        //
        *BufferSize = sizeof(VOID *);
        return EFI_BUFFER_TOO_SMALL;
    }

    PlStartShell();

    //
    // EFI_ALREADY_STARTED prevents error messages in the boot manager
    //
    return EFI_ALREADY_STARTED;
}

