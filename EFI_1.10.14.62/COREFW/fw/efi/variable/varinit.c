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

Abstract:




Revision History

--*/

#include "ivar.h"

//
// This code is coded to the least common dominator which is
// runtime access of non-volatile variables. (Which is also
// the most interesting variable type).  Since there's no
// dynamic memory allocation at runtime, the algorithm is
// slow.   No special code for boot time optimizations
// (e.g., indexing the stores) is performed
//

//
// Prototypes
//

STATIC
BOOLEAN
VarCheckBank (
    IN STORAGE_BANK         *Bank
    );

VOID
STATIC RUNTIMEFUNCTION
RtVarStoreSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    );

//
// Declare runtime code
//


//
//
//

VOID
InitializeVariableStore (
    VOID
    )
{
    UINT32                  Attributes;
    VARIABLE_STORE          *VarStore;
    EFI_EVENT               Event;
    EFI_STATUS              Status;

    //
    // Initialize a variable store driver for BS + RT
    //

    InitNvVarStoreVirtual (
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        0x4000, 2
        );

    //
    // Initialize globals for variable stores
    //

    InitializeLock (&VariableStoreLock, TPL_CALLBACK);
    RtAcquireStoreLock ();

    Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS;
    VariableStoreType[Attributes] = &VariableStore[0];
    VariableStore[0].Attributes = Attributes;
    VariableStore[0].MemoryType = EfiBootServicesData;
    VariableStore[0].Type = "BS";
    VariableStore[0].Next = &VariableStore[1];

    Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE;
    VariableStoreType[Attributes] = &VariableStore[1];
    VariableStore[1].Attributes = Attributes;
    VariableStore[1].MemoryType = EfiBootServicesData;
    VariableStore[1].Type = "NV+BS";
    VariableStore[1].Next = &VariableStore[2];

    Attributes = EFI_VARIABLE_RUNTIME_ACCESS;
    VariableStoreType[Attributes] = &VariableStore[2];
    Attributes |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
    VariableStoreType[Attributes] = &VariableStore[2];
    VariableStore[2].Attributes = Attributes;
    VariableStore[2].MemoryType = EfiRuntimeServicesData;
    VariableStore[2].Type = "BS+RT";
    VariableStore[2].Next = &VariableStore[3];

    Attributes = EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE;
    VariableStoreType[Attributes] = &VariableStore[3];
    Attributes |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
    VariableStoreType[Attributes] = &VariableStore[3];
    VariableStore[3].Attributes = Attributes;
    VariableStore[3].MemoryType = EfiRuntimeServicesData;
    VariableStore[3].Type = "NV+BS+RT";
    VariableStore[3].RuntimeUpdate = TRUE;

    for (VarStore=VariableStore; VarStore; VarStore=VarStore->Next) {
        InitializeListHead (&VarStore->Active);
    }

    RtReleaseStoreLock ();

    //
    // Notify when SetVirtualAddressMap occurs
    //

    Status = CreateEvent (
                EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                TPL_NOTIFY,
                RtVarStoreSetVirtualMapping,
                NULL,
                &Event
                );

    ASSERT (!EFI_ERROR(Status));
}

#pragma RUNTIME_CODE(RtVarStoreSetVirtualMapping)
VOID
STATIC RUNTIMEFUNCTION
RtVarStoreSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    )
{
    EFI_CONVERT_POINTER     ConvertPointer;
    VARIABLE_STORE          *VarStore;
    STORAGE_BANK            *Bank;
    UINTN                   Index;
    LIST_ENTRY              *Link;
    
    
    ConvertPointer = RT->ConvertPointer;

    for (Index=0; Index < MAX_VARIABLE_STORAGE_TYPE; Index++) {
        VarStore = &VariableStore[Index];

        ConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_PTR, &VarStore->Next);

        //
        // If this is a runtime variable store, update it
        //

        if (VarStore->MemoryType == EfiRuntimeServicesData) {

            //
            // Update all the banks for this store
            //

            for (Link=VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
                Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);
                ConvertPointer (EFI_INTERNAL_PTR, &Bank->VarStore);
                ConvertPointer (EFI_INTERNAL_PTR, &Bank->TSize);
                ConvertPointer (0, &Bank->Device);
                ConvertPointer (0, &Bank->u.Data);
            }

            //
            // If there's a tbank, update it as well
            //

            if (VarStore->TBank) {
                Bank = VarStore->TBank;
                ConvertPointer (EFI_INTERNAL_PTR, &Bank->VarStore);
                ConvertPointer (EFI_INTERNAL_PTR, &Bank->TSize);
                ConvertPointer (0, &VarStore->TBank);
                ConvertPointer (0, &Bank->Device);
                ConvertPointer (0, &Bank->u.Data);
            }

            //
            // Fix the active bank list & other pointers in this store
            //

            RtConvertList (EFI_INTERNAL_PTR, &VarStore->Active);
            VarStore->Type = NULL;
        }
    }
    //
    // fix variablestoretype contents as well
    //

    for (Index=0; Index < (VAR_ATTRIBUTE_TYPE+1); Index++) {
        if (VariableStoreType[Index]) {
            ConvertPointer (EFI_INTERNAL_PTR, &VariableStoreType[Index]);
        }
    }

}


VOID
FwNvStoreInstalled (
    VOID
    )
{
    EFI_STATUS              Status;
    UINTN                   HandleIndex;
    UINTN                   NoHandles;
    EFI_HANDLE              *Buffer, Handle;
    EFI_VARIABLE_STORE      *Device;

    RtAcquireStoreLock ();

    //
    // Get all the variable store device handles
    //

    LibLocateHandle (ByProtocol, &VariableStoreProtocol, NULL, &NoHandles, &Buffer);

    //
    // Add all the variable store banks for all the handles
    //

    for(HandleIndex = 0; HandleIndex < NoHandles; HandleIndex += 1) {
        Handle = Buffer[HandleIndex];

        //
        // Get the variable store device
        //

        Status = BS->HandleProtocol (Handle, &VariableStoreProtocol, &Device);
        if (EFI_ERROR(Status)) {
            continue;
        }

        VarAddDevice (Device);
    }

    //
    // Done with the handle buffer
    //

    FreePool (Buffer);

    //
    // Now that all the banks are added, check their states and
    // correct any partial updates
    //

    VarCheckBanks ();
        
    //
    // Done
    //

    RtReleaseStoreLock ();

    //
    // Initialize the monotonic counter
    //

    InitializeMonotonicCount();
}


VOID
INTERNAL
VarAddDevice (
    EFI_VARIABLE_STORE  *Device
    )
{
    UINTN                   BankIndex;
    VARIABLE_STORE          *VarStore;


    ASSERT_LOCKED (&VariableStoreLock);

    //
    // Determine which store based on the device's attributes
    //

    VarStore = VariableStoreType[Device->Attributes & VAR_ATTRIBUTE_TYPE];

    //
    // Must have at least two banks in the device
    //

    if (Device->SizeStore) {
        if (Device->NoBanks < 1) {
            Device->SizeStore (Device, 1);
        }

        if (Device->NoBanks < 2) {
            Device->SizeStore (Device, 2);
        }
    }

    //
    // Add each bank
    //

    for (BankIndex=0; BankIndex < Device->NoBanks; BankIndex += 1) {
        VarStoreAddBank (VarStore, Device, BankIndex);
    }
}

VOID
INTERNAL
VarCheckBanks (
    VOID
    )
{
    VARIABLE_STORE          *VarStore;
    LIST_ENTRY              *Link;
    STORAGE_BANK            *Bank, *InUseTransition, *ObsoleteTransition;
    BOOLEAN                 Restart;


    ASSERT_LOCKED (&VariableStoreLock);

    //
    // Now that all the banks in the system have been added, get a 
    // transaction bank and detect & fix partial bank updates for each 
    // variable store.
    //

    for (VarStore=VariableStore; VarStore; VarStore=VarStore->Next) {

        //
        // Allocate a transaction bank for this store
        //

        if (VarStore->TBankRequired  && !VarStore->TBank) {
            RtVarGetTransactionBank (VarStore);
        }

        //
        // See if there is are any banks transition that were in progress,
        // and complete them.  Find any Obsolete transition
        // or InUse transition banks.  (there should only be a max of
        // 1 each since we never allow more then one, and on startup
        // we fix any that are outstanding)
        //

        InUseTransition = NULL;
        ObsoleteTransition = NULL;

        for (Link = VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
            Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);

            // 
            if (Bank->State & VARH_OBSOLETE_TRANSITION) {
                ASSERT (!ObsoleteTransition);
                ObsoleteTransition = Bank;
            }

            if ((Bank->State & VARH_INUSE_TRANSITION) && !(Bank->State & VARH_INUSE)) {
                ASSERT (!InUseTransition);
                InUseTransition = Bank;
            }
        }

        //
        // There are 4 possibile conditions here:
        //      1 There are no obsolete or inuse banks in transition
        //      2 There is an obsolete transition bank, and no inuse transition bank
        //      3 There is an obsolete transition bank and an inuse transition bank
        //      4 There is an inuse transition bank and no obsolete transition bank
        //
        // 1. There is no outstanding operation - nothing to do.
        //
        // 2. The operation stopped before the transaction bank was committed.
        //    To correct, restart the garbage collection operation on the 
        //    obsolete transition bank
        //
        // 3. The operation stopped during the switch over after the new bank
        //    was written, but before the obsolete bank was cleared.  To correct
        //    complete / redo the change over sequence for the bank.
        //
        // 4. The operation stopped during the switch over but after the obsolete
        //    bank was cleared.  To correct mark the new bank as now inuse
        //

        // Is this case #2?
        if (ObsoleteTransition && !InUseTransition) {
            ASSERT(FALSE);     // need to test
            DEBUG((D_VAR|D_WARN, "InitVarStore: restarting collection on bank %x:%d (%a)\n",
                                        ObsoleteTransition,
                                        ObsoleteTransition->BankNo,
                                        ObsoleteTransition->VarStore->Type
                                        ));
            RtVarGarbageCollectBank (ObsoleteTransition);
        }
        
        // Is this case #3?
        if (ObsoleteTransition && InUseTransition) {
            RtVarClearStore (ObsoleteTransition);
            ObsoleteTransition = NULL;          // now case 4
        }

        // Is this case #4?
        if (!ObsoleteTransition && InUseTransition) {
            DEBUG((D_VAR|D_WARN, "InitVarStore: committing transition bank %x:%d (%a)\n",
                            InUseTransition,
                            InUseTransition->BankNo,
                            InUseTransition->VarStore->Type
                            ));
            RtVarUpdateBankState (InUseTransition, VARH_INUSE);
        }

        // check next var store
    }

    //
    // Now that all the variable stores are recovered, check for
    // any partial variable updates that need to be completed
    //

    for (VarStore=VariableStore; VarStore; VarStore=VarStore->Next) {

        Link = VarStore->Active.Flink;
        while (Link != &VarStore->Active) {
            Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);
            Link = Link->Flink;

            //
            // Check entries in this bank
            // 

            Restart = VarCheckBank (Bank);
            if (Restart) {
                Link = VarStore->Active.Flink;
            }
        }
    }

}


STATIC
BOOLEAN
VarCheckBank (
    IN STORAGE_BANK         *Bank
    )
{
    VARIABLE_INFO           VarInfo, NewVar;
    STORAGE_BANK            *NewBank;
    BOOLEAN                 GarbageCollected, RemoveVar;

    ASSERT_LOCKED (&VariableStoreLock);

    //
    // Check each variable in the bank
    //

    VarInfo.Bank = Bank;
    VarInfo.NextVarOffset = FIRST_VARIABLE_OFFSET;
    while (RtVarParseStore (&VarInfo)) {

        // If it's not a valid entry skip it
        if (!VarInfo.Valid) {
            continue;
        }

        //
        // Lookup this var.  In the case of obsolete transition, see if 
        // we find a new (valid) copy.  In the case of a normal variable,
        // see if we find some duplicate copy
        //
        
        RtVarFind (VarInfo.Name, VarInfo.VendorGuid, &NewVar);
        RemoveVar = FALSE;

        //
        // If this variable isn't in the obsolete transition state, skip it
        //

        if ((VarInfo.State & VAR_OBSOLETE_TRANSITION) && 
            !(VarInfo.State & VAR_OBSOLETE))  {

            //
            // We have a variable that's in the middle of being updated, we need to
            // recover it.  In any case, the current in transition var needs removed.
            //
            
            RemoveVar = TRUE;
            if (NewVar.Valid) {

                //
                // We have the old copy (which is being obsoleted) and the new
                // copy.  Just obsolete the old copy and leave the new one.
                //

                DEBUG((D_VAR|D_WARN, "VarCheckBank: Variable '%s' recovered to new copy\n", VarInfo.Name));

            } else {

                //
                // There is no new copy, restore the current copy.
                // To do this we need to re-add the value and then 
                // delete the entry that is in transition.
                //

                NewBank = RtVarFindFreeSpace (Bank->VarStore, VarInfo.VarSize, &GarbageCollected);

                //
                // If there was a garbage collection in response to the 
                // free space request, the bank data may have been mvoed.
                // Restart the search in the var store.
                //

                if (GarbageCollected) {
                    return TRUE;
                }

                //
                // Make a new copy of the variable in the free space
                //

                if (NewBank) {

                    RtVarMove (FALSE, &VarInfo, NewBank);
                    DEBUG((D_VAR|D_WARN, "VarCheckBank: Variable '%s' recovered to old copy\n", VarInfo.Name));

                } else {

                    DEBUG((D_VAR|D_ERROR, "VarCheckBank: Could not recover variable '%s'\n", VarInfo.Name));
                }
            }
        } else {
            //
            // Variable is not in transition; however, if we found a different
            // copy then remove the current one.
            //

            if (NewVar.Valid && 
                (NewVar.Bank  != VarInfo.Bank || NewVar.VarOffset != VarInfo.VarOffset)) {

                DEBUG((D_VAR|D_WARN, "VarCheckBank: Removed duplicate variable '%s'\n", VarInfo.Name));
                RemoveVar = TRUE;
            }
        }

        //
        // If we need to obsolete the transition copy, do so 
        //

        if (RemoveVar) {
            Bank->InUse -= VarInfo.VarSize;
            RtVarUpdateState (&VarInfo, VAR_OBSOLETE);
        }
    }

    return FALSE;
}



INTERNAL
EFI_STATUS
VarStoreAddBank (
    IN VARIABLE_STORE       *VarStore,
    IN EFI_VARIABLE_STORE   *Device,
    IN UINTN                BankNo
    )
{
    EFI_STATUS              Status;
    STORAGE_BANK            *Bank;
    VOID                    *Buffer;
    UINTN                   VarSize;
    VARIABLE                *Var;
    VARIABLE_INFO           VarInfo;


    Status = EFI_SUCCESS;

    ASSERT_LOCKED (&VariableStoreLock);

    Bank = NULL;

    //
    // Allocate memory to track this bank of storage
    // 

    Status = BS->AllocatePool (
                VarStore->MemoryType, 
                sizeof(STORAGE_BANK) + Device->BankSize,
                &Bank
                );

    if (EFI_ERROR(Status)) {
        DEBUG ((D_VAR, "VarStoreAddBank: allocate pool failed\n"));
        return Status;
    }

    //
    // Initialize the bank structure
    //

    ZeroMem (Bank, sizeof(STORAGE_BANK));
    Bank->Signature = STORAGE_BANK_SIGNATURE;
    Bank->Device = Device;
    Bank->BankNo = BankNo;
    Bank->BankSize = Device->BankSize;
    Bank->VarStore = VarStore;
    Bank->u.Data = (CHAR8 *) Bank + sizeof(STORAGE_BANK);
    Bank->TSize = &Bank->BankSize;

    //
    // Read the bank's header and get the banks' disposition
    //

    Buffer = RtVarReadStore (Bank, 0, sizeof(BANK_HEADER));
    Bank->State = Bank->u.Header->State ^ 0xFF;

    //
    // If there's not valid signature, or if the bank is in add transition
    // but hasn't made it to inuse transition, then there is nothing interesting
    // in the bank - clear it.
    //
    // The current store format is type 0.  Since this is the first type, there
    // is no code to migrate to a new type.  Just reset the store if it's a different
    // type.
    //


    if (Bank->u.Header->Signature != BANK_HEADER_SIGNATURE || Bank->u.Header->Format != VARH_FORMAT ||
        (Bank->State & VARH_ADD_TRANSITION) && !(Bank->State & VARH_INUSE_TRANSITION)) {

        RtVarClearStore(Bank);

    } else {

        //
        // Add the bank to the active list. Attempt to keep them
        // in oldest to youngest order such that we more or less
        // use them in order
        //

        if (Bank->u.Header->Sequence >= VarStore->Sequence) {

            InsertTailList (&VarStore->Active, &Bank->Link);
            VarStore->Sequence = Bank->u.Header->Sequence;

        } else {

            InsertHeadList (&VarStore->Active, &Bank->Link);
        }

    }

    //
    // Now we need to make a pass over the data in the bank 
    // and compute the dynamic values for the bank structure
    //

    VarInfo.Bank = Bank;
    VarInfo.NextVarOffset = FIRST_VARIABLE_OFFSET;
    VarInfo.Bank->InUse = FIRST_VARIABLE_OFFSET;
    VarInfo.Bank->Tail = VarInfo.Bank->BankSize;
    while (VarInfo.NextVarOffset < Bank->BankSize) {

        //
        // If we don't have enough space for an entry, don't read anymore
        //

        if (VarInfo.NextVarOffset + sizeof(VARIABLE) + sizeof(UINT8) + 2 + 1 > VarInfo.Bank->BankSize) {
            VarInfo.NextVarOffset = VarInfo.Bank->BankSize;
            break;
        }

        //
        // Read the header of the variable
        //
        
        Var = RtVarReadStore(VarInfo.Bank, VarInfo.NextVarOffset, sizeof(VARIABLE));
        if (Bank->DeviceError) {
            break;
        }

        //
        // If the first bytes of Variable header haven't been initialized,
        // then we've found the tail in the bank
        //

        if (Var->Size == 0xFFFF || Var->Size == 0x0000) {
            break;
        }

        //
        // Read the remaining bytes of this variable
        //

        VarSize = (Var->Size & VAR_SIZE_MASK) - sizeof(VARIABLE);
        if (VarSize + VarInfo.NextVarOffset > VarInfo.Bank->BankSize ||
            VarSize < sizeof(UINT8) + 2 + 1) {

            //
            // Due to the way data is written to the banks, a bank should never
            // become corrupt; however, if it is corrupt recover the variables 
            // up to the point of corruption in the bank.  Since this is 
            // not a designed recovery path the update is just done in place
            // 

            DEBUG ((D_VAR|D_ERROR, "InitVarStore: bank was corrupt. truncating it.\n"));
            RtVarClearStore (VarInfo.Bank);
            RtVarUpdateStore (VarInfo.Bank, 0, VarInfo.Bank->Tail);
            break;
        }

        RtVarReadStore (VarInfo.Bank, VarInfo.NextVarOffset + sizeof(VARIABLE), VarSize);

        //
        // Crack the variable entry
        //

        RtVarParseStore (&VarInfo);

        //
        // If it's a valid value that is not completely obsolete,
        // count it in InUse
        //

        if (VarInfo.Valid) {
            VarInfo.Bank->InUse += VarInfo.VarSize;
        }
    }

    VarInfo.Bank->Tail = VarInfo.NextVarOffset;

    //
    // If there's been an error, remove the bank
    //

    if (VarInfo.Bank->DeviceError) {

        RtVarBankError (VarInfo.Bank);

    } else {

        DEBUG ((D_VAR, "VarStoreAddBank: Added bank %X:%d (type %a)\n", Device, BankNo, VarStore->Type));

        //
        // If any bank on the store doesn't support transaction update,
        // then we need to allocate one of the bank to do our own
        // bank transactioning
        //

        if (!Bank->Device->TransactionUpdate) {
            VarStore->TBankRequired = TRUE;
        }
    }
    return Status;
}
