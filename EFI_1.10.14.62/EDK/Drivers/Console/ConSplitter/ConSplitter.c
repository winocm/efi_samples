/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
 

Abstract:

  Console Splitter Driver. Any Handle that attatched 
  EFI_CONSOLE_IDENTIFIER_PROTOCOL can be bound by this driver.
  
  So far it works like any other driver by opening a SimpleTextIn and/or 
  SimpleTextOut protocol with EFI_OPEN_PROTOCOL_BY_DRIVER attributes. The big 
  difference is this driver does not layer a protocol on the passed in
  handle, or construct a child handle like a standard device or bus driver. 
  This driver produces three virtual handles as children, one for console input 
  splitter, one for console output splitter and one for error output splitter.
  EFI_CONSOLE_SPLIT_PROTOCOL will be attatched onto each virtual handle to 
  identify the splitter type.
  
  Each virtual handle, that supports both the EFI_CONSOLE_SPLIT_PROTOCOL
  and Console I/O protocol, will be produced in the driver entry point. 
  The virtual handle are added on driver entry and never removed.
  Such design ensures sytem function well during none console device situation.
  
--*/

#include "ConSplitter.h"

//
// Global Variables
//
static TEXT_IN_SPLITTER_PRIVATE_DATA  mConIn = {
  TEXT_IN_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextInReset,
    ConSplitterTextInReadKeyStroke,
    (EFI_EVENT) NULL
  },
  0,
  (EFI_SIMPLE_TEXT_IN_PROTOCOL **) NULL,
  0,

  {
    ConSplitterSimplePointerReset,
    ConSplitterSimplePointerGetState,
    (EFI_EVENT)NULL,
    (EFI_SIMPLE_POINTER_MODE *) NULL
  },
  {
    0x10000,
    0x10000,
    0x10000,
    TRUE,
    TRUE
  },
  0,
  (EFI_SIMPLE_POINTER_PROTOCOL **) NULL,
  0,

  FALSE,
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  0,
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  (EFI_EVENT) NULL
};

static TEXT_OUT_SPLITTER_PRIVATE_DATA mConOut = {
  TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextOutReset,
    ConSplitterTextOutOutputString,
    ConSplitterTextOutTestString,
    ConSplitterTextOutQueryMode,
    ConSplitterTextOutSetMode,
    ConSplitterTextOutSetAttribute,
    ConSplitterTextOutClearScreen,
    ConSplitterTextOutSetCursorPosition,
    ConSplitterTextOutEnableCursor,
    (EFI_SIMPLE_TEXT_OUTPUT_MODE *) NULL
  },
  {
    1,
    0,
    0,
    0,
    0,
    FALSE,
  },
  {
    ConSpliterUgaDrawGetMode,
    ConSpliterUgaDrawSetMode,
    ConSpliterUgaDrawBlt
  },
  0,
  0,
  0,
  0,
  (EFI_UGA_PIXEL *) NULL,

  {
    ConSpliterConsoleControlGetMode,
    ConSpliterConsoleControlSetMode,
    ConSpliterConsoleControlLockStdIn
  },

  0,
  (TEXT_OUT_AND_UGA_DATA *) NULL,
  0,
  (TEXT_OUT_SPLITTER_QUERY_DATA *) NULL,
  0,
  (INT32 *) NULL,

  EfiScreenText,
  0,
  0,
  (CHAR16 *) NULL,
  (INT32 *) NULL
};

static TEXT_OUT_SPLITTER_PRIVATE_DATA mStdErr = {
  TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextOutReset,
    ConSplitterTextOutOutputString,
    ConSplitterTextOutTestString,
    ConSplitterTextOutQueryMode,
    ConSplitterTextOutSetMode,
    ConSplitterTextOutSetAttribute,
    ConSplitterTextOutClearScreen,
    ConSplitterTextOutSetCursorPosition,
    ConSplitterTextOutEnableCursor,
    (EFI_SIMPLE_TEXT_OUTPUT_MODE *) NULL
  },
  {
    1,
    0,
    0,
    0,
    0,
    FALSE,
  },
  {
    ConSpliterUgaDrawGetMode,
    ConSpliterUgaDrawSetMode,
    ConSpliterUgaDrawBlt
  },
  0,
  0,
  0,
  0,
  (EFI_UGA_PIXEL *) NULL,

  {
    ConSpliterConsoleControlGetMode,
    ConSpliterConsoleControlSetMode,
    ConSpliterConsoleControlLockStdIn
  },

  0,
  (TEXT_OUT_AND_UGA_DATA *) NULL,
  0,
  (TEXT_OUT_SPLITTER_QUERY_DATA *) NULL,
  0,
  (INT32 *) NULL,

  EfiScreenText,
  0,
  0,
  (CHAR16 *) NULL,
  (INT32 *) NULL
};

EFI_DRIVER_BINDING_PROTOCOL gConSplitterConInDriverBinding = {
  ConSplitterConInDriverBindingSupported,
  ConSplitterConInDriverBindingStart,
  ConSplitterConInDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_BINDING_PROTOCOL gConSplitterSimplePointerDriverBinding = {
  ConSplitterSimplePointerDriverBindingSupported,
  ConSplitterSimplePointerDriverBindingStart,
  ConSplitterSimplePointerDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_BINDING_PROTOCOL gConSplitterConOutDriverBinding = {
  ConSplitterConOutDriverBindingSupported,
  ConSplitterConOutDriverBindingStart,
  ConSplitterConOutDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_BINDING_PROTOCOL gConSplitterStdErrDriverBinding = {
  ConSplitterStdErrDriverBindingSupported,
  ConSplitterStdErrDriverBindingStart,
  ConSplitterStdErrDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT (ConSplitterDriverEntry)

EFI_STATUS
ConSplitterDriverEntry (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  )
/*++

Routine Description:
  Intialize a virtual console device to act as an agrigator of physical console
  devices. 

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 
  EFI_STATUS

--*/
{
  EFI_STATUS                          Status;
  
  //
  // Initialize the EFI Driver Library and install the 
  // EFI Driver Binding Protocols
  //
  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle, 
             SystemTable, 
             &gConSplitterConInDriverBinding, 
             ImageHandle,
             &gConSplitterConInComponentName,
             NULL,
             NULL
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle, 
             SystemTable, 
             &gConSplitterSimplePointerDriverBinding, 
             NULL,
             &gConSplitterSimplePointerComponentName,
             NULL,
             NULL
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle, 
             SystemTable, 
             &gConSplitterConOutDriverBinding, 
             NULL,
             &gConSplitterConOutComponentName,
             NULL,
             NULL
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle, 
             SystemTable, 
             &gConSplitterStdErrDriverBinding, 
             NULL,
             &gConSplitterStdErrComponentName,
             NULL,
             NULL
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // The driver creates virtual handles for ConIn, ConOut, and StdErr.
  // The virtual handles will always exist even if no console exist in the 
  // system. This is need to support hotplug devices like USB.
  //

  //
  // Create virtual device handle for StdErr Splitter
  //
  Status = ConSplitterTextOutConstructor (&mStdErr);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mStdErr.VirtualHandle,
                    &gEfiSimpleTextOutProtocolGuid,      &mStdErr.TextOut,
                    &gEfiPrimaryStandardErrorDeviceGuid, NULL,
                    NULL
                    );
  }

  //
  // Create virtual device handle for ConIn Splitter
  //
  Status = ConSplitterTextInConstructor (&mConIn);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mConIn.VirtualHandle,
                    &gEfiSimpleTextInProtocolGuid,   &mConIn.TextIn,
                    &gEfiSimplePointerProtocolGuid,  &mConIn.SimplePointer,
                    &gEfiPrimaryConsoleInDeviceGuid, NULL,
                    NULL
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Update the EFI System Table with new virtual console
      //
      gST->ConsoleInHandle = mConIn.VirtualHandle;
      gST->ConIn = &mConIn.TextIn;
    }
  }

  //
  // Create virtual device handle for ConOut Splitter
  //
  Status = ConSplitterTextOutConstructor (&mConOut);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mConOut.VirtualHandle,
                    &gEfiSimpleTextOutProtocolGuid,   &mConOut.TextOut,
                    &gEfiUgaDrawProtocolGuid,         &mConOut.UgaDraw,
                    &gEfiConsoleControlProtocolGuid,  &mConOut.ConsoleControl,
                    &gEfiPrimaryConsoleOutDeviceGuid, NULL,
                    NULL
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Update the EFI System Table with new virtual console
      //
      gST->ConsoleOutHandle = mConOut.VirtualHandle;
      gST->ConOut = &mConOut.TextOut;
    }

  }

  //
  // Update the CRC32 in the EFI System Table header
  //
  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 ((UINT8 *)&gST->Hdr, 
                        gST->Hdr.HeaderSize, 
                        &gST->Hdr.CRC32
                        );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConSplitterTextInConstructor (
  TEXT_IN_SPLITTER_PRIVATE_DATA       *ConInPrivate
  )
{
  EFI_STATUS                          Status;

  //
  // Initilize console input splitter's private data.
  //  
  Status = ConSplitterGrowBuffer (
            sizeof(EFI_SIMPLE_TEXT_IN_PROTOCOL *), 
            &ConInPrivate->TextInListCount, 
            (VOID **)&ConInPrivate->TextInList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  //
  // Create Event to support locking StdIn Device
  //
  Status = gBS->CreateEvent (
            EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
            EFI_TPL_CALLBACK,
            ConSpliterConsoleControlLockStdInEvent,
            NULL,
            &ConInPrivate->LockEvent
            );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEvent (
            EFI_EVENT_NOTIFY_WAIT,
            EFI_TPL_NOTIFY,
            ConSplitterTextInWaitForKey,
            ConInPrivate,
            &ConInPrivate->TextIn.WaitForKey
            );
  ASSERT_EFI_ERROR (Status);

  ConInPrivate->SimplePointer.Mode  = &ConInPrivate->SimplePointerMode;
  
  Status = ConSplitterGrowBuffer (
            sizeof(EFI_SIMPLE_POINTER_PROTOCOL*), 
            &ConInPrivate->PointerListCount, 
            (VOID **)&ConInPrivate->PointerList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = gBS->CreateEvent (
            EFI_EVENT_NOTIFY_WAIT,
            EFI_TPL_NOTIFY,
            ConSplitterSimplePointerWaitForInput,
            ConInPrivate,
            &ConInPrivate->SimplePointer.WaitForInput
            );

  return Status;
}

STATIC
EFI_STATUS
ConSplitterTextOutConstructor (
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *ConOutPrivate
  )
{
  EFI_STATUS                          Status;
  
  //
  // Initilize console output splitter's private data.
  //
  ConOutPrivate->TextOut.Mode   = &ConOutPrivate->TextOutMode;

  Status = ConSplitterGrowBuffer (
            sizeof (TEXT_OUT_AND_UGA_DATA), 
            &ConOutPrivate->TextOutListCount, 
            (VOID **)&ConOutPrivate->TextOutList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = ConSplitterGrowBuffer (
            sizeof (TEXT_OUT_SPLITTER_QUERY_DATA), 
            &ConOutPrivate->TextOutQueryDataCount, 
            (VOID **)&ConOutPrivate->TextOutQueryData
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Setup the DevNullTextOut console to 80 x 25
  //
  ConOutPrivate->TextOutQueryData[0].Columns = 80;
  ConOutPrivate->TextOutQueryData[0].Rows = 25;
  DevNullTextOutSetMode (ConOutPrivate, 0);

  //
  // Setup the DevNullUgaDraw to 800 x 600 x 32 bits per pixel
  //
  ConSpliterUgaDrawSetMode (&ConOutPrivate->UgaDraw, 800, 600, 32, 60);
  
  return Status;
}

STATIC
EFI_STATUS
ConSplitterSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_GUID                        *Guid
  )
/*++

Routine Description:
  Generic Supported Check

Arguments:
  (Standard DriverBinding Protocol Supported() function)

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  VOID                                *Instance;

  //
  // Make sure the Console Splitter does not attempt to attach to itself
  //
  if (ControllerHandle == mConIn.VirtualHandle) {
    return EFI_UNSUPPORTED;
  }
  if (ControllerHandle == mConOut.VirtualHandle) {
    return EFI_UNSUPPORTED;
  }
  if (ControllerHandle == mStdErr.VirtualHandle) {
    return EFI_UNSUPPORTED;
  }

  //
  // Check to see whether the handle has the ConsoleInDevice GUID on it
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  Guid,
                  &Instance,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if ( EFI_ERROR (Status) ) {
    return Status;
  }

  gBS->CloseProtocol (
         ControllerHandle, 
         Guid,
         This->DriverBindingHandle,
         ControllerHandle
         );
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Console In Supported Check

Arguments:
  (Standard DriverBinding Protocol Supported() function)

Returns:

  None

--*/
{
  return ConSplitterSupported (
                          This,
                          ControllerHandle,
                          &gEfiConsoleInDeviceGuid
                          );
}
  
STATIC
EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Standard Error Supported Check

Arguments:
  (Standard DriverBinding Protocol Supported() function)

Returns:

  None

--*/
{
  return ConSplitterSupported (
                              This,
                              ControllerHandle,
                              &gEfiSimplePointerProtocolGuid
                              );
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Console Out Supported Check

Arguments:
  (Standard DriverBinding Protocol Supported() function)

Returns:

  None

--*/
{
  return ConSplitterSupported (
                          This,
                          ControllerHandle,
                          &gEfiConsoleOutDeviceGuid
                          );
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Standard Error Supported Check

Arguments:
  (Standard DriverBinding Protocol Supported() function)

Returns:

  None

--*/
{
  return ConSplitterSupported (
                          This,
                          ControllerHandle,
                          &gEfiStandardErrorDeviceGuid
                          );
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ConSplitterVirtualHandle,
  IN  EFI_GUID                        *DeviceGuid,
  IN  EFI_GUID                        *InterfaceGuid,
  IN  VOID                            **Interface
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual 
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                          Status;
  VOID                                *Instance;

  //
  // Check to see whether the handle has the ConsoleInDevice GUID on it
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  DeviceGuid,
                  &Instance,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  DeviceGuid,
                  &Instance,
                  This->DriverBindingHandle,   
                  ConSplitterVirtualHandle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return gBS->OpenProtocol (
                ControllerHandle,
                InterfaceGuid,
                Interface,
                This->DriverBindingHandle,   
                ConSplitterVirtualHandle, 
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual 
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL         *TextIn; 

  //
  // Start ConSplitter on ControllerHandle, and create the virtual 
  // agrogated console device on first call Start for a SimpleTextIn handle.
  //
  Status = ConSplitterStart (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiConsoleInDeviceGuid,
             &gEfiSimpleTextInProtocolGuid,
             (VOID **)&TextIn
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return ConSplitterTextInAddDevice (&mConIn, TextIn);
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual 
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_POINTER_PROTOCOL         *SimplePointer;


  Status = ConSplitterStart (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiSimplePointerProtocolGuid,
             &gEfiSimplePointerProtocolGuid,
             (VOID **)&SimplePointer
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  return ConSplitterSimplePointerAddDevice (&mConIn, SimplePointer);
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual 
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL        *TextOut;
  EFI_UGA_DRAW_PROTOCOL               *UgaDraw;


  Status = ConSplitterStart (
             This,
             ControllerHandle,
             mConOut.VirtualHandle,
             &gEfiConsoleOutDeviceGuid,
             &gEfiSimpleTextOutProtocolGuid,
             (VOID **)&TextOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Open UGA_DRAW protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiUgaDrawProtocolGuid,
                  &UgaDraw,
                  This->DriverBindingHandle,   
                  mConOut.VirtualHandle, 
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    UgaDraw = NULL;
  }
 
  //
  // If both ConOut and StdErr incorporate the same Text Out device, 
  // their MaxMode and QueryData should be the intersection of both.
  //
  return ConSplitterTextOutAddDevice (&mConOut, TextOut, UgaDraw);
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual 
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL        *TextOut;


  Status = ConSplitterStart (
             This,
             ControllerHandle,
             mStdErr.VirtualHandle,
             &gEfiStandardErrorDeviceGuid,
             &gEfiSimpleTextOutProtocolGuid,
             (VOID **)&TextOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // If both ConOut and StdErr incorporate the same Text Out device, 
  // their MaxMode and QueryData should be the intersection of both.
  //
  Status = ConSplitterTextOutAddDevice (&mStdErr, TextOut, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (mStdErr.CurrentNumberOfConsoles == 1) {
    gST->StandardErrorHandle = mStdErr.VirtualHandle;
    gST->StdErr = &mStdErr.TextOut;
    //
    // Update the CRC32 in the EFI System Table header
    //
    gST->Hdr.CRC32 = 0;
    gBS->CalculateCrc32 ((UINT8 *)&gST->Hdr, 
                          gST->Hdr.HeaderSize, 
                          &gST->Hdr.CRC32
                          );
  }

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ConSplitterVirtualHandle,
  IN  EFI_GUID                        *DeviceGuid,
  IN  EFI_GUID                        *InterfaceGuid,
  IN  VOID                            **Interface
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                          Status;

  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  InterfaceGuid,    
                  Interface,
                  This->DriverBindingHandle,   
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // close the protocol refered.
  //
  gBS->CloseProtocol (
         ControllerHandle, 
         DeviceGuid,
         This->DriverBindingHandle,   
         ConSplitterVirtualHandle
         );
  gBS->CloseProtocol (
         ControllerHandle, 
         DeviceGuid,
         This->DriverBindingHandle,
         ControllerHandle
         );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL         *TextIn; 

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiConsoleInDeviceGuid,
             &gEfiSimpleTextInProtocolGuid,
             (VOID **)&TextIn
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Delete this console input device's data structures.
  //
  return ConSplitterTextInDeleteDevice (&mConIn, TextIn);
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_POINTER_PROTOCOL         *SimplePointer; 

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiSimplePointerProtocolGuid,
             &gEfiSimplePointerProtocolGuid,
             (VOID **)&SimplePointer
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Delete this console input device's data structures.
  //
  return ConSplitterSimplePointerDeleteDevice (&mConIn, SimplePointer);
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL        *TextOut;
  EFI_UGA_DRAW_PROTOCOL               *UgaDraw;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
             This,
             ControllerHandle,
             mConOut.VirtualHandle,
             &gEfiConsoleOutDeviceGuid,
             &gEfiSimpleTextOutProtocolGuid,
             (VOID **)&TextOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Remove any UGA devices
  //
  Status = gBS->OpenProtocol (
                ControllerHandle,
                &gEfiUgaDrawProtocolGuid,
                &UgaDraw,
                This->DriverBindingHandle,   
                mConOut.VirtualHandle, 
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );

  //
  // Delete this console output device's data structures.
  //
  return ConSplitterTextOutDeleteDevice (&mConOut, TextOut);
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL        *TextOut;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
             This,
             ControllerHandle,
             mStdErr.VirtualHandle,
             &gEfiStandardErrorDeviceGuid,
             &gEfiSimpleTextOutProtocolGuid,
             (VOID **)&TextOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Delete this console error out device's data structures.
  //
  Status = ConSplitterTextOutDeleteDevice (&mStdErr, TextOut);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (mStdErr.CurrentNumberOfConsoles == 0) {
    gST->StandardErrorHandle = NULL;
    gST->StdErr = NULL;
    //
    // Update the CRC32 in the EFI System Table header
    //
    gST->Hdr.CRC32 = 0;
    gBS->CalculateCrc32 ((UINT8 *)&gST->Hdr, 
                          gST->Hdr.HeaderSize, 
                          &gST->Hdr.CRC32
                          );
  }

  return Status;
}

EFI_STATUS
ConSplitterGrowBuffer (
  IN  UINTN                           SizeOfCount,
  IN  UINTN                           *Count,
  IN OUT  VOID                        **Buffer
  )
/*++

Routine Description:
  Take the passed in Buffer of size SizeOfCount and grow the buffer
  by MAX (CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT, MaxGrow) * SizeOfCount
  bytes. Copy the current data in Buffer to the new version of Buffer
  and free the old version of buffer.


Arguments:
  SizeOfCount - Size of element in array
  Count       - Current number of elements in array
  Buffer      - Bigger version of passed in Buffer with all the data

Returns:
  EFI_SUCCESS - Buffer size has grown
  EFI_OUT_OF_RESOURCES - Could not grow the buffer size

  None

--*/
{
  UINTN                               NewSize;
  UINTN                               OldSize;
  VOID                                *Ptr;
  
  //
  // grow the buffer to new buffer size,
  // copy the old buffer's content to the new-size buffer,
  // then free the old buffer.
  //
  OldSize = *Count * SizeOfCount;
  *Count += CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT;
  NewSize = *Count * SizeOfCount;

  Ptr = EfiLibAllocateZeroPool (NewSize);
  if ( Ptr == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }

  EfiCopyMem (Ptr, *Buffer, OldSize); 
  
  if (*Buffer != NULL) {
    gBS->FreePool (*Buffer);
  }
  
  *Buffer = Ptr;    

  return EFI_SUCCESS;
}    

EFI_STATUS
ConSplitterTextInAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *TextIn 
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  
  //
  // If the Text In List is full, enlarge it by calling growbuffer().
  //
  if ( Private->CurrentNumberOfConsoles >= Private->TextInListCount ) {    
    Status = ConSplitterGrowBuffer (
                sizeof (EFI_SIMPLE_TEXT_IN_PROTOCOL *),
                &Private->TextInListCount,
                (VOID **)&Private->TextInList
                );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }    
  }
  
  //
  // Add the new text-in device data structure into the Text In List.
  //
  Private->TextInList[Private->CurrentNumberOfConsoles] = TextIn;
  Private->CurrentNumberOfConsoles++;

  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterTextInDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *TextIn
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN                               Index;
  //
  // Remove the specified text-in device data structure from the Text In List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    if (Private->TextInList[Index] == TextIn) {
      for (Index = Index; Index < Private->CurrentNumberOfConsoles - 1; 
            Index++) {
        Private->TextInList[Index] = Private->TextInList[Index + 1];
      }
      Private->CurrentNumberOfConsoles--;
      return EFI_SUCCESS;
    }
  }   
  return EFI_NOT_FOUND;
}

EFI_STATUS
ConSplitterSimplePointerAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *SimplePointer 
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  
  //
  // If the Text In List is full, enlarge it by calling growbuffer().
  //
  if (Private->CurrentNumberOfPointers >= Private->PointerListCount) {    
    Status = ConSplitterGrowBuffer (
                sizeof (EFI_SIMPLE_POINTER_PROTOCOL *),
                &Private->PointerListCount,
                (VOID **)&Private->PointerList
                );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }    
  }
  
  //
  // Add the new text-in device data structure into the Text In List.
  //
  Private->PointerList[Private->CurrentNumberOfPointers] = SimplePointer;
  Private->CurrentNumberOfPointers++;
  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterSimplePointerDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *SimplePointer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN                               Index;
  //
  // Remove the specified text-in device data structure from the Text In List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {
    if (Private->PointerList[Index] == SimplePointer) {
      for (Index = Index; Index < Private->CurrentNumberOfPointers - 1; 
            Index++) {
        Private->PointerList[Index] = Private->PointerList[Index + 1];
      }
      Private->CurrentNumberOfPointers--;
      return EFI_SUCCESS;
    }
  }   
  return EFI_NOT_FOUND;
}

EFI_STATUS
ConSplitterGrowMapTable (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN                               Size, NewSize, TotalSize;
  INT32                               *TextOutModeMap, *OldTextOutModeMap;
  INT32                               *SrcAddress;
  INT32                               Index;

  NewSize           = Private->TextOutListCount * sizeof (INT32);
  OldTextOutModeMap = Private->TextOutModeMap;
  TotalSize         = NewSize * Private->TextOutQueryDataCount;

  TextOutModeMap    = EfiLibAllocateZeroPool (TotalSize);
  if ( TextOutModeMap == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }
  EfiSetMem (TextOutModeMap, TotalSize, 0xFF);
  Private->TextOutModeMap = TextOutModeMap;

  //
  // If TextOutList has been enlarged, need to realloc the mode map table
  // The mode map table is regarded as a two dimension array. 
  //
  //                         Old                    New
  //  0   ---------> TextOutListCount ----> TextOutListCount
  //  |   -------------------------------------------
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  // \/  |                    |                      |
  //      -------------------------------------------
  // QueryDataCount
  //
  if ( OldTextOutModeMap ) {

    Size            = Private->CurrentNumberOfConsoles * sizeof (INT32);
    Index           = 0;
    SrcAddress      = OldTextOutModeMap;

    //
    // Copy the old data to the new one
    //
    while ( Index < Private->TextOutMode.MaxMode ) {
      EfiCopyMem (TextOutModeMap, SrcAddress, Size);
      TextOutModeMap  += NewSize;
      SrcAddress      += Size;
      Index ++;
    }

    //
    // Free the old buffer
    //
    gBS->FreePool (OldTextOutModeMap);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterAddOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *TextOut
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  INT32                               MaxMode;
  INT32                               Mode;
  UINTN                               Index;

  MaxMode = TextOut->Mode->MaxMode;
  Private->TextOutMode.MaxMode = MaxMode;

  //
  // Grow the buffer if query data buffer is not large enough to 
  // hold all the mode supported by the first console.
  //
  while ( MaxMode > (INT32)Private->TextOutQueryDataCount ) {
    Status = ConSplitterGrowBuffer (
                sizeof (TEXT_OUT_SPLITTER_QUERY_DATA),
                &Private->TextOutQueryDataCount,
                (VOID **)&Private->TextOutQueryData
                );
    if ( EFI_ERROR (Status) ) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  //
  // Allocate buffer for the output mode map
  //
  Status = ConSplitterGrowMapTable (Private);
  if ( EFI_ERROR (Status) ) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // As the first textout device, directly add the mode in to QueryData
  // and at the same time record the mapping between QueryData and TextOut.
  //
  Mode  = 0;
  Index = 0;
  while ( Mode < MaxMode ) {
    TextOut->QueryMode (
               TextOut, 
               Mode, 
               &Private->TextOutQueryData[Mode].Columns, 
               &Private->TextOutQueryData[Mode].Rows
               );
    Private->TextOutModeMap[Index] = Mode;
    Mode ++;
    Index += Private->TextOutListCount;
  }

  return EFI_SUCCESS;
}

VOID
ConSplitterGetIntersection (
  IN  INT32                           *TextOutModeMap,
  IN  INT32                           *NewlyAddedMap,
  IN  UINTN                           MapStepSize,
  IN  UINTN                           NewMapStepSize,
  OUT INT32                           *MaxMode,
  OUT INT32                           *CurrentMode
  )
{
  INT32                               Index;
  INT32                               *CurrentMapEntry, *NextMapEntry;
  INT32                               CurrentMaxMode, Mode;

  Index             = 0;
  CurrentMapEntry   = TextOutModeMap;
  NextMapEntry      = TextOutModeMap;
  CurrentMaxMode    = *MaxMode;
  Mode              = *CurrentMode;

  while ( Index < CurrentMaxMode ) {
    if ( *NewlyAddedMap == -1 ) {
      //
      // This mode is not supported any more. Remove it. Special care
      // must be taken as this remove will also affect current mode;
      //
      if ( Index == *CurrentMode ) {
        Mode        = -1;
      } else if ( Index < *CurrentMode ) {
        Mode --;
      }
      (*MaxMode) --;
    } else {
      if ( CurrentMapEntry != NextMapEntry ) {
        EfiCopyMem (NextMapEntry, CurrentMapEntry, MapStepSize * sizeof (INT32));
      }
      NextMapEntry  += MapStepSize;
    }
    CurrentMapEntry += MapStepSize;
    NewlyAddedMap   += NewMapStepSize;
    Index ++;
  }

  *CurrentMode = Mode;

  return;
}

VOID
ConSplitterSyncOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *TextOut
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  INT32                               CurrentMode;
  INT32                               CurrentMaxMode, Mode;
  INT32                               Index;
  INT32                               *TextOutModeMap, *MapTable;
  TEXT_OUT_SPLITTER_QUERY_DATA        *TextOutQueryData;
  UINTN                               Rows;
  UINTN                               Columns;
  UINTN                               StepSize;

  //
  // Must make sure that current mode won't change even if mode number changes 
  //
  CurrentMode           = Private->TextOutMode.Mode;
  CurrentMaxMode        = Private->TextOutMode.MaxMode;
  TextOutModeMap        = Private->TextOutModeMap;
  StepSize              = Private->TextOutListCount;
  TextOutQueryData      = Private->TextOutQueryData;

  //
  // Query all the mode that the newly added TextOut supports
  //
  Mode                  = 0;
  MapTable              = TextOutModeMap + Private->CurrentNumberOfConsoles;
  while ( Mode < TextOut->Mode->MaxMode ) {
    TextOut->QueryMode (TextOut, Mode, &Columns, &Rows);

    //
    // Search the QueryData database to see if they intersects
    //
    Index = 0;
    while ( Index < CurrentMaxMode ) {
      if ( ( TextOutQueryData[Index].Rows == Rows )
        && ( TextOutQueryData[Index].Columns == Columns ) ) {
        MapTable[Index * StepSize]  = Mode;
        break;
      }
      Index ++;
    }

    Mode ++;
  }

  //
  // Now search the TextOutModeMap table to find the intersection of supported
  // mode between ConSplitter and the newly added device.
  //
  ConSplitterGetIntersection (
    TextOutModeMap,
    MapTable,
    StepSize,
    StepSize,
    &Private->TextOutMode.MaxMode,
    &Private->TextOutMode.Mode
    );

  return;
}

EFI_STATUS
ConSplitterGetIntersectionBetweenConOutAndStrErr (
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN                           ConOutNumOfConsoles, StdErrNumOfConsoles;
  TEXT_OUT_AND_UGA_DATA           *ConOutTextOutList, *StdErrTextOutList;
  UINTN                           Indexi, Indexj;
  UINTN                           Rows, Columns;
  INT32                           ConOutCurrentMode, StdErrCurrentMode;
  INT32                           ConOutMaxMode, StdErrMaxMode;
  INT32                           Mode, Index;
  INT32                           *ConOutModeMap, *StdErrModeMap;
  INT32                           *ConOutMapTable, *StdErrMapTable;
  TEXT_OUT_SPLITTER_QUERY_DATA    *ConOutQueryData, *StdErrQueryData;
  UINTN                           ConOutStepSize, StdErrStepSize;
  BOOLEAN                         FoundTheSameTextOut;
  UINTN                           ConOutMapTableSize, StdErrMapTableSize;

  ConOutNumOfConsoles   = mConOut.CurrentNumberOfConsoles;
  StdErrNumOfConsoles   = mStdErr.CurrentNumberOfConsoles;
  ConOutTextOutList     = mConOut.TextOutList;
  StdErrTextOutList     = mStdErr.TextOutList;

  Indexi = 0;
  FoundTheSameTextOut   = FALSE;
  while ( ( Indexi < ConOutNumOfConsoles ) && ( !FoundTheSameTextOut ) ) {
    Indexj = 0;
    while ( Indexj < StdErrNumOfConsoles ) {
      if ( ConOutTextOutList->TextOut == StdErrTextOutList->TextOut ) {
        FoundTheSameTextOut = TRUE;
        break;
      }
      Indexj ++;
      StdErrTextOutList ++;
    }
    Indexi ++;
    ConOutTextOutList ++;
  }

  if ( !FoundTheSameTextOut ) {
    return EFI_SUCCESS;
  }

  //
  // Must make sure that current mode won't change even if mode number changes 
  //
  ConOutCurrentMode     = mConOut.TextOutMode.Mode;
  ConOutMaxMode         = mConOut.TextOutMode.MaxMode;
  ConOutModeMap         = mConOut.TextOutModeMap;
  ConOutStepSize        = mConOut.TextOutListCount;
  ConOutQueryData       = mConOut.TextOutQueryData;

  StdErrCurrentMode     = mStdErr.TextOutMode.Mode;
  StdErrMaxMode         = mStdErr.TextOutMode.MaxMode;
  StdErrModeMap         = mStdErr.TextOutModeMap;
  StdErrStepSize        = mStdErr.TextOutListCount;
  StdErrQueryData       = mStdErr.TextOutQueryData;

  //
  // Allocate the map table and set the map table's index to -1. 
  //
  ConOutMapTableSize    = ConOutMaxMode * sizeof (INT32);
  ConOutMapTable        = EfiLibAllocateZeroPool (ConOutMapTableSize);
  if ( ConOutMapTable == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }
  EfiSetMem (ConOutMapTable, ConOutMapTableSize, 0xFF);

  StdErrMapTableSize    = StdErrMaxMode * sizeof (INT32);
  StdErrMapTable        = EfiLibAllocateZeroPool (StdErrMapTableSize);
  if ( StdErrMapTable == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }
  EfiSetMem (StdErrMapTable, StdErrMapTableSize, 0xFF);

  //
  // Find the intersection of the two set of modes. If they actually intersect, the
  // correponding entry in the map table is set to 1.
  //
  Mode                  = 0;
  while ( Mode < ConOutMaxMode ) {
    //
    // Search the other's QueryData database to see if they intersect
    //
    Index               = 0;
    Rows                = ConOutQueryData[Mode].Rows;
    Columns             = ConOutQueryData[Mode].Columns;
    while ( Index < StdErrMaxMode ) {
      if ( ( StdErrQueryData[Index].Rows == Rows )
        && ( StdErrQueryData[Index].Columns == Columns ) ) {
        ConOutMapTable[Mode]  = 1;
        StdErrMapTable[Index] = 1;
        break;
      }
      Index ++;
    }
    Mode ++;
  }

  //
  // Now search the TextOutModeMap table to find the intersection of supported
  // mode between ConSplitter and the newly added device.
  //
  ConSplitterGetIntersection (
    ConOutModeMap,
    ConOutMapTable,
    mConOut.TextOutListCount,
    1,
    &(mConOut.TextOutMode.MaxMode),
    &(mConOut.TextOutMode.Mode)
    );
  if ( mConOut.TextOutMode.Mode < 0 ) {
    mConOut.TextOut.SetMode (&(mConOut.TextOut), 0);
  }

  ConSplitterGetIntersection (
    StdErrModeMap,
    StdErrMapTable,
    mStdErr.TextOutListCount,
    1,
    &(mStdErr.TextOutMode.MaxMode),
    &(mStdErr.TextOutMode.Mode)
    );
  if ( mStdErr.TextOutMode.Mode < 0 ) {
    mStdErr.TextOut.SetMode (&(mStdErr.TextOut), 0);
  }

  gBS->FreePool (ConOutMapTable);
  gBS->FreePool (StdErrMapTable);

  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterTextOutAddDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *TextOut,
  IN  EFI_UGA_DRAW_PROTOCOL           *UgaDraw
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS              Status;
  UINTN                   CurrentNumOfConsoles;
  INT32                   CurrentMode, MaxMode;
  TEXT_OUT_AND_UGA_DATA   *TextAndUga;

  Status                = EFI_SUCCESS;
  CurrentNumOfConsoles  = Private->CurrentNumberOfConsoles;

  //
  // If the Text Out List is full, enlarge it by calling growbuffer().
  //
  while ( CurrentNumOfConsoles >= Private->TextOutListCount ) {
    Status = ConSplitterGrowBuffer (
                  sizeof (TEXT_OUT_AND_UGA_DATA),
                  &Private->TextOutListCount,
                  (VOID **)&Private->TextOutList
                  );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Also need to reallocate the TextOutModeMap table
    //
    Status = ConSplitterGrowMapTable (Private);
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  
  TextAndUga = &Private->TextOutList[CurrentNumOfConsoles];

  TextAndUga->TextOut = TextOut;
  TextAndUga->UgaDraw = UgaDraw;
  if (UgaDraw == NULL) {
    //
    // If No UGA device then use the ConOut device
    //
    TextAndUga->TextOutEnabled = TRUE;
  } else {
    //
    // If UGA device use ConOut device only used if UGA screen is in Text mode
    //
    TextAndUga->TextOutEnabled = (BOOLEAN)(Private->UgaMode == EfiScreenText);
  }

  if ( CurrentNumOfConsoles == 0 ) {
    //
    // Add the first device's output mode to console splitter's mode list
    //
    Status = ConSplitterAddOutputMode (Private, TextOut);
  } else {
    ConSplitterSyncOutputMode (Private, TextOut);
  }

  Private->CurrentNumberOfConsoles ++;

  //
  // Scan both TextOutList, for the intersection TextOut device 
  // maybe both ConOut and StdErr incorporate the same Text Out
  // device in them, thus the output of both should be synced.
  //
  ConSplitterGetIntersectionBetweenConOutAndStrErr();

  CurrentMode = Private->TextOutMode.Mode;
  MaxMode     = Private->TextOutMode.MaxMode;
  ASSERT (MaxMode >= 1);

  if (Private->UgaMode == EfiScreenGraphics && UgaDraw != NULL) {
    //
    // We just added a new UGA device in graphics mode
    //
    DevNullUgaSync (Private, UgaDraw);

  } else if ( ( CurrentMode >= 0 ) && ( UgaDraw != NULL ) &&
            ( CurrentMode < Private->TextOutMode.MaxMode ) ) {
    //
    // The new console supports the same mode of the current console so sync up
    //
    DevNullSyncUgaStdOut (Private);
  } else {
    //
    // If ConOut, then set the mode to Mode #0 which us 80 x 25
    //
    Private->TextOut.SetMode (&Private->TextOut, 0);
  }

  return Status;
}

EFI_STATUS
ConSplitterTextOutDeleteDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *TextOut
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  INT32                               Index;
  UINTN                               CurrentNumOfConsoles;
  TEXT_OUT_AND_UGA_DATA               *TextOutList;
  EFI_STATUS                          Status;
  
  //
  // Remove the specified text-out device data structure from the Text out List,
  // and rearrange the remaining data structures in the Text out List.
  //
  CurrentNumOfConsoles    = Private->CurrentNumberOfConsoles;
  Index                   = (INT32)CurrentNumOfConsoles - 1;
  TextOutList             = Private->TextOutList;
  while ( Index >= 0 ) {
    if (TextOutList->TextOut == TextOut) {
      EfiCopyMem (TextOutList, TextOutList + 1, sizeof (TEXT_OUT_AND_UGA_DATA) * Index);
      CurrentNumOfConsoles --;
      break;
    }
    Index --;
    TextOutList ++;
  }

  //
  // The specified TextOut is not managed by the ConSplitter driver
  //
  if ( Index < 0 ) {
    return EFI_NOT_FOUND;
  }

  if ( CurrentNumOfConsoles == 0 ) {
    //
    // If the number of consoles is zero clear the Dev NULL device
    //
    Private->CurrentNumberOfConsoles = 0;
    Private->TextOutMode.MaxMode = 1;
    Private->TextOutQueryData[0].Columns = 80;
    Private->TextOutQueryData[0].Rows = 25;
    DevNullTextOutSetMode (Private, 0);

    return EFI_SUCCESS;
  }

  //
  // Max Mode is realy an intersection of the QueryMode command to all
  // devices. So we must copy the QueryMode of the first device to
  // QueryData.
  //
  EfiZeroMem (
    Private->TextOutQueryData, 
    Private->TextOutQueryDataCount * sizeof(TEXT_OUT_SPLITTER_QUERY_DATA)
    );

  gBS->FreePool (Private->TextOutModeMap);
  Private->TextOutModeMap           = NULL;
  TextOutList                       = Private->TextOutList;

  //
  // Add the first TextOut to the QueryData array and ModeMap table
  //
  Status = ConSplitterAddOutputMode (Private, TextOutList->TextOut);

  //
  // Now add one by one
  //
  Index                             = 1;
  Private->CurrentNumberOfConsoles  = 1;
  TextOutList ++;
  while ( (UINTN)Index < CurrentNumOfConsoles ) {
    ConSplitterSyncOutputMode (Private, TextOutList->TextOut);
    Index ++;
    Private->CurrentNumberOfConsoles ++;
    TextOutList ++;
  }

  ConSplitterGetIntersectionBetweenConOutAndStrErr();

  return Status;
}


//
// ConSplitter TextIn member functions
//

EFI_STATUS 
EFIAPI
ConSplitterTextInReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *This,
  IN  BOOLEAN                         ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES            - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
{
  EFI_STATUS                          Status;
  EFI_STATUS                          ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA       *Private;
  UINTN                               Index;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  
  //
  // return the worst status met
  //
  for ( Index = 0, ReturnStatus = EFI_SUCCESS;
          Index < Private->CurrentNumberOfConsoles; Index ++ ) {
    Status  = Private->TextInList[Index]->Reset (
                        Private->TextInList[Index],
                        ExtendedVerification
                        );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextInPrivateReadKeyStroke (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  OUT EFI_INPUT_KEY                   *Key
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This   - Protocol instance pointer.
    Key    - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES        - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to 
                        hardware errors.

--*/
{
  EFI_STATUS                          Status;
  UINTN                               Index;
  EFI_INPUT_KEY                       CurrentKey;
    
  Key->UnicodeChar = 0;
  Key->ScanCode    = SCAN_NULL;
  
  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input, 
  // return the key and EFI_SUCCESS.
  //
  for ( Index = 0; Index < Private->CurrentNumberOfConsoles; Index ++ ) {
    Status  = Private->TextInList[Index]->ReadKeyStroke (
                        Private->TextInList[Index],
                        &CurrentKey
                        );
    if (!EFI_ERROR (Status)) {
      *Key = CurrentKey;
      return Status;
    }
  }
  
  return EFI_NOT_READY;
}

BOOLEAN
ConSpliterConssoleControlStdInLocked (
  VOID
  )
/*++

Routine Description:
  Return TRUE if StdIn is locked. The ConIn device on the virtual handle is
  the only device locked.

Arguments:
  NONE

Returns:
  TRUE  - StdIn locked
  FALSE - StdIn working normally

--*/
{
  return mConIn.PasswordEnabled;                
}

VOID 
EFIAPI
ConSpliterConsoleControlLockStdInEvent (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
/*++

Routine Description:
  This timer event will fire when StdIn is locked. It will check the key 
  sequence on StdIn to see if it matches the password. Any error in the
  password will cause the check to reset. As long a mConIn.PasswordEnabled is
  TRUE the StdIn splitter will not report any input.

Arguments:
  (Standard EFI_EVENT_NOTIFY)
  
Returns:
  None

--*/
{
  EFI_STATUS      Status;
  EFI_INPUT_KEY   Key;

  do {
    Status = ConSplitterTextInPrivateReadKeyStroke (&mConIn, &Key);
    if (!EFI_ERROR (Status)) {
      //
      // if it's an ENTER, match password
      //
      if ( (Key.UnicodeChar == CHAR_CARRIAGE_RETURN)
        && (Key.ScanCode == SCAN_NULL) ) {
        mConIn.PwdAttempt[mConIn.PwdIndex] = CHAR_NULL;
        if ( EfiStrCmp(mConIn.Password, mConIn.PwdAttempt) ) {
          //
          // Password not match
          //
          ConSplitterTextOutOutputString (&mConOut.TextOut, L"\n\rPassword not correct\n\r");
          mConIn.PwdIndex = 0;
        } else {
          //
          // Key matches password sequence
          //
          gBS->SetTimer (mConIn.LockEvent, TimerPeriodic, 0);
          mConIn.PasswordEnabled = FALSE;
          Status = EFI_NOT_READY;
        }
      } else {
        //
        // If it's not an ENTER, record the input
        //
        if ( mConIn.PwdIndex < (MAX_STD_IN_PASSWORD - 1) ) {
          if ( mConIn.PwdIndex == 0 ) {
            ConSplitterTextOutOutputString (&mConOut.TextOut, L"\n\r");
          }
          ConSplitterTextOutOutputString (&mConOut.TextOut, L"*");
          mConIn.PwdAttempt[mConIn.PwdIndex] = Key.UnicodeChar;
          mConIn.PwdIndex++;
        }
      }
    }
  } while (!EFI_ERROR (Status));
}

EFI_STATUS
EFIAPI
ConSpliterConsoleControlLockStdIn (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  IN  CHAR16                          *Password
  )
/*++

Routine Description:
  If Password is NULL unlock the password state variable and set the event 
  timer. If the Password is too big return an error. If the Password is valid
  Copy the Password and enable state variable and then arm the periodic timer

Arguments:

Returns:
  EFI_SUCCESS           - Lock the StdIn device
  EFI_INVALID_PARAMETER - Password is NULL
  EFI_OUT_OF_RESOURCES  - Buffer allocation to store the password fails

--*/
{
  if (Password == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (EfiStrLen (Password) >= MAX_STD_IN_PASSWORD) {
    //
    // Currently have a max password size
    //
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Save the password, initialize state variables and arm event timer
  //
  EfiStrCpy (mConIn.Password, Password);
  mConIn.PasswordEnabled  = TRUE;
  mConIn.PwdIndex         = 0;
  gBS->SetTimer (mConIn.LockEvent, TimerPeriodic, (10000 * 25));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSplitterTextInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *This,
  OUT EFI_INPUT_KEY                   *Key
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.
    If the ConIn is password locked make it look like no keystroke is availible

  Arguments:
    This   - Protocol instance pointer.
    Key    - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES        - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to 
                        hardware errors.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA       *Private;
 
  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return EFI_NOT_READY;
  }
  
  return ConSplitterTextInPrivateReadKeyStroke (Private, Key);
}

VOID 
EFIAPI
ConSplitterTextInWaitForKey (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
/*++

Routine Description:
  This event agregates all the events of the ConIn devices in the spliter.
  If the ConIn is password locked then return.
  If any events of physical ConIn devices are signaled, signal the ConIn
  spliter event. This will cause the calling code to call 
  ConSplitterTextInReadKeyStroke ().

Arguments:
  Event   - The Event assoicated with callback.
  Context - Context registered when Event was created.

Returns:
  None

--*/
{
  EFI_STATUS                          Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA       *Private;
  UINTN                               Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *)Context;
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return;
  }

  //
  // if any physical console input device has key input, signal the event. 
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = gBS->CheckEvent (Private->TextInList[Index]->WaitForKey);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
    }
  }
}



EFI_STATUS 
EFIAPI
ConSplitterSimplePointerReset (
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *This,
  IN  BOOLEAN                         ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES            - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
{
  EFI_STATUS                          Status;
  EFI_STATUS                          ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA       *Private;
  UINTN                               Index;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_SIMPLE_POINTER_THIS (This);
  
  if (Private->CurrentNumberOfPointers == 0) {
    return EFI_SUCCESS;
  }
  
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfPointers; Index++ ) {
    Status = Private->PointerList[Index]->Reset (
                                      Private->PointerList[Index], 
                                      ExtendedVerification
                                      );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }
  
  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterSimplePointerPrivateGetState (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN OUT EFI_SIMPLE_POINTER_STATE     *State
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This   - Protocol instance pointer.
    State  - 

  Returns:
    EFI_SUCCES        - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to 
                        hardware errors.

--*/
{
  EFI_STATUS                          Status;
  EFI_STATUS                          ReturnStatus;
  UINTN                               Index;
  EFI_SIMPLE_POINTER_STATE            CurrentState;

  State->RelativeMovementX = 0;
  State->RelativeMovementY = 0;
  State->RelativeMovementZ = 0;
  State->LeftButton        = FALSE;
  State->RightButton       = FALSE;
  
  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input, 
  // return the key and EFI_SUCCESS.
  //
  ReturnStatus = EFI_NOT_READY;
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++ ) {
          
    Status = Private->PointerList[Index]->GetState (
                                            Private->PointerList[Index], 
                                            &CurrentState
                                            );
    if (!EFI_ERROR (Status)) {
      ReturnStatus = EFI_SUCCESS;
      if (CurrentState.LeftButton) {
        State->LeftButton = TRUE;
      }
      if (CurrentState.RightButton) {
        State->RightButton = TRUE;
      }
      if (CurrentState.RelativeMovementX != 0 && Private->PointerList[Index]->Mode->ResolutionX != 0) {
        State->RelativeMovementX += (CurrentState.RelativeMovementX * (INT32)Private->SimplePointerMode.ResolutionX) / (INT32)Private->PointerList[Index]->Mode->ResolutionX;
      }
      if (CurrentState.RelativeMovementY != 0 && Private->PointerList[Index]->Mode->ResolutionY != 0) {
        State->RelativeMovementY += (CurrentState.RelativeMovementY * (INT32)Private->SimplePointerMode.ResolutionY) / (INT32)Private->PointerList[Index]->Mode->ResolutionY;
      }
      if (CurrentState.RelativeMovementZ != 0 && Private->PointerList[Index]->Mode->ResolutionZ != 0) {
        State->RelativeMovementZ += (CurrentState.RelativeMovementZ * (INT32)Private->SimplePointerMode.ResolutionZ) / (INT32)Private->PointerList[Index]->Mode->ResolutionZ;
      }
    }
  }
  
  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterSimplePointerGetState (
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *This,
  IN OUT EFI_SIMPLE_POINTER_STATE   *State
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.
    If the ConIn is password locked make it look like no keystroke is availible

  Arguments:
    This   - Protocol instance pointer.
    State  - 

  Returns:
    EFI_SUCCES        - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to 
                        hardware errors.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA  *Private;
 
  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_SIMPLE_POINTER_THIS (This);
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return EFI_NOT_READY;
  }
  
  return ConSplitterSimplePointerPrivateGetState (Private, State);
}

VOID 
EFIAPI
ConSplitterSimplePointerWaitForInput (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
/*++

Routine Description:
  This event agregates all the events of the ConIn devices in the spliter.
  If the ConIn is password locked then return.
  If any events of physical ConIn devices are signaled, signal the ConIn
  spliter event. This will cause the calling code to call 
  ConSplitterTextInReadKeyStroke ().

Arguments:
  Event   - The Event assoicated with callback.
  Context - Context registered when Event was created.

Returns:
  None

--*/
{
  EFI_STATUS                          Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA       *Private;
  UINTN                               Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *)Context;
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return;
  }

  //
  // if any physical console input device has key input, signal the event. 
  //
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {
    Status = gBS->CheckEvent (Private->PointerList[Index]->WaitForInput);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
    }
  }
}


EFI_STATUS 
EFIAPI
ConSplitterTextOutReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         ExtendedVerification
  )
/*++

  Routine Description:
    Reset the text output device hardware and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform more exhaustive verfication 
                           operation of the device during reset.

  Returns:
    EFI_SUCCES        - The text output device was reset.
    EFI_DEVICE_ERROR  - The text output device is not functioning correctly and
                        could not be reset.

--*/
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++) {
          
    if (Private->TextOutList[Index].TextOutEnabled) {

      Status = Private->TextOutList[Index].TextOut->Reset (
                                          Private->TextOutList[Index].TextOut,
                                          ExtendedVerification
                                          );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  //
  //reset the background and keep the foreground unchanged
  //
  Private->TextOutMode.Attribute &= 0x0f;
  Status = DevNullTextOutSetMode (Private, 0);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
/*++

  Routine Description:
    Write a Unicode string to the output device.

  Arguments:
    This    - Protocol instance pointer.
    String  - The NULL-terminated Unicode string to be displayed on the output
              device(s). All output devices must also support the Unicode 
              drawing defined in this file.

  Returns:
    EFI_SUCCES        - The string was output to the device.
    EFI_DEVICE_ERROR  - The device reported an error while attempting to output
                         the text.
    EFI_UNSUPPORTED        - The output device's mode is not currently in a 
                              defined text mode.
    EFI_WARN_UNKNOWN_GLYPH - This warning code indicates that some of the 
                              characters in the Unicode string could not be 
                              rendered and were skipped.

--*/
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
 
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++ ) {
          
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->OutputString (
                                          Private->TextOutList[Index].TextOut,
                                          WString
                                          );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
  
  Status = DevNullTextOutOutputString (Private, WString);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
/*++

  Routine Description:
    Verifies that all characters in a Unicode string can be output to the 
    target device.

  Arguments:
    This    - Protocol instance pointer.
    String  - The NULL-terminated Unicode string to be examined for the output
               device(s).

  Returns:
    EFI_SUCCES      - The device(s) are capable of rendering the output string.
    EFI_UNSUPPORTED - Some of the characters in the Unicode string cannot be 
                       rendered by one or more of the output devices mapped 
                       by the EFI handle.

--*/
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++ ) {
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->TestString (
                                      Private->TextOutList[Index].TextOut,
                                      WString
                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
  
  //
  // There is no DevNullTextOutTestString () since a Unicode buffer would 
  // always return EFI_SUCCESS. 
  // ReturnStatus will be EFI_SUCCESS if no consoles are present
  //
  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  )
/*++

  Routine Description:
    Returns information for an available text mode that the output device(s)
    supports.

  Arguments:
    This       - Protocol instance pointer.
    ModeNumber - The mode number to return information on.
    Columns, Rows - Returns the geometry of the text output device for the
                    requested ModeNumber.

  Returns:
    EFI_SUCCES       - The requested mode information was returned.
    EFI_DEVICE_ERROR - The device had an error and could not 
                       complete the request.
    EFI_UNSUPPORTED - The mode number was not valid.

--*/
{
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;                        

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);    

  if ( (INT32)ModeNumber >= This->Mode->MaxMode ) {
    return EFI_UNSUPPORTED;
  }

  *Columns = Private->TextOutQueryData[ModeNumber].Columns;
  *Rows    = Private->TextOutQueryData[ModeNumber].Rows;
  
  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber
  )
/*++

  Routine Description:
    Sets the output device(s) to a specified mode.

  Arguments:
    This       - Protocol instance pointer.
    ModeNumber - The mode number to set.

  Returns:
    EFI_SUCCES       - The requested text mode was set.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The mode number was not valid.

--*/
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  INT32                               *TextOutModeMap;
  EFI_STATUS                          ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  if ( (INT32)ModeNumber >= This->Mode->MaxMode ) {
    return EFI_UNSUPPORTED;
  }

  //
  // If the mode is being set to the curent mode, then just clear the screen and return.
  //
  if (Private->TextOutMode.Mode == (INT32)ModeNumber) {
    return ConSplitterTextOutClearScreen (This);
  }

  //
  // return the worst status met
  //
  TextOutModeMap  = Private->TextOutModeMap + Private->TextOutListCount * ModeNumber;
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++ ) {
          
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->SetMode (
                                          Private->TextOutList[Index].TextOut,
                                          TextOutModeMap[Index]
                                          );
      //
      // If this console device is based on a UGA device, then sync up the bitmap from
      // the UGA splitter and reclear the text portion of the display in the new mode.
      //
      if ( Private->TextOutList[Index].UgaDraw ) { 
//        DevNullUgaSync (Private, Private->TextOutList[Index].UgaDraw);
        Private->TextOutList[Index].TextOut->ClearScreen (Private->TextOutList[Index].TextOut);
      }

      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }  
  
  //
  // The DevNull Console will support any possible mode as it allocates memory
  //
  Status = DevNullTextOutSetMode (Private, ModeNumber);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Attribute
  )
/*++

  Routine Description:
    Sets the background and foreground colors for the OutputString () and
    ClearScreen () functions.

  Arguments:
    This      - Protocol instance pointer.
    Attribute - The attribute to set. Bits 0..3 are the foreground color, and
                bits 4..6 are the background color. All other bits are undefined
                and must be zero. The valid Attributes are defined in this file.

  Returns:
    EFI_SUCCES       - The attribute was set.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The attribute requested is not defined.

--*/  
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++ ) {
          
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->SetAttribute (
                                      Private->TextOutList[Index].TextOut,
                                      Attribute
                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
  
  Private->TextOutMode.Attribute = (INT32)Attribute;

  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This
  )
/*++

  Routine Description:
    Clears the output device(s) display to the currently selected background 
    color.

  Arguments:
    This      - Protocol instance pointer.

  Returns:
    EFI_SUCCES       - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode.

--*/  
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++ ) {
          
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->ClearScreen (
                                      Private->TextOutList[Index].TextOut
                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
  
  Status = DevNullTextOutClearScreen (Private);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  )
/*++

  Routine Description:
    Sets the current coordinates of the cursor position

  Arguments:
    This        - Protocol instance pointer.
    Column, Row - the position to set the cursor to. Must be greater than or
                  equal to zero and less than the number of columns and rows
                  by QueryMode ().

  Returns:
    EFI_SUCCES       - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode, or the 
                       cursor position is invalid for the current mode.

--*/  
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;
  UINTN                               MaxColumn, MaxRow;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  MaxColumn = Private->TextOutQueryData[Private->TextOutMode.Mode].Columns;
  MaxRow    = Private->TextOutQueryData[Private->TextOutMode.Mode].Rows;

  if (Column >= MaxColumn || Row >= MaxRow) {
    return EFI_UNSUPPORTED;
  }

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++ ) {
          
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->SetCursorPosition(
                                      Private->TextOutList[Index].TextOut,
                                      Column, 
                                      Row
                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
  
  DevNullTextOutSetCursorPosition (Private, Column, Row);

  return ReturnStatus;
}

EFI_STATUS 
EFIAPI
ConSplitterTextOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         Visible
  )
/*++

  Routine Description:
    Makes the cursor visible or invisible

  Arguments:
    This    - Protocol instance pointer.
    Visible - If TRUE, the cursor is set to be visible. If FALSE, the cursor is
              set to be invisible.

  Returns:
    EFI_SUCCES       - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and could not complete the 
                        request, or the device does not support changing
                        the cursor mode.
    EFI_UNSUPPORTED - The output device is not in a valid text mode.

--*/  
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; 
        Index < Private->CurrentNumberOfConsoles; Index++ ) {
          
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->EnableCursor (
                                      Private->TextOutList[Index].TextOut,
                                      Visible
                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  DevNullTextOutEnableCursor (Private, Visible);

  return ReturnStatus;
}
