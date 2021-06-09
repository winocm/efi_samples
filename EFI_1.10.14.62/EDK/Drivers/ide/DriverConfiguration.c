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

  DriverConfiguration.c

Abstract:

--*/

#include "IDEBus.h"

#define EFI_HANDLE_SIGNATURE            EFI_SIGNATURE_32('h','n','d','l')
typedef struct _EFI_HANDLE {
    INTN                Signature;
    EFI_LIST_ENTRY      AllHandles;         // All handles in the system
    EFI_LIST_ENTRY      Protocols;          // The protocol interfaces for this handle
    UINTN               LocateRequest;       
    UINT64              Key;                // The handle database key value when this handle was last created or modified
} IHANDLE;

extern
EFI_STATUS
IsValidateHandle (
    IN  EFI_HANDLE                UserHandle
    );

//
// EFI Driver Configuration Functions
//
EFI_STATUS
IDEBusDriverConfigurationSetOptions (
  IN  EFI_DRIVER_CONFIGURATION_PROTOCOL         *This,
  IN  EFI_HANDLE                                ControllerHandle,
  IN  EFI_HANDLE                                ChildHandle  OPTIONAL,
  IN  CHAR8                                     *Language,
  OUT EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  *ActionRequired
  );

EFI_STATUS
IDEBusDriverConfigurationOptionsValid (
  IN  EFI_DRIVER_CONFIGURATION_PROTOCOL  *This,
  IN  EFI_HANDLE                         ControllerHandle,
  IN  EFI_HANDLE                         ChildHandle  OPTIONAL
  );

EFI_STATUS
IDEBusDriverConfigurationForceDefaults (
  IN  EFI_DRIVER_CONFIGURATION_PROTOCOL         *This,
  IN  EFI_HANDLE                                ControllerHandle,
  IN  EFI_HANDLE                                ChildHandle  OPTIONAL,
  IN  UINT32                                    DefaultType,
  OUT EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  *ActionRequired
  );

//
// EFI Driver Configuration Protocol
//
EFI_DRIVER_CONFIGURATION_PROTOCOL gIDEBusDriverConfiguration = {
  IDEBusDriverConfigurationSetOptions,
  IDEBusDriverConfigurationOptionsValid,
  IDEBusDriverConfigurationForceDefaults,
  "eng"
};

EFI_STATUS
GetResponse (
  VOID
  )

{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;

  while (TRUE) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (!EFI_ERROR (Status)) {
      if (Key.ScanCode == SCAN_ESC) {
        return EFI_ABORTED;
      }
      switch (Key.UnicodeChar) {
      case L'y' : // fall through
      case L'Y' :
        Aprint("Y\n");
        return EFI_SUCCESS;
      case L'n' : // fall through
      case L'N' :
        Aprint("N\n");
        return EFI_NOT_FOUND;
      }
    }
  }
}

EFI_STATUS
IDEBusDriverConfigurationSetOptions (
  IN  EFI_DRIVER_CONFIGURATION_PROTOCOL         *This,
  IN  EFI_HANDLE                                ControllerHandle,
  IN  EFI_HANDLE                                ChildHandle  OPTIONAL,
  IN  CHAR8                                     *Language,
  OUT EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  *ActionRequired
  )
/*++

  Routine Description:
    Allows the user to set controller specific options for a controller that a 
    driver is currently managing.

  Arguments:
    This             - A pointer to the EFI_DRIVER_CONFIGURATION_ PROTOCOL 
                       instance.
    ControllerHandle - The handle of the controller to set options on.
    ChildHandle      - The handle of the child controller to set options on.  
                       This is an optional parameter that may be NULL.  
                       It will be NULL for device drivers, and for a bus drivers
                       that wish to set options for the bus controller.  
                       It will not be NULL for a bus driver that wishes to set 
                       options for one of its child controllers.
    Language         - A pointer to a three character ISO 639-2 language 
                       identifier. This is the language of the user interface 
                       that should be presented to the user, and it must match 
                       one of the languages specified in SupportedLanguages.  
                       The number of languages supported by a driver is up to 
                       the driver writer.
    ActionRequired   - A pointer to the action that the calling agent is 
                       required to perform when this function returns.  
                       See "Related Definitions" for a list of the actions that
                       the calling agent is required to perform prior to 
                       accessing ControllerHandle again.

  Returns:
    EFI_SUCCESS           - The driver specified by This successfully set the 
                            configuration options for the controller specified 
                            by ControllerHandle..
    EFI_INVALID_PARAMETER - ControllerHandle is not a valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ChildHandle is not NULL and it is not a 
                            valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ActionRequired is NULL.
    EFI_UNSUPPORTED       - The driver specified by This does not support 
                            setting configuration options for the controller 
                            specified by ControllerHandle and ChildHandle.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            language specified by Language.
    EFI_DEVICE_ERROR      - A device error occurred while attempt to set the 
                            configuration options for the controller specified 
                            by ControllerHandle and ChildHandle.
    EFI_OUT_RESOURCES     - There are not enough resources available to set the 
                            configuration options for the controller specified 
                            by ControllerHandle and ChildHandle.

--*/
{
  EFI_STATUS  Status;
  EFI_STATUS  PrimaryMasterStatus;
  EFI_STATUS  PrimarySlaveStatus;
  EFI_STATUS  SecondaryMasterStatus;
  EFI_STATUS  SecondarySlaveStatus;
  UINT8       Value;
  UINT8       NewValue;
  UINTN       DataSize;
  UINT32      Attributes;

  Status = IsValidateHandle(ControllerHandle);
  if(EFI_ERROR(Status)) {
    return EFI_INVALID_PARAMETER;
  }

  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }

  if(ActionRequired == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if(EfiAsciiStrnCmp("eng", Language, 3) != 0) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gIDEBusDriverGuid,  
                  NULL,
                  gIDEBusDriverBinding.DriverBindingHandle,             
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  *ActionRequired = EfiDriverConfigurationActionNone;

  DataSize = sizeof (Value);
  Status = gRT->GetVariable (
                  L"Configuration", 
                  &gIDEBusDriverGuid, 
                  &Attributes, 
                  &DataSize, 
                  &Value
                  );

  Aprint("PCI IDE/ATAPI Driver Configuration\n");
  Aprint("==================================\n");
  Aprint("Enable Primary Master   (Y/N)? -->");
  PrimaryMasterStatus = GetResponse();
  if (PrimaryMasterStatus == EFI_ABORTED) {
    return EFI_SUCCESS;
  }

  Aprint("Enable Primary Slave    (Y/N)? -->");
  PrimarySlaveStatus = GetResponse();
  if (PrimarySlaveStatus == EFI_ABORTED) {
    return EFI_SUCCESS;
  }

  Aprint("Enable Secondary Master (Y/N)? -->");
  SecondaryMasterStatus = GetResponse();
  if (SecondaryMasterStatus == EFI_ABORTED) {
    return EFI_SUCCESS;
  }

  Aprint("Enable Secondary Slave  (Y/N)? -->");
  SecondarySlaveStatus = GetResponse();
  if (SecondarySlaveStatus == EFI_ABORTED) {
    return EFI_SUCCESS;
  }

  NewValue = 0;
  if (!EFI_ERROR (PrimaryMasterStatus)) {
    NewValue |= 1;
  }

  if (!EFI_ERROR (PrimarySlaveStatus)) {
    NewValue |= 2;
  }

  if (!EFI_ERROR (SecondaryMasterStatus)) {
    NewValue |= 4;
  }

  if (!EFI_ERROR (SecondarySlaveStatus)) {
    NewValue |= 8;
  }

  if (EFI_ERROR (Status) || (NewValue != Value)) {
    Status = gRT->SetVariable (  
                    L"Configuration", 
                    &gIDEBusDriverGuid,
                    EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    sizeof (NewValue),
                    &NewValue
                    );

    *ActionRequired = EfiDriverConfigurationActionRestartController;
  } else {
    *ActionRequired = EfiDriverConfigurationActionNone;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
IDEBusDriverConfigurationOptionsValid (
  IN  EFI_DRIVER_CONFIGURATION_PROTOCOL  *This,
  IN  EFI_HANDLE                         ControllerHandle,
  IN  EFI_HANDLE                         ChildHandle  OPTIONAL
  )
/*++

  Routine Description:
    Tests to see if a controller's current configuration options are valid.

  Arguments:
    This             - A pointer to the EFI_DRIVER_CONFIGURATION_PROTOCOL 
                       instance.
    ControllerHandle - The handle of the controller to test if it's current 
                       configuration options are valid.
    ChildHandle      - The handle of the child controller to test if it's 
                       current
                       configuration options are valid.  This is an optional 
                       parameter that may be NULL.  It will be NULL for device 
                       drivers.  It will also be NULL for a bus drivers that 
                       wish to test the configuration options for the bus 
                       controller. It will not be NULL for a bus driver that 
                       wishes to test configuration options for one of 
                       its child controllers.

  Returns:
    EFI_SUCCESS           - The controller specified by ControllerHandle and 
                            ChildHandle that is being managed by the driver 
                            specified by This has a valid set of  configuration
                            options.
    EFI_INVALID_PARAMETER - ControllerHandle is not a valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ChildHandle is not NULL and it is not a valid 
                            EFI_HANDLE.
    EFI_UNSUPPORTED       - The driver specified by This is not currently 
                            managing the controller specified by 
                            ControllerHandle and ChildHandle.
    EFI_DEVICE_ERROR      - The controller specified by ControllerHandle and 
                            ChildHandle that is being managed by the driver 
                            specified by This has an invalid set of 
                            configuration options.

--*/
{
  EFI_STATUS  Status;
  UINT8       Value;
  UINTN       DataSize;
  UINT32      Attributes;

  Status = IsValidateHandle(ControllerHandle);
  if(EFI_ERROR(Status)) {
    return EFI_INVALID_PARAMETER;
  }

  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gIDEBusDriverGuid,  
                  NULL,
                  gIDEBusDriverBinding.DriverBindingHandle,             
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  DataSize = sizeof (Value);
  Status = gRT->GetVariable (
                  L"Configuration", 
                  &gIDEBusDriverGuid, 
                  &Attributes, 
                  &DataSize, 
                  &Value
                  );
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }
  if (Value > 0x0f) {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
IDEBusDriverConfigurationForceDefaults (
  IN  EFI_DRIVER_CONFIGURATION_PROTOCOL         *This,
  IN  EFI_HANDLE                                ControllerHandle,
  IN  EFI_HANDLE                                ChildHandle  OPTIONAL,
  IN  UINT32                                    DefaultType,
  OUT EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  *ActionRequired
  )
/*++

  Routine Description:
    Forces a driver to set the default configuration options for a controller.

  Arguments:
    This             - A pointer to the EFI_DRIVER_CONFIGURATION_ PROTOCOL 
                       instance.
    ControllerHandle - The handle of the controller to force default 
                       configuration options on.
    ChildHandle      - The handle of the child controller to force default 
                       configuration options on  This is an optional parameter 
                       that may be NULL.  It will be NULL for device drivers.  
                       It will also be NULL for a bus drivers that wish to 
                       force default configuration options for the bus 
                       controller.  It will not be NULL for a bus driver that 
                       wishes to force default configuration options for one 
                       of its child controllers.
    DefaultType      - The type of default configuration options to force on 
                       the controller specified by ControllerHandle and 
                       ChildHandle.  See Table 9-1 for legal values.  
                       A DefaultType of 0x00000000 must be supported 
                       by this protocol.
    ActionRequired   - A pointer to the action that the calling agent 
                       is required to perform when this function returns.  
                       

  Returns:
    EFI_SUCCESS           - The driver specified by This successfully forced 
                            the default configuration options on the 
                            controller specified by ControllerHandle and 
                            ChildHandle.
    EFI_INVALID_PARAMETER - ControllerHandle is not a valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ChildHandle is not NULL and it is not a 
                            valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ActionRequired is NULL.
    EFI_UNSUPPORTED       - The driver specified by This does not support 
                            forcing the default configuration options on 
                            the controller specified by ControllerHandle 
                            and ChildHandle.
    EFI_UNSUPPORTED       - The driver specified by This does not support 
                            the configuration type specified by DefaultType.
    EFI_DEVICE_ERROR      - A device error occurred while attempt to force 
                            the default configuration options on the controller 
                            specified by  ControllerHandle and ChildHandle.
    EFI_OUT_RESOURCES     - There are not enough resources available to force 
                            the default configuration options on the controller 
                            specified by ControllerHandle and ChildHandle.

--*/
{
  EFI_STATUS  Status;
  UINT8       Value;

  Status = IsValidateHandle(ControllerHandle);
  if(EFI_ERROR(Status)) {
    return EFI_INVALID_PARAMETER;
  }

  if(ActionRequired == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gIDEBusDriverGuid,  
                  NULL,
                  gIDEBusDriverBinding.DriverBindingHandle,             
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Value = 0x0f;
  Status = gRT->SetVariable (  
                  L"Configuration", 
                  &gIDEBusDriverGuid,
                  EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (Value),
                  &Value
                  );
  *ActionRequired = EfiDriverConfigurationActionRestartController;
  return EFI_SUCCESS;
}
