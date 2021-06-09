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

    Uhci.h
    
Abstract: 
    

Revision History
--*/

#ifndef _UHCI_H
#define _UHCI_H

/*
 * Universal Host Controller Interface data structures and defines
 */
#include "Efi.h"
#include "EfiDriverLib.h" 

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(PciIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (UsbHostController)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define EFI_D_UHCI   EFI_D_INFO

#define STALL_1_SECOND          1000000 // stall 1 second
#define STALL_1_MILLI_SECOND    1000    // stall 1 millisecond

//
// One memory block uses 1 page (common buffer for QH,TD use.)
//
#define NORMAL_MEMORY_BLOCK_UNIT_IN_PAGES   1

#define EFI_PAGES_TO_SIZE(a)   ( (a) << EFI_PAGE_SHIFT)

#define bit(a)   1 << (a)

//////////////////////////////////////////////////////////////////////////
//
//          Universal Host Controller Registers Definitions
//
//////////////////////////////////////////////////////////////////////////

extern  UINT16  USBBaseAddr;
/* Command register */
#define   USBCMD        0   /* Command Register Offset 00-01h */
#define   USBCMD_RS       bit(0)  /* Run/Stop */
#define   USBCMD_HCRESET  bit(1)  /* Host reset */
#define   USBCMD_GRESET   bit(2)  /* Global reset */
#define   USBCMD_EGSM     bit(3)  /* Global Suspend Mode */
#define   USBCMD_FGR      bit(4)  /* Force Global Resume */
#define   USBCMD_SWDBG    bit(5)  /* SW Debug mode */
#define   USBCMD_CF       bit(6)  /* Config Flag (sw only) */
#define   USBCMD_MAXP     bit(7)  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#define   USBSTS        2   /* Status Register Offset 02-03h */
#define   USBSTS_USBINT   bit(0)  /* Interrupt due to IOC */
#define   USBSTS_ERROR    bit(1)  /* Interrupt due to error */
#define   USBSTS_RD       bit(2)  /* Resume Detect */
#define   USBSTS_HSE      bit(3)  /* Host System Error*/
#define   USBSTS_HCPE     bit(4)  /* Host Controller Process Error*/
#define   USBSTS_HCH      bit(5)  /* HC Halted */

/* Interrupt enable register */
#define   USBINTR       4   /* Interrupt Enable Register 04-05h */
#define   USBINTR_TIMEOUT bit(0)  /* Timeout/CRC error enable */
#define   USBINTR_RESUME  bit(1)  /* Resume interrupt enable */
#define   USBINTR_IOC     bit(2)  /* Interrupt On Complete enable */
#define   USBINTR_SP      bit(3) /* Short packet interrupt enable */

/* Frame Number Register Offset 06-08h */
#define   USBFRNUM      6

/* Frame List Base Address Register Offset 08-0Bh */
#define   USBFLBASEADD  8   

/* Start of Frame Modify Register Offset 0Ch */
#define   USBSOF      0x0c

/* USB port status and control registers */
#define   USBPORTSC1    0x10  /*Port 1 offset 10-11h */
#define   USBPORTSC2    0x12  /*Port 2 offset 12-13h */

#define   USBPORTSC_CCS   bit(0)  /* Current Connect Status*/
#define   USBPORTSC_CSC   bit(1)  /* Connect Status Change */
#define   USBPORTSC_PED   bit(2)  /* Port Enable / Disable */
#define   USBPORTSC_PEDC  bit(3)  /* Port Enable / Disable Change */
#define   USBPORTSC_LSL   bit(4)  /* Line Status Low bit*/
#define   USBPORTSC_LSH   bit(5)  /* Line Status High bit*/
#define   USBPORTSC_RD    bit(6)  /* Resume Detect */
#define   USBPORTSC_LSDA  bit(8)  /* Low Speed Device Attached */
#define   USBPORTSC_PR    bit(9)  /* Port Reset */
#define   USBPORTSC_SUSP  bit(12) /* Suspend */

/* PCI Configuration Registers for USB */
// Class Code Register offset
#define CLASSC                      0x09
// USB IO Space Base Address Register offset
#define USBBASE                     0x20

// USB legacy Support
#define USB_EMULATION               0xc0

//
// USB Base Class Code,Sub-Class Code and Programming Interface.
//
#define PCI_CLASSC_BASE_CLASS_SERIAL        0x0c
#define PCI_CLASSC_SUBCLASS_SERIAL_USB      0x03
#define PCI_CLASSC_PI_UHCI                  0x00

// USB Class Code structure
typedef struct
{
  UINT8    PI;
  UINT8    SubClassCode;
  UINT8    BaseCode;
} USB_CLASSC;

#define SETUP_PACKET_ID  0x2D
#define INPUT_PACKET_ID  0x69
#define OUTPUT_PACKET_ID 0xE1
#define ERROR_PACKET_ID  0x55


//////////////////////////////////////////////////////////////////////////
//
//          USB Transfer Mechanism Data Structures
//
//////////////////////////////////////////////////////////////////////////

#pragma pack(1)

typedef struct
{
  UINT32    QHHorizontalTerminate : 1;
  UINT32    QHHorizontalQSelect : 1;
  UINT32    QHHorizontalRsvd : 2;
  UINT32    QHHorizontalPtr : 28;
  UINT32    QHVerticalTerminate : 1;
  UINT32    QHVerticalQSelect : 1;
  UINT32    QHVerticalRsvd : 2;
  UINT32    QHVerticalPtr : 28;
} QUEUE_HEAD;

typedef struct 
{
  UINT32    TDLinkPtrTerminate : 1;
  UINT32    TDLinkPtrQSelect : 1;
  UINT32    TDLinkPtrDepthSelect :1;
  UINT32    TDLinkPtrRsvd : 1;
  UINT32    TDLinkPtr : 28;
  UINT32    TDStatusActualLength :11;
  UINT32    TDStatusRsvd : 5;
  UINT32    TDStatus : 8;
  UINT32    TDStatusIOC : 1;
  UINT32    TDStatusIOS : 1;
  UINT32    TDStatusLS : 1;
  UINT32    TDStatusErr : 2;
  UINT32    TDStatusSPD : 1;
  UINT32    TDStatusRsvd2 : 2;
  UINT32    TDTokenPID : 8;
  UINT32    TDTokenDevAddr : 7;
  UINT32    TDTokenEndPt : 4;
  UINT32    TDTokenDataToggle : 1;
  UINT32    TDTokenRsvd : 1;
  UINT32    TDTokenMaxLen : 11;
  UINT32    TDBufferPtr;
} TD;

#pragma pack()

typedef struct
{
  QUEUE_HEAD  QH;
  void        *ptrNext;
  void        *ptrDown;
  void        *ptrNextIntQH;    // for interrupt transfer's special use
  void        *LoopPtr;
} QH_STRUCT;

typedef struct
{
  TD      TDData;
  UINT8   *pTDBuffer;
  void    *ptrNextTD;
  void    *ptrNextQH;
  UINT16  TDBufferLength;
  UINT16  reserved;
} TD_STRUCT;

//////////////////////////////////////////////////////////////////////////
//
//          Universal Host Controller Device Data Structure
//
//////////////////////////////////////////////////////////////////////////
#define USB_HC_DEV_FROM_THIS(a) CR(a, USB_HC_DEV, UsbHc, USB_HC_DEV_SIGNATURE)

#define   USB_HC_DEV_SIGNATURE    EFI_SIGNATURE_32('u','h','c','i')
#define   INTERRUPT_LIST_SIGNATURE  EFI_SIGNATURE_32('i','n','t','s')
typedef struct
{
  UINTN                           Signature;
  
  EFI_LIST_ENTRY                  Link ;
  UINT8                           DevAddr ;
  UINT8                           EndPoint ;
  UINT8                           DataToggle ;
  UINT8                           Reserved[5];
  TD_STRUCT                       *ptrFirstTD ;
  QH_STRUCT                       *ptrQH ;
  UINTN                           DataLen ;
  UINTN                           PollInterval;  
  VOID                            *Mapping;
  UINT8                           *DataBuffer; // allocated host memory, not mapped memory
  EFI_ASYNC_USB_TRANSFER_CALLBACK InterruptCallBack ;
  VOID                            *InterruptContext;
}INTERRUPT_LIST ;

#define INTERRUPT_LIST_FROM_LINK(a) CR(a, INTERRUPT_LIST, Link, INTERRUPT_LIST_SIGNATURE)

typedef struct
{
  UINT32            FrameListPtrTerminate:1 ;
  UINT32            FrameListPtrQSelect:1 ;
  UINT32            FrameListRsvd:2 ;
  UINT32            FrameListPtr:28 ;

} FRAMELIST_ENTRY;

typedef struct _MEMORY_MANAGE_HEADER {
  UINT8                         *BitArrayPtr;
  UINTN                         BitArraySizeInBytes;
  UINT8                         *MemoryBlockPtr;
  UINTN                         MemoryBlockSizeInBytes;
  void                          *Mapping;
  struct _MEMORY_MANAGE_HEADER  *Next;
} MEMORY_MANAGE_HEADER;

typedef struct
{
  UINTN                       Signature ;
  EFI_USB_HC_PROTOCOL         UsbHc ;
  EFI_PCI_IO_PROTOCOL         *PciIo;

  //
  // local data
  //
  EFI_LIST_ENTRY              InterruptListHead;
  FRAMELIST_ENTRY             *FrameListEntry;
  VOID                        *FrameListMapping;
  MEMORY_MANAGE_HEADER        *MemoryHeader;
  EFI_EVENT                   InterruptTransTimer;
  EFI_UNICODE_STRING_TABLE    *ControllerNameTable;

} USB_HC_DEV ;


extern EFI_DRIVER_BINDING_PROTOCOL gUhciDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gUhciComponentName;

VOID  
WriteUHCCommandReg(
  IN  EFI_PCI_IO_PROTOCOL *PciIo,
  IN  UINT32              CmdAddrOffset,
  IN  UINT16              UsbCmd
  ) ;

//STATIC
UINT16 
ReadUHCCommandReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                CmdAddrOffset
  );

//STATIC
VOID  
WriteUHCStatusReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusAddrOffset,
  IN  UINT16                UsbSts
);


//STATIC
UINT16 
ReadUHCStatusReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusAddrOffset
);


VOID
ClearStatusReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusAddrOffset
);

VOID
WriteUHCIntReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                InterruptAddrOffset,
  IN  UINT16                UsbInt
  );

UINT16 
ReadUHCIntReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                InterruptAddrOffset
  );

VOID  
WriteUHCFrameNumberReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FrameNumAddrOffset,
  IN  UINT16                UsbFrameNum
  );

UINT16 
ReadUHCFrameNumberReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FrameNumAddrOffset
  );

VOID  
WriteUHCFrameListBaseReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FlBaseAddrOffset,
  IN  UINT32                UsbFrameListBaseAddr
  );

UINT32 
ReadUHCFrameListBaseReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FlBaseAddrOffset
  );

UINT16 
ReadRootPortReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortAddrOffset
  );


//STATIC
VOID 
WriteRootPortReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                PortAddrOffset,
  IN  UINT16                ControlBits
  ) ;

EFI_STATUS
WaitForUHCHalt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN UINT32                 StatusRegAddr,
  IN UINTN                  Timeout
  ) ;


BOOLEAN
DetectUHCInterrupt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  ) ;

BOOLEAN 
IsUSBInterrupt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  ) ;

BOOLEAN 
IsUSBErrInterrupt(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  ) ;

BOOLEAN 
IsHostSysErr(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  ) ;

BOOLEAN 
IsHCProcessErr(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  ) ;

BOOLEAN 
IsHCHalted(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                StatusRegAddr
  ) ;


//
// This routine programs the USB frame number register. We assume that the
// HC schedule execution is stopped.
//
VOID
SetFrameNumberReg(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FRNUMAddr,
  IN  UINT16                Index
  ) ;


UINT16 
GetCurrentFrameNumber(
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT32                FRNUMAddr
  ) ;

VOID 
SetFrameListBaseAddress(
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UINT32                    FLBASEADDRReg,
  UINT32                    Addr
  ) ;

UINT32 
GetFrameListBaseAddress(
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UINT32                    FLBAddr
  ) ;

EFI_STATUS
CreateFrameList (
  USB_HC_DEV                *HcDev,
  UINT32                    FLBASEADDRReg
  );

EFI_STATUS
FreeFrameListEntry(
  USB_HC_DEV        *UhcDev
  );
    
VOID
InitFrameList (
  USB_HC_DEV    *HcDev
  );  


EFI_STATUS
AllocateQHStruct(
  IN  USB_HC_DEV          *HcDev,
  OUT QH_STRUCT           **ppQHStruct
  ) ;

VOID
InitQH(
  IN  QH_STRUCT *ptrQH
  ) ;


EFI_STATUS
CreateQH(
  IN  USB_HC_DEV          *HcDev,
  OUT QH_STRUCT           **pptrQH
  ) ;

VOID
SetQHHorizontalLinkPtr(
  IN QH_STRUCT  *ptrQH,
  IN VOID       *ptrNext
  ) ;

VOID*
GetQHHorizontalLinkPtr(
  IN QH_STRUCT  *ptrQH
  ) ;

VOID
SetQHHorizontalQHorTDSelect(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    bQH
  ) ;


VOID
SetQHHorizontalValidorInvalid(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    bValid
  ) ;

VOID
SetQHVerticalLinkPtr(
  IN QH_STRUCT  *ptrQH,
  IN VOID     *ptrNext
  ) ; 

VOID*
GetQHVerticalLinkPtr(
  IN QH_STRUCT  *ptrQH
  ) ;

VOID
SetQHVerticalQHorTDSelect(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    bQH
  ) ;

BOOLEAN
IsQHHorizontalQHSelect(
  IN QH_STRUCT  *ptrQH
  ) ;

VOID
SetQHVerticalValidorInvalid(
  IN QH_STRUCT  *ptrQH,
  IN BOOLEAN    bValid
  ) ;

BOOLEAN
GetQHVerticalValidorInvalid(
  IN QH_STRUCT  *ptrQH
  ) ;


EFI_STATUS
AllocateTDStruct(
  IN  USB_HC_DEV          *HcDev,
  OUT TD_STRUCT           **ppTDStruct
  ) ;

VOID
InitTD(
  IN  TD_STRUCT *ptrTD
  ) ;

EFI_STATUS
CreateTD(
  IN  USB_HC_DEV          *HcDev,
  OUT TD_STRUCT           **pptrTD
  ) ;

EFI_STATUS
GenSetupStageTD(
  IN  USB_HC_DEV          *HcDev,
  IN  UINT8               DevAddr,
  IN  UINT8               Endpoint,
  IN  BOOLEAN             bSlow,
  IN  UINT8               *pDevReq,
  IN  UINT8               RequestLen,
  OUT TD_STRUCT           **ppTD
) ;
  
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
  );


EFI_STATUS
CreateStatusTD(
  IN  USB_HC_DEV          *HcDev,
  IN  UINT8               DevAddr,
  IN  UINT8               Endpoint,
  IN  UINT8               PktID,
  IN  BOOLEAN             bSlow,
  OUT TD_STRUCT           **ppTD
  ) ;


VOID
SetTDLinkPtrValidorInvalid(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bValid
  ) ;

VOID
SetTDLinkPtrQHorTDSelect(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bQH
  ) ;

VOID
SetTDLinkPtrDepthorBreadth(
  IN  TD_STRUCT   *ptrTDStruct,
  IN  BOOLEAN     bDepth
  ) ;

VOID
SetTDLinkPtr(
  IN  TD_STRUCT *ptrTDStruct,
  IN  VOID      *ptrNext
  ) ;

VOID*
GetTDLinkPtr(
  IN  TD_STRUCT   *ptrTDStruct
  ) ;

VOID
EnableorDisableTDShortPacket (
  IN  TD_STRUCT   *ptrTDStruct,
  IN  BOOLEAN     bEnable
  ) ;

VOID
SetTDControlErrorCounter(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINT8     nMaxErrors
  ) ;


VOID
SetTDLoworFullSpeedDevice (
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bLowSpeedDevice
  ) ;

VOID
SetTDControlIsochronousorNot(
  IN  TD_STRUCT   *ptrTDStruct,
  IN  BOOLEAN     bIsochronous
  ) ;

VOID
SetorClearTDControlIOC(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bSet
  ) ;

VOID
SetTDStatusActiveorInactive(
  IN  TD_STRUCT *ptrTDStruct,
  IN  BOOLEAN   bActive
  ) ;

UINT16
SetTDTokenMaxLength(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINT16    nMaxLen
  ) ;

VOID
SetTDTokenDataToggle1(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

VOID
SetTDTokenDataToggle0(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINT8
GetTDTokenDataToggle(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

VOID
SetTDTokenEndPoint(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINTN     nEndPoint
  ) ;

VOID
SetTDTokenDeviceAddress(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINTN     nDevAddr
  ) ;

VOID
SetTDTokenPacketID(
  IN  TD_STRUCT *ptrTDStruct,
  IN  UINT8     nPID
  ) ;

VOID
SetTDDataBuffer(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

BOOLEAN
IsTDStatusActive(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

BOOLEAN
IsTDStatusStalled(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

BOOLEAN
IsTDStatusBufferError(
  IN  TD_STRUCT *ptrTDStruct
  ) ;


BOOLEAN 
IsTDStatusBabbleError(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

BOOLEAN
IsTDStatusNAKReceived(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

BOOLEAN 
IsTDStatusCRCTimeOutError(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

BOOLEAN 
IsTDStatusBitStuffError(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINT16
GetTDStatusActualLength (
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINT16 
GetTDTokenMaxLength(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINT8 
GetTDTokenEndPoint(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINT8 
GetTDTokenDeviceAddress(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINT8 
GetTDTokenPacketID(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINT8*
GetTDDataBuffer(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

BOOLEAN
GetTDLinkPtrValidorInvalid(
  IN  TD_STRUCT *ptrTDStruct
  ) ;

UINTN
CountTDsNumber(
  IN  TD_STRUCT *ptrFirstTD
  ) ;

VOID
LinkTDToQH(
  IN  QH_STRUCT *ptrQH,
  IN  TD_STRUCT *ptrTD
  ) ;

VOID
LinkTDToTD(
  IN  TD_STRUCT *ptrPreTD,
  IN  TD_STRUCT *ptrTD
  ) ;


VOID
SetorClearCurFrameListTerminate(
  IN  FRAMELIST_ENTRY   *pCurEntry,
  IN  BOOLEAN           bSet
  ) ;

VOID
SetCurFrameListQHorTD(
  IN  FRAMELIST_ENTRY    *pCurEntry,
  IN  BOOLEAN             bQH
  ) ;


BOOLEAN
GetCurFrameListTerminate(
  IN  FRAMELIST_ENTRY    *pCurEntry
  ) ;

VOID
SetCurFrameListPointer(
  IN  FRAMELIST_ENTRY   *pCurEntry,
  IN  UINT8             *ptr
  );

VOID*
GetCurFrameListPointer(
  IN  FRAMELIST_ENTRY    *pCurEntry
  ) ;

VOID
LinkQHToFrameList(
  IN  FRAMELIST_ENTRY   *pEntry,
  IN  UINT16            FrameListIndex,
  IN  QH_STRUCT         *ptrQH
  );


VOID
DeleteQHTDs(
  IN  FRAMELIST_ENTRY *pEntry,
  IN  QH_STRUCT       *ptrQH,
  IN  TD_STRUCT       *ptrFirstTD,
  IN  UINT16          FrameListIndex,
  IN  BOOLEAN         SearchOther
  ) ;
  
VOID
DelLinkSingleQH (
  IN  USB_HC_DEV      *HcDev,
  IN  QH_STRUCT       *ptrQH,
  IN  UINT16          FrameListIndex,
  IN  BOOLEAN         SearchOther,
  IN  BOOLEAN         Delete
  );

VOID
DeleteQueuedTDs(
  IN  USB_HC_DEV      *HcDev,
  IN  TD_STRUCT       *ptrFirstTD
  );
  
VOID
InsertQHTDToINTList(
  IN  USB_HC_DEV                        *HcDev,
  IN  QH_STRUCT                         *ptrQH,
  IN  TD_STRUCT                         *ptrFirstTD,
  IN  UINT8                             DeviceAddress,
  IN  UINT8                             EndPointAddress,
  IN  UINT8                             DataToggle,
  IN  UINTN                             DataLength,  
  IN  UINTN                             PollingInterval,
  IN  VOID                              *Mapping,
  IN  UINT8                             *DataBuffer,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK   CallBackFunction,
  IN  VOID                              *Context
  );

EFI_STATUS
DeleteAsyncINTQHTDs(
  IN  USB_HC_DEV  *HcDev,
  IN  UINT8       DeviceAddress,
  IN  UINT8       EndPointAddress,
  OUT UINT8       *DataToggle
  );


BOOLEAN
CheckTDsResults(
  IN  TD_STRUCT               *ptrTD,
  OUT UINT32                  *Result,
  OUT UINTN                   *ErrTDPos,
  OUT UINTN                   *ActualTransferSize
  );

VOID
ExecuteAsyncINTTDs(
  IN  USB_HC_DEV      *HcDev,
  IN  INTERRUPT_LIST  *ptrList,
  OUT UINT32          *Result,
  OUT UINTN           *ErrTDPos,
  OUT UINTN           *ActualLen
  )  ;


VOID
UpdateAsyncINTQHTDs(
  IN  INTERRUPT_LIST  *ptrList,
  IN  UINT32          Result,
  IN  UINT32          ErrTDPos
  ) ;

VOID
ClearTDStatus(
  IN  TD_STRUCT *ptrTD
  ) ;


VOID
ReleaseInterruptList(
  IN  USB_HC_DEV      *HcDev,
  IN  EFI_LIST_ENTRY  *ListHead
  ) ;

EFI_STATUS
ExecuteControlTransfer (
  IN  USB_HC_DEV  *HcDev,
  IN  TD_STRUCT   *ptrTD,
  IN  UINT32      wIndex,
  OUT UINTN       *ActualLen,
  IN  UINTN       TimeOut,
  OUT UINT32      *TransferResult
  );
  
EFI_STATUS
ExecBulkorSyncInterruptTransfer (
  IN  USB_HC_DEV  *HcDev,
  IN  TD_STRUCT   *ptrTD,
  IN  UINT32      wIndex,
  OUT UINTN       *ActualLen,
  OUT UINT8       *DataToggle,
  IN  UINTN       TimeOut,
  OUT UINT32      *TransferResult
  );    

EFI_STATUS
InitializeMemoryManagement (
  USB_HC_DEV           *HcDev
  );

EFI_STATUS
CreateMemoryBlock (
  USB_HC_DEV            *HcDev,
  MEMORY_MANAGE_HEADER  **MemoryHeader,
  UINTN                 MemoryBlockSizeInPages
  );

EFI_STATUS
FreeMemoryHeader (
  USB_HC_DEV            *HcDev,
  MEMORY_MANAGE_HEADER  *MemoryHeader
  );

EFI_STATUS
UhciAllocatePool (
  USB_HC_DEV      *UhcDev,
  UINT8           **Pool,
  UINTN           AllocSize
  );

VOID
UhciFreePool (
  USB_HC_DEV      *HcDev,
  UINT8           *Pool,
  UINTN           AllocSize
  );

VOID
InsertMemoryHeaderToList (
  MEMORY_MANAGE_HEADER  *MemoryHeader,
  MEMORY_MANAGE_HEADER  *NewMemoryHeader
  );

EFI_STATUS
AllocMemInMemoryBlock (
  MEMORY_MANAGE_HEADER  *MemoryHeader,
  VOID                  **Pool,
  UINTN                 NumberOfMemoryUnit
  );

BOOLEAN
IsMemoryBlockEmptied (
  MEMORY_MANAGE_HEADER  *MemoryHeaderPtr
  );

VOID
DelinkMemoryBlock (  
  MEMORY_MANAGE_HEADER    *FirstMemoryHeader,
  MEMORY_MANAGE_HEADER    *FreeMemoryHeader
  );

EFI_STATUS
DelMemoryManagement (
  USB_HC_DEV      *HcDev
  );

VOID
SelfLinkBulkTransferQH (
  IN  QH_STRUCT *ptrQH
  );  

VOID
EnableUhc (
  IN EFI_PCI_IO_PROTOCOL  *PciIo
  );

VOID
EnableMaxPacketSize (
  USB_HC_DEV          *HcDev
  );

EFI_STATUS
GenPseudoTD (
  USB_HC_DEV      *HcDev,
  UINT8           DeviceAddress,
  UINT8           Endpoint,
  UINT8           PktID,
  UINT8           DataToggle,
  BOOLEAN         bSlowDevice,
  TD_STRUCT       **ppTD  
  );
  
VOID
MakeQHLoop (
  QH_STRUCT   *PseudoQH,
  QH_STRUCT   *ptrQH
  );
  
BOOLEAN
IsQHsLooped (
  QH_STRUCT   *CurrentQH,
  QH_STRUCT   **LastLoopQH
  );
  
VOID
CleanUsbTransactions (
  USB_HC_DEV    *HcDev
  );

VOID
TurnOffUSBEmulation (
  IN EFI_PCI_IO_PROTOCOL  *PciIo  
  );

#endif  
