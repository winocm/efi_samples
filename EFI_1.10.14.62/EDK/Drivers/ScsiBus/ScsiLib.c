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

    scsilib.c
    
Abstract: 
    

Revision History
--*/

#include "scsilib.h"

EFI_STATUS
SubmitTestUnitReadyCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  OUT VOID                  *SenseData,
  OUT UINT8                 *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus
  )
{
  EFI_SCSI_IO_SCSI_REQUEST_PACKET   CommandPacket;
  SCSI_IO_DEV                       *ScsiIoDevice;
  EFI_STATUS                        Status;
  UINT8                             Cdb[6];
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
  
  EfiZeroMem (&CommandPacket,sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,6);
  
  CommandPacket.Timeout         = Timeout;
  CommandPacket.DataBuffer      = NULL;
  CommandPacket.SenseData       = SenseData;
  CommandPacket.TransferLength  = 0;
  CommandPacket.Cdb             = Cdb;
  //
  // Fill Cdb for Test Unit Ready Command
  //
  Cdb[0]                        = EFI_SCSI_OP_TEST_UNIT_READY;
  Cdb[1]                        = (UINT8)(ScsiIoDevice->Lun & 0xe0);
  CommandPacket.CdbLength       = (UINT8)6;
  CommandPacket.SenseDataLength = *SenseDataLength;
  
  Status = ScsiIo->ExecuteSCSICommand (ScsiIo,&CommandPacket,NULL);
  
  *HostAdapterStatus = CommandPacket.HostAdapterStatus;
  *TargetStatus = CommandPacket.TargetStatus;
  *SenseDataLength = CommandPacket.SenseDataLength;
  
  return Status;
}

EFI_STATUS
SubmitInquiryCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  IN OUT VOID               *InquiryDataBuffer,
  IN OUT UINT32             *InquiryDataLength,
  IN  BOOLEAN               EnableVitalProductData
  )
{
  EFI_SCSI_IO_SCSI_REQUEST_PACKET   CommandPacket;
  SCSI_IO_DEV                       *ScsiIoDevice;
  EFI_STATUS                        Status;
  UINT8                             Cdb[6];
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
  
  EfiZeroMem (&CommandPacket,sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,6);
  
  CommandPacket.Timeout         = Timeout;
  CommandPacket.DataBuffer      = InquiryDataBuffer;
  CommandPacket.TransferLength  = *InquiryDataLength;
  CommandPacket.SenseData       = SenseData;
  CommandPacket.SenseDataLength = *SenseDataLength;
  CommandPacket.Cdb             = Cdb;
  
  Cdb[0]                        = EFI_SCSI_OP_INQUIRY;
  Cdb[1]                        = (UINT8)(ScsiIoDevice->Lun & 0xe0);
  if (EnableVitalProductData) {
    Cdb[1] |= 0x01;
  }
  
  if (*InquiryDataLength > 0xff) {
    *InquiryDataLength = 0xff;
  }
  
  Cdb[4]                  = (UINT8)(*InquiryDataLength);
  CommandPacket.CdbLength = (UINT8)6;
  CommandPacket.DataDirection = EFI_SCSI_DATA_IN;
  
  Status = ScsiIo->ExecuteSCSICommand (ScsiIo,&CommandPacket,NULL);
  
  *HostAdapterStatus  = CommandPacket.HostAdapterStatus;
  *TargetStatus       = CommandPacket.TargetStatus;
  *SenseDataLength    = CommandPacket.SenseDataLength;
  *InquiryDataLength  = CommandPacket.TransferLength;
  
  return Status;
}

EFI_STATUS
SubmitModeSense10Command (
  IN  EFI_SCSI_IO_PROTOCOL    *ScsiIo,
  IN  UINT64                  Timeout,
  IN  VOID                    *SenseData,
  IN OUT UINT8                *SenseDataLength,
  OUT UINT8                   *HostAdapterStatus,
  OUT UINT8                   *TargetStatus,
  IN  VOID                    *DataBuffer,
  IN OUT UINT32               *DataLength,
  IN  UINT8                   DBDField,   OPTIONAL
  IN  UINT8                   PageControl,
  IN  UINT8                   PageCode
  )
{
  EFI_SCSI_IO_SCSI_REQUEST_PACKET   CommandPacket;
  SCSI_IO_DEV                       *ScsiIoDevice;
  EFI_STATUS                        Status;
  UINT8                             Cdb[10];
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
  
  EfiZeroMem (&CommandPacket,sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,10);
  
  CommandPacket.Timeout         = Timeout;
  CommandPacket.DataBuffer      = DataBuffer;
  CommandPacket.SenseData       = SenseData;
  CommandPacket.TransferLength  = *DataLength;
  CommandPacket.Cdb             = Cdb;
  //
  // Fill Cdb for Mode Sense (10) Command
  //
  Cdb[0]           = EFI_SCSI_OP_MODE_SEN10;
  Cdb[1]           = (UINT8)(ScsiIoDevice->Lun & 0xe0 + (DBDField << 3) & 0x08);
  Cdb[2]           = (UINT8)((PageControl & 0xc0)| (PageCode & 0x3f));
  Cdb[7]           = (UINT8)(*DataLength >> 8);
  Cdb[8]           = (UINT8)(*DataLength);

  CommandPacket.CdbLength        = 10;
  CommandPacket.DataDirection    = EFI_SCSI_DATA_IN;
  CommandPacket.SenseDataLength  = *SenseDataLength;
  
  Status = ScsiIo->ExecuteSCSICommand (ScsiIo,&CommandPacket,NULL);
  
  *HostAdapterStatus  = CommandPacket.HostAdapterStatus;
  *TargetStatus       = CommandPacket.TargetStatus;
  *SenseDataLength    = CommandPacket.SenseDataLength;
  *DataLength         = CommandPacket.TransferLength;
  
  return Status;
}

EFI_STATUS
SubmitRequestSenseCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus
  )
{
  EFI_SCSI_IO_SCSI_REQUEST_PACKET   CommandPacket;
  SCSI_IO_DEV                       *ScsiIoDevice;
  EFI_STATUS                        Status;
  UINT8                             Cdb[6];
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
  
  EfiZeroMem (&CommandPacket,sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,6);
  
  if (*SenseDataLength > 0xff) {
    *SenseDataLength = 0xff;
  }
  
  CommandPacket.Timeout          = Timeout;
  CommandPacket.DataBuffer       = SenseData;
  CommandPacket.SenseData        = NULL;
  CommandPacket.TransferLength   = *SenseDataLength;
  CommandPacket.Cdb              = Cdb;
  //
  // Fill Cdb for Request Sense Command
  //
  Cdb[0] = EFI_SCSI_OP_REQUEST_SENSE;
  Cdb[1] = (UINT8)(ScsiIoDevice->Lun & 0xe0);  
  Cdb[4] = (UINT8)(*SenseDataLength);

  CommandPacket.CdbLength        = (UINT8)6;
  CommandPacket.DataDirection    = EFI_SCSI_DATA_IN;
  CommandPacket.SenseDataLength  = 0;
  
  Status = ScsiIo->ExecuteSCSICommand (ScsiIo,&CommandPacket,NULL);
  
  *HostAdapterStatus  = CommandPacket.HostAdapterStatus;
  *TargetStatus       = CommandPacket.TargetStatus;
  *SenseDataLength    = (UINT8)CommandPacket.TransferLength;
  
  return Status;
}

//
// Commands for direct access command
//
EFI_STATUS
SubmitReadCapacityCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  OUT VOID                  *DataBuffer,
  IN OUT UINT32             *DataLength,
  IN  BOOLEAN               PMI  
  )
{
  EFI_SCSI_IO_SCSI_REQUEST_PACKET   CommandPacket;
  SCSI_IO_DEV                       *ScsiIoDevice;
  EFI_STATUS                        Status;
  UINT8                             Cdb[10];
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
  
  EfiZeroMem (&CommandPacket,sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,10);

  CommandPacket.Timeout         = Timeout;
  CommandPacket.DataBuffer      = DataBuffer;
  CommandPacket.SenseData       = SenseData;
  CommandPacket.TransferLength  = *DataLength;
  CommandPacket.Cdb             = Cdb;
  //
  // Fill Cdb for Read Capacity Command
  //
  Cdb[0]           = EFI_SCSI_OP_READ_CAPACITY;
  Cdb[1]           = (UINT8)(ScsiIoDevice->Lun & 0xe0);
  if (PMI == FALSE) {
    //
    // Partial medium indicator,if PMI is FALSE, the Cdb.2 ~ Cdb.5 MUST BE ZERO.
    //
    EfiZeroMem ((Cdb + 2),4);
  } else {
    Cdb[8] |= 0x01;
  }

  CommandPacket.CdbLength        = 10;
  CommandPacket.DataDirection    = EFI_SCSI_DATA_IN;
  CommandPacket.SenseDataLength  = *SenseDataLength;
  
  Status = ScsiIo->ExecuteSCSICommand (ScsiIo,&CommandPacket,NULL);
  
  *HostAdapterStatus  = CommandPacket.HostAdapterStatus;
  *TargetStatus       = CommandPacket.TargetStatus;
  *SenseDataLength    = CommandPacket.SenseDataLength;
  *DataLength         = CommandPacket.TransferLength;
  
  return Status;
}

EFI_STATUS
SubmitRead10Command (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  OUT VOID                  *DataBuffer,
  IN OUT UINT32             *DataLength,
  IN  UINT32                StartLba,
  IN  UINT32                SectorSize
  )
{
  EFI_SCSI_IO_SCSI_REQUEST_PACKET   CommandPacket;
  SCSI_IO_DEV                       *ScsiIoDevice;
  EFI_STATUS                        Status;
  UINT8                             Cdb[10];
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
  
  EfiZeroMem (&CommandPacket,sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,10);
  
  CommandPacket.Timeout         = Timeout;
  CommandPacket.DataBuffer      = DataBuffer;
  CommandPacket.SenseData       = SenseData;
  CommandPacket.TransferLength  = *DataLength;
  CommandPacket.Cdb             = Cdb;
  //
  // Fill Cdb for Read (10) Command
  //
  Cdb[0] = EFI_SCSI_OP_READ10;
  Cdb[1] = (UINT8)(ScsiIoDevice->Lun & 0xe0);
  Cdb[2] = (UINT8)(StartLba >> 24);
  Cdb[3] = (UINT8)(StartLba >> 16);
  Cdb[4] = (UINT8)(StartLba >> 8);
  Cdb[5] = (UINT8)StartLba;
  Cdb[7] = (UINT8)(SectorSize >> 8);
  Cdb[8] = (UINT8)SectorSize;

  CommandPacket.CdbLength        = 10;
  CommandPacket.DataDirection    = EFI_SCSI_DATA_IN;
  CommandPacket.SenseDataLength  = *SenseDataLength;
  
  Status = ScsiIo->ExecuteSCSICommand (ScsiIo,&CommandPacket,NULL);
  
  *HostAdapterStatus  = CommandPacket.HostAdapterStatus;
  *TargetStatus       = CommandPacket.TargetStatus;
  *SenseDataLength    = CommandPacket.SenseDataLength;
  *DataLength         = CommandPacket.TransferLength;
  
  return Status;
}

EFI_STATUS
SubmitWrite10Command (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  OUT VOID                  *DataBuffer,
  IN OUT UINT32             *DataLength,
  IN  UINT32                StartLba,
  IN  UINT32                SectorSize
  )
{
  EFI_SCSI_IO_SCSI_REQUEST_PACKET   CommandPacket;
  SCSI_IO_DEV                       *ScsiIoDevice;
  EFI_STATUS                        Status;
  UINT8                             Cdb[10];
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
  
  EfiZeroMem (&CommandPacket,sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,10);
  
  CommandPacket.Timeout         = Timeout;
  CommandPacket.DataBuffer      = DataBuffer;
  CommandPacket.SenseData       = SenseData;
  CommandPacket.TransferLength  = *DataLength;
  CommandPacket.Cdb             = Cdb;
  //
  // Fill Cdb for Write (10) Command
  //
  Cdb[0] = EFI_SCSI_OP_WRITE10;
  Cdb[1] = (UINT8)(ScsiIoDevice->Lun & 0xe0);
  Cdb[2] = (UINT8)(StartLba >> 24);
  Cdb[3] = (UINT8)(StartLba >> 16);
  Cdb[4] = (UINT8)(StartLba >> 8);
  Cdb[5] = (UINT8)StartLba;
  Cdb[7] = (UINT8)(SectorSize >> 8);
  Cdb[8] = (UINT8)SectorSize;

  CommandPacket.CdbLength        = 10;
  CommandPacket.DataDirection    = EFI_SCSI_DATA_OUT;
  CommandPacket.SenseDataLength  = *SenseDataLength;
  
  Status = ScsiIo->ExecuteSCSICommand (ScsiIo,&CommandPacket,NULL);
  
  *HostAdapterStatus  = CommandPacket.HostAdapterStatus;
  *TargetStatus       = CommandPacket.TargetStatus;
  *SenseDataLength    = CommandPacket.SenseDataLength;
  *DataLength         = CommandPacket.TransferLength;
  
  return Status;
}
