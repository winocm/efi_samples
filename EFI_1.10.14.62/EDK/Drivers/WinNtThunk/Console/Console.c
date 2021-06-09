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

  Console.c

Abstract:

  Console based on Win32 APIs. 

--*/

#include "Console.h"

EFI_STATUS
WinNtConsoleDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
WinNtConsoleDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
WinNtConsoleDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gWinNtConsoleDriverBinding = {
  WinNtConsoleDriverBindingSupported,
  WinNtConsoleDriverBindingStart,
  WinNtConsoleDriverBindingStop,
  0x10,
  NULL,
  NULL
};


EFI_DRIVER_ENTRY_POINT(InitializeWinNtConsole)

EFI_STATUS
InitializeWinNtConsole (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  )
/*++

Routine Description:

  Intialize Win32 windows to act as EFI SimpleTextOut and SimpleTextIn windows
  following the EFI 1.1 driver model. 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gWinNtConsoleDriverBinding, 
           ImageHandle,
           &gWinNtConsoleComponentName,
           NULL,
           NULL
           );
}


EFI_STATUS
WinNtConsoleDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                        Status;
  EFI_WIN_NT_IO_PROTOCOL *WinNtIo;
  
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Make sure that the WinNt Thunk Protocol is valid 
  //
  Status = EFI_UNSUPPORTED;
  if (WinNtIo->WinNtThunk->Signature == EFI_WIN_NT_THUNK_PROTOCOL_SIGNATURE) {

    //
    // Check the GUID to see if this is a handle type the driver supports
    //
    if (EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtConsoleGuid)) {
      Status = EFI_SUCCESS;
    }
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Handle,   
         &gEfiWinNtIoProtocolGuid,  
         This->DriverBindingHandle,   
         Handle   
         );

  return Status;
}


EFI_STATUS
WinNtConsoleDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                        Status;
  EFI_WIN_NT_IO_PROTOCOL *WinNtIo;
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA   *Private;

  //
  // Grab the IO abstraction we need to get any work done
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

	Status = gBS->AllocatePool(
                  EfiBootServicesData,
									sizeof (WIN_NT_SIMPLE_TEXT_PRIVATE_DATA), 
                  &Private
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  EfiZeroMem (Private, sizeof (WIN_NT_SIMPLE_TEXT_PRIVATE_DATA));

  Private->Signature  = WIN_NT_SIMPLE_TEXT_PRIVATE_DATA_SIGNATURE;
  Private->Handle     = Handle;
  Private->WinNtIo    = WinNtIo;
  Private->WinNtThunk = WinNtIo->WinNtThunk;

  WinNtSimpleTextOutOpenWindow (Private);
  WinNtSimpleTextInAttachToWindow (Private);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,              
                  &gEfiSimpleTextOutProtocolGuid, &Private->SimpleTextOut,
                  &gEfiSimpleTextInProtocolGuid,  &Private->SimpleTextIn,
                  NULL
                  );
  if (!EFI_ERROR (Status)) {
    return Status;
  }
Done:
  gBS->CloseProtocol (
         Handle,   
         &gEfiWinNtIoProtocolGuid,  
         This->DriverBindingHandle,   
         Handle   
         );
  if (Private != NULL) {

    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

    if (Private->NtOutHandle != NULL) {
      Private->WinNtThunk->CloseHandle (Private->NtOutHandle);
    }

    if (Private->SimpleTextIn.WaitForKey != NULL) {
      gBS->CloseEvent (Private->SimpleTextIn.WaitForKey);
    }

    gBS->FreePool (Private);
  }
  return Status;
}


EFI_STATUS
WinNtConsoleDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
{
  EFI_SIMPLE_TEXT_OUT_PROTOCOL      *SimpleTextOut;
  EFI_STATUS                        Status;
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA   *Private;

  //
  // Kick people off our interface???
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiSimpleTextOutProtocolGuid,  
                  &SimpleTextOut,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS (SimpleTextOut);

  ASSERT (Private->Handle == Handle);

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Handle,              
                  &gEfiSimpleTextOutProtocolGuid, &Private->SimpleTextOut,
                  &gEfiSimpleTextInProtocolGuid,  &Private->SimpleTextIn,
                  NULL
                  );
  if (!EFI_ERROR (Status)) {

    //
    // Shut down our device
    //
    Status = gBS->CloseProtocol (
                    Handle, 
                    &gEfiWinNtIoProtocolGuid, 
                    This->DriverBindingHandle, 
                    Handle
                    );

    Status = gBS->CloseEvent (Private->SimpleTextIn.WaitForKey);
    ASSERT_EFI_ERROR (Status);

    Private->WinNtThunk->CloseHandle (Private->NtOutHandle);
    //
    // DO NOT close Private->NtInHandle. It points to StdIn and not 
    //  the Private->NtOutHandle is StdIn and should not be closed!
    //

    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

    gBS->FreePool (Private);
  }
  return Status;
}

