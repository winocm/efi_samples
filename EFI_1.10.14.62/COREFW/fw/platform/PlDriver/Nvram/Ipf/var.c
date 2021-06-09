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
#include "SalEfi.h"
#include "SalProc.h"
#include "EfiVar.h"

//
// 
//
#define NvSCRATCHBUFSZ              0x10000

#define VARIABLE_STORE_SIGNATURE    EFI_SIGNATURE_32('v','d','e','v')
typedef struct {
    UINTN                   Signature;
    EFI_HANDLE              EfiHandle;
    VOID                    *ScratchBuf; // atleast 64K
    EFI_VARIABLE_STORE      VarStore;
} VAR_DEV;

#define DEV_FROM_THIS(a) CR(a, VAR_DEV, VarStore, VARIABLE_STORE_SIGNATURE)

//
//
//

VOID
STATIC RUNTIMEFUNCTION
RtPlNvRamSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    );

STATIC
EFI_STATUS
RtPlClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN OUT VOID             *Scratch
    );

STATIC
EFI_STATUS
RtPlReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    );


STATIC
EFI_STATUS
RtPlUpdateStore (
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
PlInitNvVarStoreFlash (
    IN UINTN    Attributes,
    IN UINTN    BankSize,
    IN UINTN    NoBanks
    )

{
    VAR_DEV                     *Dev;
    EFI_STATUS                  Status;
    EFI_EVENT                   Event;

    Status = BS->AllocatePool (EfiRuntimeServicesData, sizeof(VAR_DEV), &Dev);
    ASSERT(!EFI_ERROR(Status));
    ZeroMem (Dev, sizeof(VAR_DEV));

    Dev->Signature = VARIABLE_STORE_SIGNATURE;
    Dev->VarStore.Attributes = (UINT32)Attributes;

    Dev->VarStore.BankSize = (UINT32)BankSize;
    Dev->VarStore.NoBanks = (UINT32)NoBanks;

    Dev->VarStore.ClearStore = RtPlClearStore;
    Dev->VarStore.ReadStore = RtPlReadStore;
    Dev->VarStore.UpdateStore = RtPlUpdateStore;

    Status = BS->AllocatePool (EfiRuntimeServicesData, NvSCRATCHBUFSZ, &Dev->ScratchBuf);
    ASSERT(!EFI_ERROR(Status));

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
    DEBUG ((D_INIT, "NvRam banks added: Banks - %d, Bank Size - 0x%x\n", NoBanks, BankSize));

    //
    // This a runtime nvram device, we need to update any internal
    // pointers for the device if the OS sets the environment into
    // virtual mode
    //

    Status = BS->CreateEvent(
                    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                    TPL_NOTIFY,
                    RtPlNvRamSetVirtualMapping,
                    Dev,
                    &Event
                    );

    ASSERT (!EFI_ERROR(Status));
}

VOID
STATIC RUNTIMEFUNCTION
RtPlNvRamSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    )
{
    VAR_DEV                 *Dev;

    Dev = Context;

    RT->ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.ClearStore);
    RT->ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.ReadStore);
    RT->ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.UpdateStore);
    RT->ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->ScratchBuf);
}


#pragma RUNTIME_CODE(RtPlClearStore)
STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtPlClearStore(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN VOID                 *Scratch
    )
{
    VAR_DEV             *Dev;
    rArg                Return={-1,0,0,0};

    Dev = DEV_FROM_THIS(This);

    RtSalCallBack(
        ID_SALCB_NVRAM, ID_SALCB_NVRAM_CLEAR,BankNo,
        0,0,0,
        (UINTN)Dev->ScratchBuf,
        0,
        &Return
        );

    if(Return.p0) {
        DEBUG ((D_ERROR, "ClearStore: Failed to clear bank - %d\n",BankNo));
        return(EFI_UNSUPPORTED);
    }

    return(EFI_SUCCESS);
}

#pragma RUNTIME_CODE(RtPlReadStore)
STATIC
EFI_STATUS
RtPlReadStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    )
{
    VAR_DEV             *Dev;
    rArg                Return={-1,0,0,0};

    Dev = DEV_FROM_THIS(This);

    ASSERT (Offset + BufferSize <= Dev->VarStore.BankSize);

    RtSalCallBack(
        ID_SALCB_NVRAM, ID_SALCB_NVRAM_READ,BankNo,
        Offset,BufferSize,(UINTN)Buffer,
        (UINTN)Dev->ScratchBuf,
        0,
        &Return
        );

    if(Return.p0) {
        DEBUG ((D_ERROR, "ReadStore: Failed to Read offset - 0x%x from bank - %d\n",Offset,BankNo));
        return(EFI_UNSUPPORTED);
    }

    return(EFI_SUCCESS);
   
}

#pragma RUNTIME_CODE(RtPlUpdateStore)
STATIC
EFI_STATUS
RtPlUpdateStore (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *UserBuffer
    )
{
    VAR_DEV             *Dev;
    rArg                Return={-1,0,0,0};

    Dev = DEV_FROM_THIS(This);

    ASSERT (Offset + BufferSize <= Dev->VarStore.BankSize);

    RtSalCallBack(
        ID_SALCB_NVRAM, ID_SALCB_NVRAM_UPDATE,BankNo,
        Offset,BufferSize,(UINTN)UserBuffer,
        (UINTN)Dev->ScratchBuf,
        0,
        &Return
        );

    if(Return.p0) {
        DEBUG ((D_ERROR, "WriteStore: Failed to write offset - 0x%x into bank - %d\n",Offset,BankNo));
        return(EFI_UNSUPPORTED);
    }

    return(EFI_SUCCESS);

}


