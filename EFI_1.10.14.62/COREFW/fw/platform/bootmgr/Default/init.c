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

    EFI Boot Manager

Revision History

--*/

#include "bm.h"

//#define EXTRA_DHCP

#ifdef EXTRA_DHCP

#include EFI_PROTOCOL_DEFINITION(PxeDhcp4)
#include EFI_PROTOCOL_DEFINITION(PxeDhcp4Callback)

EFI_PXE_DHCP4_CALLBACK_STATUS
pxe_dhcp4_callback(
	IN EFI_PXE_DHCP4_PROTOCOL *This,
	IN EFI_PXE_DHCP4_FUNCTION function,
	IN UINT32 pkt_len,
	IN DHCP4_PACKET *pkt OPTIONAL)
{
	//
	// Keep the compiler happy.
	//

	This = This;
	function = function;
	pkt_len = pkt_len;

	//
	//
	//

	return pkt ?
		EFI_PXE_DHCP4_CALLBACK_STATUS_KEEP_CONTINUE :
		EFI_PXE_DHCP4_CALLBACK_STATUS_CONTINUE;
}

VOID
extra_dhcp(VOID)
{
	EFI_PXE_DHCP4_PROTOCOL *PxeDhcp4 = NULL;
	EFI_PXE_DHCP4_CALLBACK_PROTOCOL Callback;
	EFI_STATUS efi_status = EFI_SUCCESS;
	EFI_HANDLE *handle_list = NULL;
	UINTN handle_cnt = 0;
	UINTN n = 0;
	UINTN op_len = 11;
	UINT8 *op_list = "\x3D\x09PXEClient\xFF" ;

	//
	//
	//

	Callback.Revision = EFI_PXE_DHCP4_CALLBACK_INTERFACE_REVISION;
	Callback.Callback = &pxe_dhcp4_callback;

	//
	// Find and start all PXE DHCPv4 protocol handles
	//

	LibLocateHandle(
		ByProtocol,
		&gEfiPxeDhcp4ProtocolGuid,
		NULL,
		&handle_cnt,
		&handle_list);

	for (n = 0; n < handle_cnt; ++n) {
		//
		// Get PXE DHCP protocol interface
		//

		efi_status = BS->HandleProtocol(
			handle_list[n],
			&gEfiPxeDhcp4ProtocolGuid,
			&PxeDhcp4);

		if (EFI_ERROR(efi_status)) {
			continue;
		}

		//
		// Install callback.
		//

		efi_status = BS->InstallProtocolInterface(
			&handle_list[n],
			&gEfiPxeDhcp4CallbackProtocolGuid, 
			EFI_NATIVE_INTERFACE,
			&Callback);

		//
		// Perform DHCPv4 D.O.R.A.
		// Discover/Offer/Request/Acknowledge
		//

		efi_status = PxeDhcp4->Run(PxeDhcp4, op_len, op_list);

		if (EFI_ERROR(efi_status)) {
			Print(L"PxeDhcp4Run() %r\n", efi_status);
		}

		//
		// Renew.
		//

		efi_status = PxeDhcp4->Renew(PxeDhcp4, 4);

		if (EFI_ERROR(efi_status)) {
			Print(L"PxeDhcp4Renew() %r\n", efi_status);
		}

		//
		// Rebind.
		//

		efi_status = PxeDhcp4->Rebind(PxeDhcp4, 4);

		if (EFI_ERROR(efi_status)) {
			Print(L"PxeDhcp4Rebind() %r\n", efi_status);
		}

		//
		// Clean up after DHCPv4 operation.
		//

		efi_status = PxeDhcp4->Release(PxeDhcp4);

		if (EFI_ERROR(efi_status)) {
			Print(L"PxeDhcp4Release() %r\n", efi_status);
		}

		//
		// Remove callback.
		//

		efi_status = BS->UninstallProtocolInterface(
			handle_list[n],
			&gEfiPxeDhcp4CallbackProtocolGuid,
			&Callback);
	}

	BmPause();
}
#endif /* EXTRA_DHCP */

static BOOLEAN BootManagerInitialized = FALSE;

//
// Internal prototypes
//

EFI_STATUS
InitializeBootManager (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

//
//
//

EFI_DRIVER_ENTRY_POINT(InitializeBootManager)

EFI_STATUS
InitializeBootManager (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
/*++

Routine Description:

    The entry point to the boot manager application

Arguments:

    ImageHandle     - The handle for this application

    SystemTable     - The system table

Returns:

    EFI file system driver is enabled

--*/
{

    EFI_STATUS  Status;
    BOOLEAN     Maintenance;

    //
    // Initialize EFI library
    //

    InitializeLib (ImageHandle, SystemTable);

    //
    // Disable the watchdog timer
    //

    BS->SetWatchdogTimer ( 0x0000, 0x0000, 0x0000, NULL );

    BmImageHandle = ImageHandle;

    //
    // Find the protocol that support booting the legacy way
    //
    LibLocateProtocol (&LegacyBootProtocol, &BmBootLegacy);

    //
    // Read all the data variable's and settings
    //
    BmReadVariables();

    //
    // Initialize language support
    //
    BmInitLangCodes();

    //
    // Reset lib in case there was no language code before
    //
    InitializeLib (ImageHandle, SystemTable);

    //
    // Initialize display functions
    //
    BmInitDisplay();

    if (BootManagerInitialized == FALSE) {
      //
      // Load the drivers
      //
      BmLoadDrivers ();

      BootManagerInitialized = TRUE;
    }

#ifdef EXTRA_DHCP
	extra_dhcp();
#endif /* EXTRA_DHCP */

    //
    // Boot the BootNext option if it is present.
    //
    BmBootNextOption();

    //
    // If there are no boot list, then prompt for what to do
    //
    do {
        Maintenance = FALSE;
        Status = BmBootMenu(&Maintenance);
    } while (EFI_ERROR(Status) || !Maintenance);

    //
    // The boot manager exits to run the Boot Manitenance code
    //
    return EFI_SUCCESS;
}
