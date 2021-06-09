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

    UhcHlp.c
    
Abstract: 
    

Revision History
--*/

#include "Efi.h"
#include "EfiDriverLib.h"

#include "uhci.h"

//
// UHCI IO Space Address Register Register locates at 
// offset 20 ~ 23h of PCI Configuration Space (UHCI spec, Revision 1.1),
// so, its BAR Index is 4.
//
#define USB_BAR_INDEX       4

UINT8
USBReadPortB (IN  EFI_PCI_IO_PROTOCOL   *PciIo, IN  UINT32   PortOffset)
{
  UINT8         Data;
  
  //
  // Perform 8bit Read in PCI IO Space 
  //
  PciIo->Io.Read (PciIo,
                 EfiPciIoWidthUint8,
                 USB_BAR_INDEX,
                 (UINT64)PortOffset,
                 1,
                 &Data
                 );
  return Data;                 
}   

UINT16
USBReadPortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortOffset
  )
{
  UINT16          Data;
  
  //
  // Perform 16bit Read in PCI IO Space 
  //
  PciIo->Io.Read (PciIo,
                 EfiPciIoWidthUint16,
                 USB_BAR_INDEX,
                 (UINT64)PortOffset,
                 1,
                 &Data
                 );
  return Data;      
}   


UINT32
USBReadPortDW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortOffset
  )
{
  UINT32          Data;
  
  //
  // Perform 32bit Read in PCI IO Space 
  //
  PciIo->Io.Read (PciIo,
                 EfiPciIoWidthUint32,
                 USB_BAR_INDEX,
                 (UINT64)PortOffset,
                 1,
                 &Data
                 );
  return Data;      
}   

VOID
USBWritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortOffset, 
  IN  UINT8                 Data
  )
{
  //
  // Perform 8bit Write in PCI IO Space 
  //
  PciIo->Io.Write (PciIo,
                  EfiPciIoWidthUint8,
                  USB_BAR_INDEX,
                  (UINT64)PortOffset,
                  1,
                  &Data
                  );
}   

VOID
USBWritePortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortOffset, 
  IN  UINT16                Data
  )
{
  //
  // Perform 16bit Write in PCI IO Space 
  //
  PciIo->Io.Write (PciIo,
                  EfiPciIoWidthUint16,
                  USB_BAR_INDEX,
                  (UINT64)PortOffset,
                  1,
                  &Data
                  );
}   

VOID
USBWritePortDW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortOffset, 
  IN  UINT32                Data
  )
{
  //
  // Perform 32bit Write in PCI IO Space 
  //
  PciIo->Io.Write (PciIo,
                  EfiPciIoWidthUint32,
                  USB_BAR_INDEX,
                  (UINT64)PortOffset,
                  1,
                  &Data
                  );
}


//
//  USB register-base helper functions
//

VOID
WriteUHCCommandReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                CmdAddrOffset,
  IN  UINT16                UsbCmd
  )
{
  //
  // Write to UHC's Command Register
  //
  USBWritePortW (PciIo,CmdAddrOffset, UsbCmd);
  return;
}


UINT16 
ReadUHCCommandReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                 CmdAddrOffset
  )
{
  //
  // Read from UHC's Command Register
  //
  return USBReadPortW (PciIo,CmdAddrOffset);
}

VOID
WriteUHCStatusReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusAddrOffset,
  IN  UINT16                UsbSts
)
{
  //
  // Write to UHC's Status Register
  //
  USBWritePortW (PciIo,StatusAddrOffset, UsbSts);
  return;
}

UINT16 
ReadUHCStatusReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusAddrOffset
)
{
  //
  // Read from UHC's Status Register
  //
  return USBReadPortW (PciIo,StatusAddrOffset);
}


VOID
ClearStatusReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusAddrOffset
)
{
  UINT16 UsbSts;
  
  //
  // Clear the content of UHC's Status Register
  //
  UsbSts = 0x003F;
  WriteUHCStatusReg (PciIo,StatusAddrOffset,UsbSts);
}


VOID  
WriteUHCIntReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                InterruptAddrOffset,
  IN  UINT16                UsbInt
)
{
  //
  // Write to UHC's Interrupt Enable Register
  //
  USBWritePortW (PciIo,InterruptAddrOffset, UsbInt);
  return;
}


UINT16 
ReadUHCIntReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                InterruptAddrOffset
)
{
  //
  // Read from UHC's Interrupt Enable Register
  //
  return USBReadPortW (PciIo,InterruptAddrOffset);
}


VOID  
WriteUHCFrameNumberReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FrameNumAddrOffset,
  IN  UINT16                UsbFrameNum
)
{
  //
  // Write to UHC's Frame Number Register
  //
  USBWritePortW (PciIo,FrameNumAddrOffset, UsbFrameNum);
  return;
}


UINT16 
ReadUHCFrameNumberReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FrameNumAddrOffset
)
{
  //
  // Read from UHC's Frame Number Register
  //
  return USBReadPortW (PciIo,FrameNumAddrOffset);
}


VOID  
WriteUHCFrameListBaseReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FlBaseAddrOffset,
  IN  UINT32                UsbFrameListBaseAddr
  )
{
  //
  // Write to UHC's Frame List Base Register
  //
  USBWritePortDW (PciIo,FlBaseAddrOffset, UsbFrameListBaseAddr);  
  return;
}


UINT32 
ReadUHCFrameListBaseReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FlBaseAddrOffset
  )
{
  UINT32 UsbFrameListBaseAddr;
  
  //
  // Read from UHC's Frame List Register, and return the valid value.
  //
  UsbFrameListBaseAddr = USBReadPortDW (PciIo,FlBaseAddrOffset);
  
  UsbFrameListBaseAddr  = UsbFrameListBaseAddr & 0xFFFFF000 ;
  
  return UsbFrameListBaseAddr;
}


UINT16 
ReadRootPortReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortAddrOffset
  )
{
  //
  // Read from UHC's Root Port Register
  //
  return USBReadPortW (PciIo,PortAddrOffset);
}


VOID 
WriteRootPortReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortAddrOffset,
  IN  UINT16                ControlBits
  )
{
  //
  // Write to UHC's Root Port Register
  //
  USBWritePortW (PciIo,PortAddrOffset,ControlBits);
  return ;
}



EFI_STATUS
WaitForUHCHalt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN UINT32                 StatusRegAddr,
  IN UINTN                  Timeout
  )
{
  UINTN               Delay; 
  UINT16              Status ;
  
  //
  // Timeout is in us unit
  //
  Delay = (Timeout / 50) + 1;
  do {
    Status = ReadUHCStatusReg(PciIo,StatusRegAddr) ;
    if ( (Status & USBSTS_HCH) == USBSTS_HCH) {
      break;
    }

    gBS->Stall(50);   // Stall for 50 us

  } while (Delay--);
    
  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}


BOOLEAN
DetectUHCInterrupt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  )
{
  UINT16      Status ;
  
  //
  // Scan whether there is any interrupt flag set in Status Register. 
  //
  Status = ReadUHCStatusReg (PciIo,StatusRegAddr) ;

  if (Status & 0x003B) {   // bit0~bit1,bit3~bit5
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN 
IsUSBInterrupt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
)
{
  UINT16      Status ;
  
  //
  // Detect whether the interrupt is caused by IOC or Short Packet Detected.
  // see "UHCI Design Guid".
  //
  Status = ReadUHCStatusReg (PciIo,StatusRegAddr) ;
  
  if (Status & USBSTS_USBINT) {
    return TRUE;
  } else {
    return FALSE;
  }

}

BOOLEAN 
IsUSBErrInterrupt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  )
{
  UINT16      Status ;
  
  //
  // Detect whether the interrupt is caused by any error condition.
  // see "UHCI Design Guid".
  //
  Status = ReadUHCStatusReg (PciIo,StatusRegAddr) ;
  
  if (Status & USBSTS_ERROR) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN 
IsHostSysErr(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  )
{
  UINT16      Status ;
  
  //
  // Detect whether the interrupt is caused by serious error.
  // see "UHCI Design Guid".
  //
  Status = ReadUHCStatusReg (PciIo,StatusRegAddr) ;
  
  if (Status & USBSTS_HSE) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN 
IsHCProcessErr(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  )
{
  UINT16      Status ;
  
  //
  // Detect whether the interrupt is caused by fatal error.
  // see "UHCI Design Guid".
  //
  Status = ReadUHCStatusReg (PciIo,StatusRegAddr) ;
  
  if (Status & USBSTS_HCPE) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN 
IsHCHalted(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  )
{
  UINT16      Status ;
  
  //
  // Detect whether the the Host Controller is halted.
  //
  Status = ReadUHCStatusReg (PciIo,StatusRegAddr) ;
    
  if (Status & USBSTS_HCH) {
    return TRUE;
  } else {
    return FALSE;
  }
}


VOID
SetFrameNumberReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FrameNumAddrOffset,
  IN  UINT16                Index
 )
{
  //
  // Modifies the USB frame number register. 
  // It is assumed that the HC schedule execution is stopped.
  //
  Index &= 0x03FF;
  WriteUHCFrameNumberReg (PciIo,FrameNumAddrOffset,Index);
}


UINT16 
GetCurrentFrameNumber(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN   UINT32               FrameNumAddrOffset
  )
{
  //
  // Gets value in the USB frame number register. 
  //
  return (UINT16)(ReadUHCFrameNumberReg (PciIo,FrameNumAddrOffset) & 0x03FF);
}

VOID 
SetFrameListBaseAddress(
  EFI_PCI_IO_PROTOCOL   *PciIo,
  UINT32                FlBaseAddrReg,
  UINT32                Addr
  )
{
  //
  // Sets value in the USB Frame List Base Address register. 
  //
  WriteUHCFrameListBaseReg(PciIo,FlBaseAddrReg,(UINT32)(Addr & 0xFFFFF000));
}

UINT32 
GetFrameListBaseAddress(
  EFI_PCI_IO_PROTOCOL   *PciIo,
  UINT32                    FLBAddr
)
{
  //
  // Gets value from the USB Frame List Base Address register. 
  //
  return ReadUHCFrameListBaseReg(PciIo,FLBAddr) & 0xFFFFF000;
}


VOID
EnableUhc (
  IN EFI_PCI_IO_PROTOCOL  *PciIo
  )
{
  UINT16  Command;
  
  //
  // Enable Bas master & IO access
  //
  Command = 0x05;
  PciIo->Pci.Write (
          PciIo,
          EfiPciIoWidthUint16,
          0x04,                 //command reg
          1,
          &Command
         );          

  return;
}

VOID
EnableMaxPacketSize (
  USB_HC_DEV          *HcDev
  )
{
  UINT16    CommandContent;
  
  CommandContent = ReadUHCCommandReg (
                     HcDev->PciIo,
                     (UINT32)(USBCMD)
                   );

  if ((CommandContent & USBCMD_MAXP) != USBCMD_MAXP) {
    CommandContent |= USBCMD_MAXP;
    WriteUHCCommandReg (
      HcDev->PciIo,
      (UINT32)(USBCMD),
      CommandContent
    );
  }

  return;
}

VOID
SelfLinkBulkTransferQH (
  IN  QH_STRUCT *ptrQH
  )
{
  if (ptrQH == NULL) {
    return;
  }
  
  //
  // Make the QH's horizontal link pointer pointing to itself.
  //
  ptrQH->ptrNext = ptrQH;
  SetQHHorizontalQHorTDSelect(ptrQH,TRUE);
  SetQHHorizontalLinkPtr(ptrQH,ptrQH);
  SetQHHorizontalValidorInvalid(ptrQH,TRUE);
  
  return;
}    

EFI_STATUS
CreateFrameList (
  USB_HC_DEV                *HcDev,
  UINT32                    FlBaseAddrReg
  )
{
  EFI_STATUS              Status ;
  VOID                    *CommonBuffer;
  EFI_PHYSICAL_ADDRESS    MappedAddress;
  VOID                    *Mapping;
  UINTN                   BufferSizeInPages;
  UINTN                   BufferSizeInBytes;
  
  //
  // The Frame List is a common buffer that will be 
  // accessed by both the cpu and the usb bus master
  // at the same time.
  // The Frame List ocupies 4K bytes, 
  // and must be aligned on 4-Kbyte boundaries.
  //
  BufferSizeInBytes = 4096;
  BufferSizeInPages = EFI_SIZE_TO_PAGES(BufferSizeInBytes);
  Status = HcDev->PciIo->AllocateBuffer (HcDev->PciIo,
                                         AllocateAnyPages,
                                         EfiBootServicesData,
                                         BufferSizeInPages,
                                         &CommonBuffer,
                                         0
                                         );
  if(EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = HcDev->PciIo->Map (HcDev->PciIo,
                              EfiPciIoOperationBusMasterCommonBuffer,
                              CommonBuffer,
                              &BufferSizeInBytes,
                              &MappedAddress,
                              &Mapping
                              );
  if (EFI_ERROR(Status) || (BufferSizeInBytes != 4096)) {
    HcDev->PciIo->FreeBuffer (HcDev->PciIo,BufferSizeInPages,CommonBuffer);
    return EFI_UNSUPPORTED;
  }
  
  HcDev->FrameListEntry = (FRAMELIST_ENTRY*)((UINTN)MappedAddress);
  
  HcDev->FrameListMapping = Mapping;

  InitFrameList (HcDev);
  
  //
  // Tell the Host Controller where the Frame List lies,
  // by set the Frame List Base Address Register.
  //  
  SetFrameListBaseAddress(
    HcDev->PciIo,
    FlBaseAddrReg,
    (UINT32)((UINTN)HcDev->FrameListEntry)
  );


  return EFI_SUCCESS;
}

EFI_STATUS
FreeFrameListEntry(
  USB_HC_DEV        *HcDev
  )
{
  //
  // Unmap the common buffer for framelist entry,
  // and free the common buffer.
  // Uhci's frame list occupy 4k memory.
  //
  HcDev->PciIo->Unmap (HcDev->PciIo,HcDev->FrameListMapping);
  HcDev->PciIo->FreeBuffer (
                      HcDev->PciIo,
                      EFI_SIZE_TO_PAGES(4096),
                      (VOID*)(HcDev->FrameListEntry)
                      );
  return EFI_SUCCESS;
}    

VOID
InitFrameList (
  USB_HC_DEV    *HcDev
  )
{
  FRAMELIST_ENTRY     *FrameListPtr;
  UINTN               Index;
  
  //
  // Validate each Frame List Entry
  //
  FrameListPtr = HcDev->FrameListEntry;
  for(Index = 0 ; Index < 1024 ; Index ++) {
    FrameListPtr->FrameListPtrTerminate = 1 ;
    FrameListPtr->FrameListPtr = 0;
    FrameListPtr->FrameListPtrQSelect = 0;
    FrameListPtr->FrameListRsvd = 0;
    FrameListPtr++;
  }
}

////////////////////////////////////////////////////////////////
//
// QH TD related Helper Functions
//
////////////////////////////////////////////////////////////////

//
// functions for QH
//

EFI_STATUS
AllocateQHStruct(
  USB_HC_DEV            *HcDev,
  OUT QH_STRUCT         **ppQHStruct
)
{
  EFI_STATUS  Status;
  
  *ppQHStruct = NULL;
  
  //
  // QH must align on 16 bytes alignment,
  // since the memory allocated by UhciAllocatePool ()
  // is aligned on 32 bytes, it is no need to adjust
  // the allocated memory returned.
  //
  Status = UhciAllocatePool (HcDev,(UINT8**)ppQHStruct,sizeof(QH_STRUCT));
   
  if(EFI_ERROR(Status)) {
    return Status;
  }    
  
  EfiZeroMem(*ppQHStruct, sizeof(QH_STRUCT));
  
  return EFI_SUCCESS;
}

VOID
InitQH(
  IN  QH_STRUCT *ptrQH
)
{
  //
  // Make QH ready
  //
  SetQHHorizontalValidorInvalid(ptrQH, FALSE);
  SetQHVerticalValidorInvalid(ptrQH, FALSE);
  ptrQH->LoopPtr = NULL;
}

EFI_STATUS
CreateQH(
  USB_HC_DEV          *HcDev,
  QH_STRUCT           **pptrQH
  )
{
  EFI_STATUS    Status;

  //
  // allocate align memory for QH_STRUCT
  //
  Status = AllocateQHStruct(HcDev,pptrQH) ;
  if(EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES ;
  }

  //
  // init each field of the QH_STRUCT
  //
  InitQH(*pptrQH) ;

  return EFI_SUCCESS ;
}

void
SetQHHorizontalLinkPtr(
  IN QH_STRUCT  *ptrQH,
  IN VOID       *ptrNext
)
{
  //
  // Since the QH_STRUCT is aligned on 16-byte boundaries,
  // Only the highest 28bit of the address is valid 
  // (take 32bit address as an example).
  //
  ptrQH->QH.QHHorizontalPtr = (UINT32)((UINTN)ptrNext >> 4);
}

VOID*
GetQHHorizontalLinkPtr(
  IN QH_STRUCT  *ptrQH
)
{
  //
  // Restore the 28bit address to 32bit address 
  //(take 32bit address as an example)
  //
  return ((VOID*)((UINTN)(ptrQH->QH.QHHorizontalPtr << 4)));
}

VOID
SetQHHorizontalQHorTDSelect(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    bQH
)
{
  //
  // if QH is connected, the specified bit is set,
  // if TD is connected, the specified bit is cleared.
  //
  ptrQH->QH.QHHorizontalQSelect = bQH ? 1 : 0;
}


VOID
SetQHHorizontalValidorInvalid(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    bValid
)
{
  //
  // Valid means the horizontal link pointer is valid,
  // else, it's invalid.
  //
  ptrQH->QH.QHHorizontalTerminate = bValid ? 0 : 1;
}

VOID
SetQHVerticalLinkPtr(
  IN QH_STRUCT  *ptrQH,
  IN VOID     *ptrNext
)
{
  //
  // Since the QH_STRUCT is aligned on 16-byte boundaries,
  // Only the highest 28bit of the address is valid 
  // (take 32bit address as an example).
  //
  ptrQH->QH.QHVerticalPtr = (UINT32)((UINTN)ptrNext >> 4);
}

VOID*
GetQHVerticalLinkPtr(
  IN QH_STRUCT  *ptrQH
)
{
  //
  // Restore the 28bit address to 32bit address 
  //(take 32bit address as an example)
  //
  return ((VOID*)((UINTN)(ptrQH->QH.QHVerticalPtr << 4)));
}

VOID
SetQHVerticalQHorTDSelect(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    bQH
)
{
  //
  // Set the specified bit if the Vertical Link Pointer pointing to a QH,
  // Clear the specified bit if the Vertical Link Pointer pointing to a TD.
  //
  ptrQH->QH.QHVerticalQSelect = bQH ? 1 : 0;
}

BOOLEAN
IsQHHorizontalQHSelect(
  IN QH_STRUCT  *ptrQH
)
{
  //
  // Retrieve the information about whether the Horizontal Link Pointer
  // pointing to a QH or TD.
  //
  return (BOOLEAN)(ptrQH->QH.QHHorizontalQSelect? TRUE : FALSE);
}

VOID
SetQHVerticalValidorInvalid(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    IsValid
)
{
  //
  // If TRUE, indicates the Vertical Link Pointer field is valid,
  // else, the field is invalid.
  //
  ptrQH->QH.QHVerticalTerminate = IsValid ? 0 : 1;
}


BOOLEAN
GetQHVerticalValidorInvalid(
  IN QH_STRUCT  *ptrQH
)
{
  //
  // If TRUE, indicates the Vertical Link Pointer field is valid,
  // else, the field is invalid.
  //
  return (BOOLEAN)(!(ptrQH->QH.QHVerticalTerminate));
}

BOOLEAN
GetQHHorizontalValidorInvalid(
  IN QH_STRUCT  *ptrQH
)
{
  //
  // If TRUE, meaning the Horizontal Link Pointer field is valid,
  // else, the field is invalid.
  //
  return (BOOLEAN)(!(ptrQH->QH.QHHorizontalTerminate));
}

//
// functions for TD
//
EFI_STATUS
AllocateTDStruct(
  USB_HC_DEV        *HcDev,
  TD_STRUCT         **ppTDStruct
  )
{
  EFI_STATUS  Status;
  
  *ppTDStruct = NULL;
  
  //
  // TD must align on 16 bytes alignment,
  // since the memory allocated by UhciAllocatePool ()
  // is aligned on 32 bytes, it is no need to adjust
  // the allocated memory returned.
  //
  Status = UhciAllocatePool (HcDev,
                           (UINT8**)ppTDStruct,
                           sizeof(TD_STRUCT)
                           );   
  if(EFI_ERROR(Status)) {
    return Status;
  }
  
  EfiZeroMem(*ppTDStruct, sizeof (TD_STRUCT));
  
  return EFI_SUCCESS;
}

VOID
InitTD(
  IN  TD_STRUCT *ptrTD
)
{
  //
  // Make TD ready.
  //
  SetTDLinkPtrValidorInvalid(ptrTD, FALSE);
}

EFI_STATUS
CreateTD(
  USB_HC_DEV      *HcDev,
  TD_STRUCT       **pptrTD
  )
{
  EFI_STATUS      Status;
  //
  // create memory for TD_STRUCT, and align the memory.
  //
  Status = AllocateTDStruct(HcDev,pptrTD) ;
  if(EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitTD(*pptrTD);

  return EFI_SUCCESS ;
}

EFI_STATUS
GenSetupStageTD(
  IN  USB_HC_DEV          *HcDev,
  IN  UINT8               DevAddr,
  IN  UINT8               Endpoint,
  IN  BOOLEAN             bSlow,
  IN  UINT8               *pDevReq,
  IN  UINT8               RequestLen,
  OUT TD_STRUCT           **ppTD
)
{
  EFI_STATUS      Status;
  TD_STRUCT       *pTDStruct;

  Status = CreateTD(HcDev,&pTDStruct);
  if(EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetTDLinkPtr(pTDStruct,NULL);

  //
  // Depth first fashion
  //
  SetTDLinkPtrDepthorBreadth(pTDStruct, TRUE);

  //SetTDLinkPtrQHorTDSelect(pTDStruct,FALSE) ;

  //
  // initialize as the last TD in the QH context,
  // this field will be updated in the TD linkage process.
  //
  SetTDLinkPtrValidorInvalid(pTDStruct, FALSE);
  
  //
  // Disable Short Packet Detection by default
  //
  EnableorDisableTDShortPacket (pTDStruct, FALSE) ;

  //
  // Max error counter is 3, retry 3 times when error encountered.
  //
  SetTDControlErrorCounter(pTDStruct, 3);

  //
  // set device speed attribute 
  // (TRUE - Slow Device; FALSE - Full Speed Device)
  //
  SetTDLoworFullSpeedDevice (pTDStruct, bSlow);

  //
  // Non isochronous transfer TD
  //
  SetTDControlIsochronousorNot(pTDStruct,FALSE) ;

  //
  // Interrupt On Complete bit be set to zero,
  // Disable IOC interrupt.
  //
  SetorClearTDControlIOC(pTDStruct,FALSE) ;

  //
  // Set TD Active bit
  //
  SetTDStatusActiveorInactive(pTDStruct, TRUE);
  
  SetTDTokenMaxLength(pTDStruct, RequestLen) ;
  
  SetTDTokenDataToggle0(pTDStruct);

  SetTDTokenEndPoint(pTDStruct, Endpoint);

  SetTDTokenDeviceAddress (pTDStruct, DevAddr);

  SetTDTokenPacketID(pTDStruct, SETUP_PACKET_ID);
  
  pTDStruct->pTDBuffer = (UINT8*)pDevReq;
  pTDStruct->TDBufferLength = RequestLen;
  SetTDDataBuffer(pTDStruct);

  *ppTD = pTDStruct;

  return EFI_SUCCESS;
}

EFI_STATUS
GenDataTD(
  IN  USB_HC_DEV          *HcDev,
  IN  UINT8               DevAddr,
  IN  UINT8               Endpoint,
  IN  UINT8               *pData,
  IN  UINT8               Len,
  IN  UINT8               PktID,
  IN  UINT8               Toggle,
  IN  BOOLEAN             bSlow,
  OUT TD_STRUCT           **ppTD
  )
{
  TD_STRUCT   *pTDStruct;
  EFI_STATUS  Status;

  Status = CreateTD(HcDev,&pTDStruct);
  if(EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetTDLinkPtr(pTDStruct,NULL) ;

  //
  // Depth first fashion
  //
  SetTDLinkPtrDepthorBreadth(pTDStruct,TRUE);

  //
  // Link pointer pointing to TD struct
  //
  SetTDLinkPtrQHorTDSelect(pTDStruct,FALSE);

  //
  // initialize as the last TD in the QH context,
  // this field will be updated in the TD linkage process.
  //
  SetTDLinkPtrValidorInvalid(pTDStruct, FALSE);

  //
  // Disable short packet detect
  //
  EnableorDisableTDShortPacket (pTDStruct, FALSE) ;
  //
  // Max error counter is 3
  //
  SetTDControlErrorCounter(pTDStruct, 3);

  //
  // set device speed attribute
  // (TRUE - Slow Device; FALSE - Full Speed Device)
  //
  SetTDLoworFullSpeedDevice (pTDStruct, bSlow);

  //
  // Non isochronous transfer TD
  //
  SetTDControlIsochronousorNot(pTDStruct,FALSE) ;

  //
  // Disable Interrupt On Complete
  // Disable IOC interrupt.
  //
  SetorClearTDControlIOC(pTDStruct,FALSE) ;

  //
  // Set Active bit
  //
  SetTDStatusActiveorInactive(pTDStruct, TRUE);
  
  SetTDTokenMaxLength(pTDStruct, Len) ;
  
  if(Toggle) {
    SetTDTokenDataToggle1(pTDStruct);
  } else {
    SetTDTokenDataToggle0(pTDStruct);
  }
  
  SetTDTokenEndPoint(pTDStruct, Endpoint);

  SetTDTokenDeviceAddress(pTDStruct, DevAddr);

  SetTDTokenPacketID(pTDStruct, PktID);
  
  pTDStruct->pTDBuffer = (UINT8*)pData;
  pTDStruct->TDBufferLength = Len;
  SetTDDataBuffer(pTDStruct);
  *ppTD = pTDStruct;

  return EFI_SUCCESS;
}


EFI_STATUS
CreateStatusTD(
  IN  USB_HC_DEV          *HcDev,
  IN  UINT8               DevAddr,
  IN  UINT8               Endpoint,
  IN  UINT8               PktID,
  IN  BOOLEAN             bSlow,
  OUT TD_STRUCT           **ppTD
)
{
  TD_STRUCT   *ptrTDStruct;
  EFI_STATUS  Status;

  Status = CreateTD(HcDev,&ptrTDStruct);
  if(EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetTDLinkPtr(ptrTDStruct,NULL) ;

  // Depth first fashion
  SetTDLinkPtrDepthorBreadth(ptrTDStruct, TRUE);

  //SetTDLinkPtrQHorTDSelect(pTDStruct,FALSE) ;

  //
  // initialize as the last TD in the QH context,
  // this field will be updated in the TD linkage process.
  //
  SetTDLinkPtrValidorInvalid(ptrTDStruct, FALSE);
  
  //
  // Disable short packet detect
  //
  EnableorDisableTDShortPacket (ptrTDStruct, FALSE) ;
  
  //
  // Max error counter is 3
  //
  SetTDControlErrorCounter(ptrTDStruct, 3);

  //
  // set device speed attribute
  // (TRUE - Slow Device; FALSE - Full Speed Device)
  //
  SetTDLoworFullSpeedDevice (ptrTDStruct, bSlow);

  //
  // Non isochronous transfer TD
  //
  SetTDControlIsochronousorNot(ptrTDStruct,FALSE) ;

  //
  // Disable Interrupt On Complete
  // Disable IOC interrupt.
  //
  SetorClearTDControlIOC(ptrTDStruct,FALSE) ;

  //
  // Set TD Active bit
  //
  SetTDStatusActiveorInactive(ptrTDStruct, TRUE);

  SetTDTokenMaxLength(ptrTDStruct, 0) ;
  
  SetTDTokenDataToggle1(ptrTDStruct);

  SetTDTokenEndPoint(ptrTDStruct, Endpoint);

  SetTDTokenDeviceAddress(ptrTDStruct, DevAddr);

  SetTDTokenPacketID(ptrTDStruct, PktID);
  
  ptrTDStruct->pTDBuffer = NULL;
  ptrTDStruct->TDBufferLength = 0;
  SetTDDataBuffer(ptrTDStruct);

  *ppTD = ptrTDStruct;

  return EFI_SUCCESS;
}


VOID
SetTDLinkPtrValidorInvalid(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bValid
)
{
  //
  // Valid means the link pointer is valid,
  // else, it's invalid.
  //
  ptrTDStruct->TDData.TDLinkPtrTerminate = (bValid ? 0 : 1);
}

VOID
SetTDLinkPtrQHorTDSelect(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bQH
)
{
  //
  // Indicate whether the Link Pointer pointing to a QH or TD
  //
  ptrTDStruct->TDData.TDLinkPtrQSelect = (bQH ? 1 : 0);
}

VOID
SetTDLinkPtrDepthorBreadth(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN     bDepth
)
{
  //
  // If TRUE, indicating the host controller should process in depth first 
  // fashion,
  // else, the host controller should process in breadth first fashion
  //
  ptrTDStruct->TDData.TDLinkPtrDepthSelect = (bDepth ? 1 : 0) ;
}

VOID
SetTDLinkPtr(
  IN  TD_STRUCT *ptrTDStruct,
  IN  VOID    *ptrNext
)
{
  //
  // Set TD Link Pointer. Since QH,TD align on 16-byte boundaries,
  // only the highest 28 bits are valid. (if take 32bit address as an example)
  //
  ptrTDStruct->TDData.TDLinkPtr = (UINT32)((UINTN)ptrNext >> 4);
}

VOID*
GetTDLinkPtr(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Get TD Link Pointer. Restore it back to 32bit
  // (if take 32bit address as an example)
  //
  return ((VOID*)((UINTN)(ptrTDStruct->TDData.TDLinkPtr << 4)));
}

BOOLEAN
IsTDLinkPtrQHOrTD (
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Get the information about whether the Link Pointer field pointing to
  // a QH or a TD.
  //
  return (BOOLEAN)(ptrTDStruct->TDData.TDLinkPtrQSelect);
}

VOID
EnableorDisableTDShortPacket(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bEnable
)
{
  //
  // TRUE means enable short packet detection mechanism.
  //
  ptrTDStruct->TDData.TDStatusSPD = (bEnable ? 1 : 0);
}

VOID
SetTDControlErrorCounter(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINT8   nMaxErrors
)
{
  //
  // valid value of nMaxErrors is 0,1,2,3
  //
  if(nMaxErrors > 3) {
    nMaxErrors = 3;
  }
  ptrTDStruct->TDData.TDStatusErr = nMaxErrors;
}


VOID
SetTDLoworFullSpeedDevice(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bLowSpeedDevice
)
{
  //
  // TRUE means the TD is targeting at a Low-speed device
  //
  ptrTDStruct->TDData.TDStatusLS = (bLowSpeedDevice ? 1 : 0);
}

VOID
SetTDControlIsochronousorNot (
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   IsIsochronous
)
{
  //
  // TRUE means the TD belongs to Isochronous transfer type.
  //
  ptrTDStruct->TDData.TDStatusIOS = (IsIsochronous ? 1 : 0);
}

VOID
SetorClearTDControlIOC(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   IsSet
)
{
  //
  // If this bit is set, it indicates that the host controller should issue
  // an interrupt on completion of the frame in which this TD is executed.
  //
  ptrTDStruct->TDData.TDStatusIOC = IsSet ? 1 : 0;
}

VOID
SetTDStatusActiveorInactive(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   IsActive
)
{
  //
  // If this bit is set, it indicates that the TD is active and can be
  // executed.
  //
  if (IsActive) {
    ptrTDStruct->TDData.TDStatus |= 0x80;
  } else {
    ptrTDStruct->TDData.TDStatus &= 0x7F;
  }
}

UINT16
SetTDTokenMaxLength(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINT16    MaximumLength
)
{
  //
  // Specifies the maximum number of data bytes allowed for the transfer.
  // the legal value extent is 0 ~ 0x500.
  //
  if(MaximumLength > 0x500) {
    MaximumLength = 0x500 ;
  }
  ptrTDStruct->TDData.TDTokenMaxLen = MaximumLength - 1;

  return MaximumLength;
}

VOID
SetTDTokenDataToggle1(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Set the data toggle bit to DATA1
  //
  ptrTDStruct->TDData.TDTokenDataToggle = 1;
}

VOID
SetTDTokenDataToggle0(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Set the data toggle bit to DATA0
  //
  ptrTDStruct->TDData.TDTokenDataToggle = 0;
}

UINT8
GetTDTokenDataToggle(
  IN  TD_STRUCT *ptrTDStruct
  )
{
  //
  // Get the data toggle value.
  //
  return (UINT8)(ptrTDStruct->TDData.TDTokenDataToggle) ;
}

VOID
SetTDTokenEndPoint(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINTN     EndPoint
)
{
  //
  // Set EndPoint Number the TD is targeting at.
  //
  ptrTDStruct->TDData.TDTokenEndPt = (UINT8)EndPoint;
}

VOID
SetTDTokenDeviceAddress(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINTN     DeviceAddress
)
{
  //
  // Set Device Address the TD is targeting at.
  //
  ptrTDStruct->TDData.TDTokenDevAddr = (UINT8)DeviceAddress;
}

VOID
SetTDTokenPacketID(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINT8     PID
)
{
  //
  // Set the Packet Identification to be used for this transaction.
  //
  ptrTDStruct->TDData.TDTokenPID = PID;
}

VOID
SetTDDataBuffer(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Set the beginning address of the data buffer that will be used
  // during the transaction.
  //
  ptrTDStruct->TDData.TDBufferPtr = (UINT32)((UINTN)(ptrTDStruct->pTDBuffer));
}

BOOLEAN
IsTDStatusActive(
  IN  TD_STRUCT *ptrTDStruct
)
{
  UINT8 TDStatus;
  
  //
  // Detect whether the TD is active.
  //
  TDStatus = (UINT8)(ptrTDStruct->TDData.TDStatus);
  return (BOOLEAN)(TDStatus & 0x80);
}

BOOLEAN
IsTDStatusStalled(
  IN  TD_STRUCT *ptrTDStruct
)
{
  UINT8 TDStatus;
  
  //
  // Detect whether the device/endpoint addressed by this TD is stalled.
  //
  TDStatus = (UINT8)(ptrTDStruct->TDData.TDStatus);
  return (BOOLEAN)(TDStatus & 0x40);
}

BOOLEAN
IsTDStatusBufferError(
  IN  TD_STRUCT *ptrTDStruct
)
{
  UINT8 TDStatus;
  
  //
  // Detect whether Data Buffer Error is happened.
  //
  TDStatus = (UINT8)(ptrTDStruct->TDData.TDStatus);
  return (BOOLEAN)(TDStatus & 0x20);
}


BOOLEAN 
IsTDStatusBabbleError(
  IN  TD_STRUCT *ptrTDStruct
)
{
  UINT8 TDStatus;
  
  //
  // Detect whether Babble Error is happened.
  //
  TDStatus = (UINT8)(ptrTDStruct->TDData.TDStatus);
  return (BOOLEAN)(TDStatus & 0x10);
}

BOOLEAN
IsTDStatusNAKReceived(
  IN  TD_STRUCT *ptrTDStruct
)
{
  UINT8 TDStatus;
  
  //
  // Detect whether NAK is received.
  //
  TDStatus = (UINT8)(ptrTDStruct->TDData.TDStatus);
  return (BOOLEAN)(TDStatus & 0x08);
}

BOOLEAN 
IsTDStatusCRCTimeOutError(
  IN  TD_STRUCT *ptrTDStruct
)
{
  UINT8 TDStatus;
  
  //
  // Detect whether CRC/Time Out Error is encountered.
  //
  TDStatus = (UINT8)(ptrTDStruct->TDData.TDStatus);
  return (BOOLEAN)(TDStatus & 0x04);
}

BOOLEAN 
IsTDStatusBitStuffError(
  IN  TD_STRUCT *ptrTDStruct
)
{
  UINT8 TDStatus;
  
  //
  // Detect whether Bitstuff Error is received.
  //
  TDStatus = (UINT8)(ptrTDStruct->TDData.TDStatus);
  return (BOOLEAN)(TDStatus & 0x02);
}

UINT16
GetTDStatusActualLength(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Retrieve the actual number of bytes that were tansferred.
  // the value is encoded as n-1. so return the decoded value.
  //
  return (UINT16)((ptrTDStruct->TDData.TDStatusActualLength) + 1);
}

UINT16 
GetTDTokenMaxLength(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Retrieve the maximum number of data bytes allowed for the trnasfer.
  //
  return (UINT16)((ptrTDStruct->TDData.TDTokenMaxLen) + 1);
}

UINT8 
GetTDTokenEndPoint(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Retrieve the endpoint number the transaction is targeting at.
  //
  return (UINT8)(ptrTDStruct->TDData.TDTokenEndPt);
}

UINT8 
GetTDTokenDeviceAddress(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Retrieve the device address the transaction is targeting at.
  //
  return (UINT8)(ptrTDStruct->TDData.TDTokenDevAddr);
}

UINT8 
GetTDTokenPacketID(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Retrieve the Packet Identification information.
  //
  return (UINT8)(ptrTDStruct->TDData.TDTokenPID);
}

UINT8*
GetTDDataBuffer(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Retrieve the beginning address of the data buffer 
  // that involved in this transaction.
  //
  return ptrTDStruct->pTDBuffer;
}

BOOLEAN
GetTDLinkPtrValidorInvalid(
  IN  TD_STRUCT *ptrTDStruct
)
{
  //
  // Retrieve the information of whether the Link Pointer field 
  // is valid or not.
  //
  if (ptrTDStruct->TDData.TDLinkPtrTerminate) {
    return FALSE;
  } else {
    return TRUE;
  }

}

UINTN
CountTDsNumber(
  IN  TD_STRUCT *ptrFirstTD
  )
{
  UINTN     Number;
  TD_STRUCT *ptr;
  
  //
  // Count the queued TDs number.
  //
  Number = 0 ;
  ptr = ptrFirstTD ;
  while(ptr) {
    ptr = (TD_STRUCT*)ptr->ptrNextTD ;
    Number ++ ;
  }

  return Number ;
}



VOID
LinkTDToQH(
  IN  QH_STRUCT *ptrQH,
  IN  TD_STRUCT *ptrTD
)
{
  if (ptrQH == NULL || ptrTD == NULL) {
    return;
  }
  //
  //  Validate QH Vertical Ptr field
  //
  SetQHVerticalValidorInvalid(ptrQH, TRUE);

  //
  //  Vertical Ptr pointing to TD structure
  //
  SetQHVerticalQHorTDSelect(ptrQH, FALSE);

  SetQHVerticalLinkPtr(ptrQH, (VOID*)ptrTD);

  ptrQH->ptrDown = (VOID*)ptrTD;
}

VOID
LinkTDToTD(
  IN  TD_STRUCT *ptrPreTD,
  IN  TD_STRUCT *ptrTD
  )
{
  if (ptrPreTD == NULL || ptrTD == NULL) {
    return ;
  }
  //
  // Depth first fashion
  //
  SetTDLinkPtrDepthorBreadth(ptrPreTD, TRUE);

  //
  // Link pointer pointing to TD struct
  //
  SetTDLinkPtrQHorTDSelect(ptrPreTD,FALSE) ;
  
  //
  // Validate the link pointer valid bit
  //
  SetTDLinkPtrValidorInvalid(ptrPreTD, TRUE);

  SetTDLinkPtr(ptrPreTD,ptrTD) ;

  ptrPreTD->ptrNextTD = (VOID*)ptrTD ;  
}


//
// Transfer Schedule related Helper Functions
//
VOID
SetorClearCurFrameListTerminate(
  IN  FRAMELIST_ENTRY   *pCurEntry,
  IN  BOOLEAN           IsSet
  )
{
  //
  // If TRUE, empty the frame. If FALSE, indicate the Pointer field is valid.
  //
  pCurEntry-> FrameListPtrTerminate = (IsSet ? 1 : 0) ;
}

VOID
SetCurFrameListQHorTD(
  IN  FRAMELIST_ENTRY    *pCurEntry,
  IN  BOOLEAN            IsQH
  )
{
  //
  // This bit indicates to the hardware whether the item referenced by the
  // link pointer is a TD or a QH.
  //
  pCurEntry-> FrameListPtrQSelect = (IsQH ? 1 : 0) ;
}


BOOLEAN
IsCurFrameListQHorTD(
  IN  FRAMELIST_ENTRY    *pCurEntry
  )  
{
  //
  //TRUE is QH
  //FALSE is TD
  //
  return (BOOLEAN)(pCurEntry->FrameListPtrQSelect);
}

BOOLEAN
GetCurFrameListTerminate(
  IN  FRAMELIST_ENTRY    *pCurEntry
  )
{
  //
  // TRUE means the frame is empty,
  // FALSE means the link pointer field is valid.
  //
  return (BOOLEAN)(pCurEntry->FrameListPtrTerminate) ;
}

VOID
SetCurFrameListPointer(
  IN  FRAMELIST_ENTRY   *pCurEntry,
  IN  UINT8             *ptr
  )
{
  //
  // Set the pointer field of the frame.
  //
  pCurEntry-> FrameListPtr = (UINT32)((UINTN)ptr >> 4);
}

VOID*
GetCurFrameListPointer(
  IN  FRAMELIST_ENTRY    *pCurEntry
  )
{
  VOID     *ptr ;
  //
  // Get the link pointer of the frame.
  //
  ptr = (VOID*)((UINTN)(pCurEntry-> FrameListPtr << 4));
  return ptr;
}

VOID
LinkQHToFrameList(
  IN  FRAMELIST_ENTRY   *pEntry,
  IN  UINT16            FrameListIndex,
  IN  QH_STRUCT         *ptrQH
  ) 
{
  FRAMELIST_ENTRY     *pCurFrame ;
  QH_STRUCT           *TempQH;
  QH_STRUCT           *NextTempQH;
  TD_STRUCT           *TempTD;
  BOOLEAN             LINK;
  
  //
  // Get frame list entry that the link process will begin from.
  //
  pCurFrame = pEntry + FrameListIndex ; 

  //
  // if current frame is empty 
  // then link the specified QH directly to the Frame List.
  // 
  if (GetCurFrameListTerminate (pCurFrame)) {  
    
    //
    // Link new QH to the frame list entry.
    //
    SetCurFrameListQHorTD (pCurFrame, TRUE) ;
    
    SetCurFrameListPointer (pCurFrame, (UINT8 *)ptrQH) ;
    
    //
    // clear T bit in the Frame List, indicating that the frame list entry 
    // is no longer empty.
    //
    SetorClearCurFrameListTerminate(pCurFrame, FALSE) ;
    
    return;
    
  } else {        
    //
    // current frame list has link pointer
    //
    if (!IsCurFrameListQHorTD (pCurFrame)) {
    //  
    //  a TD is linked to the framelist entry
    //
      TempTD = (TD_STRUCT*)GetCurFrameListPointer (pCurFrame);
      
      while (GetTDLinkPtrValidorInvalid (TempTD)) {
        
        if (IsTDLinkPtrQHOrTD (TempTD)) { // QH linked next to the TD
          break;
        }
        
        TempTD = (TD_STRUCT*)GetTDLinkPtr (TempTD);
      }
      
      //
      // either no ptr linked next to the TD or QH is linked next to the TD
      //
      if (!GetTDLinkPtrValidorInvalid (TempTD)) {
        
        //
        // no ptr linked next to the TD
        //
        TempTD->ptrNextQH = ptrQH;        
        SetTDLinkPtrQHorTDSelect (TempTD,TRUE);
        SetTDLinkPtr (TempTD,ptrQH);
        SetTDLinkPtrValidorInvalid(TempTD,TRUE) ;
        //ptrQH->ptrNext = NULL; 
        
        return;
        
      } else {
        //
        //  QH is linked next to the TD
        //
        TempQH = (QH_STRUCT*)GetTDLinkPtr (TempTD) ;
      }
    
    } else {  
    //
    // a QH is linked to the framelist entry
    //
      TempQH = (QH_STRUCT*)GetCurFrameListPointer(pCurFrame) ;
    }
    
    //
    // Set up Flag
    //
    LINK = TRUE;
    
    //
    // avoid the same qh repeated linking in one frame entry
    //
    if (TempQH == ptrQH) {
      LINK = FALSE;
    }
    //
    // if current QH has next QH connected
    //
    while (GetQHHorizontalValidorInvalid (TempQH)) { 

      //
      // Get next QH pointer
      //
      NextTempQH = (QH_STRUCT*)GetQHHorizontalLinkPtr (TempQH);
      
      //
      // Bulk transfer qh may be self-linked,
      // so, the code below is to avoid dead-loop when meeting self-linked qh
      //
      if (NextTempQH == TempQH) {
        LINK = FALSE;
        break;
      }
      
      TempQH = NextTempQH;
      
      //
      // avoid the same qh repeated linking in one frame entry
      //
      if (TempQH == ptrQH) {
        LINK = FALSE;
      }
    }
     
    if (LINK) {
       // Link
      TempQH->ptrNext = ptrQH;
      SetQHHorizontalQHorTDSelect(TempQH,TRUE) ;
      SetQHHorizontalLinkPtr(TempQH,ptrQH) ;
      SetQHHorizontalValidorInvalid(TempQH,TRUE) ;
      //ptrQH->ptrNext = NULL;
    }   
    
    return;
      
  }

}

EFI_STATUS
ExecuteControlTransfer (
  IN  USB_HC_DEV  *HcDev,
  IN  TD_STRUCT   *ptrTD,
  IN  UINT32      wIndex,
  OUT UINTN       *ActualLen,
  IN  UINTN       TimeOut,
  OUT UINT32      *TransferResult
  )
{
  UINTN               ErrTDPos;
  UINTN               Delay;
  
  ErrTDPos          = 0 ;
  *TransferResult   = EFI_USB_NOERROR;  
  *ActualLen        = 0;
  
  Delay = (TimeOut * STALL_1_MILLI_SECOND / 50) + 1;
        
  do {
      
    CheckTDsResults(ptrTD,TransferResult,&ErrTDPos,ActualLen);
    
    //
    // TD is inactive, means the control transfer is end.
    //
    if ((*TransferResult & EFI_USB_ERR_NOTEXECUTE) != EFI_USB_ERR_NOTEXECUTE) {
      break;
    }
    gBS->Stall(50);
      
  } while(Delay--);
    
    
  if(*TransferResult != EFI_USB_NOERROR) {
    return EFI_DEVICE_ERROR;
  }
    
  return EFI_SUCCESS;
}    


EFI_STATUS
ExecBulkorSyncInterruptTransfer (
  IN  USB_HC_DEV  *HcDev,
  IN  TD_STRUCT   *ptrTD,
  IN  UINT32      wIndex,
  OUT UINTN       *ActualLen,
  OUT UINT8       *DataToggle,
  IN  UINTN       TimeOut,
  OUT UINT32      *TransferResult
  )
{
  UINTN               ErrTDPos;
  UINTN               ScrollNum;
  UINTN               Delay;

  ErrTDPos          = 0;
  *TransferResult   = EFI_USB_NOERROR;
  *ActualLen        = 0;

  Delay = (TimeOut * STALL_1_MILLI_SECOND / 50) + 1;
    
  do {

    CheckTDsResults (ptrTD,TransferResult,&ErrTDPos,ActualLen);
    //
    // TD is inactive, thus meaning bulk transfer's end.
    //    
    if ((*TransferResult & EFI_USB_ERR_NOTEXECUTE) != EFI_USB_ERR_NOTEXECUTE) {
      break;
    }  
    gBS->Stall(50);   // Stall for 50 us

  } while (Delay--);
  
  //
  // has error
  //
  if(*TransferResult != EFI_USB_NOERROR) {
  
    //
    // scroll the Data Toggle back to the last success TD
    //
    ScrollNum = CountTDsNumber(ptrTD) - ErrTDPos;
    if(ScrollNum % 2) {
      *DataToggle ^= 1 ;
    }
    
    return EFI_DEVICE_ERROR ;
  }

  return EFI_SUCCESS;
} 

VOID
DelLinkSingleQH (
  IN  USB_HC_DEV      *HcDev,
  IN  QH_STRUCT       *ptrQH,
  IN  UINT16          FrameListIndex,
  IN  BOOLEAN         SearchOther,
  IN  BOOLEAN         Delete
  )
{
  FRAMELIST_ENTRY   *pCurFrame;
  UINTN             i;
  UINTN             BeginFrame;
  UINTN             EndFrame;
  QH_STRUCT         *CurrentQH;
  QH_STRUCT         *NextQH;
  QH_STRUCT         *LastLoopQH;
  TD_STRUCT         *CurrentTD;
  VOID              *PtrPreQH;
  BOOLEAN           Found;
  
  NextQH    = NULL;
  PtrPreQH  = NULL;
  Found     = FALSE;
  
  if (ptrQH == NULL) {
    return;
  }
  
  if (SearchOther) {
    BeginFrame = 0;
    EndFrame = 1024;
  } else {
    BeginFrame = FrameListIndex;
    EndFrame = FrameListIndex + 1;
  }
  
  for ( i = BeginFrame; i < EndFrame; i ++) {
    
    pCurFrame = HcDev->FrameListEntry + (i % 1024);
    
    if (GetCurFrameListTerminate (pCurFrame)) {
      //
      // current frame list is empty,search next frame list entry
      //
      continue;
    }
    
    if (!IsCurFrameListQHorTD (pCurFrame)) {  
    //
    // TD linked to current framelist
    //  
      CurrentTD = (TD_STRUCT*)GetCurFrameListPointer (pCurFrame);
      
      while (GetTDLinkPtrValidorInvalid (CurrentTD)) {
        
        if (IsTDLinkPtrQHOrTD (CurrentTD)) { 
        //
        // QH linked next to the TD,break while()
        //
          break;
        }
        
        CurrentTD = (TD_STRUCT*)GetTDLinkPtr (CurrentTD);
      }
      
      if (!GetTDLinkPtrValidorInvalid (CurrentTD)) {
      //
      // no QH linked next to the last TD,
      // search next frame list
      //
        continue;
      }
      
      //
      // a QH linked next to the last TD
      //
      CurrentQH = (QH_STRUCT*)GetTDLinkPtr (CurrentTD);
      
      PtrPreQH = CurrentTD;
    
    } else {  
    //
    // a QH linked to current framelist
    //      
      CurrentQH = (QH_STRUCT*)GetCurFrameListPointer (pCurFrame);
      
      PtrPreQH = NULL;
    }
    
    if (CurrentQH == ptrQH) {
      
      //
      // tell whether CurrentQH is in QH loop and is one of the ends of the 
      // loop,
      // if yes, break the QH loop.
      //
      if (IsQHsLooped (CurrentQH,&LastLoopQH)) {
        
        if ((CurrentQH == LastLoopQH) || (CurrentQH == LastLoopQH->LoopPtr)) {
        
          LastLoopQH->LoopPtr = NULL;
          SetQHHorizontalValidorInvalid(LastLoopQH,FALSE);
          SetQHHorizontalLinkPtr(LastLoopQH,NULL);
          LastLoopQH->ptrNext = NULL;
        }
      }
      
      if(GetQHHorizontalValidorInvalid (ptrQH)) {
      //
      // there is QH connected after the QH found
      //
        //
        // retrieve nex qh pointer of the qh found.
        //
        NextQH = GetQHHorizontalLinkPtr (ptrQH);
      } else {
        NextQH = NULL;
      }
      
      if (PtrPreQH) {  // QH linked to a TD struct
        
        CurrentTD = (TD_STRUCT*)PtrPreQH;
                
        SetTDLinkPtrValidorInvalid (CurrentTD,(BOOLEAN)((NextQH == NULL) ? FALSE : TRUE));
        SetTDLinkPtr (CurrentTD,NextQH);
        CurrentTD->ptrNextQH = NextQH;
        
      } else {  // QH linked directly to current framelist entry
        
        SetorClearCurFrameListTerminate (pCurFrame,(BOOLEAN)((NextQH == NULL) ? TRUE : FALSE));
        SetCurFrameListPointer (pCurFrame,(UINT8*)NextQH);
      }
      
      Found = TRUE;
      //
      // search next framelist entry
      //
      continue;
    }
    
    //
    // retrieve the Last QH in the Loop,
    // if no Loop, LastLoopQH = NULL
    //
    IsQHsLooped (CurrentQH,&LastLoopQH);
    
    while (CurrentQH != LastLoopQH) {
      
      PtrPreQH = CurrentQH;
      //
      // Get next horizontal linked QH
      //
      CurrentQH = (QH_STRUCT*)GetQHHorizontalLinkPtr (CurrentQH);
      
      //
      // the qh is found
      //
      if (CurrentQH == ptrQH) {
        break;
      }
    }
    
    if (CurrentQH != ptrQH) {
      //
      // search next frame list entry
      //
      continue;
    }   
    
    // 
    // find the specified qh, then delink it from
    // the horizontal QH list in the frame entry.
    //
    
    if (LastLoopQH != NULL) {
      //
      // break the qh loop 
      // if exists and the specified qh is one of the ends of the loop 
      //
      if ((CurrentQH == LastLoopQH) || (CurrentQH == LastLoopQH->LoopPtr)) {
        LastLoopQH->LoopPtr = NULL;
        SetQHHorizontalValidorInvalid(LastLoopQH,FALSE);
        SetQHHorizontalLinkPtr(LastLoopQH,NULL);
        LastLoopQH->ptrNext = NULL;
      }
    }
    
    if(GetQHHorizontalValidorInvalid (ptrQH)) {
    //
    // there is QH connected after the QH found
    //
      //
      // retrieve nex qh pointer of the qh found.
      //
      NextQH = GetQHHorizontalLinkPtr (ptrQH);

    } else {
    //
    // NO QH connected after the QH found
    //
      NextQH = NULL;
      //
      // NULL the previous QH's link ptr and set Terminate field.
      //
      SetQHHorizontalValidorInvalid((QH_STRUCT*)PtrPreQH,FALSE);
    }
    
    SetQHHorizontalLinkPtr((QH_STRUCT*)PtrPreQH,NextQH);
    ((QH_STRUCT*)PtrPreQH)->ptrNext = NextQH;

    Found = TRUE;
  }
  
  if (Found && Delete) {
    //
    // free memory once used by the specific QH
    //
    UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
  }

  return;
}


VOID
DeleteQueuedTDs(
  IN  USB_HC_DEV      *HcDev,
  IN  TD_STRUCT       *ptrFirstTD
  )
/*
  The last TD in the queue may be linked to itself.
  Must take this condition into account.
*/
{
  TD_STRUCT         *Tptr1;
  TD_STRUCT         *Tptr2;
  
  Tptr1 = ptrFirstTD;
  //
  // Delete all the TDs in a queue.
  //  
  while(Tptr1) {
    
    Tptr2 = Tptr1;
    
    if(!GetTDLinkPtrValidorInvalid (Tptr2)) {
      Tptr1 = NULL ;
    } else  {
      
      Tptr1 = GetTDLinkPtr(Tptr2);
      
      //
      // TD link to itself
      //
      if (Tptr1 == Tptr2) {
        Tptr1 = NULL;
      }
    }

    UhciFreePool(HcDev,(UINT8*)Tptr2,sizeof(TD_STRUCT));
  }

  return;
}    


VOID
InsertQHTDToINTList(
  IN  USB_HC_DEV                      *HcDev,
  IN  QH_STRUCT                       *ptrQH,
  IN  TD_STRUCT                       *ptrFirstTD,
  IN  UINT8                           DeviceAddress,
  IN  UINT8                           EndPointAddress,
  IN  UINT8                           DataToggle,
  IN  UINTN                           DataLength,
  IN  UINTN                           PollingInterval,
  IN  VOID                            *Mapping,
  IN  UINT8                           *DataBuffer,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK CallBackFunction,
  IN  VOID                            *Context
  )
{
  EFI_STATUS        Status;
  INTERRUPT_LIST    *Node ;

  Status = gBS->AllocatePool (EfiBootServicesData,sizeof(INTERRUPT_LIST),&Node);
  if(EFI_ERROR(Status)) {
    return;
  }   
  
  //
  // Fill Node field
  //
  Node->Signature         = INTERRUPT_LIST_SIGNATURE;
  Node->DevAddr           = DeviceAddress;
  Node->EndPoint          = EndPointAddress;
  Node->ptrQH             = ptrQH;
  Node->ptrFirstTD        = ptrFirstTD;
  Node->DataToggle        = DataToggle ;
  Node->DataLen           = DataLength;
  Node->PollInterval      = PollingInterval;
  Node->Mapping           = Mapping;
  //
  // DataBuffer is allocated host memory, not mapped memory
  //
  Node->DataBuffer        = DataBuffer;
  Node->InterruptCallBack = CallBackFunction;
  Node->InterruptContext  = Context;
  
  //
  // insert the new interrupt transfer to the head of the list.
  // The interrupt transfer's monitor function scans the whole list from head
  // to tail. The new interrupt transfer MUST be added to the head of the list
  // for the sake of error recovery. 
  //
  InsertHeadList (&(HcDev->InterruptListHead),&(Node->Link));

  return;
}


EFI_STATUS
DeleteAsyncINTQHTDs(
  IN  USB_HC_DEV  *HcDev,
  IN  UINT8       DeviceAddress,
  IN  UINT8       EndPointAddress,
  OUT UINT8       *DataToggle
  ) 
{
  QH_STRUCT           *MatchQH;
  QH_STRUCT           *ptrNextQH;
  TD_STRUCT           *MatchTD;
  EFI_LIST_ENTRY      *Link;
  INTERRUPT_LIST      *MatchList;
  INTERRUPT_LIST      *ptrList;
  BOOLEAN             Found;
  
  UINT32              Result;
  UINTN               ErrTDPos;
  UINTN               ActualLen;
  
  MatchQH = NULL;
  MatchTD = NULL;
  MatchList = NULL;
  
  //
  // no interrupt transaction exists
  //
  if (IsListEmpty (&(HcDev->InterruptListHead))) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // find the correct QH-TD that need to delete
  // (by matching Device address and EndPoint number to match QH-TD )
  //

  Found = FALSE ;
  Link = &(HcDev->InterruptListHead);
  do {
    
    Link = Link->ForwardLink;
    ptrList = INTERRUPT_LIST_FROM_LINK(Link);

    if((ptrList->DevAddr == DeviceAddress) && 
        ((ptrList->EndPoint & 0x0f) == (EndPointAddress & 0x0f))) {
      MatchList = ptrList;
      
      Found = TRUE ;
      break ;
    }
    
  } while (Link->ForwardLink != &(HcDev->InterruptListHead)); 

  if(!Found) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // get current endpoint's data toggle bit and save.
  //
  ExecuteAsyncINTTDs(HcDev,MatchList,&Result,&ErrTDPos,&ActualLen) ;
  UpdateAsyncINTQHTDs(MatchList,Result,(UINT32)ErrTDPos) ;
  *DataToggle = MatchList->DataToggle;
  
  MatchTD = MatchList->ptrFirstTD;
  MatchQH = MatchList->ptrQH;
  //
  // find the first matching QH position in the FrameList
  //
  while(MatchQH) {
    
    ptrNextQH = MatchQH->ptrNextIntQH;
    
    //
    // Search all the entries
    //
    DelLinkSingleQH (HcDev,MatchQH,0,TRUE,TRUE);
        
    MatchQH = ptrNextQH;
  }  
  
  //
  // Call PciIo->Unmap() to unmap the busmaster read/write
  //
  HcDev->PciIo->Unmap (HcDev->PciIo,MatchList->Mapping);
  
  //
  // free host data buffer allocated,
  // mapped data buffer is freed by Unmap
  //
  if (MatchList->DataBuffer != NULL) {
    gBS->FreePool (MatchList->DataBuffer);
  }
  
  //
  // at last delete the TDs, to avoid problems
  //  
  DeleteQueuedTDs (HcDev,MatchTD);
    
  //
  // remove Match node from interrupt list
  //
  RemoveEntryList (&(MatchList->Link));
  gBS->FreePool (MatchList);
  return EFI_SUCCESS;
}

BOOLEAN
CheckTDsResults(
  IN  TD_STRUCT               *ptrTD,
  OUT UINT32                  *Result,
  OUT UINTN                   *ErrTDPos,
  OUT UINTN                   *ActualTransferSize
  )
{
  UINTN           Len;
  
  *Result = EFI_USB_NOERROR; 
  *ErrTDPos = 0;
  
  //
  // Init to zero.
  //  
  *ActualTransferSize = 0;
  
  while(ptrTD) {
    
    if(IsTDStatusActive(ptrTD)) {
      *Result |= EFI_USB_ERR_NOTEXECUTE;
    }

    if(IsTDStatusStalled(ptrTD)) {
      *Result |= EFI_USB_ERR_STALL;
    }

      
    if(IsTDStatusBufferError(ptrTD)) {
      *Result |= EFI_USB_ERR_BUFFER;
    } 

    if(IsTDStatusBabbleError(ptrTD)) {
      *Result |= EFI_USB_ERR_BABBLE;
    }

    if(IsTDStatusNAKReceived(ptrTD)) {
      *Result |= EFI_USB_ERR_NAK;
    } 

    if(IsTDStatusCRCTimeOutError(ptrTD)) {
      *Result |= EFI_USB_ERR_TIMEOUT ;
    } 

    if(IsTDStatusBitStuffError(ptrTD)) {
      *Result |= EFI_USB_ERR_BITSTUFF;
    }
    
    //
    // Accumulate actual transferred data length in each TD.
    //
    Len = GetTDStatusActualLength (ptrTD) % 0x800;
    *ActualTransferSize += Len;
    
    //
    // if any error encountered, stop processing the left TDs.
    //
    if(*Result) {
      return FALSE;
    }
    
    ptrTD = (TD_STRUCT*)(ptrTD->ptrNextTD);
    //
    // Record the first Error TD's position in the queue,
    // this value is zero-based.
    //
    (*ErrTDPos) ++ ;
  }
  
  return TRUE ;
}


VOID
ExecuteAsyncINTTDs(
  IN  USB_HC_DEV      *HcDev,
  IN  INTERRUPT_LIST  *ptrList,
  OUT UINT32          *Result,
  OUT UINTN           *ErrTDPos,
  OUT UINTN           *ActualLen
  ) 
{
  //
  // *ErrTDPos is zero-based value, indicating the first error TD's position 
  // in the TDs' sequence.
  // *ErrTDPos value is only valid when *Result is not equal NOERROR.
  //
  CheckTDsResults(ptrList->ptrFirstTD,Result,ErrTDPos,ActualLen) ;

  return ;
}


VOID
UpdateAsyncINTQHTDs(
  IN  INTERRUPT_LIST  *ptrList,
  IN  UINT32          Result,
  IN  UINT32          ErrTDPos
  )
{
  QH_STRUCT       *ptrFirstQH;
  QH_STRUCT       *ptrQH;
  TD_STRUCT       *ptrFirstTD;
  TD_STRUCT       *ptrTD;
  UINT8           DataToggle;
  UINT32          i;
  
  ptrFirstQH  = ptrList->ptrQH;
  ptrFirstTD  = ptrList->ptrFirstTD;
  
  DataToggle  = 0 ;

  if(Result == EFI_USB_NOERROR) {
    
    ptrTD = ptrFirstTD;
    while(ptrTD) {
      DataToggle = GetTDTokenDataToggle(ptrTD) ;
      ptrTD = ptrTD->ptrNextTD ;        
    }
    
    //
    // save current DataToggle value to interrupt list.
    // this value is used for tracing the interrupt endpoint DataToggle.
    // when this interrupt transfer is deleted, the last DataToggle is saved 
    //
    ptrList->DataToggle = DataToggle ;
    
    ptrTD = ptrFirstTD ;

    //
    // Since DataToggle bit should toggle after each success transaction,
    // the First TD's DataToggle bit will be updated to XOR of Last TD's 
    // DataToggle bit. If the First TD's DataToggle bit is not equal Last
    // TD's DataToggle bit, that means it already be the XOR of Last TD's,
    // so no update is needed.
    //
    if(DataToggle == GetTDTokenDataToggle(ptrFirstTD)) {
      ptrTD = ptrFirstTD ;
      while(ptrTD) {
        
        DataToggle ^= 1 ;
        if(DataToggle) {
          SetTDTokenDataToggle1(ptrTD);
        } else {
          SetTDTokenDataToggle0(ptrTD);
        }
        ptrTD = ptrTD->ptrNextTD ;
      }
    }
    
    //
    // restore Link Pointer of QH to First TD
    // (because QH's Link Pointer will change during TD execution)
    //
    ptrQH = ptrFirstQH;
    while(ptrQH) {
      
      LinkTDToQH(ptrQH,ptrFirstTD) ;
      ptrQH = ptrQH->ptrNextIntQH;
    }
    
    //
    // set all the TDs active
    //
    ptrTD = ptrFirstTD;
    while(ptrTD) {
      SetTDStatusActiveorInactive(ptrTD, TRUE);
      ptrTD = ptrTD->ptrNextTD ;      
    }
    
    return ;
  }

  else if(((Result & EFI_USB_ERR_NOTEXECUTE) == EFI_USB_ERR_NOTEXECUTE) 
            || ((Result & EFI_USB_ERR_NAK) == EFI_USB_ERR_NAK))
  {
    // no update
    return;
  }
  // Have Errors
  else
  { 
    ptrTD = ptrFirstTD ;
    if(ErrTDPos != 0)   // not first TD error
    {
      //
      // get the last success TD
      //
      for( i = 1 ; i < ErrTDPos ; i ++) {
        ptrTD = ptrTD->ptrNextTD ;
      }

      //
      // update Data Toggle in the interrupt list node
      //
      ptrList->DataToggle = GetTDTokenDataToggle(ptrTD);

      //
      // get the error TD
      //
      ptrTD = ptrTD->ptrNextTD ;

    } else {
      ptrList->DataToggle = GetTDTokenDataToggle(ptrTD);
    }
    //
    // do not restore the QH's vertical link pointer,
    // let the callback function do the rest of error handling.
    //
    return;
  }
}

VOID
ClearTDStatus(
  IN  TD_STRUCT *ptrTD
  )
{
  //
  // clear all the status bit in TD.
  //
  if(!ptrTD) {
    return ;
  }
  
  ptrTD->TDData.TDStatus = 0 ;
  return ;
}


VOID
ReleaseInterruptList(
  IN  USB_HC_DEV      *HcDev,
  IN  EFI_LIST_ENTRY  *ListHead
  )
{
  EFI_LIST_ENTRY  *Link;
  EFI_LIST_ENTRY  *SavedLink;
  INTERRUPT_LIST  *pNode;
  TD_STRUCT       *ptrTD;
  TD_STRUCT       *ptrNextTD;
  QH_STRUCT       *ptrQH; 
  QH_STRUCT       *SavedQH;
  
  if (ListHead == NULL) {
    return;
  }
  
  Link = ListHead;
  
  //
  // Free all the resources in the interrupt list
  //
  SavedLink = Link->ForwardLink;
  while (!IsListEmpty (ListHead)) {
    
    Link = SavedLink;
    
    SavedLink = Link->ForwardLink;
    
    pNode = INTERRUPT_LIST_FROM_LINK(Link);
    
    RemoveEntryList (&pNode->Link);
    
    SavedQH = pNode->ptrQH;
    for (ptrQH = SavedQH;ptrQH != NULL;ptrQH = SavedQH) {
      SavedQH = ptrQH->ptrNextIntQH;
      UhciFreePool(HcDev,(UINT8*)ptrQH,sizeof(QH_STRUCT));
    }

    ptrTD = pNode->ptrFirstTD ;
    while (!ptrTD) {
      
      ptrNextTD = ptrTD->ptrNextTD ;
      UhciFreePool(HcDev,(UINT8*)ptrTD,sizeof(TD_STRUCT));
      ptrTD = ptrNextTD ;     
    }
    
    gBS->FreePool(pNode);
  }
}


EFI_STATUS
InitializeMemoryManagement (
  USB_HC_DEV           *HcDev
  )
{
  MEMORY_MANAGE_HEADER    *MemoryHeader;
  EFI_STATUS              Status;
  UINTN                   MemPages;
  
  MemPages = NORMAL_MEMORY_BLOCK_UNIT_IN_PAGES;
  Status = CreateMemoryBlock (HcDev,&MemoryHeader,MemPages);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  HcDev->MemoryHeader = MemoryHeader;  

  return EFI_SUCCESS;
}

EFI_STATUS
CreateMemoryBlock (
  USB_HC_DEV            *HcDev,
  MEMORY_MANAGE_HEADER  **MemoryHeader,
  UINTN                 MemoryBlockSizeInPages
  )
/*
  Use PciIo->AllocateBuffer to allocate common buffer for the memory block,
  and use PciIo->Map to map the common buffer for Bus Master Read/Write.
*/
{
  EFI_STATUS              Status;
  VOID                    *CommonBuffer;
  EFI_PHYSICAL_ADDRESS    MappedAddress;
  UINTN                   MemoryBlockSizeInBytes;
  void                    *Mapping;
  
  //
  // Allocate memory for MemoryHeader
  //
  Status = gBS->AllocatePool (EfiBootServicesData, sizeof(MEMORY_MANAGE_HEADER),MemoryHeader);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EfiZeroMem (*MemoryHeader,sizeof(MEMORY_MANAGE_HEADER));
  
  (*MemoryHeader)->Next = NULL;
  
  //
  // set Memory block size
  //
  (*MemoryHeader)->MemoryBlockSizeInBytes = EFI_PAGES_TO_SIZE (MemoryBlockSizeInPages);
  
  //
  // each bit in Bit Array will manage 32 bytes memory in memory block
  //
  (*MemoryHeader)->BitArraySizeInBytes = ((*MemoryHeader)->MemoryBlockSizeInBytes / 32) / 8;
  
  //
  // Allocate memory for BitArray
  //
  Status = gBS->AllocatePool (EfiBootServicesData, 
                              (*MemoryHeader)->BitArraySizeInBytes,
                              &((*MemoryHeader)->BitArrayPtr)
                              );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (*MemoryHeader);
    return Status;
  }
  EfiZeroMem ((*MemoryHeader)->BitArrayPtr,(*MemoryHeader)->BitArraySizeInBytes);
  
  //
  // Memory Block uses MemoryBlockSizeInPages pages,
  // and it is allocated as common buffer use.
  //  
  Status = HcDev->PciIo->AllocateBuffer (
                             HcDev->PciIo,
                             AllocateAnyPages,
                             EfiBootServicesData,
                             MemoryBlockSizeInPages,
                             &CommonBuffer,
                             0
                             );
  if (EFI_ERROR(Status)) {
    gBS->FreePool ((*MemoryHeader)->BitArrayPtr);
    gBS->FreePool (*MemoryHeader);
    return Status;
  }
  
  MemoryBlockSizeInBytes = EFI_PAGES_TO_SIZE(MemoryBlockSizeInPages);
  Status = HcDev->PciIo->Map (HcDev->PciIo,
                              EfiPciIoOperationBusMasterCommonBuffer,
                              CommonBuffer,
                              &MemoryBlockSizeInBytes,
                              &MappedAddress,
                              &Mapping
                              );
  //
  // if returned Mapped size is less than the size we request,do not support.
  //
  if (EFI_ERROR(Status) || (MemoryBlockSizeInBytes != EFI_PAGES_TO_SIZE(MemoryBlockSizeInPages))) {
    HcDev->PciIo->FreeBuffer (HcDev->PciIo,MemoryBlockSizeInPages,CommonBuffer);
    gBS->FreePool ((*MemoryHeader)->BitArrayPtr);
    gBS->FreePool (*MemoryHeader);
    return EFI_UNSUPPORTED;
  }
  //
  // Set Memory block initial address
  //
  (*MemoryHeader)->MemoryBlockPtr = (UINT8*)((UINTN)MappedAddress);
  (*MemoryHeader)->Mapping = Mapping;  
  
  EfiZeroMem (
        (*MemoryHeader)->MemoryBlockPtr,
        EFI_PAGES_TO_SIZE(MemoryBlockSizeInPages)
        ); 
  
  return EFI_SUCCESS;
}

EFI_STATUS
FreeMemoryHeader (
  USB_HC_DEV            *HcDev,
  MEMORY_MANAGE_HEADER  *MemoryHeader
  )
{
  if ((MemoryHeader == NULL) || (HcDev == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // unmap the common buffer used by the memory block
  //
  HcDev->PciIo->Unmap (HcDev->PciIo,MemoryHeader->Mapping);
  
  //
  // free common buffer
  //
  HcDev->PciIo->FreeBuffer (
                      HcDev->PciIo,
                      EFI_SIZE_TO_PAGES (MemoryHeader->MemoryBlockSizeInBytes),
                      MemoryHeader->MemoryBlockPtr
                      );
  //
  // free bit array
  //
  gBS->FreePool (MemoryHeader->BitArrayPtr);
  //
  // free memory header
  //
  gBS->FreePool (MemoryHeader);
  
  return EFI_SUCCESS;
}

EFI_STATUS
UhciAllocatePool (
  USB_HC_DEV      *HcDev,
  UINT8           **Pool,
  UINTN           AllocSize
  )
{
  MEMORY_MANAGE_HEADER    *MemoryHeader;
  MEMORY_MANAGE_HEADER    *TempHeaderPtr;
  MEMORY_MANAGE_HEADER    *NewMemoryHeader;
  UINTN                   RealAllocSize;
  UINTN                   MemoryBlockSizeInPages;
  EFI_STATUS              Status;
  
  *Pool = NULL;
  
  MemoryHeader = HcDev->MemoryHeader;

  //
  // allocate unit is 32 bytes (align on 32 byte)
  //
  if (AllocSize % 32) {
    RealAllocSize = (AllocSize / 32 + 1) * 32;
  } else {
    RealAllocSize = AllocSize;
  }
  
  //
  // There may be linked MemoryHeaders.
  // To allocate a free pool in Memory blocks,
  // must search in the MemoryHeader link list
  // until enough free pool is found.
  //
  Status = EFI_NOT_FOUND;
  for (TempHeaderPtr = MemoryHeader;TempHeaderPtr != NULL;
      TempHeaderPtr = TempHeaderPtr->Next) {
    
    Status = AllocMemInMemoryBlock (
                        TempHeaderPtr,
                        Pool,
                        RealAllocSize / 32
                        );
    if (!EFI_ERROR(Status)) {
      return EFI_SUCCESS;
    }
  }
  
  //
  // There is no enough memory,
  // Create a new Memory Block
  //
  
  //
  // if pool size is larger than NORMAL_MEMORY_BLOCK_UNIT_IN_PAGES,
  // just allocate a large enough memory block.
  //
  if (RealAllocSize > EFI_PAGES_TO_SIZE (NORMAL_MEMORY_BLOCK_UNIT_IN_PAGES)) {
    MemoryBlockSizeInPages = EFI_SIZE_TO_PAGES (RealAllocSize) + 1;
  } else {
    MemoryBlockSizeInPages = NORMAL_MEMORY_BLOCK_UNIT_IN_PAGES;
  }
  
  Status = CreateMemoryBlock (HcDev,&NewMemoryHeader,MemoryBlockSizeInPages);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  //
  // Link the new Memory Block to the Memory Header list
  //
  InsertMemoryHeaderToList (MemoryHeader,NewMemoryHeader);
  
  Status = AllocMemInMemoryBlock (NewMemoryHeader,
                                  Pool,
                                  RealAllocSize / 32
                                  );
  return Status;
}

VOID
UhciFreePool (
  USB_HC_DEV      *HcDev,
  UINT8           *Pool,
  UINTN           AllocSize
  )
{
  MEMORY_MANAGE_HEADER    *MemoryHeader;
  MEMORY_MANAGE_HEADER    *TempHeaderPtr;
  UINTN                   StartBytePos,i;
  UINT8                   StartBitPos,j;
  UINTN                   Count;
  UINTN                   RealAllocSize;
  
  MemoryHeader = HcDev->MemoryHeader;
  
  //
  // allocate unit is 32 byte (align on 32 byte)
  //
  if (AllocSize % 32) {
    RealAllocSize = (AllocSize / 32 + 1) * 32;
  } else {
    RealAllocSize = AllocSize;
  }
  
  //
  // scan the memory header linked list for
  // the asigned memory to free.
  //
  for (TempHeaderPtr = MemoryHeader;TempHeaderPtr != NULL;
      TempHeaderPtr = TempHeaderPtr->Next) {
    
    if ((Pool >= TempHeaderPtr->MemoryBlockPtr) && 
        ((Pool + RealAllocSize) <= (TempHeaderPtr->MemoryBlockPtr + 
                                    TempHeaderPtr->MemoryBlockSizeInBytes))) {
      
      //
      // Pool is in the Memory Block area, 
      // find the start byte and bit in the bit array      
      //      
      StartBytePos = ((Pool - TempHeaderPtr->MemoryBlockPtr) / 32) / 8;
      StartBitPos  = (UINT8)(((Pool - TempHeaderPtr->MemoryBlockPtr) / 32) % 8);
      
      //
      // reset associated bits in bit arry
      //
      for (i = StartBytePos,j = StartBitPos,Count = 0;
         Count < (RealAllocSize / 32); Count ++) {
    
        TempHeaderPtr->BitArrayPtr[i] ^= (UINT8)(bit(j));
        j ++;
        if (j == 8) {
          i += 1;
          j = 0;
        }
      }
      //
      // break the loop
      //
      break;
    }
  }
  
  //
  // Release emptied memory blocks (only if the memory block is not 
  // the first one in the memory header list
  //
  for (TempHeaderPtr = MemoryHeader->Next;TempHeaderPtr != NULL;) {
    
    if (IsMemoryBlockEmptied (TempHeaderPtr)) {
      
      DelinkMemoryBlock (MemoryHeader,TempHeaderPtr);
      //
      // when the TempHeaderPtr is freed in FreeMemoryHeader(),
      // the TempHeaderPtr is pointing to nonsense content.
      //
      FreeMemoryHeader(HcDev,TempHeaderPtr);
      //
      // reset the TempHeaderPtr, continue search for
      // another empty memory block.
      //
      TempHeaderPtr = MemoryHeader->Next;
      continue;
    }
    
    TempHeaderPtr = TempHeaderPtr->Next;
  }
}

VOID
InsertMemoryHeaderToList (
  MEMORY_MANAGE_HEADER  *MemoryHeader,
  MEMORY_MANAGE_HEADER  *NewMemoryHeader
  )
{
  MEMORY_MANAGE_HEADER    *TempHeaderPtr;
  
  for (TempHeaderPtr = MemoryHeader; TempHeaderPtr != NULL;
        TempHeaderPtr = TempHeaderPtr->Next) {
    if (TempHeaderPtr->Next == NULL) {
      TempHeaderPtr->Next = NewMemoryHeader;
      break;
    }
  }
}

EFI_STATUS
AllocMemInMemoryBlock (
  MEMORY_MANAGE_HEADER  *MemoryHeader,
  VOID                  **Pool,
  UINTN                 NumberOfMemoryUnit
  )
{
  UINTN                 TempBytePos;
  UINTN                 FoundBytePos;
  UINT8                 i,FoundBitPos;
  UINT8                 ByteValue;
  UINT8                 BitValue;
  UINTN                 NumberOfZeros;
  UINTN                 Count;
  
  FoundBytePos  = 0;
  FoundBitPos   = 0;
  ByteValue     = MemoryHeader->BitArrayPtr[0];
  NumberOfZeros = 0;
  i = 0;
  
  for (TempBytePos = 0;TempBytePos < MemoryHeader->BitArraySizeInBytes;) {  
    
    //
    // Pop out BitValue from a byte in TempBytePos.
    //
    BitValue = (UINT8)(ByteValue % 2);    
    
    if (BitValue == 0) {
    //
    // Found a free bit, the NumberOfZeros only record the number 
    // of those consecutive zeros
    //
      NumberOfZeros ++;
      //
      // Found enough consecutive free space, break the loop
      //
      if (NumberOfZeros >= NumberOfMemoryUnit) {                
        break;
      }
    } else {
    //
    // Encountering a '1', meant the bit is ocupied.
    //
      if (NumberOfZeros >= NumberOfMemoryUnit) {
      //
      // Found enough consecutive free space,break the loop
      //
        break;
      } else {
        //
        // the NumberOfZeros only record the number of those consecutive zeros,
        // so reset the NumberOfZeros to 0 when encountering '1' before finding
        // enough consecutive '0's
        //
        NumberOfZeros = 0;
        //
        // reset the (FoundBytePos,FoundBitPos) to the position of '1'
        //
        FoundBytePos = TempBytePos;
        FoundBitPos = i;
      }
    }
    
    //
    // right shift the byte
    //
    ByteValue /= 2;
    
    //
    // step forward a bit
    //
    i ++;
    if (i == 8) {
      //
      // step forward a byte, getting the byte value,
      // and reset the bit pos.
      //
      TempBytePos += 1;
      ByteValue = MemoryHeader->BitArrayPtr[TempBytePos];
      i = 0;
    }
  }
  
  if (NumberOfZeros < NumberOfMemoryUnit) {
    return EFI_NOT_FOUND;
  }
  
  //
  // Found enough free space.
  //
  
  //
  // The values recorded in (FoundBytePos,FoundBitPos) have two conditions:
  //  1)(FoundBytePos,FoundBitPos) record the position 
  //    of the last '1' before the consecutive '0's, it must
  //    be adjusted to the start position of the consecutive '0's.
  //  2)the start address of the consecutive '0's is just the start of 
  //    the bitarray. so no need to adjust the values of 
  //    (FoundBytePos,FoundBitPos).
  //
  if ((MemoryHeader->BitArrayPtr[0] & bit (0)) != 0) {
    FoundBitPos += 1;    
  }
  
  //
  // Have the (FoundBytePos,FoundBitPos) make sense.
  //
  if (FoundBitPos > 7) {
    FoundBytePos += 1;
    FoundBitPos -= 8;
  }
  
  //
  // Set the memory as allocated
  //
  for (TempBytePos = FoundBytePos,i = FoundBitPos,Count = 0;
         Count < NumberOfMemoryUnit; Count ++) {
    
    MemoryHeader->BitArrayPtr[TempBytePos] |= bit(i);
    i ++;
    if (i == 8) {
      TempBytePos += 1;
      i = 0;
    }
  }
  
  *Pool = MemoryHeader->MemoryBlockPtr + (FoundBytePos * 8 + FoundBitPos) * 32;
  
  return EFI_SUCCESS;
}

BOOLEAN
IsMemoryBlockEmptied (
  MEMORY_MANAGE_HEADER  *MemoryHeaderPtr
  )
{
  UINTN     Index;
  
  for (Index = 0; Index < MemoryHeaderPtr->BitArraySizeInBytes;Index ++) {
    if (MemoryHeaderPtr->BitArrayPtr[Index] != 0) {
      return FALSE;
    }
  }
  
  return TRUE;
}

VOID
DelinkMemoryBlock (  
  MEMORY_MANAGE_HEADER    *FirstMemoryHeader,
  MEMORY_MANAGE_HEADER    *NeedFreeMemoryHeader
  )
{
  MEMORY_MANAGE_HEADER    *TempHeaderPtr;
  
  if ((FirstMemoryHeader == NULL) || (NeedFreeMemoryHeader == NULL)) {
    return;
  }
  for (TempHeaderPtr = FirstMemoryHeader; TempHeaderPtr != NULL;
        TempHeaderPtr = TempHeaderPtr->Next) {
    
    if (TempHeaderPtr->Next == NeedFreeMemoryHeader) {
      //
      // Link the before and after
      //
      TempHeaderPtr->Next = NeedFreeMemoryHeader->Next;
      break;
    }
  }
}

EFI_STATUS
DelMemoryManagement (
  USB_HC_DEV      *HcDev
  )
{
  MEMORY_MANAGE_HEADER    *TempHeaderPtr;
  
  for (TempHeaderPtr = HcDev->MemoryHeader->Next;TempHeaderPtr != NULL; ) {
            
    DelinkMemoryBlock (HcDev->MemoryHeader,TempHeaderPtr);
    //
    // when the TempHeaderPtr is freed in FreeMemoryHeader(),
    // the TempHeaderPtr is pointing to nonsense content.
    //
    FreeMemoryHeader(HcDev,TempHeaderPtr);
    //
    // reset the TempHeaderPtr,continue free another memory block.
    //
    TempHeaderPtr = HcDev->MemoryHeader->Next;
  }
  
  FreeMemoryHeader(HcDev,HcDev->MemoryHeader);
  
  return EFI_SUCCESS;
}

EFI_STATUS
GenPseudoTD (
  USB_HC_DEV      *HcDev,
  UINT8           DeviceAddress,
  UINT8           Endpoint,
  UINT8           PktID,
  UINT8           DataToggle,
  BOOLEAN         bSlowDevice,
  TD_STRUCT       **ppTD  
  )
{
  EFI_STATUS      Status;
  
  Status = GenDataTD(HcDev,DeviceAddress,Endpoint, NULL, 
              0, PktID, DataToggle, bSlowDevice, ppTD);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  //
  // Pseudo TD points to itself
  //
  SetTDLinkPtrValidorInvalid(*ppTD,TRUE);
  SetTDLinkPtr(*ppTD,*ppTD);
  //
  // Link pointer pointing to TD struct
  //
  SetTDLinkPtrQHorTDSelect(*ppTD,FALSE);
  
  //
  // Bread first fashion
  //
  SetTDLinkPtrDepthorBreadth(*ppTD, FALSE);
  
  //
  // Set the Pseudo TD inactive
  //
  SetTDStatusActiveorInactive(*ppTD,FALSE); 
  
  return EFI_SUCCESS; 
}

VOID
MakeQHLoop (
  QH_STRUCT   *PseudoQH,
  QH_STRUCT   *ptrQH
  )
{
  //
  // ptrQH next to the PseudoQH
  //
  PseudoQH->ptrNext = ptrQH;
  SetQHHorizontalQHorTDSelect(PseudoQH,TRUE);
  SetQHHorizontalLinkPtr(PseudoQH,ptrQH);
  SetQHHorizontalValidorInvalid(PseudoQH,TRUE);
  
  //
  // PseudoQH also next to ptrQH
  //
  ptrQH->ptrNext = PseudoQH;
  SetQHHorizontalQHorTDSelect(ptrQH,TRUE);
  SetQHHorizontalLinkPtr(ptrQH,PseudoQH);
  SetQHHorizontalValidorInvalid(ptrQH,TRUE);
  
  //
  // set LoopPtr to indicate QHs' positions in the Loop
  // the one has LoopPtr set is the last QH in the Loop.
  // the one pointed by the LoopPtr is the first QH in the Loop.
  //
  PseudoQH->LoopPtr = NULL;
  ptrQH->LoopPtr = PseudoQH;
  
  return;
}

BOOLEAN
IsQHsLooped (
  QH_STRUCT   *CurrentQH,
  QH_STRUCT   **LastLoopQH
  )
/*
  If there is Looped QHs within the QH list that the CurrentQH resides
*/
{
  *LastLoopQH = NULL;
  
  while (CurrentQH) {
    
    if (CurrentQH->LoopPtr != NULL) {
      *LastLoopQH = CurrentQH;
      return TRUE;
    }
    
    //
    // retrieve nex qh pointer
    //
    CurrentQH = GetQHHorizontalLinkPtr (CurrentQH);
  }
  
  return FALSE;
}

VOID
CleanUsbTransactions (
  USB_HC_DEV    *HcDev
  )
{
  //
  // only asynchronous interrupt transfers are always alive on the bus
  //
  ReleaseInterruptList (HcDev,&(HcDev->InterruptListHead));
}

VOID
TurnOffUSBEmulation (
  IN EFI_PCI_IO_PROTOCOL  *PciIo  
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/
{
  UINT16  Command;
  
  //
  // Disable USB Emulation
  //
  Command = 0;
  PciIo->Pci.Write (
          PciIo,
          EfiPciIoWidthUint16,
          USB_EMULATION,                 
          1,
          &Command
         );          

  return;
}