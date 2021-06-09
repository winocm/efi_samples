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

  Ps2Mouse.c

Abstract:

  PS/2 Mouse driver. Routines that interacts with callers,
  conforming to EFI driver model

Revision History:

--*/

#include "Ps2Mouse.h"
#include "CommPs2.h"

//
// Function prototypes
//
EFI_STATUS
EFIAPI
PS2MouseDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
PS2MouseDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
PS2MouseDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN EFI_HANDLE                    Controller,
  IN UINTN                         NumberOfChildren,
  IN EFI_HANDLE                    *ChildHandleBuffer
  );

EFI_STATUS
MouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  );

EFI_STATUS
MouseGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN OUT EFI_SIMPLE_POINTER_STATE   *State
  );

VOID 
EFIAPI
MouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

void
PollMouse(
  IN EFI_EVENT  Event,
  IN void       *Context
  );

EFI_STATUS
In8042Data (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN OUT UINT8            *Data
  );

//
// DriverBinding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gPS2MouseDriver = {
  PS2MouseDriverSupported,
  PS2MouseDriverStart,
  PS2MouseDriverStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT(PS2MouseDriverEntryPoint)

EFI_STATUS
EFIAPI
PS2MouseDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Driver entry point. Installs DriverBinding protocol.

Arguments:

  ImageHandle
  SystemTable

Returns:

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gPS2MouseDriver, 
           ImageHandle,
           &gPs2MouseComponentName,
           NULL,
           NULL
           );
}


EFI_STATUS
EFIAPI
PS2MouseDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

Routine Description:

  ControllerDriver Protocol Method

Arguments:

Returns:

--*/

{
  EFI_STATUS            Status;
  EFI_ISA_IO_PROTOCOL   *IsaIo;

  Status = EFI_SUCCESS;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Use the ISA I/O Protocol to see if Controller is the Keyboard controller
  //
  switch (IsaIo->ResourceList->Device.HID) {
    case EISA_PNP_ID(0xF03): 
      //
      // Microsoft PS/2 style mouse
      //
    case EISA_PNP_ID(0xF13): 
      //
      // PS/2 Port for PS/2-style Mice
      //
      break;
    case EISA_PNP_ID(0x303): 
      //
      // IBM Enhanced (101/102-key, PS/2 mouse support)
      //
      if (IsaIo->ResourceList->Device.UID == 1) {
        break;
      }
    default:
      Status = EFI_UNSUPPORTED;
      break;
  }
        
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
                Controller,
                &gEfiIsaIoProtocolGuid,
                This->DriverBindingHandle,
                Controller
                );

  return Status;
}


EFI_STATUS
EFIAPI
PS2MouseDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

Routine Description:

Arguments:

Returns:

--*/

{
  EFI_STATUS                Status;
  EFI_STATUS                Status1;
  EFI_ISA_IO_PROTOCOL       *IsaIo;
  PS2_MOUSE_DEV             *MouseDev; 
  UINT8                     Data;
  EFI_TPL                   OldTpl;

    
  MouseDev = NULL;
    
  //
  // Get the ISA I/O Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol(
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR(Status)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Raise TPL to avoid keyboard operation impact
  //
  OldTpl = gBS -> RaiseTPL(EFI_TPL_NOTIFY);

  //
  // Allocate private data
  //
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(PS2_MOUSE_DEV),
                  (VOID **)&MouseDev
                  );

  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
  EfiZeroMem(MouseDev, sizeof(PS2_MOUSE_DEV));

  //
  // Setup the device instance
  //
  MouseDev->Signature         = PS2_MOUSE_DEV_SIGNATURE;

  MouseDev->Handle            = Controller;

  MouseDev->SampleRate        = SSR_20;
  MouseDev->Resolution        = CMR4;
  MouseDev->Scaling           = SF1;
  MouseDev->DataPackageSize   = 3;

  MouseDev->IsaIo             = IsaIo;

  // Resolution = 4 counts/mm
  MouseDev->Mode.ResolutionX  = 4;
  MouseDev->Mode.ResolutionY  = 4;
  MouseDev->Mode.LeftButton   = TRUE;
  MouseDev->Mode.RightButton  = TRUE;
  

  MouseDev->SimplePointerProtocol.Reset     = MouseReset;
  MouseDev->SimplePointerProtocol.GetState  = MouseGetState;
  MouseDev->SimplePointerProtocol.Mode      = &(MouseDev->Mode);
  
    
  //
  // Initialize keyboard controller if necessary
  //
  IsaIo->Io.Read (IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Data);
  if ( (Data & KBC_SYSF) != KBC_SYSF  ) {
    
    Status = KbcSelfTest(IsaIo);
    if (EFI_ERROR(Status)) {
       goto ErrorExit;
    }
    
  }

  KbcEnableAux(IsaIo);


  //
  // Reset the mouse
  //
  Status = MouseDev->SimplePointerProtocol.Reset (&MouseDev->SimplePointerProtocol, TRUE);
  if (EFI_ERROR(Status)) {
    // mouse not connect
    Status = EFI_SUCCESS;
    goto ErrorExit;
  }

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent (
        EFI_EVENT_NOTIFY_WAIT,
        EFI_TPL_NOTIFY,
        MouseWaitForInput,
        MouseDev,
        &((MouseDev->SimplePointerProtocol).WaitForInput)
        );

  if (EFI_ERROR(Status)) {
    goto ErrorExit;
  }
  
  //
  // Setup a periodic timer, used to poll mouse state
  //
  Status = gBS->CreateEvent(
      EFI_EVENT_TIMER|EFI_EVENT_NOTIFY_SIGNAL,
      EFI_TPL_NOTIFY,
      PollMouse,
      MouseDev,
      &MouseDev->TimerEvent
      );

  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  //
  // Start timer to poll mouse (100 samples per second)
  //
  Status = gBS->SetTimer( MouseDev->TimerEvent, TimerPeriodic, 100000 ) ;

  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  MouseDev->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gPs2MouseComponentName.SupportedLanguages, 
    &MouseDev->ControllerNameTable, 
    L"PS/2 Mouse Device"
    );

  //
  // Install protocol interfaces for the keyboard device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
        &Controller,
        &gEfiSimplePointerProtocolGuid,        &MouseDev->SimplePointerProtocol,
        NULL
        );
    
  if (EFI_ERROR(Status)) {
    goto ErrorExit;
  }
  
  gBS -> RestoreTPL(OldTpl);
  return Status;

ErrorExit:  
  KbcDisableAux(IsaIo);
  
  if ( MouseDev && MouseDev->SimplePointerProtocol.WaitForInput ) {
    gBS->CloseEvent(MouseDev->SimplePointerProtocol.WaitForInput);
  }
  
  if ( MouseDev && MouseDev->TimerEvent ) {
    gBS->CloseEvent(MouseDev->TimerEvent);
  }

  if ( MouseDev && MouseDev->ControllerNameTable ) {
    EfiLibFreeUnicodeStringTable(MouseDev->ControllerNameTable);
  }
  
  //
  // Since there will be no timer handler for mouse input any more,
  // exhaust input data just in case there is still mouse data left
  //

  Status1 = EFI_SUCCESS;
  while (!EFI_ERROR (Status1)) {
    Status1 = In8042Data (IsaIo, &Data);
  }

  if ( MouseDev ) {
    gBS->FreePool(MouseDev);
  }
  
  gBS->CloseProtocol (
              Controller,
              &gEfiIsaIoProtocolGuid,
              This->DriverBindingHandle,
              Controller
              );

  gBS -> RestoreTPL(OldTpl);
  return Status;
}


EFI_STATUS
EFIAPI
PS2MouseDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS                    Status;
  EFI_SIMPLE_POINTER_PROTOCOL   *SimplePointerProtocol;
  PS2_MOUSE_DEV                 *MouseDev;
  UINT8                         Data;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimplePointerProtocolGuid,
                  (VOID **)&SimplePointerProtocol,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  MouseDev = PS2_MOUSE_DEV_FROM_THIS (SimplePointerProtocol);
  
  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimplePointerProtocolGuid,    
                  &MouseDev->SimplePointerProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  
  //
  // Disable mouse on keyboard controller
  //
  KbcDisableAux(MouseDev->IsaIo);

  //
  // Cancel mouse data polling timer, close timer event
  //
  gBS->SetTimer(MouseDev->TimerEvent, TimerCancel, 0);
  gBS->CloseEvent(MouseDev->TimerEvent);
  
  //
  // Since there will be no timer handler for mouse input any more,
  // exhaust input data just in case there is still mouse data left
  //

  Status = EFI_SUCCESS;
  while (!EFI_ERROR (Status)) {
    Status = In8042Data (MouseDev->IsaIo, &Data);
  }

  gBS->CloseEvent(MouseDev->SimplePointerProtocol.WaitForInput);
  EfiLibFreeUnicodeStringTable (MouseDev->ControllerNameTable);
  gBS->FreePool(MouseDev);

  return EFI_SUCCESS;
}

EFI_STATUS
MouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  EFI_STATUS            Status;
  PS2_MOUSE_DEV         *MouseDev;
  EFI_TPL               OldTpl;
  BOOLEAN               KeyboardEnable;
  UINT8                 Data;


  KeyboardEnable = FALSE;

  //
  // Raise TPL to avoid keyboard operation impact
  //
  OldTpl = gBS -> RaiseTPL(EFI_TPL_NOTIFY);

  MouseDev = PS2_MOUSE_DEV_FROM_THIS (This);

  EfiZeroMem (&MouseDev->State, sizeof (EFI_SIMPLE_POINTER_STATE));
  MouseDev->StateChanged = FALSE;

  //
  // Exhaust input data
  //
  Status = EFI_SUCCESS;
  while (!EFI_ERROR (Status)) {
    Status = In8042Data (MouseDev->IsaIo, &Data);
  }

  CheckKbStatus(MouseDev->IsaIo, &KeyboardEnable);

  KbcDisableKb(MouseDev->IsaIo);

  MouseDev->IsaIo->Io.Read (MouseDev->IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Data);
  
  //
  // if there's data block on KBC data port, read it out
  //
  if ( (Data & KBC_OUTB) == KBC_OUTB ) {
      MouseDev->IsaIo->Io.Read (MouseDev->IsaIo, EfiIsaIoWidthUint8, KBC_DATA_PORT, 1, &Data);
  }

  //
  // Send mouse reset command and set mouse default configure
  //
  Status = PS2MouseReset ( MouseDev->IsaIo );
  if ( EFI_ERROR( Status ) ) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }
  
  Status = PS2MouseSetSampleRate ( MouseDev->IsaIo, MouseDev->SampleRate );
  if ( EFI_ERROR( Status ) ) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  Status = PS2MouseSetResolution ( MouseDev->IsaIo, MouseDev->Resolution );
  if ( EFI_ERROR( Status ) ) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  Status = PS2MouseSetScaling ( MouseDev->IsaIo, MouseDev->Scaling );
  if ( EFI_ERROR( Status ) ) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  Status = PS2MouseEnable ( MouseDev->IsaIo );
  if ( EFI_ERROR( Status ) ) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

Exit:
  gBS -> RestoreTPL(OldTpl);

  if (KeyboardEnable) {
    KbcEnableKb(MouseDev->IsaIo);
  }
  
  return Status;
}

EFI_STATUS
MouseGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN OUT EFI_SIMPLE_POINTER_STATE   *State
  )
{
  PS2_MOUSE_DEV     *MouseDev;
  EFI_TPL           OldTpl;

  MouseDev = PS2_MOUSE_DEV_FROM_THIS (This);

  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (MouseDev->StateChanged == FALSE) {
    return EFI_NOT_READY;
  }
  
  OldTpl = gBS -> RaiseTPL(EFI_TPL_NOTIFY);
  EfiCopyMem( State, &(MouseDev->State), sizeof( EFI_SIMPLE_POINTER_STATE ) );

  //
  // clear mouse state
  //
  MouseDev->State.RelativeMovementX = 0;
  MouseDev->State.RelativeMovementY = 0;
  MouseDev->State.RelativeMovementZ = 0;
  MouseDev->StateChanged = FALSE;
  gBS -> RestoreTPL(OldTpl);

  return EFI_SUCCESS;
}       

VOID 
EFIAPI
MouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
/*++

Routine Description:

  Event notification function for SIMPLE_POINTER.WaitForInput event
  Signal the event if there is input from mouse

Arguments:

Returns:

--*/
{
  PS2_MOUSE_DEV     *MouseDev;

  MouseDev = (PS2_MOUSE_DEV*)Context;
  
  //
  // Someone is waiting on the mouse event, if there's
  // input from mouse, signal the event
  //
  if (MouseDev->StateChanged) {
    gBS->SignalEvent(Event);
  }
  
}

void
PollMouse(
  IN EFI_EVENT  Event,
  IN void       *Context
  )
{
  PS2_MOUSE_DEV     *MouseDev;

  MouseDev = (PS2_MOUSE_DEV*)Context;

  //
  // Polling mouse packet data
  //
  PS2MouseGetPacket(MouseDev);
}    

