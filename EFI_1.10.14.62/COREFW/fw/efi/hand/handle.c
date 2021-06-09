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

    handle.c

Abstract:

    EFI handle & protocol handling



Revision History

--*/

#include "hand.h"

//
// ProtocolDatabase - A list of all protocols in the system.  (simple list for now)
// HandleQueue - A list of all the handles in the system
// ProtocolDatabaseLock - Lock to protect the ProtocolDatabase
//

STATIC LIST_ENTRY   ProtocolDatabase;
INTERNAL LIST_ENTRY HandleList;
INTERNAL FLOCK      ProtocolDatabaseLock;
UINT64              HandleDatabaseKey = 0;

VOID
InitializeHandle(
    VOID
    )
/*++

Routine Description:

    Initialize handle & protocol support

Arguments:
    
    None

Returns:

    None

--*/
{
    InitializeListHead (&ProtocolDatabase);
    InitializeListHead (&HandleList);
    InitializeLock (&ProtocolDatabaseLock, TPL_NOTIFY);
}

EFI_STATUS
ValidateHandle (
  IN  EFI_HANDLE                UserHandle
  )

{
  IHANDLE             *Handle;

  Handle = UserHandle;
  if (Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}

INTERNAL
PROTOCOL_ENTRY  *
FindProtocolEntry(
    IN EFI_GUID     *Protocol,
    IN BOOLEAN      Create
    )
/*++

Routine Description:

    Finds the protocl entry for the requested protocol.
    
    N.B.  The ProtocolDatabaseLock must be owned

Arguments:
    
    Protocol    - The ID of the protocol 

    Create      - Create a new entry if not found

Returns:

    Protocol entry

--*/
{
    LIST_ENTRY              *Link;
    PROTOCOL_ENTRY          *Item, *ProtEntry;


    ASSERT_LOCKED(&ProtocolDatabaseLock);

    //
    // Search the database for the matching GUID
    //

    ProtEntry = NULL;
    for (Link=ProtocolDatabase.Flink; 
         Link != &ProtocolDatabase; 
         Link=Link->Flink) {

        Item = CR(Link, PROTOCOL_ENTRY, AllEntries, PROTOCOL_ENTRY_SIGNATURE);
        if (CompareGuid (&Item->ProtocolID, Protocol) == 0) {

            //
            // This is the protocol entry
            //

            ProtEntry = Item;
            break;
        }
    }

    //
    // If the protocol entry was not found and Create is TRUE, then 
    // allocate a new entry
    //
         
    if (!ProtEntry && Create) {
        ProtEntry = (PROTOCOL_ENTRY *) AllocatePool (sizeof(PROTOCOL_ENTRY));
        if (ProtEntry) {

            //
            // Initialize new protocol entry structure
            //

            ProtEntry->Signature = PROTOCOL_ENTRY_SIGNATURE;
            ProtEntry->ProtocolID = *Protocol;
            InitializeListHead (&ProtEntry->Protocols);
            InitializeListHead (&ProtEntry->Notify);

            //
            // Add it to protocol database
            //

            InsertTailList (&ProtocolDatabase, &ProtEntry->AllEntries);
        }
    }

    return ProtEntry;
}

PROTOCOL_INTERFACE *
FindProtocolInterface (
    IN IHANDLE              *Handle,
    IN EFI_GUID             *Protocol,
    IN VOID                 *Interface
    )
{
    PROTOCOL_INTERFACE      *Prot;
    PROTOCOL_ENTRY          *ProtEntry;
    LIST_ENTRY              *Link;

    ASSERT_LOCKED(&ProtocolDatabaseLock);
    Prot = NULL;

    //
    // Lookup the protocol entry for this protocol ID
    //

    ProtEntry = FindProtocolEntry (Protocol, FALSE);
    if (ProtEntry) {

        //
        // Look at each protocol interface for any matches
        //

        for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link=Link->Flink) {

            //
            // If this protocol interface matches, remove it
            //

            Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
            if (Prot->Interface == Interface && Prot->Protocol == ProtEntry) {
                break;
            }

            Prot = NULL;
        }
    }

    return Prot;
}

STATIC
EFI_STATUS
UnregisterProtocolNotifyEvent (
    IN EFI_EVENT            Event
    )
/*++

Routine Description:

    Removes an event from a register protocol notify list on a protocol.

Arguments:
    
    Event       - The event to search for in the protocol database.

Returns:

    EFI_SUCCESS if the event was found and removed.
    EFI_NOT_FOUND if the event was not found in the protocl database.

--*/
{
    LIST_ENTRY         *Link;
    PROTOCOL_ENTRY     *ProtEntry;
    LIST_ENTRY         *NotifyLink;
    PROTOCOL_NOTIFY    *ProtNotify;

    AcquireLock (&ProtocolDatabaseLock);

    for (Link =  ProtocolDatabase.Flink; 
         Link != &ProtocolDatabase; 
         Link =  Link->Flink) {

        ProtEntry = CR(Link, PROTOCOL_ENTRY, AllEntries, PROTOCOL_ENTRY_SIGNATURE);

        for (NotifyLink =  ProtEntry->Notify.Flink; 
             NotifyLink != &ProtEntry->Notify; 
             NotifyLink =  NotifyLink->Flink) {

            ProtNotify = CR(NotifyLink, PROTOCOL_NOTIFY, Link, PROTOCOL_NOTIFY_SIGNATURE);

            if (ProtNotify->Event == Event) {
                RemoveEntryList(&ProtNotify->Link);
                FreePool(ProtNotify);
                ReleaseLock (&ProtocolDatabaseLock);
                return EFI_SUCCESS;
            }
        }
    }

    ReleaseLock (&ProtocolDatabaseLock);

    return EFI_NOT_FOUND;
}

EFI_STATUS
UnregisterProtocolNotify (
    IN EFI_EVENT            Event
    )
/*++

Routine Description:

    Removes all the events in the protocol database that match Event.

Arguments:
    
    Event       - The event to search for in the protocol database.

Returns:

    EFI_SUCCESS when done searching the entire database.

--*/
{
    EFI_STATUS         Status;

    do {
        Status = UnregisterProtocolNotifyEvent(Event);
    } while (!EFI_ERROR(Status));
    return EFI_SUCCESS;
}

EFI_STATUS
BOOTSERVICE
InstallProtocolInterface (
    IN OUT EFI_HANDLE           *UserHandle,
    IN EFI_GUID                 *Protocol,
    IN EFI_INTERFACE_TYPE       InterfaceType,
    IN VOID                     *Interface
    )
/*++

Routine Description:

    Installs a protocol interface into the boot services environment

Arguments:

    Handle          - The handle to install the protocol handler on,
                      or NULL if a new handle is to be allocated

    Protocol        - The protocol to add to the handle

    Interface       - The interface for the protocol being added

Returns:

    Status code    

--*/
{
    PROTOCOL_INTERFACE      *Prot;
    PROTOCOL_ENTRY          *ProtEntry;
    IHANDLE                 *Handle;
    EFI_STATUS              Status;
    VOID                    *ExistingInterface;

    Status = EFI_OUT_OF_RESOURCES;
    Prot = NULL;
    Handle = NULL;

    //
    // Validate parameters
    //
    if (UserHandle == NULL || Protocol == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Only native style interfaces are supported
    //
    if (InterfaceType != EFI_NATIVE_INTERFACE) {
        return EFI_INVALID_PARAMETER;
    }

    if (*UserHandle != NULL) {
        Status = BS->HandleProtocol(*UserHandle, Protocol, (VOID **)&ExistingInterface);

        if (!EFI_ERROR (Status)) {
          return EFI_INVALID_PARAMETER;
        }
    }

    //
    // Lock the protocol database 
    //

    AcquireLock (&ProtocolDatabaseLock);

    //
    // Lookup the Protocol Entry for the requested protocol
    //

    ProtEntry = FindProtocolEntry (Protocol, TRUE);
    if (!ProtEntry) {
        goto Done;
    }

    //
    // Allocate a new protocol interface structure
    //

    Prot = AllocateZeroPool(sizeof(PROTOCOL_INTERFACE));
    if (!Prot) {
        goto Done;
    }

    //
    // If caller didn't supply a handle, allocate a new one
    //

    Handle = *UserHandle;
    if (!Handle) {
        Handle = (IHANDLE *) AllocateZeroPool(sizeof(IHANDLE));
        if (!Handle) {
            goto Done;
        }

        //
        // Initialize new handler structure
        //

        Handle->Signature = EFI_HANDLE_SIGNATURE;
        InitializeListHead (&Handle->Protocols);

        //
        // Initialize the Key to show that the handle has been created/modified
        //
        HandleDatabaseKey++;
        Handle->Key = HandleDatabaseKey;

        //
        // Add this handle to the list global list of all handles
        // in the system
        //

        InsertTailList (&HandleList, &Handle->AllHandles);
    } 
      
    if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    //
    // Each interface that is added must be unique
    //

    ASSERT (FindProtocolInterface (Handle, Protocol, Interface) == NULL);

    //
    // Initialize the protocol interface structure
    //

    Prot->Signature = PROTOCOL_INTERFACE_SIGNATURE;
    Prot->Handle = Handle;
    Prot->Protocol = ProtEntry;
    Prot->Interface = Interface;

    //
    // Initalize OpenProtocol Data base
    //
    InitializeListHead (&Prot->OpenList);
    Prot->OpenListCount = 0;

    //
    // Add this protocol interface to the head of the supported 
    // protocol list for this handle
    //

    InsertHeadList (&Handle->Protocols, &Prot->Link);

    //
    // Add this protocol interface to the tail of the 
    // protocol entry
    // 

    InsertTailList (&ProtEntry->Protocols, &Prot->ByProtocol);

    //
    // Notify the notification list for this protocol 
    //

    NotifyProtocolEntry (ProtEntry);
    Status = EFI_SUCCESS;

Done:
    //
    // Done, unlock the database and return
    //

    ReleaseLock (&ProtocolDatabaseLock);
    if (!EFI_ERROR(Status)) {

        //
        // Return the new handle back to the caller
        //

        *UserHandle = Handle;

    } else {
        
        //
        // There was an error, clean up
        //

        if (Prot) {
            FreePool (Prot);
        }
    }

    return Status;
}

EFI_STATUS
BOOTSERVICE
InstallMultipleProtocolInterfaces (
  IN OUT EFI_HANDLE           *Handle,
  ...
  )
/*++

Routine Description:

  Installs a list of protocol interface into the boot services environment.
  This function calls InstallProtocolInterface() in a loop. If any error
  occures all the protocols added by this function are removed. This is 
  basically a lib function to save space.

Arguments:

  Handle      - The handle to install the protocol handlers on,
                or NULL if a new handle is to be allocated


  ...         - EFI_GUID followed by protocol instance. A NULL terminates the 
                list. The pairs are the arguments to InstallProtocolInterface().
                All the protocols are added to Handle.

Returns:

  Status code    

--*/
{
  va_list         args;
  EFI_STATUS      Status;
  EFI_GUID        *Protocol;
  VOID            *Interface;
  EFI_TPL         OldTpl;
  UINTN           Index;
  EFI_HANDLE                OldHandle;
  EFI_DEVICE_PATH           *DevicePath;
  EFI_HANDLE                DeviceHandle;

  //
  // Syncronize with notifcations. 
  // 
  OldTpl = BS->RaiseTPL (TPL_NOTIFY);
  OldHandle = *Handle;

  //
  // Check for duplicate device path 
  //
  va_start (args, Handle);
  for (Index = 0, Status = EFI_SUCCESS; !EFI_ERROR(Status); Index++) {
    //
    // If protocol is NULL, then it's the end of the list
    //
    Protocol = va_arg(args, EFI_GUID *);
    if (Protocol == NULL) {
      break;
    }

    Interface = va_arg(args, VOID *);
    //
    // Install it
    //
    if (CompareGuid (Protocol, &DevicePathProtocol) == 0) {
      //
      // Make sure you are installing on top a device path that has already
      // been added.
      //
      DeviceHandle = NULL;
      DevicePath   = Interface;
      Status = LocateDevicePath (&DevicePathProtocol, &DevicePath, &DeviceHandle);
      if (!EFI_ERROR(Status) && DeviceHandle != NULL && IsDevicePathEnd(DevicePath)) {
        BS->RestoreTPL(OldTpl);
        return EFI_ALREADY_STARTED;
      }
    }
  }

  //
  // Install the protocol interfaces
  //
  va_start (args, Handle);
  for (Index = 0, Status = EFI_SUCCESS; !EFI_ERROR(Status); Index++) {
   //
    // If protocol is NULL, then it's the end of the list
    //
    Protocol = va_arg(args, EFI_GUID *);
    if (Protocol == NULL) {
      break;
    }

    Interface = va_arg(args, VOID *);
    //
    // Install it
    //
    DEBUG((D_INFO, "InstallProtocolInterface: %d %x\n", Protocol, Interface));
    Status = InstallProtocolInterface (Handle, Protocol, EFI_NATIVE_INTERFACE, Interface);
    if (EFI_ERROR(Status)) {
      break;
    }
  }

  //
  // If there was an error, remove all the interfaces that were
  // installed without any errors
  //
  if (EFI_ERROR(Status)) {
    //
    // Reset the va_arg back to the first argument.
    //
    va_start (args, Handle);
    while (Index) {
      Protocol = va_arg(args, EFI_GUID *);
      Interface = va_arg(args, VOID *);
      UninstallProtocolInterface (*Handle, Protocol, Interface);
      Index -= 1;
    }        
    *Handle = OldHandle;
  }

  //
  // Done
  //
  BS->RestoreTPL(OldTpl);
  return Status;
}

EFI_STATUS
DisconnectControllersUsingProtocolInterface (
  IN EFI_HANDLE       UserHandle,
  PROTOCOL_INTERFACE  *Prot
  )
/*++

Routine Description:

  Uninstalls all instances of a protocol:interfacer from a handle. 
  If the last protocol interface is remove from the handle, the 
  handle is freed.

Arguments:

  Handle      - The handle to remove the protocol handler from

  ProtocolHandle  - The protocol, of protocol:interface, to remove

  Interface   - The interface, of protocol:interface, to remove

Returns:

  Status code    

--*/
{
  EFI_STATUS            Status;
  BOOLEAN               ItemFound;
  LIST_ENTRY            *Link;
  OPEN_PROTOCOL_DATA    *OpenData;

  //
  // Attempt to disconnect all drivers from this protocol interface
  //
  do {
    ItemFound = FALSE;
    for ( Link = Prot->OpenList.Flink;
          (Link != &Prot->OpenList) & !ItemFound;
          Link = Link->Flink
          ) {
      OpenData = CR (Link, OPEN_PROTOCOL_DATA, Link, OPEN_PROTOCOL_DATA_SIGNATURE);
      if (OpenData->Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) {
        ItemFound = TRUE;
        ReleaseLock (&ProtocolDatabaseLock);
//        Status = BS->DisconnectController (OpenData->ControllerHandle, OpenData->AgentHandle, NULL);
        Status = BS->DisconnectController (UserHandle, OpenData->AgentHandle, NULL);
        AcquireLock (&ProtocolDatabaseLock);
        if (EFI_ERROR (Status)) {
          return EFI_ACCESS_DENIED;
        }
      }
    }
  } while (ItemFound == TRUE);

  //
  // Attempt to remove BY_HANDLE_PROTOOCL and GET_PROTOCOL and TEST_PROTOCOL Open List items
  //
  do {
    ItemFound = FALSE;
    for ( Link = Prot->OpenList.Flink;
          (Link != &Prot->OpenList) & !ItemFound;
          Link = Link->Flink
          ) {
      OpenData = CR (Link, OPEN_PROTOCOL_DATA, Link, OPEN_PROTOCOL_DATA_SIGNATURE);
      if (OpenData->Attributes & (EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL | EFI_OPEN_PROTOCOL_GET_PROTOCOL | EFI_OPEN_PROTOCOL_TEST_PROTOCOL)) {
        ItemFound = TRUE;
        RemoveEntryList (&OpenData->Link);  
        Prot->OpenListCount--;
        BS->FreePool (OpenData);
      }
    }
  } while (ItemFound == TRUE);

  //
  // If there are still Open Items in the list, then reconnect all the drivers and return an error
  //
  if (Prot->OpenListCount > 0) {
    ReleaseLock (&ProtocolDatabaseLock);
    Status = BS->ConnectController (UserHandle, NULL, NULL, TRUE);
    AcquireLock (&ProtocolDatabaseLock);
    return EFI_ACCESS_DENIED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
BOOTSERVICE
UninstallProtocolInterface (
  IN EFI_HANDLE       UserHandle,
  IN EFI_GUID         *Protocol,
  IN VOID             *Interface
  )
/*++

Routine Description:

  Uninstalls all instances of a protocol:interfacer from a handle. 
  If the last protocol interface is remove from the handle, the 
  handle is freed.

Arguments:

  Handle      - The handle to remove the protocol handler from

  ProtocolHandle  - The protocol, of protocol:interface, to remove

  Interface   - The interface, of protocol:interface, to remove

Returns:

  Status code    

--*/
{
  EFI_STATUS            Status;
  IHANDLE               *Handle;
  PROTOCOL_INTERFACE    *Prot;

  //
  // Check that Protocol is valid
  //
  if (Protocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check that UserHandle is a valid handle
  //
  Status = ValidateHandle (UserHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Lock the protocol database
  //
  AcquireLock (&ProtocolDatabaseLock);

  //
  // Check that Protocol exists on UserHandle, and Interface matches the interface in the database
  //
  Prot = FindProtocolInterface (UserHandle, Protocol, Interface);
  if (Prot == NULL) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // Attempt to disconnect all drivers that are using the protocol interface that is about to be removed
  //
  Status = DisconnectControllersUsingProtocolInterface (
             UserHandle,
             Prot
             );
  if (EFI_ERROR (Status)) {
    //
    // One or more drivers refused to release, so return the error
    //
    goto Done;
  }

  //
  // Remove the protocol interface from the protocol
  //
  Status = EFI_NOT_FOUND;
  Handle = UserHandle;
  Prot   = RemoveInterfaceFromProtocol (Handle, Protocol, Interface);

  //
  // Update the Key to show that the handle has been created/modified
  //
  HandleDatabaseKey++;
  Handle->Key = HandleDatabaseKey;

  if (Prot != NULL) {
    //
    // Remove the protocol interface from the handle
    //
    RemoveEntryList (&Prot->Link);

    //
    // Free the memory
    //
    Prot->Signature = 0;
    BS->FreePool (Prot);
    Status = EFI_SUCCESS;
  }

  //
  // If there are no more handlers for the handle, free the handle
  //
  if (IsListEmpty (&Handle->Protocols)) {
    Handle->Signature = 0;
    RemoveEntryList (&Handle->AllHandles);
    BS->FreePool (Handle);
  }

Done:  
  //
  // Done, unlock the database and return
  //
  ReleaseLock (&ProtocolDatabaseLock);
  return Status;
}

EFI_STATUS
BOOTSERVICE
UninstallMultipleProtocolInterfaces (
  IN EFI_HANDLE           Handle,
  ...
  )
/*++

Routine Description:

  Uninstalls a list of protocol interface in the boot services environment. 
  This function calls UnisatllProtocolInterface() in a loop. This is 
  basically a lib function to save space.

Arguments:

  Handle      - The handle to uninstall the protocol

  ...         - EFI_GUID followed by protocol instance. A NULL terminates the 
                list. The pairs are the arguments to UninstallProtocolInterface().
                All the protocols are added to Handle.

Returns:

  Status code    

--*/
{
  EFI_STATUS      Status;
  va_list         args;
  EFI_GUID        *Protocol;
  VOID            *Interface;
  UINTN           Index;

  //
  // Uninstall the protocol interfaces
  //
  va_start (args, Handle);
  for (Index = 0, Status = EFI_SUCCESS; !EFI_ERROR(Status); Index++) {
   //
    // If protocol is NULL, then it's the end of the list
    //
    Protocol = va_arg(args, EFI_GUID *);
    if (Protocol == NULL) {
      break;
    }

    Interface = va_arg(args, VOID *);
    //
    // Uninstall it
    //
    DEBUG((D_INFO, "UninstallProtocolInterface: %d %x\n", Protocol, Interface));
    Status = UninstallProtocolInterface (Handle, Protocol, Interface);
    if (EFI_ERROR(Status)) {
      break;
    }
  }

  //
  // If there was an error, add all the interfaces that were
  // uninstalled without any errors
  //
  if (EFI_ERROR(Status)) {
    //
    // Reset the va_arg back to the first argument.
    //
    va_start (args, Handle);
    while (Index) {
      Protocol = va_arg(args, EFI_GUID *);
      Interface = va_arg(args, VOID *);
      InstallProtocolInterface (&Handle, Protocol, EFI_NATIVE_INTERFACE, Interface);
      Index -= 1;
    }        
  }

  return Status;
}    

PROTOCOL_INTERFACE  *
GetProtocolEntry (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol
  )
{
  PROTOCOL_ENTRY      *ProtEntry;
  PROTOCOL_INTERFACE  *Prot;
  IHANDLE             *Handle;
  LIST_ENTRY      *Link;

  Handle = UserHandle;
  if (Handle == NULL) {
    return NULL;
  }
  if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
    return NULL;
  }

  //
  // Look at each protocol interface for a match
  //
  for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link = Link->Flink) {
    Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
    ProtEntry = Prot->Protocol;
    if (CompareGuid (&ProtEntry->ProtocolID, Protocol) == 0) {
      return Prot;
    }
  }
  return NULL;
}

EFI_STATUS
BOOTSERVICE
HandleProtocol (
  IN EFI_HANDLE       UserHandle,
  IN EFI_GUID         *Protocol,
  OUT VOID            **Interface
  )
{
  return OpenProtocol (
          UserHandle,     
          Protocol, 
          Interface, 
          NULL, //gMyImageHandle, 
          NULL,     
          EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
          );
}

EFI_STATUS
BOOTSERVICE
OpenProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  OUT VOID                      **Interface,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle,
  IN  UINT32                    Attributes
  )
/*++

Routine Description:

  Locates the installed protocol handler for the handle, and
  invokes it to obtain the protocol interface. Usage information
  is registered in the protocol data base.

Arguments:

  UserHandle    - The handle to obtain the protocol interface on

  Protocol  - The ID of the protocol 

  Interface - The location to return the protocol interface

  ImageHandle    -
  
  NotifyFunction -

  Attributes     -

Returns:

  The requested protocol interface for the handle
  
--*/
{
  EFI_STATUS          Status;
  PROTOCOL_INTERFACE  *Prot;
  LIST_ENTRY      *Link;
  OPEN_PROTOCOL_DATA  *OpenData;
  BOOLEAN             ByDriver;
  BOOLEAN             Exclusive;
  BOOLEAN             Disconnect;
  BOOLEAN             ExactMatch;

  //
  // Check for invalid Protocol
  //
  if (Protocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check for invalid Interface
  //
  if (Attributes != EFI_OPEN_PROTOCOL_TEST_PROTOCOL && Interface == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check for invalid UserHandle
  //
  Status = ValidateHandle (UserHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Check for invalid Attributes
  //
  switch (Attributes) {
  case EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER :
    Status = ValidateHandle (ImageHandle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = ValidateHandle (ControllerHandle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (UserHandle == ControllerHandle) {
      return EFI_INVALID_PARAMETER;
    }
    break;
  case EFI_OPEN_PROTOCOL_BY_DRIVER :
  case EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE :
    Status = ValidateHandle (ImageHandle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = ValidateHandle (ControllerHandle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    break;
  case EFI_OPEN_PROTOCOL_EXCLUSIVE :
    Status = ValidateHandle (ImageHandle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    break;
  case EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL :
  case EFI_OPEN_PROTOCOL_GET_PROTOCOL :
  case EFI_OPEN_PROTOCOL_TEST_PROTOCOL :
    break;
  default:
    return EFI_INVALID_PARAMETER;
  }

  //
  // Lock the protocol database
  //
  AcquireLock (&ProtocolDatabaseLock);

  //
  // Look at each protocol interface for a match
  //
  Prot = GetProtocolEntry (UserHandle, Protocol);
  if (Prot == NULL) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  //
  // This is the protocol interface entry for this protocol
  //    
  if (Attributes != EFI_OPEN_PROTOCOL_TEST_PROTOCOL) {
    *Interface = Prot->Interface;
  }
  Status = EFI_SUCCESS;

  ByDriver        = FALSE;
  Exclusive       = FALSE;
  for ( Link = Prot->OpenList.Flink; Link != &Prot->OpenList; Link = Link->Flink) {
    OpenData = CR (Link, OPEN_PROTOCOL_DATA, Link, OPEN_PROTOCOL_DATA_SIGNATURE);
    ExactMatch =  (BOOLEAN)(OpenData->AgentHandle == ImageHandle && 
                            OpenData->Attributes == Attributes  &&
                            OpenData->ControllerHandle == ControllerHandle);
    if (OpenData->Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) {
      ByDriver = TRUE;
      if (ExactMatch) {
        Status = EFI_ALREADY_STARTED;
        goto Done;
      }
    }
    if (OpenData->Attributes & EFI_OPEN_PROTOCOL_EXCLUSIVE) {
      Exclusive = TRUE;
    } else if (ExactMatch) {
      OpenData->OpenCount++;
      Status = EFI_SUCCESS;
      goto Done;
    }
  }

  //
  // ByDriver  TRUE  -> A driver is managing (UserHandle, Protocol)
  // ByDriver  FALSE -> There are no drivers managing (UserHandle, Protocol)
  // Exclusive TRUE  -> Something has exclusive access to (UserHandle, Protocol)
  // Exclusive FALSE -> Nothing has exclusive access to (UserHandle, Protocol)
  //

  switch (Attributes) {
  case EFI_OPEN_PROTOCOL_BY_DRIVER :
    if (Exclusive || ByDriver) {
      Status = EFI_ACCESS_DENIED;
      goto Done;
    }
    break;
  case EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE :
  case EFI_OPEN_PROTOCOL_EXCLUSIVE :
    if (Exclusive) {
      Status = EFI_ACCESS_DENIED;
      goto Done;
    }
    if (ByDriver) {
      do {
        Disconnect = FALSE;
        for ( Link = Prot->OpenList.Flink; Link != &Prot->OpenList && !Disconnect; Link = Link->Flink) {
          OpenData = CR (Link, OPEN_PROTOCOL_DATA, Link, OPEN_PROTOCOL_DATA_SIGNATURE);
          if (OpenData->Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) {
            Disconnect = TRUE;
            ReleaseLock (&ProtocolDatabaseLock);
            Status = BS->DisconnectController (UserHandle, OpenData->AgentHandle, NULL);
            AcquireLock (&ProtocolDatabaseLock);
            if (EFI_ERROR (Status)) {
              Status = EFI_ACCESS_DENIED;
              goto Done;
            }
          }
        }
      } while (Disconnect);
    } 
    break;
  case EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER :
  case EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL :
  case EFI_OPEN_PROTOCOL_GET_PROTOCOL :
  case EFI_OPEN_PROTOCOL_TEST_PROTOCOL :
    break;
  } 

  if(ImageHandle != NULL){
    //
    // Create new entry
    //
    Status = BS->AllocatePool (EfiBootServicesData, sizeof(OPEN_PROTOCOL_DATA), &OpenData);
    if (!EFI_ERROR (Status)) {
      OpenData->Signature         = OPEN_PROTOCOL_DATA_SIGNATURE;
      OpenData->AgentHandle       = ImageHandle;
      OpenData->ControllerHandle  = ControllerHandle;
      OpenData->Attributes        = Attributes;
      OpenData->OpenCount         = 1;
      InsertTailList (&Prot->OpenList, &OpenData->Link);
      Prot->OpenListCount++;
      Status = EFI_SUCCESS;
    }
  }

Done:
  //
  // Done. Release the database lock are return
  //
  ReleaseLock (&ProtocolDatabaseLock);
  return Status;
}

EFI_STATUS
BOOTSERVICE
CloseProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle  OPTIONAL
  )
/*++

Routine Description:

  Close Protocol

Arguments:

  UserHandle       - The handle to close the protocol interface on

  Protocol         - The ID of the protocol 

  ImageHandle      - The user of the protocol to close

  ControllerHandle - The user of the protocol to close

Returns:

  
--*/
{
  EFI_STATUS          Status;
  PROTOCOL_INTERFACE  *ProtocolInterface;
  LIST_ENTRY      *Link;
  OPEN_PROTOCOL_DATA  *OpenData;
  BOOLEAN             Removed;

  //
  // Check for invalid parameters
  //
  Status = ValidateHandle (UserHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = ValidateHandle (ImageHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (ControllerHandle != NULL) {
    Status = ValidateHandle (ControllerHandle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  if (Protocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Lock the protocol database
  //
  AcquireLock (&ProtocolDatabaseLock);

  //
  // Look at each protocol interface for a match
  //
  Status = EFI_NOT_FOUND;
  ProtocolInterface = GetProtocolEntry (UserHandle, Protocol);
  if (ProtocolInterface == NULL) {
    goto Done;
  }

  //
  // Walk the Open data base looking for ImageHandle
  //
  do {
    Removed = FALSE;
    for ( Link = ProtocolInterface->OpenList.Flink; 
          (Link != &ProtocolInterface->OpenList) && Removed == FALSE;
          Link = Link->Flink
          ) {
      OpenData = CR (Link, OPEN_PROTOCOL_DATA, Link, OPEN_PROTOCOL_DATA_SIGNATURE);
    
      if (OpenData->AgentHandle == ImageHandle && OpenData->ControllerHandle == ControllerHandle) {
        RemoveEntryList (&OpenData->Link);  
        ProtocolInterface->OpenListCount--;
        BS->FreePool (OpenData);
        Removed = TRUE;
        Status = EFI_SUCCESS;
      }
    }
  } while (Removed == TRUE);

Done:
  //
  // Done. Release the database lock are return
  //
  ReleaseLock (&ProtocolDatabaseLock);
  return Status;
}

EFI_STATUS
BOOTSERVICE
OpenProtocolInformation (
  IN  EFI_HANDLE                          UserHandle,
  IN  EFI_GUID                            *Protocol,
  IN  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer,
  OUT UINTN                               *EntryCount
  )
/*++

Routine Description:

  Return information about Opened protocols in the system

Arguments:

  UserHandle  - The handle to close the protocol interface on

  Protocol    - The ID of the protocol 

  EntryBuffer - 

  EntryCount  - Number of EntryBuffer entries

Returns:

  
--*/
{
  EFI_STATUS                          Status;
  PROTOCOL_INTERFACE                  *ProtocolInterface;
  LIST_ENTRY                          *Link;
  OPEN_PROTOCOL_DATA                  *OpenData;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *Buffer;
  UINTN                               Count;
  UINTN                               Size;

  *EntryBuffer = NULL;
  *EntryCount = 0;

  //
  // Lock the protocol database
  //
  AcquireLock (&ProtocolDatabaseLock);

  //
  // Look at each protocol interface for a match
  //
  Status = EFI_NOT_FOUND;
  ProtocolInterface = GetProtocolEntry (UserHandle, Protocol);
  if (ProtocolInterface == NULL) {
    goto Done;
  }

  //
  // Count the number of Open Entries
  //
  for ( Link = ProtocolInterface->OpenList.Flink, Count = 0; 
        (Link != &ProtocolInterface->OpenList) ;
        Link = Link->Flink  ) {
    Count++;
  } 

  ASSERT (Count == ProtocolInterface->OpenListCount);

  if (Count == 0) {
    Size = sizeof(EFI_OPEN_PROTOCOL_INFORMATION_ENTRY);
  } else {
    Size = Count * sizeof(EFI_OPEN_PROTOCOL_INFORMATION_ENTRY);
  }

  Status = BS->AllocatePool (EfiBootServicesData, Size, &Buffer);
  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  
  for ( Link = ProtocolInterface->OpenList.Flink, Count = 0; 
        (Link != &ProtocolInterface->OpenList);
        Link = Link->Flink, Count++  ) {
    OpenData = CR (Link, OPEN_PROTOCOL_DATA, Link, OPEN_PROTOCOL_DATA_SIGNATURE);

    Buffer[Count].AgentHandle      = OpenData->AgentHandle;
    Buffer[Count].ControllerHandle = OpenData->ControllerHandle;
    Buffer[Count].Attributes       = OpenData->Attributes;
    Buffer[Count].OpenCount        = OpenData->OpenCount;
  } 

  *EntryBuffer = Buffer;
  *EntryCount = Count;
        
Done:
  //
  // Done. Release the database lock are return
  //
  ReleaseLock (&ProtocolDatabaseLock);
  return Status;
}

EFI_STATUS
BOOTSERVICE
ProtocolsPerHandle (
  IN EFI_HANDLE       UserHandle,
  OUT EFI_GUID        ***ProtocolBuffer,
  OUT UINTN           *ProtocolBufferCount
  )
{
  EFI_STATUS                          Status;
  IHANDLE                             *Handle;
  PROTOCOL_INTERFACE                  *Prot;
  LIST_ENTRY                          *Link;
  UINTN                               ProtocolCount;
  EFI_GUID                            **Buffer;

  Handle = UserHandle;
  if (Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  if (ProtocolBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ProtocolBufferCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *ProtocolBufferCount = 0;

  ProtocolCount = 0;
  for (Link = Handle->Protocols.Flink; Link != &Handle->Protocols; Link = Link->Flink) {
    Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
    ProtocolCount++;
  }

  //
  // If there are no protocol interfaces installed on Handle, then Handle is not a valid EFI_HANDLE
  //
  if (ProtocolCount == 0) {
    return EFI_INVALID_PARAMETER;
  }

  Status = BS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GUID *) * ProtocolCount,
                  (VOID **)&Buffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *ProtocolBuffer = Buffer;
  *ProtocolBufferCount = ProtocolCount;

  for ( Link = Handle->Protocols.Flink, ProtocolCount = 0;
        Link != &Handle->Protocols; 
        Link = Link->Flink, ProtocolCount++) {
    Prot = CR(Link, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
    Buffer[ProtocolCount] = &(Prot->Protocol->ProtocolID);
  }

  return Status;
}

UINT64
CoreGetHandleDatabaseKey (
  )

{
  return HandleDatabaseKey;
}


VOID
CoreConnectHandlesByKey (
  UINT64  Key
  )

{
  EFI_STATUS      Status;
  UINTN           Count;
  LIST_ENTRY      *Link;
  EFI_HANDLE      *HandleBuffer;
  IHANDLE         *Handle;
  UINTN           Index;

  //
  // Lock the protocol database
  //
  AcquireLock (&ProtocolDatabaseLock);

  for (Link = HandleList.Flink, Count = 0; Link != &HandleList; Link = Link->Flink) {
    Handle = CR (Link, IHANDLE, AllHandles, EFI_HANDLE_SIGNATURE);
    if (Handle->Key > Key) {
      Count++;
    }
  }

  ///////////////////////////////////////////////////////////////////////
  //Append following code to avoid allocate zero size memory 2003-08-07
  if(Count == 0){
	  ReleaseLock(&ProtocolDatabaseLock);
	  return;
  }
  ///////////////////////////////////////////////////////////////////////

  Status = BS->AllocatePool (EfiBootServicesData, Count * sizeof (EFI_HANDLE), (VOID **)&HandleBuffer);
  if (EFI_ERROR (Status)) {
    //Append following line to unlock the resource before exit 2003-08-07
    ReleaseLock(&ProtocolDatabaseLock);
    return;
  }

  for (Link = HandleList.Flink, Count = 0; Link != &HandleList; Link = Link->Flink) {
    Handle = CR (Link, IHANDLE, AllHandles, EFI_HANDLE_SIGNATURE);
    if (Handle->Key > Key) {
      HandleBuffer[Count++] = Handle;
    }
  }

  //
  // Unlock the protocol database
  //
  ReleaseLock (&ProtocolDatabaseLock);

  //
  // Connect all handles whose Key value is greater than Key
  //
  for (Index = 0; Index < Count; Index++) {
    ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
  }

  FreePool (HandleBuffer);
}