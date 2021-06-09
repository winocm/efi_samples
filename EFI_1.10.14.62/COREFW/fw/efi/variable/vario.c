/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name: vario.c

Abstract: Low level access abstractions for NVRAM




Revision History

--*/

#include "ivar.h"

//
//
//
#pragma RUNTIME_CODE(RtVarClearStore)
EFI_STATUS
INTERNAL RUNTIMESERVICE
RtVarClearStore (
    IN STORAGE_BANK         *Bank
    )
{
    EFI_STATUS              Status;
    EFI_VARIABLE_STORE      *Device;        
    UINTN                   BankNo;
    VARIABLE_STORE          *VarStore;

    ASSERT_LOCKED (&VariableStoreLock);
    Device = Bank->Device;
    BankNo = Bank->BankNo;
    VarStore = Bank->VarStore;

    //
    // Remove the bank from any list
    //

    if (Bank->Link.Flink) {
        RemoveEntryList (&Bank->Link);
        Bank->Link.Flink = NULL;
    }

    //
    // If there's a device error, don't erase the store
    //

    if (Bank->DeviceError) {
        DEBUG ((D_VAR|D_ERROR, "VarClearStore: not clearing due to prior device error\n"));
        return EFI_DEVICE_ERROR;
    }

    //
    // If the bank has a valid signature, clear it before clearing
    // the bank as the clear operation may not work LSB to MSB.
    //

    if (Bank->u.Header->Signature == BANK_HEADER_SIGNATURE) {
        ZeroMem (Bank->u.Header, sizeof(BANK_HEADER));
        RtVarUpdateStore (Bank, 0, sizeof(BANK_HEADER));
        if (Bank->DeviceError) {
            goto Done;
        }
    }
    
    //
    // Clear the store
    //

    Status = Device->ClearStore(Device, BankNo, Bank->u.Data);
    if (EFI_ERROR(Status)) {
        goto Done;
    }

    //
    // Initialize the bank structure
    //

    Bank->State = 0;
    Bank->Tail = FIRST_VARIABLE_OFFSET;
    Bank->InUse = FIRST_VARIABLE_OFFSET;

    //
    // Initialize the bank header
    //

    Bank->u.Header->Signature = BANK_HEADER_SIGNATURE;
    Bank->u.Header->Format = VARH_FORMAT;
    Bank->u.Header->State = 0xFF;         // value is stored inverted - all bits "clear"

    //
    // Get the next sequence number, stop at the max
    //

    if (VarStore->Sequence < VARH_MAX_SEQUENCE) {
        VarStore->Sequence += 1;
    }

    Bank->u.Header->Sequence = VarStore->Sequence;
    
    //
    // Write the bank header
    //

    Status = Device->UpdateStore(Device, BankNo, 0, sizeof(BANK_HEADER), Bank->u.Header);
    if (EFI_ERROR(Status)) {
        goto Done;
    }

    //
    // Done
    //

    InsertTailList (&VarStore->Active, &Bank->Link);

Done:

    if (EFI_ERROR(Status) || Bank->DeviceError) {
        Bank->DeviceError = TRUE;
        DEBUG ((D_VAR|D_INIT, "VarClearStore: Error clearing storage bank %x\n", Status));
    }

    return Status;
}
    
#pragma RUNTIME_CODE(RtVarReadStore)
VOID *
INTERNAL RUNTIMESERVICE
RtVarReadStore (
    IN STORAGE_BANK         *Bank, 
    IN UINTN                Offset,
    IN UINTN                Size
    )
{
    EFI_STATUS              Status;
    EFI_VARIABLE_STORE      *Device;        
    UINTN                   BankNo;
    CHAR8                   *Buffer;

    ASSERT_LOCKED (&VariableStoreLock);

    Device = Bank->Device;
    BankNo = Bank->BankNo;
    Buffer = Bank->u.Data + Offset;

    Status = Device->ReadStore(Device, BankNo, Offset, Size, Buffer);
    if (EFI_ERROR(Status)) {
        DEBUG((D_VAR|D_ERROR, "RtVarReadStore: Error reading variable store %x\n", Status));
        Bank->DeviceError = TRUE;
    }

    return Buffer;
}

#pragma RUNTIME_CODE(RtVarUpdateStore)
VOID
INTERNAL RUNTIMESERVICE
RtVarUpdateStore (
    IN STORAGE_BANK         *Bank, 
    IN UINTN                Offset,
    IN UINTN                Size
    )
{
    EFI_STATUS              Status;
    EFI_VARIABLE_STORE      *Device;        
    UINTN                   BankNo;
    CHAR8                   *Buffer;

    ASSERT_LOCKED (&VariableStoreLock);

    Device = Bank->Device;
    BankNo = Bank->BankNo;
    Buffer = Bank->u.Data + Offset;

    if (Bank->DeviceError) {
        DEBUG ((D_VAR|D_ERROR, "RtVarUpdateStore: not updating due to prior device error\n"));
        return ;
    }

    Status = Device->UpdateStore(Device, BankNo, Offset, Size, Buffer);
    if (EFI_ERROR(Status)) {
        DEBUG((D_VAR|D_ERROR, "RtVarUpdateStore: Error updating variable store %x\n", Status));
        Bank->DeviceError = TRUE;
    }
}

#pragma RUNTIME_CODE(RtVarParseStore)
BOOLEAN
INTERNAL RUNTIMESERVICE
RtVarParseStore (
    OUT VARIABLE_INFO       *VarInfo
    )
// In:
//  VarInfo->Bank
//  VarInfo->NextVarOffset
// Out:
//  TRUE if not EOF
{
    VARIABLE                *Var;
    UINTN                   Index, Offset;
    UINTN                   VarSize, PadSize;
    STORAGE_BANK            *Bank;

    ASSERT_LOCKED (&VariableStoreLock);

    Offset = VarInfo->NextVarOffset;
    Bank = VarInfo->Bank;

    //
    // Assume it's not valid, and initialize some fields
    //

    VarInfo->Valid = FALSE;
    VarInfo->VarOffset = Offset;
    VarInfo->NextVarOffset = Bank->BankSize;
    VarInfo->NameOffset = Offset + sizeof(VARIABLE);
    VarInfo->Name = (CHAR16 *) (Bank->u.Data + VarInfo->NameOffset);

    //
    // If there's not enough space for an entry, return we're at the end
    // of the bank
    //

    if (Offset + sizeof(VARIABLE) + sizeof(UINT8) + 2 + 1 > Bank->Tail) {
        return FALSE;
    }

    //
    // Get the variable header & total size
    //

    Var = (VARIABLE *) (Bank->u.Data + Offset);
    VarSize = Var->Size;
    if (VarSize == 0xFFFF) {
        return FALSE;                       // End of bank marker
    }

    VarSize = VarSize & VAR_SIZE_MASK;

    // Make sure VarSize is legit - if it's not, then we need
    // to indicate that this is the end of the bank
    if (VarSize - sizeof(VARIABLE) + Offset > Bank->BankSize) {
        return FALSE;
    }

    //
    // Compute the rounded size of this entry, and the location
    // of the next entry
    //

    Index = Offset + VarSize;
    PadSize = (sizeof(UINT32) - VarSize % sizeof(UINT32)) & (sizeof(UINT32)-1);
    VarInfo->NextVarOffset = Index + PadSize;
    VarInfo->VarSize = VarInfo->NextVarOffset - Offset;
    VarInfo->Attributes = Var->Attributes;
    
    ASSERT (VarInfo->VarSize % sizeof(UINT32) == 0);
    ASSERT (VarInfo->NextVarOffset <= Bank->BankSize);
    ASSERT (!(VarInfo->NextVarOffset % sizeof(UINT32)));

    //
    // Get the variable's state value
    //

    VarInfo->StateOffset = Offset + VarSize - 1;
    VarInfo->State = Bank->u.Data[VarInfo->StateOffset] ^ 0xFF;

    //
    // If the variable wasn't completely added, treat this
    // entry as a deleted entry and return !Valid
    //

    if (!(VarInfo->State & VAR_ADDED) || (VarInfo->State & VAR_OBSOLETE)) {
        VarInfo->State = VAR_ADDED | VAR_OBSOLETE_TRANSITION | VAR_OBSOLETE;
        return TRUE;
    }

    //
    // Get the guid
    //

    VarInfo->VendorGuid = &Var->VendorGuid;

    //
    // Parse the name in order to locate the data
    //

    Index = VarInfo->NameOffset;
    while (Bank->u.Data[Index] || Bank->u.Data[Index+1]) {

        if (Index >= VarInfo->StateOffset) {
            // end of Name not found
            goto Done;
        }

        Index += 2;
    }

    VarInfo->NameSize = Index - VarInfo->NameOffset + sizeof(CHAR16);

    //
    // Supply the data info
    //

    VarInfo->DataOffset = VarInfo->NameOffset + VarInfo->NameSize;
    VarInfo->Data = Bank->u.Data + VarInfo->DataOffset;
    VarInfo->DataSize = VarInfo->StateOffset - VarInfo->DataOffset;

    //
    // All the fields are parsed and the data is valid
    //

    VarInfo->Valid = TRUE;

Done:
    return TRUE;
}    

#pragma RUNTIME_CODE(RtVarUpdateState)
BOOLEAN
INTERNAL RUNTIMEFUNCTION
RtVarUpdateState (
    IN VARIABLE_INFO        *VarInfo,
    IN UINTN                NewBit
    )
{
    STORAGE_BANK            *Bank;    

    Bank = VarInfo->Bank;
    VarInfo->State |= (UINT8) NewBit;
    Bank->u.Data[VarInfo->StateOffset] = VarInfo->State ^ 0xFF;
    RtVarUpdateStore (Bank, VarInfo->StateOffset, sizeof(UINT8));
    return Bank->DeviceError;
}

#pragma RUNTIME_CODE(RtVarUpdateBankState)
VOID
INTERNAL RUNTIMEFUNCTION
RtVarUpdateBankState (
    IN STORAGE_BANK         *Bank,
    IN UINTN                NewBit
    )
{
    Bank->State |= (UINT8) NewBit;
    Bank->u.Header->State = Bank->State ^ 0xFF;
    RtVarUpdateStore (Bank, 0, sizeof(BANK_HEADER));
}


#pragma RUNTIME_CODE(RtVarBankError)
VOID
INTERNAL RUNTIMEFUNCTION
RtVarBankError (
    IN STORAGE_BANK         *Bank
    )
{
    ASSERT_LOCKED (&VariableStoreLock);

    //
    // Free any resources used
    //

    DEBUG ((D_VAR|D_INIT, "VarBankError: Removing defective bank %X:%d (type %a)\n", 
        Bank->Device, 
        Bank->BankNo, 
        Bank->VarStore->Type
        ));

    //
    // Remove it from any list
    //

    if (Bank->Link.Flink) {
        RemoveEntryList (&Bank->Link);
    }

    //
    // If at boottime, free the memory
    //

    if (!EfiAtRuntime) {
        FreePool (Bank);
    }
}

