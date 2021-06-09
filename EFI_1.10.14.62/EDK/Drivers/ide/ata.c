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

    ata.c
    
Abstract: 
    
Revision History
--*/

#include "idebus.h"

EFI_STATUS                                                         
ATAIdentify (                                                  
  IN  IDE_BLK_IO_DEV  *IdeDev
  )
/*++
  Name:
        ATAIdentify


  Purpose: 
        This function is called by DiscoverIdeDevice() during its device
        identification. It sends out the ATA Identify Command to the 
        specified device. Only ATA device responses to this command. If 
        the command succeeds, it returns the Identify data structure which 
        contains information about the device. This function extracts the 
        information it needs to fill the IDE_BLK_IO_DEV data structure, 
        including device type, media block size, media capacity, and etc.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure,used
          to record all the information of the IDE device.


  Returns:  
        EFI_SUCCESS
          Identify ATA device successfully.

        EFI_DEVICE_ERROR
          ATA Identify Device Command failed or device is not 
          ATA device.


  Notes:
        parameter IdeDev will be updated in this function.
--*/
{
  EFI_STATUS  Status;
  IDENTIFY    *AtaIdentifyPointer;
  UINT32      Capacity;
  UINT8       DeviceSelect;
      
  //
  //  AtaIdentifyPointer is used for accommodating returned IDENTIFY data of 
  //  the ATA Identify command
  //
  Status = gBS->AllocatePool(EfiBootServicesData,
                             sizeof(IDENTIFY),
                             (VOID**)&AtaIdentifyPointer
                             );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EfiZeroMem(AtaIdentifyPointer,sizeof(IDENTIFY));
  

  //
  //  use ATA PIO Data In protocol to send ATA Identify command
  //  and receive data from device
  //
  DeviceSelect = 0;
  DeviceSelect = (UINT8)((IdeDev->Device) << 4);
  Status = AtaPioDataIn (
          IdeDev, 
          (VOID*)AtaIdentifyPointer, 
          sizeof(IDENTIFY),
          IDENTIFY_DRIVE_CMD, 
          DeviceSelect, 
          0,
          0,
          0,
          0
        );
  //
  // If ATA Identify command succeeds, then according to the received 
  // IDENTIFY data,
  // identify the device type ( ATA or not ).
  // If ATA device, fill the information in IdeDev.
  // If not ATA device, return IDE_DEVICE_ERROR
  //
  if (!EFI_ERROR(Status)) {
    
    IdeDev->pIdData = AtaIdentifyPointer;
    
    //
    // Print ATA Module Name
    //
    PrintAtaModuleName(IdeDev);
    
    GetDeviceBestPIOMode (IdeDev);
    
    // bit 15 of pAtaIdentify->config is used to identify whether device is 
    // ATA device or ATAPI device.
    // if 0, means ATA device; if 1, means ATAPI device.
    //
    if ((AtaIdentifyPointer->config & 0x8000) == 0x00) {
      
      IdeDev->Type = IdeHardDisk;

      // 
      // Block Media Information:
      // Media->LogicalPartition , Media->WriteCaching will be filled
      // in the DiscoverIdeDevcie() function.
      // Media->IoAlign is default 0 .
      //
      IdeDev->BlkIo.Media->MediaId = 1 ;
      IdeDev->BlkIo.Media->RemovableMedia = FALSE ;
      IdeDev->BlkIo.Media->MediaPresent = TRUE ;
      IdeDev->BlkIo.Media->ReadOnly = FALSE ;
      IdeDev->BlkIo.Media->BlockSize = 0x200;

      //
      // "bit0 of field_validity == 1" means the value given to 
      // iCapacity is valid.
      //
      if ((AtaIdentifyPointer->field_validity & 0x1) == 0x1) {
        
        Capacity = (AtaIdentifyPointer->user_addressable_sectors_hi << 16) 
                    | AtaIdentifyPointer->user_addressable_sectors_lo ;
        IdeDev->BlkIo.Media->LastBlock = Capacity - 1;
        
        return EFI_SUCCESS;
      } 
    }
  }

  gBS->FreePool (AtaIdentifyPointer);
  //
  // Make sure the pIdData will not be freed again. 
  //
  IdeDev->pIdData = NULL;
  
  return EFI_DEVICE_ERROR;
}    

VOID 
PrintAtaModuleName(
  IN  IDE_BLK_IO_DEV  *IdeDev
  )
/*++
  Name:
        PrintAtaModuleName


  Purpose: 
        This function is called by ATAIdentify() or ATAPIIdentify()
        to print device's module name. 


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

  Returns:  
        no returns.

  Notes:
--*/   
{
  if (IdeDev->pIdData == NULL) {
    return;
  }
  
  SwapStringChars(IdeDev->ModelName, IdeDev->pIdData->ModelName, 40);
  IdeDev->ModelName[40] = 0x00;
}


EFI_STATUS
AtaPioDataIn (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *Buffer,
  IN  UINT32          ByteCount,
  IN  UINT8           AtaCommand,
  IN  UINT8           Head,
  IN  UINT8           SectorCount,
  IN  UINT8           SectorNumber,
  IN  UINT8           CylinderLsb,
  IN  UINT8           CylinderMsb
  )
/*++
  Name:
        AtaPioDataIn


  Purpose: 
        This function is used to send out ATA commands conforms to the 
        PIO Data In Protocol.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        VOID      IN    *Buffer
          buffer contained data transferred from device to host.

        UINT32      IN    ByteCount
          data size in byte unit of the buffer.

        UINT8     IN    AtaCommand
          value of the Command Register

        UINT8     IN    Head
          value of the Head/Device Register

        UINT8     IN    SectorCount
          value of the Sector Count Register

        UINT8     IN    SectorNumber
          value of the Sector Number Register

        UINT8     IN    CylinderLsb
          value of the low byte of the Cylinder Register

        UINT8     IN    CylinderMsb
          value of the high byte of the Cylinder Register


  Returns:  
        EFI_SUCCESS
          send out the ATA command and device send required
          data successfully.

        EFI_DEVICE_ERROR
          command sent failed.
  Notes:
--*/  
{
  UINTN       WordCount;
  UINTN       Increment;
  UINT16      *Buffer16;
  EFI_STATUS  Status;
  
  Status = WaitForBSYClear (IdeDev, ATATIMEOUT);
  if(EFI_ERROR(Status)){
    return EFI_DEVICE_ERROR;
  }

  //
  //  e0:1110,0000-- bit7 and bit5 are reserved bits.
  //           bit6 set means LBA mode
  //
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Head, 
                (UINT8)((IdeDev->Device << 4) | 0xe0 | Head)
                );   

  //
  // All ATAPI device's ATA commands can be issued regardless of the 
  // state of the DRDY
  //
  if(IdeDev->Type == IdeHardDisk){
    
    Status = DRDYReady(IdeDev, ATATIMEOUT);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }
  }
 
  //
  // set all the command parameters
  // Before write to all the following registers, BSY and DRQ must be 0.
  //
  Status = DRQClear2 (IdeDev, ATATIMEOUT);
  if(EFI_ERROR(Status)) {    
    return EFI_DEVICE_ERROR ;
  }
  if(AtaCommand == SET_FEATURES_CMD) {
    IDEWritePortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Feature, 0x03);
  }
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount, SectorCount); 
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorNumber, SectorNumber);
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->CylinderLsb, CylinderLsb); 
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->CylinderMsb, CylinderMsb); 

  //
  // send command via Command Register
  //
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Command, AtaCommand);
 
  Buffer16 = (UINT16 *)Buffer;
  
  // According to PIO data in protocol, host can perform a series of reads to 
  // the data register after each time device set DRQ ready;
  // The data size of "a series of read" is command specific.
  // For most ATA command, data size received from device will not exceed 
  // 1 sector, hence the data size for "a series of read" can be the whole data
  // size of one command request.
  // For ATA command such as Read Sector command, the data size of one ATA 
  // command request is often larger than 1 sector, according to the 
  // Read Sector command, the data size of "a series of read" is exactly 1 
  // sector.
  // Here for simplification reason, we specify the data size for 
  // "a series of read" to 1 sector (256 words) if data size of one ATA command
  // request is larger than 256 words.
  // 
  Increment = 256;      // 256 words

  WordCount = 0;        // used to record bytes of currently transfered data

  while ( WordCount < ByteCount/2 ) {
    //
    // Poll DRQ bit set, data transfer can be performed only when DRQ is ready.
    //
    Status = DRQReady2(IdeDev, ATATIMEOUT);
    if (EFI_ERROR(Status)) {      
      return EFI_DEVICE_ERROR;
    } 
    
    Status = CheckErrorStatus(IdeDev);
    if(EFI_ERROR(Status)) {      
      return EFI_DEVICE_ERROR;
    }

    //
    // Get the byte count for one series of read
    //
    if ( (WordCount + Increment) > ByteCount/2) {
      Increment = ByteCount/2 - WordCount ;
    }

    IDEReadPortWMultiple (IdeDev->PciIo,
                          IdeDev->IoPort->Data, 
                          Increment, 
                          Buffer16
                          );

    WordCount += Increment;
    Buffer16 += Increment;
    
  } // while 

  DRQClear(IdeDev, ATATIMEOUT);
  
  return CheckErrorStatus(IdeDev);
}




EFI_STATUS
AtaPioDataOut (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *Buffer,
  IN  UINT32          ByteCount,
  IN  UINT8           AtaCommand,
  IN  UINT8           Head,
  IN  UINT8           SectorCount,
  IN  UINT8           SectorNumber,
  IN  UINT8           CylinderLsb,
  IN  UINT8           CylinderMsb
  )
/*++
  Name:
        AtaPioDataOut


  Purpose: 
        This function is used to send out ATA commands conforms to the 
        PIO Data Out Protocol.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        VOID      IN    *Buffer
          buffer contained data transferred from host to device.

        UINT32      IN    ByteCount
          data size in byte unit of the buffer.

        UINT8     IN    AtaCommand
          value of the Command Register

        UINT8     IN    Head
          value of the Head/Device Register

        UINT8     IN    SectorCount
          value of the Sector Count Register

        UINT8     IN    SectorNumber
          value of the Sector Number Register

        UINT8     IN    CylinderLsb
          value of the low byte of the Cylinder Register

        UINT8     IN    CylinderMsb
          value of the high byte of the Cylinder Register


  Returns:  
        EFI_SUCCESS
          send out the ATA command and device received required
          data successfully.

        EFI_DEVICE_ERROR
          command sent failed. 

  Notes:
--*/  
{
  UINTN         WordCount;
  UINTN         Increment;
  UINT16        *Buffer16;
  EFI_STATUS    Status;

  Status = WaitForBSYClear (IdeDev, ATATIMEOUT);
  if(EFI_ERROR(Status)){
    return EFI_DEVICE_ERROR;
  }

  //
  // select device via Head/Device register.
  // Before write Head/Device register, BSY and DRQ must be 0.
  //
  Status = DRQClear2 (IdeDev, ATATIMEOUT);
  if(EFI_ERROR(Status)) {    
    return EFI_DEVICE_ERROR ;
  }

  //
  // e0:1110,0000-- bit7 and bit5 are reserved bits.
  //          bit6 set means LBA mode
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Head, 
                (UINT8)((IdeDev->Device << 4) | 0xe0 | Head)
                );   
  
  Status = DRDYReady(IdeDev, ATATIMEOUT);
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // set all the command parameters
  // Before write to all the following registers, BSY and DRQ must be 0.
  //
  Status = DRQClear2 (IdeDev, ATATIMEOUT);
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR ;
  }
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount, SectorCount); 
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorNumber, SectorNumber);
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->CylinderLsb, CylinderLsb); 
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->CylinderMsb, CylinderMsb); 

  //
  // send command via Command Register
  //
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Command, AtaCommand);
 
  Buffer16 = (UINT16 *)Buffer;

  // According to PIO data out protocol, host can perform a series of 
  // writes to the data register after each time device set DRQ ready;
  // The data size of "a series of read" is command specific.
  // For most ATA command, data size written to device will not exceed 1 sector,
  // hence the data size for "a series of write" can be the data size of one 
  // command request.
  // For ATA command such as Write Sector command, the data size of one 
  // ATA command request is often larger than 1 sector, according to the 
  // Write Sector command, the data size of "a series of read" is exactly
  // 1 sector.
  // Here for simplification reason, we specify the data size for 
  // "a series of write" to 1 sector (256 words) if data size of one ATA command
  // request is larger than 256 words.
  // 
  Increment = 256 ;
  WordCount = 0 ;

  while (WordCount < ByteCount/2)  {    
    
    //
    // DRQReady2-- read Alternate Status Register to determine the DRQ bit
    // data transfer can be performed only when DRQ is ready.
    //
    Status = DRQReady2(IdeDev, ATATIMEOUT);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    } 
    
    Status = CheckErrorStatus(IdeDev);
    if(EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }

    //
    // perform a series of write without check DRQ ready
    //
    IDEWritePortWMultiple (IdeDev->PciIo,
                            IdeDev->IoPort->Data, 
                            Increment, 
                            Buffer16
                            );
    WordCount += Increment ;
    Buffer16 += Increment;
    
  } // while

  DRQClear(IdeDev, ATATIMEOUT);
     
  return CheckErrorStatus(IdeDev);
}


EFI_STATUS
CheckErrorStatus (
  IN  IDE_BLK_IO_DEV  *IdeDev
  )
/*++
  Name:
        CheckErrorStatus


  Purpose: 
        This function is used to analyze the Status Register and print out 
        some debug information and if there is ERR bit set in the Status
        Register, the Error Register's value is also be parsed and print out.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

  Returns:  
        EFI_SUCCESS
          No err information in the Status Register.

        EFI_DEVICE_ERROR
          Any err information in the Status Register.

  Notes:
--*/
{
  UINT8   StatusRegister;

#ifdef EFI_DEBUG

  UINT8   ErrorRegister;

#endif

  StatusRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Status); 

  DEBUG_CODE (
    
    if (StatusRegister & DWF)  {
      DEBUG((EFI_D_BLKIO, "CheckErrorStatus()-- %02x : Error : Write Fault\n", 
             StatusRegister));
    }
    
    if (StatusRegister & CORR) {
      DEBUG((EFI_D_BLKIO,"CheckErrorStatus()-- %02x : Error : Corrected Data\n",
             StatusRegister));
    }
  
    if (StatusRegister & ERR) {
      ErrorRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg1.Error);
  
      if (ErrorRegister & BBK_ERR) {
        DEBUG((EFI_D_BLKIO, "CheckErrorStatus()-- %02x : Error : Bad Block Detected\n",
               ErrorRegister));
      }
      
      if (ErrorRegister & UNC_ERR) {
        DEBUG((EFI_D_BLKIO, "CheckErrorStatus()-- %02x : Error : Uncorrectable Data\n",
                ErrorRegister));
      }
      
      if (ErrorRegister & MC_ERR) {
        DEBUG((EFI_D_BLKIO, "CheckErrorStatus()-- %02x : Error : Media Change\n", 
                ErrorRegister));
      }
      
      if (ErrorRegister & ABRT_ERR) {
        DEBUG((EFI_D_BLKIO, "CheckErrorStatus()-- %02x : Error : Abort\n",
               ErrorRegister));
      }
      
      if (ErrorRegister & TK0NF_ERR) {
        DEBUG((EFI_D_BLKIO, "CheckErrorStatus()-- %02x : Error : Track 0 Not Found\n",
               ErrorRegister));
      }
      
      if (ErrorRegister & AMNF_ERR) {
        DEBUG((EFI_D_BLKIO, "CheckErrorStatus()-- %02x : Error : Address Mark Not Found\n",
              ErrorRegister));
      }
  
    }
  )

  if ( (StatusRegister & (ERR | DWF | CORR) ) == 0 ) {    
    return EFI_SUCCESS;  
  } 
  
  return EFI_DEVICE_ERROR;
  
}


EFI_STATUS
AtaReadSectors (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *DataBuffer, 
  IN  EFI_LBA         Lba, 
  IN  UINTN           NumberOfBlocks
  )
/*++
  Name:
        AtaReadSectors


  Purpose: 
        This function is called by the AtaBlkIoReadBlocks() to perform
        reading from media in block unit.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        VOID      IN    *DataBuffer
          A pointer to the destination buffer for the data. 

        EFI_LBA     IN    Lba
          The starting logical block address to read from 
          on the device media.

        UINTN     IN    NumberOfBlocks
          The number of transfer data blocks.

  Returns:  
        return status is fully dependent on the return status
        of AtaPioDataIn() function.

  Notes:
--*/  
{
  EFI_STATUS    Status;
  UINTN         BlocksRemaining;
  UINT32        Lba32;
  UINT8         Lba0, Lba1, Lba2, Lba3;
  UINT8         AtaCommand;
  UINT8         SectorCount8;
  UINT16        SectorCount;
  UINTN         ByteCount;
  VOID          *Buffer;
  
  Buffer = DataBuffer;

  //
  //Using ATA Read Sector(s) command (opcode=0x20) with PIO DATA IN protocol 
  //
  AtaCommand = READ_SECTORS_CMD;

  
  BlocksRemaining = NumberOfBlocks;
  
  Lba32 = (UINT32)Lba;

  Status = EFI_SUCCESS ;

  while (BlocksRemaining > 0) {
    
    //
    // in ATA-3 spec, LBA is in 28 bit width
    //
    Lba0 = (UINT8) (Lba32 & 0xff);
    Lba1 = (UINT8) (Lba32 >> 8);
    Lba2 = (UINT8) (Lba32 >> 16);
    //
    // low 4 bit of Lba3 stands for LBA bit24~bit27.
    //
    Lba3 = (UINT8) ((Lba32 >> 24) & 0x0f); 

    if (BlocksRemaining >= 0x100) {
      
      //
      //  SectorCount8 is sent to Sector Count register, 0x00 means 256 
      //  sectors to be read
      //
      SectorCount8 = 0x00 ;   
      //
      //  SectorCount is used to record the number of sectors to be read
      //
      SectorCount = 256 ;     
    } else {
      
      SectorCount8 = (UINT8)BlocksRemaining ;
      SectorCount = (UINT16)BlocksRemaining ;
    }

    //
    // ByteCount is the number of bytes that will be read
    //
    ByteCount = SectorCount * (IdeDev->BlkIo.Media->BlockSize) ;  

    //
    // call AtaPioDataIn() to send Read Sector Command and receive data read
    //
    Status = AtaPioDataIn(
          IdeDev, Buffer, (UINT32)ByteCount,
          AtaCommand, 
          Lba3, SectorCount8, Lba0, Lba1, Lba2
          );
    if (EFI_ERROR(Status)) {      
      return Status ;
    }

    Lba32 += SectorCount;
    Buffer = ((UINT8 *)Buffer + SectorCount*(IdeDev->BlkIo.Media->BlockSize));
    BlocksRemaining -= SectorCount;
  }

  return Status;
}



EFI_STATUS
AtaWriteSectors (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *BufferData, 
  IN  EFI_LBA         Lba, 
  IN  UINTN           NumberOfBlocks
  )
/*++
  Name:
        AtaWriteSectors


  Purpose: 
        This function is called by the AtaBlkIoWriteBlocks() to perform
        writing onto media in block unit.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure,used
          to record all the information of the IDE device.

        VOID      IN    *DataBuffer
          A pointer to the source buffer for the data. 

        EFI_LBA     IN    Lba
          The starting logical block address to write onto 
          the device media.

        UINTN     IN    NumberOfBlocks
          The number of transfer data blocks.


  Returns:  
        return status is fully dependent on the return status
        of AtaPioDataOut() function.

  Notes:
--*/  
{
  EFI_STATUS    Status;
  UINTN         BlocksRemaining;
  UINT32        Lba32;
  UINT8         Lba0, Lba1, Lba2, Lba3;
  UINT8         AtaCommand;
  UINT8         SectorCount8;
  UINT16        SectorCount;
  UINTN         ByteCount;
  VOID          *Buffer;

  Buffer = BufferData ;


  //
  //Using Write Sector(s) command (opcode=0x30) with PIO DATA OUT protocol 
  //
  AtaCommand = WRITE_SECTORS_CMD;

  BlocksRemaining = NumberOfBlocks;
  
  Lba32 = (UINT32)Lba;

  Status = EFI_SUCCESS ;      // init Status

  while (BlocksRemaining > 0)  {
    
    Lba0 = (UINT8) (Lba32 & 0xff);
    Lba1 = (UINT8) (Lba32 >> 8);
    Lba2 = (UINT8) (Lba32 >> 16);
    Lba3 = (UINT8) ((Lba32 >> 24) & 0x0f);

    if (BlocksRemaining >= 0x100) {
      
      //
      //  SectorCount8 is sent to Sector Count register, 0x00 means 256 sectors
      //  to be written
      //
      SectorCount8 = 0x00 ; 
      //
      //  SectorCount is used to record the number of sectors to be written
      //
      SectorCount = 256 ;   
    } else {
      
      SectorCount8 = (UINT8)BlocksRemaining ;
      SectorCount = (UINT16)BlocksRemaining ;
    }

    ByteCount = SectorCount * (IdeDev->BlkIo.Media->BlockSize);
    
    Status = AtaPioDataOut(
          IdeDev, Buffer, (UINT32)ByteCount,
          AtaCommand, 
          Lba3, SectorCount8, Lba0, Lba1, Lba2
           );
    if (EFI_ERROR(Status)) {      
      return Status ;
    }

    Lba32 += SectorCount;
    Buffer = ((UINT8 *)Buffer + SectorCount*(IdeDev->BlkIo.Media->BlockSize));
    BlocksRemaining -= SectorCount;
  }

  return Status;
}



EFI_STATUS
AtaSoftReset (
  IN  IDE_BLK_IO_DEV  *IdeDev
  )
/*++
  Name:
        AtaSoftReset

  Purpose: 
        This function is used to implement the Soft Reset on the specified
        device. But, the ATA Soft Reset mechanism is so strong a reset method 
        that it will force resetting on both devices connected to the 
        same cable.
        It is called by IdeBlkIoReset(), a interface function of Block
        I/O protocol.
        This function can also be used by the ATAPI device to perform reset when
        ATAPI Reset command is failed.
            
  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.
  Returns:  
        EFI_SUCCESS
          Soft reset completes successfully.

        EFI_DEVICE_ERROR
          Any step during the reset process is failed.
  Notes:
        The registers initial values after ATA soft reset are different
        to the ATA device and ATAPI device.
--*/  
{

  UINT8 DeviceControl;
    
  
  DeviceControl = 0;
  DeviceControl |= SRST;      // set SRST bit to initiate soft reset
  DeviceControl |= bit1;      // disable Interrupt
  
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Alt.DeviceControl, DeviceControl);
 
  gBS->Stall(10);     // Wait 10us 
 
  //
  // Clear SRST bit
  //
  DeviceControl &= 0xfb ;     // 0xfb:1111,1011
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Alt.DeviceControl, DeviceControl);

  //
  // slave device needs at most 31s to clear BSY 
  //
  if ( WaitForBSYClear (IdeDev, 31000) == EFI_TIMEOUT) {    
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
AtaBlkIoReadBlocks (
  IN IDE_BLK_IO_DEV   *IdeBlkIoDevice,
  IN UINT32           MediaId,
  IN EFI_LBA          LBA,
  IN UINTN            BufferSize,
  OUT VOID            *Buffer
  )
/*++
  Name:
      AtaBlkIoReadBlocks


  Purpose: 
      This function is the ATA implementation for ReadBlocks in the
      Block I/O Protocol interface.


  Parameters:
      IDE_BLK_IO_DEV    IN    *IdeBlkIoDevice
        Indicates the calling context.

      UINT32      IN    MediaId
        The media id that the read request is for.

      EFI_LBA     IN    LBA
        The starting logical block address to read from 
        on the device.

      UINTN     IN    BufferSize
        The size of the Buffer in bytes. This must be a
        multiple of the intrinsic block size of the device.

      VOID      OUT   *Buffer
        A pointer to the destination buffer for the data. 
        The caller is responsible for either having implicit
        or explicit ownership of the memory that data is read into.

  Returns:  
      EFI_SUCCESS 
        Read Blocks successfully.

      EFI_DEVICE_ERROR
        Read Blocks failed.

      EFI_NO_MEDIA
        There is no media in the device.

      EFI_MEDIA_CHANGE
        The MediaId is not for the current media.

      EFI_BAD_BUFFER_SIZE
        The BufferSize parameter is not a multiple of the 
        intrinsic block size of the device.

      EFI_INVALID_PARAMETER
        The read request contains LBAs that are not valid,
        or the data buffer is not valid.

  Notes:
      If Read Block error because of device error, this function will call
      AtaSoftReset() function to reset device.
--*/  
{
  EFI_BLOCK_IO_MEDIA  *Media;
  UINTN               BlockSize;        
  UINTN               NumberOfBlocks ;  
  EFI_STATUS          Status;


  if(!Buffer) {    
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {    
    return EFI_SUCCESS;
  }

  //
  //  Get the intrinsic block size
  //
  Media = IdeBlkIoDevice->BlkIo.Media;
  BlockSize = Media->BlockSize;

  NumberOfBlocks = BufferSize / BlockSize ;

  //
  //Check buffer alignment
  //
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }


  if (MediaId != Media->MediaId) {    
    return EFI_MEDIA_CHANGED;
  }

  if (BufferSize % BlockSize != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  if (!(Media->MediaPresent)) {
    return EFI_NO_MEDIA;
  }

  if (LBA > Media->LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {
    return EFI_INVALID_PARAMETER;
  }


  Status = AtaReadSectors ( IdeBlkIoDevice, Buffer, LBA, NumberOfBlocks ) ;
  if (EFI_ERROR(Status)) {
    AtaSoftReset (IdeBlkIoDevice);
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;

}

EFI_STATUS
AtaBlkIoWriteBlocks (
  IN IDE_BLK_IO_DEV   *IdeBlkIoDevice,
  IN UINT32           MediaId,
  IN EFI_LBA          LBA,
  IN UINTN            BufferSize,
  OUT VOID            *Buffer
  )
/*++
  Name:
        AtaBlkIoWriteBlocks


  Purpose: 
        This function is the ATA implementation for WriteBlocks in the
        Block I/O Protocol interface.

  Parameters:
        IDE_BLK_IO_DEV   IN    *IdeBlkIoDevice
          Indicates the calling context.

        UINT32      IN    MediaId
          The media id that the write request is for.

        EFI_LBA     IN    LBA
          The starting logical block address to write onto 
          the device.

        UINTN     IN    BufferSize
          The size of the Buffer in bytes. This must be a
          multiple of the intrinsic block size of the device.

        VOID      OUT   *Buffer
          A pointer to the source buffer for the data. 
          The caller is responsible for either having implicit
          or explicit ownership of the memory that data is 
          written from.


  Returns:  
        EFI_SUCCESS 
          Write Blocks successfully.

        EFI_DEVICE_ERROR
          Write Blocks failed.

        EFI_NO_MEDIA
          There is no media in the device.

        EFI_MEDIA_CHANGE
          The MediaId is not for the current media.

        EFI_BAD_BUFFER_SIZE
          The BufferSize parameter is not a multiple of the 
          intrinsic block size of the device.

        EFI_INVALID_PARAMETER
          The write request contains LBAs that are not valid,
          or the data buffer is not valid.

  Notes:
        If Write Block error because of device error, this function will call
        AtaSoftReset() function to reset device.
--*/
{

  EFI_BLOCK_IO_MEDIA  *Media;
  UINTN               BlockSize;        
  UINTN               NumberOfBlocks; 
  UINTN               Status;


  if(!Buffer) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }

  //
  //Get the intrinsic block size
  //
  Media = IdeBlkIoDevice->BlkIo.Media;
  BlockSize = Media->BlockSize;
  NumberOfBlocks = BufferSize/BlockSize;

  //
  //Check buffer alignment
  //
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }

  
  if (!(Media->MediaPresent)) {
    return EFI_NO_MEDIA;
  }

  if (MediaId != Media->MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  if (Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  if (LBA > Media->LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize % BlockSize != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  Status = AtaWriteSectors ( IdeBlkIoDevice, Buffer, LBA, NumberOfBlocks ) ;
  if (EFI_ERROR(Status)) {
    AtaSoftReset(IdeBlkIoDevice);
    return EFI_DEVICE_ERROR;
  }
  
  return EFI_SUCCESS;
}
