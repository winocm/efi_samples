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

    locate.c

Abstract:

    Locate handle functions    



Revision History

--*/

#include "hand.h"

//
// ProtocolRequest - Last LocateHandle request ID
//

STATIC UINTN EfiLocateHandleRequest;

//
// Internal prototypes
//

typedef struct {
    EFI_GUID        *Protocol;
    VOID            *SearchKey;
    LIST_ENTRY      *Position;
    PROTOCOL_ENTRY  *ProtEntry;
} LOCATE_POSITION;

typedef 
IHANDLE *
(* GET_NEXT) (
    IN OUT LOCATE_POSITION    *Position,
    OUT VOID                  **Interface
    );


STATIC
IHANDLE *
GetNextLocateAllHandles (
    IN OUT LOCATE_POSITION    *Position,
    OUT VOID                  **Interface
    );

STATIC
IHANDLE *
GetNextLocateByRegisterNotify (
    IN OUT LOCATE_POSITION    *Position,
    OUT VOID                  **Interface
    );

STATIC
IHANDLE *
GetNextLocateByProtocol (
    IN OUT LOCATE_POSITION    *Position,
    OUT VOID                  **Interface
    );

//
//
//


EFI_STATUS
BOOTSERVICE
LocateHandle (
    IN EFI_LOCATE_SEARCH_TYPE   SearchType,
    IN EFI_GUID                 *Protocol OPTIONAL,
    IN VOID                     *SearchKey OPTIONAL,
    IN OUT UINTN                *BufferSize,
    OUT EFI_HANDLE              *Buffer
    )
/*++

Routine Description:

    Locates the requested handle(s) and returns them in Buffer

Arguments:

    SearchType      - The type of search to perform to locate the handles

    Protocol        - The protocol to search for
    
    SearchKey       - Dependant on SearchType

    BufferSize      - On input the size of Buffer.  On output the 
                      size of data returned.  

    Buffer          - The buffer to return the results in


Returns:

    Status code.
    On success, BufferSize and Buffer is returned
    On EFI_BUFFER_TOO_SMALL, BufferSize is returned

    On succes, the registration record that has been added
    
--*/
{
    LOCATE_POSITION         Position;
    PROTOCOL_NOTIFY         *ProtNotify;
    GET_NEXT                GetNext;
    UINTN                   ResultSize;
    IHANDLE                 *Handle;
    IHANDLE                 **ResultBuffer;
    EFI_STATUS              Status;
    VOID                    *Interface;

    //
    // Set initial position
    //

    Position.Protocol = Protocol;
    Position.SearchKey = SearchKey;
    Position.Position = &HandleList;

    ResultSize = 0;
    ResultBuffer = (IHANDLE **) Buffer;
    Status = EFI_SUCCESS;

    //
    // Lock the protocol database
    //

    AcquireLock (&ProtocolDatabaseLock);

    //
    // Get the search function based on type
    //

    switch (SearchType) {
    case AllHandles:            
        GetNext = GetNextLocateAllHandles;             
        break;

    case ByRegisterNotify:      
        if(SearchKey == NULL){
            Status =  EFI_INVALID_PARAMETER;
            break;
        }
            
        GetNext = GetNextLocateByRegisterNotify;       
        break;

    case ByProtocol:            
        if(Protocol == NULL){
            Status = EFI_INVALID_PARAMETER;
            break;
        }
        GetNext = GetNextLocateByProtocol;

        //
        // Look up the protocol entry and set the head pointer
        //

        Position.ProtEntry = FindProtocolEntry (Protocol, FALSE);
        if (!Position.ProtEntry) {
            Status = EFI_NOT_FOUND;
            break;
        }

        Position.Position = &Position.ProtEntry->Protocols;
        break;

    default:
        Status = EFI_INVALID_PARAMETER;
        break;
    }

    if (EFI_ERROR(Status)) {
        ReleaseLock (&ProtocolDatabaseLock);
        return Status;
    }

    //
    // Enumerate out the matching handles
    //

    EfiLocateHandleRequest += 1;
    for (; ;) {

        //
        // Get the next handle.  If no more handles, stop
        //
        
        Handle = GetNext(&Position, &Interface);
        if (!Handle) {
            break;
        }

        //
        // Increase the resulting buffer size, and if this handle
        // fits return it
        //

        ResultSize += sizeof(Handle);
        if (ResultSize <= *BufferSize) {
            *ResultBuffer = Handle;
            ResultBuffer += 1;
        }
    }

    //
    // Return the resulting buffer size.  If it's larger then what
    // was passed, then set the error code
    //

    if (ResultSize > *BufferSize) {
        Status = EFI_BUFFER_TOO_SMALL;
    }
    *BufferSize = ResultSize;

    //
    // If the result is a zero length buffer, then there were no
    // matching handles
    //

    if (ResultSize == 0) {
        Status = EFI_NOT_FOUND;
    }

    //
    // If this is a search by register notify and a handle was
    // returned, update the register notification position
    // 

    if (SearchType == ByRegisterNotify && !EFI_ERROR(Status)) {
        ProtNotify = SearchKey;
        ProtNotify->Position = ProtNotify->Position->Flink;
    }

    ReleaseLock (&ProtocolDatabaseLock);
    return Status;
}



STATIC
IHANDLE *
GetNextLocateAllHandles (
    IN OUT LOCATE_POSITION    *Position,
    OUT VOID                  **Interface
    )
{
    IHANDLE         *Handle;

    //
    // Next handle
    //
    Position->Position = Position->Position->Flink;

    //
    // If not at the end of the list, get the handle
    //

    Handle = NULL;
    *Interface  = NULL;
    if (Position->Position != &HandleList) {
        Handle = CR(Position->Position, IHANDLE, AllHandles, EFI_HANDLE_SIGNATURE);
    }

    return Handle;
}

STATIC
IHANDLE *
GetNextLocateByRegisterNotify (
  IN OUT LOCATE_POSITION    *Position,
  OUT VOID                  **Interface
  )
/*++

Routine Description:

  Routine to get the next Handle, when you are searching for register protocol 
  notifies.

Arguments:

  Position  - Information about which Handle to seach for.

  Interface - Return the interface structure for the matching protocol.
  
Returns:
  IHANDLE - An IHANDLE is returned if the next Position is not the end of the
            list. A NULL_HANDLE is returned if it's the end of the list.
  
--*/
{
  IHANDLE             *Handle;
  PROTOCOL_NOTIFY     *ProtNotify;
  PROTOCOL_INTERFACE  *Prot;
  LIST_ENTRY          *Link;    

  Handle      = NULL;
  *Interface  = NULL;
  ProtNotify = Position->SearchKey;

  //
  // If this is the first request, get the next handle
  //
  if (ProtNotify) {
    ASSERT(ProtNotify->Signature == PROTOCOL_NOTIFY_SIGNATURE);
    Position->SearchKey = NULL;

    //
    // If not at the end of the list, get the next handle
    //
    Link = ProtNotify->Position->Flink;
    if (Link != &ProtNotify->Protocol->Protocols) {
      Prot = CR (Link, PROTOCOL_INTERFACE, ByProtocol, PROTOCOL_INTERFACE_SIGNATURE);
      Handle = (IHANDLE *) Prot->Handle;
      *Interface = Prot->Interface;
    }  
  }

  return Handle;
}


STATIC
IHANDLE *
GetNextLocateByProtocol (
  IN OUT LOCATE_POSITION    *Position,
  OUT VOID                  **Interface
  )
/*++

Routine Description:

  Routine to get the next Handle, when you are searching for a given protocol.

Arguments:

  Position  - Information about which Handle to seach for.

  Interface - Return the interface structure for the matching protocol.
  
Returns:
  IHANDLE - An IHANDLE is returned if the next Position is not the end of the
            list. A NULL_HANDLE is returned if it's the end of the list.
  
--*/
{
  IHANDLE             *Handle;
  LIST_ENTRY          *Link;
  PROTOCOL_INTERFACE  *Prot;
 
  Handle      = NULL;
  *Interface  = NULL;
  for (; ;) {
    //
    // Next entry
    //
    Link = Position->Position->Flink;
    Position->Position = Link;

    //
    // If not at the end, return the handle
    //
    if (Link == &Position->ProtEntry->Protocols) {
      Handle = NULL;
      break;
    }

    //
    // Get the handle
    //
    Prot = CR(Link, PROTOCOL_INTERFACE, ByProtocol, PROTOCOL_INTERFACE_SIGNATURE);
    Handle = (IHANDLE *) Prot->Handle;
    *Interface = Prot->Interface;

    //
    // If this handle has not been returned this request, then 
    // return it now
    //
    if (Handle->LocateRequest != EfiLocateHandleRequest) {
      Handle->LocateRequest = EfiLocateHandleRequest;
      break;
    }
  }

  return Handle;
}

EFI_STATUS
BOOTSERVICE
LocateDevicePath (
    IN EFI_GUID             *Protocol,
    IN OUT EFI_DEVICE_PATH  **FilePath,
    OUT EFI_HANDLE          *Device
    )
{
    UINTN                   NoHandles, Index;
    INTN                    BestMatch, SourceSize, Size;
    EFI_HANDLE              *Handles, Handle;
    EFI_DEVICE_PATH         *SourcePath, *DevicePath, *Eop, *Inst;


    // For now just get it working..

    *Device = NULL;
    SourcePath = *FilePath;

    //
    // The source path can only have 1 instance
    //

    if (DevicePathInstanceCount(SourcePath) != 1) {
        DEBUG((D_ERROR, "LocateDevicePath: Device path has too many instances\n"));
        return EFI_INVALID_PARAMETER;
    }

    //
    // Get a list of all handles that support the requested protocol
    //

    LibLocateHandle (ByProtocol, Protocol, NULL, &NoHandles, &Handles);
    SourceSize = DevicePathSize(SourcePath) - sizeof(EFI_DEVICE_PATH);

    BestMatch = -1;
    for(Index = 0; Index < NoHandles; Index += 1) {
        Handle = Handles[Index];
        DevicePath = DevicePathFromHandle (Handle);

        //
        // If this handle doesn't support device path, then skip it
        //

        if (!DevicePath) {
            continue;
        }

        //
        // Check each instance 
        // 

        while (Inst = DevicePathInstance (&DevicePath, &Size)) {

            //
            // Check if DevicePath is first part of SourcePath
            //

            if (Size <= SourceSize &&
                CompareMem (SourcePath, Inst, Size) == 0) {

                Eop = (EFI_DEVICE_PATH *) (((UINT8 *) Inst) + Size);
                if (IsDevicePathEndType(Eop)) {

                    //
                    // If the size is equal to the best match, then we
                    // have a duplice device path for 2 different device
                    // handles
                    //

                    ASSERT (Size != BestMatch);

                    //
                    // We've got a match, see if it's the best match so far
                    //

                    if (Size > BestMatch) {
                        BestMatch = Size;
                        *Device = Handle;
                    }
                }
            }
        }
    }

    if (Handles) {
        FreePool(Handles);
    }
     
    //
    // If there wasn't any match, then no parts of the device path
    // was found.  (which is strange since there is likely a "root level"
    // device path in the system
    //

    if (BestMatch == -1) {
        return EFI_NOT_FOUND;
    }

    //
    // Return the remaining part of the device path
    //

    *FilePath = (EFI_DEVICE_PATH *) (((UINT8 *) SourcePath) + BestMatch);
    return EFI_SUCCESS;
}

EFI_STATUS
BOOTSERVICE
LocateProtocol (
  EFI_GUID  *Protocol,
  VOID      *Registration, OPTIONAL
  VOID      **Interface
  )
/*++

Routine Description:

  Return the first Protocol Interface that matches the Protocol GUID. If
  Registration is pasased in return a Protocol Instance that was just add
  to the system. If Retistration is NULL return the first Protocol Interface
  you find.

Arguments:

  Protocol     - The protocol to search for
  
  Registration - Optional Registration Key returned from RegisterProtocolNotify() 

  Interface    - Return the Protocol interface (instance).

Returns:

  EFI_SUCCESS - If a valid Interface is returned
  other       - Interface not found and NULL is returned.

--*/
{
  EFI_STATUS              Status;
  LOCATE_POSITION         Position;
  PROTOCOL_NOTIFY         *ProtNotify;
  IHANDLE                 *Handle;

  if (Interface == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Interface = NULL;
  Status = EFI_SUCCESS;

  //
  // Set initial position
  //
  Position.Protocol  = Protocol;
  Position.SearchKey = Registration;
  Position.Position  = &HandleList;
  
  //
  // Lock the protocol database
  //
  AcquireLock (&ProtocolDatabaseLock);

  EfiLocateHandleRequest += 1;

  if (NULL == Registration) {
    //
    // Look up the protocol entry and set the head pointer
    //
    Position.ProtEntry = FindProtocolEntry (Protocol, FALSE);
    if (!Position.ProtEntry) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
    Position.Position = &Position.ProtEntry->Protocols;

    Handle = GetNextLocateByProtocol (&Position, Interface);
  } else {
    Handle = GetNextLocateByRegisterNotify (&Position, Interface);   
  }

  if (NULL == Handle) {
    Status = EFI_NOT_FOUND;
  } else if (NULL != Registration) {
    //
    // If this is a search by register notify and a handle was
    // returned, update the register notification position
    // 
    ProtNotify = Registration;
    ProtNotify->Position = ProtNotify->Position->Flink;
  }

Done:
  ReleaseLock (&ProtocolDatabaseLock);
  return Status;
}

EFI_STATUS
BOOTSERVICE
LocateHandleBuffer (
  IN EFI_LOCATE_SEARCH_TYPE       SearchType,
  IN EFI_GUID                     *Protocol OPTIONAL,
  IN VOID                         *SearchKey OPTIONAL,
  IN OUT UINTN                    *NumberHandles,
  OUT EFI_HANDLE                  **Buffer
  )
/*++

Routine Description:

  Function returns an array of handles that support the requested protocol 
  in a buffer allocated from pool. This is a version of LocateHandle()
  that allocates a buffer for the caller.

Arguments:

  SearchType           - Specifies which handle(s) are to be returned.
  Protocol             - Provides the protocol to search by.   
                         This parameter is only valid for SearchType ByProtocol.
  SearchKey            - Supplies the search key depending on the SearchType.
  NoHandles            - The number of handles returned in Buffer.
  Buffer               - A pointer to the buffer to return the requested array of 
                         handles that support Protocol.

Returns:
  
  EFI_SUCCESS	         - The result array of handles was returned.
  EFI_NOT_FOUND	       - No handles match the search. 
  EFI_OUT_OF_RESOURCES - There is not enough pool memory to store the matching results.
  EFI_INVALIDE_PARAMETER - One of the parameters has an invalid value.

--*/
{
  EFI_STATUS          Status;
  UINTN               BufferSize;

  if (NumberHandles == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  BufferSize = 0;
  *NumberHandles = 0;
  *Buffer = NULL;
  Status = LocateHandle (
             SearchType,  Protocol, SearchKey,
             &BufferSize, *Buffer
             );
  //
  //EFI_INVALID_PARAMETER should be returned if one of the parameters is invalid, otherwise return EFI_NOT_FOUND.
  //
  switch(Status){
    case EFI_BUFFER_TOO_SMALL:
        break;
    case EFI_INVALID_PARAMETER:
        return Status;
    default:
        return EFI_NOT_FOUND;
    }

  Status = BS->AllocatePool (EfiBootServicesData, BufferSize, (VOID *)Buffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = LocateHandle (
             SearchType,  Protocol, SearchKey,
             &BufferSize, *Buffer
             );

  *NumberHandles = BufferSize/sizeof(EFI_HANDLE);
  if (EFI_ERROR(Status)) {
    *NumberHandles = 0;
  }

  return Status;
}
