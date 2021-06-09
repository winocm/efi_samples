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

  WinNtPciRootBridge.c
    
Abstract:

  EFI WinNt Emulated PCI Root Bridge Controller

--*/

#include "WinNtPciRootBridge.h"

EFI_DRIVER_ENTRY_POINT(InitializeWinNtPciRootBridge);

EFI_STATUS
InitializeWinNtPciRootBridge (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
    )
/*++

Routine Description:

    Initializes the PCI Root Bridge Controller

Arguments:

    ImageHandle -

    SystemTable -
    
Returns:

    None

--*/
{
  EFI_STATUS		             Status;
  WIN_NT_PCI_ROOT_BRIDGE_INSTANCE *PrivateData;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(WIN_NT_PCI_ROOT_BRIDGE_INSTANCE),
                  &PrivateData
                  );
  ASSERT(!EFI_ERROR(Status));

  PrivateData->Signature = WIN_NT_PCI_ROOT_BRIDGE_SIGNATURE;

  WinNtRootBridgeDevicePathConstructor (&PrivateData->DevicePath);
  WinNtRootBridgeIoConstructor         (&PrivateData->Io);
    
  PrivateData->Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                   &PrivateData->Handle,           
                   &gEfiDevicePathProtocolGuid,      PrivateData->DevicePath,
                   &gEfiPciRootBridgeIoProtocolGuid, &PrivateData->Io,
                   NULL
                   );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (PrivateData);
  }
  return Status;
}