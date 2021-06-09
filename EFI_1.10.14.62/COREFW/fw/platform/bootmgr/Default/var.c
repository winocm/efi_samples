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

    EFI Boot Manager, variable manipulation



Revision History

--*/


#include "bm.h"


//
// Internal prototypes
//

VOID
BmVarCleanup (
    IN OUT BM_VARIABLE  *Var
    );

BOOLEAN
BmParseBootOption (
    IN CHAR16       *Name,
    IN UINT8        *Data,
    IN UINTN        DataSize,
    IN CHAR16       *OptionName,
    IN LIST_ENTRY   *OptionList
    );

VOID
BmOrderOptions (
    IN BM_VARIABLE      *Order,
    IN LIST_ENTRY       *OptionList,
    IN LIST_ENTRY       *OrderList
    );


//
//
//

VOID
BmReadVariables (
    VOID
    )
{
    CHAR16              *Name;
    VOID                *Data;
    UINTN               BufferSize;
    UINTN               NameSize, DataSize;
    EFI_GUID            Id;
    UINT32              Attributes;
    EFI_STATUS          Status;
    BOOLEAN             Parsed;
    UINTN               Index;
    BM_VARIABLE         *Var;
    BOOLEAN             FoundBootOption;

    //
    // Initialize the load option lists
    //

    InitializeListHead (&BmDriverOptions);
    InitializeListHead (&BmBootOptions);
   
    BmBootOrder.MaxIndex = 0;
    BmDriverOrder.MaxIndex = 0;

    BufferSize = 1024;
    Name = AllocatePool (BufferSize);
    Data = AllocatePool (BufferSize);
    ASSERT (Name && Data);

    //
    // Read all variables in the system and collect ours
    //
    Name[0] = 0;
    FoundBootOption = FALSE;
    for (; ;) {
        NameSize = BufferSize;
        Status = RT->GetNextVariableName (&NameSize, Name, &Id);
        if (EFI_ERROR(Status)) {
            break;
        }

        // If this is not an EFI defined variable, skip it
        if (CompareGuid(&Id, &EfiGlobalVariable)) {
            continue;
        }

        //
        // It's an EFI variable, read the data
        //
        DataSize = BufferSize;
        Status = RT->GetVariable (Name, &Id, &Attributes, &DataSize, Data);
        ASSERT (!EFI_ERROR(Status));

        //
        // See if it's a boot or driver option
        //
        Parsed = BmParseBootOption (Name, Data, DataSize, L"Boot", &BmBootOptions);
        if (Parsed) {
            FoundBootOption = TRUE;
            continue;
        }

        Parsed = BmParseBootOption (Name, Data, DataSize, L"Driver", &BmDriverOptions);
        if (Parsed) {
            FoundBootOption = TRUE;
            continue;
        }

        //
        // Variable is not an driver or boot option, check for match
        //
        for (Index=0; BmVariables[Index]; Index++) {
            Var = BmVariables[Index];
            if (StrCmp(Name, Var->Name) == 0) {
                BmSetVariable (Var, Data, DataSize);
                if (Var->BootDefault) {
                    FoundBootOption = TRUE;
                }
                break;
            }
        }
    }

    //
    // Writes now get shadowed NVRAM
    //
    BmUpdateVariables = TRUE;

    if (!FoundBootOption) {
        //
        // Set Defaults based on OEM preference. The current policy is if no boot 
        // variables exist set defaults.
        //
        BmSetDefaultDriverOptions (&BmDriverOptions);
        BmSetDefaultBootOptions (&BmBootOptions);
    }

    //
    // Do some sanity checking.  Check order list references with options
    //
    BmOrderOptions (&BmBootOrder, &BmBootOptions, &BmOrderedBootOptions);
    BmOrderOptions (&BmDriverOrder, &BmDriverOptions, &BmOrderedDriverOptions);

    FreePool (Name);
    FreePool (Data);
}


VOID
BmRemoveVariable (
    IN CHAR16           *Name
    )
{
    EFI_STATUS          Status;

    Status = RT->SetVariable (Name, &EfiGlobalVariable, 0, 0, NULL);
    ASSERT (!EFI_ERROR(Status));
}


VOID
BmDeleteLoadOption (
    IN BM_LOAD_OPTION   *Option
    )
{
    BmRemoveVariable (Option->Name);

    FreePool (Option->Name);
    if (Option->Description) {
        FreePool (Option->Description);
    }

    if (Option->FilePath) {
        FreePool (Option->FilePath);
    }

    if (Option->Order.Flink) {
        RemoveEntryList (&Option->Order);
    }

    ASSERT (!Option->All.Flink);
    FreePool (Option);
}

VOID
BmDeleteAllLoadOptions (
    IN BM_VARIABLE      *Order,
    IN LIST_ENTRY       *OrderList
    )
{
    BM_LOAD_OPTION      *Option;


    while (!IsListEmpty(OrderList)) {
        Option = CR(OrderList->Flink, BM_LOAD_OPTION, Order, BM_LOAD_OPTION_SIGNATURE);
        BmDeleteLoadOption (Option);
    }

    BmUpdateOrder (Order, OrderList);
}



VOID
BmVarCleanup (
    IN OUT BM_VARIABLE  *Var
    )
{
    //
    // Round to an even number of units
    //
    Var->DataSize = (Var->DataSize / Var->IntegralSize) * Var->IntegralSize;

    //
    // If this is not an array, make sure there's only 1 unit
    //
    if (!Var->Array && Var->DataSize > Var->IntegralSize) {
        Var->DataSize = Var->IntegralSize;
    }

    //
    // If the size is zero, free it
    //
    if (!Var->DataSize && Var->u.Data) {
        BmRemoveVariable (Var->Name);
        FreePool (Var->u.Data);
        Var->u.Data = NULL;
        Var->MaxIndex = 0;
    }
}


VOID
BmSetVariable (
    IN OUT BM_VARIABLE  *Var,
    IN VOID             *Data,
    IN UINTN            DataSize
    )
{
    EFI_STATUS          Status;


    if (Var->DataSize) {
        FreePool (Var->u.Data);
        Var->DataSize = 0;
        Var->u.Data = NULL;
    }

    Var->DataSize = DataSize;
    Var->MaxIndex = DataSize / sizeof(UINT16);

    Var->u.Data = AllocatePool(DataSize);
    ASSERT (Var->u.Data);
    CopyMem (Var->u.Data, Data, DataSize);

    if (BmUpdateVariables) {
        Status = RT->SetVariable (
                    Var->Name, 
                    &EfiGlobalVariable, 
                        (Var->NonVolatile ? EFI_VARIABLE_NON_VOLATILE : 0) | 
                        EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                        (Var->RuntimeAccess ? EFI_VARIABLE_RUNTIME_ACCESS : 0),
                    DataSize, 
                    Data
                    );

        ASSERT (!EFI_ERROR(Status));
    }
    BmVarCleanup (Var);
}



BOOLEAN
BmParseBootOption (
    IN CHAR16       *Name,
    IN UINT8        *Data,
    IN UINTN        DataSize,
    IN CHAR16       *OptionName,
    IN LIST_ENTRY   *OptionList
    )
{
    UINTN               Size;
    UINTN               Value;    
    CHAR16              *Digits;
    BM_LOAD_OPTION      *Option;
    UINT8               *End;
    EFI_DEVICE_PATH     *DevicePathNode;

    //
    // If the name of the form NAMExxxx where XXXX is a non-zero value?
    //
    Size = StrSize(OptionName) - sizeof(CHAR16);
    if (CompareMem(Name, OptionName, Size)) {
        return FALSE;
    }

    Digits = Name + Size / sizeof(UINT16);
    Value = xtoi(Digits);
    if (!Value) {
        //
        // check for a special case 'Boot0000' or 'Driver0000' as xtoi would
        // return '0' whether input is '0' or 'non-hex' characters found
        //
        if (CompareMem (Digits,L"0000",4*sizeof(UINT16))) {
            return FALSE;
        }
    }

    //
    //  Got a valid variable name
    //
    Option = AllocateZeroPool(sizeof(BM_LOAD_OPTION));
    Option->Signature = BM_LOAD_OPTION_SIGNATURE;
    Option->Name = StrDuplicate(Name);
    Option->OptionNumber = Value;
    InsertTailList (OptionList, &Option->All);

    //
    // If there's not at least 10 chars, then it's not a valid entry
    //
    if (DataSize < 10) {
        goto Done;
    }

    //
    // First 32bits are the load option attributes
    //
    CopyMem (&Option->Attributes, Data, sizeof (UINT32));
    Data += sizeof(UINT32);
    DataSize -= sizeof(UINT32);

    //
    // Next 16-bits are the load option FilePathListLength
    //
    CopyMem (&Option->FilePathListLength, Data, sizeof (UINT16));
    Data += sizeof(UINT16);
    DataSize -= sizeof(UINT16);
    //
    // Next is a null terminated string
    //
    Option->Description = AllocatePool (DataSize);
    CopyMem (Option->Description, Data, DataSize);

    // find the string terminator
    Data = (UINT8 *) Option->Description;
    End = Data + DataSize;
    while (*((CHAR16 *) Data)) {
        if (Data > End - sizeof(CHAR16) - 1) {
            goto Done;
        }
        Data += sizeof(UINT16);
    }
    Data += sizeof(UINT16);
    DataSize = End - Data;
        
    //
    // Next is the file path
    //
    Option->FilePath = AllocatePool (DataSize);
    CopyMem (Option->FilePath, Data, DataSize);

    // find the end of path terminator to check for errors
    DevicePathNode = (EFI_DEVICE_PATH *) Data;
    while (!IsDevicePathEnd (DevicePathNode)) {
        DevicePathNode = NextDevicePathNode (DevicePathNode);
        if ((UINT8 *) DevicePathNode > End - sizeof(EFI_DEVICE_PATH)) {
            goto Done;
        }
    }

    Data = ((UINT8 *) Data + Option->FilePathListLength);
    if (Data > End) {
        goto Done;
    }
    DataSize = End - Data;

    //
    // Next is the load options
    //
    if (DataSize) {
      Option->LoadOptions = AllocatePool (DataSize);
      CopyMem (Option->LoadOptions, Data, DataSize);
      Option->LoadOptionsSize = DataSize;
    }

    //
    // Valid
    //
    Option->Valid = TRUE;

Done:
    return TRUE;
}

        

VOID
BmOrderOptions (
    IN BM_VARIABLE      *Order,
    IN LIST_ENTRY       *OptionList,
    IN LIST_ENTRY       *OrderList
    )
{
    BOOLEAN             Remove;
    LIST_ENTRY          *Link;
    UINTN               Index;
    UINTN               OptionNumber;
    BM_LOAD_OPTION      *Option;


    InitializeListHead (OrderList);

    //
    // Walk the ordering array and build a new list to the requested order
    //
    for (Index=0; Index < Order->MaxIndex; Index++) {
        OptionNumber = Order->u.Value[Index];

        //
        // Find this load option
        //
        for (Link=OptionList->Flink; Link != OptionList; Link = Link->Flink) {
            Option = CR(Link, BM_LOAD_OPTION, All, BM_LOAD_OPTION_SIGNATURE);

            if (Option->OptionNumber == OptionNumber) {

                //
                // If it's not already found, add it to the end of the ordered list
                //
                if (!Option->Order.Flink) {
                    InsertTailList (OrderList, &Option->Order);
                }

                break;
            }
        }
    }

    //
    // Remove any invalid, or any unreferenced options
    //
    while (!IsListEmpty(OptionList)) {
        Option = CR(OptionList->Flink, BM_LOAD_OPTION, All, BM_LOAD_OPTION_SIGNATURE);
        RemoveEntryList (&Option->All);
        Option->All.Flink = NULL;

        Remove = FALSE;
        if (!Option->Valid) {
            DEBUG ((D_BM | D_ERROR, "BmOrderOptions: Removing un-parsable load option: %s\n", Option->Name));
            Remove = TRUE;
        }

        if (!Option->Order.Flink) {
            DEBUG ((D_BM | D_ERROR, "BmOrderOptions: Removing un-referenced load option: %s\n", Option->Name));
            Remove = TRUE;
        }

        if (Remove) {
            BmDeleteLoadOption (Option);
        } 
    }    

    //
    // Update the ordered list if needed
    //
    BmUpdateOrder(Order, OrderList);
}


VOID
BmUpdateOrder (
    IN BM_VARIABLE      *Order,
    IN LIST_ENTRY       *OrderList
    )
{
    UINTN               Index, Count;
    LIST_ENTRY          *Link;
    BM_LOAD_OPTION      *Option;
    BOOLEAN             UpdateOrder;
    EFI_STATUS          Status;

    UpdateOrder = FALSE;

    //
    // Count the number of load options
    //
    Count = 0;
    for (Link=OrderList->Flink; Link != OrderList; Link = Link->Flink) {
        Count = Count + 1;
    }

    //
    // If the order array is too small, allocate a new one
    //
    if (Order->MaxIndex < Count) {
        if (Order->u.Data) {
            FreePool (Order->u.Data);
        }
        Order->u.Data = AllocatePool(Count * sizeof(UINT16));
    }

    //
    // Fill If the order array was not valid, update it
    //
    Index = 0;
    for (Link=OrderList->Flink; Link != OrderList; Link = Link->Flink) {
        Option = CR(Link, BM_LOAD_OPTION, Order, BM_LOAD_OPTION_SIGNATURE);

        if (Order->u.Value[Index] != Option->OptionNumber) {
            Order->u.Value[Index] = (UINT16) Option->OptionNumber;
            UpdateOrder = TRUE;
        }

        Index += 1;
        if (Index >= Count) {
            break;
        }
    }

    if (Index != Order->MaxIndex || UpdateOrder) {
        DEBUG ((D_BM | D_ERROR, "BmUpdateOrder: Updating order list: %s\n", Order->Name));

        Order->MaxIndex = Index;
        Order->DataSize = Index * sizeof(UINT16);
        Status = RT->SetVariable (
                    Order->Name, 
                    &EfiGlobalVariable, 
                    (OrderList == &BmOrderedBootOptions) ?
                        (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS) :
                        (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS),
                    Order->DataSize, 
                    Order->u.Data
                    );

        ASSERT (!EFI_ERROR(Status));
    }
}


VOID
BmSetBootCurrent(
    BOOLEAN Set,
    UINT16  Value
    )
{
    EFI_STATUS Status;
    UINTN      Size;

    Size = sizeof(UINT16);
    if (Set == FALSE) {
        Size = 0;
    }

    Status = RT->SetVariable (
                VarBootCurrent,
                &EfiGlobalVariable, 
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                Size,
                &Value
                );
}

