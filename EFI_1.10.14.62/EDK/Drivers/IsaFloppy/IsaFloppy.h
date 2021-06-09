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

  IsaFloppy.h
  
Abstract:
  Include for ISA Floppy Driver
  Define the data structure and so on
    
Revision History:

--*/

#ifndef _ISA_FLOPPY_H
#define _ISA_FLOPPY_H

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (IsaIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (BlockIo)

// 
// define some value
//
#define STALL_1_SECOND  1000000
#define STALL_1_MSECOND 1000

#define DATA_IN     1
#define DATA_OUT    0
#define READ        0
#define WRITE       1

//
// Internal Data Structures
//
#define FDC_BLK_IO_DEV_SIGNATURE  EFI_SIGNATURE_32('F','B','I','O')
#define FLOPPY_CONTROLLER_CONTEXT_SIGNATURE  EFI_SIGNATURE_32('F','D','C','C')


typedef enum {
  FDC_DISK0 = 0,
  FDC_DISK1 = 1,
  FDC_MAX_DISK = 2
} EFI_FDC_DISK;

typedef struct {
  UINT32          Signature;
  EFI_LIST_ENTRY  Link;
  BOOLEAN         FddResetPerformed;
  EFI_STATUS      FddResetStatus;
  BOOLEAN         NeedRecalibrate;
  UINT8           NumberOfDrive;
  UINT16          BaseAddress;
} FLOPPY_CONTROLLER_CONTEXT;

typedef struct {
  UINTN                     Signature;
  EFI_HANDLE                Handle;
  EFI_BLOCK_IO_PROTOCOL     BlkIo;
  EFI_BLOCK_IO_MEDIA        BlkMedia;
  
  EFI_ISA_IO_PROTOCOL       *IsaIo;
  
  UINT16                    BaseAddress;

  EFI_FDC_DISK              Disk;
  UINT8                     PresentCylinderNumber;
  UINT8                     *Cache;

  EFI_EVENT                 Event;
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;
  FLOPPY_CONTROLLER_CONTEXT *ControllerState;
} FDC_BLK_IO_DEV;

#define FDD_BLK_IO_FROM_THIS(a) CR(a, FDC_BLK_IO_DEV, BlkIo, FDC_BLK_IO_DEV_SIGNATURE)
#define FLOPPY_CONTROLLER_FROM_LIST_ENTRY(a) \
  CR(a, FLOPPY_CONTROLLER_CONTEXT, Link, FLOPPY_CONTROLLER_CONTEXT_SIGNATURE)
  
#define DISK_1440K_EOT                   0x12
#define DISK_1440K_GPL                   0x1b
#define DISK_1440K_DTL                   0xff
#define DISK_1440K_NUMBER                0x02
#define DISK_1440K_MAXTRACKNUM           0x4f
#define DISK_1440K_BYTEPERSECTOR         512

typedef struct {
  UINT8 CommandCode;      
  UINT8 DiskHeadSel;
  UINT8 Cylinder;
  UINT8 Head;
  UINT8 Sector;
  UINT8 Number;
  UINT8 EndOfTrack;
  UINT8 GapLength;
  UINT8 DataLength;     
} FDD_COMMAND_PACKET1;

typedef struct {
  UINT8 CommandCode;      
  UINT8 DiskHeadSel;
} FDD_COMMAND_PACKET2;

typedef struct {
  UINT8 CommandCode;
  UINT8 SrtHut;
  UINT8 HltNd;
} FDD_SPECIFY_CMD;

typedef struct {
  UINT8 CommandCode;      
  UINT8 DiskHeadSel;
  UINT8 NewCylinder;  
} FDD_SEEK_CMD;

typedef struct {
  UINT8 CommandCode;      
  UINT8 DiskHeadSel;
  UINT8 Cylinder;
  UINT8 Head;
  UINT8 Sector;
  UINT8 EndOfTrack;
  UINT8 GapLength;
  UINT8 ScanTestPause;      
} FDD_SCAN_CMD;

typedef struct {
  UINT8 Status0;
  UINT8 Status1;
  UINT8 Status2;
  UINT8 C;
  UINT8 H;
  UINT8 S;
  UINT8 Number;
} FDD_RESULT_PACKET;

//
// FDC Registers
//

//
// 0x3F2 Digital Output Register
//
#define FDC_REGISTER_DOR    2 

//
// 0x3F4 Main Status Register
//
#define FDC_REGISTER_MSR    4

//
// 0x3F5 Data Register 
//
#define FDC_REGISTER_DTR    5 

//
// 0x3F7 Configuration Control Register(data rate select)
//
#define FDC_REGISTER_CCR    7 

//
// 0x3F7 Digital Input Register(diskchange)
//
#define FDC_REGISTER_DIR    7 

//
//Set up mask bit
//
#define BIT0  (1 << 0)
#define BIT1  (1 << 1)
#define BIT2  (1 << 2)
#define BIT3  (1 << 3)
#define BIT4  (1 << 4)
#define BIT5  (1 << 5)
#define BIT6  (1 << 6)
#define BIT7  (1 << 7)

//
// FDC Register Bit Definitions 
//

//
// Digital Out Register(WO)
//

//
// Select Drive: 0=A 1=B
//
#define SELECT_DRV        BIT0  

//
// Reset FDC 
//
#define RESET_FDC         BIT2  

//
// Enable Int & DMA
//
#define INT_DMA_ENABLE    BIT3  

//
// Turn On Drive A Motor
//
#define DRVA_MOTOR_ON     BIT4  

//
// Turn On Drive B Motor
//
#define DRVB_MOTOR_ON     BIT5  

//
// Main Status Register(RO)
//

//
// Drive A Busy
//
#define MSR_DAB           BIT0  

//
// Drive B Busy
//
#define MSR_DBB           BIT1  

//
// FDC Busy
//
#define MSR_CB            BIT4  

//
// Non-DMA Mode
//
#define MSR_NDM           BIT5  

//
// Data Input/Output
//
#define MSR_DIO           BIT6  

//
// Request For Master
//
#define MSR_RQM           BIT7  

//
// Configuration Control Register(WO)
//

//
// Data Rate select
//
#define CCR_DRC           (BIT0|BIT1) 

//
// Digital Input Register(RO)
//

//
// Disk change line
//
#define DIR_DCL           BIT7 
//#define CCR_DCL         BIT7      // Diskette change

//
// 500K
//
#define DRC_500KBS        0x0  

//
// 300K
//
#define DRC_300KBS        0x01 

//
// 250K
// 
#define DRC_250KBS        0x02 

//
// FDC Command Code
//

#define READ_DATA_CMD         0x06
#define WRITE_DATA_CMD        0x05
#define WRITE_DEL_DATA_CMD    0x09
#define READ_DEL_DATA_CMD     0x0C
#define READ_TRACK_CMD        0x02
#define READ_ID_CMD           0x0A
#define FORMAT_TRACK_CMD      0x0D
#define SCAN_EQU_CMD          0x11
#define SCAN_LOW_EQU_CMD      0x19
#define SCAN_HIGH_EQU_CMD     0x1D
#define SEEK_CMD              0x0F
#define RECALIBRATE_CMD       0x07
#define SENSE_INT_STATUS_CMD  0x08
#define SPECIFY_CMD           0x03
#define SENSE_DRV_STATUS_CMD  0x04

//
//CMD_MT: Multi_Track Selector
//when set , this flag selects the multi-track operating mode.
//In this mode, the FDC treats a complete cylinder under head0 and 1 
//as a single track
//
#define CMD_MT          BIT7  

//
//CMD_MFM: MFM/FM Mode Selector
//A one selects the double density(MFM) mode
//A zero selects single density (FM) mode
//
#define CMD_MFM         BIT6

//
//CMD_SK: Skip Flag
//When set to 1, sectors containing a deleted data address mark will 
//automatically be skipped during the execution of Read Data.
//When set to 0, the sector is read or written the same as the read and 
//write commands.
//
#define CMD_SK          BIT5

//
// FDC Status Register Bit Definitions
//

//
// Status Register 0
//

//
// Interrupt Code
//
#define STS0_IC   (BIT7|BIT6)

//
// Seek End: the FDC completed a seek or recalibrate command   
//
#define STS0_SE   BIT5

//
// Equipment Check      
//
#define STS0_EC   BIT4      

//
// Not Ready(unused), this bit is always 0
//
#define STS0_NR   BIT3   

//
// Head Address: the current head address   
//
#define STS0_HA   BIT2     

//
// STS0_US1 & STS0_US0: Drive Select(the current selected drive)
//

//
// Unit Select1
//
#define STS0_US1  BIT1      

//
// Unit Select0
//
#define STS0_US0  BIT0      

//
// Status Register 1
//

//
// End of Cylinder
//
#define STS1_EN   BIT7      

//
//BIT6 is unused
//

//
// Data Error: The FDC detected a CRC error in either the ID field or 
// data field of a sector
//
#define STS1_DE   BIT5      

//
// Overrun/Underrun: Becomes set if FDC does not receive CPU or DMA service 
// within the required time interval
//
#define STS1_OR   BIT4      

//
// BIT3 is unused
//

//
// No data
//
#define STS1_ND   BIT2      

//
// Not Writable
//
#define STS1_NW   BIT1      

//
// Missing Address Mark
//
#define STS1_MA   BIT0      

//
// Status Register 2
//

//
// BIT7 is unused
//

//
// Control Mark
//
#define STS2_CM   BIT6      

//
// Data Error in Data Field: The FDC detected a CRC error in the data field
//
#define STS2_DD   BIT5      

//
// Wrong Cylinder: The track address from sector ID field is different from 
// the track address maintained inside FDC
//
#define STS2_WC   BIT4 

//
// Scan Equal Hit     
//
//#define STS2_SH   BIT3

//
// Scan Not Satisfied        
//
//#define STS2_SN   BIT2       

//BIT3 is unused
//BIT2 is unused

//
// Bad Cylinder
//
#define STS2_BC   BIT1      

//
// Missing Address Mark in Data Field
//
#define STS2_MD   BIT0      

//
// Status Register 3
//

//#define STS3_FT   BIT7        // Fault
//BIT7 is unused

//
// Write Protected
//
#define STS3_WP   BIT6      

//#define STS3_RDY    BIT5        // Ready
//BIT5 is unused

//
// Track 0
//
#define STS3_T0   BIT4      

//#define STS3_TS   BIT3        // Two Side
//BIT3 is unused

//
// Head Address
//
#define STS3_HD   BIT2      

//
//STS3_US1 & STS3_US0 : Drive Select
//
#define STS3_US1  BIT1      // Unit Select1
#define STS3_US0  BIT0      // Unit Select0

//
// Status Register 0 Interrupt Code Description
//

//
// Normal Termination of Command
//
#define IC_NT     0x0       

//
// Abnormal Termination of Command
//
#define IC_AT     0x40

//
// Invalid Command
//      
#define IC_IC     0x80      

//
// Abnormal Termination caused by Polling 
//
#define IC_ATRC   0xC0      

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gFdcControllerDriver;
extern EFI_COMPONENT_NAME_PROTOCOL gIsaFloppyComponentName;
                  
//
// EFI Driver Binding Protocol Functions
//
EFI_STATUS
FdcControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
FdcControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
FdcControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  );

//
// EFI Block I/O Protocol Functions
//
EFI_STATUS 
FdcReset (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  );

EFI_STATUS 
FddFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  );

EFI_STATUS
FddReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                LBA,
  IN  UINTN                  BufferSize,
  OUT VOID                   *Buffer
  );

EFI_STATUS
FddWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                LBA,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
  );
  
//
// Prototypes of internal functions   
//
EFI_STATUS
DiscoverFddDevice (
  IN FDC_BLK_IO_DEV  *FdcDev
  );

EFI_STATUS                                                         
FddIdentify (                                                  
  IN FDC_BLK_IO_DEV  *FdcDev
);

EFI_STATUS
FddReset (
  IN FDC_BLK_IO_DEV  *FdcDev
  );
  
EFI_STATUS
MotorOn (
  IN FDC_BLK_IO_DEV  *FdcDev
  );

EFI_STATUS
MotorOff (
  IN FDC_BLK_IO_DEV  *FdcDev
  );

EFI_STATUS
DisketChanged (
  IN FDC_BLK_IO_DEV  *FdcDev  
  );

EFI_STATUS
Specify ( 
  IN FDC_BLK_IO_DEV  *FdcDev  
  );

EFI_STATUS
Recalibrate (
  IN FDC_BLK_IO_DEV  *FdcDev
  );

EFI_STATUS
Seek (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN EFI_LBA         Lba
  );
  
STATIC
EFI_STATUS
SenseIntStatus ( 
  IN     FDC_BLK_IO_DEV  *FdcDev,
  IN OUT UINT8           *StatusRegister0,
  IN OUT UINT8           *PresentCylinderNumber
  );

EFI_STATUS
SenseDrvStatus (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN EFI_LBA         Lba
  );

EFI_STATUS
DetectMedia (
  IN FDC_BLK_IO_DEV  *FdcDev
  );

EFI_STATUS
Setup (
  IN FDC_BLK_IO_DEV  *FdcDev
  );
  
EFI_STATUS
ReadWriteDataSector (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN VOID            *HostAddress,
  IN EFI_LBA         Lba, 
  IN UINTN           NumberOfBlocks,
  IN BOOLEAN         Read
  );

VOID
FillPara (
  IN FDC_BLK_IO_DEV       *FdcDev,
  IN EFI_LBA              Lba,
  IN FDD_COMMAND_PACKET1  *Command
  );
  
EFI_STATUS
DataInByte (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINT8           *Pointer
  );

EFI_STATUS
DataOutByte (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINT8           *Pointer
  );
  
EFI_STATUS
FddWaitForBSYClear (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINTN           TimeoutInSeconds
  );

EFI_STATUS  
FddDRQReady (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN BOOLEAN         Dio,
  IN UINTN           TimeoutInSeconds
  );

EFI_STATUS
CheckResult (
  IN     FDD_RESULT_PACKET  Result,
  IN OUT FDC_BLK_IO_DEV     *FdcDev
  );

EFI_STATUS
CheckStatus3 (
  IN UINT8 StatusRegister3
  );

UINTN
GetTransferBlockCount (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN EFI_LBA         LBA,
  IN UINTN           NumberOfBlocks
  );

VOID
FddTimerProc ( 
  IN EFI_EVENT  Event, 
  IN VOID       *Context 
  );  

UINT8 
FdcReadPort (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINT32          Offset
  );

VOID 
FdcWritePort (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINT32          Offset,
  IN UINT8           Data
  );

EFI_STATUS
FddReadWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                LBA,
  IN  UINTN                  BufferSize,
  IN  BOOLEAN                Operation,
  OUT VOID                   *Buffer
  );

VOID
FdcFreeCache (
  IN    FDC_BLK_IO_DEV  *FdcDev
);

#endif
