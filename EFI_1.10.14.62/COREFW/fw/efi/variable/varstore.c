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

    varstore.c

Abstract:

    handles variable store/reads with emulated memory

Revision History

--*/


#include "efi.h"
#include "efilib.h"
#include "EfiVar.h"

//
// 
//

#define VARIABLE_STORE_SIGNATURE    EFI_SIGNATURE_32('v','d','e','v')
typedef struct {
    UINTN                   Signature;
    EFI_HANDLE              EfiHandle;    
    EFI_VARIABLE_STORE      VarStore;
} VAR_DEV;

#define DEV_FROM_THIS(a) CR(a, VAR_DEV, VarStore, VARIABLE_STORE_SIGNATURE)

//
//
//
STATIC 
VOID
RUNTIMEFUNCTION
RtNvRamSetVirtualMappingVirtual (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    );

STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtClearStoreVirtual(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN OUT VOID             *Scratch
    );

STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtReadStoreVirtual (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    OUT VOID                *Buffer
    );


STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtUpdateStoreVirtual (
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
InitNvVarStoreVirtual (
    IN UINTN    Attributes,
    IN UINTN    BankSize,
    IN UINTN    NoBanks
    )

{
    VAR_DEV                 *Dev;
    EFI_STATUS              Status;
    EFI_EVENT               Event;
  
    Status = BS->AllocatePool (EfiRuntimeServicesData, sizeof(VAR_DEV), &Dev);
    ASSERT(!EFI_ERROR(Status));

    ZeroMem (Dev, sizeof(VAR_DEV));

    Dev->Signature = VARIABLE_STORE_SIGNATURE;
    Dev->VarStore.Attributes = (UINT32)Attributes;
    
    Dev->VarStore.BankSize = (UINT32)BankSize;
    Dev->VarStore.NoBanks  = (UINT32)NoBanks;

    Dev->VarStore.ClearStore    = RtClearStoreVirtual;
    Dev->VarStore.ReadStore     = RtReadStoreVirtual;
    Dev->VarStore.UpdateStore   = RtUpdateStoreVirtual;
    
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
    DEBUG ((D_INIT, "NvRam banks added: %x\n", Attributes));

    //
    // This a runtime nvram device, we need to update any internal
    // pointers for the device if the OS sets the environment into
    // virtual mode
    //

    Status = BS->CreateEvent(
                    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                    TPL_NOTIFY,
                    RtNvRamSetVirtualMappingVirtual,
                    Dev,
                    &Event
                    );

    ASSERT (!EFI_ERROR(Status));
}

#pragma RUNTIME_CODE(RtNvRamSetVirtualMappingVirtual)
STATIC
VOID
RUNTIMEFUNCTION
RtNvRamSetVirtualMappingVirtual (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    )
{
    EFI_CONVERT_POINTER     ConvertPointer;
    VAR_DEV                 *Dev;

    Dev = Context;
    ConvertPointer = RT->ConvertPointer;

    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.ClearStore);
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.ReadStore);
    ConvertPointer (EFI_INTERNAL_FNC, (VOID **) &Dev->VarStore.UpdateStore);
}

#pragma RUNTIME_CODE(RtClearStoreVirtual)
STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtClearStoreVirtual(
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN VOID                 *Scratch
    )
{
    return EFI_SUCCESS;
}


#pragma RUNTIME_CODE(RtReadStoreVirtual)
STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtReadStoreVirtual (
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

#pragma RUNTIME_CODE(RtUpdateStoreVirtual)
STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtUpdateStoreVirtual (
    IN EFI_VARIABLE_STORE   *This,
    IN UINTN                BankNo,
    IN UINTN                Offset,
    IN UINTN                BufferSize,
    IN VOID                 *UserBuffer
    )
{
    return EFI_SUCCESS;
}
