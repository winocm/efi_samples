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
//
//
#pragma RUNTIME_CODE(RtAcquireStoreLock)
VOID
INTERNAL RUNTIMEFUNCTION
RtAcquireStoreLock (
    VOID
    )
{
    AcquireLock (&VariableStoreLock);
}

#pragma RUNTIME_CODE(RtReleaseStoreLock)
VOID
INTERNAL RUNTIMEFUNCTION
RtReleaseStoreLock (
    VOID
    )
{
    RtReleaseLock (&VariableStoreLock);
}

#pragma RUNTIME_CODE(RtVarFind)
EFI_STATUS
INTERNAL RUNTIMEFUNCTION
RtVarFind (
    IN CHAR16               *Name,
    IN EFI_GUID             *VendorGuid,
    OUT VARIABLE_INFO       *VarInfo
    )
{
    VARIABLE_STORE          *VarStore;
    LIST_ENTRY              *Link;

    ASSERT_LOCKED (&VariableStoreLock);

    //
    // Search all variable stores
    //

    for (VarStore=VariableStore; VarStore; VarStore=VarStore->Next) {

        //
        // If this is runtime, only skip any non-runtime variable store
        //

        if (EfiAtRuntime && VarStore->MemoryType != EfiRuntimeServicesData) {
            continue;
        }

        //
        // Search all banks in this store
        //

        for (Link=VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
            VarInfo->Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);

            //
            // Search all variables in this bank
            //

            VarInfo->NextVarOffset = FIRST_VARIABLE_OFFSET;
            while (RtVarParseStore (VarInfo)) {

                //
                // If it's a valid entry, and it's not an obsolete entry,
                // or being transitioned to obslete, and the name & guid
                // matches the request - then this the entry we're looking
                // for.
                //

                if (VarInfo->Valid &&
                    !(VarInfo->State & VAR_OBSOLETE_TRANSITION) &&
                    StrCmp(Name, VarInfo->Name) == 0 &&
                    CompareGuid(VendorGuid, VarInfo->VendorGuid) == 0) {

                    if (!EfiAtRuntime || (EfiAtRuntime && (VarInfo->Attributes & EFI_VARIABLE_RUNTIME_ACCESS))) {

                        //
                        // Got a match
                        //

                        return EFI_SUCCESS;
                    }
                }
            }
        }
    }

    //
    // Variable was not found
    //

    VarInfo->Valid = FALSE;
    return EFI_NOT_FOUND;
}


#pragma RUNTIME_CODE(RtVarFindFreeSpace)
STORAGE_BANK *
STATIC RUNTIMEFUNCTION
RtVarFindFreeSpace (
    IN VARIABLE_STORE           *VarStore,
    IN UINTN                    Size,
    OUT BOOLEAN                 *GarbageCollected
    )
{
    LIST_ENTRY                  *Link;
    STORAGE_BANK                *Bank;
    BOOLEAN                     Updated;

    *GarbageCollected = FALSE;

    for (; ;) {
        //
        // Search the active banks for the available space
        //

        for (Link=VarStore->Active.Flink; Link != &VarStore->Active; Link=Link->Flink) {
            Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);

            if (!Bank->DeviceError  && 
                Bank->Tail + Size  < Bank->BankSize  &&
                Bank->InUse + Size < *Bank->TSize) {
                // There's enough space in this bank, use it
                return Bank;
            }
        }

        //
        // Perform some garbage collection to make some space
        //

        Updated = RtVarGarbageCollect (VarStore);

        //
        // If there were no updates from the garbage collection 
        // then there is no space.
        //

        if (!Updated) {
            Bank = NULL;
            break;
        }

        //
        // Inform the caller that garbage collection was done
        //

        *GarbageCollected = TRUE;
    }

    DEBUG((D_VAR|D_WARN, "VarFindFreeSpace: no space found for %a (needed %d)\n", VarStore->Type, Size));
    return NULL;
}

#pragma RUNTIME_CODE(RtGetVariable)
EFI_STATUS
RUNTIMESERVICE
RtGetVariable (
    IN CHAR16                       *VariableName,
    IN EFI_GUID                     *VendorGuid,
    OUT UINT32                      *Attributes OPTIONAL,
    IN OUT UINTN                    *DataSize,
    OUT VOID                        *Data
    )
{
    VARIABLE_INFO                   VarInfo;
    EFI_STATUS                      Status;

    if (VariableName == NULL || VendorGuid == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    RtAcquireStoreLock(); 

    Status = RtVarFind (VariableName, VendorGuid, &VarInfo);
    if( !EFI_ERROR(Status) )
		{
        ASSERT (VarInfo.Valid);

        if( DataSize == NULL )
				{
            Status = EFI_INVALID_PARAMETER;
        }
        else
        {
          if( *DataSize < VarInfo.DataSize )
          {
           Status = EFI_BUFFER_TOO_SMALL;
          }
          else
          {
            if( Data == NULL )
            {
              Status = EFI_INVALID_PARAMETER;
            }
            else
            {
              CopyMem (Data, VarInfo.Data, VarInfo.DataSize);
              if( Attributes )
              {
                *Attributes = VarInfo.Attributes;
              }
            }
          }
            
          *DataSize = VarInfo.DataSize;
        }
    }
    
    RtReleaseStoreLock ();
    return Status;
}


#pragma RUNTIME_CODE(RtGetNextVariableName)
EFI_STATUS
RUNTIMESERVICE
RtGetNextVariableName (
    IN OUT UINTN                    *VariableNameSize,
    IN OUT CHAR16                   *VariableName,
    IN OUT EFI_GUID                 *VendorGuid
    )
{
    VARIABLE_INFO                   VarInfo;
    VARIABLE_STORE                  *VarStore;
    EFI_STATUS                      Status;
    LIST_ENTRY                      *Link;

    if (VariableNameSize == NULL || VariableName == NULL || VendorGuid == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    RtAcquireStoreLock ();

    //
    // Get last position
    //

    if (*VariableName) {

        //
        // Find last position
        //

        Status = RtVarFind (VariableName, VendorGuid, &VarInfo);
        if (EFI_ERROR(Status)) {
            goto Done;
        }

        Link = &VarInfo.Bank->Link;
        VarStore = VarInfo.Bank->VarStore;

    } else {

        // 
        // Set to first entry
        //

        VarStore = VariableStore;
        if (EfiAtRuntime) {
            while (VarStore && VarStore->MemoryType != EfiRuntimeServicesData) {
                VarStore = VarStore->Next;
            }
        }
        Link = NULL;
    }

    //
    // Advance to next position
    //

    Status = EFI_NOT_FOUND;
    while (VarStore) {

        //
        // If we don't have a bank position, set to head of current store
        //

        if (!Link) {
            Link = VarStore->Active.Flink;
            VarInfo.NextVarOffset = FIRST_VARIABLE_OFFSET;
        }

        //
        // If we're off the end of this var store, adcance to the next one
        //

        if (Link == &VarStore->Active) {
            VarStore = VarStore->Next;
            if (EfiAtRuntime) {
                while (VarStore && VarStore->MemoryType != EfiRuntimeServicesData) {
                    VarStore = VarStore->Next;
                }
            }
            Link = NULL;
            continue;
        }

        //
        // Check the entry at the current location
        //

        VarInfo.Bank = CR(Link, STORAGE_BANK, Link, STORAGE_BANK_SIGNATURE);
        if (!RtVarParseStore (&VarInfo)) {
            //
            // End of bank hit, try the next one
            //

            Link = Link->Flink;
            VarInfo.NextVarOffset = FIRST_VARIABLE_OFFSET;
            continue;
        }

        //
        // If this is a valid entry, return it
        //
        if (VarInfo.Valid) {

            if (!EfiAtRuntime || (EfiAtRuntime && (VarInfo.Attributes & EFI_VARIABLE_RUNTIME_ACCESS))) {

                Status = EFI_BUFFER_TOO_SMALL;
                if (*VariableNameSize >= VarInfo.NameSize) {
                    CopyMem (VariableName, VarInfo.Name, VarInfo.NameSize);
                    CopyMem (VendorGuid, VarInfo.VendorGuid, sizeof(EFI_GUID));
                    Status = EFI_SUCCESS;        
                }

                *VariableNameSize = VarInfo.NameSize;
                break;
            }
        }
    }

Done:    
    RtReleaseStoreLock ();
    return Status;
}

#pragma RUNTIME_CODE(RtSetVariable)
EFI_STATUS
RUNTIMESERVICE
RtSetVariable (
    IN CHAR16                       *VariableName,
    IN EFI_GUID                     *VendorGuid,
    IN UINT32                       Attributes,
    IN UINTN                        DataSize,
    IN VOID                         *Data
    )
{
    VARIABLE_STORE                  *VarStore;
    VARIABLE_INFO                   OldVar, NewVar;
    UINTN                           NameSize,VarSize, PadSize;
    BOOLEAN                         Failed, Junk;
    VARIABLE                        *Var;
    EFI_STATUS                      Status;
    CHAR8                           *Buffer;

    Failed = FALSE;
    Status = EFI_SUCCESS;

    //
    // Make sure VariableName is valid
    //

    if (VariableName == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (VariableName[0] == 0x0000) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Make sure that the input variable does not exceed the maximum size
    //

    if (DataSize > EFI_MAXIMUM_VARIABLE_SIZE) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Make sure reserved bits aren't set
    //

    if (Attributes & ~EFI_VARIABLE_VALID_ATTRIBUTES) {
        return EFI_INVALID_PARAMETER;
    }
    
    //
    // Make sure if runtime bit is set, boot service bit is set also
    //
    if ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == EFI_VARIABLE_RUNTIME_ACCESS ) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Determine which store to save the variable in
    //

    VarStore = VariableStoreType[Attributes & VAR_ATTRIBUTE_TYPE];
    if (VarStore) {
        if (IsListEmpty(&VarStore->Active) && !(Attributes & EFI_VARIABLE_RUNTIME_ACCESS)) {
            VarStore = VariableStoreType[(Attributes | EFI_VARIABLE_RUNTIME_ACCESS) & VAR_ATTRIBUTE_TYPE];
        }
    }
    if (!DataSize) {
        VarStore = NULL;
    }

    //
    // If this is runtime, make sure variable store is accessible
    //

    if (EfiAtRuntime && VarStore && !VarStore->RuntimeUpdate) {
        return EFI_UNSUPPORTED;
    }
    
    //
    // Lock the variable store and make the update
    //

    RtAcquireStoreLock ();

    //
    // If we're going to save an update, compute the amount of space 
    // and locate the bank to write it too
    //

    if (VarStore) {
        NameSize = StrSize(VariableName);
        VarSize = sizeof(VARIABLE) + sizeof(UINT8) + NameSize + DataSize;
        PadSize = (sizeof(UINT32) - VarSize % sizeof(UINT32)) & (sizeof(UINT32)-1);

        NewVar.Bank = RtVarFindFreeSpace (VarStore, VarSize + PadSize, &Junk);
        if (!NewVar.Bank) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }
    }

    //
    // Lookup the existing value
    //

    RtVarFind (VariableName, VendorGuid, &OldVar);

    //
    // If not saving a new value, then we can skip the
    // transistion and just mark the old value as obsolete
    //
    
    if (VarStore) {

        //
        // Mark the old variable as in transistion
        //
    
        if (OldVar.Valid) {

            //
            // If this is a set for the same value as the current
            // value, then we don't need to update the store
            //

            if (OldVar.Bank->VarStore == VarStore &&
                CompareGuid (OldVar.VendorGuid, VendorGuid) == 0 &&
                OldVar.DataSize == DataSize &&
                CompareMem (OldVar.Data, Data, DataSize) == 0) {

                goto Done2;
            }

            //
            // Mark the old entry as obsolete transition
            //

            Failed = RtVarUpdateState (&OldVar, VAR_OBSOLETE_TRANSITION);
            if (Failed) {
                goto Done;
            }
        }
    
        //
        //  Write the updated record
        //

        NewVar.NextVarOffset = NewVar.Bank->Tail;
        Buffer = NewVar.Bank->u.Data + NewVar.NextVarOffset;
        Var = (VARIABLE *) Buffer;

        // Fill in the header
        Var->Size = (UINT16) VarSize;
        Var->Attributes = (UINT16) Attributes;
        CopyMem (&Var->VendorGuid, VendorGuid, sizeof(EFI_GUID));
        Buffer += sizeof (VARIABLE);

        // append the variable name
        CopyMem (Buffer, VariableName, NameSize);
        Buffer += NameSize;

        // append the variable data
        CopyMem (Buffer, Data, DataSize);
        Buffer += DataSize;

        // append the variable state
        Buffer[0] = 0xFF ^ VAR_ADDED;

        // write it, and calculate the new bank tail
        VarSize = VarSize + PadSize;
        RtVarUpdateStore (NewVar.Bank, NewVar.NextVarOffset, VarSize);
        NewVar.Bank->Tail  += VarSize;
        NewVar.Bank->InUse += VarSize;

#if EFI_DEBUG
        RtVarParseStore (&NewVar);
        ASSERT (NewVar.Valid);
        ASSERT (VarSize == NewVar.VarSize);
        ASSERT (NewVar.Bank->Tail == NewVar.NextVarOffset);
#endif
        //
        // If there was an update error, abort
        //

        Failed = NewVar.Bank->DeviceError;
        if (Failed) {
            goto Done;
        }

    } else {
        if (!OldVar.Valid) {
            Status = EFI_NOT_FOUND;
            goto Done2;
        }
    }

    //
    // Mark the old variable entry as obsolete
    //

    if (OldVar.Valid) {
        Failed = RtVarUpdateState (&OldVar, VAR_OBSOLETE_TRANSITION | VAR_OBSOLETE);
        OldVar.Bank->InUse -= OldVar.VarSize;
    }

    //
    // Done
    //  

Done:    
    DEBUG((D_VAR, "SetVariable: %s variable '%hg:%hs' %a %es\n", 
            VarStore ? L"setting" : L"removing",
            VendorGuid, 
            VariableName,
            VarStore ? VarStore->Type : "",
            Failed ? L"failed" : L""
            ));

Done2:
    RtReleaseStoreLock ();

    if (Failed) {
        Status = EFI_DEVICE_ERROR;
    }

    return Status;
}
