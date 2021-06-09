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

    notify.c

Abstract:

    EFI notify infrastructure



Revision History

--*/

#include "hand.h"

VOID
NotifyProtocolEntry (
    IN PROTOCOL_ENTRY       *ProtEntry
    )
{
    PROTOCOL_NOTIFY         *ProtNotify;
    LIST_ENTRY              *Link;

    ASSERT_LOCKED(&ProtocolDatabaseLock);

    for (Link=ProtEntry->Notify.Flink; Link != &ProtEntry->Notify; Link=Link->Flink) {
        ProtNotify = CR(Link, PROTOCOL_NOTIFY, Link, PROTOCOL_NOTIFY_SIGNATURE);
        SignalEvent (ProtNotify->Event);
    }
}


PROTOCOL_INTERFACE *
RemoveInterfaceFromProtocol (
    IN IHANDLE              *Handle,
    IN EFI_GUID             *Protocol,
    IN VOID                 *Interface
    )
// NB: removes Prot from the protocol list (but not the handle list)
{
    PROTOCOL_INTERFACE      *Prot;
    PROTOCOL_NOTIFY         *ProtNotify;
    PROTOCOL_ENTRY          *ProtEntry;
    LIST_ENTRY              *Link;

    ASSERT_LOCKED(&ProtocolDatabaseLock);


    Prot = FindProtocolInterface (Handle, Protocol, Interface);
    if (Prot) {

        ProtEntry = Prot->Protocol;

        //
        // If there's a protocol notify location pointing to this entry, back it up one
        //

        for(Link = ProtEntry->Notify.Flink; Link != &ProtEntry->Notify; Link=Link->Flink) {
            ProtNotify = CR(Link, PROTOCOL_NOTIFY, Link, PROTOCOL_NOTIFY_SIGNATURE);

            if (ProtNotify->Position == &Prot->ByProtocol) {
                ProtNotify->Position = Prot->ByProtocol.Blink;
            }
        }

        //
        // Remove the protocol interface entry
        //

        RemoveEntryList (&Prot->ByProtocol);
    }

    return Prot;
}



EFI_STATUS
BOOTSERVICE
RegisterProtocolNotify (
    IN EFI_GUID             *Protocol,
    IN EFI_EVENT            Event,
    IN VOID                 **Registration
    )
/*++

Routine Description:

    Add a new protocol notification record for the request protocol

Arguments:

    Protocol        - The requested protocol to add the notify registration too

    Event           - The event to signal 

    Registration    - Returns the registration record


Returns:

    Status code.

    On succes, the registration record that has been added
    
--*/
{
    PROTOCOL_ENTRY          *ProtEntry;
    PROTOCOL_NOTIFY         *ProtNotify;
    EFI_STATUS              Status;

    AcquireLock (&ProtocolDatabaseLock);

    //
    // Get the protocol entry to add the notification too
    //

    ProtEntry = FindProtocolEntry (Protocol, TRUE);
    if (ProtEntry) {

        //
        // Allocate a new notification record
        //

        ProtNotify = (PROTOCOL_NOTIFY *) AllocatePool (sizeof(PROTOCOL_NOTIFY));
        if (ProtNotify) {
            
            ProtNotify->Signature = PROTOCOL_NOTIFY_SIGNATURE;
            ProtNotify->Protocol = ProtEntry;
            ProtNotify->Event = Event;
            ProtNotify->Position = &ProtEntry->Protocols;   // start at the begining

            InsertTailList (&ProtEntry->Notify, &ProtNotify->Link);
        }
    }

    ReleaseLock (&ProtocolDatabaseLock);

    //
    // Done.  If we have a protocol notify entry, then return it.
    // Otherwise, we must have run out of resources trying to add one
    //

    Status = EFI_OUT_OF_RESOURCES;
    if (ProtNotify) {
        *Registration = ProtNotify;
        Status = EFI_SUCCESS;
    }

    return Status;
}

EFI_STATUS
BOOTSERVICE
ReinstallProtocolInterface (
  IN EFI_HANDLE     UserHandle,
  IN EFI_GUID       *Protocol,
  IN VOID           *OldInterface,
  IN VOID           *NewInterface
  )
/*++

Routine Description:

  Reinstall a protocol interface on a device handle.  The OldInterface for Protocol is replaced by the NewInterface.

Arguments:

  UserHandle	  - Handle on which the interface is to be reinstalled
  Protocol      - The numeric ID of the interface
  OldInterface  - A pointer to the old interface
  NewInterface  - A pointer to the new interface 


Returns:

  Status code.

  On EFI_SUCCESS	          The protocol interface was installed
  On EFI_NOT_FOUND	        The OldInterface on the handle was not found
  On EFI_INVALID_PARAMETER	One of the parameters has an invalid value
  
--*/
{
  EFI_STATUS                Status;
  IHANDLE                   *Handle;
  PROTOCOL_INTERFACE        *Prot;
  PROTOCOL_ENTRY            *ProtEntry;

  Status = EFI_NOT_FOUND;
  Handle = (IHANDLE *) UserHandle;

  if (Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  if (Protocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Lock the protocol database
  //
  AcquireLock (&ProtocolDatabaseLock);

  //
  // Check that Protocol exists on UserHandle, and Interface matches the interface in the database
  //
  Prot = FindProtocolInterface (UserHandle, Protocol, OldInterface);
  if (Prot == NULL) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  //
  // Attempt to disconnect all drivers that are using the protocol interface that is about to be reinstalled
  //
  Status = DisconnectControllersUsingProtocolInterface (
             UserHandle,
             Prot
             );
  if (EFI_ERROR (Status)) {
    //
    // One or more drivers refused to release, so return the error
    //

    ReleaseLock (&ProtocolDatabaseLock);
    BS->ConnectController (
          UserHandle,
          NULL, 
          NULL, 
          TRUE
          );
    AcquireLock (&ProtocolDatabaseLock);

    goto Done;
  }

  //
  // Remove the protocol interface from the protocol
  //
  Prot = RemoveInterfaceFromProtocol (Handle, Protocol, OldInterface);

  //
  // Update the key to show that the handle has been created/modified.
  //
  HandleDatabaseKey++;
  Handle->Key = HandleDatabaseKey;

  if (Prot == NULL) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  ProtEntry = Prot->Protocol;

  //
  // Update the interface on the protocol
  //
  Prot->Interface = NewInterface;

  //
  // Add this protocol interface to the tail of the
  // protocol entry
  //
  InsertTailList (&ProtEntry->Protocols, &Prot->ByProtocol);

  //
  // Release the lock and connect all drivers to UserHandle
  //
  ReleaseLock (&ProtocolDatabaseLock);
  Status = BS->ConnectController (
                 UserHandle, 
                 NULL, 
                 NULL, 
                 TRUE
                 );
  AcquireLock (&ProtocolDatabaseLock);
  
  //
  // Notify the notification list for this protocol
  //
  NotifyProtocolEntry (ProtEntry);

  Status = EFI_SUCCESS;

Done:
  ReleaseLock (&ProtocolDatabaseLock);
  return Status;
}
