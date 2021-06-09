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

  UsbMassStorageHelper.c
    
Abstract:

  Helper functions for USB Mass Storage Driver

Revision History

--*/
#include "UsbMassStorageHelper.h"

STATIC
BOOLEAN
IsNoMedia(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );

STATIC
BOOLEAN
IsMediaError(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );

STATIC
BOOLEAN
IsMediaChange(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );

STATIC
BOOLEAN
IsDriveReady(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts,
  OUT BOOLEAN               *NeedRetry
  );

STATIC    
BOOLEAN
IsMediaWriteProtected(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );

STATIC  
BOOLEAN
IsLogicalUnitCommunicationOverRun (
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );
  
EFI_STATUS
USBFloppyPacketCommand(
  USB_FLOPPY_DEV            *UsbFloppyDevice,
  VOID                      *Command,
  UINT8                     CommandSize,
  VOID                      *DataBuffer,
  UINT32                    BufferLength,
  EFI_USB_DATA_DIRECTION    Direction,
  UINT16                    TimeOutInMilliSeconds
  )
/*++

  Routine Description:
    Sends Packet Command to USB Floppy Drive.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
    Command:            A pointer to the command packet.
    CommandSize:        Indicates the size of the command packet.
    DataBuffer:         A pointer to the buffer for the data transfer
                        after the command packet.              
    BufferLength:       Indicates the size of the Data Buffer.
  
  Returns:  

--*/    
{
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  EFI_STATUS                Status;
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
  //
  // Directly calling EFI_USB_ATAPI_PROTOCOL.UsbAtapiPacketCmd()
  // to perform the command request.
  //
  Status = UsbAtapiInterface->UsbAtapiPacketCmd (
                                UsbAtapiInterface,
                                Command,
                                CommandSize,
                                DataBuffer,
                                BufferLength,
                                Direction,
                                TimeOutInMilliSeconds
                                );
    
  return Status;
}         

EFI_STATUS
USBFloppyIdentify (                                                    
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

  Routine Description:
    Retrieves device information to tell the device type.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
      
  Returns:  

--*/    
{

  EFI_STATUS            Status ; 
  USB_INQUIRY_DATA      *Idata;  
  BOOLEAN               MediaChange;


  //
  // Send Inquiry Packet Command to get INQUIRY data.
  //
  Status = USBFloppyInquiry(UsbFloppyDevice,&Idata);
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
   
  //
  // Get media removable info from INQUIRY data.
  //
  UsbFloppyDevice->BlkIo.Media->RemovableMedia = 
                                  (UINT8)((Idata->RMB & 0x80) == 0x80);

  //
  // Identify device type via INQUIRY data.
  //
  switch ((Idata->peripheral_type) & 0x1f) 
  {
  case 0x00: // Floppy
    UsbFloppyDevice->DeviceType = USBFLOPPY;
    UsbFloppyDevice->BlkIo.Media->MediaId = 0 ;
    UsbFloppyDevice->BlkIo.Media->MediaPresent = FALSE;    // Give initial value
    UsbFloppyDevice->BlkIo.Media->LastBlock = 0 ;
    UsbFloppyDevice->BlkIo.Media->BlockSize = 0x200; 
    break;
  
  case 0x05:  // CD-ROM
    UsbFloppyDevice->DeviceType = USBCDROM;
    UsbFloppyDevice->BlkIo.Media->MediaId = 0 ;
    UsbFloppyDevice->BlkIo.Media->MediaPresent = FALSE;    // Give initial value
    UsbFloppyDevice->BlkIo.Media->LastBlock = 0 ;
    UsbFloppyDevice->BlkIo.Media->BlockSize = 0x800; 
    UsbFloppyDevice->BlkIo.Media->ReadOnly = TRUE;
    break;
    
  default:
    gBS->FreePool (Idata);
    return EFI_DEVICE_ERROR;
  }
  
  //
  // Initialize some device specific data.
  //
  
  UsbFloppyDevice->SenseDataNumber = 6;   // original sense data numbers 
  
  if (UsbFloppyDevice->SenseData != NULL) {
    gBS->FreePool (UsbFloppyDevice->SenseData);
    UsbFloppyDevice->SenseData = NULL;
  }
  
  Status = gBS->AllocatePool(
            EfiBootServicesData,
            UsbFloppyDevice->SenseDataNumber * sizeof(REQUEST_SENSE_DATA),
            &UsbFloppyDevice->SenseData
         );
  if(EFI_ERROR(Status)) {
    gBS->FreePool (Idata);    
    return EFI_DEVICE_ERROR ;
  }
  
  //
  // Get media information.
  //
  UsbFloppyDetectMedia(UsbFloppyDevice,&MediaChange);
  
  gBS->FreePool (Idata);

  return EFI_SUCCESS;
}

EFI_STATUS
USBFloppyInquiry(
  IN    USB_FLOPPY_DEV        *UsbFloppyDevice,
  OUT   USB_INQUIRY_DATA      **Idata
  )
/*++

  Routine Description:
    Send Inquiry Packet Command to device and retrieve Inquiry Data.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
    Idata:              A pointer pointing to the address of 
                        Inquiry Data.
      
  Returns:  

--*/      
{
  ATAPI_PACKET_COMMAND      Packet;
  EFI_STATUS                Status;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
  
  //
  // prepare command packet for the Inquiry Packet Command.
  //
  EfiZeroMem(&Packet, sizeof(ATAPI_PACKET_COMMAND));
  Packet.Inquiry.opcode             = INQUIRY;  
  Packet.Inquiry.page_code          = 0;
  Packet.Inquiry.allocation_length  = sizeof(USB_INQUIRY_DATA);

  Status = gBS->AllocatePool(EfiBootServicesData,
                             sizeof(USB_INQUIRY_DATA),
                             Idata
                             );
  if (EFI_ERROR(Status)) {    
    return EFI_DEVICE_ERROR ;
  }
  EfiZeroMem (*Idata,sizeof (USB_INQUIRY_DATA));

  //
  // Send command packet and retrieve requested Inquiry Data. 
  //
  Status = USBFloppyPacketCommand (UsbFloppyDevice, 
                                   &Packet,
                                   sizeof(ATAPI_PACKET_COMMAND), 
                                   (VOID *)(*Idata), 
                                   sizeof(USB_INQUIRY_DATA),
                                   EfiUsbDataIn,
                                   USBFLPTIMEOUT * 3
                                   );
  if (EFI_ERROR(Status)) {
   return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
} 

EFI_STATUS
USBFloppyRead10(
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  IN    VOID              *Buffer, 
  IN    EFI_LBA           Lba, 
  IN    UINTN             NumberOfBlocks
  )
/*++

  Routine Description:
    Sends Read10 Packet Command to device to perform data transfer
    from device to host.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
    Buffer:             A pointer to the destination buffer for the data. 
                        The caller is responsible for either having implicit
                        or explicit ownership of the buffer.
    Lba:                The starting logical block address to read from 
                        on the device.
    NumberOfBlocks:     Indicates the number of blocks that the read 
                        operation requests.
      
  Returns:  

--*/      
{
  ATAPI_PACKET_COMMAND      Packet;
  READ10_CMD                *Read10Packet;
  UINT16                    MaxBlock ;
  UINT16                    BlocksRemaining;
  UINT16                    SectorCount;
  UINT32                    Lba32;
  UINT32                    BlockSize, ByteCount;
  VOID                      *ptrBuffer;
  EFI_STATUS                Status;
  UINT16                    TimeOut;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  UINTN                     SenseCounts;
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;

  //
  // prepare command packet for the Inquiry Packet Command.
  //
  EfiZeroMem(&Packet, sizeof(ATAPI_PACKET_COMMAND));
  Read10Packet = &Packet.Read10;
  Lba32 = (UINT32)Lba;
  ptrBuffer = Buffer;
  BlockSize = UsbFloppyDevice->BlkIo.Media->BlockSize;

  MaxBlock = (UINT16)(65536 / BlockSize);  
  BlocksRemaining = (UINT16)NumberOfBlocks;

  Status = EFI_SUCCESS;
  while (BlocksRemaining > 0) {
    
    if (BlocksRemaining <= MaxBlock) {
      
      SectorCount = BlocksRemaining ;
    
    } else {
      
      SectorCount = MaxBlock ;
    }

    //
    // fill the Packet data structure
    //

    Read10Packet->opcode = READ_10;

    //
    // Lba0 ~ Lba3 specify the start logical block address of the data transfer.
    // Lba0 is MSB, Lba3 is LSB   
    //
    Read10Packet->Lba3 = (UINT8)(Lba32 & 0xff);          
    Read10Packet->Lba2 = (UINT8)(Lba32 >> 8);          
    Read10Packet->Lba1 = (UINT8)(Lba32 >> 16);          
    Read10Packet->Lba0 = (UINT8)(Lba32 >> 24); 
      
    //
    // TranLen0 ~ TranLen1 specify the transfer length in block unit.
    // TranLen0 is MSB, TranLen is LSB 
    //
    Read10Packet->TranLen1 = (UINT8)(SectorCount & 0xff);   
    Read10Packet->TranLen0 = (UINT8)(SectorCount >> 8);    

    ByteCount = SectorCount * BlockSize;
    
    TimeOut = (UINT16)(SectorCount * USBFLPTIMEOUT);
    
    Status = USBFloppyPacketCommand(
                                UsbFloppyDevice, 
                                &Packet,
                                sizeof(ATAPI_PACKET_COMMAND), 
                                (VOID *)ptrBuffer,
                                ByteCount,
                                EfiUsbDataIn,
                                TimeOut
                                );
    if (EFI_ERROR(Status)) {
      
      Status = UsbFloppyRequestSense (UsbFloppyDevice,&SenseCounts);
      if (!EFI_ERROR(Status)) {
        if (IsLogicalUnitCommunicationOverRun (
                                      UsbFloppyDevice->SenseData,
                                      SenseCounts)) {
          Lba32 = (UINT32)Lba;
          ptrBuffer = Buffer;
          BlocksRemaining = (UINT16)NumberOfBlocks;
          MaxBlock = (UINT16)(MaxBlock / 4);
          if (MaxBlock < 1) {
            MaxBlock = 1;
          }
          continue;
        }
      } else {
        return EFI_DEVICE_ERROR;
      }

      //
      // retry read10 command
      //
      Status = USBFloppyPacketCommand(
                                UsbFloppyDevice, 
                                &Packet,
                                sizeof(ATAPI_PACKET_COMMAND), 
                                (VOID *)ptrBuffer,
                                ByteCount,
                                EfiUsbDataIn,
                                TimeOut
                                );
      if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
      }
    }
    
    Lba32 += SectorCount;
    ptrBuffer = (UINT8 *)ptrBuffer + SectorCount * BlockSize ;
    BlocksRemaining = (UINT16)(BlocksRemaining - SectorCount);
  }
  
  return Status;
}     


EFI_STATUS 
USBFloppyReadCapacity (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

  Routine Description:
    Retrieves media capacity information via 
    sending Read Capacity Packet Command.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
      
  Returns:  

--*/        
{ 
  //
  // status returned by Read Capacity Packet Command
  //
  EFI_STATUS                Status;       
  ATAPI_PACKET_COMMAND      Packet;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  
  //
  // used for capacity data returned from Usb Floppy
  //
  READ_CAPACITY_DATA        Data ;
  
  EfiZeroMem(&Data,sizeof(Data));
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
  
  EfiZeroMem (&Packet, sizeof(ATAPI_PACKET_COMMAND));
  Packet.Inquiry.opcode = READ_CAPACITY;  
  Status = USBFloppyPacketCommand (UsbFloppyDevice, 
                                    &Packet,
                                    sizeof(ATAPI_PACKET_COMMAND),
                                    (VOID *)&Data, 
                                    sizeof(READ_CAPACITY_DATA), 
                                    EfiUsbDataIn,
                                    USBFLPTIMEOUT
                                    ) ;
  
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
    
  UsbFloppyDevice->BlkIo.Media->LastBlock = (Data.LastLba3 << 24) |
                                            (Data.LastLba2 << 16) | 
                                            (Data.LastLba1 << 8)  |
                                             Data.LastLba0;

  UsbFloppyDevice->BlkIo.Media->MediaPresent = TRUE;

  UsbFloppyDevice->BlkIo.Media->BlockSize = 0x800 ;

  return EFI_SUCCESS ;  

}


EFI_STATUS 
USBFloppyReadFormatCapacity (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

  Routine Description:
    Retrieves media capacity information via sending Read Format 
    Capacity Packet Command.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
      
  Returns:  

--*/         
{ 
  //
  // status returned by Read Capacity Packet Command
  //
  EFI_STATUS                Status;       
  ATAPI_PACKET_COMMAND      Packet;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  
  //
  // used for capacity data returned from Usb Floppy
  //
  READ_FORMAT_CAPACITY_DATA   FormatData ;

  EfiZeroMem(&FormatData,sizeof(FormatData));

  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
    
  EfiZeroMem(&Packet,sizeof(ATAPI_PACKET_COMMAND));
  Packet.ReadFormatCapacity.opcode = READ_FORMAT_CAPACITY ;
  Packet.ReadFormatCapacity.allocation_length_lo = 12 ;
  Status = USBFloppyPacketCommand (UsbFloppyDevice, 
                                    &Packet,
                                    sizeof(ATAPI_PACKET_COMMAND),
                                    (VOID *)&FormatData, 
                                    sizeof(READ_FORMAT_CAPACITY_DATA), 
                                    EfiUsbDataIn,
                                    USBFLPTIMEOUT
                                    ) ;
  
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  if(FormatData.DesCode == 3) {
  //
  // Media is not present
  //      
    UsbFloppyDevice->BlkIo.Media->MediaId = 0 ;
    UsbFloppyDevice->BlkIo.Media->MediaPresent = FALSE;
    UsbFloppyDevice->BlkIo.Media->LastBlock = 0;
  } else {
        
    UsbFloppyDevice->BlkIo.Media->LastBlock = (FormatData.LastLba3 << 24) |
                      (FormatData.LastLba2 << 16) | 
                        (FormatData.LastLba1 << 8)  |
                        FormatData.LastLba0;

    UsbFloppyDevice->BlkIo.Media->LastBlock--;      
                  
    UsbFloppyDevice->BlkIo.Media->BlockSize =  (FormatData.BlockSize2 << 16) |
                                               (FormatData.BlockSize1 << 8)  |
                                                FormatData.BlockSize0;
 
    UsbFloppyDevice->BlkIo.Media->MediaPresent = TRUE;

    UsbFloppyDevice->BlkIo.Media->BlockSize = 0x200 ;

  }

  return EFI_SUCCESS ;  

}

EFI_STATUS  
UsbFloppyRequestSense ( 
  IN  USB_FLOPPY_DEV  *UsbFloppyDevice,
  OUT UINTN           *SenseCounts
  )
/*++

  Routine Description:
    Retrieves Sense Data from device via 
    sending Request Sense Packet Command.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
    SenseCounts:        A pointer to the number of Sense Data returned.
      
  Returns:  

--*/         
{
  EFI_STATUS                Status;
  REQUEST_SENSE_DATA        *Sense;
  UINT8                     *Ptr ;
  BOOLEAN                   SenseReq;
  ATAPI_PACKET_COMMAND      Packet;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;

  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
  
  *SenseCounts = 0 ;
  
  EfiZeroMem(UsbFloppyDevice->SenseData, 
             sizeof(REQUEST_SENSE_DATA) * (UsbFloppyDevice->SenseDataNumber));
  //
  // fill command packet for Request Sense Packet Command
  //
  EfiZeroMem(&Packet, sizeof(ATAPI_PACKET_COMMAND));
  Packet.RequestSense.opcode = REQUEST_SENSE; 
  Packet.RequestSense.allocation_length = sizeof(REQUEST_SENSE_DATA);

  Ptr = (UINT8*)(UsbFloppyDevice->SenseData) ; // initialize pointer

  //
  //  request sense data from device continuously 
  //  until no sense data exists in the device.
  //
  for (SenseReq = TRUE; SenseReq == TRUE; ) {
    
    Sense = (REQUEST_SENSE_DATA *) Ptr ;

    //
    // send out Request Sense Packet Command and get one Sense
    // data from device.
    //
    Status = USBFloppyPacketCommand (UsbFloppyDevice, 
                                      &Packet,
                                      sizeof(ATAPI_PACKET_COMMAND),
                                      (VOID*)Ptr,
                                      sizeof(REQUEST_SENSE_DATA),
                                      EfiUsbDataIn,
                                      USBFLPTIMEOUT
                                      );
    //
    // failed to get Sense data
    //
    if (EFI_ERROR(Status)) {
      //
      // Recovery the device back to normal state.
      //
      UsbFloppyDevice->AtapiProtocol->UsbAtapiReset 
        (UsbFloppyDevice->AtapiProtocol,FALSE);
      
      if (*SenseCounts == 0) {
      //
      // never retrieved any sense data from device,
      // just return error.
      //
        return EFI_DEVICE_ERROR;
      } else {
      //
      // has retrieved some sense data from device,
      // so return success.
      // 
        return EFI_SUCCESS ;
      }
    }
    
    if (Sense->sense_key != SK_NO_SENSE) {       
      
      Ptr += sizeof(REQUEST_SENSE_DATA)  ; // Ptr is byte based pointer
      (*SenseCounts) ++;
  
      
    } else {
      //
      // when no sense key, skip out the loop
      //
      SenseReq = FALSE;   
    }
  
    //
    // If the sense key numbers exceed Sense Data Buffer size,
    // just skip the loop and do not fetch the sense key in this function.
    //
    if (*SenseCounts == UsbFloppyDevice->SenseDataNumber ) {
       SenseReq = FALSE;
    }       
  }
  
  return EFI_SUCCESS; 
} 


EFI_STATUS                                                         
UsbFloppyTestUnitReady (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  ) 
/*++

  Routine Description:
    Sends Test Unit ReadyPacket Command to the device.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
      
  Returns:  

--*/  
{ 
  ATAPI_PACKET_COMMAND      Packet; 
  EFI_STATUS                Status;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;

  //
  // fill command packet
  //
  EfiZeroMem(&Packet, sizeof(ATAPI_PACKET_COMMAND));
  Packet.TestUnitReady.opcode = TEST_UNIT_READY;

  //
  // send command packet
  //
  Status = USBFloppyPacketCommand (UsbFloppyDevice, 
                                  &Packet,
                                  sizeof(ATAPI_PACKET_COMMAND), 
                                  NULL, 
                                  0,
                                  EfiUsbNoData,
                                  USBFLPTIMEOUT
                                  );
  if(EFI_ERROR(Status)) {    
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
} 



EFI_STATUS
USBFloppyWrite10(
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  IN    VOID              *Buffer, 
  IN    EFI_LBA           Lba, 
  IN    UINTN             NumberOfBlocks
  )
/*++

  Routine Description:
    Sends Write10 Packet Command to device to perform data transfer
    from host to device.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
    Buffer:             A pointer to the source buffer for the data. 
                        The caller is responsible for either having implicit
                        or explicit ownership of the buffer.
    Lba:                The starting logical block address to written to 
                        the device.
    NumberOfBlocks:     Indicates the number of blocks that the write 
                        operation requests.
      
  Returns:  

--*/      
{
  ATAPI_PACKET_COMMAND    Packet;
  READ10_CMD              *Write10Packet;
  UINT16                  MaxBlock ;
  UINT16                  BlocksRemaining;
  UINT16                  SectorCount;
  UINT32                  Lba32;
  UINT32                  BlockSize, ByteCount;
  VOID                    *ptrBuffer;
  EFI_STATUS              Status;
  UINT16                  TimeOut;
  EFI_USB_ATAPI_PROTOCOL  *UsbAtapiInterface;
  UINTN                   SenseCounts;
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;

  //
  // prepare command packet for the Write10 Packet Command.
  //
  EfiZeroMem(&Packet, sizeof(ATAPI_PACKET_COMMAND));
  Write10Packet = &Packet.Read10;
  Lba32 = (UINT32)Lba;
  ptrBuffer = Buffer;
  BlockSize = UsbFloppyDevice->BlkIo.Media->BlockSize;
  
  MaxBlock = (UINT16)(65536 / BlockSize);  
  BlocksRemaining = (UINT16)NumberOfBlocks;
  
  Status = EFI_SUCCESS;
  while (BlocksRemaining > 0) {
    
    if (BlocksRemaining <= MaxBlock) {
      
      SectorCount = BlocksRemaining ;
    } else {
      
      SectorCount = MaxBlock ;
    }

    //
    // fill the Packet data structure
    //

    Write10Packet->opcode = WRITE_10;

    //
    // Lba0 ~ Lba3 specify the start logical block address 
    // of the data transfer.
    // Lba0 is MSB, Lba3 is LSB   
    //
    Write10Packet->Lba3 = (UINT8)(Lba32 & 0xff);          
    Write10Packet->Lba2 = (UINT8)(Lba32 >> 8);          
    Write10Packet->Lba1 = (UINT8)(Lba32 >> 16);          
    Write10Packet->Lba0 = (UINT8)(Lba32 >> 24); 
      
    //
    // TranLen0 ~ TranLen1 specify the transfer length in block unit.
    // TranLen0 is MSB, TranLen is LSB 
    //
    Write10Packet->TranLen1 = (UINT8)(SectorCount & 0xff);   
    Write10Packet->TranLen0 = (UINT8)(SectorCount >> 8);    

    ByteCount = SectorCount * BlockSize;
    
    TimeOut = (UINT16)(SectorCount * USBFLPTIMEOUT);
 
    Status = USBFloppyPacketCommand(
                              UsbFloppyDevice, 
                              &Packet,
                              sizeof(ATAPI_PACKET_COMMAND), 
                              (void *)ptrBuffer,
                              ByteCount,
                              EfiUsbDataOut,
                              TimeOut
                              );
    if (EFI_ERROR(Status)) {
      Status = UsbFloppyRequestSense (UsbFloppyDevice,&SenseCounts);
      if (!EFI_ERROR(Status)) {
        if (IsLogicalUnitCommunicationOverRun (
                                      UsbFloppyDevice->SenseData,
                                      SenseCounts)) {
          Lba32 = (UINT32)Lba;
          ptrBuffer = Buffer;
          BlocksRemaining = (UINT16)NumberOfBlocks;
          MaxBlock = (UINT16)(MaxBlock / 4);
          if (MaxBlock < 1) {
            MaxBlock = 1;
          }
          continue;
        }
      }
      //
      // retry write10 command
      //
      Status = USBFloppyPacketCommand(
                              UsbFloppyDevice, 
                              &Packet,
                              sizeof(ATAPI_PACKET_COMMAND), 
                              (void *)ptrBuffer,
                              ByteCount,
                              EfiUsbDataOut,
                              TimeOut
                              );
      if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
      }
    } 
    
    Lba32 += SectorCount;
    ptrBuffer = (UINT8 *)ptrBuffer + SectorCount * BlockSize;
    BlocksRemaining = (UINT16)(BlocksRemaining - SectorCount);
  }
  
  return Status;
}     

EFI_STATUS 
UsbFloppyDetectMedia (
  IN  USB_FLOPPY_DEV  *UsbFloppyDevice,
  OUT BOOLEAN         *MediaChange
  )
/*++

  Routine Description:
    Retrieves media information.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
    MediaChange:        Indicates whether media was changed.
      
  Returns:  

--*/        
{
  EFI_STATUS          Status;
  EFI_STATUS          FloppyStatus;
  //
  // the following variables are used to record previous media information
  //
  EFI_BLOCK_IO_MEDIA  OldMediaInfo;
  UINTN               SenseCounts;
  UINTN               RetryIndex;
  UINTN               RetryTimes;
  UINTN               MaximumRetryTimes;
  BOOLEAN             NeedRetry;

  //
  // a flag used to determine whether need to perform Read Capacity command.
  //
  BOOLEAN             NeedReadCapacity;

  REQUEST_SENSE_DATA  *SensePtr;

  //
  // init 
  //
  Status            = EFI_SUCCESS;
  FloppyStatus      = EFI_SUCCESS;
  OldMediaInfo      = *UsbFloppyDevice->BlkIo.Media;
  *MediaChange      = FALSE ;
  NeedReadCapacity  = TRUE;
  
  //
  // if there is no media present,or media not changed,
  // the request sense command will detect faster than read capacity command.
  // read capacity command can be bypassed, thus improve performance.
  //
  SenseCounts = 0;
  Status = UsbFloppyRequestSense(UsbFloppyDevice, &SenseCounts);
  
  if (!EFI_ERROR(Status)) {
  
    SensePtr = UsbFloppyDevice->SenseData;

    //
    // No Media
    //
    if(IsNoMedia (UsbFloppyDevice->SenseData,SenseCounts)) {
        
      NeedReadCapacity = FALSE ;
      UsbFloppyDevice->BlkIo.Media->MediaId = 0 ;
      UsbFloppyDevice->BlkIo.Media->MediaPresent = FALSE;
      UsbFloppyDevice->BlkIo.Media->LastBlock = 0;
    } else {        
      //
      // Media Changed
      //
      if (IsMediaChange (UsbFloppyDevice->SenseData,SenseCounts)) {
        UsbFloppyDevice->BlkIo.Media->MediaId++;
      }
        
      //
      // Media Write-protected
      //
      if(IsMediaWriteProtected(UsbFloppyDevice->SenseData,SenseCounts)) {
        UsbFloppyDevice->BlkIo.Media->ReadOnly = TRUE;        
      }
        
      //
      // Media Error
      //
      if (IsMediaError (UsbFloppyDevice->SenseData, SenseCounts)) {
        //
        // if media error encountered, make it look like no media present.
        // 
        UsbFloppyDevice->BlkIo.Media->MediaId = 0 ;
        UsbFloppyDevice->BlkIo.Media->MediaPresent = FALSE;
        UsbFloppyDevice->BlkIo.Media->LastBlock = 0;        
      }

    }
    
  }
  
  if (NeedReadCapacity) {
    //
    // at most retry 5 times
    //
    MaximumRetryTimes = 5;
    RetryTimes = 2 ;      // initial retry twice

    for (RetryIndex = 0; (RetryIndex < RetryTimes) && (RetryIndex < MaximumRetryTimes); RetryIndex ++) {
      
      //
      // Using different command to retrieve media capacity.
      //
      switch (UsbFloppyDevice->DeviceType)  {
        case USBCDROM:
          Status = USBFloppyReadCapacity(UsbFloppyDevice);
          break;
        
        case USBFLOPPY:
          UsbFloppyModeSense1C (UsbFloppyDevice);
          Status = USBFloppyReadFormatCapacity(UsbFloppyDevice);
          if (EFI_ERROR(Status)) {            
            //
            // retry the ReadCapacity command
            //
            UsbFloppyDevice->DeviceType = USBFLOPPY2;
          }
          break;
        
        case USBFLOPPY2:        
          UsbFloppyModeSense1C (UsbFloppyDevice);
          Status = USBFloppyReadCapacity(UsbFloppyDevice);          
          if (EFI_ERROR(Status)) {            
            //
            // retry the ReadFormatCapacity command
            //
            UsbFloppyDevice->DeviceType = USBFLOPPY;
          }
          //
          // force the BlockSize to be 0x200.
          //
          UsbFloppyDevice->BlkIo.Media->BlockSize = 0x200;
          break;
          
        default:
          return EFI_INVALID_PARAMETER;
      }
      
      if (!EFI_ERROR(Status)) {
        //
        // skip the loop when read capacity succeeds.
        //
        break;
      } 
  
      SenseCounts = 0 ;
      
      FloppyStatus = UsbFloppyRequestSense (UsbFloppyDevice, &SenseCounts);
       
      //
      // If Request Sense data failed,retry.
      //
      if (EFI_ERROR(FloppyStatus)) {          
        RetryTimes ++ ;     // retry once more
        continue ;
      }
        
      //
      // No Media
      //
      if (IsNoMedia (UsbFloppyDevice->SenseData,SenseCounts)) {
          
        UsbFloppyDevice->BlkIo.Media->MediaId = 0 ;
        UsbFloppyDevice->BlkIo.Media->MediaPresent = FALSE;
        UsbFloppyDevice->BlkIo.Media->LastBlock = 0;
        break; 
      }

      if( IsMediaError (UsbFloppyDevice->SenseData,SenseCounts)) {
        //
        // if media error encountered, make it look like no media present.
        // 
        UsbFloppyDevice->BlkIo.Media->MediaId = 0 ;
        UsbFloppyDevice->BlkIo.Media->MediaPresent = FALSE;
        UsbFloppyDevice->BlkIo.Media->LastBlock = 0;
        break;
      }
        
      if(IsMediaWriteProtected(UsbFloppyDevice->SenseData,SenseCounts)) {
        UsbFloppyDevice->BlkIo.Media->ReadOnly = TRUE;
        continue;
      }

      if (!IsDriveReady (UsbFloppyDevice->SenseData,SenseCounts,&NeedRetry)) {
          
        //
        // Drive not ready: if NeedRetry, then retry once more;
        // else return error
        //
        if (NeedRetry) {
          //
          // Stall 0.1 second to wait for drive becoming ready
          //
          gBS->Stall(100 * STALL_1_MILLI_SECOND); 
          //
          // reset retry variable to zero, 
          // to make it retry for "drive in progress of becoming ready".
          //
          RetryIndex = 0;
          continue ;
        } else {            
          return EFI_DEVICE_ERROR;
        }
      }
        
      //
      // if read capacity fail not for above reasons, retry once more
      //
      RetryTimes ++;

    }     // ENDFOR
  
    //
    // tell whether the readcapacity process is successful or not
    // ("Status" variable record the latest status returned 
    // by ReadCapacity AND "FloppyStatus" record the latest status
    // returned by RequestSense)
    //
    if (EFI_ERROR(Status) && EFI_ERROR(FloppyStatus)) {
      return EFI_DEVICE_ERROR;
    }

  }

  if (UsbFloppyDevice->BlkIo.Media->MediaPresent != OldMediaInfo.MediaPresent) {
   
    if (UsbFloppyDevice->BlkIo.Media->MediaPresent) {
      UsbFloppyDevice->BlkIo.Media->MediaId = 1;
    }
    *MediaChange = TRUE;
  }

  if (UsbFloppyDevice->BlkIo.Media->ReadOnly != OldMediaInfo.ReadOnly) {
    *MediaChange = TRUE;
    UsbFloppyDevice->BlkIo.Media->MediaId += 1;
  }

  if (UsbFloppyDevice->BlkIo.Media->BlockSize != OldMediaInfo.BlockSize) {
    *MediaChange = TRUE;
    UsbFloppyDevice->BlkIo.Media->MediaId += 1;
  }

  if (UsbFloppyDevice->BlkIo.Media->LastBlock != OldMediaInfo.LastBlock) {
    *MediaChange = TRUE;
    UsbFloppyDevice->BlkIo.Media->MediaId += 1;
  }

  if (UsbFloppyDevice->BlkIo.Media->MediaId != OldMediaInfo.MediaId) {
    *MediaChange = TRUE;
  }

  return EFI_SUCCESS; 
}



EFI_STATUS
UsbFloppyModeSense5 (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

  Routine Description:
    Retrieves media capacity information via sending Read Format 
    Capacity Packet Command.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
      
  Returns:  

--*/         
{ 
  //
  // status returned by Read Capacity Packet Command
  //
  EFI_STATUS                Status;       
  ATAPI_PACKET_COMMAND      Packet;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  MODE_PARAMETER_PAGE_5     ModePage5;
  EFI_LBA                   LastBlock;
  UINT32                    SectorsPerTrack;
  UINT32                    NumberOfCylinders;
  UINT32                    NumberOfHeads;
  UINT32                    DataBytesPerSector;
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
  
  EfiZeroMem (&ModePage5,sizeof(MODE_PARAMETER_PAGE_5));
  
  EfiZeroMem (&Packet,sizeof(ATAPI_PACKET_COMMAND));
  Packet.ModeSense.opcode = MODE_SENSE ;
  Packet.ModeSense.page_code = 5 ;    // Flexible Disk Page
  Packet.ModeSense.page_control = 0;  // current values
  Packet.ModeSense.parameter_list_length_hi = 0;
  Packet.ModeSense.parameter_list_length_lo = sizeof (MODE_PARAMETER_PAGE_5); 
  Status = USBFloppyPacketCommand (UsbFloppyDevice, 
                                    &Packet,
                                    sizeof(ATAPI_PACKET_COMMAND),
                                    (VOID *)&ModePage5, 
                                    sizeof(MODE_PARAMETER_PAGE_5), 
                                    EfiUsbDataIn,
                                    USBFLPTIMEOUT
                                    ) ;
  
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  NumberOfHeads = ModePage5.flex_disk_page.number_of_heads;
  SectorsPerTrack = ModePage5.flex_disk_page.sectors_per_track;
  NumberOfCylinders = ModePage5.flex_disk_page.number_of_cylinders_msb << 8 
                        | ModePage5.flex_disk_page.number_of_cylinders_lsb;
                        
  LastBlock = SectorsPerTrack * NumberOfHeads * NumberOfCylinders;
  DataBytesPerSector = ModePage5.flex_disk_page.databytes_per_sector_msb << 8 
                        | ModePage5.flex_disk_page.databytes_per_sector_lsb;
                        
  UsbFloppyDevice->BlkIo.Media->LastBlock = LastBlock;

  UsbFloppyDevice->BlkIo.Media->LastBlock--;      
                  
  UsbFloppyDevice->BlkIo.Media->BlockSize = DataBytesPerSector;
 
  UsbFloppyDevice->BlkIo.Media->MediaPresent = TRUE;

  UsbFloppyDevice->BlkIo.Media->ReadOnly = 
    ModePage5.mode_param_header.write_protected;

  return EFI_SUCCESS ;

}

EFI_STATUS
UsbFloppyModeSense1C (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

  Routine Description:
    Retrieves media capacity information via sending Read Format 
    Capacity Packet Command.
  
  Arguments:
    UsbFloppyDevice:    The USB_FLOPPY_DEV instance.
      
  Returns:  

--*/         
{ 
  //
  // status returned by Read Capacity Packet Command
  //
  EFI_STATUS                Status;       
  ATAPI_PACKET_COMMAND      Packet;
  EFI_USB_ATAPI_PROTOCOL    *UsbAtapiInterface;
  MODE_PARAMETER_PAGE_1C    ModePage1C;
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
  
  EfiZeroMem (&ModePage1C,sizeof(MODE_PARAMETER_PAGE_1C));
  
  EfiZeroMem (&Packet,sizeof(ATAPI_PACKET_COMMAND));
  Packet.ModeSense.opcode = MODE_SENSE ;
  Packet.ModeSense.page_code = 0x1C;    // Flexible Disk Page
  Packet.ModeSense.page_control = 0;  // current values
  Packet.ModeSense.parameter_list_length_hi = 0;
  Packet.ModeSense.parameter_list_length_lo = sizeof (MODE_PARAMETER_PAGE_1C); 
  Status = USBFloppyPacketCommand (UsbFloppyDevice, 
                                    &Packet,
                                    sizeof(ATAPI_PACKET_COMMAND),
                                    (VOID *)&ModePage1C, 
                                    sizeof(MODE_PARAMETER_PAGE_1C), 
                                    EfiUsbDataIn,
                                    USBFLPTIMEOUT
                                    ) ;
  
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  UsbFloppyDevice->BlkIo.Media->ReadOnly = 
    ModePage1C.mode_param_header.write_protected;

  return EFI_SUCCESS ;

}



/*++

  The following functions are a set of helper functions,
  which are used to parse sense key returned by the device.

--*/


BOOLEAN
IsNoMedia(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  REQUEST_SENSE_DATA    *SensePtr;
  UINTN                 Index;
  BOOLEAN               NoMedia;

  NoMedia = FALSE ;

  SensePtr = SenseData;

  for ( Index = 0 ; Index < SenseCounts ; Index ++ ) {

    if ((SensePtr->sense_key == SK_NOT_READY)
        && (SensePtr->addnl_sense_code == ASC_NO_MEDIA)) {
      
      NoMedia = TRUE;
    }
            
    SensePtr ++;
  }

  return NoMedia;
}


BOOLEAN
IsMediaError(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  REQUEST_SENSE_DATA  *SensePtr;
  UINTN               Index;
  BOOLEAN             IsError;
  
  IsError = FALSE;
  SensePtr = SenseData;

  for ( Index = 0 ; Index < SenseCounts ; Index ++ ) {
  
    switch (SensePtr->sense_key) {
      
      //
      // Medium error case
      //      
      case SK_MEDIUM_ERROR:
        switch (SensePtr->addnl_sense_code) {
          case ASC_MEDIA_ERR1:  // fall through
          case ASC_MEDIA_ERR2:  // fall through
          case ASC_MEDIA_ERR3:  // fall through
          case ASC_MEDIA_ERR4:  
            IsError = TRUE;
            break;
          
          default:
            break;
        }
        
        break;
      //
      // Medium upside-down case
      //
      case SK_NOT_READY:
        switch (SensePtr->addnl_sense_code) {
          case ASC_MEDIA_UPSIDE_DOWN:
            IsError = TRUE;
            break;

          default:
            break;
        }
        break;

      default:
        break; 
    }
            
    SensePtr ++;
  }

  return IsError;
}

BOOLEAN
IsMediaChange(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  REQUEST_SENSE_DATA    *SensePtr;
  UINTN                 Index;
  BOOLEAN               MediaChanged;

  MediaChanged = FALSE;
  SensePtr = SenseData;

  for ( Index = 0 ; Index < SenseCounts ; Index ++ ) {
    
    if ((SensePtr->sense_key == SK_UNIT_ATTENTION)
        && (SensePtr->addnl_sense_code == ASC_MEDIA_CHANGE)) {
      
      MediaChanged = TRUE;
    }
            
    SensePtr ++ ;
  }

  return MediaChanged;
}

BOOLEAN
IsDriveReady(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts,
  OUT BOOLEAN               *NeedRetry
  )
{
  REQUEST_SENSE_DATA    *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsReady;

  IsReady = TRUE;
  *NeedRetry = FALSE;
  SensePtr = SenseData;

  for ( Index = 0 ; Index < SenseCounts ; Index ++ ) {
    
    if ((SensePtr->sense_key == SK_NOT_READY)
        && (SensePtr->addnl_sense_code == ASC_NOT_READY)) {
          
      switch (SensePtr->addnl_sense_code_qualifier) {
        case ASCQ_IN_PROGRESS:
        case ASCQ_DEVICE_BUSY:
          IsReady = FALSE;
          *NeedRetry = TRUE;
          break;
                
        default:
          //
          // Drive is in error condition,
          // no need to retry.
          //
          IsReady = FALSE;
          *NeedRetry = FALSE;
          break;
      }               
    }
                
    SensePtr ++ ;
  }

  return IsReady;
}

BOOLEAN
IsMediaWriteProtected(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  REQUEST_SENSE_DATA    *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsWriteProtected;

  IsWriteProtected = FALSE;
  SensePtr = SenseData;

  for ( Index = 0 ; Index < SenseCounts ; Index ++ ) {
    
    //
    // catch media write-protected condition.
    //
    if ((SensePtr->sense_key == SK_DATA_PROTECT)
        && (SensePtr->addnl_sense_code == ASC_WRITE_PROTECTED)) {
      
      IsWriteProtected = TRUE;
    }
  
    SensePtr ++;
  }

  return IsWriteProtected;
}

BOOLEAN
IsLogicalUnitCommunicationOverRun (
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  REQUEST_SENSE_DATA    *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsOverRun;
  
  IsOverRun = FALSE;
  SensePtr = SenseData;
  
  for ( Index = 0 ; Index < SenseCounts ; Index ++ ) {
    
    if ((SensePtr->sense_key == SK_NOT_READY)
        && (SensePtr->addnl_sense_code == ASC_LOGICAL_UNIT_STATUS)
        && (SensePtr->addnl_sense_code_qualifier == ASCQ_LOGICAL_UNIT_OVERRUN)) {
          IsOverRun = TRUE;
    }
    
    SensePtr ++;
  }

  return IsOverRun;
}
