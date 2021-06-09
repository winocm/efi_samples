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

    Emulate a variable storage device



Revision History

--*/


#include "ntemul.h"

//
// 
//

#define VARIABLE_STORE_SIGNATURE    EFI_SIGNATURE_32('v','d','e','v')
typedef struct {
    UINTN                   Signature;
    EFI_HANDLE              EfiHandle;    
    CHAR16                  *FileName;
    HANDLE                  h;
    EFI_VARIABLE_STORE      VarStore;
} VAR_DEV;

#define DEV_FROM_THIS(a) CR(a, VAR_DEV, VarStore, VARIABLE_STORE_SIGNATURE)

//
//
//

VOID
STATIC RUNTIMEFUNCTION
PlNvRamSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    );

STATIC
EFI_STATUS
PlClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN OUT VOID             *Scratch
    );

STATIC
EFI_STATUS
PlReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    );


STATIC
EFI_STATUS
PlUpdateStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *Buffer
    );


//
//
//


VOID
PlInitNvVarStoreEmul (
    IN UINTN    Attributes,
    IN UINTN    BankSize,
    IN UINTN    NoBanks
    )

{
    VAR_DEV                     *Dev;
    UINTN                       eof, bw;
    INT8                        Buffer[512];
    EFI_STATUS                  Status;
    UINT32                      FileSize;
    EFI_EVENT                   Event;
  
    Dev = AllocateZeroPool(sizeof(VAR_DEV));

    Dev->Signature = VARIABLE_STORE_SIGNATURE;
    Dev->FileName = L"NV1File";
    Dev->VarStore.Attributes = Attributes;

    Dev->VarStore.BankSize = BankSize;
    Dev->VarStore.NoBanks = NoBanks;

    Dev->VarStore.ClearStore = PlClearStore;
    Dev->VarStore.ReadStore = PlReadStore;
    Dev->VarStore.UpdateStore = PlUpdateStore;
    
    Dev->h = CreateFile (
                Dev->FileName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_ALWAYS,
                0,
                NULL
                );

    if (Dev->h == INVALID_HANDLE_VALUE) {
        DEBUG ((D_ERROR, "Could not open %s %d\n", Dev->FileName, GetLastError()));
        return ;
    }

    FileSize = SetFilePointer (Dev->h, 0, NULL, FILE_END);
    if (FileSize == -1) {
        DEBUG ((D_ERROR, "Could not get filesize of %s\n", Dev->FileName));
    }
    
    eof = BankSize * NoBanks;
    if (FileSize != eof) {
        // file is not the proper size, change it
        DEBUG ((D_INIT, "Initializing NvRam banks: %hs\n", Dev->FileName));

        SetFilePointer (Dev->h, eof, NULL, FILE_BEGIN);
        SetEndOfFile (Dev->h);

        *((UINT32 *) Buffer) = -1;
        SetFilePointer (Dev->h, 0, NULL, FILE_BEGIN);
        WriteFile (Dev->h, Buffer, sizeof(UINT32), &bw, NULL);
    }

    //
    // Add a new handle to the system for this device
    //

    Dev->EfiHandle = NULL;
    Status = LibInstallProtocolInterfaces (
                    &Dev->EfiHandle,
                    &VariableStoreProtocol, &Dev->VarStore,
                    NULL
                    );

    ASSERT(!EFI_ERROR(Status));
    DEBUG ((D_INIT, "NvRam banks added: %hs\n", Dev->FileName));

    //
    // This a runtime nvram device, we need to update any internal
    // pointers for the device if the OS sets the environment into
    // virtual mode
    //

    Status = BS->CreateEvent(
                    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                    TPL_NOTIFY,
                    PlNvRamSetVirtualMapping,
                    Dev,
                    &Event
                    );

    ASSERT (!EFI_ERROR(Status));
}

VOID
STATIC RUNTIMEFUNCTION
PlNvRamSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    )
{
    EFI_CONVERT_POINTER     ConvertPointer;
    VAR_DEV                 *Dev;

    Dev = Context;
    ConvertPointer = RT->ConvertPointer;

    Dev->FileName = NULL;
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.ClearStore);
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.ReadStore);
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.UpdateStore);
}


STATIC
EFI_STATUS
PlClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN VOID                 *Scratch
    )
{
    VAR_DEV             *Dev;
    UINTN               i, bw;
    BOOLEAN             f;

    Dev = DEV_FROM_THIS(This);

    i = SetFilePointer (Dev->h, BankNo * Dev->VarStore.BankSize, NULL, FILE_BEGIN);
    if (i == 0xffffffff) {
        DEBUG ((D_ERROR, "ClearStore: SetFilePointer failed\n"));
        return EFI_DEVICE_ERROR;
    }

    SetMem (Scratch, Dev->VarStore.BankSize, 0xFF);
    f = WriteFile (Dev->h, Scratch, Dev->VarStore.BankSize, &bw, NULL);
    if (!f || bw != Dev->VarStore.BankSize) {
        DEBUG ((D_ERROR, "ClearStore: WriteFile failed. (%d)\n", GetLastError()));
        return EFI_DEVICE_ERROR;
    }

    ZeroMem (Scratch, Dev->VarStore.BankSize);      // debug
    FlushFileBuffers (Dev->h);
    return EFI_SUCCESS;
}


STATIC
EFI_STATUS
PlReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    )
{
    VAR_DEV             *Dev;
    UINTN               i, br;
    BOOLEAN             f;

    Dev = DEV_FROM_THIS(This);

    ASSERT (Offset + BufferSize <= Dev->VarStore.BankSize);
    i = SetFilePointer (Dev->h, BankNo * Dev->VarStore.BankSize + Offset, NULL, FILE_BEGIN);
    if (i == 0xffffffff) {
        DEBUG ((D_ERROR, "ReadStore: SetFilePointer failed\n"));
        return EFI_DEVICE_ERROR;
    }

    f = ReadFile (Dev->h, Buffer, BufferSize, &br, NULL);
    if (!f || br != BufferSize) {
        DEBUG ((D_ERROR, "ReadStore: ReadFile failed. (%d)\n", GetLastError()));
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


STATIC
EFI_STATUS
PlUpdateStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *UserBuffer
    )
{
    VAR_DEV             *Dev;
    UINTN               Size, Index;
    UINTN               i, brw;
    BOOLEAN             f;
    CHAR8               OrigData[512];
    CHAR8               *Buffer;

    Dev = DEV_FROM_THIS(This);

    ASSERT (Offset + BufferSize <= Dev->VarStore.BankSize);
    Offset = Offset + BankNo * Dev->VarStore.BankSize;
    Buffer = UserBuffer;
    
    while (BufferSize) {
        Size = BufferSize;
        if (Size > sizeof(OrigData)) {
            Size = sizeof(OrigData);
        }

        i = SetFilePointer (Dev->h, Offset, NULL, FILE_BEGIN);
        if (i == 0xffffffff) {
            DEBUG ((D_ERROR, "UpdateStore: SetFilePointer failed\n"));
            return EFI_DEVICE_ERROR;
        }

        f = ReadFile (Dev->h, OrigData, Size, &brw, NULL);
        if (!f || brw != Size) {
            DEBUG ((D_ERROR, "UpdateStore: ReadFile failed. (%d)\n", GetLastError()));
            return EFI_DEVICE_ERROR;
        }

        // verify bits are only being turned off
        for (Index=0; Index<Size; Index++) {
            if ((Buffer[Index] & OrigData[Index]) != Buffer[Index]) {
                DEBUG ((D_ERROR, "UpdateStore: attempting to turn on store bits\n"));
                ASSERT (FALSE);
                return EFI_UNSUPPORTED;
            }
        }

        i = SetFilePointer (Dev->h, Offset, NULL, FILE_BEGIN);
        if (i == 0xffffffff) {
            DEBUG ((D_ERROR, "UpdateStore: SetFilePointer failed\n"));
            return EFI_DEVICE_ERROR;
        }

        f = WriteFile (Dev->h, Buffer, Size, &brw, NULL);
        if (!f || brw != Size) {
            DEBUG ((D_ERROR, "UpdateStore: WriteFile failed. (%d)\n", GetLastError()));
            return EFI_DEVICE_ERROR;
        }

        BufferSize -= Size;
        Offset += Size;
        Buffer += Size;
    }

    FlushFileBuffers (Dev->h);
    return EFI_SUCCESS;
}
