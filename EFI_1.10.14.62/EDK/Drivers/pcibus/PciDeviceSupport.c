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

  PciDeviceSupport.c
  
Abstract:

  This file provides routine to support Pci device node manipulation

Revision History

--*/

#include "Pcibus.h"

//
// This device structure is serviced as a header.
// Its Next field points to the first root bridge device node
// 
EFI_LIST_ENTRY       gPciDevicePool;

EFI_STATUS
InitializePciDevicePool(
  VOID
)
/*++

Routine Description:

  Initialize the gPciDevicePool

Arguments:

Returns:

  None

--*/
{
  InitializeListHead(&gPciDevicePool);

  return EFI_SUCCESS;
}

EFI_STATUS 
InsertRootBridge (
  PCI_IO_DEVICE *RootBridge
)
/*++

Routine Description:

  Insert a root bridge into PCI device pool

Arguments:

Returns:

  None

--*/
{

  InsertTailList (&gPciDevicePool, &(RootBridge->Link));

  return EFI_SUCCESS;
}


EFI_STATUS 
InsertPciDevice (
  PCI_IO_DEVICE *Bridge,
  PCI_IO_DEVICE *PciDeviceNode) 
/*++

Routine Description:

  This function is used to insert a PCI device node under
  a bridge

Arguments:

Returns:

  None

--*/


{

  InsertTailList(&Bridge->ChildList, &(PciDeviceNode->Link));
  PciDeviceNode->Parent = Bridge;

  return EFI_SUCCESS;
}


EFI_STATUS 
DestroyRootBridge ( 
   IN PCI_IO_DEVICE *RootBridge 
) 
/*++

Routine Description:

  
Arguments:

Returns:

  None

--*/
{
  DestroyPciDeviceTree (RootBridge);

  gBS->FreePool (RootBridge);

  return EFI_SUCCESS;
}

EFI_STATUS 
DestroyPciDeviceTree ( 
   IN PCI_IO_DEVICE *Bridge 
) 
/*++

Routine Description:

  Destroy all the pci device node under the bridge.
  Bridge itself is not included.

Arguments:

Returns:

  None

--*/
{
  EFI_LIST_ENTRY  *CurrentLink;
  PCI_IO_DEVICE   *Temp;
  

  while (!IS_LIST_EMPTY(&Bridge->ChildList)) {

    CurrentLink = Bridge->ChildList.ForwardLink;

    //
    // Remove this node from the linked list
    //
    REMOVE_ENTRY_LIST(CurrentLink);

    Temp = PCI_IO_DEVICE_FROM_LINK(CurrentLink);

    if (IS_PCI_BRIDGE (&(Temp->Pci))) {
      DestroyPciDeviceTree (Temp);
    }
    gBS->FreePool (Temp);
  }
  
  return EFI_SUCCESS;
}


EFI_STATUS 
DestroyRootBridgeByHandle (
   EFI_HANDLE Controller
)
/*++

Routine Description:

  Destroy all device nodes under the root bridge
  specified by Controller. 
  The root bridge itself is also included.

Arguments:

Returns:

  None

--*/
{

  EFI_LIST_ENTRY  *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  CurrentLink = gPciDevicePool.ForwardLink;

  while (CurrentLink && CurrentLink != &gPciDevicePool) {
    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (Temp->Handle == Controller) {

      REMOVE_ENTRY_LIST(CurrentLink);

      DestroyPciDeviceTree(Temp);

      gBS->FreePool(Temp);

      return EFI_SUCCESS;
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS 
RegisterPciDevice ( 
  IN  EFI_HANDLE                     Controller,
  IN  PCI_IO_DEVICE                  *PciIoDevice,
  OUT EFI_HANDLE                     *Handle
  ) 
/*++

Routine Description:

  This function is used to register the PCI device to the EFI,
  create a handle for this PCI device,then attach apporpriate protocols
  onto the handle.

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS            Status;
  
  //
  // Install the pciio protocol, device path protocol and 
  // Bus Specific Driver Override Protocol
  //

  if (PciIoDevice->BusOverride) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                  &PciIoDevice->Handle,             
                  &gEfiDevicePathProtocolGuid,               PciIoDevice->DevicePath,
                  &gEfiPciIoProtocolGuid,                    &PciIoDevice->PciIo,
                  &gEfiBusSpecificDriverOverrideProtocolGuid,&PciIoDevice->PciDriverOverride,
                  NULL
                  );
  } else {
    Status = gBS->InstallMultipleProtocolInterfaces (
                  &PciIoDevice->Handle,             
                  &gEfiDevicePathProtocolGuid,               PciIoDevice->DevicePath,
                  &gEfiPciIoProtocolGuid,                    &PciIoDevice->PciIo,
                  NULL
                  );
  }

  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    Status = gBS->OpenProtocol (
                    Controller,           
                    &gEfiPciRootBridgeIoProtocolGuid, 
                    (VOID **)&(PciIoDevice->PciRootBridgeIo),
                    gPciBusDriverBinding.DriverBindingHandle,
                    PciIoDevice->Handle,   
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (Handle) {
    *Handle = PciIoDevice->Handle;
  }

  //
  // Indicate the pci device is registered
  //
  PciIoDevice->Registered = TRUE;
  
  return EFI_SUCCESS;
}


EFI_STATUS 
DeRegisterPciDevice ( 
  IN  EFI_HANDLE                     Controller,
  IN  EFI_HANDLE                     Handle
  ) 
/*++

Routine Description:

  This function is used to de-register the PCI device from the EFI,
  That includes un-installing PciIo protocol from the specified PCI 
  device handle.

Arguments:

Returns:

  None

--*/
{
  EFI_PCI_IO_PROTOCOL               *PciIo;
  EFI_STATUS                        Status;
  PCI_IO_DEVICE                     *PciIoDevice;
  PCI_IO_DEVICE                     *Node;
  EFI_LIST_ENTRY                    *CurrentLink;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *PciRootBridgeIo;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiPciIoProtocolGuid,  
                  (VOID **)&PciIo,
                  gPciBusDriverBinding.DriverBindingHandle,
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    PciIoDevice = PCI_IO_DEVICE_FROM_PCI_IO_THIS (PciIo);

    //
    // If it is already de-registered 
    //
    if (PciIoDevice->Registered == FALSE) {
      return EFI_SUCCESS;
    }

    //
    // If it is PPB, first de-register its children
    //
    if (IS_PCI_BRIDGE (&(PciIoDevice->Pci))) {

      CurrentLink = PciIoDevice->ChildList.ForwardLink;

      while (CurrentLink && CurrentLink != &PciIoDevice->ChildList) {
        Node = PCI_IO_DEVICE_FROM_LINK(CurrentLink);
        Status = DeRegisterPciDevice (Controller, Node->Handle);

        if (EFI_ERROR (Status)) {
          return Status;
        }
        CurrentLink = CurrentLink->ForwardLink;
      }
    }

    //
    // First disconnect this device
    //
//    PciIoDevice->PciIo.Attributes(&(PciIoDevice->PciIo),
//                                    EfiPciIoAttributeOperationDisable,
//                                    EFI_PCI_DEVICE_ENABLE,
//                                    NULL
//                                    );
       
    //
    // Close the child handle
    //
    Status = gBS->CloseProtocol (
                    Controller, 
                    &gEfiPciRootBridgeIoProtocolGuid, 
                    gPciBusDriverBinding.DriverBindingHandle,
                    Handle
                    );

    //
    // Un-install the device path protocol and pci io protocol
    //

    if (PciIoDevice->BusOverride) {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Handle, 
                      &gEfiDevicePathProtocolGuid,                PciIoDevice->DevicePath,
                      &gEfiPciIoProtocolGuid,                     &PciIoDevice->PciIo,
                      &gEfiBusSpecificDriverOverrideProtocolGuid, &PciIoDevice->PciDriverOverride,
                      NULL
                      );
    } else {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Handle, 
                      &gEfiDevicePathProtocolGuid,      PciIoDevice->DevicePath,
                      &gEfiPciIoProtocolGuid,           &PciIoDevice->PciIo,
                      NULL
                      );
    }

    if (EFI_ERROR (Status)) {
      gBS->OpenProtocol (
             Controller,
             &gEfiPciRootBridgeIoProtocolGuid, 
             (VOID **)&PciRootBridgeIo,
             gPciBusDriverBinding.DriverBindingHandle,
             Handle,   
             EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
             );
      return Status;
    }
    
    //
    // Restore the register field to the original value
    //
    PciIoDevice->Registered = FALSE;
    PciIoDevice->Handle = NULL;
  } else {

    //
    // Handle may be closed before
    //
    return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

EFI_STATUS 
StartPciDevicesOnBridge ( 
 IN EFI_HANDLE                          Controller,
 IN PCI_IO_DEVICE                       *RootBridge,
 IN EFI_DEVICE_PATH_PROTOCOL            *RemainingDevicePath 
 ) 
/*++

Routine Description:

  Start to manage the PCI device on specified the root bridge or PCI-PCI Bridge

Arguments:

Returns:

  None

--*/
{
  PCI_IO_DEVICE             *Temp;
  PCI_IO_DEVICE             *PciIoDevice;
  EFI_DEV_PATH_PTR          Node;
  EFI_DEVICE_PATH_PROTOCOL  *CurrentDevicePath;
  EFI_STATUS                Status;
  EFI_LIST_ENTRY            *CurrentLink;
    
  CurrentLink = RootBridge->ChildList.ForwardLink;

  while (CurrentLink && CurrentLink != &RootBridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK(CurrentLink);
    if (RemainingDevicePath) {
      
      Node.DevPath = RemainingDevicePath;

      if (Node.Pci->Device != Temp->DeviceNumber ||
            Node.Pci->Function != Temp->FunctionNumber) {
        CurrentLink = CurrentLink->ForwardLink;
        continue;
      }

      //
      // Check if the device has been assigned with required resource
      //
      if (!Temp->Allocated) {
        return  EFI_NOT_READY;
      }

      
      //
      // Check if the current node has been registered before
      // If it is not, register it
      //
      if (!Temp->Registered) {
        PciIoDevice = Temp;

        Status = RegisterPciDevice (
                   Controller,
                   PciIoDevice,
                   NULL
                   );
      }
      
      //
      // Get the next device path
      //
      CurrentDevicePath = EfiNextDevicePathNode (RemainingDevicePath);
      if (EfiIsDevicePathEnd (CurrentDevicePath)) {
        return EFI_SUCCESS;
      }
  
      //
      //If it is a PPB
      if (IS_PCI_BRIDGE (&(Temp->Pci))) {
        Status = StartPciDevicesOnBridge (
                   Controller,
                   Temp,
                   CurrentDevicePath
                   );

        Temp->PciIo.Attributes(&(Temp->PciIo),
                                    EfiPciIoAttributeOperationEnable,
                                    EFI_PCI_DEVICE_ENABLE,
                                    NULL
                                    );
        
        return Status;
      } else {

        //
        // Currently, the PCI bus driver only support PCI-PCI bridge 
        //
        return EFI_UNSUPPORTED;
      }
       
    } else {

      //
      // If remaining device path is NULL, 
      // try to enable all the pci devices under this bridge
      //

      if (!Temp->Registered && Temp->Allocated) {

        PciIoDevice = Temp;

        Status = RegisterPciDevice (
                   Controller,
                   PciIoDevice,
                   NULL
                   );
         
      }

      if (IS_PCI_BRIDGE (&(Temp->Pci))) {
          Status = StartPciDevicesOnBridge ( 
                     Controller,
                     Temp,
                     RemainingDevicePath
                     );

          Temp->PciIo.Attributes(&(Temp->PciIo),
                                    EfiPciIoAttributeOperationEnable,
                                    EFI_PCI_DEVICE_ENABLE,
                                    NULL
                                    );
       
      }

      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }
  }

  return EFI_NOT_FOUND;
}


EFI_STATUS 
StartPciDevices ( 
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
/*++

Routine Description:

  Start to manage the PCI device according to RemainingDevicePath
  If RemainingDevicePath == NULL, the PCI bus driver will start 
  to manage all the PCI devices it found previously

Arguments:

Returns:

  None

--*/
{
  EFI_DEV_PATH_PTR  Node;
  PCI_IO_DEVICE     *RootBridge;
  EFI_LIST_ENTRY    *CurrentLink;

  if (RemainingDevicePath) {

    //
    // Check if the RemainingDevicePath is valid
    //
    Node.DevPath = RemainingDevicePath;
    if (Node.DevPath->Type != HARDWARE_DEVICE_PATH ||
          Node.DevPath->SubType != HW_PCI_DP &&
          DevicePathNodeLength (Node.DevPath) != sizeof (PCI_DEVICE_PATH)) {
      return EFI_UNSUPPORTED;
    }
  }
    
  CurrentLink = gPciDevicePool.ForwardLink;

  while (CurrentLink && CurrentLink != &gPciDevicePool) {

    RootBridge = PCI_IO_DEVICE_FROM_LINK(CurrentLink);
    //
    // Locate the right root bridge to start
    //
    if (RootBridge->Handle == Controller) {
      StartPciDevicesOnBridge ( 
        Controller, 
        RootBridge,
        RemainingDevicePath
        );
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return EFI_SUCCESS;
}


PCI_IO_DEVICE *
CreateRootBridge (
  IN EFI_HANDLE RootBridgeHandle 
 )
/*++

Routine Description:


Arguments:

Returns:

  None

--*/

{

  EFI_STATUS    Status;
  PCI_IO_DEVICE *Dev;

  Dev = NULL;
  Status = gBS->AllocatePool (
                  EfiBootServicesData, 
                  sizeof(PCI_IO_DEVICE) ,
                  (VOID **)&Dev
                  );

  if (EFI_ERROR (Status)) {
      return NULL;
  }

  EfiZeroMem (Dev, sizeof(PCI_IO_DEVICE) );
  Dev->Signature = PCI_IO_DEVICE_SIGNATURE;
  Dev->Handle = RootBridgeHandle;
  InitializeListHead(&Dev->ChildList);
  return Dev;
}

PCI_IO_DEVICE* 
GetRootBridgeByHandle (
   EFI_HANDLE RootBridgeHandle
)
/*++

Routine Description:


Arguments:

Returns:

  None

--*/
{
  PCI_IO_DEVICE     *RootBridgeDev;
  EFI_LIST_ENTRY    *CurrentLink;

  CurrentLink = gPciDevicePool.ForwardLink;
  
  while (CurrentLink && CurrentLink != &gPciDevicePool) {

    RootBridgeDev = PCI_IO_DEVICE_FROM_LINK(CurrentLink);
    if (RootBridgeDev->Handle == RootBridgeHandle) {
      return RootBridgeDev;
    }
    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}


BOOLEAN 
RootBridgeExisted( 
  IN EFI_HANDLE RootBridgeHandle 
)
/*++

Routine Description:

  This function searches if RootBridgeHandle has already existed
  in current device pool.

  If so, it means the given root bridge has been already enumerated.

Arguments:

Returns:

  None

--*/
 
{
  PCI_IO_DEVICE     *Bridge;

  Bridge = GetRootBridgeByHandle(RootBridgeHandle);

  if (Bridge) {
    return TRUE;
  }
  
  return FALSE;
}


BOOLEAN 
PciDeviceExisted( 
  IN PCI_IO_DEVICE    *Bridge,
  IN PCI_IO_DEVICE    *PciIoDevice
)
/*++

Routine Description:
  
Arguments:

Returns:

  None

--*/
 
{

  PCI_IO_DEVICE     *Temp;
  EFI_LIST_ENTRY    *CurrentLink;

  CurrentLink = Bridge->ChildList.ForwardLink;

  while (CurrentLink && CurrentLink != &Bridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK(CurrentLink);

    if (Temp == PciIoDevice) {
      return TRUE;
    }

    if (!IS_LIST_EMPTY(&Temp->ChildList)) {
      if (PciDeviceExisted(Temp,PciIoDevice)) {
        return TRUE;
      }
    }
    
    CurrentLink = CurrentLink->ForwardLink;
  }
 
  return FALSE;
}

PCI_IO_DEVICE*
ActiveVGADeviceOnTheSameSegment(
  IN PCI_IO_DEVICE        *VgaDevice
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_LIST_ENTRY            *CurrentLink;
  PCI_IO_DEVICE             *Temp;

  CurrentLink = gPciDevicePool.ForwardLink;

  while (CurrentLink && CurrentLink != &gPciDevicePool) {

    Temp = PCI_IO_DEVICE_FROM_LINK(CurrentLink);

    if (Temp->PciRootBridgeIo->SegmentNumber == VgaDevice->PciRootBridgeIo->SegmentNumber) {

      Temp = ActiveVGADeviceOnTheRootBridge (Temp);

      if (Temp) {
        return Temp;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}


PCI_IO_DEVICE*
ActiveVGADeviceOnTheRootBridge(
  IN PCI_IO_DEVICE        *RootBridge
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_LIST_ENTRY            *CurrentLink;
  PCI_IO_DEVICE             *Temp;
 
  CurrentLink = RootBridge->ChildList.ForwardLink;

  while (CurrentLink && CurrentLink != &RootBridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK(CurrentLink);

    if (IS_PCI_VGA(&Temp->Pci) && 
      (Temp->Attributes & (EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_IO))) {
      return Temp;
    }

    if (IS_PCI_BRIDGE(&Temp->Pci)) {

      Temp = ActiveVGADeviceOnTheRootBridge (Temp);

      if (Temp) {
        return Temp;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}
