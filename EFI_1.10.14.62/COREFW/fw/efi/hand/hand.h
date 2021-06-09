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

    protocol.h

Abstract:

    EFI internal protocol definitions



Revision History

--*/


#include "efifw.h"


//
// EFI_HANDLE - contains a list of protocol handles
//

#define EFI_HANDLE_SIGNATURE            EFI_SIGNATURE_32('h','n','d','l')
typedef struct _EFI_HANDLE {
    INTN                Signature;
    LIST_ENTRY          AllHandles;         // All handles in the system
    LIST_ENTRY          Protocols;          // The protocol interfaces for this handle
    UINTN               LocateRequest;       
    UINT64              Key;                // The handle database key value when this handle was last created or modified
} IHANDLE;

#define ASSERT_IS_HANDLE(a)  ASSERT((a)->Signature == EFI_HANDLE_SIGNATURE);

//
// PROTOCOL_INTERFACE - each protocol installed on a handle is tracked
// with a protocol interface structure
//

#define PROTOCOL_INTERFACE_SIGNATURE  EFI_SIGNATURE_32('p','i','f','c')
typedef struct _PROTOCOL_INTERFACE {
    INTN                        Signature;
    EFI_HANDLE                  Handle;         // Back pointer
    LIST_ENTRY                  Link;           // Link on IHANDLE.Protocols
    LIST_ENTRY                  ByProtocol;     // Link on PROTOCOL_ENTRY.Protocols
    struct _PROTOCOL_ENTRY      *Protocol;      // The protocol ID
    VOID                        *Interface;     // The interface value

    LIST_ENTRY                  OpenList;       // OPEN_PROTOCOL_DATA list.
    UINTN                       OpenListCount;  // Just for Debug
    EFI_HANDLE                  ControllerHandle;
} PROTOCOL_INTERFACE;

#define OPEN_PROTOCOL_DATA_SIGNATURE  EFI_SIGNATURE_32('p','o','d','l')

typedef struct {
  UINTN                       Signature;
  LIST_ENTRY                  Link;
  EFI_HANDLE                  AgentHandle;
  EFI_HANDLE                  ControllerHandle;
  UINT32                      Attributes;
  UINT32                      OpenCount;
} OPEN_PROTOCOL_DATA;

//
// PROTOCOL_ENTRY - each different protocol has 1 entry in the protocol 
// database.  Each handler that supports this protocol is listed, along
// with a list of registered notifies.
//

#define PROTOCOL_ENTRY_SIGNATURE        EFI_SIGNATURE_32('p','r','t','e')
typedef struct _PROTOCOL_ENTRY {
    INTN                Signature;
    LIST_ENTRY          AllEntries;             // All entries
    EFI_GUID            ProtocolID;             // ID of the protocol
    LIST_ENTRY          Protocols;              // All protocol interfaces
    LIST_ENTRY          Notify;                 // Registerd notification handlers
} PROTOCOL_ENTRY;

//
// PROTOCOL_NOTIFY - used for each register notification for a protocol
//

#define PROTOCOL_NOTIFY_SIGNATURE       EFI_SIGNATURE_32('p','r','t','n')
typedef struct _PROTOCOL_NOTIFY {
    INTN                Signature;
    PROTOCOL_ENTRY      *Protocol;
    LIST_ENTRY          Link;                   // All notifications for this protocol
    EFI_EVENT           Event;                  // Event to notify
    LIST_ENTRY          *Position;              // Last position notified
} PROTOCOL_NOTIFY;

//
// Internal prototypes
//

INTERNAL
PROTOCOL_ENTRY  *
FindProtocolEntry(
    IN EFI_GUID     *Protocol,
    IN BOOLEAN      Create
    );

VOID
NotifyProtocolEntry (
    IN PROTOCOL_ENTRY       *ProtEntry
    );

PROTOCOL_INTERFACE *
FindProtocolInterface (
    IN IHANDLE              *Handle,
    IN EFI_GUID             *Protocol,
    IN VOID                 *Interface
    );

PROTOCOL_INTERFACE *
RemoveInterfaceFromProtocol (
    IN IHANDLE              *Handle,
    IN EFI_GUID             *Protocol,
    IN VOID                 *Interface
    );

EFI_STATUS
UnregisterProtocolNotify (
    IN EFI_EVENT            Event
    );

EFI_STATUS
DisconnectControllersUsingProtocolInterface (
    IN EFI_HANDLE       UserHandle,
    PROTOCOL_INTERFACE  *Prot
    );

//
// Externs
//

extern INTERNAL FLOCK      ProtocolDatabaseLock;
extern INTERNAL LIST_ENTRY HandleList;
extern UINT64              HandleDatabaseKey;