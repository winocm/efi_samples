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

  EfiDriverLib.c

Abstract:

  Light weight lib to support EFI drivers.

--*/

#include "Efi.h"
#include "EfiDriverLib.h"


//
// Driver Lib Module Globals
//
BOOLEAN       mDriverLibInitialized = FALSE;

EFI_STATUS
EfiInitializeDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Intialize Driver Lib if it has not yet been initialized. 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS always returns EFI_SUCCESS

--*/
{

  if (!mDriverLibInitialized) {
    mDriverLibInitialized = TRUE;

    gST = SystemTable;

    ASSERT (gST != NULL);

    gBS = gST->BootServices;
    gRT = gST->RuntimeServices;

    ASSERT (gBS != NULL);
    ASSERT (gRT != NULL);
  }

  EfiDebugAssertInit ();

  //
  // Should be at EFI_D_INFO, but lets us know things are running
  //
  DEBUG((EFI_D_INFO, "EfiInitializeDriverLib: Started\n"));
  return EFI_SUCCESS;
}

EFI_STATUS
EfiLibInstallDriverBinding (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN EFI_HANDLE                   DriverBindingHandle
  )
/*++

Routine Description:

  Intialize a driver by installing the Driver Binding Protocol onto the 
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

Arguments:

  ImageHandle         - The image handle of the driver

  SystemTable         - The EFI System Table that was passed to the driver's entry point

  DriverBinding       - A Driver Binding Protocol instance that this driver is producing

  DriverBindingHandle - The handle that DriverBinding is to be installe onto.  If this
                        parameter is NULL, then a new handle is created.

Returns: 

  EFI_SUCCESS is DriverBinding is installed onto DriverBindingHandle

  Otherwise, then return status from gBS->InstallProtocolInterface()

--*/
{
  EfiInitializeDriverLib(ImageHandle, SystemTable);

  DriverBinding->ImageHandle = ImageHandle;
  
  DriverBinding->DriverBindingHandle = DriverBindingHandle;

  return gBS->InstallProtocolInterface (
                &DriverBinding->DriverBindingHandle,
                &gEfiDriverBindingProtocolGuid,
                EFI_NATIVE_INTERFACE,
                DriverBinding
                );
}

EFI_STATUS
EfiLibInstallAllDriverProtocols (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL        *DriverBinding,
  IN EFI_HANDLE                         DriverBindingHandle,
  IN EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,        OPTIONAL
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration,  OPTIONAL
  IN EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics     OPTIONAL
  )
/*++

Routine Description:

  Intialize a driver by installing the Driver Binding Protocol onto the 
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

Arguments:

  ImageHandle         - The image handle of the driver

  SystemTable         - The EFI System Table that was passed to the driver's entry point

  DriverBinding       - A Driver Binding Protocol instance that this driver is producing

  DriverBindingHandle - The handle that DriverBinding is to be installe onto.  If this
                        parameter is NULL, then a new handle is created.

  ComponentName       - A Component Name Protocol instance that this driver is producing

  DriverConfiguration - A Driver Configuration Protocol instance that this driver is producing
  
  DriverDiagnostics   - A Driver Diagnostics Protocol instance that this driver is producing

Returns: 

  EFI_SUCCESS if all the protocols were installed onto DriverBindingHandle

  Otherwise, then return status from gBS->InstallProtocolInterface()

--*/
{
  EFI_STATUS  Status;

  Status = EfiLibInstallDriverBinding (ImageHandle, SystemTable, DriverBinding, DriverBindingHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (ComponentName != NULL) {
    Status = gBS->InstallProtocolInterface (
                    &DriverBinding->DriverBindingHandle,
                    &gEfiComponentNameProtocolGuid, 
                    EFI_NATIVE_INTERFACE, 
                    ComponentName
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (DriverConfiguration != NULL) {
    Status = gBS->InstallProtocolInterface (
                    &DriverBinding->DriverBindingHandle,
                    &gEfiDriverConfigurationProtocolGuid, 
                    EFI_NATIVE_INTERFACE, 
                    DriverConfiguration
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (DriverDiagnostics != NULL) {
    Status = gBS->InstallProtocolInterface (
                    &DriverBinding->DriverBindingHandle,
                    &gEfiDriverDiagnosticsProtocolGuid, 
                    EFI_NATIVE_INTERFACE, 
                    DriverDiagnostics
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  return EFI_SUCCESS;
}


EFI_STATUS
EfiLibGetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  IN OUT VOID **Table
  )
/*++

Routine Description:

Arguments:

Returns: 

  None

--*/
{
  UINTN Index;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (EfiCompareGuid (TableGuid, &(gST->ConfigurationTable[Index].VendorGuid))) {
      *Table = gST->ConfigurationTable[Index].VendorTable;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

EFI_STATUS
EfiLibLookupUnicodeString (
  CHAR8                     *Language,
  CHAR8                     *SupportedLanguages,
  EFI_UNICODE_STRING_TABLE  *UnicodeStringTable,
  CHAR16                    **UnicodeString
  )
/*++

Routine Description:

Arguments:

Returns: 

  None

--*/
{
  //
  // Make sure the parameters are valid
  //
  if (Language == NULL || UnicodeString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If there are no supported languages, or the Unicode String Table is empty, then the 
  // Unicode String specified by Language is not supported by this Unicode String Table
  //
  if (SupportedLanguages == NULL || UnicodeStringTable == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Make sure Language is in the set of Supported Languages
  //
  while (*SupportedLanguages != 0) {
    if (EfiCompareMem (Language, SupportedLanguages, 3) == 0) {

      //
      // Search the Unicode String Table for the matching Language specifier
      //
      while (UnicodeStringTable->Language != NULL) {
        if (EfiCompareMem (Language, UnicodeStringTable->Language, 3) == 0) {

          //
          // A matching string was found, so return it
          //
          *UnicodeString = UnicodeStringTable->UnicodeString;
          return EFI_SUCCESS;
        }
        UnicodeStringTable++;
      }
      return EFI_UNSUPPORTED;
    }
    SupportedLanguages += 3;
  }
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EfiLibAddUnicodeString (
  CHAR8                     *Language,
  CHAR8                     *SupportedLanguages,
  EFI_UNICODE_STRING_TABLE  **UnicodeStringTable,
  CHAR16                    *UnicodeString
  )
/*++

Routine Description:

Arguments:

Returns: 

  None

--*/
{
  EFI_STATUS                Status;
  UINTN                     NumberOfEntries;
  EFI_UNICODE_STRING_TABLE  *OldUnicodeStringTable;
  EFI_UNICODE_STRING_TABLE  *NewUnicodeStringTable;
  UINTN                     UnicodeStringLength;

  //
  // Make sure the parameter are valid
  //
  if (Language == NULL || UnicodeString == NULL || UnicodeStringTable == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If there are no supported languages, then a Unicode String can not be added
  //
  if (SupportedLanguages == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // If the Unicode String is empty, then a Unicode String can not be added
  //
  if (UnicodeString[0] == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Make sure Language is a member of SupportedLanguages
  //
  while (*SupportedLanguages != 0) {
    if (EfiCompareMem (Language, SupportedLanguages, 3) == 0) {

      //
      // Determine the size of the Unicode String Table by looking for a NULL Language entry
      //
      NumberOfEntries = 0;
      if (*UnicodeStringTable != NULL) {
        OldUnicodeStringTable = *UnicodeStringTable;
        while (OldUnicodeStringTable->Language != NULL) {
          if (EfiCompareMem (Language, OldUnicodeStringTable->Language, 3) == 0) {
            return EFI_ALREADY_STARTED;
          }
          OldUnicodeStringTable++;
          NumberOfEntries++;
        }
      }

      //
      // Allocate space for a new Unicode String Table.  It must hold the current number of
      // entries, plus 1 entry for the new Unicode String, plus 1 entry for the end of table 
      // marker
      //
      Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      (NumberOfEntries + 2) * sizeof(EFI_UNICODE_STRING_TABLE),
                      (VOID **)&NewUnicodeStringTable
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // If the current Unicode Strinf Table contains anyt entries, then copy them to the
      // newly allocated Unicode String Table.
      //
      if (*UnicodeStringTable != NULL) {
        EfiCopyMem(
          NewUnicodeStringTable, 
          *UnicodeStringTable, 
          NumberOfEntries * sizeof(EFI_UNICODE_STRING_TABLE)
          );
      }

      //
      // Allocate space for a copy of the Language specifier
      //
      Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      3,
                      (VOID **)&NewUnicodeStringTable[NumberOfEntries].Language
                      );
      if (EFI_ERROR (Status)) {
        gBS->FreePool (NewUnicodeStringTable);
        return Status;
      }

      //
      // Copy the Language specifier to the allocated buffer
      //
      EfiCopyMem(
        NewUnicodeStringTable[NumberOfEntries].Language,
        Language,
        3
        );

      //
      // Compute the length of the Unicode String
      //
      for (UnicodeStringLength = 0;UnicodeString[UnicodeStringLength] != 0; UnicodeStringLength++);

      //
      // Allocate space for a copy of the Unicode String
      //
      Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      (UnicodeStringLength + 1) * sizeof(CHAR16),
                      (VOID **)&NewUnicodeStringTable[NumberOfEntries].UnicodeString
                      );
      if (EFI_ERROR (Status)) {
        gBS->FreePool (NewUnicodeStringTable[NumberOfEntries].Language);
        gBS->FreePool (NewUnicodeStringTable);
        return Status;
      }

      //
      // Copy the Unicode String to the allocated buffer
      //
      EfiCopyMem(
        NewUnicodeStringTable[NumberOfEntries].UnicodeString,
        UnicodeString,
        (UnicodeStringLength + 1) * sizeof(CHAR16)
        );

      //
      // Mark the end of the Unicode String Table
      //
      NewUnicodeStringTable[NumberOfEntries + 1].Language      = NULL;
      NewUnicodeStringTable[NumberOfEntries + 1].UnicodeString = NULL;

      //
      // Free the old Unicode String Table
      //
      if (*UnicodeStringTable != NULL) {
        gBS->FreePool(*UnicodeStringTable);
      }

      //
      // Point UnicodeStringTable at the newly allocated Unicode String Table
      //
      *UnicodeStringTable = NewUnicodeStringTable;

      return EFI_SUCCESS;
    }
    SupportedLanguages += 3;
  }
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EfiLibFreeUnicodeStringTable (
  EFI_UNICODE_STRING_TABLE  *UnicodeStringTable
  )
/*++

Routine Description:

Arguments:

Returns: 

  None

--*/
{
  UINTN       Index;

  //
  // If the Unicode String Table is NULL, then it is already freed
  //
  if (UnicodeStringTable == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Loop through the Unicode String Table until we reach the end of table marker
  //
  for (Index = 0; UnicodeStringTable[Index].Language != NULL; Index++) {

    //
    // Free the Language string from the Unicode String Table
    //
    gBS->FreePool(UnicodeStringTable[Index].Language);

    //
    // Free the Unicode String from the Unicode String Table
    //
    if (UnicodeStringTable[Index].UnicodeString != NULL) {
      gBS->FreePool(UnicodeStringTable[Index].UnicodeString);
    }
  }

  //
  // Free the Unicode String Table itself
  //
  gBS->FreePool(UnicodeStringTable);

  return EFI_SUCCESS;
}
