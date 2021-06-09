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

    Default.c

Abstract:

    EFI Boot Manager, default variable values


Revision History

--*/

#include "bm.h"
#include "IntLoad.h"

STATIC
VOID
BmBootOptionFromHandle (
    IN  EFI_HANDLE  Handle, 
    IN  UINT16      BootOptionNumber,
    LIST_ENTRY      *BootOptions,
    IN  CHAR16      *OptionName
    );


EFI_STATUS
BmSetDefaultDriverOptions (
    LIST_ENTRY  *DriverOptions
    )
{
    return EFI_SUCCESS;
}

EFI_STATUS
BmSetDefaultBootOptions (
    LIST_ENTRY  *BootOptions
    )
{
    EFI_STATUS          Status;
    UINTN               NumberFileSystemHandles;
    EFI_HANDLE          *FileSystemHandles;
    UINTN               NumberBlkIoHandles;
    EFI_HANDLE          *BlkIoHandles;
    EFI_BLOCK_IO        *BlkIo;
    UINTN               Index;
    UINT16              TimeOut;
    UINTN               OptionSize;
    UINT16              *BootOptionOrder;
    UINT16              BootOptionNumber;
    UINTN               NumberLoadFileHandles;
    EFI_HANDLE          *LoadFileHandles;
    VOID                *ProtocolInstance;
#ifdef EFI_BOOTSHELL    
    UINTN               NumberInternalShellHandles;
    EFI_HANDLE          *InternalShellHandles;
#endif

    //
    // Add Default Boot Options
    //
    BootOptionNumber = 0;

    //
    // Add removable media devices.
    //
    LibLocateHandle (ByProtocol, &BlockIoProtocol, NULL, &NumberBlkIoHandles, &BlkIoHandles);
    for (Index = 0; Index < NumberBlkIoHandles; Index++) {
        Status = BS->HandleProtocol (BlkIoHandles[Index], &BlockIoProtocol, (VOID **)&BlkIo);
        if (EFI_ERROR(Status)) {
            continue;
        }
        if (!BlkIo->Media->RemovableMedia) {
            
            //
            // Skip fixed Media device on first loop interration
            //
            continue;
        }

        BmBootOptionFromHandle (BlkIoHandles[Index], BootOptionNumber++, BootOptions, L"Boot");
    }
    if(NumberBlkIoHandles) {
        FreePool (BlkIoHandles);
    }

    //
    // Add Fixed Disk Devices.
    //
    LibLocateHandle (ByProtocol, &FileSystemProtocol, NULL, &NumberFileSystemHandles, &FileSystemHandles);
    for (Index = 0; Index < NumberFileSystemHandles; Index++) {
        Status = BS->HandleProtocol (FileSystemHandles[Index], &BlockIoProtocol, (VOID **)&BlkIo);
        if (!EFI_ERROR(Status)) {
            if (BlkIo->Media->RemovableMedia) {
                
                //             
                // If the file system handle supports a BlkIo protocol, skip removable media devices 
                //
                continue;
            }
        }
        //
        // If the FileSystem protocol does not contain a BlkIo protocol, that is O.K.
        //

        BmBootOptionFromHandle (FileSystemHandles[Index], BootOptionNumber++, BootOptions, L"Boot");
    }
    if(NumberFileSystemHandles) {
        FreePool (FileSystemHandles);
    }

    //
    // Add Network Boot
    //
    LibLocateHandle (ByProtocol, &SimpleNetworkProtocol, NULL, &NumberLoadFileHandles, &LoadFileHandles);
    for (Index = 0; Index < NumberLoadFileHandles; Index++) {
        Status = BS->HandleProtocol (LoadFileHandles[Index], &LoadFileProtocol, (VOID **)&ProtocolInstance);
        if (EFI_ERROR(Status)) {
            continue;
        }
        BmBootOptionFromHandle (LoadFileHandles[Index], BootOptionNumber++, BootOptions, L"Boot");
    }
    if(NumberLoadFileHandles) {
        FreePool (LoadFileHandles);
    }

#ifdef EFI_BOOTSHELL    
    //
    // Add the built in shell
    //
    LibLocateHandle (ByProtocol, &InternalShellProtocol, NULL, &NumberInternalShellHandles, &InternalShellHandles);
    for (Index = 0; Index < NumberInternalShellHandles; Index++) {
        Status = BS->HandleProtocol (InternalShellHandles[Index], &InternalShellProtocol, (VOID **)&ProtocolInstance);
        if (EFI_ERROR(Status)) {
            continue;
        }
        BmBootOptionFromHandle (InternalShellHandles[Index], BootOptionNumber++, BootOptions, L"Boot");
    }
    if (NumberInternalShellHandles) {
        FreePool (InternalShellHandles);
    }
#endif

    //
    // Set Boot Order list to match all BootOptions that were added
    //

    //
    // Pad an extra spot for the Internal Shell
    //
    OptionSize = (BootOptionNumber + 1) * sizeof(UINT16);
    BootOptionOrder = AllocatePool (OptionSize);
    for (Index = 0; Index < BootOptionNumber; Index++) {
        BootOptionOrder[Index] = (UINT16)Index;    
    }

    BmSetVariable (&BmBootOrder, BootOptionOrder, OptionSize);
    if (BootOptionOrder) {
        FreePool (BootOptionOrder);
    }

    //
    // Set timeout variable
    //
    TimeOut = 7;
    BmSetVariable (&BmTimeout, &TimeOut, sizeof(TimeOut));

    return EFI_SUCCESS;
}
    
STATIC
VOID
BmBootOptionFromHandle (
    IN  EFI_HANDLE  Handle, 
    IN  UINT16      BootOptionNumber,
    LIST_ENTRY      *BootOptions,
    IN  CHAR16      *OptionName
    )
{
    EFI_DEVICE_PATH *DevicePath;
    UINTN           SizeDevicePath;
    CHAR16          *UiStr;
    UINTN           SizeUiStr;
    UINT8           *Data;
    UINT8           *Option;
    UINTN           SizeOption;
    CHAR16          VariableName[20];

    DevicePath = DevicePathFromHandle (Handle);
    SizeDevicePath = DevicePathSize (DevicePath);
    UiStr = LibGetUiString (Handle, UiDeviceString, LanguageCodeEnglish, TRUE);
    if (!UiStr) {
        UiStr = DevicePathToStr (DevicePath);
    }
    SizeUiStr = StrSize (UiStr);

    SizeOption = sizeof(UINT32) + sizeof(UINT16) + SizeUiStr + SizeDevicePath;
    Option = Data = AllocateZeroPool (SizeOption);

    *(UINT32 *)Data = LOAD_OPTION_ACTIVE;
    Data += sizeof(UINT32);

    *(UINT16 *)Data = (UINT16)SizeDevicePath;
    Data += sizeof(UINT16);

    StrCpy ((CHAR16 *)Data, UiStr);
    Data += SizeUiStr;

    CopyMem (Data, DevicePath, SizeDevicePath); 

    SPrint (VariableName, 20, VarBootOption, BootOptionNumber);

    BmParseBootOption (
        VariableName, Option, SizeOption, OptionName, BootOptions
        );

    RT->SetVariable (
            VariableName, 
            &EfiGlobalVariable, 
            EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS ,
            SizeOption, 
            Option
            );

    FreePool (Option);
}
