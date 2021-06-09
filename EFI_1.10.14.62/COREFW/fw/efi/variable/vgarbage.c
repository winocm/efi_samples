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
// Prototypes
//

#pragma RUNTIME_CODE(RtVarMove)
VOID
INTERNAL RUNTIMEFUNCTION
RtVarMove (
    IN BOOLEAN                      BankTransition,
    IN VARIABLE_INFO                *VarInfo,
    IN STORAGE_BANK                 *NewBank
    )
// N.B. VarInfo is not updated for the new varaible    
{
    STORAGE_BANK                    *OldBank;
    UINTN                           NewOffset, Index;

    OldBank = VarInfo->Bank;
    ASSERT (!(VarInfo->State & VAR_OBSOLETE));

    //
    // If this isn't a complete bank transaction, then the
    // source entry should be marked as in transition.
    //

    if (!BankTransition) {
        ASSERT (VarInfo->State & VAR_OBSOLETE_TRANSITION);
    }

    //
    // By design there should always be enough space in the 
    // new bank, but make sure
    //

    if (NewBank->Tail + VarInfo->VarSize > NewBank->BankSize) {

        DEBUG ((D_VAR|D_ERROR, "VarMove: variable does not fit in new bank!\n"));

    } else {

        //
        // Move the data to the new bank
        //

        NewOffset = NewBank->Tail;
        CopyMem (NewBank->u.Data + NewOffset, OldBank->u.Data + VarInfo->VarOffset, VarInfo->VarSize);

        // Fix the State byte to be VAR_ADDED
        Index = NewOffset + VarInfo->StateOffset - VarInfo->VarOffset;
        NewBank->u.Data[Index] = 0xFF ^ VAR_ADDED;

        // Account for it 
        NewBank->Tail += VarInfo->VarSize;
        NewBank->InUse += VarInfo->VarSize;
    }

    //
    // If this isn't a complete bank transaction, write the new entry
    //

    if (!BankTransition) {
        RtVarUpdateStore (NewBank, NewOffset, VarInfo->VarSize);
    }
}


#pragma RUNTIME_CODE(RtVarGarbageCollect)
BOOLEAN
INTERNAL RUNTIMEFUNCTION
RtVarGarbageCollect (
    IN VARIABLE_STORE               *VarStore
    )
// returns TRUE if space was made available    
{
    EFI_VARIABLE_STORE              *Device;
    UINTN                           NoBanks;
    LIST_ENTRY                      *Link;
    STORAGE_BANK                    *Bank;
    EFI_STATUS                      Status;

    Status = EFI_SUCCESS;
    
    //
    // Find a bank that has obsolete data in it that will collect
    // into TBank, and then go garbage collect it
    //

    for (Link=VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
        Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);

        if (!Bank->DeviceError  &&
            Bank->InUse < Bank->Tail  &&  
            Bank->InUse <= *Bank->TSize) {

            RtVarGarbageCollectBank (Bank);
            return TRUE;
        }
    }    

    //
    // No bank has obsolete data in it.  See if we can allocate another.
    // NOTE: Here we could pack entries to make space
    //

    for (Link=VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
        Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);
        Device = Bank->Device;

        //
        // Does this device support SizeStore?
        //

        if (Device->SizeStore) {

            //
            // Try to allocate a new bank
            //

            NoBanks = Device->NoBanks + 1;
            Device->SizeStore (Device, NoBanks);

            //
            // If we got a new bank, initialize it and return that there
            // is more space available
            //

            if (Device->NoBanks == NoBanks) {
                Status = VarStoreAddBank (VarStore, Device, NoBanks-1);

                if (!EFI_ERROR(Status)) {
                    return TRUE;
                } else {
                    return FALSE;
                }
            }
        }
    }

    //
    // Nothing was done to make any more space
    //

    return FALSE;
}



#pragma RUNTIME_CODE(RtVarGarbageCollectBank)
VOID
INTERNAL RUNTIMEFUNCTION
RtVarGarbageCollectBank (
    IN STORAGE_BANK         *OldBank
    )
{
    STORAGE_BANK            *NewBank;
    VARIABLE_STORE          *VarStore;
    VARIABLE_INFO           VarInfo;
    EFI_STATUS              Status;
     
    //
    // Get the transaction bank ready
    //

    VarStore = OldBank->VarStore;
    NewBank = VarStore->TBank;
    VarStore->TBank = NULL;
    VarStore->TSize = 0;

    if (!VarStore->TBankRequired) {
        ASSERT (NewBank == NULL);
        NewBank = OldBank;
    }

    if (!NewBank) {
        DEBUG ((D_VAR|D_ERROR, "VarGarbageCollect: No transaction bank\n"));
        return ;
    }

    //
    // Find all the valid data variables in the bank and move them
    //
    
    VarInfo.Bank = OldBank;
    VarInfo.NextVarOffset = FIRST_VARIABLE_OFFSET;
    while (RtVarParseStore (&VarInfo)) {
        //
        // If this entry is not valid or obsolete, skip it
        //

        if (!VarInfo.Valid) {
            continue;
        }

        //
        // Move the variable to the transaction bank
        //

        RtVarMove (TRUE, &VarInfo, NewBank);
    }

    if (VarStore->TBankRequired) {

        //
        // Write the new bank, but first set the bank ADD_TRANSITION
        // so that we know that the bank is no loner a "clear" bank
        //

        RtVarUpdateBankState (NewBank, VARH_ADD_TRANSITION);
        RtVarUpdateStore     (NewBank, 0, NewBank->Tail);

        //
        // Interlocked change over to the NewBank.  
        //
        // The function works by assuming that the is only one
        // bank (per volstore) in transition to being obsolete and 
        // one bank in transition to being inuse.  If the change 
        // over is stopped in the middle, the initialization
        // code knows that the bank that banks that are in transition
        // are for each other.
        //
        // First mark the old bank as being in transition to be
        // obsolete, then mark the new bank as being in transtion 
        // to being inuse.
        //
        // Then, clear the old bank, and mark the new bank 
        // as inuse.
        //

        RtVarUpdateBankState (OldBank, VARH_OBSOLETE_TRANSITION);
        RtVarUpdateBankState (NewBank, VARH_INUSE_TRANSITION);
        RtVarClearStore (OldBank);
        RtVarUpdateBankState (NewBank, VARH_INUSE);

        //
        // Put the new bank on the tail of the active list
        //

        InsertTailList (&VarStore->Active, &NewBank->Link);

        //
        // Get a new transaction bank 
        //

        RtVarGetTransactionBank (VarStore);

    } else {

        //
        // The drive can perform an atomic update of the bank,
        // let him do it
        //
        
        ASSERT (OldBank == NewBank);
        Status = NewBank->Device->TransactionUpdate (
                    NewBank->Device,
                    NewBank->BankNo,
                    NewBank->u.Data
                    );

        if (EFI_ERROR(Status)) {
            NewBank->DeviceError = TRUE;
            DEBUG ((D_VAR|D_ERROR, "VarGarbageCollectBank: TransactionUpdate failed %x\n", Status));
        }
    }
}

#pragma RUNTIME_CODE(RtVarGetTransactionBank)
VOID
INTERNAL RUNTIMEFUNCTION
RtVarGetTransactionBank (
    IN VARIABLE_STORE       *VarStore
    )
{
    STORAGE_BANK            *Bank;
    STORAGE_BANK            *Best;
    LIST_ENTRY              *Link;

    ASSERT (!VarStore->TBank);
    ASSERT (!VarStore->TSize);
    ASSERT (VarStore->TBankRequired);
    
    //
    // If the update sequence number is at it's limit, reset it
    //

    if (VarStore->Sequence == VARH_MAX_SEQUENCE) {

        DEBUG((D_VAR, "VarGetTransactionBank: reseting update sequence values\n"));
        
        for (Link=VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
            Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);
            Bank->u.Header->Sequence = 0;
            RtVarUpdateStore (Bank, 0, sizeof(BANK_HEADER));
        }

        VarStore->Sequence = 0;
    }

    //
    // We know that the largest free bank is large enough to at least
    // hold the last bank that was garbage collected.  Plus by using
    // the largest we hold off collection as long as possible.  (And
    // it's most sane to have all banks in a store to be the same size 
    // anyway).  Find the first largest free bank
    //

    Best = NULL;
    for (Link=VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
        Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);

        //
        // Initialize TSize to be per store
        //

        Bank->TSize = &VarStore->TSize;

        //
        // Track the largest currently available
        //

        if (Bank->Tail == FIRST_VARIABLE_OFFSET) {
            if (!Best || Bank->BankSize > Best->BankSize) {
                Best = Bank;
            }
        }
    }

    if (Best) {
        //
        // Make the largest free bank the transaction bank
        //

        RemoveEntryList (&Best->Link);
        Best->Link.Flink = NULL;
        VarStore->TBank = Best;
        VarStore->TSize = Best->BankSize;

    } else {    

        DEBUG((D_VAR|D_ERROR, "VarGetTranactionBank: No available bank (%a)!\n", VarStore->Type));

    }

    return ;
}


