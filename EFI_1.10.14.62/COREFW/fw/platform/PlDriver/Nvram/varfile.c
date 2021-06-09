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

    var.c

Abstract:

    handles variable store/reads

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "EfiVar.h"
#include "PlVar.h"

//
// 
//

#define VARIABLE_STORE_SIGNATURE    'vstr'
typedef struct {
    UINTN                   Signature;
    UINTN                   BankSize;
    VOID                    *Scratch;
    EFI_HANDLE              Handle;
    EFI_FILE_HANDLE         File;           // If opened BS store
    EFI_VARIABLE_STORE      VarStore;
    EFI_DEVICE_PATH         *DevicePath;
} VAR_DEV;

#define DEV_FROM_THIS(a) CR(a, VAR_DEV, VarStore, VARIABLE_STORE_SIGNATURE)

//
// Prototypes
//

VOID
STATIC RUNTIMEFUNCTION
RtVarBSDeviceSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    );

STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtVarBSClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN OUT VOID             *Scratch
    );

STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtVarBSReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    );


STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtVarBSUpdateStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *Buffer
    );

STATIC
VOID
VarOpenBSStore (
    IN VAR_DEV              *Dev,
    IN UINT64               OpenMode,
    OUT UINT64              *AvailableSpace
    );

STATIC
EFI_STATUS
VarBSClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN OUT VOID             *Scratch
    );

STATIC
EFI_STATUS
VarBSReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    );

STATIC
EFI_STATUS
VarBSUpdateStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *Buffer
    );

STATIC
EFI_STATUS
VarBSSizeStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                NoBanks
    );

//
//
//
VOID
PlInitNvVarStoreFile (
    IN UINTN    Attributes,
    IN UINTN    BankSize,
    IN UINTN    NoBanks
    )

{
    VAR_DEV                 *Dev;
    EFI_STATUS              Status;
    EFI_EVENT               Event;
    EFI_HANDLE              *Handles, Handle, BestHandle;
    UINTN                   NoHandles, Index;
    UINT64                  AvailableSpace, BestSpace;
    EFI_FILE_INFO           *Info;
    EFI_BLOCK_IO            *BlkIo;
    EFI_DEVICE_PATH         *DevicePath;
    VOID                    *Interface;

    Status = BS->AllocatePool (EfiRuntimeServicesData, sizeof(VAR_DEV), &Dev);
    ASSERT(!EFI_ERROR(Status));
    ZeroMem (Dev, sizeof(VAR_DEV));

    Dev->Signature = VARIABLE_STORE_SIGNATURE;
    Dev->BankSize  = BankSize;

    Status = BS->AllocatePool (EfiRuntimeServicesData, Dev->BankSize, &Dev->Scratch);
    ASSERT(!EFI_ERROR(Status));
    SetMem(Dev->Scratch, Dev->BankSize, 0xff);

    Dev->VarStore.Attributes = (UINT32)Attributes;

    //
    // Get all handles that support the FS protocol
    //

    LibLocateHandle (ByProtocol, &FileSystemProtocol, NULL, &NoHandles, &Handles);

    //
    // Check devices that starts with the BootStoreDevice device path
    //

    Dev->File = NULL;
    BestHandle = NULL;
    BestSpace = 0;
    for(Index = 0; Index < NoHandles; Index += 1) {
        Handle = Handles[Index];

        //
        // Only use EFI system partitions on non-removable media for simulated NVRAM store.
        //

        Status = BS->HandleProtocol (Handle,
                                     &BlockIoProtocol,
                                     (VOID*)&BlkIo
                                     );

        if (!EFI_ERROR(Status) && BlkIo != NULL) {
            if (!BlkIo->Media->MediaPresent) {
                continue;
            }
            if (BlkIo->Media->RemovableMedia) {
                continue;
            }
        }

        //
        // Get the device path of this device
        //

        Dev->DevicePath = DevicePathFromHandle (Handle);
        if (!Dev->DevicePath) {
            continue;
        }

        //
        // Check if this volume has the Boot Store file on it
        //

        VarOpenBSStore (Dev, 0, &AvailableSpace);

        //
        // If we got it open, return 
        //

        if (Dev->File) {
            BestSpace = AvailableSpace;
            BestHandle = Handle;
            break;
        }

        //
        // Check if this volume has the 
        //

        if (AvailableSpace > BestSpace) {
            BestSpace = AvailableSpace;
            BestHandle = Handle;
        }
    }

    //
    // Done with handle list
    //

    if (Handles) {
        FreePool (Handles);
    }

    //
    // If there's no file, then create one of the volume with the lagest
    // free space
    //

    if (!Dev->File) {

        //
        //  Create a device path for the file
        //

        if (BestHandle) {
            Dev->DevicePath = DevicePathFromHandle (BestHandle);
            if (Dev->DevicePath) {
                VarOpenBSStore (Dev, EFI_FILE_MODE_CREATE, &AvailableSpace);
                if (Dev->File) {
                    DEBUG((D_VAR|D_WARN, "VarAddBSStore: Created bootservices store file\n"));
                }
            }
        }
    }

    //
    // If we opened a file, check it
    //

    if (Dev->File) {
        //
        // Determine how many banks of storage are in the file
        //

        Info = LibFileInfo (Dev->File);
        if (Info) {
            Dev->VarStore.BankSize = (UINT32)BankSize;
            Dev->VarStore.NoBanks  = (UINT32)NoBanks;
            FreePool (Info);
        }

        Dev->File->Flush (Dev->File);
        Dev->File->Close (Dev->File);
        Dev->File = NULL;

        DevicePath = FileDevicePath(BestHandle,L"\\Efi\\BootStr");
        Dev->Handle = NULL;
        LibInstallProtocolInterfaces (
            &Dev->Handle,
            &DevicePathProtocol, DevicePath,
            NULL
            );

        Dev->VarStore.ClearStore  = VarBSClearStore;
        Dev->VarStore.ReadStore   = VarBSReadStore;
        Dev->VarStore.UpdateStore = VarBSUpdateStore;
        Dev->VarStore.SizeStore   = VarBSSizeStore;

        //
        // Open then mounted file system that contains the BootStr file Exclusive,
        // so it will never be disconnected.
        //
        Status = BS->OpenProtocol (
                       BestHandle,
                       &FileSystemProtocol,
                       (VOID **)&Interface,
                       Dev->Handle,
                       Dev->Handle,
                       EFI_OPEN_PROTOCOL_EXCLUSIVE
                       );

        Status = LibInstallProtocolInterfaces (
                        &Dev->Handle,
                        &VariableStoreProtocol, &Dev->VarStore,
                        NULL
                        );

        ASSERT(!EFI_ERROR(Status));
        DEBUG ((D_INIT, "NvRam banks added: Banks - %d, Bank Size - 0x%x\n", NoBanks, BankSize));

        Status = BS->CreateEvent(
                        EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                        TPL_NOTIFY,
                        RtVarBSDeviceSetVirtualMapping,
                        Dev,
                        &Event
                        );

        ASSERT (!EFI_ERROR(Status));

    } else {

        BS->FreePool(Dev);

        DEBUG ((D_VAR|D_ERROR, "VarAddBSStore: Volume not available, NV+BS is using BS only\n"));

        PlInitNvVarStoreMem (Attributes, BankSize, NoBanks);
    }
}

#pragma RUNTIME_CODE(RtVarBSDeviceSetVirtualMapping)
VOID
STATIC RUNTIMEFUNCTION
RtVarBSDeviceSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    )
{
    VAR_DEV              *VarDevice;
    EFI_CONVERT_POINTER  ConvertPointer;

    VarDevice = Context;

    ConvertPointer = RT->ConvertPointer;

    VarDevice->VarStore.ClearStore  = RtVarBSClearStore;
    VarDevice->VarStore.ReadStore   = RtVarBSReadStore;
    VarDevice->VarStore.UpdateStore = RtVarBSUpdateStore;
    VarDevice->VarStore.SizeStore   = NULL;

    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &VarDevice->VarStore.ClearStore);
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &VarDevice->VarStore.ReadStore);
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &VarDevice->VarStore.UpdateStore);
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &VarDevice->VarStore.SizeStore);
}

#pragma RUNTIME_CODE(RtVarBSClearStore)
STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtVarBSClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN VOID                 *Scratch
    )
{
    return EFI_SUCCESS;
}

#pragma RUNTIME_CODE(RtVarBSReadStore)
STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtVarBSReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    )
{
    RtSetMem (Buffer, BufferSize, 0xff);
    return EFI_SUCCESS;
}

#pragma RUNTIME_CODE(RtVarBSUpdateStore)
STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtVarBSUpdateStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *UserBuffer
    )
{
    return EFI_SUCCESS;
}


STATIC
VOID
VarOpenBSStore (
    IN VAR_DEV              *Dev,
    IN UINT64               OpenMode,
    OUT UINT64              *AvailableSpace
    )
{
    EFI_HANDLE              Handle;
    EFI_FILE_HANDLE         Vol, DirHandle;
    EFI_FILE_SYSTEM_INFO    *Info;    
    EFI_STATUS              Status;
    UINTN                   BufferSize;
    UINT64                  Attr;
    EFI_DEVICE_PATH         *DevicePath;

    ASSERT (!Dev->File);

    if (AvailableSpace) {
        *AvailableSpace = 0;
    }

    DevicePath = Dev->DevicePath;
    Status = BS->LocateDevicePath (&FileSystemProtocol, &DevicePath, &Handle);
    if (EFI_ERROR(Status) || (Handle == NULL)) {
        return;
    }

    OpenMode = OpenMode | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    Attr = EFI_FILE_HIDDEN | EFI_FILE_SYSTEM;

    //
    // Connect to the volume
    //

    Vol = LibOpenRoot(Handle);
    if (Vol) {

        //
        // Open/create the efi directory
        //

        Status = Vol->Open (
                    Vol, 
                    &DirHandle, 
                    L"efi",
                    EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 
                    EFI_FILE_DIRECTORY
                    );
        if (EFI_ERROR (Status)) {

          Status = Vol->Open (
                      Vol, 
                      &DirHandle, 
                      L"efi",
                      OpenMode, 
                      Attr | EFI_FILE_DIRECTORY
                      );
        }

        if (!EFI_ERROR(Status)) {

            //
            // Open the boot store file
            //

            Status = DirHandle->Open (
                        DirHandle, 
                        &Dev->File, 
                        L"BootStr", 
                        OpenMode,
                        Attr
                        );

            DirHandle->Close (DirHandle);
        }

        //
        // On error, return the available space
        //

        if (EFI_ERROR(Status) && AvailableSpace) {
            BufferSize = sizeof(Info);
            Info = LibFileSystemInfo (Vol);
            if (Info) {
                if (!Info->ReadOnly) {
                    *AvailableSpace = Info->FreeSpace;
                }

                FreePool (Info);
            }
        }

        //
        // Close the volume handle
        //

        Vol->Close (Vol);
    }
}

STATIC
EFI_STATUS
VarBSOpenStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,    
    IN UINTN                Offset
    )
{
    EFI_STATUS              Status;    
    VAR_DEV                 *Dev;

    Dev = DEV_FROM_THIS(This);

    //
    // If the boot store file isn't open, open it now
    //

    if (!Dev->File) {
        VarOpenBSStore (Dev, 0, NULL);
    }

    //
    // Set the position for the io
    //

    if (Dev->File) {
        Offset = Offset + BankNo * Dev->BankSize;
        Status = Dev->File->SetPosition (Dev->File, Offset);
    } else {
        DEBUG((D_ERROR,"NVRAM : Can not open \\Efi\\BootStr on %s\n",DevicePathToStr(Dev->DevicePath)));
        Status = EFI_NOT_FOUND;
    }

    return Status;
}
    

STATIC
EFI_STATUS
VarBSClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN OUT VOID             *Scratch
    )
{
    EFI_STATUS              Status;
    UINTN                   WriteSize;
    VAR_DEV                 *Dev;

    Dev = DEV_FROM_THIS(This);

    Status = VarBSOpenStore (This, BankNo, 0);

    if (!EFI_ERROR(Status)) {

        //
        // Write all 1 bits to the bank
        //

        WriteSize = Dev->BankSize;
        Status = Dev->File->Write (Dev->File, &WriteSize, Dev->Scratch);
        Dev->File->Flush (Dev->File);
        Dev->File->Close (Dev->File);
    }
    Dev->File = NULL;

    return Status;
}


STATIC
EFI_STATUS
VarBSReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    )
{
    EFI_STATUS              Status;
    VAR_DEV                 *Dev;

    Dev = DEV_FROM_THIS(This);

    Status = VarBSOpenStore (This, BankNo, Offset);

    if (!EFI_ERROR(Status)) {
        Status = Dev->File->Read (Dev->File, &BufferSize, Buffer);
        Dev->File->Flush (Dev->File);
        Dev->File->Close (Dev->File);
    }
    Dev->File = NULL;

    return Status;
}


STATIC
EFI_STATUS
VarBSUpdateStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *Buffer
    )
{
    EFI_STATUS              Status;
    VAR_DEV                 *Dev;

    Dev = DEV_FROM_THIS(This);

    Status = VarBSOpenStore (This, BankNo, Offset);

    if (!EFI_ERROR(Status)) {
        Status = Dev->File->Write (Dev->File, &BufferSize, Buffer);
        Dev->File->Flush (Dev->File);
        Dev->File->Close (Dev->File);
    }
    Dev->File = NULL;

    return Status;
}

STATIC
EFI_STATUS
VarBSSizeStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                NoBanks
    )
{
    EFI_STATUS              Status;
    UINT8                   c;
    VAR_DEV                 *Dev;

    Dev = DEV_FROM_THIS(This);

    Status = VarBSUpdateStore (This, NoBanks, -1, 1, &c);
    if (!EFI_ERROR(Status)) {
        Dev->VarStore.NoBanks = (UINT32) NoBanks;
    }

    return Status;
}
