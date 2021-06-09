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

    Pci.c
    
Abstract:

    Pci functions that can be used to build device Paths.



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Pci22.h" 

#define EFI_ROOT_BRIDGE_LIST    'eprb'
typedef struct {
    UINTN           Signature;

    UINT16          BridgeNumber;
    UINT16          PrimaryBus;
    UINT16          SubordinateBus;

    EFI_DEVICE_PATH *DevicePath;

    LIST_ENTRY      Link;
} PCI_ROOT_BRIDGE_ENTRY;

extern EFI_DEVICE_IO_INTERFACE   *GlobalIoFncs;
extern LIST_ENTRY                PciRootBusList; 


VOID
PrintPciRootBridgeInfo (
    IN LIST_ENTRY   *ListHead
    );

EFI_DEVICE_PATH *
AppendPciDevicePath (
    IN      UINT8               Bus,
    IN      UINT8               Device,
    IN      UINT8               Func,
    IN      EFI_DEVICE_PATH     *DevicePath, 
    IN  OUT UINT16              *BridgePrimaryBus,
    IN  OUT UINT16              *BridgeSubordinateBus,
	OUT     EFI_STATUS          *Status 
    );

EFI_STATUS
PciFindDeviceClass (
    IN  OUT EFI_PCI_ADDRESS_UNION   *ClassAddress,
    IN      UINT8                   BaseClass,
    IN      UINT8                   SubClass
    )
/*++

    Find the First device that matches BassClass & SubClass

--*/
{
    UINT16       Bus;
    UINT8        Device, Func;
    UINT64       Address;
    PCI_TYPE00   Pci;

    //
    // BugBug: Shutdown PCI bus scan
    //
    for (Bus = 0; Bus <= PCI_MAX_BUS; Bus++) {
        for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
            for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {
                Address = EFI_PCI_ADDRESS (Bus, Device, Func, 0);
                ZeroMem(&Pci,sizeof(PCI_TYPE00));
                GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, sizeof(UINT16), &Pci);
                if (Pci.Hdr.VendorId == 0xffff) {
                    break;
                } else {
                    GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, sizeof(PCI_TYPE00)/sizeof(UINT32), &Pci);
                    if (Pci.Hdr.ClassCode[2] == BaseClass && 
                        Pci.Hdr.ClassCode[1] == SubClass) {
                        ClassAddress->EfiAddress.Bus = (UINT8)Bus;
                        ClassAddress->EfiAddress.Device = Device;
                        ClassAddress->EfiAddress.Function = Func;
                        return EFI_SUCCESS;
                    }
                    if (Func == 0 && !(Pci.Hdr.HeaderType & HEADER_TYPE_MULTI_FUNCTION)) {
                        //
                        // Skip sub functions, this is not a multi function device
                        //
                        Func = 8;
                    }
                }
            }
        }
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS
PciFindDevice (
    IN  OUT EFI_PCI_ADDRESS_UNION   *DeviceAddress,
    IN      UINT16                  VendorId,
    IN      UINT16                  DeviceId,
    IN OUT  PCI_TYPE00              *Pci
    )
/*++

    Find the First device that matches VendorId & DeviceId

--*/
{
    UINT16       Bus;
    UINT8        Device, Func;
    UINT64       Address;

    //
    // BugBug: Shutdown PCI bus scan
    //
    for (Bus = 0; Bus <= PCI_MAX_BUS; Bus++) {
        for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
            for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {
                Address = EFI_PCI_ADDRESS (Bus, Device, Func, 0);
                ZeroMem(Pci,sizeof(PCI_TYPE00));
                GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, sizeof(UINT16), Pci);
                if (Pci->Hdr.VendorId == 0xffff) {
                    break;
                } else {
                    GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, sizeof(PCI_TYPE00)/sizeof(UINT32), Pci);
                    if (Pci->Hdr.VendorId == VendorId && 
                        Pci->Hdr.DeviceId == DeviceId) {
                        DeviceAddress->EfiAddress.Bus = (UINT8)Bus;
                        DeviceAddress->EfiAddress.Device = Device;
                        DeviceAddress->EfiAddress.Function = Func;
                        return EFI_SUCCESS;
                    }
                    if (Func == 0 && !(Pci->Hdr.HeaderType & HEADER_TYPE_MULTI_FUNCTION)) {
                        //
                        // Skip sub functions, this is not a multi function device
                        //
                        Func = 8;
                    }
                }
            }
        }
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS
PciDevicePath (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN UINT64                           Address,
    IN OUT EFI_DEVICE_PATH              **PciDevicePath
    )
{
    PCI_ROOT_BRIDGE_ENTRY   *Entry;
    LIST_ENTRY              *Item;
    UINT16                  PrimaryBus, SubordinateBus; 
    UINT8                   Bus, Device, Func;
    EFI_PCI_ADDRESS_UNION   AddressUnion;
    ACPI_HID_DEVICE_PATH    AcpiNode;
	EFI_STATUS   Status;

    if (IsListEmpty(&PciRootBusList)) {
        return EFI_UNSUPPORTED;
    }

    AddressUnion.Address = Address;

    Bus = AddressUnion.EfiAddress.Bus;
    Device = AddressUnion.EfiAddress.Device;
    Func = AddressUnion.EfiAddress.Function;

    for (Item = PciRootBusList.Flink; Item != &PciRootBusList; Item = Item->Flink) {
        Entry = CR(Item, PCI_ROOT_BRIDGE_ENTRY, Link, EFI_ROOT_BRIDGE_LIST);
        if (Bus >= Entry->PrimaryBus && Bus <= Entry->SubordinateBus) {
            break;
        }
    }

    if (Item == &PciRootBusList) {
        return EFI_UNSUPPORTED;
    }

    ZeroMem (&AcpiNode, sizeof(AcpiNode));
    AcpiNode.Header.Type = ACPI_DEVICE_PATH;
    AcpiNode.Header.SubType = ACPI_DP;
    SetDevicePathNodeLength (&AcpiNode.Header, sizeof(AcpiNode));
    AcpiNode.HID = EISA_PNP_ID(0x0A03);
    AcpiNode.UID = Entry->BridgeNumber;

    *PciDevicePath = AppendDevicePathNode (EndDevicePath, &AcpiNode.Header);
    if (*PciDevicePath == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }
    
    PrimaryBus = Entry->PrimaryBus;
    SubordinateBus = Entry->SubordinateBus;
    do {
        *PciDevicePath = AppendPciDevicePath (
                             Bus, Device, Func, 
                             *PciDevicePath,
                             &PrimaryBus, &SubordinateBus,
							 &Status
                             );
        if (*PciDevicePath == NULL) {
            return EFI_OUT_OF_RESOURCES;
        }
    } while (PrimaryBus != 0xffff);

    if (EFI_ERROR (Status)) {
        return EFI_UNSUPPORTED;
	}

    return EFI_SUCCESS;
}

EFI_DEVICE_PATH *
AppendPciDevicePath (
    IN      UINT8               Bus,
    IN      UINT8               Device,
    IN      UINT8               Function,
    IN      EFI_DEVICE_PATH     *DevicePath, 
    IN  OUT UINT16              *BridgePrimaryBus,
    IN  OUT UINT16              *BridgeSubordinateBus,
	OUT     EFI_STATUS          *Status 
    )
{
    UINT16              ThisBus; 
    UINT8               ThisDevice, ThisFunc;
    UINT64              Address;
    PCI_TYPE01          PciBridge, *PciPtr;
    EFI_DEVICE_PATH     *ReturnDevicePath;
    PCI_DEVICE_PATH     PciNode;    

    //
    // BugBug: Check to see if PtoP bridges are leagal on functions other than 0. Fix code if not true
    //

    PciPtr = &PciBridge;
    ReturnDevicePath=DevicePath;
	*Status = EFI_SUCCESS;
    for (ThisBus = *BridgePrimaryBus; ThisBus <= *BridgeSubordinateBus; ThisBus++) {
        for (ThisDevice = 0; ThisDevice <= PCI_MAX_DEVICE; ThisDevice++) {
            for (ThisFunc = 0; ThisFunc <= PCI_MAX_FUNC; ThisFunc++) {
                Address = EFI_PCI_ADDRESS (ThisBus, ThisDevice, ThisFunc, 0);
                ZeroMem(PciPtr,sizeof(PCI_TYPE01));
                GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, 1, &(PciPtr->Hdr.VendorId));
                if (PciPtr->Hdr.VendorId == 0xffff) {
					if (ThisBus==Bus && ThisDevice==Device && ThisFunc==Function){
                        *BridgePrimaryBus = 0xffff;
                        *BridgeSubordinateBus = 0xffff;
						*Status = EFI_NOT_FOUND;
                        return ReturnDevicePath;
                	}	
                    break;
                } else {
                    GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, sizeof(PCI_TYPE01)/sizeof(UINT32), PciPtr);
                    if (IS_PCI_BRIDGE(PciPtr)) {
                        if (Bus >= PciPtr->Bridge.SecondaryBus && 
                            Bus <= PciPtr->Bridge.SubordinateBus) {

                            PciNode.Header.Type = HARDWARE_DEVICE_PATH;
                            PciNode.Header.SubType = HW_PCI_DP;
                            SetDevicePathNodeLength (&PciNode.Header, sizeof(PciNode));

                            PciNode.Device = ThisDevice;
                            PciNode.Function = ThisFunc;
                            ReturnDevicePath = AppendDevicePathNode (DevicePath, &PciNode.Header);

                            *BridgePrimaryBus = PciPtr->Bridge.SecondaryBus;
                            *BridgeSubordinateBus = PciPtr->Bridge.SubordinateBus;
                            return ReturnDevicePath;
                        }
                    }
                    if (ThisFunc == 0 && !(PciPtr->Hdr.HeaderType & HEADER_TYPE_MULTI_FUNCTION)) {
                        //
                        // Skip sub functions, this is not a multi function device
                        //
                        ThisFunc = 8;
                    }
                }
            }
        }
    }

    ZeroMem (&PciNode, sizeof(PciNode));
    PciNode.Header.Type = HARDWARE_DEVICE_PATH;
    PciNode.Header.SubType = HW_PCI_DP;
    SetDevicePathNodeLength (&PciNode.Header, sizeof(PciNode));
    PciNode.Device = Device;
    PciNode.Function = Function;

    ReturnDevicePath = AppendDevicePathNode (DevicePath, &PciNode.Header);

    *BridgePrimaryBus = 0xffff;
    *BridgeSubordinateBus = 0xffff;
    return ReturnDevicePath;
}

VOID
InitializePciRootBusList (
    IN OUT LIST_ENTRY   *ListHead
    )
{
    UINT16                  Bus, MaxBus;
    UINT16                  BridgeNumber;
    UINT8                   Device, Func;
    UINT64                  Address;
    PCI_TYPE01              PciBridge, *PciPtr;
    PCI_ROOT_BRIDGE_ENTRY   *Entry;
    BOOLEAN                 AnyPciToPciBridges, AnyPciDevice;
    

    InitializeListHead (ListHead);

    PciPtr = &PciBridge;
    BridgeNumber = 0;

    //
    // BugBug: Shutdown PCI bus scan
    //
    for (Bus =0; Bus <= PCI_MAX_BUS; Bus++) {
        AnyPciToPciBridges = AnyPciDevice = FALSE;
        MaxBus = Bus;
        for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
            for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {
                Address = EFI_PCI_ADDRESS (Bus, Device, Func, 0);
                ZeroMem(PciPtr,sizeof(PCI_TYPE01));
                GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, sizeof(UINT16), PciPtr);
                if (PciPtr->Hdr.VendorId == 0xffff) {
                    break;
                } else {
                    GlobalIoFncs->Pci.Read (GlobalIoFncs, IO_UINT32, Address, sizeof(PCI_TYPE01)/sizeof(UINT32), PciPtr);
                    AnyPciDevice = TRUE;
                    if (IS_PCI_BRIDGE(PciPtr)) {
                        AnyPciToPciBridges = TRUE;
                        if (PciPtr->Bridge.SubordinateBus > MaxBus) {
                            MaxBus = PciPtr->Bridge.SubordinateBus;
                        }
                    }
                    if (Func == 0 && !(PciPtr->Hdr.HeaderType & HEADER_TYPE_MULTI_FUNCTION)) {
                        //
                        // Skip sub functions, this is not a multi function device
                        //
                        Func = 8;
                    }
                }
            }
        }

        if (MaxBus > Bus) {
            Entry = AllocatePool (sizeof(PCI_ROOT_BRIDGE_ENTRY));
            Entry->Signature = EFI_ROOT_BRIDGE_LIST;
            Entry->BridgeNumber = BridgeNumber++;
            Entry->PrimaryBus = Bus;
            Entry->SubordinateBus = MaxBus;
            //
            // Assume only one PCI domain in the system
            //
            Entry->DevicePath = RootDevicePath;
            InsertTailList (ListHead, &Entry->Link);
 
            Bus = MaxBus;
        }
        if (!AnyPciToPciBridges && AnyPciDevice) {
            //
            // Root Bridge with no PtoP
            //
            Entry = AllocatePool (sizeof(PCI_ROOT_BRIDGE_ENTRY));
            Entry->Signature = EFI_ROOT_BRIDGE_LIST;
            Entry->BridgeNumber = BridgeNumber++;
            Entry->PrimaryBus = Bus; 
            Entry->SubordinateBus = Bus; 
            //
            // Assume only one PCI domain in the system
            //
            Entry->DevicePath = RootDevicePath;
            InsertTailList (ListHead, &Entry->Link);
        }
    }        


}

#ifdef EFI_DEBUG
VOID
PrintPciRootBridgeInfo (
    IN LIST_ENTRY   *ListHead
    )
{   
    PCI_ROOT_BRIDGE_ENTRY   *Entry;
    LIST_ENTRY              *Item;

    for (Item = ListHead->Flink; Item != ListHead; Item = Item->Flink) {
        Entry = CR(Item, PCI_ROOT_BRIDGE_ENTRY, Link, EFI_ROOT_BRIDGE_LIST);
        Print (L"\n PCI Root Bridge %d [0x%02x - 0x%02x]", 
                    Entry->BridgeNumber,
                    Entry->PrimaryBus,
                    Entry->SubordinateBus
                    );
    }
}

#endif
