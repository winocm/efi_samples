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

    AtapiPassThru.h
    
Abstract: 
    

Revision History
--*/

#ifndef _APT_H
#define _APT_H

#include "Efi.h"
#include "EfiDriverLib.h"
#include "Pci22.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)

// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (ScsiPassThru)

//
// bit definition 
//
#define bit(a)  1 << (a)

#define MAX_TARGET_ID 4
//
// IDE Registers 
//
typedef union 
{
    UINT16  Command;    /* when write */
    UINT16  Status;     /* when read */
} IDE_CMD_OR_STATUS;

typedef union {
    UINT16  Error;      /* when read */
    UINT16  Feature;    /* when write */ 
} IDE_ERROR_OR_FEATURE;

typedef union {
    UINT16 AltStatus;       /* when read */
    UINT16 DeviceControl;   /* when write */
} IDE_AltStatus_OR_DeviceControl;

//
// IDE registers set 
//
typedef struct {
    UINT16                  Data;
    IDE_ERROR_OR_FEATURE    Reg1;                  
    UINT16                  SectorCount;
    UINT16                  SectorNumber;
    UINT16                  CylinderLsb;
    UINT16                  CylinderMsb;
    UINT16                  Head;
    IDE_CMD_OR_STATUS       Reg;                   

    IDE_AltStatus_OR_DeviceControl       Alt;
    UINT16                  DriveAddress;

    UINT16                  MasterSlave;
} IDE_BASE_REGISTERS;


#define ATAPI_SCSI_PASS_THRU_DEV_SIGNATURE   EFI_SIGNATURE_32('a','s','p','t')

typedef struct {
    UINTN                         Signature;
    
    EFI_HANDLE                    Handle;
    EFI_SCSI_PASS_THRU_PROTOCOL   ScsiPassThru;
    EFI_SCSI_PASS_THRU_MODE       ScsiPassThruMode;
    EFI_PCI_IO_PROTOCOL           *PciIo;

    //
    // Local Data goes here
    //
    IDE_BASE_REGISTERS            *IoPort;

    CHAR16                        ControllerName[100];
    CHAR16                        ChannelName[100];
    
    UINT32                        LatestTargetId;
    UINT64                        LatestLun;

} ATAPI_SCSI_PASS_THRU_DEV;

#define ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS(a) CR(a, ATAPI_SCSI_PASS_THRU_DEV, ScsiPassThru, ATAPI_SCSI_PASS_THRU_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gAtapiScsiPassThruDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gAtapiScsiPassThruComponentName;

//
// ATAPI Command op code
//
#define OP_INQUIRY              0x12
#define OP_LOAD_UNLOAD_CD       0xa6
#define OP_MECHANISM_STATUS     0xbd
#define OP_MODE_SELECT_10       0x55
#define OP_MODE_SENSE_10        0x5a
#define OP_PAUSE_RESUME         0x4b
#define OP_PLAY_AUDIO_10        0x45
#define OP_PLAY_AUDIO_MSF       0x47
#define OP_PLAY_CD              0xbc
#define OP_PLAY_CD_MSF          0xb4
#define OP_PREVENT_ALLOW_MEDIUM_REMOVAL   0x1e
#define OP_READ_10              0x28
#define OP_READ_12              0xa8
#define OP_READ_CAPACITY        0x25
#define OP_READ_CD              0xbe
#define OP_READ_CD_MSF          0xb9
#define OP_READ_HEADER          0x44
#define OP_READ_SUB_CHANNEL     0x42
#define OP_READ_TOC             0x43
#define OP_REQUEST_SENSE        0x03
#define OP_SCAN                 0xba
#define OP_SEEK_10              0x2b
#define OP_SET_CD_SPEED         0xbb
#define OP_STOPPLAY_SCAN        0x4e
#define OP_START_STOP_UNIT      0x1b
#define OP_TEST_UNIT_READY      0x00

#define OP_FORMAT_UNIT          0x04
#define OP_READ_FORMAT_CAPACITIES 0x23
#define OP_VERIFY               0x2f
#define OP_WRITE_10             0x2a
#define OP_WRITE_12             0xaa
#define OP_WRITE_AND_VERIFY     0x2e


//
// ATA Command
//
#define ATAPI_SOFT_RESET_CMD            0x08

typedef enum {
  DataIn = 0,
  DataOut = 1,
  NoData = 2,
  End = 0xff
} DATA_DIRECTION;

typedef struct {
  UINT8               OpCode;
  DATA_DIRECTION      Direction;
} SCSI_COMMAND_SET;

#define MAX_CHANNEL 2

#define ValidCdbLength(Len)  ((Len) == 6 || (Len) == 10 || (Len) == 12)? 1 : 0

//
// IDE registers bit definitions
//

// ATA Err Reg bitmap
#define BBK_ERR     bit(7)    /* Bad block detected */
#define UNC_ERR     bit(6)    /* Uncorrectable Data */
#define MC_ERR      bit(5)    /* Media Change */
#define IDNF_ERR    bit(4)    /* ID Not Found */
#define MCR_ERR     bit(3)    /* Media Change Requested */
#define ABRT_ERR    bit(2)    /* Aborted Command */
#define TK0NF_ERR   bit(1)    /* Track 0 Not Found */
#define AMNF_ERR    bit(0)    /* Address Mark Not Found */

// ATAPI Err Reg bitmap
#define SENSE_KEY_ERR   (bit(7) | bit(6) | bit(5) | bit(4))  
//#define MCR_ERR       bit(3)
//#define ABRT_ERR      bit(2)
#define EOM_ERR         bit(1)    /* End of Media Detected */
#define ILI_ERR         bit(0)    /* Illegal Length Indication */

// Device/Head Reg
#define LBA_MODE    bit(6)
#define DEV         bit(4)
#define HS3         bit(3)
#define HS2         bit(2)
#define HS1         bit(1)
#define HS0         bit(0)
#define CHS_MODE    (0)
#define DRV0        (0)
#define DRV1        (1)
#define MST_DRV     DRV0
#define SLV_DRV     DRV1

//Status Reg
#define BSY     bit(7)    /* Controller Busy */ 
#define DRDY    bit(6)    /* Drive Ready */             
#define DWF     bit(5)    /* Drive Write Fault */       
#define DSC     bit(4)    /* Disk Seek Complete */      
#define DRQ     bit(3)    /* Data Request */            
#define CORR    bit(2)    /* Corrected Data */          
#define IDX     bit(1)    /* Index */                   
#define ERR     bit(0)    /* Error */ 
#define CHECK   bit(0)    /* Check bit for ATAPI Status Reg */                  

// Device Control Reg
#define  SRST   bit(2)    /* Software Reset */
#define  IEN_L  bit(1)    /* Interrupt Enable #*/

// ATAPI Feature Register
#define OVERLAP bit(1)
#define DMA     bit(0)

// ATAPI Interrupt Reason Reson Reg (ATA Sector Count Register)
#define RELEASE   bit(2)
#define IO        bit(1)
#define CoD       bit(0)

#define PACKET_CMD                      0xA0

#define DEFAULT_CMD    (0xa0)
// default content of device control register, disable INT  
#define DEFAULT_CTL    (0x0a)
#define MAX_ATAPI_BYTE_COUNT (0xfffe)

//
// function prototype
//
EFI_STATUS
AtapiScsiPassThruDriverEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  ) ;
  
EFI_STATUS
RegisterAtapiScsiPassThru (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_PCI_IO_PROTOCOL         *PciIo
  );
  
EFI_STATUS
AtapiScsiPassThruFunction (
  IN EFI_SCSI_PASS_THRU_PROTOCOL                  *This,
  IN UINT32                                       Target,
  IN UINT64                                       Lun,
  IN OUT EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET   *Packet,
  IN EFI_EVENT                                    Event OPTIONAL
  );      

EFI_STATUS
AtapiScsiPassThruGetNextDevice (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN OUT UINT32                      *Target,
  IN OUT UINT64                      *Lun
  );

EFI_STATUS
AtapiScsiPassThruBuildDevicePath (
  IN     EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN     UINT32                         Target,
  IN     UINT64                         Lun,
  IN OUT EFI_DEVICE_PATH_PROTOCOL       **DevicePath
  );

EFI_STATUS
AtapiScsiPassThruGetTargetLun (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN  EFI_DEVICE_PATH_PROTOCOL       *DevicePath,
  OUT UINT32                         *Target,
  OUT UINT64                         *Lun
  );

EFI_STATUS
AtapiScsiPassThruResetChannel (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL   *This
  );
  
EFI_STATUS
AtapiScsiPassThruResetTarget (
  IN EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN UINT32                         Target,
  IN UINT64                         Lun
  );

EFI_STATUS
CheckSCSIRequestPacket (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET      *Packet
  );
  
EFI_STATUS
SubmitBlockingIoCommand (
  ATAPI_SCSI_PASS_THRU_DEV                  *AtapiScsiPrivate,
  UINT32                                    Target,
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET    *Packet  
  );
  
BOOLEAN
IsCommandValid (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET   *Packet
  ) ;        

EFI_STATUS
RequestSenseCommand(
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT32                      Target,
  UINT64                      Timeout,
  VOID                        *SenseData,
  UINT8                       *SenseDataLength
  );

EFI_STATUS
AtapiPacketCommand (
  ATAPI_SCSI_PASS_THRU_DEV                  *AtapiScsiPrivate,
  UINT32                                    Target,
  UINT8                                     *PacketCommand,
  VOID                                      *Buffer,   
  UINT32                                    *ByteCount,
  DATA_DIRECTION                            Direction,
  UINT64                                    TimeOutInMicroSeconds
  );

STATIC
UINT8
ReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  );

STATIC
UINT16
ReadPortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  );

STATIC
VOID
WritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT8                 Data
  );

STATIC
VOID
WritePortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT16                Data
  );

EFI_STATUS  
StatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  );

EFI_STATUS  
AltStatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  );

  
EFI_STATUS  
StatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  );

EFI_STATUS  
AltStatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  );

EFI_STATUS  
StatusWaitForBSYClear (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  );

EFI_STATUS  
AltStatusWaitForBSYClear  (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  );

EFI_STATUS  
StatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  );      

EFI_STATUS
AltStatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  );      
  
EFI_STATUS 
AtapiPassThruPioReadWriteData (       
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate, 
  UINT16                    *Buffer,   
  UINT32                    *ByteCount,
  DATA_DIRECTION            Direction,
  UINT64                    TimeOutInMicroSeconds
  );
  
EFI_STATUS
AtapiPassThruCheckErrorStatus (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate
  );      
#endif
