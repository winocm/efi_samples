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

  WinNtUgaDriver.c

Abstract:

  This file implements the EFI 1.1 Device Driver model requirements for UGA

  UGA is short hand for Universal Graphics Abstraction protocol.

  This file is a verision of UgaIo the uses WinNtThunk system calls as an IO 
  abstraction. For a PCI device WinNtIo would be replaced with
  a PCI IO abstraction that abstracted a specific PCI device. 

--*/

#include "WinNtUga.h"

EFI_DRIVER_BINDING_PROTOCOL gWinNtUgaDriverBinding = {
  WinNtUgaDriverBindingSupported,
  WinNtUgaDriverBindingStart,
  WinNtUgaDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT  (WinNtUgaInitialize);

EFI_STATUS
WinNtUgaInitialize (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  )
/*++

Routine Description:

  Intialize Win32 windows to act as EFI SimpleTextOut and SimpleTextIn windows. . 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gWinNtUgaDriverBinding, 
           ImageHandle,
           &gWinNtUgaComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
WinNtUgaDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS               Status;
  EFI_WIN_NT_IO_PROTOCOL   *WinNtIo;

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

  Status = WinNtUgaSupported (WinNtIo);

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
WinNtUgaDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_WIN_NT_IO_PROTOCOL  *WinNtIo;                
  EFI_STATUS              Status;
  UGA_PRIVATE_DATA        *Private;


  //
  // Grab the protocols we need
  //
  Status = gBS->OpenProtocol (
                  Handle,           
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate Private context data for SGO inteface.
  //
  Private = NULL;
  Status = gBS->AllocatePool (
                  EfiBootServicesData, sizeof(UGA_PRIVATE_DATA), &Private
                  );
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // Set up context record
  //
  Private->Signature        = UGA_PRIVATE_DATA_SIGNATURE;
  Private->Handle           = Handle;
  Private->WinNtThunk       = WinNtIo->WinNtThunk;

  Private->ControllerNameTable = NULL;

  EfiLibAddUnicodeString (
    "eng", 
    gWinNtUgaComponentName.SupportedLanguages, 
    &Private->ControllerNameTable, 
    WinNtIo->EnvString
    );

  Private->WindowName = WinNtIo->EnvString;

  Status = WinNtUgaConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Publish the Uga interface to the world
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,         
                  &gEfiUgaDrawProtocolGuid,       &Private->UgaDraw,
                  &gEfiUgaIoProtocolGuid,         &Private->UgaIo,
                  &gEfiSimpleTextInProtocolGuid,  &Private->SimpleTextIn,
                  NULL
                  );

Done:
  if (EFI_ERROR (Status)) {

    gBS->CloseProtocol (
           Handle,           
           &gEfiWinNtIoProtocolGuid,  
           This->DriverBindingHandle,   
           Handle
           );

    if (Private != NULL) {
      //
      // On Error Free back private data
      //
      if (Private->ControllerNameTable != NULL) {
        EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);
      }

      gBS->FreePool (Private);
    }
  }
  return Status;
}


EFI_STATUS
WinNtUgaDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_UGA_DRAW_PROTOCOL *UgaDraw;                  
  EFI_STATUS            Status;
  UGA_PRIVATE_DATA      *Private;

  Status = gBS->OpenProtocol (
                  Handle,           
                  &gEfiUgaDrawProtocolGuid,  
                  &UgaDraw,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the UGA interface does not exist the driver is not started
    //
    return EFI_NOT_STARTED;
  }

  //
  // Get our private context information
  //
  Private = UGA_DRAW_PRIVATE_DATA_FROM_THIS (UgaDraw); 

  //
  // Remove the SGO interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle, 
                  &gEfiUgaDrawProtocolGuid,        &Private->UgaDraw,
                  &gEfiUgaIoProtocolGuid,          &Private->UgaIo,
                  &gEfiSimpleTextInProtocolGuid,   &Private->SimpleTextIn,
                  NULL
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Shutdown the hardware
    //
    Status = WinNtUgaDestructor (Private);
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }

    gBS->CloseProtocol (
           Handle,           
           &gEfiWinNtIoProtocolGuid,  
           This->DriverBindingHandle,   
           Handle
           );

    //
    // Free our instance data
    //
    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

    gBS->FreePool (Private);

  }
  return Status;
}


UINTN
Atoi (
  CHAR16  *String
  )
/*++

Routine Description:

  Convert a unicode string to a UINTN

Arguments:

  String - Unicode string.

Returns: 

  UINTN of the number represented by String.  

--*/
{
  UINTN   Number;
  CHAR16  *Str;

  //
  // skip preceeding white space
  //
  Str = String;
  while ((*Str) && (*Str == ' ' || *Str == '"')) {
    Str++;
  }

  //
  // Convert ot a Number
  //
  Number = 0;
  while (*Str != '\0') {
      if ((*Str >= '0') && (*Str <= '9')) {
          Number = (Number * 10) + *Str - '0';
      } else {
          break;
      }
      Str++;
  }

  return Number;
}
