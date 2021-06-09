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

    Connect.c
    
Abstract:   

Revision History

--*/

#include "shelle.h"

EFI_STATUS
SEnvDisconnectAll (
  VOID
  );

VOID
SEnvConnectAllDriversToAllControllers (
  VOID
  );

VOID
SEnvConnectAllConsoles (
  VOID
  );

EFI_STATUS
SEnvCmdConnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:
  Connect DriverHandle [DeviceHandle]

Arguments:

Returns:
  
--*/
{
  EFI_STATUS                      Status;
  BOOLEAN                         Recursive;
  BOOLEAN                         Consoles;
  UINTN                           Index;
  UINTN                           HandleNumber;
  UINTN                           NumberOfHandles;
  EFI_HANDLE                      HandleBuffer[3];
  BOOLEAN                         GetAllDevices;
  UINTN                           AllHandleCount;
  EFI_HANDLE                      *AllHandleBuffer;
  UINTN                           DriverBindingHandleCount;
  EFI_HANDLE                      *DriverBindingHandleBuffer;
  UINTN                           DeviceHandleCount;
  EFI_HANDLE                      *DeviceHandleBuffer;
  VOID                            *Instance;
  UINTN                           Driver;
  UINTN                           Device;
  EFI_HANDLE                      ContextOverride[2];

  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  AllHandleCount = 0;
  AllHandleBuffer = NULL;
  DeviceHandleCount = 0;
  DeviceHandleBuffer = NULL;
  DriverBindingHandleCount = 0;
  DriverBindingHandleBuffer = NULL;
  HandleBuffer[0] = NULL;
  HandleBuffer[1] = NULL;
  HandleBuffer[2] = NULL;

  GetAllDevices = FALSE;

  if ((SI->Argc < 1) || (SI->Argc >= 4)) {
    Print (L"connect: Argument error\n");
    Status = EFI_INVALID_PARAMETER;
    goto ConnectDone;
  }

  Recursive = FALSE;
  Consoles  = FALSE;
  NumberOfHandles = 0;
  for (Index = 1; Index < SI->Argc; Index++) {
    if ((SI->Argv[Index][0] == '-' || SI->Argv[Index][0] == '/') && SI->Argv[Index][1] == '?') {
      goto ConnectDone;
    }
    if (SI->Argv[Index][0] == '-' || SI->Argv[Index][0] == '/') {
      if (SI->Argv[Index][1] == 'r' || SI->Argv[Index][1] == 'R') {
        Recursive = TRUE;
      } else if (SI->Argv[Index][1] == 'c' || SI->Argv[Index][1] == 'C') {
        Consoles = TRUE;
      } else {
        Print (L"connect: Unknown flag '%hc'\n", SI->Argv[Index][1]);
        Status = EFI_INVALID_PARAMETER;
        goto ConnectDone;
      }
    } else {
      HandleNumber = SEnvHandleNoFromStr (SI->Argv[Index]);
      if (HandleNumber == 0) {
        Print (L"connect: Handle not found\n");
        Status = EFI_NOT_FOUND;
        goto ConnectDone;
      } else {
        HandleBuffer[NumberOfHandles++] = SEnvHandles[HandleNumber - 1];
      }
    }
  }

  if (NumberOfHandles > 2) {
    Status = EFI_ABORTED;
    Print (L"connect: Too many handles found\n");
    goto ConnectDone;
  }

  switch (NumberOfHandles) {
  case 0 :
    if (Consoles) {
      SEnvConnectAllConsoles ();
      if (Recursive) {
        SEnvConnectAllDriversToAllControllers();
      }
      SEnvConnectAllConsoles ();
      goto ConnectDone;
    }
    if (Recursive) {
      SEnvConnectAllDriversToAllControllers();
      goto ConnectDone;
    }
    GetAllDevices = TRUE;
    break;
    
  case 1:
    Status = BS->HandleProtocol (HandleBuffer[0], 
                                 &gEfiDriverBindingProtocolGuid, 
                                 &Instance);
    if (!EFI_ERROR(Status)) {
      DriverBindingHandleCount = 1;
      DriverBindingHandleBuffer = &HandleBuffer[0];
      GetAllDevices = TRUE;
    } else {
      DeviceHandleCount = 1;
      DeviceHandleBuffer = &HandleBuffer[0];
    }
    break;

  case 2:
    Status = BS->HandleProtocol (HandleBuffer[0], 
                                 &gEfiDriverBindingProtocolGuid, 
                                 &Instance);
    if (!EFI_ERROR(Status)) {
      goto ConnectDone;
    } 
    DeviceHandleCount = 1;
    DeviceHandleBuffer = &HandleBuffer[0];

    Status = BS->HandleProtocol (HandleBuffer[1], 
                                 &gEfiDriverBindingProtocolGuid, 
                                 &Instance);
    if (EFI_ERROR(Status)) {
      goto ConnectDone;
    } 
    DriverBindingHandleCount = 1;
    DriverBindingHandleBuffer = &HandleBuffer[1];
    break;
  }

  if (GetAllDevices) {
    Status = LibLocateHandle (
              AllHandles,
              NULL,
              NULL,
              &AllHandleCount,
              &AllHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      goto ConnectDone;
    }

    Status = BS->AllocatePool (
                    EfiBootServicesData,
                    AllHandleCount * sizeof(EFI_HANDLE),
                    (VOID **)&DeviceHandleBuffer
                    );
    if (EFI_ERROR (Status)) {
      goto ConnectDone;
    }

    for (Index = 0; Index < AllHandleCount; Index++) {
      Status = BS->HandleProtocol (AllHandleBuffer[Index], 
                                   &gEfiDriverBindingProtocolGuid, 
                                   &Instance);
      if (EFI_ERROR (Status)) {
        DeviceHandleBuffer[DeviceHandleCount++] = AllHandleBuffer[Index];
      }
    }
  }

  for (Device = 0; Device < DeviceHandleCount; Device++) {
    if (DriverBindingHandleCount == 0) {
        Status = BS->ConnectController (
                       DeviceHandleBuffer[Device],
                       NULL,
                       NULL,
                       Recursive
                       );
        if (!EFI_ERROR (Status)) {
          Print (
            L"ConnectController(%x) : ",
            SEnvHandleToNumber (DeviceHandleBuffer[Device])
            );
          Print (L"Status = %r\n", Status);
        }
    } else {
      for (Driver = 0; Driver < DriverBindingHandleCount; Driver++) {
        ContextOverride[0] = DriverBindingHandleBuffer[Driver];
        ContextOverride[1] = NULL;
        Status = BS->ConnectController (
                       DeviceHandleBuffer[Device],
                       ContextOverride,
                       NULL,
                       Recursive
                       );
        if (!EFI_ERROR (Status)) {
          Print (
            L"ConnectController(%x, %x) : ",
            SEnvHandleToNumber (DeviceHandleBuffer[Device]),
            SEnvHandleToNumber (DriverBindingHandleBuffer[Driver])
            );
          Print (L"Status = %r\n", Status);
        }
      }
    }
  }

ConnectDone:
  //
  // Clean up our dynamically allocated data
  //
  if (GetAllDevices) {
    BS->FreePool (AllHandleBuffer);
    BS->FreePool (DeviceHandleBuffer);
  }
  SEnvFreeHandleTable ();

  return Status;
}


EFI_STATUS
SEnvCmdDisconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:
  Disconnect DriverHandle [DeviceHandle]

Arguments:

Returns:
  
--*/
{
  EFI_STATUS    Status;
  EFI_STATUS    ReturnStatus;
  UINTN         HandleNumber;
  EFI_HANDLE    DriverImageHandle;
  EFI_HANDLE    DeviceHandle;
  EFI_HANDLE    ChildHandle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL *TextOut;
  EFI_SIMPLE_TEXT_IN_PROTOCOL *TextIn;
  EFI_HANDLE    *DeviceHandleBuffer;
  UINTN         DeviceHandleCount;
  UINTN         Index;

  InitializeShellApplication (ImageHandle, SystemTable);

  if (SI->Argc < 2 ) {
    Print (L"disconnect: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }
  if (SI->Argc > 4) {
    Print (L"disconnect: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  Status            = EFI_SUCCESS;
  DeviceHandle      = NULL;
  DriverImageHandle = NULL;
  ChildHandle       = NULL;

  //
  // If command line is disconnect all the do that.
  //
  if (SI->Argv[1][0] == '-' || SI->Argv[1][0] == '/') {
    if (SI->Argv[1][1] == 'r' || SI->Argv[1][1] == 'R') {
      SEnvDisconnectAll ();
      ReturnStatus = EFI_SUCCESS;
      goto Done;
    } else {
      Print (L"disconnect: Unknown flag '%hc'\n", SI->Argv[1][1]);
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  DeviceHandle = NULL;
  HandleNumber = SEnvHandleNoFromStr (SI->Argv[1]);
  if (HandleNumber != 0) {
    DeviceHandle = SEnvHandles[HandleNumber - 1];
  }

  DriverImageHandle = NULL;
  if (SI->Argc >= 3) {
    HandleNumber = SEnvHandleNoFromStr (SI->Argv[2]);
    if (HandleNumber != 0) {
      DriverImageHandle = SEnvHandles[HandleNumber - 1];
    }
  }

  if (DeviceHandle == NULL && DriverImageHandle == NULL) {
    Print (L"disconnect: Handle not found\n");
    return EFI_NOT_FOUND;
  }

  ChildHandle = NULL;
  if (SI->Argc >= 4) {
    HandleNumber = SEnvHandleNoFromStr (SI->Argv[3]);
    if (HandleNumber != 0) {
      ChildHandle = SEnvHandles[HandleNumber - 1];
    }
  }

  if (DeviceHandle != NULL) {
    ReturnStatus = BS->DisconnectController (DeviceHandle, 
                                             DriverImageHandle,
                                             ChildHandle);
  } else {
    Status = LibLocateHandle (
               ByProtocol,
               &gEfiDevicePathProtocolGuid,
               NULL,
               &DeviceHandleCount,
               &DeviceHandleBuffer
               );
    if (EFI_ERROR(Status)) {
      Print(L"disconnect: Device Path Protocols not found\n");
      return EFI_UNSUPPORTED;
    }

    ReturnStatus = EFI_NOT_FOUND;
    for (Index = 0; Index < DeviceHandleCount; Index++) {
      Status = BS->DisconnectController (DeviceHandleBuffer[Index], 
                                         DriverImageHandle, 
                                         ChildHandle);
      if (!EFI_ERROR (Status)) {
        ReturnStatus = EFI_SUCCESS;
      }
    }
    if (DeviceHandleCount != 0) {
      BS->FreePool (DeviceHandleBuffer);
    }
  } 

Done:
  Status = BS->HandleProtocol ( SystemTable->ConsoleOutHandle, 
                                &gEfiSimpleTextOutProtocolGuid, 
                                (VOID **)&TextOut
                              );
  if (EFI_ERROR(Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  Status = BS->HandleProtocol ( SystemTable->ConsoleInHandle, 
                                &gEfiSimpleTextInProtocolGuid, 
                                (VOID **)&TextIn);
  if (EFI_ERROR(Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  SEnvFreeHandleTable ();

  Print (L"DisconnectController(%x,%x,%x) : Status = %r\n", 
          DeviceHandle, DriverImageHandle, ChildHandle, Status);

  return ReturnStatus;
}


EFI_STATUS
SEnvDisconnectAll (
  VOID
  )
/*++

Routine Description:
  Disconnect All Handles and Exit the shell. This lets us go back to the BSD

Arguments:

Returns:
  
--*/
{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  Status = LibLocateHandle (
             AllHandles,
             NULL,
             NULL,
             &AllHandleCount,
             &AllHandleBuffer
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < AllHandleCount; Index++) {

    //
    // Scan the handle database
    //
    Status = LibScanHandleDatabase (
               NULL,
               NULL,
               AllHandleBuffer[Index],
               NULL,
               &HandleCount, 
               &HandleBuffer, 
               &HandleType
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Device = TRUE;
    if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }
    if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
      Device = FALSE;
    }

    if (Device) {
      Parent = FALSE;
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
          Parent = TRUE;
        }
      }

      if (!Parent) {
        if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
          Status = BS->DisconnectController (
                         AllHandleBuffer[Index],
                         NULL,
                         NULL
                         );
        }
      }
    }

    BS->FreePool (HandleBuffer);
    BS->FreePool (HandleType);
  }

  BS->FreePool (AllHandleBuffer);
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvConnectDevicePath (
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathToConnect
  )

{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *OriginalDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Next;
  EFI_HANDLE                Handle;
  EFI_HANDLE                PreviousHandle;
  UINTN                     Size;

  if (DevicePathToConnect == NULL) {
    return EFI_SUCCESS;
  }

  DevicePath         = DuplicateDevicePath (DevicePathToConnect);
  OriginalDevicePath = DevicePath;

  if (DevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  do {

    Instance = DevicePathInstance (&DevicePath, &Size);

    Next = Instance;
    while (!IsDevicePathEndType(Next)) {
      Next = NextDevicePathNode(Next);
    }
    SetDevicePathEndNode(Next);

    PreviousHandle = NULL;
    do {
      //
      // Find the handle that best matches the Device Path. If it is only a 
      // partial match the remaining part of the device path is returned in
      // RemainingDevicePath.
      //
      RemainingDevicePath = Instance;
      Status = BS->LocateDevicePath (
                      &gEfiDevicePathProtocolGuid,
                      &RemainingDevicePath,
                      &Handle
                      );
      if (!EFI_ERROR(Status)) {

        if (Handle == PreviousHandle) {
          Status = EFI_NOT_FOUND;
        } else {
          PreviousHandle = Handle;
          //
          // Connect all drivers that apply to Handle and RemainingDevicePath
          // If no drivers are connected Handle, then return EFI_NOT_FOUND
          // The Recursive flag is FALSE so only one level will be expanded.
          //
          Status = BS->ConnectController (
                          Handle, 
                          NULL, 
                          RemainingDevicePath, 
                          FALSE
                          );
        }
      }

      //
      // Loop until RemainingDevicePath is an empty device path
      //
    } while (!EFI_ERROR (Status) && !IsDevicePathEnd (RemainingDevicePath));

  } while (DevicePath != NULL);

  if (OriginalDevicePath != NULL) {
    FreePool(OriginalDevicePath);
  }

  //
  // A handle with DevicePath exists in the handle database
  //
  return EFI_SUCCESS;
}

VOID
SEnvConnectConsole (
  CHAR16      *VariableName,
  EFI_GUID    *PrimaryGuid,
  EFI_GUID    *ConsoleGuid,
  EFI_HANDLE  *ConsoleHandle,
  VOID        **ConsoleInterface
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  UINTN                     Index;
  UINTN                     ConsoleIndex;
  UINTN                     AllHandleCount;
  EFI_HANDLE                *AllHandleBuffer;
  VOID                      *Interface;

  *ConsoleHandle = NULL;
  *ConsoleInterface = NULL;
  ConsoleIndex = 0;

  DevicePath = LibGetVariable (VariableName, &EfiGlobalVariable);
  if (DevicePath != NULL) {
    SEnvConnectDevicePath (DevicePath);
    FreePool (DevicePath);
  }

  AllHandleBuffer = NULL;
  Status = BS->LocateHandleBuffer (
                 ByProtocol,
                 PrimaryGuid,
                 NULL,
                 &AllHandleCount,
                 &AllHandleBuffer
                 );
  if (!EFI_ERROR (Status) && AllHandleCount > 0) {
    *ConsoleHandle = AllHandleBuffer[0];
  } else if (*ConsoleHandle == NULL) {
    AllHandleBuffer = NULL;
    Status = BS->LocateHandleBuffer (
                   ByProtocol,
                   ConsoleGuid,
                   NULL,
                   &AllHandleCount,
                   &AllHandleBuffer
                   );
    //
    // Step 1: If a textout consplitter exists, attach to it, otherwise,
    // Step 2: Attach to first real console device. 
    // For ConIn/ConOut device detection, the step 1 does the same thing as
    // previous does (find consplitter), but for StdErr device, it is useful
    // because we need to attach textout consplitter to StdErr first if 
    // possible.
    // 
    //
    ConsoleIndex = 0;
    if (!EFI_ERROR(Status) && AllHandleCount > 0) {
      for (Index = 0; Index < AllHandleCount; Index++) {
        Status = BS->HandleProtocol (
                       AllHandleBuffer[Index],
                       &gEfiDevicePathProtocolGuid,
                       &Interface
                       );
        if (EFI_ERROR (Status)) {
          ConsoleIndex = Index;
          break;
        }
      }
    }
  }

  *ConsoleHandle = AllHandleBuffer[ConsoleIndex];

  if (*ConsoleHandle != NULL) {
    BS->HandleProtocol (
          *ConsoleHandle,
          ConsoleGuid,
          ConsoleInterface
          );
  }

  if (AllHandleBuffer) {
    BS->FreePool(AllHandleBuffer);
  }
}

VOID
SEnvConnectAllDriversToAllControllers (
  VOID
  )

{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  Status = LibLocateHandle (
             AllHandles,
             NULL,
             NULL,
             &AllHandleCount,
             &AllHandleBuffer
             );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < AllHandleCount; Index++) {

    //
    // Scan the handle database
    //
    Status = LibScanHandleDatabase (
               NULL,
               NULL,
               AllHandleBuffer[Index],
               NULL,
               &HandleCount, 
               &HandleBuffer, 
               &HandleType
               );
    if (EFI_ERROR (Status)) {
      return;
    }

    Device = TRUE;
    if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }
    if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
      Device = FALSE;
    }

    if (Device) {
      Parent = FALSE;
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
          Parent = TRUE;
        }
      }

      if (!Parent) {
        if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
          Status = BS->ConnectController (
                         AllHandleBuffer[Index],
                         NULL,
                         NULL,
                         TRUE
                         );
        }
      }
    }

    BS->FreePool (HandleBuffer);
    BS->FreePool (HandleType);
  }

  BS->FreePool (AllHandleBuffer);
}

VOID
SEnvConnectAllConsoles (
    VOID
    )

{
  SEnvConnectConsole (
    VarErrorOut,
    &gEfiPrimaryStandardErrorDeviceGuid,
    &gEfiSimpleTextOutProtocolGuid,
    &ST->StandardErrorHandle,
    (VOID **)&ST->StdErr
    );

  SEnvConnectConsole (
    VarConsoleOut,
    &gEfiPrimaryConsoleOutDeviceGuid,
    &gEfiSimpleTextOutProtocolGuid,
    &ST->ConsoleOutHandle,
    (VOID **)&ST->ConOut
    );

  SEnvConnectConsole (
    VarConsoleInp,
    &gEfiPrimaryConsoleInDeviceGuid,
    &gEfiSimpleTextInProtocolGuid,
    &ST->ConsoleInHandle,
    (VOID **)&ST->ConIn
    );

    SetCrc (&ST->Hdr);
}

EFI_STATUS
SEnvCmdReconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )

{
  EFI_STATUS    Status;
  EFI_STATUS    ReturnStatus;
  UINTN         HandleNumber;
  EFI_HANDLE    DriverImageHandle;
  EFI_HANDLE    DeviceHandle;
  EFI_HANDLE    ChildHandle;
  EFI_HANDLE    ContextOverride[2];
  EFI_SIMPLE_TEXT_OUT_PROTOCOL *TextOut;
  EFI_SIMPLE_TEXT_IN_PROTOCOL *TextIn;

  InitializeShellApplication (ImageHandle, SystemTable);

  if (SI->Argc < 2 ) {
    Print (L"reconnect: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }
  if (SI->Argc > 4) {
    Print (L"reconnect: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  Status            = EFI_SUCCESS;
  DeviceHandle      = NULL;
  DriverImageHandle = NULL;
  ChildHandle       = NULL;

  //
  // If command line is reconnect all the do that.
  //
  if (SI->Argv[1][0] == '-' || SI->Argv[1][0] == '/') {
    if (SI->Argv[1][1] == 'r' || SI->Argv[1][1] == 'R') {
      SEnvDisconnectAll();
      SEnvConnectAllDriversToAllControllers ();
      SEnvConnectAllConsoles ();
      ReturnStatus = EFI_SUCCESS;
      goto Done;
    } else {
      Print (L"reconnect: Unknown flag '%hc'\n", SI->Argv[1][1]);
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  DeviceHandle = NULL;
  HandleNumber = SEnvHandleNoFromStr (SI->Argv[1]);
  if (HandleNumber != 0) {
    DeviceHandle = SEnvHandles[HandleNumber - 1];
  }

  DriverImageHandle = NULL;
  if (SI->Argc >= 3) {
    HandleNumber = SEnvHandleNoFromStr (SI->Argv[2]);
    if (HandleNumber != 0) {
      DriverImageHandle = SEnvHandles[HandleNumber - 1];
    }
  }

  if (DeviceHandle == NULL && DriverImageHandle == NULL) {
    Print (L"reconnect: Handle not found\n");
    return EFI_NOT_FOUND;
  }

  ChildHandle = NULL;
  if (SI->Argc >= 4) {
    HandleNumber = SEnvHandleNoFromStr (SI->Argv[3]);
    if (HandleNumber != 0) {
      ChildHandle = SEnvHandles[HandleNumber - 1];
    }
  }

  if (DeviceHandle != NULL) {
    ReturnStatus = BS->DisconnectController (DeviceHandle, 
                                             DriverImageHandle, 
                                             ChildHandle
                                             );
    ContextOverride[0] = DriverImageHandle;
    ContextOverride[1] = NULL;
    Status = BS->ConnectController (DeviceHandle, 
                                    ContextOverride,
                                    NULL, 
                                    TRUE
                                    );
    if (!EFI_ERROR (ReturnStatus)) {
      ReturnStatus = Status;
    }
  } else {

    SEnvDisconnectAll ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
    ReturnStatus = EFI_SUCCESS;
  } 

Done:
  Status = BS->HandleProtocol ( SystemTable->ConsoleOutHandle, 
                                &gEfiSimpleTextOutProtocolGuid, 
                                (VOID **)&TextOut
                              );
  if (EFI_ERROR(Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  Status = BS->HandleProtocol ( SystemTable->ConsoleInHandle, 
                                &gEfiSimpleTextInProtocolGuid, 
                                (VOID **)&TextIn
                              );
  if (EFI_ERROR(Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  SEnvFreeHandleTable ();

  Print (L"ReconnectController(%x,%x,%x) : Status = %r\n", 
         DeviceHandle, DriverImageHandle, 
         ChildHandle, ReturnStatus);

  return ReturnStatus;
}
