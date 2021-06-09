/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    DriverSupport.c
    
Abstract:

    EFI Driver Support Protocol

Revision History

--*/

#include "Efi.h"
#include "hand.h"

#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (PlatformDriverOverride)
#include EFI_PROTOCOL_DEFINITION (BusSpecificDriverOverride)

//
// Driver Support Function Prototypes
//
EFI_STATUS 
EFIAPI
ConnectController (
  IN  EFI_HANDLE                ControllerHandle,
  IN  EFI_HANDLE                *DriverImageHandle    OPTIONAL,
  IN  EFI_DEVICE_PATH           *RemainingDevicePath  OPTIONAL,
  IN  BOOLEAN                   Recursive
  );

EFI_STATUS 
ConnectSingleController (
  IN  EFI_HANDLE                ControllerHandle,
  IN  EFI_HANDLE                *DriverImageHandle    OPTIONAL,
  IN  EFI_DEVICE_PATH           *RemainingDevicePath  OPTIONAL
  );

EFI_STATUS 
EFIAPI
DisconnectController (
  IN EFI_HANDLE  ControllerHandle,
  IN EFI_HANDLE  DriverImageHandle,
  IN EFI_HANDLE  ChildHandle        OPTIONAL
  );

//
// Driver Support Functions
//

EFI_STATUS 
EFIAPI
ConnectController (
  IN  EFI_HANDLE                ControllerHandle,
  IN  EFI_HANDLE                *DriverImageHandle    OPTIONAL,
  IN  EFI_DEVICE_PATH           *RemainingDevicePath  OPTIONAL,
  IN  BOOLEAN                   Recursive
  )

{
  EFI_STATUS                           Status;
  EFI_STATUS                           ReturnStatus;
  IHANDLE                              *Handle;
  UINTN                                Index;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY  *OpenInfoBuffer;
  UINTN                                EntryCount;
  PROTOCOL_ENTRY                       *ProtEntry;
  PROTOCOL_INTERFACE                   *Prot;
  LIST_ENTRY                           *Link;
  EFI_DEVICE_PATH                      *AlignedRemainingDevicePath;

  //
  // Make sure ControllerHandle is valid
  //
  Handle = ControllerHandle;
  if (Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Connect all drivers to ControllerHandle 
  //
  AlignedRemainingDevicePath = NULL;
  if (RemainingDevicePath != NULL) {
    AlignedRemainingDevicePath = DuplicateDevicePath (RemainingDevicePath);
  }
  ReturnStatus = ConnectSingleController (
                   ControllerHandle,
                   DriverImageHandle,
                   AlignedRemainingDevicePath
                   );
  if (AlignedRemainingDevicePath != NULL) {
    BS->FreePool (AlignedRemainingDevicePath);
  }

  //
  // If not recursive, then just return after connecting drivers to ControllerHandle
  //
  if (!Recursive) {
    return ReturnStatus;
  }

  //
  // If recursive, then connect all drivers to all of ControllerHandle's children
  //
  for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link = Link->Flink) {
    Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
    ProtEntry = Prot->Protocol;
    Status = OpenProtocolInformation (
               ControllerHandle,
               &ProtEntry->ProtocolID,
               &OpenInfoBuffer,
               &EntryCount
               );
    if (!EFI_ERROR (Status) && OpenInfoBuffer != NULL) {
      for (Index = 0; Index < EntryCount; Index++) {
        if (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
          Status = ConnectController (
                     OpenInfoBuffer[Index].ControllerHandle,
                     NULL,
                     NULL,
                     TRUE
                     );
        }
      }
      BS->FreePool (OpenInfoBuffer);
    }
  }

  return ReturnStatus;
}

VOID
AddSortedDriverBindingProtocol (
  EFI_HANDLE                   DriverBindingHandle,
  UINTN                        *NumberOfSortedDriverBindingProtocols, 
  EFI_DRIVER_BINDING_PROTOCOL  **SortedDriverBindingProtocols,
  UINTN                        DriverBindingHandleCount,
  EFI_HANDLE                   *DriverBindingHandleBuffer
  )

{
  EFI_STATUS                   Status;
  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding;
  UINTN                        Index;

  //
  // Make sure the DriverBindingHandle is valid
  //
  if (DriverBindingHandle == NULL) {
    return;
  }

  //
  // Retrieve the Driver Binding Protocol from DriverBindingHandle
  //
  Status = BS->HandleProtocol(
                 DriverBindingHandle,
                 &gEfiDriverBindingProtocolGuid,
                 &DriverBinding
                 );
  //
  // If DriverBindingHandle does not support the Driver Binding Protocol then return
  //
  if (EFI_ERROR (Status) || DriverBinding == NULL) {
    return;
  }

  //
  // See if DriverBinding is already in the sorted list
  //
  for (Index = 0; Index < *NumberOfSortedDriverBindingProtocols; Index++) {
    if (DriverBinding == SortedDriverBindingProtocols[Index]) {
      return;
    }
  }

  //
  // Add DriverBinding to the end of the list
  //
  SortedDriverBindingProtocols[*NumberOfSortedDriverBindingProtocols] = DriverBinding;
  *NumberOfSortedDriverBindingProtocols = *NumberOfSortedDriverBindingProtocols + 1;

  //
  // Mark the cooresponding handle in DriverBindingHandleBuffer as used
  //
  for (Index = 0; Index < DriverBindingHandleCount; Index++) {
    if (DriverBindingHandleBuffer[Index] == DriverBindingHandle) {
      DriverBindingHandleBuffer[Index] = NULL;
    }
  }
}

EFI_STATUS 
ConnectSingleController (
  IN  EFI_HANDLE                ControllerHandle,
  IN  EFI_HANDLE                *ContextDriverImageHandles OPTIONAL,
  IN  EFI_DEVICE_PATH           *RemainingDevicePath       OPTIONAL     
  )
/*++

Routine Description:

    Connects a controller to a driver

Arguments:

    ControllerHandle    -

    DriverImageHandle   -

    DriverImagePath     -

    RemainingDevicePath - 
    
Returns:

    None

--*/
{
  EFI_STATUS                                 Status;
  UINTN                                      Index;
  EFI_HANDLE                                 DriverImageHandle;
  UINTN                                      PlatformDriverOverrideHandleCount;
  EFI_HANDLE                                 *PlatformDriverOverrideHandleBuffer;
  EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL      *PlatformDriverOverride;
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL  *BusSpecificDriverOverride;
  UINTN                                      DriverBindingHandleCount;
  EFI_HANDLE                                 *DriverBindingHandleBuffer;
  EFI_DRIVER_BINDING_PROTOCOL                *DriverBinding;
  UINTN                                      NumberOfSortedDriverBindingProtocols;
  EFI_DRIVER_BINDING_PROTOCOL                **SortedDriverBindingProtocols;
  UINT32                                     HighestVersion;
  UINTN                                      HighestIndex;
  UINTN                                      SortIndex;
  BOOLEAN                                    OneStarted;
  BOOLEAN                                    DriverFound;

  //
  // Initialize local variables
  //
  DriverBindingHandleCount              = 0;
  DriverBindingHandleBuffer             = NULL;
  PlatformDriverOverrideHandleCount     = 0;
  PlatformDriverOverrideHandleBuffer    = NULL;
  NumberOfSortedDriverBindingProtocols  = 0;
  SortedDriverBindingProtocols          = NULL;

  //
  // Get list of all Driver Binding Protocol Instances
  //
  Status = LibLocateHandle (
              ByProtocol,   
              &DriverBindingProtocol,  
              NULL,
              &DriverBindingHandleCount, 
              &DriverBindingHandleBuffer
              );
  if (EFI_ERROR (Status) || DriverBindingHandleCount == 0) {
    return EFI_NOT_FOUND;
  }

  //
  // Allocate a duplicate array for the sorted Driver Binding Protocol Instances
  //
  Status = BS->AllocatePool(
                  EfiBootServicesData,
                  sizeof (VOID *) * DriverBindingHandleCount,
                  (VOID **)&SortedDriverBindingProtocols
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Add Driver Binding Protocols from Context Driver Image Handles first
  //
  if (ContextDriverImageHandles != NULL) {
    for (Index = 0; ContextDriverImageHandles[Index] != NULL; Index++) {
      AddSortedDriverBindingProtocol (
        ContextDriverImageHandles[Index],
        &NumberOfSortedDriverBindingProtocols, 
        SortedDriverBindingProtocols,
        DriverBindingHandleCount,
        DriverBindingHandleBuffer
        );
    }
  }

  //
  // Add the Platform Driver Override Protocol drivers for ControllerHandle next
  //

  Status = BS->LocateProtocol (
             &gEfiPlatformDriverOverrideProtocolGuid, 
             NULL, 
             &PlatformDriverOverride
             );
  if (!EFI_ERROR (Status) && PlatformDriverOverride != NULL) {
    DriverImageHandle = NULL;
    do {
      Status = PlatformDriverOverride->GetDriver (
                                         PlatformDriverOverride,
                                         ControllerHandle,
                                         &DriverImageHandle
                                         );
      if (!EFI_ERROR (Status)) {
        AddSortedDriverBindingProtocol (
          DriverImageHandle,
          &NumberOfSortedDriverBindingProtocols, 
          SortedDriverBindingProtocols,
          DriverBindingHandleCount,
          DriverBindingHandleBuffer
          );
      }
    } while (!EFI_ERROR (Status));
  }

  //
  // Get the Bus Specific Driver Override Protocol instance on the Controller Handle
  //
  Status = BS->HandleProtocol(
                 ControllerHandle,  
                 &gEfiBusSpecificDriverOverrideProtocolGuid, 
                 &BusSpecificDriverOverride
                 );
  if (!EFI_ERROR (Status) && BusSpecificDriverOverride != NULL) {
    DriverImageHandle = NULL;
    do {
      Status = BusSpecificDriverOverride->GetDriver (
                                            BusSpecificDriverOverride,
                                            &DriverImageHandle
                                            );
      if (!EFI_ERROR (Status)) {
        AddSortedDriverBindingProtocol (
          DriverImageHandle,
          &NumberOfSortedDriverBindingProtocols, 
          SortedDriverBindingProtocols,
          DriverBindingHandleCount,
          DriverBindingHandleBuffer
          );
      }
    } while (!EFI_ERROR (Status));
  }

  //
  // Then add all the remaining Driver Binding Protocols
  //
  SortIndex = NumberOfSortedDriverBindingProtocols;
  for (Index = 0; Index < DriverBindingHandleCount; Index++) {
    AddSortedDriverBindingProtocol (
      DriverBindingHandleBuffer[Index],
      &NumberOfSortedDriverBindingProtocols, 
      SortedDriverBindingProtocols,
      DriverBindingHandleCount,
      DriverBindingHandleBuffer
      );
  }

  //
  // Free the Driver Binding Handle Buffer
  //
  BS->FreePool(DriverBindingHandleBuffer);

  //
  // Sort the remaining DriverBinding Protocol based on their Version field from
  // highest to lowest.
  //
  for ( ; SortIndex < DriverBindingHandleCount; SortIndex++) {
    HighestVersion = SortedDriverBindingProtocols[SortIndex]->Version;
    HighestIndex   = SortIndex;
    for (Index = SortIndex + 1; Index < DriverBindingHandleCount; Index++) {
      if (SortedDriverBindingProtocols[Index]->Version > HighestVersion) {
        HighestVersion = SortedDriverBindingProtocols[Index]->Version;
        HighestIndex   = Index;
      }
    }
    if (SortIndex != HighestIndex) {
      DriverBinding = SortedDriverBindingProtocols[SortIndex];
      SortedDriverBindingProtocols[SortIndex] = SortedDriverBindingProtocols[HighestIndex];
      SortedDriverBindingProtocols[HighestIndex] = DriverBinding;
    }
  }

  //
  // Loop until no more drivers can be started on ControllerHandle
  //
  OneStarted = FALSE;
  do {

    //
    // Loop through the sorted Driver Binding Protocol Instances in order, and see if
    // any of the Driver Binding Protocols support the controller specified by 
    // ControllerHandle.
    //
    DriverBinding = NULL;
    DriverFound = FALSE;
    for (Index = 0; Index < NumberOfSortedDriverBindingProtocols; Index++) {
      if (SortedDriverBindingProtocols[Index] != NULL) {
        DriverBinding = SortedDriverBindingProtocols[Index];
        Status = DriverBinding->Supported(
                                  DriverBinding, 
                                  ControllerHandle,
                                  (EFI_DEVICE_PATH_PROTOCOL *)RemainingDevicePath
                                  );
        if (!EFI_ERROR (Status)) {
          SortedDriverBindingProtocols[Index] = NULL;
          DriverFound = TRUE;

          //
          // A driver was found that supports ControllerHandle, so attempt to start the driver
          // on ControllerHandle.
          //
          Status = DriverBinding->Start (
                                    DriverBinding, 
                                    ControllerHandle,
                                    (EFI_DEVICE_PATH_PROTOCOL *)RemainingDevicePath
                                    );
          if (!EFI_ERROR (Status)) {

            //
            // The driver was successfully started on ControllerHandle, so set a flag
            //
            OneStarted = TRUE;
          }
          break;
        }
      }
    }
  } while (DriverFound);

  //
  // Free any buffer that were allocated with AllocatePool()
  //
  BS->FreePool (SortedDriverBindingProtocols);

  //
  // If at least one driver was started on ControllerHandle, then return EFI_SUCCESS.
  //
  if (OneStarted) {
    return EFI_SUCCESS;
  } 

  //
  // If no drivers started and RemainingDevicePath is an End Device Path Node, then return EFI_SUCCESS
  //
  if (RemainingDevicePath != NULL) {
    if (IsDevicePathEnd (RemainingDevicePath)) {
      return EFI_SUCCESS;
    }
  } 

  //
  // Otherwise, no drivers were started on ControllerHandle, so return EFI_NOT_FOUND
  //
  return EFI_NOT_FOUND;
}

EFI_STATUS 
DisconnectController (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_HANDLE  DriverImageHandle, OPTIONAL
  IN  EFI_HANDLE  ChildHandle        OPTIONAL
  )
/*++

Routine Description:

    Disonnects a controller from a driver

Arguments:

    ControllerHandle    -

    GuidList            - 

Returns:

    None

--*/
{
  EFI_STATUS                          Status;
  EFI_DRIVER_BINDING_PROTOCOL         *DriverBinding;
  IHANDLE                             *Handle;
  PROTOCOL_ENTRY                      *ProtEntry;
  PROTOCOL_INTERFACE                  *Prot;
  LIST_ENTRY                          *Link;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfoBuffer;
  UINTN                               EntryCount;
  UINTN                               ChildBufferCount;
  EFI_HANDLE                          *ChildBuffer;
  UINTN                               Index;
  UINTN                               j;
  UINTN                               k;
  BOOLEAN                             Duplicate;
  BOOLEAN                             ChildHandleValid;
  BOOLEAN                             DriverImageHandleValid;
  UINTN                               ChildrenToStop;
  EFI_HANDLE                          *DriverImageHandleBuffer;
  UINTN                               DriverImageHandleCount;
  UINTN                               StopCount;

  //
  // Make sure ControllerHandle is valid
  //
  Handle = ControllerHandle;
  if (Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Make sure ChildHandle is valid if it is not NULL
  //
  if (ChildHandle != NULL) {
    Handle= ChildHandle;
    if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
      return EFI_INVALID_PARAMETER;
    }
  }

  Handle = ControllerHandle;

  //
  // Get list of drivers that are currently managing ControllerHandle
  //
  DriverImageHandleBuffer = NULL;
  DriverImageHandleCount  = 1;
  if (DriverImageHandle == NULL) {
    //
    // Look at each protocol interface for a match
    //
    DriverImageHandleCount = 0;
    for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link = Link->Flink) {
      Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
      ProtEntry = Prot->Protocol;
      Status = OpenProtocolInformation (
                 ControllerHandle,
                 &ProtEntry->ProtocolID,
                 &OpenInfoBuffer,
                 &EntryCount
                 );
      if (!EFI_ERROR (Status) && OpenInfoBuffer != NULL) {
        for (Index = 0; Index < EntryCount; Index++) {
          if (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) {
            DriverImageHandleCount++;
          }
        }
        BS->FreePool (OpenInfoBuffer);
      }
    }

    //
    // If there are no drivers managing this controller, then return EFI_SUCCESS
    //
    if (DriverImageHandleCount == 0) {
      return EFI_SUCCESS;
    }

    Status = BS->AllocatePool (
                    EfiBootServicesData,
                    sizeof (EFI_HANDLE) * DriverImageHandleCount,
                    (VOID **)&DriverImageHandleBuffer
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    DriverImageHandleCount = 0;
    for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link = Link->Flink) {
      Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
      ProtEntry = Prot->Protocol;
      Status = OpenProtocolInformation (
                 ControllerHandle,
                 &ProtEntry->ProtocolID,
                 &OpenInfoBuffer,
                 &EntryCount
                 );
      if (!EFI_ERROR (Status) && OpenInfoBuffer != NULL) {
        for (Index = 0; Index < EntryCount; Index++) {
          if (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) {
            Duplicate = FALSE;
            for (j = 0; j< DriverImageHandleCount; j++) {
              if (DriverImageHandleBuffer[j] == OpenInfoBuffer[Index].AgentHandle) {
                Duplicate = TRUE;
              }
            }
            if (Duplicate == FALSE) {
              DriverImageHandleBuffer[DriverImageHandleCount] = OpenInfoBuffer[Index].AgentHandle;
              DriverImageHandleCount++;
            }
          }
        }
        BS->FreePool (OpenInfoBuffer);
      }
    }
  }

  StopCount = 0;
  for (j = 0; j < DriverImageHandleCount; j++) {

    if (DriverImageHandleBuffer != NULL) {
      DriverImageHandle = DriverImageHandleBuffer[j];
    }

    //
    // Get the Driver Binding Protocol of the driver that is managing this controller
    //
    Status = BS->OpenProtocol (
                    DriverImageHandle,  
                    &DriverBindingProtocol,   
                    &DriverBinding,
                    NULL, //gMyImageHandle,     
                    NULL,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR(Status)) {
      return Status;
    }

    //
    // Look at each protocol interface for a match
    //
    DriverImageHandleValid = FALSE;
    ChildBufferCount = 0;
    for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link = Link->Flink) {
      Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
      ProtEntry = Prot->Protocol;
      Status = OpenProtocolInformation (
                 ControllerHandle,
                 &ProtEntry->ProtocolID,
                 &OpenInfoBuffer,
                 &EntryCount
                 );
      if (!EFI_ERROR (Status) && OpenInfoBuffer != NULL) {
        for (Index = 0; Index < EntryCount; Index++) {
          if (OpenInfoBuffer[Index].AgentHandle == DriverImageHandle &&
              (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER)) {
            ChildBufferCount++;
          }
          if (OpenInfoBuffer[Index].AgentHandle == DriverImageHandle &&
              (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER)) {
            DriverImageHandleValid = TRUE;
          }
        }
        BS->FreePool (OpenInfoBuffer);
      }
    }

    if (DriverImageHandleValid) {
      ChildHandleValid = FALSE;
      ChildBuffer = NULL;
      if (ChildBufferCount != 0) {

        Status = BS->AllocatePool (
                        EfiBootServicesData,
                        sizeof (EFI_HANDLE) * ChildBufferCount,
                        (VOID **)&ChildBuffer
                        );
        if (EFI_ERROR (Status)) {
          if (DriverImageHandleBuffer != NULL) {
            BS->FreePool (DriverImageHandleBuffer);
          }
          return Status;
        }

        ChildBufferCount = 0;
        for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link = Link->Flink) {
          Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
          ProtEntry = Prot->Protocol;
          Status = OpenProtocolInformation (
                     ControllerHandle,
                     &ProtEntry->ProtocolID,
                     &OpenInfoBuffer,
                     &EntryCount
                     );
          if (!EFI_ERROR (Status) && OpenInfoBuffer != NULL) {
            for (Index = 0; Index < EntryCount; Index++) {
              if (OpenInfoBuffer[Index].AgentHandle == DriverImageHandle &&
                  (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER)) {
                Duplicate = FALSE;
                for (k = 0; k < ChildBufferCount; k++) {
                  if (ChildBuffer[k] == OpenInfoBuffer[Index].ControllerHandle) {
                    Duplicate = TRUE;
                    break;
                  }
                }
                if (Duplicate == FALSE) {
                  ChildBuffer[ChildBufferCount] = OpenInfoBuffer[Index].ControllerHandle;
                  if (ChildHandle == ChildBuffer[ChildBufferCount]) {
                    ChildHandleValid = TRUE;
                  }
                  ChildBufferCount++;
                }
              }
            }
            BS->FreePool (OpenInfoBuffer);
          }
        }
      }

      if (ChildHandle == NULL || ChildHandleValid) {
        ChildrenToStop = 0;
        Status = EFI_SUCCESS;
        if (ChildBufferCount > 0) {
          if (ChildHandle != NULL) {
            ChildrenToStop = 1;
            Status = DriverBinding->Stop (DriverBinding, ControllerHandle, ChildrenToStop, &ChildHandle);
          } else {
            ChildrenToStop = ChildBufferCount;
            Status = DriverBinding->Stop (DriverBinding, ControllerHandle, ChildrenToStop, ChildBuffer);
          }
        }
        if (!EFI_ERROR (Status) && ChildHandle == NULL && ChildBufferCount == ChildrenToStop) {
          Status = DriverBinding->Stop (DriverBinding, ControllerHandle, 0, NULL);
        }
        if (!EFI_ERROR(Status)) {
          StopCount++;
        }
      }

      if (ChildBuffer != NULL) {
        BS->FreePool (ChildBuffer);
      }

    }
  }

  if (DriverImageHandleBuffer != NULL) {
    BS->FreePool (DriverImageHandleBuffer);
  }

  if (StopCount > 0) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}
