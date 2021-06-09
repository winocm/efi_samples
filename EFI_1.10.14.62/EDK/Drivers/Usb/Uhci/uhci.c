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

    Uhci.c
    
Abstract: 
    

Revision History
--*/

#include "Efi.h"
#include "EfiDriverLib.h"

#include "uhci.h"

//
// Prototypes
// Driver model protocol interface
//

EFI_STATUS
UHCIDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
UHCIDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
UHCIDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
UHCIDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );
  
  

//
// UHCI interface functions
//

EFI_STATUS
UHCIReset(
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT16                 Attributes
  );
  
EFI_STATUS
UHCIGetState (
  IN  EFI_USB_HC_PROTOCOL    *This,
  OUT EFI_USB_HC_STATE      *State
  );

EFI_STATUS
UHCISetState (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  EFI_USB_HC_STATE       State
  );
  
EFI_STATUS
UHCIControlTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  BOOLEAN                IsSlowDevice,
  IN  UINT8                  MaximumPacketLength,
  IN  EFI_USB_DEVICE_REQUEST *Request,
  IN  EFI_USB_DATA_DIRECTION TransferDirection,
  IN  OUT VOID               *Data                 OPTIONAL,
  IN  OUT UINTN              *DataLength           OPTIONAL,  
  IN  UINTN                  TimeOut,
  OUT UINT32                 *TransferResult
  );
  
EFI_STATUS
UHCIBulkTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  UINT8                  EndPointAddress,
  IN  UINT8                  MaximumPacketLength,
  IN OUT VOID                *Data,
  IN OUT UINTN               *DataLength,  
  IN OUT UINT8               *DataToggle,
  IN  UINTN                  TimeOut,
  OUT UINT32                 *TransferResult
  );

EFI_STATUS
UHCIAsyncInterruptTransfer (
  IN  EFI_USB_HC_PROTOCOL               *This,
  IN  UINT8                             DeviceAddress,
  IN  UINT8                             EndPointAddress,
  IN  BOOLEAN                           IsSlowDevice,
  IN  UINT8                             MaxiumPacketLength,
  IN  BOOLEAN                           IsNewTransfer,
  IN OUT UINT8                          *DataToggle      OPTIONAL,
  IN  UINTN                             PollingInterval  OPTIONAL,
  IN  UINTN                             DataLength       OPTIONAL,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK   CallBackFunction OPTIONAL,
  IN  VOID                              *Context         OPTIONAL  
  );
  
EFI_STATUS
UHCISyncInterruptTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  UINT8                  EndPointAddress,
  IN  BOOLEAN                IsSlowDevice,
  IN  UINT8                  MaximumPacketLength,
  IN OUT VOID                *Data,
  IN OUT UINTN               *DataLength,
  IN OUT UINT8               *DataToggle,
  IN  UINTN                  TimeOut,
  OUT UINT32                 *TransferResult
  );

EFI_STATUS
UHCIIsochronousTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  UINT8                  EndPointAddress,
  IN  UINT8                  MaximumPacketLength,
  IN OUT VOID                *Data,
  IN OUT UINTN               DataLength,
  OUT UINT32                 *TransferResult
  );
  
EFI_STATUS
UHCIAsyncIsochronousTransfer (
  IN  EFI_USB_HC_PROTOCOL               *This,
  IN  UINT8                             DeviceAddress,
  IN  UINT8                             EndPointAddress,
  IN  UINT8                             MaximumPacketLength,
  IN OUT VOID                           *Data,
  IN  UINTN                             DataLength,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK   IsochronousCallBack,
  IN  VOID                              *Context   OPTIONAL
  );

EFI_STATUS
UHCIGetRootHubPortNumber (
  IN  EFI_USB_HC_PROTOCOL    *This,
  OUT UINT8                  *PortNumber
  );

EFI_STATUS
UHCIGetRootHubPortStatus (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  PortNumber,
  OUT EFI_USB_PORT_STATUS    *PortStatus
  );

EFI_STATUS
UHCISetRootHubPortFeature (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  PortNumber,
  IN  EFI_USB_PORT_FEATURE   PortFeature
  );

EFI_STATUS
UHCIClearRootHubPortFeature (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  PortNumber,
  IN  EFI_USB_PORT_FEATURE   PortFeature
  );   

//
// Asynchronous interrupt transfer monitor function
//  
VOID
MonitorInterruptTrans(
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  );                  

  
//
// UHCI Driver Global Variables
//

EFI_DRIVER_BINDING_PROTOCOL gUhciDriverBinding = {
  UHCIDriverBindingSupported,
  UHCIDriverBindingStart,
  UHCIDriverBindingStop,
  0x10,
  NULL,
  NULL
};


EFI_DRIVER_ENTRY_POINT(UHCIDriverEntryPoint)

EFI_STATUS
UHCIDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/       
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gUhciDriverBinding, 
           ImageHandle,
           &gUhciComponentName,
           NULL,
           NULL
         );
}

EFI_STATUS
UHCIDriverBindingSupported (
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
  EFI_STATUS              OpenStatus;
  EFI_STATUS              Status;
  EFI_PCI_IO_PROTOCOL     *PciIo;
  USB_CLASSC              UsbClassCReg;
  
  //
  // Test whether there is PCI IO Protocol attached on the controller handle.
  //
  OpenStatus = gBS->OpenProtocol (
                          Controller,       
                          &gEfiPciIoProtocolGuid, 
                          &PciIo,
                          This->DriverBindingHandle,   
                          Controller,   
                          EFI_OPEN_PROTOCOL_BY_DRIVER
                          );
  if (EFI_ERROR (OpenStatus)) {
    return OpenStatus;
  }
  
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8,
                            CLASSC,sizeof(USB_CLASSC) / sizeof(UINT8),
                            &UsbClassCReg
                            );
  if (EFI_ERROR(Status)) {
    gBS->CloseProtocol (
            Controller,  
            &gEfiPciIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
            );
    return EFI_UNSUPPORTED;
  }
  
  //
  // Test whether the controller belongs to UHCI type
  //
  if ((UsbClassCReg.BaseCode != PCI_CLASSC_BASE_CLASS_SERIAL) 
        || (UsbClassCReg.SubClassCode != PCI_CLASSC_SUBCLASS_SERIAL_USB)
        || (UsbClassCReg.PI != PCI_CLASSC_PI_UHCI)) {
    
    gBS->CloseProtocol (
            Controller,  
            &gEfiPciIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
            );
  
    return EFI_UNSUPPORTED;
  }
  gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );  
  return EFI_SUCCESS;
    
} 


EFI_STATUS
UHCIDriverBindingStart (
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
  EFI_STATUS              Status; 
  UINTN                   FlBaseAddrReg; 
  EFI_PCI_IO_PROTOCOL     *PciIo; 
  USB_HC_DEV              *HcDev;
  
  HcDev = NULL;
  
  Status = gBS->OpenProtocol (
                  Controller, 
                  &gEfiPciIoProtocolGuid, 
                  &PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Turn off USB emulation 
  //
  TurnOffUSBEmulation (PciIo);
  
  //
  // Enable the USB Host Controller
  //
  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationEnable,
                    EFI_PCI_DEVICE_ENABLE, 
                    NULL
                    );
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return EFI_UNSUPPORTED;
  }

  EnableUhc (PciIo);

  //
  // allocate memory for UHC private data structure
  //
  Status = gBS->AllocatePool(
              EfiBootServicesData,
              sizeof(USB_HC_DEV),
              &HcDev
              );

  if(EFI_ERROR(Status)) {
    gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return EFI_OUT_OF_RESOURCES;
  }

  EfiZeroMem(HcDev,sizeof(USB_HC_DEV));  
  
  //
  // init EFI_USB_HC_PROTOCOL protocol interface and install the protocol
  //
  HcDev->UsbHc.Reset                    = UHCIReset;
  HcDev->UsbHc.GetState                 = UHCIGetState;
  HcDev->UsbHc.SetState                 = UHCISetState;
  HcDev->UsbHc.ControlTransfer          = UHCIControlTransfer;
  HcDev->UsbHc.BulkTransfer             = UHCIBulkTransfer;
  HcDev->UsbHc.AsyncInterruptTransfer   = UHCIAsyncInterruptTransfer;
  HcDev->UsbHc.SyncInterruptTransfer    = UHCISyncInterruptTransfer;
  HcDev->UsbHc.IsochronousTransfer      = UHCIIsochronousTransfer;
  HcDev->UsbHc.AsyncIsochronousTransfer = UHCIAsyncIsochronousTransfer;
  HcDev->UsbHc.GetRootHubPortNumber     = UHCIGetRootHubPortNumber;
  HcDev->UsbHc.GetRootHubPortStatus     = UHCIGetRootHubPortStatus;
  HcDev->UsbHc.SetRootHubPortFeature    = UHCISetRootHubPortFeature;
  HcDev->UsbHc.ClearRootHubPortFeature  = UHCIClearRootHubPortFeature;
  
  HcDev->UsbHc.MajorRevision            = 0x1;   // indicate usb1.1
  HcDev->UsbHc.MinorRevision            = 0x1;   // 
  
  //
  // Install Host Controller Protocol
  //
  Status = gBS->InstallProtocolInterface (
                            &Controller,
                            &gEfiUsbHcProtocolGuid, 
                            EFI_NATIVE_INTERFACE,
                            &HcDev->UsbHc
                            );
  if (EFI_ERROR(Status)) {
    
    if(HcDev != NULL) {
      gBS->FreePool(HcDev);
    }  
    
    gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return Status;
  } 

  //
  //  Init UHCI private data structures
  //  
  HcDev->Signature = USB_HC_DEV_SIGNATURE;
  HcDev->PciIo = PciIo;
  
  FlBaseAddrReg = USBFLBASEADD;

  //
  // Allocate and Init Host Controller's Frame List Entry
  //
  Status = CreateFrameList (HcDev,(UINT32)FlBaseAddrReg);
  if (EFI_ERROR(Status)) {
    
    gBS->UninstallProtocolInterface(
                  Controller,
                  &gEfiUsbHcProtocolGuid,
                  &HcDev->UsbHc
                  ) ;
    
    if(HcDev != NULL) {
      gBS->FreePool(HcDev);
    }    
    
    gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return EFI_OUT_OF_RESOURCES ;
  }
  
  //
  //  Init interrupt list head in the HcDev structure.
  //
  InitializeListHead (&(HcDev->InterruptListHead)) ;

  //
  //  Create timer for interrupt transfer result polling
  //
  Status = gBS->CreateEvent (
            EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL, 
            EFI_TPL_NOTIFY, 
            MonitorInterruptTrans, 
            HcDev, 
            &HcDev->InterruptTransTimer
            ) ;
  if (EFI_ERROR(Status)) {
    
    FreeFrameListEntry(HcDev);
    
    gBS->UninstallProtocolInterface(
                  Controller,
                  &gEfiUsbHcProtocolGuid,
                  &HcDev->UsbHc
                  ) ;
    
    if(HcDev != NULL) {
      gBS->FreePool(HcDev);
    }    
    
    gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,
         Controller   
         );
    return EFI_UNSUPPORTED;
  }

  //
  // Here set interrupt transfer polling timer in 50ms unit.
  //
  Status = gBS->SetTimer(HcDev->InterruptTransTimer, TimerPeriodic, 50*1000*10);
  if (EFI_ERROR(Status)) {
    gBS->CloseEvent(HcDev->InterruptTransTimer);
    
    FreeFrameListEntry(HcDev);
    
    gBS->UninstallProtocolInterface(
                  Controller,
                  &gEfiUsbHcProtocolGuid,
                  &HcDev->UsbHc
                  ) ;
    
    if(HcDev != NULL) {
      gBS->FreePool(HcDev);
    }    
    
    gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller
         );
    return EFI_UNSUPPORTED;
  }
  
  //
  // QH,TD structures must in common buffer that will be
  // accessed by both cpu and usb bus master at the same time.
  // so, there must has memory management for QH,TD structures.
  //
  Status = InitializeMemoryManagement (HcDev);
  if (EFI_ERROR(Status)) {

    gBS->SetTimer(HcDev->InterruptTransTimer,TimerCancel, 0) ;
    
    gBS->CloseEvent(HcDev->InterruptTransTimer);
    
    FreeFrameListEntry(HcDev);
    
    gBS->UninstallProtocolInterface(
                  Controller,
                  &gEfiUsbHcProtocolGuid,
                  &HcDev->UsbHc
                  ) ;
    
    if(HcDev != NULL) {
      gBS->FreePool(HcDev);
    }    
    
    gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller
         );
    return Status;
  }
  
  //
  // component name protocol.
  //
  HcDev->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gUhciComponentName.SupportedLanguages, 
    &HcDev->ControllerNameTable, 
    L"Usb Universal Host Controller"
  );

  return EFI_SUCCESS ;    
}


EFI_STATUS
UnInstallUHCInterface(
  IN  EFI_HANDLE            Controller,
  IN  EFI_USB_HC_PROTOCOL   *This
  )
{
  USB_HC_DEV    *HcDev ;  
  
  HcDev = USB_HC_DEV_FROM_THIS(This) ;
  
  gBS->UninstallProtocolInterface(
            Controller,
            &gEfiUsbHcProtocolGuid,
            &HcDev->UsbHc
            ) ;
  
  //
  // first stop USB Host Controller
  //
  This->SetState (This,EfiUsbHcStateHalt);
  
  //
  // Delete interrupt transfer polling timer
  //
  gBS->SetTimer(HcDev->InterruptTransTimer,TimerCancel, 0) ;
  gBS->CloseEvent(HcDev->InterruptTransTimer) ;
  
  //
  // Delete all the asynchronous interrupt transfers in the interrupt list 
  // and free associated memory
  //
  ReleaseInterruptList(HcDev,&(HcDev->InterruptListHead)) ;
  
  //
  // free Frame List Entry.
  //
  FreeFrameListEntry(HcDev);  
  
  //
  // Free common buffer allocated for QH,TD structures
  //
  DelMemoryManagement (HcDev);    

  if (HcDev->ControllerNameTable) {
    EfiLibFreeUnicodeStringTable (HcDev->ControllerNameTable);
  }

  //
  // Disable the USB Host Controller
  //
  HcDev->PciIo->Attributes (
                  HcDev->PciIo,
                  EfiPciIoAttributeOperationDisable,
                  EFI_PCI_DEVICE_ENABLE, 
                  NULL
                  );

  gBS->FreePool(HcDev) ;
  
  return EFI_SUCCESS ;  
}


EFI_STATUS
UHCIDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/       
{
  EFI_USB_HC_PROTOCOL   *UsbHc;
  EFI_STATUS            OpenStatus;

  OpenStatus = gBS->OpenProtocol(
                       Controller,   
                       &gEfiUsbHcProtocolGuid,  
                       &UsbHc,
                       This->DriverBindingHandle,     
                       Controller,   
                       EFI_OPEN_PROTOCOL_GET_PROTOCOL
                       );   

  //
  // Test whether the Controller handler passed in is a valid
  // Usb controller handle that should be supported, if not,
  // return the error status directly
  //
  if (EFI_ERROR (OpenStatus)) {
    return OpenStatus;
  }

  //
  // free all the controller related memory and uninstall UHCI Protocol.
  //         
  UnInstallUHCInterface(Controller,UsbHc);  
  
  gBS->CloseProtocol (
           Controller, 
           &gEfiPciIoProtocolGuid, 
           This->DriverBindingHandle, 
           Controller
           );
    
  return EFI_SUCCESS;

}


EFI_STATUS
UHCIReset(
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT16                 Attributes
  )
/*++
  
  Routine Description:
    Provides software reset for the USB host controller.
  
  Arguments:
  
  This        A pointer to the EFI_USB_HC_PROTOCOL instance.  
  
  Attributes  A bit mask of the reset operation to perform. 
              See below for a list of the supported bit mask values.
  
  #define EFI_USB_HC_RESET_GLOBAL           0x0001
  #define EFI_USB_HC_RESET_HOST_CONTROLLER  0x0002

  EFI_USB_HC_RESET_GLOBAL 
        If this bit is set, a global reset signal will be sent to the USB bus.
        This resets all of the USB bus logic, including the USB host 
        controller hardware and all the devices attached on the USB bus.
  EFI_USB_HC_RESET_HOST_CONTROLLER  
        If this bit is set, the USB host controller hardware will be reset. 
        No reset signal will be sent to the USB bus.
  
  Returns:
    EFI_SUCCESS 
        The reset operation succeeded.
    EFI_INVALID_PARAMETER 
        Attributes is not valid.
    EFI_DEVICE_ERROR  
        An error was encountered while attempting to perform 
        the reset operation.
--*/
{  
  BOOLEAN       Match;
  USB_HC_DEV    *HcDev;
  UINT32        CommandRegAddr;
  UINT32        FlBaseAddrReg;
  UINT16        Command;
  
  Match           = FALSE;
  HcDev           = USB_HC_DEV_FROM_THIS(This);
  
  CommandRegAddr  = (UINT32)(USBCMD);
  FlBaseAddrReg   = (UINT32)(USBFLBASEADD);
  
  if ((Attributes & EFI_USB_HC_RESET_GLOBAL) != 0) {
    Match = TRUE;    
    //
    // set the Global Reset bit in the command register
    //
    Command = ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr);
    Command |= USBCMD_GRESET;
    WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr,Command);

    //
    // wait 20ms to let reset complete
    // (the UHCI spec asks for a minimum of 10ms to let reset complete)
    //
    gBS->Stall(20000);
    
    //
    // Clear the Global Reset bit to zero.
    //
    Command &= ~USBCMD_GRESET;
    WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr,Command);
    
    gBS->Stall(10000);     
  }
  
  if ((Attributes & EFI_USB_HC_RESET_HOST_CONTROLLER) != 0) {
    Match = TRUE;
    //
    // set Host Controller Reset bit to 1
    //
    Command = ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr);
    Command |= USBCMD_HCRESET;
    WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr,Command);
    //
    // this bit will be reset by Host Controller when reset is completed.
    //
    gBS->Stall(10000);     // wait 10ms to let reset complete
  }
  
  if (!Match) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Delete all old transactions on the USB bus
  //
  CleanUsbTransactions (HcDev);
  
  //
  // Initialize Universal Host Controller's Frame List Data Structure
  //
  InitFrameList (HcDev);
  
  // 
  // Reset may cause Frame List Base Address Register reset to zero,
  // so set the original value back again.
  //  
  SetFrameListBaseAddress(
    HcDev->PciIo,
    FlBaseAddrReg,
    (UINT32)((UINTN)HcDev->FrameListEntry)
  );
  
  return EFI_SUCCESS;  
} 

EFI_STATUS
UHCIGetState (
  IN  EFI_USB_HC_PROTOCOL    *This,
  OUT EFI_USB_HC_STATE      *State
  )
/*++
  
  Routine Description:
    Retrieves current state of the USB host controller.
  
  Arguments:
    
    This      A pointer to the EFI_USB_HC_PROTOCOL instance.
    
    State     A pointer to the EFI_USB_HC_STATE data structure that 
              indicates current state of the USB host controller.  
              Type EFI_USB_HC_STATE is defined below.
              
    typedef enum {
      EfiUsbHcStateHalt,
      EfiUsbHcStateOperational,
      EfiUsbHcStateSuspend,
      EfiUsbHcStateMaximum
    } EFI_USB_HC_STATE;
  
  Returns:
    EFI_SUCCESS 
            The state information of the host controller was returned in State.
    EFI_INVALID_PARAMETER 
            State is NULL.
    EFI_DEVICE_ERROR  
            An error was encountered while attempting to retrieve the 
            host controller's current state.  
--*/        
{
  USB_HC_DEV    *HcDev;
  UINT32        CommandRegAddr;
  UINT32        StatusRegAddr;
  UINT16        UhcCommand;
  UINT16        UhcStatus;
  
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  HcDev           = USB_HC_DEV_FROM_THIS(This);

  CommandRegAddr  = (UINT32)(USBCMD);
  StatusRegAddr   = (UINT32)(USBSTS);
  
  UhcCommand = ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr);
  UhcStatus = ReadUHCCommandReg(HcDev->PciIo,StatusRegAddr);
  
  if (UhcCommand & USBCMD_EGSM) {
    *State = EfiUsbHcStateSuspend;
    return EFI_SUCCESS;
  } 

  if ((UhcStatus & USBSTS_HCH) == 0) {
    *State = EfiUsbHcStateOperational;    
  } else {
    *State = EfiUsbHcStateHalt;
  }
    
  return EFI_SUCCESS;  
}


EFI_STATUS
UHCISetState (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  EFI_USB_HC_STATE       State
  )
/*++
  
  Routine Description:
    Sets the USB host controller to a specific state.
  
  Arguments:
    
    This      A pointer to the EFI_USB_HC_PROTOCOL instance.

    State     Indicates the state of the host controller that will be set.
  
  Returns:
    EFI_SUCCESS 
          The USB host controller was successfully placed in the state 
          specified by State.
    EFI_INVALID_PARAMETER 
          State is invalid.
    EFI_DEVICE_ERROR  
          Failed to set the state specified by State due to device error.  
--*/
{
  USB_HC_DEV          *HcDev;
  UINT32              CommandRegAddr;
  UINT32              StatusRegAddr;
  UINT16              Command;
  EFI_USB_HC_STATE    CurrentState;
  EFI_STATUS          Status;

  HcDev           = USB_HC_DEV_FROM_THIS(This) ;

  CommandRegAddr  = (UINT32)(USBCMD);
  StatusRegAddr   = (UINT32)(USBSTS);
  
  Status = UHCIGetState (This,&CurrentState);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  switch (State) {
    
    case EfiUsbHcStateHalt:      
      if (CurrentState == EfiUsbHcStateHalt) {
        return EFI_SUCCESS;
      }
      
      Command = ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr);
      Command &= ~USBCMD_RS ;
      WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr,Command);
      StatusRegAddr = (UINT32)(USBSTS);
      //
      // ensure the HC is in halt status after send the stop command
      //
      if(WaitForUHCHalt(HcDev->PciIo,StatusRegAddr,STALL_1_SECOND) == EFI_TIMEOUT) {
        return EFI_DEVICE_ERROR ;
      }
      break;
      
    case EfiUsbHcStateOperational:
      if (IsHostSysErr(HcDev->PciIo,StatusRegAddr) 
          || IsHCProcessErr(HcDev->PciIo,StatusRegAddr)) {
        return EFI_DEVICE_ERROR;
      }
      
      switch (CurrentState) {
        
        case EfiUsbHcStateOperational:
          return EFI_SUCCESS;
        
        case EfiUsbHcStateHalt:
          //
          // Set Run/Stop bit to 1.
          //
          Command = ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr);
          Command |= USBCMD_RS ;
          WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr,Command) ;
          break;
          
        case EfiUsbHcStateSuspend:
          Command = ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr);
          
          //
          // FGR(Force Global Resume) bit is 0
          //
          if((Command | (~USBCMD_FGR)) != 0xFF) {
            //
            // Write FGR bit to 1
            //
            Command |= USBCMD_FGR;
            WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr,Command);       
          }
          
          //
          // wait 20ms to let resume complete
          // (20ms is specified by UHCI spec)
          //
          gBS->Stall(20000);
        
          //
          // Write FGR bit to 0 and EGSM(Enter Global Suspend Mode) bit to 0 
          //
          Command &= ~USBCMD_FGR;
          Command &= ~USBCMD_EGSM;
          WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr, Command);
          break;
      }
      break;
      
    case EfiUsbHcStateSuspend:  
      if (CurrentState == EfiUsbHcStateSuspend) {
        return EFI_SUCCESS;
      }
      
      Status = UHCISetState (This,EfiUsbHcStateHalt);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      //
      // Set Enter Global Suspend Mode bit to 1.
      //
      Command = ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr);
      Command |= USBCMD_EGSM ;
      WriteUHCCommandReg(HcDev->PciIo,CommandRegAddr,Command);
      break;
      
    default:
      return EFI_INVALID_PARAMETER;
  }
  
  return EFI_SUCCESS;
}    


EFI_STATUS
UHCIGetRootHubPortNumber (
  IN  EFI_USB_HC_PROTOCOL    *This,
  OUT UINT8                  *PortNumber
  )
/*++
  
  Routine Description:
    Retrieves the number of root hub ports.
    
  Arguments:
  
    This        A pointer to the EFI_USB_HC_PROTOCOL instance.
    
    PortNumber  A pointer to the number of the root hub ports.
  
  Returns:
    EFI_SUCCESS 
          The port number was retrieved successfully.
    EFI_INVALID_PARAMETER 
          PortNumber is NULL.
    EFI_DEVICE_ERROR  
          An error was encountered while attempting to 
          retrieve the port number.  
--*/  
{
  USB_HC_DEV    *HcDev;
  UINT32        PSAddr;
  UINT16        RHPortControl;
  UINT32        Index;

  HcDev = USB_HC_DEV_FROM_THIS(This);
  
  if (PortNumber == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  *PortNumber = 0;
  
  for (Index = 0 ; Index < 2; Index ++) {
    PSAddr = (UINT32)(USBPORTSC1 + Index * 2);
    RHPortControl = ReadRootPortReg (HcDev->PciIo, PSAddr);
    //
    // Port Register content is valid
    //
    if (RHPortControl != 0xff) {
      (*PortNumber) ++;
    }
  }
  
  return EFI_SUCCESS;
}

    
EFI_STATUS
UHCIGetRootHubPortStatus (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  PortNumber,
  OUT EFI_USB_PORT_STATUS    *PortStatus
  )
/*++
  
  Routine Description:
    Retrieves the current status of a USB root hub port.
  
  Arguments:
  
    This        A pointer to the EFI_USB_HC_PROTOCOL.
    
    PortNumber  Specifies the root hub port from which the status 
                is to be retrieved.  This value is zero-based. For example, 
                if a root hub has two ports, then the first port is numbered 0,
                and the second port is numbered 1.
    
    PortStatus  A pointer to the current port status bits and 
                port status change bits.  
  
  Returns:
    EFI_SUCCESS 
        The status of the USB root hub port specified by PortNumber 
        was returned in PortStatus.
    EFI_INVALID_PARAMETER 
        PortNumber is invalid.  
--*/
{
  USB_HC_DEV    *HcDev;
  UINT32        PSAddr;
  UINT16        RHPortStatus;    // root hub port status
  UINT8         TotalPortNumber;

  if (PortStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  UHCIGetRootHubPortNumber(This,&TotalPortNumber);
  if(PortNumber >= TotalPortNumber) {
    return EFI_INVALID_PARAMETER;
  }
  
  HcDev       = USB_HC_DEV_FROM_THIS(This) ;

  PSAddr      = (UINT32)(USBPORTSC1 + PortNumber * 2);

  PortStatus -> PortStatus = 0 ;
  PortStatus -> PortChangeStatus = 0 ; 

  RHPortStatus = ReadRootPortReg(HcDev->PciIo,PSAddr) ;

  //
  //    Fill Port Status bits
  //
  
  //
  // Current Connect Status
  //
  if(RHPortStatus & USBPORTSC_CCS) {
    PortStatus -> PortStatus |= USB_PORT_STAT_CONNECTION ;
  }

  //
  // Port Enabled/Disabled
  //
  if(RHPortStatus & USBPORTSC_PED) {
    PortStatus -> PortStatus |= USB_PORT_STAT_ENABLE ;
  }
  
  //
  // Port Suspend
  //
  if(RHPortStatus & USBPORTSC_SUSP) {
    PortStatus -> PortStatus |= USB_PORT_STAT_SUSPEND ;
  }
  
  //
  // Port Reset
  //
  if(RHPortStatus & USBPORTSC_PR) {
    PortStatus -> PortStatus |= USB_PORT_STAT_RESET ;
  }
  
  //
  // Low Speed Device Attached
  //
  if(RHPortStatus & USBPORTSC_LSDA) {
    PortStatus -> PortStatus |= USB_PORT_STAT_LOW_SPEED ;
  }

  //
  //   Fill Port Status Change bits
  //
  
  //
  // Connect Status Change
  //
  if(RHPortStatus & USBPORTSC_CSC) {
    PortStatus -> PortChangeStatus |= USB_PORT_STAT_C_CONNECTION ;
  }
  
  //
  // Port Enabled/Disabled Change
  //
  if(RHPortStatus & USBPORTSC_PEDC) {
    PortStatus -> PortChangeStatus |= USB_PORT_STAT_C_ENABLE ;
  }

  return EFI_SUCCESS ;
}    

EFI_STATUS
UHCISetRootHubPortFeature (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  PortNumber,
  IN  EFI_USB_PORT_FEATURE   PortFeature
  )
/*++
  
  Routine Description:
    Sets a feature for the specified root hub port.
  
  Arguments:
  
    This        A pointer to the EFI_USB_HC_PROTOCOL.
    
    PortNumber  Specifies the root hub port whose feature 
                is requested to be set.
    
    PortFeature Indicates the feature selector associated 
                with the feature set request. 
  
  Returns:
    EFI_SUCCESS 
        The feature specified by PortFeature was set for the 
        USB root hub port specified by PortNumber.
    EFI_INVALID_PARAMETER 
        PortNumber is invalid or PortFeature is invalid.
--*/
{
  USB_HC_DEV    *HcDev;
  UINT32        PSAddr;
  UINT32        CommandRegAddr;
  UINT16        RHPortControl;    // root hub port status
  UINT8         TotalPortNumber;
  
  UHCIGetRootHubPortNumber(This,&TotalPortNumber);
  if(PortNumber >= TotalPortNumber) {
    return EFI_INVALID_PARAMETER;
  }
  
  HcDev           = USB_HC_DEV_FROM_THIS(This) ;

  PSAddr          = (UINT32)(USBPORTSC1 + PortNumber * 2) ;
  CommandRegAddr  = (UINT32)(USBCMD) ;

  RHPortControl   = ReadRootPortReg(HcDev->PciIo,PSAddr) ;

  switch(PortFeature) {
    
    case EfiUsbPortSuspend:
      if(!(ReadUHCCommandReg(HcDev->PciIo,CommandRegAddr) & USBCMD_EGSM)) {
        //
        // if global suspend is not active, can set port suspend
        //
        RHPortControl &= 0xfff5;
        RHPortControl |= USBPORTSC_SUSP;
      }
      break ;
      
    case EfiUsbPortReset:
      RHPortControl &= 0xfff5;
      RHPortControl |= USBPORTSC_PR;        // Set the reset bit 
      break ;
  
    case EfiUsbPortPower:
      break ;
      
    case EfiUsbPortEnable:
      RHPortControl &= 0xfff5;
      RHPortControl |= USBPORTSC_PED;
      break;
      
    default:
      return EFI_INVALID_PARAMETER;
  }

  WriteRootPortReg(HcDev->PciIo,PSAddr,RHPortControl) ;

  return EFI_SUCCESS ;
}    

EFI_STATUS
UHCIClearRootHubPortFeature (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  PortNumber,
  IN  EFI_USB_PORT_FEATURE   PortFeature
  )
/*++
  
  Routine Description:
    Clears a feature for the specified root hub port.
  
  Arguments:
  
    This        A pointer to the EFI_USB_HC_PROTOCOL instance.
    
    PortNumber  Specifies the root hub port whose feature 
                is requested to be cleared.
    
    PortFeature Indicates the feature selector associated with the 
                feature clear request.
                  
  Returns:
    EFI_SUCCESS 
        The feature specified by PortFeature was cleared for the 
        USB root hub port specified by PortNumber.
    EFI_INVALID_PARAMETER 
        PortNumber is invalid or PortFeature is invalid.
--*/
{
  USB_HC_DEV    *HcDev;
  UINT32        PSAddr;
  UINT16        RHPortControl;
  UINT8         TotalPortNumber;
  
  UHCIGetRootHubPortNumber(This,&TotalPortNumber);
  
  if(PortNumber >= TotalPortNumber) {
    return EFI_INVALID_PARAMETER;
  }
  
  HcDev         = USB_HC_DEV_FROM_THIS(This);

  PSAddr        = (UINT32)(USBPORTSC1 + PortNumber * 2) ;

  RHPortControl = ReadRootPortReg(HcDev->PciIo,PSAddr) ;

  switch (PortFeature) {
    
    //
    // clear PORT_ENABLE feature means disable port.
    //
    case EfiUsbPortEnable:
      RHPortControl &= 0xfff5;
      RHPortControl &= ~USBPORTSC_PED ;             
      break;
    
    // 
    // clear PORT_SUSPEND feature means resume the port.
    // (cause a resume on the specified port if in suspend mode)
    //
    case EfiUsbPortSuspend:
      RHPortControl &= 0xfff5;
      RHPortControl &= ~USBPORTSC_SUSP ;            
      break;
    
    //
    // no operation
    //
    case EfiUsbPortPower:
      break;
    
    //
    // clear PORT_RESET means clear the reset signal.
    //  
    case EfiUsbPortReset:
      RHPortControl &= 0xfff5;
      RHPortControl &= ~USBPORTSC_PR ;        
      break ;
    
    //
    // clear connect status change
    //
    case EfiUsbPortConnectChange:
      RHPortControl &= 0xfff5;
      RHPortControl |= USBPORTSC_CSC ;              
      break;
    
    //
    // clear enable/disable status change
    //
    case EfiUsbPortEnableChange:
      RHPortControl &= 0xfff5;
      RHPortControl |= USBPORTSC_PEDC ;             
      break;
    
    //
    // root hub does not support this request
    //
    case EfiUsbPortSuspendChange:                              
      break;
    
    //
    // root hub does not support this request
    //
    case EfiUsbPortOverCurrentChange:                         
      break;
    
    //
    // root hub does not support this request
    //
    case EfiUsbPortResetChange:
      break;
    
    default:
      return EFI_INVALID_PARAMETER ;
  }

  WriteRootPortReg(HcDev->PciIo,PSAddr,RHPortControl) ;

  return EFI_SUCCESS ;
}    

EFI_STATUS
UHCIControlTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  BOOLEAN                IsSlowDevice,
  IN  UINT8                  MaximumPacketLength,
  IN  EFI_USB_DEVICE_REQUEST *Request,
  IN  EFI_USB_DATA_DIRECTION TransferDirection,
  IN  OUT VOID               *Data                 OPTIONAL,
  IN  OUT UINTN              *DataLength           OPTIONAL,  
  IN  UINTN                  TimeOut,
  OUT UINT32                 *TransferResult
  )
/*++
  
  Routine Description:
    Submits control transfer to a target USB device.
  
  Arguments:
    
    This          A pointer to the EFI_USB_HC_PROTOCOL instance.

    DeviceAddress Represents the address of the target device on the USB,
                  which is assigned during USB enumeration.

    IsSlowDevice  Indicates whether the target device is slow device 
                  or full-speed device.
    
    MaximumPacketLength Indicates the maximum packet size that the 
                        default control transfer endpoint is capable of 
                        sending or receiving.
    
    Request       A pointer to the USB device request that will be sent 
                  to the USB device. 
    
    TransferDirection Specifies the data direction for the transfer.
                      There are three values available, DataIn, DataOut 
                      and NoData.
    
    Data          A pointer to the buffer of data that will be transmitted 
                  to USB device or received from USB device.
    
    DataLength    Indicates the size, in bytes, of the data buffer 
                  specified by Data.
    
    TimeOut       Indicates the maximum time, in microseconds, 
                  which the transfer is allowed to complete.
    
    TransferResult  A pointer to the detailed result information generated 
                    by this control transfer.
                    
  Returns:
    EFI_SUCCESS 
        The control transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  
        The control transfer could not be completed due to a lack of resources.
    EFI_INVALID_PARAMETER 
        Some parameters are invalid.
    EFI_TIMEOUT 
        The control transfer failed due to timeout.
    EFI_DEVICE_ERROR  
        The control transfer failed due to host controller or device error. 
        Caller should check TranferResult for detailed error information.

--*/
{
  USB_HC_DEV    *HcDev;
  UINT32        StatusReg;
  UINT32        FrameNumReg;
  UINT8         PktID;
  QH_STRUCT     *ptrQH;
  TD_STRUCT     *ptrTD;
  TD_STRUCT     *ptrPreTD;
  TD_STRUCT     *ptrSetupTD;
  TD_STRUCT     *ptrStatusTD;
  EFI_STATUS    Status;  
  UINTN         i;
  UINTN         DataLen;
  UINT8         *ptrDataSource;
  UINT8         *ptr;
  UINT8         DataToggle;
  UINT16        LoadFrameListIndex;
  UINT8         pktsize;
  
  UINT8         *RequestMappedAddress = NULL;
  VOID          *RequestMapping = NULL;
  UINTN         RequestLen;
  
  EFI_PHYSICAL_ADDRESS    TempPtr;
  VOID                    *Mapping = NULL;
  
  TD_STRUCT     *ptrFirstDataTD = NULL;
  TD_STRUCT     *ptrLastDataTD = NULL;
  BOOLEAN       FirstTD = FALSE;
  
  PktID       = INPUT_PACKET_ID;
  Mapping     = NULL;  
  HcDev       = USB_HC_DEV_FROM_THIS(This);  
  StatusReg   = (UINT32)(USBSTS);
  FrameNumReg = (UINT32)(USBFRNUM);
  ptrPreTD    = NULL;
  ptrTD       = NULL;
  
  //
  // Parameters Checking
  //
  
  if (Request == NULL || TransferResult == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // if errors exist that cause host controller halt, 
  // then return EFI_DEVICE_ERROR.
  //
  if(IsHCHalted(HcDev->PciIo,StatusReg)       || 
     IsHCProcessErr(HcDev->PciIo,StatusReg)   ||
     IsHostSysErr(HcDev->PciIo,StatusReg)) {      
    
    ClearStatusReg(HcDev->PciIo,StatusReg);
    *TransferResult = EFI_USB_ERR_SYSTEM;
    return EFI_DEVICE_ERROR;
  }
  
  //
  // low speed usb devices are limited to only an eight-byte 
  // maximum data payload size
  //
  if (IsSlowDevice && (MaximumPacketLength != 8)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (MaximumPacketLength != 8 && MaximumPacketLength != 16 
      && MaximumPacketLength != 32 && MaximumPacketLength != 64) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((TransferDirection != EfiUsbNoData) && (DataLength == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  switch(TransferDirection) {
    
    case EfiUsbDataIn:
      PktID         = INPUT_PACKET_ID;
      ptrDataSource = Data;
      DataLen       = *DataLength;
      
      //
      // map the source data buffer for bus master access.
      // BusMasterWrite means cpu read
      //
      Status = HcDev->PciIo->Map (
                        HcDev->PciIo,
                        EfiPciIoOperationBusMasterWrite,
                        ptrDataSource,
                        &DataLen,
                        &TempPtr,
                        &Mapping
                        );
      if (EFI_ERROR(Status)) {
        return Status;
      }
      ptr = (UINT8*)((UINTN)TempPtr);
      break;
      
    case EfiUsbDataOut:
      PktID         = OUTPUT_PACKET_ID;
      ptrDataSource = Data;
      DataLen       = *DataLength;
      
      //
      // map the source data buffer for bus master access.
      // BusMasterRead means cpu write
      //
      Status = HcDev->PciIo->Map (HcDev->PciIo,
                        EfiPciIoOperationBusMasterRead,
                        ptrDataSource,
                        &DataLen,
                        &TempPtr,
                        &Mapping
                        );
      if (EFI_ERROR(Status)) {
        return Status;
      }
      ptr = (UINT8*)((UINTN)TempPtr);
      break;

    //
    // no data stage
    //
    case EfiUsbNoData:
      if ((DataLength != NULL) && (*DataLength != 0) ) {
        return EFI_INVALID_PARAMETER;
      }
      PktID         = OUTPUT_PACKET_ID;
      ptrDataSource = NULL;
      DataLen       = 0;
      ptr           = NULL;
      break;
      
    default:
      return EFI_INVALID_PARAMETER;      
  }
  
  ClearStatusReg(HcDev->PciIo,StatusReg);
  
  //  
  // create QH structure and init
  //
  Status = CreateQH(HcDev,&ptrQH);
  if(EFI_ERROR(Status)) {
    HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
    return Status;
  }
  
  //
  // map the Request for bus master access.
  // BusMasterRead means cpu write
  //
  
  RequestLen = sizeof (EFI_USB_DEVICE_REQUEST);
  Status = HcDev->PciIo->Map (HcDev->PciIo,
                     EfiPciIoOperationBusMasterRead,
                     (UINT8*)Request,
                     &RequestLen,
                     &TempPtr,
                     &RequestMapping
                     );
  if (EFI_ERROR(Status)) {
    HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
    UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
    return Status;
  }
  RequestMappedAddress = (UINT8*)((UINTN)TempPtr);
  
  //
  // generate Setup Stage TD
  //
  
  Status = GenSetupStageTD(HcDev,DeviceAddress,0,
                  IsSlowDevice, (UINT8*)RequestMappedAddress, 
                  sizeof(EFI_USB_DEVICE_REQUEST), &ptrSetupTD);
  if (EFI_ERROR(Status)) {
    HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
    UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
    HcDev->PciIo->Unmap (HcDev->PciIo,RequestMapping);
    return Status;
  }  
  
  //
  //  Data Stage of Control Transfer
  //   
  DataToggle = 1;
  FirstTD = TRUE;
  while(DataLen > 0) {
    //
    // create TD structures and link together
    //    
    
    //
    // pktsize is the data load size that each TD carries. 
    //
    pktsize = (UINT8)DataLen;
    if (DataLen > MaximumPacketLength) {
      pktsize = MaximumPacketLength;
    }
    Status = GenDataTD(HcDev,DeviceAddress, 0, ptr, pktsize, PktID, DataToggle, IsSlowDevice,&ptrTD);
    if (EFI_ERROR(Status)) {
      //
      // free all resources occupied
      //
      HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
      UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
      HcDev->PciIo->Unmap (HcDev->PciIo,RequestMapping);
      DeleteQueuedTDs (HcDev, ptrSetupTD);
      DeleteQueuedTDs (HcDev,ptrFirstDataTD);
      return Status;
    }
    
    //
    // Link two TDs in vertical depth
    //
    if (FirstTD) {
      ptrFirstDataTD            = ptrTD ;
      ptrFirstDataTD->ptrNextTD = NULL;
      FirstTD = FALSE;
    } else {
      LinkTDToTD(ptrPreTD,ptrTD);      
    }
    ptrPreTD = ptrTD;
    
    DataToggle ^= 1;
    ptr += pktsize;
    DataLen -= pktsize;
  }
  ptrLastDataTD = ptrTD;
  
  
  //
  // Status Stage of Control Transfer
  //
  if(PktID == OUTPUT_PACKET_ID) {
    PktID = INPUT_PACKET_ID;
  } else {
    PktID = OUTPUT_PACKET_ID;
  }
  
  //
  // create Status Stage TD structure
  //
  Status = CreateStatusTD(HcDev,DeviceAddress,0,PktID,IsSlowDevice,&ptrStatusTD);
  if (EFI_ERROR(Status)) {
    HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
    UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
    HcDev->PciIo->Unmap (HcDev->PciIo,RequestMapping);
    DeleteQueuedTDs (HcDev, ptrSetupTD);
    DeleteQueuedTDs (HcDev,ptrFirstDataTD);
    return Status;
  }
  
  if (IsSlowDevice) {
    //
    // link setup TD structures to QH structure
    //
    LinkTDToQH(ptrQH,ptrSetupTD);
    
    LoadFrameListIndex = (UINT16)((GetCurrentFrameNumber(HcDev->PciIo,FrameNumReg)) % 1024);
  
    //
    // link QH-TDs to total 50 frame list entry to speed up the execution.
    //
    for(i = 0; i < 100; i ++) {
      LinkQHToFrameList(HcDev->FrameListEntry,
                        (UINT16)((LoadFrameListIndex + i) % 1024),
                         ptrQH) ;
    }
  
    //
    // Poll QH-TDs execution and get result.
    // detail status is returned
    //
    Status = ExecuteControlTransfer (HcDev,ptrSetupTD,
                                      LoadFrameListIndex,DataLength,
                                      TimeOut,TransferResult);
    //
    // Remove Control Transfer QH-TDs structure from the frame list
    // and update the pointers in the Frame List
    // and other pointers in other related QH structures.
    //
    for (i = 0; i < 100; i ++) {
      DelLinkSingleQH (HcDev,ptrQH,(UINT16)((LoadFrameListIndex + i) % 1024),FALSE,FALSE);
    }
    //
    // delete setup stage TD; the QH is reserved for the next stages.
    //
    DeleteQueuedTDs (HcDev,ptrSetupTD);
    
    //
    // some control transfers do not have Data Stage
    //
    if (ptrFirstDataTD != NULL) {
    
      LinkTDToQH (ptrQH,ptrFirstDataTD);
      LoadFrameListIndex = (UINT16)((GetCurrentFrameNumber(HcDev->PciIo,FrameNumReg)) % 1024);
    
      for(i = 0; i < 500; i ++) {
        LinkQHToFrameList(HcDev->FrameListEntry,
                          (UINT16)((LoadFrameListIndex + i) % 1024),
                           ptrQH) ;
      }
    
      Status = ExecuteControlTransfer (HcDev,ptrFirstDataTD,
                                        LoadFrameListIndex,DataLength,
                                        TimeOut,TransferResult);
    
      for (i = 0; i < 500; i ++) {
        DelLinkSingleQH (HcDev,ptrQH,(UINT16)((LoadFrameListIndex + i) % 1024),FALSE,FALSE);
      }
      //
      // delete data stage TD; the QH is reserved for the next stage.
      //
      DeleteQueuedTDs (HcDev,ptrFirstDataTD);
    }
    
    LinkTDToQH(ptrQH,ptrStatusTD);  
    //
    // get the frame list index that the QH-TDs will be linked to.
    //  
    LoadFrameListIndex = (UINT16)((GetCurrentFrameNumber(HcDev->PciIo,FrameNumReg)) % 1024);
  
    for(i = 0; i < 100; i ++) {
      
      //
      // put the QH-TDs directly or indirectly into the proper place 
      // in the Frame List 
      //
      LinkQHToFrameList(HcDev->FrameListEntry,
                        (UINT16)((LoadFrameListIndex + i) % 1024),
                         ptrQH) ;
    }
  
    //
    // Poll QH-TDs execution and get result.
    // detail status is returned
    //
    Status = ExecuteControlTransfer (HcDev,ptrStatusTD,
                                      LoadFrameListIndex,DataLength,
                                      TimeOut,TransferResult);
  
    //
    // Delete Control Transfer QH-TDs structure
    // and update the pointers in the Frame List
    // and other pointers in other related QH structures.
    //
    // TRUE means must search other framelistindex
    //
    for (i = 0; i < 100; i ++) {
      DelLinkSingleQH (HcDev,ptrQH,(UINT16)((LoadFrameListIndex + i) % 1024),FALSE,FALSE);
    }
    DeleteQueuedTDs (HcDev,ptrStatusTD);
    
  } else {    
    //
    // link setup stage TD with data stage TD
    //
    ptrPreTD = ptrSetupTD;
    if (ptrFirstDataTD != NULL) {
      LinkTDToTD (ptrSetupTD, ptrFirstDataTD);
      ptrPreTD = ptrLastDataTD;
    }
    
    //
    // link status TD with previous TD
    //
    LinkTDToTD (ptrPreTD, ptrStatusTD);
    
    //
    // link QH with TD
    //
    LinkTDToQH (ptrQH, ptrSetupTD);
    
    LoadFrameListIndex = (UINT16)((GetCurrentFrameNumber(HcDev->PciIo,FrameNumReg)) % 1024);
    for(i = 0; i < 500; i ++) {
    
      //
      // put the QH-TDs directly or indirectly into the proper place 
      // in the Frame List 
      //
      LinkQHToFrameList(HcDev->FrameListEntry,
                        (UINT16)((LoadFrameListIndex + i) % 1024),
                         ptrQH) ;
    }
    
    //
    // Poll QH-TDs execution and get result.
    // detail status is returned
    //
    Status = ExecuteControlTransfer (HcDev,ptrSetupTD,
                                      LoadFrameListIndex,DataLength,
                                      TimeOut,TransferResult);
    //
    // Remove Control Transfer QH-TDs structure from the frame list
    // and update the pointers in the Frame List
    // and other pointers in other related QH structures.
    //
    for (i = 0; i < 500; i ++) {
      DelLinkSingleQH (HcDev,ptrQH,(UINT16)((LoadFrameListIndex + i) % 1024),FALSE,FALSE);
    }    
    DeleteQueuedTDs (HcDev,ptrSetupTD);
  }
  
  UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
  
  if (Mapping != NULL) {
    HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
  }
  
  if (RequestMapping != NULL) {
    HcDev->PciIo->Unmap (HcDev->PciIo,RequestMapping);
  }
  //
  // if has errors that cause host controller halt, 
  // then return EFI_DEVICE_ERROR directly.
  //
  if(IsHCHalted(HcDev->PciIo,StatusReg)       || 
     IsHCProcessErr(HcDev->PciIo,StatusReg)   ||
     IsHostSysErr(HcDev->PciIo,StatusReg)) {      
    
    ClearStatusReg(HcDev->PciIo,StatusReg) ;
    *TransferResult |= EFI_USB_ERR_SYSTEM;
    return EFI_DEVICE_ERROR;
  }
  
  ClearStatusReg(HcDev->PciIo,StatusReg);
  
  return Status ;
}

EFI_STATUS
UHCIBulkTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  UINT8                  EndPointAddress,
  IN  UINT8                  MaximumPacketLength,
  IN OUT VOID                *Data,
  IN OUT UINTN               *DataLength,  
  IN OUT UINT8               *DataToggle,
  IN  UINTN                  TimeOut,
  OUT UINT32                 *TransferResult
  )
/*++
  
  Routine Description:
    Submits bulk transfer to a bulk endpoint of a USB device.
    
  Arguments:
    
    This          A pointer to the EFI_USB_HC_PROTOCOL instance.
    
    DeviceAddress Represents the address of the target device on the USB,
                  which is assigned during USB enumeration.
    EndPointAddress   The combination of an endpoint number and an 
                      endpoint direction of the target USB device. 
                      Each endpoint address supports data transfer in 
                      one direction except the control endpoint 
                      (whose default endpoint address is 0). 
                      It is the caller's responsibility to make sure that 
                      the EndPointAddress represents a bulk endpoint. 
                      
    MaximumPacketLength Indicates the maximum packet size the target endpoint
                        is capable of sending or receiving.
                        
    Data          A pointer to the buffer of data that will be transmitted 
                  to USB device or received from USB device.
    DataLength    When input, indicates the size, in bytes, of the data buffer
                  specified by Data. When output, indicates the actually 
                  transferred data size.
                  
    DataToggle    A pointer to the data toggle value. On input, it indicates 
                  the initial data toggle value the bulk transfer should adopt;
                  on output, it is updated to indicate the data toggle value 
                  of the subsequent bulk transfer. 
                  
    TimeOut       Indicates the maximum time, in microseconds, which the 
                  transfer is allowed to complete.
                  
    TransferResult  A pointer to the detailed result information of the 
                    bulk transfer.

  Returns:
    EFI_SUCCESS 
        The bulk transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  
        The bulk transfer could not be submitted due to lack of resource.
    EFI_INVALID_PARAMETER 
        Some parameters are invalid.
    EFI_TIMEOUT 
        The bulk transfer failed due to timeout.
    EFI_DEVICE_ERROR  
        The bulk transfer failed due to host controller or device error.
        Caller should check TranferResult for detailed error information.

--*/
{
  USB_HC_DEV              *HcDev;             
  UINT32                  StatusReg;          
  UINT32                  FrameNumReg;        
  UINTN                   DataLen;            
  QH_STRUCT               *ptrQH ;            
  TD_STRUCT               *ptrFirstTD;        
  TD_STRUCT               *ptrTD;             
  TD_STRUCT               *ptrPreTD;          
  UINT16                  LoadFrameListIndex; 
  UINT16                  SavedFrameListIndex;
  UINT8                   PktID;              
  UINT8                   *ptrDataSource;     
  UINT8                   *ptr;               
  BOOLEAN                 IsFirstTD;          
  EFI_STATUS              Status ;            
  UINT32                  i;
  UINT8                   pktsize;
                                          
  EFI_USB_DATA_DIRECTION TransferDirection;
  //
  //  Used to calculate how many entries are linked to the 
  //  specified bulk transfer QH-TDs 
  //
  UINT32                  LinkTimes;
  
  BOOLEAN                 ShortPacketEnable;  
  EFI_PHYSICAL_ADDRESS    TempPtr;
  VOID                    *Mapping;
  
  HcDev             = USB_HC_DEV_FROM_THIS(This);  
  StatusReg         = (UINT32)(USBSTS);
  FrameNumReg       = (UINT32)(USBFRNUM);
  PktID             = INPUT_PACKET_ID;
  ptrTD             = NULL;
  ptrFirstTD        = NULL;
  ptrPreTD          = NULL;
  LinkTimes         = 1;
  DataLen           = 0;
  ptr               = NULL;  
  ShortPacketEnable = FALSE;
  Mapping           = NULL;  
  
  //
  // Parameters Checking
  //
  
  if ((DataLength == NULL) || (Data == NULL) 
      || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // if has errors that cause host controller halt, 
  // then return EFI_DEVICE_ERROR directly.
  //
  if(IsHCHalted(HcDev->PciIo,StatusReg)       || 
     IsHCProcessErr(HcDev->PciIo,StatusReg)   ||
     IsHostSysErr(HcDev->PciIo,StatusReg)) {      
    
    ClearStatusReg(HcDev->PciIo,StatusReg) ;
    *TransferResult = EFI_USB_ERR_SYSTEM;
    return EFI_DEVICE_ERROR;
  }
  
  if (*DataLength == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((*DataToggle != 1) && (*DataToggle != 0)) {
    return EFI_INVALID_PARAMETER;
  } 
  
  if(MaximumPacketLength != 8 && MaximumPacketLength != 16 
      && MaximumPacketLength != 32 && MaximumPacketLength != 64) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Enable the maximum packet size (64bytes)
  // that can be used for full speed bandwidth reclamation 
  // at the end of a frame.
  //
  EnableMaxPacketSize (HcDev);  
  
  ClearStatusReg(HcDev->PciIo,StatusReg);

  //
  // construct QH and TD data structures,
  // and link them together
  //
  if (EndPointAddress & 0x80) {
    TransferDirection = EfiUsbDataIn;
  } else {
    TransferDirection = EfiUsbDataOut;
  }
  
  switch(TransferDirection) {
    
    case EfiUsbDataIn:
      ShortPacketEnable = TRUE;
      PktID             = INPUT_PACKET_ID;  
      ptrDataSource     = Data;
      DataLen           = *DataLength;
      
      //
      // BusMasterWrite means cpu read
      //
      Status = HcDev->PciIo->Map (HcDev->PciIo,
                        EfiPciIoOperationBusMasterWrite,
                        ptrDataSource,
                        &DataLen,
                        &TempPtr,
                        &Mapping
                        );
      if (EFI_ERROR(Status)) {
        return Status;
      }
      ptr = (UINT8*)((UINTN)TempPtr);
      break;
      
    case EfiUsbDataOut:
      PktID             = OUTPUT_PACKET_ID;
      ptrDataSource     = Data;
      DataLen           = *DataLength;
      
      //
      // BusMasterRead means cpu write
      //
      Status = HcDev->PciIo->Map (HcDev->PciIo,
                        EfiPciIoOperationBusMasterRead,
                        ptrDataSource,
                        &DataLen,
                        &TempPtr,
                        &Mapping
                        );
      if (EFI_ERROR(Status)) {
        return Status;
      }
      ptr = (UINT8*)((UINTN)TempPtr);
      break;
      
    default:
      return EFI_INVALID_PARAMETER;
  }
  
  //
  //  create QH structure and init
  //
  Status = CreateQH(HcDev,&ptrQH);
  if(EFI_ERROR(Status)) {
    HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
    return Status;
  }
  
  //
  // i is used to calculate the total number of TDs.
  //
  i = 0;
  
  IsFirstTD = TRUE ;
  while(DataLen > 0) {
    
    //
    // create TD structures and link together 
    //
    
    pktsize = (UINT8)DataLen;
    if(DataLen > MaximumPacketLength) {
      pktsize = MaximumPacketLength;
    }
    
    Status = GenDataTD(HcDev,DeviceAddress,EndPointAddress, ptr, 
                       pktsize, PktID, *DataToggle, FALSE,&ptrTD);
    if (EFI_ERROR(Status)) {
      HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
      UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
      DeleteQueuedTDs (HcDev,ptrFirstTD);
      return Status;
    }
    
    //
    // Enable short packet detection. 
    // (default action is disabling short packet detection)
    //
    if(ShortPacketEnable) {
      EnableorDisableTDShortPacket (ptrTD, TRUE) ;
    }
    
    if(IsFirstTD) {
      ptrFirstTD            = ptrTD ;
      ptrFirstTD->ptrNextTD = NULL;
      IsFirstTD             = FALSE ;
    } else {    
      //
      // Link two TDs in vertical depth
      //
      LinkTDToTD(ptrPreTD,ptrTD) ;
    }
    
    i ++ ;
    
    ptrPreTD = ptrTD ;      
    
    *DataToggle ^= 1;
    ptr += pktsize;
    DataLen -= pktsize;
  }
  
  //
  // link TD structures to QH structure
  //
  LinkTDToQH(ptrQH,ptrFirstTD);
  
  //
  // calculate how many entries are linked to the specified bulk transfer QH-TDs 
  // the below values are referred to the USB spec revision1.1.
  //
  switch(MaximumPacketLength) {
    case 8:
      LinkTimes = i / 71 + 1;
      break;
    case 16:
      LinkTimes = i / 51 + 1;
      break;
    case 32:
      LinkTimes = i / 33 + 1;
      break;
    case 64:
      LinkTimes = i / 19 + 1;
      break;
  }
  LinkTimes += 500;   // redundant
  
  //
  // put QH-TDs into  Frame list
  //

  LoadFrameListIndex = (UINT16)((GetCurrentFrameNumber(HcDev->PciIo,FrameNumReg)) % 1024);
  SavedFrameListIndex = LoadFrameListIndex;

  for(i = 0; i <= LinkTimes ; i ++) {
    
    //
    // put the QH-TD directly or indirectly into the proper place 
    // in the Frame List 
    //
    LinkQHToFrameList(HcDev->FrameListEntry,LoadFrameListIndex,ptrQH) ;

    LoadFrameListIndex += 1 ;
    LoadFrameListIndex %= 1024 ;
  }
  
  LoadFrameListIndex = SavedFrameListIndex;
  
  //
  // Execute QH-TD and get result
  //
  //
  // detail status is put into the Result field in the pIRP
  // the Data Toggle value is also re-updated to the value
  // of the last successful TD
  //
  Status = ExecBulkorSyncInterruptTransfer (HcDev,ptrFirstTD,
                                                LoadFrameListIndex,DataLength,
                                                DataToggle,TimeOut,TransferResult);

  //
  // Delete Bulk transfer QH-TD structure
  // and maitain the pointers in the Frame List
  // and other pointers in related QH structure
  //
  // TRUE means must search other framelistindex
  //
  for(i = 0; i <= LinkTimes ; i ++) {
    DelLinkSingleQH (HcDev,ptrQH,LoadFrameListIndex,FALSE,FALSE);
    LoadFrameListIndex += 1;
    LoadFrameListIndex %= 1024 ;
  }
  
  UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
  
  DeleteQueuedTDs (HcDev,ptrFirstTD);
  
  if (Mapping != NULL) {
    HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
  }
  
  //
  // if has errors that cause host controller halt, 
  // then return EFI_DEVICE_ERROR directly.
  //
  if(IsHCHalted(HcDev->PciIo,StatusReg)       || 
     IsHCProcessErr(HcDev->PciIo,StatusReg)   ||
     IsHostSysErr(HcDev->PciIo,StatusReg)) {      
    
    ClearStatusReg(HcDev->PciIo,StatusReg) ;
    *TransferResult |= EFI_USB_ERR_SYSTEM;
    return EFI_DEVICE_ERROR;
  }
  
  ClearStatusReg(HcDev->PciIo,StatusReg);
  
  return Status ;  
}

EFI_STATUS
UHCIAsyncInterruptTransfer (
  IN  EFI_USB_HC_PROTOCOL               *This,
  IN  UINT8                             DeviceAddress,
  IN  UINT8                             EndPointAddress,
  IN  BOOLEAN                           IsSlowDevice,
  IN  UINT8                             MaxiumPacketLength,
  IN  BOOLEAN                           IsNewTransfer,
  IN OUT UINT8                          *DataToggle,
  IN  UINTN                             PollingInterval  OPTIONAL,
  IN  UINTN                             DataLength       OPTIONAL,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK   CallBackFunction OPTIONAL,
  IN  VOID                              *Context         OPTIONAL  
  )
/*++
  
  Routine Description:
    Submits an asynchronous interrupt transfer to an 
    interrupt endpoint of a USB device.
  
  Arguments:
    
    This            A pointer to the EFI_USB_HC_PROTOCOL instance.
    
    DeviceAddress   Represents the address of the target device on the USB,
                    which is assigned during USB enumeration.
                    
    EndPointAddress The combination of an endpoint number and an endpoint 
                    direction of the target USB device. Each endpoint address 
                    supports data transfer in one direction except the 
                    control endpoint (whose default endpoint address is 0). 
                    It is the caller's responsibility to make sure that 
                    the EndPointAddress represents an interrupt endpoint.
                    
    IsSlowDevice    Indicates whether the target device is slow device 
                    or full-speed device.
                    
    MaxiumPacketLength  Indicates the maximum packet size the target endpoint
                        is capable of sending or receiving.
                        
    IsNewTransfer   If TRUE, an asynchronous interrupt pipe is built between
                    the host and the target interrupt endpoint. 
                    If FALSE, the specified asynchronous interrupt pipe 
                    is canceled.
                    
    DataToggle      A pointer to the data toggle value.  On input, it is valid 
                    when IsNewTransfer is TRUE, and it indicates the initial 
                    data toggle value the asynchronous interrupt transfer 
                    should adopt.  
                    On output, it is valid when IsNewTransfer is FALSE, 
                    and it is updated to indicate the data toggle value of 
                    the subsequent asynchronous interrupt transfer.
                    
    PollingInterval Indicates the interval, in milliseconds, that the 
                    asynchronous interrupt transfer is polled.  
                    This parameter is required when IsNewTransfer is TRUE.
                    
    DataLength      Indicates the length of data to be received at the 
                    rate specified by PollingInterval from the target 
                    asynchronous interrupt endpoint.  This parameter 
                    is only required when IsNewTransfer is TRUE.
                    
    CallBackFunction  The Callback function.This function is called at the 
                      rate specified by PollingInterval.This parameter is 
                      only required when IsNewTransfer is TRUE.
                      
    Context         The context that is passed to the CallBackFunction.
                    This is an optional parameter and may be NULL.
  
  Returns:
    EFI_SUCCESS 
        The asynchronous interrupt transfer request has been successfully 
        submitted or canceled.
    EFI_INVALID_PARAMETER 
        Some parameters are invalid.
    EFI_OUT_OF_RESOURCES  
        The request could not be completed due to a lack of resources.  
--*/
{
  USB_HC_DEV            *HcDev;
  UINT32                StatusReg;
  UINT32                FrameNumReg;
  UINTN                 DataLen;
  QH_STRUCT             *ptrFirstQH;
  QH_STRUCT             *ptrQH;
  QH_STRUCT             *ptrPreQH;
  TD_STRUCT             *ptrFirstTD;
  TD_STRUCT             *ptrTD;
  TD_STRUCT             *ptrPreTD;
  UINT16                LoadFrameListIndex;
  UINT16                i;
  UINT8                 PktID;
  UINT8                 *ptr;
  UINT8                 *MappedPtr;
  BOOLEAN               IsFirstTD;
  BOOLEAN               IsFirstQH;
  EFI_STATUS            Status ;
  BOOLEAN               ShortPacketEnable;  
  UINT8                 CurrentDataToggle;  
  EFI_PHYSICAL_ADDRESS  TempPtr;
  VOID                  *Mapping;
  UINT8                 pktsize;
  QH_STRUCT             *TempQH;
  
  HcDev             = USB_HC_DEV_FROM_THIS (This);
  StatusReg         = (UINT32)(USBSTS);
  FrameNumReg       = (UINT32)(USBFRNUM);
  Mapping           = NULL;
  ShortPacketEnable = FALSE;
  
  PktID             = INPUT_PACKET_ID;
  ptrTD             = NULL;
  ptrFirstTD        = NULL;
  ptrPreTD          = NULL;  
  ptr               = NULL; 
  ptrQH             = NULL;
  ptrPreQH          = NULL;
  ptrFirstQH        = NULL;
  
  if ((EndPointAddress & 0x80) == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // delete Async interrupt transfer request
  //
  if (!IsNewTransfer) {
    
    Status = DeleteAsyncINTQHTDs(HcDev,DeviceAddress,EndPointAddress,DataToggle);
    
    return Status ;
  } 
  
  //
  // if has errors that cause host controller halt, 
  // then return EFI_DEVICE_ERROR directly.
  //
  if(IsHCHalted(HcDev->PciIo,StatusReg)       || 
     IsHCProcessErr(HcDev->PciIo,StatusReg)   ||
     IsHostSysErr(HcDev->PciIo,StatusReg)) {
    
    ClearStatusReg(HcDev->PciIo,StatusReg) ;
    return EFI_DEVICE_ERROR;
  }
  
  ClearStatusReg(HcDev->PciIo,StatusReg);

  //
  // submit Async interrupt transfer request
  //
  if (PollingInterval < 1 || PollingInterval > 255) {  
    return EFI_INVALID_PARAMETER;
  }
  
  if (DataLength == 0 ) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((*DataToggle != 1) && (*DataToggle != 0)) {
    return EFI_INVALID_PARAMETER;
  } 

  ShortPacketEnable = TRUE;     
  PktID             = INPUT_PACKET_ID;  
  DataLen           = DataLength;  
  Status = gBS->AllocatePool(EfiBootServicesData,DataLen,&ptr);
  if (EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // BusMasterWrite means cpu read
  //
  Status = HcDev->PciIo->Map (HcDev->PciIo,
                              EfiPciIoOperationBusMasterWrite,
                              ptr,
                              &DataLen,
                              &TempPtr,
                              &Mapping
                              );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (ptr);
    return Status;
  }
  MappedPtr = (UINT8*)((UINTN)TempPtr);

  CurrentDataToggle = *DataToggle;
          
  IsFirstTD = TRUE;

  while(DataLen > 0) {
    //
    // create TD structures and link together 
    //
        
    pktsize = (UINT8)DataLen;
    if (DataLen > MaxiumPacketLength) {      
      pktsize = MaxiumPacketLength;
    }
    
    Status = GenDataTD(HcDev,DeviceAddress, EndPointAddress, 
                       MappedPtr, pktsize, PktID, CurrentDataToggle, 
                       IsSlowDevice,&ptrTD);
    if (EFI_ERROR(Status)) {
      gBS->FreePool (ptr);
      HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
      DeleteQueuedTDs (HcDev,ptrFirstTD);
      return Status;
    }
    //
    // Enable short packet detection. 
    //
    if(ShortPacketEnable) {
      EnableorDisableTDShortPacket (ptrTD, TRUE) ;      
    }
    if(IsFirstTD) {
      ptrFirstTD            = ptrTD ;
      ptrFirstTD->ptrNextTD = NULL;
      IsFirstTD             = FALSE ;
    } else {      
      // Link two TDs in vertical depth
      LinkTDToTD(ptrPreTD,ptrTD) ;
    }
    
    ptrPreTD = ptrTD ;      
    
    CurrentDataToggle ^= 1;
    MappedPtr += pktsize;
    DataLen -= pktsize;
  }
  
  //
  // roll one value back
  //
  CurrentDataToggle ^= 1;
    
  //
  // create a list of QH structures and init,
  // link TDs to all the QHs, and link all the QHs together using internal 
  // defined pointer of the QH_STRUCT.
  //
  IsFirstQH = TRUE;
  ptrPreQH  = NULL;
  for (i = 0 ; i < 1024 ;) {
  
    Status = CreateQH(HcDev,&ptrQH);
    if (EFI_ERROR(Status)) {      
      gBS->FreePool (ptr);
      HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
      DeleteQueuedTDs (HcDev,ptrFirstTD);
      ptrQH = ptrFirstQH;
      while (ptrQH) {
        TempQH = ptrQH;
        ptrQH = TempQH->ptrNextIntQH;
        UhciFreePool(HcDev,(UINT8*)TempQH,sizeof(QH_STRUCT));        
      }
      return Status;
    }
    
    //
    // link TD structures to QH structure
    //
    LinkTDToQH(ptrQH,ptrFirstTD) ;        
    
    if(IsFirstQH) {
      ptrFirstQH                = ptrQH;
      ptrFirstQH->ptrNextIntQH  = NULL;
      IsFirstQH                 = FALSE ;
    } else {      
      //
      // link neighbor QH structures together
      //
      ptrPreQH->ptrNextIntQH = ptrQH;
    }     
      
    ptrPreQH = ptrQH; 
      
    i = (UINT16)(PollingInterval + i) ;      
  }
  //
  // last QH in QH list should set its next QH pointer to NULL.
  //
  ptrQH->ptrNextIntQH = NULL;     

  //
  // Save QH-TD structures in Interrupt transfer list,
  // for monitor interrupt transfer execution routine use.
  //
  InsertQHTDToINTList(HcDev,ptrFirstQH,ptrFirstTD,
                      DeviceAddress,EndPointAddress,
                      CurrentDataToggle,DataLength,PollingInterval,Mapping,ptr,
                      CallBackFunction,Context);

  //
  // put QHs-TDs into  Frame list
  //
  LoadFrameListIndex = (UINT16)((GetCurrentFrameNumber(HcDev->PciIo,FrameNumReg)) % 1024);
  
  ptrQH = ptrFirstQH;    

  for(i = LoadFrameListIndex; i < (1024 + LoadFrameListIndex); ) {
      
    //
    // put the QH-TD directly or indirectly into the proper place 
    // in the Frame List 
    //
    LinkQHToFrameList(HcDev->FrameListEntry,(UINT16)(i %1024), ptrQH);
  
    i = (UINT16)(PollingInterval + i);
    
    ptrQH = ptrQH->ptrNextIntQH;        
  }
  
  return EFI_SUCCESS ;
}
  
EFI_STATUS
UHCISyncInterruptTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  UINT8                  EndPointAddress,
  IN  BOOLEAN                IsSlowDevice,
  IN  UINT8                  MaximumPacketLength,
  IN OUT VOID                *Data,
  IN OUT UINTN               *DataLength,
  IN OUT UINT8               *DataToggle,
  IN  UINTN                  TimeOut,
  OUT UINT32                 *TransferResult
  )
/*++
  
  Routine Description:
    Submits synchronous interrupt transfer to an interrupt endpoint 
    of a USB device.
  
  Arguments:
    
    This            A pointer to the EFI_USB_HC_PROTOCOL instance.
    
    DeviceAddress   Represents the address of the target device on the USB, 
                    which is assigned during USB enumeration.
                    
    EndPointAddress   The combination of an endpoint number and an endpoint 
                      direction of the target USB device. Each endpoint 
                      address supports data transfer in one direction 
                      except the control endpoint (whose default 
                      endpoint address is 0). It is the caller's responsibility
                      to make sure that the EndPointAddress represents 
                      an interrupt endpoint. 
                      
    IsSlowDevice    Indicates whether the target device is slow device 
                    or full-speed device.
                    
    MaximumPacketLength Indicates the maximum packet size the target endpoint 
                        is capable of sending or receiving.
                        
    Data            A pointer to the buffer of data that will be transmitted 
                    to USB device or received from USB device.
                    
    DataLength      On input, the size, in bytes, of the data buffer specified 
                    by Data. On output, the number of bytes transferred.
                    
    DataToggle      A pointer to the data toggle value. On input, it indicates
                    the initial data toggle value the synchronous interrupt 
                    transfer should adopt; 
                    on output, it is updated to indicate the data toggle value 
                    of the subsequent synchronous interrupt transfer. 
                    
    TimeOut         Indicates the maximum time, in microseconds, which the 
                    transfer is allowed to complete.
                    
    TransferResult  A pointer to the detailed result information from 
                    the synchronous interrupt transfer.  

  Returns:
    EFI_SUCCESS 
        The synchronous interrupt transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  
        The synchronous interrupt transfer could not be submitted due 
        to lack of resource.
    EFI_INVALID_PARAMETER 
        Some parameters are invalid.
    EFI_TIMEOUT 
        The synchronous interrupt transfer failed due to timeout.
    EFI_DEVICE_ERROR  
        The synchronous interrupt transfer failed due to host controller 
        or device error. Caller should check TranferResult for detailed 
        error information.  
--*/
{
  USB_HC_DEV            *HcDev;
  UINT32                StatusReg;
  UINT32                FrameNumReg;
  UINTN                 DataLen;  
  QH_STRUCT             *ptrQH ;
  TD_STRUCT             *ptrFirstTD;
  TD_STRUCT             *ptrTD;
  TD_STRUCT             *ptrPreTD;
  UINT16                LoadFrameListIndex;
  UINT16                SavedFrameListIndex;
  UINT32                i;  
  UINT32                LinkTimes;
  UINT8                 PktID;
  UINT8                 *ptrDataSource;
  UINT8                 *ptr;
  BOOLEAN               IsFirstTD;
  EFI_STATUS            Status ;
  BOOLEAN               ShortPacketEnable;  
  EFI_PHYSICAL_ADDRESS  TempPtr;
  VOID                  *Mapping;
  UINT8                 pktsize;
  
  HcDev                 = USB_HC_DEV_FROM_THIS (This);
  StatusReg             = (UINT32)(USBSTS);
  FrameNumReg           = (UINT32)(USBFRNUM);
  ShortPacketEnable     = FALSE;
  Mapping               = NULL;
  PktID                 = INPUT_PACKET_ID;
  ptrTD                 = NULL;
  ptrFirstTD            = NULL;
  ptrPreTD              = NULL;
  DataLen               = 0;
  ptr                   = NULL;  
  i                     = 0;
  LinkTimes             = 0;
  
  //
  // Parameters Checking
  //  
  
  if ((DataLength == NULL) || (Data == NULL) 
      || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // if has errors that cause host controller halt, 
  // then return EFI_DEVICE_ERROR directly.
  //
  if(IsHCHalted(HcDev->PciIo,StatusReg)       || 
     IsHCProcessErr(HcDev->PciIo,StatusReg)   ||
     IsHostSysErr(HcDev->PciIo,StatusReg)) {      
    
    ClearStatusReg(HcDev->PciIo,StatusReg) ;
    *TransferResult = EFI_USB_ERR_SYSTEM;
    return EFI_DEVICE_ERROR;
  }
  
  if ((EndPointAddress & 0x80) == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (*DataLength == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((*DataToggle != 1) && (*DataToggle != 0)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (MaximumPacketLength > 64) {
    return EFI_INVALID_PARAMETER;
  } 
  
  if (IsSlowDevice && (MaximumPacketLength > 8)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (TransferResult == NULL) {
    return EFI_INVALID_PARAMETER;
  }  
  
  ClearStatusReg(HcDev->PciIo,StatusReg);
  
  //
  // submit Sync interrupt transfer request
  //
  ShortPacketEnable = TRUE;
  PktID             = INPUT_PACKET_ID;  
  DataLen           = *DataLength;
  ptrDataSource     = Data;
  
  //
  // create QH structure and init
  //
  Status = CreateQH(HcDev,&ptrQH);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  //
  // BusMasterWrite means cpu read
  //
  Status = HcDev->PciIo->Map (HcDev->PciIo,
                    EfiPciIoOperationBusMasterWrite,
                    ptrDataSource,
                    &DataLen,
                    &TempPtr,
                    &Mapping
                    );
  if (EFI_ERROR(Status)) {
    UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
    return Status;
  }
  ptr = (UINT8*)((UINTN)TempPtr);
  
  IsFirstTD = TRUE;
  while(DataLen > 0) {
    //
    // create TD structures and link together 
    //

    pktsize = (UINT8)DataLen;
    if (DataLen > MaximumPacketLength) {
      pktsize = MaximumPacketLength;
    }
    
    Status = GenDataTD(HcDev,DeviceAddress, EndPointAddress, 
                       ptr, pktsize, PktID, *DataToggle, IsSlowDevice,&ptrTD);
    if (EFI_ERROR(Status)) {
      UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
      HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);
      DeleteQueuedTDs (HcDev,ptrFirstTD);
      return Status;
    }
    //
    // Enable short packet detection. 
    //
    if (ShortPacketEnable) {
      EnableorDisableTDShortPacket (ptrTD, TRUE) ;      
    }    
    if(IsFirstTD) {
      ptrFirstTD            = ptrTD ;
      ptrFirstTD->ptrNextTD = NULL;
      IsFirstTD             = FALSE ;
    } else {      
      // Link two TDs in vertical depth
      LinkTDToTD(ptrPreTD,ptrTD) ;
    }
    
    i ++;
          
    ptrPreTD = ptrTD ;      
      
    *DataToggle ^= 1;
    ptr += pktsize;
    DataLen -= pktsize;
  }
  
  //
  // link TD structures to QH structure
  //
  LinkTDToQH(ptrQH,ptrFirstTD) ;
  
  switch(MaximumPacketLength) {
    case 8:
      LinkTimes = i / 71 + 1;
      break;
    case 16:
      LinkTimes = i / 51 + 1;
      break;
    case 32:
      LinkTimes = i / 33 + 1;
      break;
    case 64:
      LinkTimes = i / 19 + 1;
      break;
  }
  LinkTimes += 100;   // redundant value
  
  LoadFrameListIndex = (UINT16)((GetCurrentFrameNumber(HcDev->PciIo,FrameNumReg)) % 1024);
  SavedFrameListIndex = LoadFrameListIndex;

  for(i = 0; i < LinkTimes; i++ ) {
        
    //
    // put the QH-TD directly or indirectly into the proper place 
    // in the Frame List 
    //
    LinkQHToFrameList(HcDev->FrameListEntry,LoadFrameListIndex, ptrQH) ;
    
    LoadFrameListIndex += 1 ;
    LoadFrameListIndex %= 1024 ;        
  }
  
  LoadFrameListIndex = SavedFrameListIndex;
  //
  // detail status is put into the Result field in the pIRP
  // the Data Toggle value is also re-updated to the value
  // of the last successful TD
  //
  Status = ExecBulkorSyncInterruptTransfer (HcDev,ptrFirstTD,
                                            LoadFrameListIndex,DataLength,
                                            DataToggle,TimeOut,TransferResult);
  //
  // Delete Sync Interrupt transfer QH-TD structure
  // and maintain the pointers in the Frame List
  // and other pointers in related QH structure
  //
  // TRUE means must search other framelistindex
  //
  for(i = 0; i <= LinkTimes ; i ++) {
    DelLinkSingleQH (HcDev,ptrQH,LoadFrameListIndex,FALSE,FALSE);
    LoadFrameListIndex += 1;
    LoadFrameListIndex %= 1024 ;
  }
  UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
  
  DeleteQueuedTDs (HcDev,ptrFirstTD);
  
  HcDev->PciIo->Unmap (HcDev->PciIo,Mapping);

  //
  // if has errors that cause host controller halt, 
  // then return EFI_DEVICE_ERROR directly.
  //
  if(IsHCHalted(HcDev->PciIo,StatusReg)       || 
     IsHCProcessErr(HcDev->PciIo,StatusReg)   ||
     IsHostSysErr(HcDev->PciIo,StatusReg)) {      
    
    ClearStatusReg(HcDev->PciIo,StatusReg) ;
    *TransferResult |= EFI_USB_ERR_SYSTEM;
    return EFI_DEVICE_ERROR;
  }
  
  ClearStatusReg(HcDev->PciIo,StatusReg);
  
  return Status ;  
}

EFI_STATUS
UHCIIsochronousTransfer (
  IN  EFI_USB_HC_PROTOCOL    *This,
  IN  UINT8                  DeviceAddress,
  IN  UINT8                  EndPointAddress,
  IN  UINT8                  MaximumPacketLength,
  IN OUT VOID                *Data,
  IN OUT UINTN               DataLength,
  OUT UINT32                 *TransferResult
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
UHCIAsyncIsochronousTransfer (
  IN  EFI_USB_HC_PROTOCOL               *This,
  IN  UINT8                             DeviceAddress,
  IN  UINT8                             EndPointAddress,
  IN  UINT8                             MaximumPacketLength,
  IN OUT VOID                           *Data,
  IN  UINTN                             DataLength,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK   IsochronousCallBack,
  IN  VOID                              *Context   OPTIONAL
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/
{
  return EFI_UNSUPPORTED;
}  
  
VOID
MonitorInterruptTrans(
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/            
{

  USB_HC_DEV          *HcDev ;
  INTERRUPT_LIST      *PtrList;
  EFI_LIST_ENTRY      *Link;
  UINT32              Result;
  VOID                *DataBuffer;
  UINTN               DataLen;
  UINTN               ActualLen;  
  UINTN               ErrTDPos ;
  UINT32              StatusAddr;
  EFI_STATUS          Status;
  EFI_LIST_ENTRY      *NextLink;

  HcDev       = (USB_HC_DEV*)Context;
  StatusAddr  = (UINT32)(USBSTS);
  
  //
  // interrupt transfer list is empty, means that no interrupt transfer 
  // is submitted by far.
  //
  if (IsListEmpty(&(HcDev->InterruptListHead))) {
    return;
  }
  
  NextLink = HcDev->InterruptListHead.ForwardLink;
  do {

    Link = NextLink;
    NextLink = Link->ForwardLink;
    
    PtrList = INTERRUPT_LIST_FROM_LINK(Link); 
    
    //
    // get TD execution results.
    // ErrTDPos is zero-based value indicating the first error TD's position 
    // in the TDs' list.
    // This value is only valid when Result not equal NOERROR.
    //
    ExecuteAsyncINTTDs(HcDev,PtrList,&Result,&ErrTDPos,&ActualLen);
    
    //
    // interrupt transfer has not been executed yet.
    //
    if(((Result & EFI_USB_ERR_NAK) == EFI_USB_ERR_NAK) 
        || ((Result & EFI_USB_ERR_NOTEXECUTE) == EFI_USB_ERR_NOTEXECUTE)) {
      continue;
    }
    //
    // get actual data length transferred data and its data length.
    //
    DataLen = ActualLen;
    Status = gBS->AllocatePool (EfiBootServicesData,DataLen,&DataBuffer);
    if (EFI_ERROR(Status)) {
      return;
    }
    
    EfiCopyMem(DataBuffer,PtrList->ptrFirstTD->pTDBuffer,DataLen);
    
    //
    // only if interrupt endpoint responds 
    // and the interrupt transfer stops because of completion
    // or error, then we will call callback function.
    //
    if(Result == EFI_USB_NOERROR) {
      
      //
      // add for real platform debug
      //
      if (PtrList->InterruptCallBack != NULL) {
        (PtrList->InterruptCallBack)(DataBuffer,DataLen,PtrList->InterruptContext,Result);
      }
      
      if (DataBuffer) {
        gBS->FreePool(DataBuffer);
      }
      
      //
      // update should done after data buffer got.
      //          
      UpdateAsyncINTQHTDs(PtrList,Result, (UINT32)ErrTDPos);

    } else {
        
      //DEBUG((EFI_D_ERROR, "interrupt transfer error code is %x\n", Result));
      
      if (DataBuffer) {
        gBS->FreePool(DataBuffer);
      }
      
      //
      // leave error recovery to its related device driver.
      // A common case of the error recovery is to re-submit the interrupt
      // transfer.
      // When an interrupt transfer is re-submitted, its position in the linked
      // list is changed. It is inserted to the head of the linked list, while
      // this function scans the whole list from head to tail. Thus, the 
      // re-submitted interrupt transfer's callback function will not be called
      // again in this round.
      //
      if (PtrList->InterruptCallBack != NULL) {
        (PtrList->InterruptCallBack)(NULL,0,PtrList->InterruptContext,Result) ;
      }      
    }    
  } while (NextLink != &(HcDev->InterruptListHead));
  
}