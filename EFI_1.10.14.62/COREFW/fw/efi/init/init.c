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

    init.c

Abstract:

    Initialize code



Revision History

--*/

#include "efifw.h"

#ifdef  RUNTIME_CODE
#pragma RUNTIME_CODE(ExitBootServices)
#endif


//
// Internal prototypes
//

VOID
EFIFirmwareBanner (
    VOID
    );


//
// EFI's system table
// EFI's boot service table
// EFI's runtime service table
// EFI's internal firmware table
//

extern INTERNAL EFI_SYSTEM_TABLE         SystemTable;
extern INTERNAL EFI_BOOT_SERVICES        BootServices;
extern INTERNAL EFI_RUNTIME_SERVICES     RuntimeServices;
extern INTERNAL EFI_FIRMWARE_TABLE       FirmwareTable;

extern 
BOOLEAN 
SetInterruptStateThunk(
    IN BOOLEAN  Enable
    );

VOID
EFIEntryPoint (
    IN struct _EFI_PLATFORM_TABLE   *PlTable,
    OUT struct _EFI_FIRMWARE_TABLE  **Fw,
    OUT EFI_SYSTEM_TABLE            **St
    )
{

    //
    // Remember EmTable
    //

    PL = PlTable;

    //
    // Set globals we need before lib is initialized
    //

    ST = &SystemTable;
    BS = SystemTable.BootServices;
    RT = SystemTable.RuntimeServices;

    //
    // Get internal components ready to receive
    // a memory map
    //

    InitializeEvent();
    InitializeHandle();
    InitializeMemoryMap();
    InitializeGuid();
    InitializeLoader();
    LibRuntimeDebugOut = NULL;

    //
    // Return our tables
    //

    *Fw = &FirmwareTable;
    *St = &SystemTable;
}


VOID
FwMemoryMapInstalled (
    VOID
    )
{
    CHAR16  *FirmwareVendor;
    //
    // Set table CRCs 
    //

    SetCrc (&SystemTable.Hdr);
    SetCrc (&BootServices.Hdr);
    SetCrc (&RuntimeServices.Hdr);

    //
    // Initialze remaining internal components 
    //
    
    InitializeMemoryMapWatermarks();

    InitializeLib(NULL, &SystemTable);
    LibFwInstance = TRUE;

    InitializeSystemTablePointerStructure();
    InitializeDebugImageInfoTable();

    InitializeTimer();
    InitializeVariableStore ();

    //
    // Allow interrupts to start functioning
    //

    EfiSetInterruptState = PL->SetInterruptState;
    EfiSetInterruptState (TRUE);

    //
    // Fill in Firmware Vendor and Firmware Revision
    //

    FirmwareVendor = StrDuplicate(EFI_FIRMWARE_VENDOR);
    BS->AllocatePool (EfiRuntimeServicesData, StrSize(FirmwareVendor), &ST->FirmwareVendor);
    CopyMem (ST->FirmwareVendor, FirmwareVendor, StrSize(FirmwareVendor));
    BS->FreePool (FirmwareVendor);
    ST->FirmwareRevision = EFI_FIRMWARE_REVISION;
    SetCrc (&SystemTable.Hdr);
}


VOID
EFIFirmwareBanner (
    VOID
    )
/*++

Routine Description:

    Displays a banner on the output console to show some progress

Arguments:

    None

Returns:

    None
    
--*/
{
    //
    // Put the banner on the stdout
    //

    ST->ConOut->SetMode (ST->ConOut, 0);    // 80 x 25 mode
    ST->ConOut->SetAttribute (ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    ST->ConOut->ClearScreen (ST->ConOut);

    Print (L"EFI version %01d.%02d [%d.%d] Build flags: %E",
        (ST->Hdr.Revision >> 16),
        (ST->Hdr.Revision & 0xffff),
        ST->FirmwareRevision >> 16,
        (ST->FirmwareRevision & 0xffff));

#if EFI32
    Print (L"EFI32 ");
#endif

#if EFI64
    Print (L"%HEFI64%N Running on %EIntel(R) Itanium Processor%N ");
#endif

#if EFI_DEBUG
    Print (L"EFI_DEBUG ");
#endif

#if EFI_NT_EMULATOR
    Print (L"EFI_NT_EMULATOR ");
#endif

#if EFI_FW_NT
    Print (L"EFI_FW_NT ");
#endif

#if SOFT_SDV
    Print (L"SOFT_SDV ");
#endif

    Print (L"%N\n");
}

EFI_STATUS
BOOTSERVICE
InstallConfigurationTable (
    IN EFI_GUID *Guid,
    IN VOID     *Table
    )
/*++

Routine Description:

    Boot Service called to add, modify, or remove a system configuration table from 
    the EFI System Table.

Arguments:

    Guid

    Table

Returns:

    Status code

--*/
{
    EFI_STATUS              Status;
    UINTN                   Index;
    EFI_CONFIGURATION_TABLE *EfiConfigurationTable;

    //
    // If Guid is NULL, then this operation can not be performed
    //

    if (Guid == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Search all the table for an entry that matches Guid
    //

    for (Index = 0; Index < ST->NumberOfTableEntries; Index++) {

        if (CompareGuid(&(ST->ConfigurationTable[Index].VendorGuid), Guid) == 0) {

            //
            // A match was found.  
            //

            if (Table != NULL) {

                //
                // If Table is not NULL, then this is a modify operation.
                // Modify the table enty and return.
                //

                ST->ConfigurationTable[Index].VendorTable = Table;
                return EFI_SUCCESS;
            }

            //
            // A match was found and Table is NULL, so this is a delete operation.
            //

            ST->NumberOfTableEntries--;
            EfiConfigurationTable = NULL;

            if (ST->NumberOfTableEntries != 0) {

                //
                // There was more than one entry in the table, so we have to allocate
                // a new table with one less entry, and copy all but the matching entry to
                // the new table.
                //

                Status = BS->AllocatePool(EfiRuntimeServicesData,
                                          ST->NumberOfTableEntries * sizeof(EFI_CONFIGURATION_TABLE),
                                          &EfiConfigurationTable);

                //
                // If a new table could not be allocated, then return an error.
                //

                if (EFI_ERROR(Status)) {
                    ST->NumberOfTableEntries++;
                    return EFI_OUT_OF_RESOURCES;
                }

                //
                // Copy all but the matching entry to the new table.
                //

                if (Index > 0) {
                    CopyMem (EfiConfigurationTable,
                             ST->ConfigurationTable,
                             Index * sizeof(EFI_CONFIGURATION_TABLE)
                             );
                }
                if (Index < ST->NumberOfTableEntries) {
                    CopyMem (&(EfiConfigurationTable[Index]),
                             &(ST->ConfigurationTable[Index+1]),
                             (ST->NumberOfTableEntries - Index) * sizeof(EFI_CONFIGURATION_TABLE)
                             );
                }
            }

            //
            // Free the old table, point to the new table, and recompute the CRC.
            //

            FreePool(ST->ConfigurationTable);
            ST->ConfigurationTable = EfiConfigurationTable;
            SetCrc (&ST->Hdr);
            return EFI_SUCCESS;
        }
    }

    //
    // No matching GUIDs were found, so this is an add operation.
    //

    if (Table == NULL) {

        //
        // If Table is NULL on an add operation, then return an error.
        //

        return EFI_NOT_FOUND;
    }

    Index = ST->NumberOfTableEntries;
    ST->NumberOfTableEntries++;

    //
    // Allocate a table with one additional entry.
    //

    Status = BS->AllocatePool(EfiRuntimeServicesData,
                              ST->NumberOfTableEntries * sizeof(EFI_CONFIGURATION_TABLE),
                              &EfiConfigurationTable);

    //
    // If a new table could not be allocated, then return an error.
    //

    if (EFI_ERROR(Status)) {
        ST->NumberOfTableEntries--;
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Copy the old table to the new table.
    //

    if (Index != 0) {
        CopyMem (EfiConfigurationTable,
                 ST->ConfigurationTable,
                 Index * sizeof(EFI_CONFIGURATION_TABLE)
                 );
    }

    //
    // Fill in the new entry
    //

    EfiConfigurationTable[Index].VendorGuid  = *Guid;
    EfiConfigurationTable[Index].VendorTable = Table;

    //
    // Free the old table, point to the new table, and recompute the CRC.
    //

    if (ST->ConfigurationTable != NULL) {
        FreePool(ST->ConfigurationTable);
    }
    ST->ConfigurationTable = EfiConfigurationTable;
    SetCrc (&ST->Hdr);
    return EFI_SUCCESS;
}

EFI_STATUS
RUNTIMESERVICE
ExitBootServices (
    IN EFI_HANDLE               ImageHandle,
    IN UINTN                    MapKey
    )
/*++

Routine Description:

    Boot Service called to terminate all boot services

Arguments:


Returns:

    Status code

--*/
{
    EFI_STATUS                      Status;

    //
    // Can only exit while at normal tpl
    //

    if (CurrentTPL() != TPL_APPLICATION) {
        return EFI_UNSUPPORTED;
    }

    //
    // Terminate memory map services
    //

    Status = TerminateMemoryMap (MapKey);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Notify everyone that we're at runtime now
    //

    RtNotifySignalList (EVT_SIGNAL_EXIT_BOOT_SERVICES);

    //
    // Stop adjusting the processors interrupt flag
    //

    EfiSetInterruptState = SetInterruptStateThunk;

    //
    // Turn off all debug messages
    //

    EFIDebug = 0;

    //
    // Setup global variables that point to the Runtime functions in the Boot Services Table.
    //

    LibRuntimeRaiseTPL   = BS->RaiseTPL;
    LibRuntimeRestoreTPL = BS->RestoreTPL;

    
    //
    // Clear various non-runtime pointers
    //
    ST->BootServices        = NULL;
    ST->ConIn               = NULL;
    ST->ConsoleInHandle     = NULL;
    ST->ConOut              = NULL;
    ST->ConsoleOutHandle    = NULL;
    ST->StdErr              = NULL;
    ST->StandardErrorHandle = NULL;
    SetCrc (&ST->Hdr);

    //
    // Clear the private firmware services table
    //
    ZeroMem (&FirmwareTable, sizeof(FirmwareTable));

    //
    // Clear the EFI Boot Services Tables and clear the BS global variable
    //
    ZeroMem (&BootServices, sizeof(BootServices));
    BS = NULL;

    //
    // Cleanup any information not needed during boot services
    // 

    RtLoaderExitBootServices ();

    //
    // In case we have runtime support, provide a debug message
    //

    DEBUG ((D_INIT, "%EExitBootServices has occurred%N\n"));
    return EFI_SUCCESS;
}
