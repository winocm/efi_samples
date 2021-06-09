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

    BiosBlkio.c
    
Abstract:

    EFI glue for BIOS INT 13h block devices.

    This file is coded to EDD 3.0 as defined by T13 D1386 Revision 4
    Availible on http://www.t13.org/#Project drafts
    Currently at ftp://fission.dt.wdc.com/pub/standards/x3t13/project/d1386r4.pdf
    

Revision History

--*/

#include "biosDriver.h"
#include "plcd.h"

INTN
PrintBlkIO (
    BIOS_BLK_IO_DEV     *Dev
    );

//
// Address packet is a buffer under 1 MB for all version EDD calls
//
EDD_DEVICE_ADDRESS_PACKET   *GlobalEDDBufferUnder1MB;

//
// This is a buffer for INT 13h func 48 information
//
BIOS_LEGACY_DRIVE           *GlobalLegacyDriverUnder1MB;

//
// Buffer of 0xFE00 bytes for EDD 1.1 transfer must be under 1 MB
//  0xFE00 bytes is the max transfer size supported.
//
VOID                        *GlobalEDD11Buffer;

//
// Global Io Functions 
//
extern EFI_DEVICE_IO_INTERFACE   *GlobalIoFncs; 

#if EFI_DEBUG
extern EFI_GUID AdapterDebugProtocol;
#endif

VOID
SetBiosInitBlkIoDevicePath (
    IN  BIOS_LEGACY_DRIVE   *Drive,
    IN  EFI_DEVICE_PATH     **DevPath
    );

EFI_DRIVER_ENTRY_POINT(InstallBiosBlkIoDrivers)

EFI_STATUS
InstallBiosBlkIoDrivers (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS              Status;
    UINTN                   i;
    UINTN                   NumberOfDisketteDrives, NumberOfHardDrives, NumberOfCdRoms;
    BIOS_BLK_IO_DEV         *Dev;
    EFI_PHYSICAL_ADDRESS    MemPage;
    UINTN                   PageSize, IncrLength;
    UINT8                   StartDriveLetter;
    CDROM_INFO              *Info, *Info_Orginal;    

    InitializeLib (ImageHandle, SystemTable);

    DEBUG ((D_INIT, "BiosBlkIo: Detecting INT 13h EDD 3.0 Devices\n"));    
    //
    // Allocate buffer under 1MB to put real mode thunk code in
    //

    PageSize = sizeof(EDD_DEVICE_ADDRESS_PACKET) + sizeof(BIOS_LEGACY_DRIVE) + MAX_EDD11_XFER;
    PageSize = PageSize/EFI_PAGE_SIZE + 1; // extra page taken to adjust for alignment (if needed)
    MemPage = ONEMB - 1;

    Status = BS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, PageSize, &MemPage);
    ASSERT (!EFI_ERROR(Status));

    GlobalEDD11Buffer = (VOID *)MemPage;
    //
    // Adjusting the value to be on proper boundary
    //
    ALIGN_VARIABLE((UINTN)GlobalEDD11Buffer, IncrLength);

    GlobalLegacyDriverUnder1MB = (BIOS_LEGACY_DRIVE *)((UINT8 *)MemPage + 
                                        MAX_EDD11_XFER + 
                                        IncrLength);
    //
    // Adjusting the value to be on proper boundary
    //
    ALIGN_VARIABLE((UINTN)GlobalLegacyDriverUnder1MB, IncrLength);

    GlobalEDDBufferUnder1MB = (EDD_DEVICE_ADDRESS_PACKET *)((UINT8 *)MemPage + 
                                        sizeof(BIOS_LEGACY_DRIVE) + 
                                        MAX_EDD11_XFER + 
                                        IncrLength);
    //
    // Adjusting the value to be on proper boundary
    //
    ALIGN_VARIABLE((UINTN)GlobalEDDBufferUnder1MB, IncrLength);

    //
    // Attach Floppy Drives
    //
    NumberOfDisketteDrives = BiosGetNumberOfDiskettes();
    for (i=0; i<NumberOfDisketteDrives; i++) {
        Dev = AllocateZeroPool(sizeof(BIOS_BLK_IO_DEV));
        
        Dev->Signature = BIOS_CONSOLE_BLK_IO_DEV_SIGNATURE;
        Dev->Bios.Floppy = TRUE;
        Dev->Bios.Number = (UINT8) i;
        Dev->Bios.Letter = (UINT8) i + 'A';
        Dev->BlkMedia.RemovableMedia = TRUE;
        if (BiosInitBlkIo (Dev)) {
            SetBiosInitBlkIoDevicePath (&Dev->Bios, &Dev->DevicePath);

            Status = LibInstallProtocolInterfaces (
                        &Dev->Handle,
                        &BlockIoProtocol,       &Dev->BlkIo,
                        &DevicePathProtocol,    Dev->DevicePath,
#if EFI_DEBUG
                        &AdapterDebugProtocol,  &Dev->Bios,
#endif
                        NULL
                        );
            DEBUG ((D_INIT, "BiosBlkIo: Installed BIOS Diskette BlkIo %X\n", Dev->Handle));    
        } else {
            FreePool (Dev);
        }
    }
   

    //
    // Attach Hard Drives
    //
    NumberOfHardDrives = BiosGetNumberOfHardDrives();
    for (i=0; i<NumberOfHardDrives; i++) {
        Dev = AllocateZeroPool(sizeof(BIOS_BLK_IO_DEV));
        
        Dev->Signature = BIOS_CONSOLE_BLK_IO_DEV_SIGNATURE;
        Dev->Bios.Floppy = FALSE;
        Dev->Bios.Number = (UINT8) i + 0x80;
        Dev->Bios.Letter = (UINT8) i + 'C';
        Dev->BlkMedia.RemovableMedia = FALSE;
        if (BiosInitBlkIo (Dev)) {
            SetBiosInitBlkIoDevicePath (&Dev->Bios, &Dev->DevicePath);
           
            Status = LibInstallProtocolInterfaces (
                        &Dev->Handle,
                        &BlockIoProtocol,       &Dev->BlkIo,
                        &DevicePathProtocol,    Dev->DevicePath,   
#if EFI_DEBUG
                        &AdapterDebugProtocol,  &Dev->Bios,
#endif
                        NULL
                        );
            DEBUG ((D_INIT, "BiosBlkIo: Installed BIOS Hard Drive BlkIo %X\n", Dev->Handle));    
        } else {
           FreePool (Dev);
        }
    }

    //
    // There is no standard way to discover the CD-ROM on a PC
    //  What we wan't for EFI is the BIOS to materialize a no emulation mode
    //  type drive letter for the CD-ROM. We want this in all cases, even if
    //  no CD-ROM is in the drive.
    //
    // Start drive letter from where HD left off, why not
    //
    StartDriveLetter = (UINT8)NumberOfHardDrives  + 'C';

    //
    // Attach CDRoms
    //
    PlGetCDRomInfo (&NumberOfCdRoms, &Info);
    Info_Orginal = Info;
    for (i=0; i<NumberOfCdRoms; i++) {
        Dev = AllocateZeroPool(sizeof(BIOS_BLK_IO_DEV));
    
        Dev->Signature = BIOS_CONSOLE_BLK_IO_DEV_SIGNATURE;
        Dev->Bios.Floppy = FALSE;
        Dev->Bios.Number =(UINT8) Info->DeviceNumber;
        Dev->Bios.Letter = (UINT8)i + StartDriveLetter; // Drive letter 
        Dev->BlkMedia.RemovableMedia = TRUE;
        if (BiosInitBlkIo (Dev) == TRUE) {
            SetBiosInitBlkIoDevicePath (&Dev->Bios, &Dev->DevicePath);

            //
            // for now assume CD is read only
            //
            Dev->BlkIo.Media->ReadOnly = TRUE;
          
            Status = LibInstallProtocolInterfaces (
                        &Dev->Handle,
                        &BlockIoProtocol,       &Dev->BlkIo,
                        &DevicePathProtocol,    Dev->DevicePath,   
#if EFI_DEBUG
                        &AdapterDebugProtocol,  &Dev->Bios,
#endif
                        NULL
                        );

            DEBUG ((D_INIT, "BiosBlkIo: Installed BIOS CDROMS BlkIo %X\n", Dev->Handle));    
        } else {
            FreePool (Dev);
        }
        Info++;
    } 

    if(NumberOfCdRoms) {
        //
        // Free pool if cdroms detected
        //
        FreePool(Info_Orginal);
    }

    return EFI_SUCCESS;
}

VOID
SetBiosInitBlkIoDevicePath (
    IN  BIOS_LEGACY_DRIVE   *Drive,
    IN  EFI_DEVICE_PATH     **DevicePath
    )
{
    ACPI_HID_DEVICE_PATH                AcpiNode;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH   VendorNode;

    //
    // BugBug: Check for memory leaks!
    //

    if (Drive->Floppy && !Drive->ATAPI_Floppy) {

        ZeroMem (&AcpiNode, sizeof(AcpiNode));
        AcpiNode.Header.Type = ACPI_DEVICE_PATH;
        AcpiNode.Header.SubType = ACPI_DP;
        SetDevicePathNodeLength (&AcpiNode.Header, sizeof(AcpiNode));
        AcpiNode.HID = EISA_PNP_ID(0x0604);
        AcpiNode.UID = Drive->Number;

        *DevicePath = AppendDevicePathNode (LegacyDevicePath, &AcpiNode.Header);

    } else if (Drive->EDDVersion == EDD_VERSION_30) {

        BuildEDD30DevicePath (Drive, DevicePath);

    } else {

        //
        // EDD 1.1 case. 
        //
        
        ZeroMem (&VendorNode, sizeof(VendorNode));
        VendorNode.DevicePath.Header.Type = HARDWARE_DEVICE_PATH;
        VendorNode.DevicePath.Header.SubType = HW_VENDOR_DP;
        SetDevicePathNodeLength (&VendorNode.DevicePath.Header, sizeof(VendorNode));
        CopyMem (&VendorNode.DevicePath.Guid, &UnknownDevice, sizeof(EFI_GUID));
        VendorNode.LegacyDriveLetter = Drive->Number;
        *DevicePath = AppendDevicePathNode (EndDevicePath, &VendorNode.DevicePath.Header);
    }
}

VOID
BuildEDD30DevicePath (
    IN  BIOS_LEGACY_DRIVE   *Drive,
    IN  EFI_DEVICE_PATH     **DevicePath
    )
{
    UINT64                  Address;
    EFI_DEV_PATH            Node;
    UINT32                  Controller;
    EFI_HANDLE              Handle;

    if (strncmpa ("PCI", Drive->Parameters.HostBusType, 3) == 0 ) {
        Address = EFI_PCI_ADDRESS(Drive->Parameters.InterfacePath.Pci.Bus, Drive->Parameters.InterfacePath.Pci.Device, Drive->Parameters.InterfacePath.Pci.Function,0);
        GlobalIoFncs->PciDevicePath (GlobalIoFncs, Address, DevicePath);
        Controller = (UINT32)Drive->Parameters.InterfacePath.Pci.Controller;

        //
        // Create a handle with the PCI Controller's device path.  This will prevent the PCI Bus Driver
        // from creating a child handle for this controller.
        //
        Handle = NULL;
        BS->InstallMultipleProtocolInterfaces (
              &Handle,
              &DevicePathProtocol, *DevicePath,
              NULL
              );

    } else {
        //
        // Assume no EDD 3.0 legacy Devices on ISA
        //
        Print (L"\nEDD 3.0 INT 13 function 48 buffer\n");
        DumpHex (0, 0, sizeof(Drive->Parameters), &Drive->Parameters);
        DBGASSERT(Drive->Parameters.HostBusType);
        Controller = 0;
    }

    ZeroMem (&Node, sizeof(Node));
    if ((strncmpa ("ATAPI", Drive->Parameters.InterfaceType, 5) == 0) ||
        (strncmpa ("ATA", Drive->Parameters.InterfaceType, 3) == 0)     ) {

        Node.Atapi.Header.Type = MESSAGING_DEVICE_PATH; 
        Node.Atapi.Header.SubType = MSG_ATAPI_DP;
        SetDevicePathNodeLength(&Node.Atapi.Header, sizeof(ATAPI_DEVICE_PATH));
        Node.Atapi.SlaveMaster = Drive->Parameters.DevicePath.Atapi.Master;
        Node.Atapi.Lun = Drive->Parameters.DevicePath.Atapi.Lun;
        Node.Atapi.PrimarySecondary = (UINT8)Controller;
    } else {
        if (Controller != 0) {
            ZeroMem (&Node, sizeof(Node));
            Node.Controller.Header.Type = HARDWARE_DEVICE_PATH;
            Node.Controller.Header.SubType = HW_CONTROLLER_DP;
            SetDevicePathNodeLength(&Node.Controller.Header, sizeof(CONTROLLER_DEVICE_PATH));
            Node.Controller.Controller = Controller;
            *DevicePath = AppendDevicePathNode (*DevicePath, &Node.DevPath);
        }
        ZeroMem (&Node, sizeof(Node));
        if (strncmpa("SCSI", Drive->Parameters.InterfaceType, 4) == 0 ) {
            Node.Scsi.Header.Type = MESSAGING_DEVICE_PATH;
            Node.Scsi.Header.SubType = MSG_SCSI_DP;
            SetDevicePathNodeLength(&Node.Scsi.Header, sizeof(SCSI_DEVICE_PATH));

            //
            // Lun is miss aligned in both EDD and Device Path data structures.
            //  thus we do a byte copy, to prevent alignment traps on IA-64.
            //
            CopyMem (&Node.Scsi.Lun, &Drive->Parameters.DevicePath.Scsi.Lun, sizeof(UINT16));
            Node.Scsi.Pun = Drive->Parameters.DevicePath.Scsi.Pun;

        } else if (strncmpa("USB", Drive->Parameters.InterfaceType, 3) == 0 ) {
            Node.Usb.Header.Type = MESSAGING_DEVICE_PATH;
            Node.Usb.Header.SubType = MSG_USB_DP;
            SetDevicePathNodeLength(&Node.Usb.Header, sizeof(USB_DEVICE_PATH));
            Node.Usb.ParentPortNumber = (UINT8)Drive->Parameters.DevicePath.Usb.Reserved;

        } else if (strncmpa("1394", Drive->Parameters.InterfaceType, 4) == 0 ) {
            Node.F1394.Header.Type = MESSAGING_DEVICE_PATH; 
            Node.F1394.Header.SubType = MSG_1394_DP;
            SetDevicePathNodeLength(&Node.F1394.Header, sizeof(F1394_DEVICE_PATH));
            Node.F1394.Guid = Drive->Parameters.DevicePath.FireWire.Guid;

        } else if (strncmpa("FIBRE", Drive->Parameters.InterfaceType, 5) == 0 ) {
            Node.FibreChannel.Header.Type = MESSAGING_DEVICE_PATH;
            Node.FibreChannel.Header.SubType = MSG_FIBRECHANNEL_DP;
            SetDevicePathNodeLength(&Node.FibreChannel.Header, sizeof(FIBRECHANNEL_DEVICE_PATH));
            Node.FibreChannel.WWN = Drive->Parameters.DevicePath.FibreChannel.Wwn;
            Node.FibreChannel.Lun = Drive->Parameters.DevicePath.FibreChannel.Lun;

        } else {
            DBGASSERT(Drive->Parameters.InterfaceType);
        }
    }
    if (Node.DevPath.Type) {
        *DevicePath = AppendDevicePathNode (*DevicePath, &Node.DevPath);
    }
}
