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

  dprot.c

Abstract:

  Shell environment - dump protocol functions for the "dh" command

Revision History

--*/

#include "shelle.h"
#include "acpi.h"

STATIC CHAR16 *SEnvDP_IlleagalStr[] = {
  L"Illegal"
};

STATIC CHAR16 *SEnvDP_HardwareStr[] = {
  L"Illegal", L"PCI", L"PCCARD", L"Memory Mapped", L"Vendor-Defined", L"Controller"
};

STATIC CHAR16 *SEnvDP_ACPI_Str[] = {
  L"Illegal", L"ACPI"
};

STATIC CHAR16 *SEnvDP_MessageStr[] = {
  L"Illegal", 
  L"ATAPI", L"SCSI", L"Fibre Channel", L"1394", L"USB",
  L"I2O", L"Illegal", L"Illegal", L"InfiniBand", L"Vendor-Defined",
  L"MAC", L"IPv4", L"IPv6", L"UART", L"USB Class"
};

STATIC CHAR16 *SEnvDP_MediaStr[] = {
  L"Illegal", L"Hard Drive", L"CD-ROM", L"Vender-Defined", L"File Path", L"Media Protocol"
};

STATIC CHAR16 *SEnvDP_BBS_Str[] = {
  L"Illegal", L"BIOS Boot Spec"
};

struct DevicePathTypes {
  UINT8   Type;
  UINT8   MaxSubType;
  CHAR16  *TypeString;
  CHAR16  **SubTypeStr;
  VOID    (*Function)(EFI_DEVICE_PATH_PROTOCOL *);
};


VOID
SEnvHardwareDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  PCI_DEVICE_PATH         *PciDevicePath;
  PCCARD_DEVICE_PATH      *PcCardDevicePath;
  MEMMAP_DEVICE_PATH      *MemMapDevicePath;
  CONTROLLER_DEVICE_PATH  *ControllerDevicePath;
  VENDOR_DEVICE_PATH      *VendorDevicePath;
  
  if (DevicePathType (DevicePath) != HW_PCI_DP) {
    return;
  }

  //
  // Process hardware device path entry
  //
  switch (DevicePathSubType (DevicePath)) {
  case HW_PCI_DP:
    PciDevicePath = (PCI_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Function (%x) Device (%x)",
      PciDevicePath->Function,
      PciDevicePath->Device
      );
    break;
  case HW_PCCARD_DP:
    PcCardDevicePath = (PCCARD_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Function Number (%x)",
      PcCardDevicePath->FunctionNumber
      );
    break;
  case HW_MEMMAP_DP:
    MemMapDevicePath = (MEMMAP_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Memory Type (%d: %.lx-%.lx)",
      MemMapDevicePath->MemoryType,
      MemMapDevicePath->StartingAddress,
      MemMapDevicePath->EndingAddress
      );
    break;
  case HW_CONTROLLER_DP:
    ControllerDevicePath = (CONTROLLER_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Controller Number (%d)", ControllerDevicePath->Controller);
    break;
  case HW_VENDOR_DP:
    VendorDevicePath = (VENDOR_DEVICE_PATH *)DevicePath;
    Print(L"\n%N       Guid %g",
      &VendorDevicePath->Guid
      );
    if (DevicePathNodeLength (DevicePath) > sizeof (VENDOR_DEVICE_PATH)) {
      Print (L"\n");
      DumpHex (
        7, 0,
        DevicePathNodeLength (DevicePath) - sizeof (VENDOR_DEVICE_PATH),
        VendorDevicePath + 1
        );
    }
    break;
  }
  
  //
  // End processing
  //
}


VOID
SEnvAcpiDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  ACPI_HID_DEVICE_PATH    *AcpiDevicePath;

  if (DevicePathType (DevicePath) != ACPI_DEVICE_PATH) {
    return;
  }

  //
  // Process ACPI device path entry
  //
  if (DevicePathSubType (DevicePath) == ACPI_DP) {
    AcpiDevicePath = (ACPI_HID_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       HID %x, UID %x",
      AcpiDevicePath->HID,
      AcpiDevicePath->UID
      );
  }
}


VOID
SEnvMessagingDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  ATAPI_DEVICE_PATH         *AtapiDevicePath;
  SCSI_DEVICE_PATH          *ScsiDevicePath;
  FIBRECHANNEL_DEVICE_PATH  *FibreChannelDevicePath;
  F1394_DEVICE_PATH         *F1394DevicePath;
  USB_DEVICE_PATH           *UsbDevicePath;
  USB_CLASS_DEVICE_PATH     *UsbClassDevicePath;
  I2O_DEVICE_PATH           *I2ODevicePath;
  MAC_ADDR_DEVICE_PATH      *MacAddrDevicePath;
  IPv4_DEVICE_PATH          *IPv4DevicePath;
  IPv6_DEVICE_PATH          *IPv6DevicePath;
  INFINIBAND_DEVICE_PATH    *InfinibandDevicePath;
  UART_DEVICE_PATH          *UartDevicePath;
  VENDOR_DEVICE_PATH        *VendorDevicePath;

  UINTN                   HwAddressSize;
  UINTN                   Index;
  CHAR8                   Parity;

  if (DevicePathType (DevicePath) != MESSAGING_DEVICE_PATH) {
    return;
  }

  //
  // Process messaging device path entry
  //
  switch (DevicePathSubType (DevicePath)) {
  case MSG_ATAPI_DP:
    AtapiDevicePath = (ATAPI_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       ATAPI (%s, %s) LUN (%x)",
      AtapiDevicePath->PrimarySecondary ? L"Secondary" : L"Primary",
      AtapiDevicePath->SlaveMaster ? L"Slave" : L"Master",
      AtapiDevicePath->Lun
    );
    break;
  case MSG_SCSI_DP:
    ScsiDevicePath = (SCSI_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       SCSI (PUN %x, LUN %x)",
      ScsiDevicePath->Pun,
      ScsiDevicePath->Lun
      );
    break;
  case MSG_FIBRECHANNEL_DP:
    FibreChannelDevicePath = (FIBRECHANNEL_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Fibre Channel (WWN %lx, LUN %lx)",
      FibreChannelDevicePath->WWN,
      FibreChannelDevicePath->Lun
      );
    break;
  case MSG_1394_DP:
    F1394DevicePath = (F1394_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       1394 (GUID %g)", F1394DevicePath->Guid);
    break;
  case MSG_USB_DP:
    UsbDevicePath = (USB_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       USB (%x, %x)",
      UsbDevicePath->ParentPortNumber,
      UsbDevicePath->InterfaceNumber
      );
    break;
  case MSG_USB_CLASS_DP:
    UsbClassDevicePath = (USB_CLASS_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       USB Class (Vendor ID: %x, Product ID: %x)",
      UsbClassDevicePath->VendorId,
      UsbClassDevicePath->ProductId
      );
    Print (L"\n%N                 (Device: %x, %x, %x)",
      UsbClassDevicePath->DeviceClass,
      UsbClassDevicePath->DeviceSubClass,
      UsbClassDevicePath->DeviceProtocol
      );
    break;
  case MSG_I2O_DP:
    I2ODevicePath = (I2O_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       I2O (%x)", 
      I2ODevicePath->Tid
      );
    break;
  case MSG_MAC_ADDR_DP:
    MacAddrDevicePath = (MAC_ADDR_DEVICE_PATH *)DevicePath;
    HwAddressSize = sizeof (EFI_MAC_ADDRESS);
    if (MacAddrDevicePath->IfType == 0x01 || MacAddrDevicePath->IfType == 0x00) {
      HwAddressSize = 6;
    }
    Print (L"\n%N       MAC (");
    for (Index = 0; Index < HwAddressSize; Index++) {
      Print (L"%02x",MacAddrDevicePath->MacAddress.Addr[Index]);
    }
    Print (L")");
    break;
  case MSG_IPv4_DP:
    IPv4DevicePath = (IPv4_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       IPv4 (Local IP: %d.%d.%d.%d:%d)",
      IPv4DevicePath->LocalIpAddress.Addr[0],
      IPv4DevicePath->LocalIpAddress.Addr[1],
      IPv4DevicePath->LocalIpAddress.Addr[2],
      IPv4DevicePath->LocalIpAddress.Addr[3],
      IPv4DevicePath->LocalPort
      );
    Print (L"\n%N            (Remote IP: %d.%d.%d.%d:%d)",
      IPv4DevicePath->RemoteIpAddress.Addr[0],
      IPv4DevicePath->RemoteIpAddress.Addr[1],
      IPv4DevicePath->RemoteIpAddress.Addr[2],
      IPv4DevicePath->RemoteIpAddress.Addr[3],
      IPv4DevicePath->RemotePort
      );
    Print (L"\n%N            (Protocol: %x)",
      IPv4DevicePath->Protocol
      );
    Print (L"\n%N            (Source IP: %s)",
      IPv4DevicePath->StaticIpAddress ? L"Static" : L"DHCP"
      );
    break;
  case MSG_IPv6_DP:
    IPv6DevicePath = (IPv6_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       IPv6 (Not Available)");
    break;
  case MSG_INFINIBAND_DP:
    InfinibandDevicePath = (INFINIBAND_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Infiniband (Node GUID: %lX IOC GUID: %lX)",
      InfinibandDevicePath->ServiceId,
      InfinibandDevicePath->TargetPortId
      );
    Print (L"\n%N                  (DeviceId: %lX)",
      InfinibandDevicePath->DeviceId
      );
    break;
  case MSG_UART_DP:
    UartDevicePath = (UART_DEVICE_PATH *)DevicePath;
    switch (UartDevicePath->Parity) {
      case 0  : Parity = 'D'; break;
      case 1  : Parity = 'N'; break;
      case 2  : Parity = 'E'; break;
      case 3  : Parity = 'O'; break;
      case 4  : Parity = 'M'; break;
      case 5  : Parity = 'S'; break;
      default : Parity = 'x'; break;
    }
    if (UartDevicePath->BaudRate == 0) {
      Print (L"\n%N       UART (DEFAULT %c", Parity);
    } else {
      Print (L"\n%N       UART (%d %c", UartDevicePath->BaudRate, Parity);
    }
    if (UartDevicePath->DataBits == 0) {
      Print (L" D");
    } else {
      Print (L" %d", UartDevicePath->DataBits);
    }
    switch (UartDevicePath->StopBits) {
      case 0  : Print (L" D)");   break;
      case 1  : Print (L" 1)");   break;
      case 2  : Print (L" 1.5)"); break;
      case 3  : Print (L" 2)");   break;
      default : Print (L" x)");   break;
    }
    break;
  case MSG_VENDOR_DP:
    VendorDevicePath = (VENDOR_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Guid %g",
      &VendorDevicePath->Guid
      );
    if (DevicePathNodeLength (DevicePath) > sizeof (VENDOR_DEVICE_PATH)) {
      Print (L"\n");
      DumpHex (
        7, 0,
        DevicePathNodeLength (DevicePath) - sizeof (VENDOR_DEVICE_PATH),
        VendorDevicePath + 1
        );
    }
    break;
  }
  
  //
  // End processing
  //
}


VOID
SEnvMediaDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  HARDDRIVE_DEVICE_PATH       *HardDriveDevicePath;
  CDROM_DEVICE_PATH           *CDDevicePath;
  VENDOR_DEVICE_PATH          *VendorDevicePath;
  FILEPATH_DEVICE_PATH        *FilePath;
  MEDIA_PROTOCOL_DEVICE_PATH  *MediaProtocol;

  if (DevicePathType (DevicePath) != MEDIA_DEVICE_PATH) {
    return;
  }

  //
  // Process media device path entry
  //
  switch (DevicePathSubType (DevicePath)) {
  case MEDIA_HARDDRIVE_DP:
    HardDriveDevicePath = (HARDDRIVE_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Partition (%d) Start (%lX) Size (%lX)",
      HardDriveDevicePath->PartitionNumber,
      HardDriveDevicePath->PartitionStart,
      HardDriveDevicePath->PartitionSize
      );
    break;
  case MEDIA_CDROM_DP:
    CDDevicePath = (CDROM_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       BootEntry (%x) Start (%lX) Size (%lX)",
      CDDevicePath->BootEntry,
      CDDevicePath->PartitionStart,
      CDDevicePath->PartitionSize
      );
    break;
  case MEDIA_VENDOR_DP:
    VendorDevicePath = (VENDOR_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Guid %g",
      &VendorDevicePath->Guid
      );
    if (DevicePathNodeLength (DevicePath) > sizeof (VENDOR_DEVICE_PATH)) {
      Print (L"\n");
      DumpHex (
        7, 0,
        DevicePathNodeLength (DevicePath) - sizeof (VENDOR_DEVICE_PATH),
        VendorDevicePath + 1
        );
    }
    break;
  case MEDIA_FILEPATH_DP:
    FilePath = (FILEPATH_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       File '%hs'", FilePath->PathName);
    break;
  case MEDIA_PROTOCOL_DP:
    MediaProtocol = (MEDIA_PROTOCOL_DEVICE_PATH *)DevicePath;
    Print (L"\n%N       Protocol '%hg'", &MediaProtocol->Protocol);
    break;
  };
  
  //
  // End processing
  //
}


struct DevicePathTypes SEnvDP_Strings[] = {
  0x00, 0x01, L"Illegal",        SEnvDP_IlleagalStr, NULL,
  0x01, 0x05, L"Hardware",       SEnvDP_HardwareStr, SEnvHardwareDevicePathEntry,
  0x02, 0x01, L"ACPI",           SEnvDP_ACPI_Str,    SEnvAcpiDevicePathEntry,
  0x03, 0x0f, L"Messaging",      SEnvDP_MessageStr,  SEnvMessagingDevicePathEntry,
  0x04, 0x05, L"Media",          SEnvDP_MediaStr,    SEnvMediaDevicePathEntry,
  0x05, 0x01, L"BIOS Boot Spec", SEnvDP_BBS_Str,     NULL,
  
  END_DEVICE_PATH_TYPE, 0x01, L"End", SEnvDP_IlleagalStr, NULL
};


VOID
SEnvPrintDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
  IN BOOLEAN                      Verbose
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINT8   Type;
  UINT8   SubType;
  INTN    Index;

  //
  // Process print device path entry
  //
  Type = (UINT8)DevicePathType (DevicePath);
  SubType = DevicePathSubType (DevicePath);

  for (Index = 0; SEnvDP_Strings[Index].Type != END_DEVICE_PATH_TYPE; Index++) {
    if (Type == SEnvDP_Strings[Index].Type) {
      if (SubType > SEnvDP_Strings[Index].MaxSubType) {
        SubType = 0;
      }
      Print (L"\n%N      %s Device Path for %s",
        SEnvDP_Strings[Index].TypeString,
        SEnvDP_Strings[Index].SubTypeStr[SubType]
        );
      if (Verbose) {
        if (SEnvDP_Strings[Index].Function != NULL) {
          SEnvDP_Strings[Index].Function (DevicePath);
        }
      }
      return;
    }
  }

  Print (L"\n%E      Device Path Error%N - Unknown Device Type");
}


VOID
SEnvDPath (
  IN EFI_HANDLE           h,
  IN VOID                 *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathNode;
  CHAR16                    *Str;

  DevicePath = Interface;
  Str = DevicePathToStr (DevicePath);
  DevicePath = UnpackDevicePath (DevicePath);

  //
  // Print device path entry
  //
  DevicePathNode = DevicePath;
  while (!IsDevicePathEnd (DevicePathNode)) {
    SEnvPrintDevicePathEntry (DevicePathNode, TRUE);
    DevicePathNode = NextDevicePathNode (DevicePathNode);
  }

  Print (L"\n      AsStr:%N '%s'", Str);
  FreePool (Str);
  FreePool (DevicePath);
}


VOID
SEnvDPathTok (
  IN EFI_HANDLE   h,
  IN VOID         *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *Str;
  CHAR16                    *Disp;
  UINTN                     Len;

  DevicePath = Interface;
  Str = DevicePathToStr (DevicePath);
  Disp = L"";

  //
  // Print device path token
  //
  if (Str) {
    Len = StrLen (Str);
    Disp = Str;
    if (Len > 30) {
      Disp = Str + Len - 30;
      Disp[0] = '.';
      Disp[1] = '.';
    }
  }

  Print (L"DevPath (%s)", Disp);

  if (Str) {
    FreePool (Str);
  }
}

VOID
SEnvTextOut (
  IN EFI_HANDLE   h,
  IN VOID         *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_SIMPLE_TEXT_OUT_PROTOCOL     *Dev;
  INTN                             Index;
  UINTN                            Col;
  UINTN                            Row;
  EFI_STATUS                       Status;

  Dev = Interface;

  Print (L"Attrib %x",
    Dev->Mode->Attribute
    );

  //
  // Dump TextOut Info
  //
  for (Index = 0; Index < Dev->Mode->MaxMode; Index++) {
    Status = Dev->QueryMode (Dev, Index, &Col, &Row);
    Print (L"\n%N      %hc  mode %d: ",
      Index == Dev->Mode->Mode ? '*' : ' ',
      Index
      );

    if (EFI_ERROR (Status)) {
      Print (L"%error %rx\n", Status);
    } else {
      Print (L"col %3d row %3d", Col, Row);
    }
  }
}


VOID
SEnvBlkIo (
  IN EFI_HANDLE   h,
  IN VOID         *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_BLOCK_IO_PROTOCOL *BlkIo;
  EFI_BLOCK_IO_MEDIA    *BlkMedia;
  VOID                  *Buffer;

  BlkIo = Interface;
  BlkMedia = BlkIo->Media;

  //
  // Issue a dummy read to the device to check for media change
  //
  Buffer = AllocatePool (BlkMedia->BlockSize);
  if (Buffer) {
    BlkIo->ReadBlocks (BlkIo, 
                       BlkMedia->MediaId, 
                       0, 
                       BlkMedia->BlockSize, 
                       Buffer);
    FreePool (Buffer);
  }

  //
  // Dump BlkIo Info
  //
  Print (L"%s%esMId:%x ",
    BlkMedia->RemovableMedia ? L"Removable " : L"Fixed ",
    BlkMedia->MediaPresent ? L"" : L"not-present ",
    BlkMedia->MediaId
    );

  Print (L"bsize %x, lblock %lx (%,ld), %s %s %s",
    BlkMedia->BlockSize,
    BlkMedia->LastBlock,
    MultU64x32 (BlkMedia->LastBlock + 1, BlkMedia->BlockSize),
    BlkMedia->LogicalPartition ? L"partition" : L"raw",
    BlkMedia->ReadOnly ? L"ro" : L"rw",
    BlkMedia->WriteCaching ? L"cached" : L"!cached"
    );
}


CHAR8 *
GetPdbPath (
  VOID *ImageBase
  )
/*++

Routine Description:
  Located PDB path name in PE image

Arguments:
  ImageBase - base of PE to search

Returns:
  Pointer into image at offset of PDB file name if PDB file name is found,
  Otherwise a pointer to an empty string.
  
--*/
{
  CHAR8                           *PdbPath;
  UINT32                          DirCount;
  EFI_IMAGE_DOS_HEADER            *DosHdr;
  EFI_IMAGE_NT_HEADERS            *NtHdr;
  EFI_IMAGE_OPTIONAL_HEADER       *OptionalHdr;
  EFI_IMAGE_DATA_DIRECTORY        *DirectoryEntry;
  EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *DebugEntry;
  VOID                            *CodeViewEntryPointer;

  if (ImageBase == NULL) {
    return NULL;
  }

  CodeViewEntryPointer = NULL;
  PdbPath = NULL;
  DosHdr = ImageBase;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    NtHdr = (EFI_IMAGE_NT_HEADERS *)((UINT8 *)DosHdr + DosHdr->e_lfanew);
    OptionalHdr = (void *) &NtHdr->OptionalHeader;
    DirectoryEntry = (EFI_IMAGE_DATA_DIRECTORY *)&(OptionalHdr->DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG]);
    if (DirectoryEntry->VirtualAddress != 0) {
      for (DirCount = 0;
           (DirCount < DirectoryEntry->Size / sizeof (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY)) && CodeViewEntryPointer == NULL;
           DirCount++) {
        DebugEntry = (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *) (
                        DirectoryEntry->VirtualAddress +
                        (UINTN) ImageBase +
                        DirCount * sizeof (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY)
                      );
        if (DebugEntry->Type == EFI_IMAGE_DEBUG_TYPE_CODEVIEW) {
          CodeViewEntryPointer = (VOID *) ((UINTN) DebugEntry->RVA + (UINTN) ImageBase);
          switch (* (UINT32 *) CodeViewEntryPointer) {
            case CODEVIEW_SIGNATURE_NB10:
              PdbPath = (CHAR8 *) CodeViewEntryPointer + sizeof (EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY);
              break;
            case CODEVIEW_SIGNATURE_RSDS:
              PdbPath = (CHAR8 *) CodeViewEntryPointer + sizeof (EFI_IMAGE_DEBUG_CODEVIEW_RSDS_ENTRY);
              break;
            default:
              break;
          }
        }
      }
    }
  }
  return (PdbPath);
}

VOID
PrintShortPdbFileName (
  CHAR8  *PdbFileName,
  UINTN  Length
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN  Index;
  UINTN  StartIndex;
  UINTN  EndIndex;
 
  if (PdbFileName == NULL) {
    Print (L"NULL");
  } else {
    StartIndex = 0;
    for (EndIndex = 0; PdbFileName[EndIndex] != 0; EndIndex++);
    for (Index = 0; PdbFileName[Index] != 0; Index++) {
      if (PdbFileName[Index] == '\\') {
        StartIndex = Index+1;
      }
      if (PdbFileName[Index] == '.') {
        EndIndex = Index;
      }
    }
    for (Index = StartIndex; Index < EndIndex && Length > 0; Index++, Length--) {
      Print (L"%c", PdbFileName[Index]);
    }
  }
}


VOID
SEnvImageTok (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL    *Image;
  CHAR16                       *Tok;

  Image = Interface;
  Tok = DevicePathToStr (Image->FilePath);
  Print (L"%HImage%N(%s) ", Tok);

  if (Tok) {
    FreePool (Tok);
  }
}


VOID
SEnvImage (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL    *Image;
  CHAR16                       *FilePath;
  CHAR8                        *PdbFileName;

  Image = Interface;

  FilePath = DevicePathToStr (Image->FilePath);

  PdbFileName = GetPdbPath (Image->ImageBase);

  if (PdbFileName != NULL) {
    Print (L"  File:%H");
    PrintShortPdbFileName (PdbFileName,62);
    Print (L"%N\n");
  } else {
    Print (L"  File:%hs\n", FilePath);
  }

  //
  // Dump Image Info
  //
  if (!Image->ImageBase) {
    Print (L"     %EInternal Image:%N %s\n",  FilePath);
    if (Image->ImageCodeType >= 0) {
      Print (L"     CodeType......: %s\n",  MemoryTypeStr (Image->ImageCodeType));
    } else {
      Print (L"     CodeType......: %08x\n",  Image->ImageCodeType);
    }
    if (Image->ImageDataType >= 0) {
      Print (L"     DataType......: %s\n",  MemoryTypeStr (Image->ImageDataType));
    } else {
      Print (L"     DataType......: %08x\n",  Image->ImageDataType);
    }
  } else {
    Print (L"     ParentHandle..: %X\n",  Image->ParentHandle);
    Print (L"     SystemTable...: %X\n",  Image->SystemTable);
    Print (L"     DeviceHandle..: %X\n",  Image->DeviceHandle);
    Print (L"     FilePath......: %s\n",  FilePath);
    Print (L"     ImageBase.....: %X - %X\n",  
           Image->ImageBase,
           (CHAR8 *) Image->ImageBase + Image->ImageSize
          );
    Print (L"     ImageSize.....: %lx\n", Image->ImageSize);
    if (Image->ImageCodeType >= 0) {
      Print (L"     CodeType......: %s\n",  MemoryTypeStr (Image->ImageCodeType));
    } else {
      Print (L"     CodeType......: %08x\n",  Image->ImageCodeType);
    }
    if (Image->ImageDataType >= 0) {
      Print (L"     DataType......: %s\n",  MemoryTypeStr (Image->ImageDataType));
    } else {
      Print (L"     DataType......: %08x\n",  Image->ImageDataType);
    }
  }

  if (FilePath) {
    FreePool (FilePath);
  }
}


VOID
SEnvIsaIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_ISA_IO_PROTOCOL   *IsaIo;
  UINTN                 Index;

  IsaIo = Interface;

  //
  // Dump IsaIo Info
  //
  Print (L"\n");
  Print (L"     ROM Size......: %08x\n",IsaIo->RomSize);
  Print (L"     ROM Location..: %08x\n",IsaIo->RomImage);

  Print (L"     ISA Resource List :\n");
  for (Index = 0; IsaIo->ResourceList->ResourceItem[Index].Type != EfiIsaAcpiResourceEndOfList; Index++) {
    switch (IsaIo->ResourceList->ResourceItem[Index].Type) {
      case EfiIsaAcpiResourceIo        : Print (L"       IO  : "); break;
      case EfiIsaAcpiResourceMemory    : Print (L"       MEM : "); break;
      case EfiIsaAcpiResourceDma       : Print (L"       DMA : "); break;
      case EfiIsaAcpiResourceInterrupt : Print (L"       INT : "); break;
    }
    
    if (IsaIo->ResourceList->ResourceItem[Index].StartRange == IsaIo->ResourceList->ResourceItem[Index].EndRange) {
      Print (L"%08x           Attr : %08x\n",
            IsaIo->ResourceList->ResourceItem[Index].StartRange,
            IsaIo->ResourceList->ResourceItem[Index].EndRange,
            IsaIo->ResourceList->ResourceItem[Index].Attribute
            );
    } else {
      Print (L"%08x-%08x  Attr : %08x\n",
            IsaIo->ResourceList->ResourceItem[Index].StartRange,
            IsaIo->ResourceList->ResourceItem[Index].EndRange,
            IsaIo->ResourceList->ResourceItem[Index].Attribute
            );
    }
  }
}


VOID
SEnvPciRootBridgeIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*
Routine Description:

Arguments:

Returns:
*/
{
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL    *PciRootBridgeIo;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Configuration;
  UINT64                             Supports;
  UINT64                             Attributes;

  PciRootBridgeIo = Interface;

  Configuration = NULL;
  PciRootBridgeIo->Configuration (PciRootBridgeIo, (VOID **)&Configuration);
  PciRootBridgeIo->GetAttributes (PciRootBridgeIo, &Supports, &Attributes);

  //
  // Dump PciRootBridgeIo Info
  //
  Print (L"\n");
  Print (L"     ParentHandle..: %X\n",PciRootBridgeIo->ParentHandle);
  Print (L"     Segment #.....: %X\n",PciRootBridgeIo->SegmentNumber);
  Print (L"     Attributes....: %X\n",Attributes);
  Print (L"     Supports......: %X",Supports);

  if (Configuration != NULL) {
    Print (L"\n");
    Print (L"      Type  Flag  Base              Limit             Gran\n");
    Print (L"      ====  ====  ================  ================  ====");
    while (Configuration->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
      Print(L"\n");
      switch (Configuration->ResType) {
      case ACPI_ADDRESS_SPACE_TYPE_MEM :
        Print (L"      MEM : ");
        break;
      case ACPI_ADDRESS_SPACE_TYPE_IO :
        Print (L"      I/O : ");
        break;
      case ACPI_ADDRESS_SPACE_TYPE_BUS :
        Print (L"      BUS : ");
        break;
      }

      Print (L"%02x    %016lx  %016lx  %02x", 
       Configuration->SpecificFlag, 
       Configuration->AddrRangeMin, 
       Configuration->AddrRangeMax,
       Configuration->AddrSpaceGranularity
       );
      Configuration++;
    }
  }
}


VOID
SEnvPciIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                          Status;
  EFI_PCI_IO_PROTOCOL                 *PciIo;
  PCI_TYPE00                          Pci;
  UINTN                               Segment;
  UINTN                               Bus;
  UINTN                               Device;
  UINTN                               Function;
  UINTN                               Index;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR   *Configuration;

  PciIo = Interface;

  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0, sizeof (Pci), &Pci);

  PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);

  //
  // Dump PciIo Info
  //
  Print (L"\n");
  Print (L"     Location......: Seg(%02hx)  Bus(%02hx)  Dev(%02hx)  Func(%02hx)\n",Segment, Bus, Device, Function);
  Print (L"     Configuration.: VendorID(%04hx)  DeviceID(%04hx)  ClassCode(%02hx %02hx %02hx)\n",Pci.Hdr.VendorId, Pci.Hdr.DeviceId, Pci.Hdr.ClassCode[0],Pci.Hdr.ClassCode[1],Pci.Hdr.ClassCode[2]);
  for (Index = 0; Index < 6; Index++) {
    Status = PciIo->GetBarAttributes (PciIo, (UINT8)Index, NULL, &Configuration);
    if (!EFI_ERROR (Status)) {
      if (Configuration->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
        Print (L"     BAR #%d........: ", Index);
        switch (Configuration->ResType) {
        case ACPI_ADDRESS_SPACE_TYPE_IO:
          Print (L"I/O      ");
          break;
        case ACPI_ADDRESS_SPACE_TYPE_MEM:
          if (Configuration->SpecificFlag == 0x06) {
            Print (L"PMEM%2d   ",Configuration->AddrSpaceGranularity);
          } else {
            Print (L"MEM%2d    ",Configuration->AddrSpaceGranularity);
          }
          break;
        default:
          Print (L"Unknown  ");
          break;
        }
        Print (L"Base(%16lhx)  Length(%lhx)\n", Configuration->AddrRangeMin, Configuration->AddrLen);
      }
      BS->FreePool (Configuration);
    }
  }
  Print (L"     ROM...........:          Base(%16lhx)  Length(%hx)\n", (UINT64)PciIo->RomImage, PciIo->RomSize);
  Print (L"     Configuration Header :");
  for (Index = 0; Index < sizeof (Pci); Index++) {
    if ((Index % 0x10) == 0) {
      Print (L"\n       ");
    }
    Print (L"%02x ",*((UINT8 *)(&Pci) + Index));
  }

  Print (L"\n");
}


VOID
SEnvUsbIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDesc;

  UsbIo = Interface;

  UsbIo->UsbGetInterfaceDescriptor (UsbIo, &InterfaceDesc);

  //
  // Dump UsbIo Info
  //
  Print (L"\n");
  Print (L"     Interface Number #.....: %02x\n", InterfaceDesc.InterfaceNumber);
  Print (L"     Interface Class #......: %02x\n", InterfaceDesc.InterfaceClass);
  Print (L"     Interface Subclass #...: %02x\n", InterfaceDesc.InterfaceSubClass);
  Print (L"     Interface Protocol #...: %02x\n", InterfaceDesc.InterfaceProtocol);
}

VOID
SEnvDebugSupport (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*
Routine Description:

Arguments:

Returns:
*/
{
  EFI_DEBUG_SUPPORT_PROTOCOL     *DebugSupport;

  DebugSupport = Interface;

  //
  // Dump Debug support info
  //
  Print (L"\n");
  Print (L"     Isa = ");
  switch (DebugSupport->Isa) {
    case (IsaIa32):
    Print (L"IA-32");
    break;
    case (IsaIpf):
    Print (L"IPF");
    break;
    case (IsaEbc):
    Print (L"EBC");
    break;
    default:
    Print (L"Unknown (%X)", DebugSupport->Isa);
    break;
  }
}

VOID
SEnvBusSpecificDriverOverride (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                                 Status;
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL  *BusSpecificDriverOverride;
  EFI_HANDLE                                 ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL                  *Image;

  Print (L"\n");
  BusSpecificDriverOverride = Interface;
  do {
    Status = BusSpecificDriverOverride->GetDriver (BusSpecificDriverOverride, &ImageHandle);
    if (!EFI_ERROR (Status)) {
      Status = BS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&Image);
      if (!EFI_ERROR (Status)) {
        Print (L"     Drv[%02x] File:%hs\n", SEnvHandleToNumber (ImageHandle), DevicePathToStr (Image->FilePath));
      }
    }
  } while (!EFI_ERROR (Status));
}
