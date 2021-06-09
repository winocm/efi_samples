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

  IsaFloppyCtrl.c

Abstract:

  ISA Floppy Driver
  1. Support two types diskette drive  
     1.44M drive and 2.88M drive (and now only support 1.44M)
  2. Support two diskette drives
  3. Use DMA channel 2 to transfer data
  4. Do not use interrupt
  5. Support diskette change line signal and write protect
  
  The internal function for the floppy driver

Revision History:

--*/

#include "IsaFloppy.h"

EFI_STATUS
DiscoverFddDevice (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description:  Detect the floppy drive is presented or not   
  Parameters:
    FdcDev FDC_BLK_IO_DEV * : A pointer to the Data Structure FDC_BLK_IO_DEV       
  Returns:
    EFI_SUCCESS    Drive is presented 
    EFI_NOT_FOUND  Drive is not presented

--*/
{
  EFI_STATUS  Status;

  FdcDev->BlkIo.Media = &FdcDev->BlkMedia;
  
  //
  // Call FddIndentify subroutine
  //
  Status = FddIdentify (FdcDev);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  FdcDev->BlkIo.Reset               = FdcReset;
  FdcDev->BlkIo.FlushBlocks         = FddFlushBlocks;
  FdcDev->BlkIo.ReadBlocks          = FddReadBlocks;
  FdcDev->BlkIo.WriteBlocks         = FddWriteBlocks;
  FdcDev->BlkMedia.LogicalPartition = FALSE;
  FdcDev->BlkMedia.WriteCaching     = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
FddIdentify (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description:   Do recalibrate  and see the drive is presented or not
         Set the media parameters
  Parameters:
    FdcDev FDC_BLK_IO_DEV * : A pointer to the Data Structure FDC_BLK_IO_DEV       
  Returns:
    EFI_SUCCESS:    
    EFI_DEVICE_ERROR: 

--*/
{
  EFI_STATUS         Status;

  //
  // Set Floppy Disk Controller's motor on
  //
  Status = MotorOn (FdcDev);
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR; 
  }

  Status = Recalibrate(FdcDev);

  if (EFI_ERROR(Status)) {
    MotorOff (FdcDev);
    FdcDev->ControllerState->NeedRecalibrate = TRUE;
    return EFI_DEVICE_ERROR;
  }

  //
  // Set Media Parameter
  //
  FdcDev->BlkIo.Media->RemovableMedia = TRUE;
  FdcDev->BlkIo.Media->MediaPresent   = TRUE; //investigate
  FdcDev->BlkIo.Media->MediaId        = 0;

  //
  // Check Media
  //
  Status = DisketChanged (FdcDev);
  switch (Status) {
    case EFI_NO_MEDIA :
      FdcDev->BlkIo.Media->MediaPresent = FALSE;
      break;
    case EFI_MEDIA_CHANGED :
    case EFI_SUCCESS :
      break;
    default :
      MotorOff (FdcDev);
      return Status;
  }

  //
  // Check Disk Write Protected
  //
  Status = SenseDrvStatus (FdcDev, 0);
  switch (Status) {
    case EFI_WRITE_PROTECTED :
      FdcDev->BlkIo.Media->ReadOnly = TRUE;
      break;
    case EFI_SUCCESS:
      FdcDev->BlkIo.Media->ReadOnly = FALSE;
      break;
    default :
      return EFI_DEVICE_ERROR;
      break;
  }

  MotorOff (FdcDev);

  //
  // Set Media Default Type
  //
  FdcDev->BlkIo.Media->BlockSize = DISK_1440K_BYTEPERSECTOR;
  FdcDev->BlkIo.Media->LastBlock = DISK_1440K_EOT * 2 * (DISK_1440K_MAXTRACKNUM + 1) - 1;

  return EFI_SUCCESS;   
}

EFI_STATUS
FddReset (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description:  Reset the Floppy Logic Drive   
  Parameters:
    FdcDev FDC_BLK_IO_DEV * : A pointer to the Data Structure FDC_BLK_IO_DEV       
  Returns:
    EFI_SUCCESS:    The Floppy Logic Drive is reset
    EFI_DEVICE_ERROR: The Floppy Logic Drive is not functioning correctly and 
                      can not be reset

--*/
{
  UINT8  data;
  UINT8  StatusRegister0;
  UINT8  PresentCylinderNumber;
  UINTN  Index;
  
  //
  // Reset specified Floppy Logic Drive according to FdcDev -> Disk
  // Set Digital Output Register(DOR) to do reset work
  //   bit0 & bit1 of DOR : Drive Select
  //   bit2 : Reset bit
  //   bit3 : DMA and Int bit
  // Reset : a "0" written to bit2 resets the FDC, this reset will remain 
  //         active until 
  //         a "1" is written to this bit. 
  // Reset step 1:
  //         use bit0 & bit1 to  select the logic drive
  //         write "0" to bit2   
  //
  data = 0x0;
  data |= (SELECT_DRV & FdcDev->Disk);
  FdcWritePort (FdcDev, FDC_REGISTER_DOR, data);

  //
  // wait some time,at least 120us
  //
  gBS->Stall(500);

  //
  // Reset step 2:
  //   write "1" to bit2
  //   write "1" to bit3 : enable DMA 
  //
  data |= 0x0C;
  FdcWritePort (FdcDev, FDC_REGISTER_DOR, data);
  
  //
  // Experience value
  //
  gBS->Stall(2000);
  
  //
  // wait specified floppy logic drive is not busy
  //
  if (EFI_ERROR(FddWaitForBSYClear (FdcDev, 1))) {
    return EFI_DEVICE_ERROR;
  }   
  
  //
  // Set the Transfer Data Rate
  //
  FdcWritePort (FdcDev, FDC_REGISTER_CCR, 0x0);
  
  //
  // Experience value
  //
  gBS->Stall(900);
  
  //
  // Issue Sense interrupt command for each drive (total 4 drives)
  //
  for (Index = 0; Index < 4; Index++) {
    if (EFI_ERROR(SenseIntStatus (FdcDev, &StatusRegister0, &PresentCylinderNumber))) {
      return EFI_DEVICE_ERROR;
    }
  }

  //
  // issue Specify command
  //
  if (EFI_ERROR(Specify (FdcDev))) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
MotorOn (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description:  Turn the drive's motor on
        The drive's motor must be on before any command can be executed   
  Parameters:
    FdcDev FDC_BLK_IO_DEV * : A pointer to the Data Structure FDC_BLK_IO_DEV       
  Returns:
    EFI_SUCCESS:       Turn the drive's motor on successfully
    EFI_DEVICE_ERROR:    The drive is busy, so can not turn motor on 
    EFI_INVALID_PARAMETER: Fail to Set timer(Cancel timer)  

--*/
{
  EFI_STATUS  Status;
  UINT8       data;
  
  //
  // Control of the floppy drive motors is a big pain. If motor is off, you have
  // to turn it on first. But you can not leave the motor on all the time, since
  // that would wear out the disk. On the other hand, if you turn the motor off 
  // after each operation, the system performance will be awful. The compromise 
  // used in this driver is to leave the motor on for 2 seconds after 
  // each operation. If a new operation is started in that interval(2s), 
  // the motor need not be turned on again. If no new operation is started, 
  // a timer goes off and the motor is turned off
  //
  
  //
  // Cancel the timer 
  //
  Status = gBS->SetTimer (FdcDev->Event, TimerCancel, 0 );
    
  if (EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }
    
  //
  // Get the motor status
  //
  data = FdcReadPort (FdcDev, FDC_REGISTER_DOR);
  
  if (((FdcDev->Disk == FDC_DISK0) && ((data & 0x10) == 0x10))
      || ((FdcDev->Disk == FDC_DISK1) && ((data & 0x21) == 0x21))) {
    return EFI_SUCCESS;
  }  
  
  //
  //The drive's motor is off, so need turn it on
  //first look at command and drive are busy or not 
  //
  if (EFI_ERROR(FddWaitForBSYClear (FdcDev, 1))) {
    return EFI_DEVICE_ERROR;
  }
  
  //
  // for drive A: 1CH, drive B: 2DH   
  //
  data = 0x0C;
  data |= (SELECT_DRV & FdcDev->Disk);
  if ( FdcDev->Disk == FDC_DISK0 ) {//drive A
    data |= DRVA_MOTOR_ON;
  } else {  //drive B
    data |= DRVB_MOTOR_ON;
  }
  
  FdcWritePort (FdcDev, FDC_REGISTER_DOR, data);
  
  //
  // Experience value
  //
  gBS->Stall(4000);

  return EFI_SUCCESS;
}


EFI_STATUS
MotorOff (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description:  Set a Timer and when Timer goes off, turn the motor off
  Parameters:
    FdcDev FDC_BLK_IO_DEV * : A pointer to the Data Structure FDC_BLK_IO_DEV       
  Returns:
    EFI_SUCCESS:       Set the Timer successfully
    EFI_INVALID_PARAMETER: Fail to Set the timer  

--*/
{
  //
  // Set the timer : 2s
  //
  return gBS->SetTimer (FdcDev->Event, TimerRelative, 20000000);
}


EFI_STATUS
DisketChanged ( 
  IN FDC_BLK_IO_DEV  *FdcDev  
  )
/*++

  Routine Description:  Detect the disk in the drive is changed or not
  Parameters:
    FdcDev FDC_BLK_IO_DEV *: A pointer to Data Structure FDC_BLK_IO_DEV   
  Returns:
    EFI_SUCCESS:    No disk media change
    EFI_DEVICE_ERROR: Fail to do the recalibrate or seek operation
    EFI_NO_MEDIA:   No disk in the drive
    EFI_MEDIA_CHANGED:  There is a new disk in the drive

--*/
{
  EFI_STATUS  Status;
  UINT8       data;
  
  //
  // Check change line
  //
  data = FdcReadPort (FdcDev, FDC_REGISTER_DIR);
  
  //
  // Io delay
  //
  gBS->Stall(50);
  
  if ((data & DIR_DCL) == 0x80) { //disk change line is active
    if ( FdcDev->PresentCylinderNumber != 0 ) {
      Status = Recalibrate (FdcDev);
    } else {
      Status = Seek(FdcDev, 0x30);
    }

    if (EFI_ERROR(Status)) {
      FdcDev->ControllerState->NeedRecalibrate = TRUE;
      return EFI_DEVICE_ERROR; //Fail to do the seek or recalibrate operation
    }

    data = FdcReadPort (FdcDev, FDC_REGISTER_DIR);
    
    //
    // Io delay
    //
    gBS->Stall (50);

    if ((data & DIR_DCL) == 0x80) {
      return EFI_NO_MEDIA;
    }

    return EFI_MEDIA_CHANGED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Specify (
  IN FDC_BLK_IO_DEV  *FdcDev  
  )
/*++

  Routine Description:  Do the Specify command, this command sets DMA operation
                        and the initial values for each of the three internal 
                        times: HUT, SRT and HLT
  Parameters:
    None
  Returns:
    EFI_SUCCESS:    Execute the Specify command successfully
    EFI_DEVICE_ERROR: Fail to execute the command

--*/
{
  FDD_SPECIFY_CMD  Command;
  UINTN            Index;
  UINT8            *CommandPointer;

  EfiZeroMem (&Command, sizeof (FDD_SPECIFY_CMD));
  Command.CommandCode = SPECIFY_CMD;
  //
  //set SRT, HUT
  //
  Command.SrtHut = 0xdf;//0xdf;
  //
  //set HLT and DMA
  //
  Command.HltNd = 0x02;

  CommandPointer = (UINT8*)(&Command);
  for (Index = 0; Index < sizeof (FDD_SPECIFY_CMD); Index++) {
    if (EFI_ERROR(DataOutByte (FdcDev, CommandPointer++))) {
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Recalibrate (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description:  Set the head of floppy drive to track 0
  Parameters:
    FdcDev FDC_BLK_IO_DEV *: A pointer to Data Structure FDC_BLK_IO_DEV
  Returns:
    EFI_SUCCESS:    Execute the Recalibrate operation successfully
    EFI_DEVICE_ERROR: Fail to execute the Recalibrate operation

--*/
{
  FDD_COMMAND_PACKET2  Command;
  UINTN                Index;
  UINT8                StatusRegister0;
  UINT8                PresentCylinderNumber;
  UINT8                *CommandPointer;
  UINT8                Count;
    
  Count = 2;
  
  while (Count > 0) {
    EfiZeroMem (&Command, sizeof (FDD_COMMAND_PACKET2));
    Command.CommandCode = RECALIBRATE_CMD;
    //
    //drive select
    //
    if (FdcDev->Disk == FDC_DISK0) {
      Command.DiskHeadSel = 0; //0
    } else {
      Command.DiskHeadSel = 1; //1
    }

    CommandPointer = (UINT8*)(&Command);
    for ( Index = 0; Index < sizeof (FDD_COMMAND_PACKET2); Index++) {
      if (EFI_ERROR(DataOutByte (FdcDev, CommandPointer++))) {
        return EFI_DEVICE_ERROR;
      }
    }

    //
    // Experience value
    //
    gBS->Stall (250000); //need modify according to 1.44M or 2.88M

    if (EFI_ERROR(SenseIntStatus (FdcDev, &StatusRegister0, &PresentCylinderNumber))) {
      return EFI_DEVICE_ERROR;
    }
    
    if ((StatusRegister0 & 0xf0) == 0x20 && PresentCylinderNumber == 0) {
      FdcDev->PresentCylinderNumber = 0;
      FdcDev->ControllerState->NeedRecalibrate = FALSE;
      return EFI_SUCCESS;
    } else {
      Count -- ;
      if (Count == 0) {
        return EFI_DEVICE_ERROR;
      }
    }
  } //end while

  return EFI_SUCCESS;
}

EFI_STATUS
Seek (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN EFI_LBA         Lba
  )
/*++

  Routine Description:  Set the head of floppy drive to the new cylinder
  Parameters:
    FdcDev FDC_BLK_IO_DEV *: A pointer to Data Structure FDC_BLK_IO_DEV
    Lba EFI_LBA     : The logic block address want to seek
  Returns:
    EFI_SUCCESS:    Execute the Seek operation successfully
    EFI_DEVICE_ERROR: Fail to execute the Seek operation

--*/
{
  FDD_SEEK_CMD        Command;
  UINT8               EndOfTrack;
  UINT8               Head;
  UINT8               Cylinder;
  UINT8               StatusRegister0;
  UINT8               *CommandPointer;
  UINT8               PresentCylinderNumber;
  UINTN               Index;
  UINT8               DelayTime;
    
  if (FdcDev->ControllerState->NeedRecalibrate == TRUE) {
    if (EFI_ERROR(Recalibrate (FdcDev))) {
      FdcDev->ControllerState->NeedRecalibrate = TRUE;
      return EFI_DEVICE_ERROR;
    }
  }
  
  EndOfTrack = DISK_1440K_EOT;
  //
  // Calculate cylinder based on Lba and EOT
  //
  Cylinder = (UINT8)((UINTN)Lba / EndOfTrack / 2);
  
  //
  // if the destination cylinder is the present cylinder, unnecessary to do the
  // seek operation
  //
  if (FdcDev->PresentCylinderNumber == Cylinder) {
    return EFI_SUCCESS;
  }

  //
  // Calculate the head : 0 or 1
  //
  Head = (UINT8)((UINTN)Lba / EndOfTrack % 2);
  
  EfiZeroMem (&Command, sizeof (FDD_SEEK_CMD));
  Command.CommandCode = SEEK_CMD;
  if (FdcDev->Disk == FDC_DISK0) {
    Command.DiskHeadSel = 0; //0
  } else {
    Command.DiskHeadSel = 1; //1
  }

  Command.DiskHeadSel |= Head << 2;
  Command.NewCylinder = Cylinder;

  CommandPointer = (UINT8*)(&Command);
  for ( Index = 0; Index < sizeof (FDD_SEEK_CMD); Index++ ) {
    if (EFI_ERROR(DataOutByte (FdcDev, CommandPointer++))) {
      return EFI_DEVICE_ERROR;
    }
  }

  //
  // Io delay
  //
  gBS->Stall (100);
  
  //
  // Calculate waiting time  
  //
  if (FdcDev->PresentCylinderNumber > Cylinder) {
    DelayTime = (UINT8)(FdcDev->PresentCylinderNumber - Cylinder);
  } else {
    DelayTime = (UINT8)(Cylinder - FdcDev->PresentCylinderNumber);
  }

  gBS->Stall ((DelayTime + 1) * 4000);  

  if (EFI_ERROR(SenseIntStatus (FdcDev, &StatusRegister0, &PresentCylinderNumber))) {
    return EFI_DEVICE_ERROR;
  }

  if ((StatusRegister0 & 0xf0) == 0x20) {
    FdcDev->PresentCylinderNumber = Command.NewCylinder;
    return EFI_SUCCESS;
  } else {
    FdcDev->ControllerState->NeedRecalibrate = TRUE;
    return EFI_DEVICE_ERROR;
  }
}

EFI_STATUS
SenseIntStatus (
  IN     FDC_BLK_IO_DEV  *FdcDev,
  IN OUT UINT8           *StatusRegister0,
  IN OUT UINT8           *PresentCylinderNumber
  )
/*++

  Routine Description:  Do the Sense Interrupt Status command, this command 
                        resets the interrupt signal
  Parameters:
    StatusRegister0 UINT8 *: Be used to save Status Register 0 read from FDC   
    PresentCylinderNumber  UINT8 *: Be used to save present cylinder number 
                                    read from FDC
  Returns:
    EFI_SUCCESS:    Execute the Sense Interrupt Status command successfully
    EFI_DEVICE_ERROR: Fail to execute the command

--*/
{
  UINT8   command;

  command = SENSE_INT_STATUS_CMD;
  if (EFI_ERROR(DataOutByte (FdcDev, &command))) {
    return EFI_DEVICE_ERROR;
  }

  if (EFI_ERROR(DataInByte (FdcDev, StatusRegister0))) {
    return EFI_DEVICE_ERROR;
  }

  if (EFI_ERROR(DataInByte (FdcDev, PresentCylinderNumber))) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SenseDrvStatus (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN EFI_LBA         Lba
  )
/*++

  Routine Description:  Do the Sense Drive Status command
  Parameters:
    FdcDev FDC_BLK_IO_DEV *: A pointer to Data Structure FDC_BLK_IO_DEV   
    Lba EFI_LBA     : Logic block address
  Returns:
    EFI_SUCCESS:    Execute the Sense Drive Status command successfully
    EFI_DEVICE_ERROR: Fail to execute the command
    EFI_WRITE_PROTECTED:The disk is write protected 

--*/
{
  FDD_COMMAND_PACKET2  Command;
  UINT8                Head;
  UINT8                EndOfTrack;
  UINTN                Index;
  UINT8                StatusRegister3;
  UINT8                *CommandPointer;
    
  //
  // Sense Drive Status command obtains drive status information,
  // it has not execution phase and goes directly to the result phase from the 
  // command phase, Status Register 3 contains the drive status information
  //
  
  EfiZeroMem (&Command, sizeof (FDD_COMMAND_PACKET2));
  Command.CommandCode = SENSE_DRV_STATUS_CMD;
  
  if (FdcDev->Disk == FDC_DISK0) {
    Command.DiskHeadSel = 0; 
  } else {
    Command.DiskHeadSel = 1; 
  }

  EndOfTrack = DISK_1440K_EOT;
  Head = (UINT8)((UINTN)Lba / EndOfTrack % 2);
  Command.DiskHeadSel |= Head << 2;
  
  CommandPointer = (UINT8*)(&Command);
  for ( Index = 0; Index < sizeof (FDD_COMMAND_PACKET2); Index++ ) {
    if (EFI_ERROR(DataOutByte (FdcDev, CommandPointer++))) {
      return EFI_DEVICE_ERROR;
    }
  }
  
  if (EFI_ERROR(DataInByte (FdcDev, &StatusRegister3))) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Io delay
  //
  gBS->Stall (50);
  
  //
  // Check Status Register 3 to get drive status information
  //
  return CheckStatus3 (StatusRegister3);
}

EFI_STATUS
DetectMedia (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description:  Update the disk media properties and if necessary 
                        reinstall Block I/O interface
  Parameters:
    FdcDev FDC_BLK_IO_DEV *: A pointer to Data Structure FDC_BLK_IO_DEV   
  Returns:
    EFI_SUCCESS:    Do the operation successfully
    EFI_DEVICE_ERROR: Fail to the operation

--*/
{
  EFI_STATUS  Status;
  BOOLEAN     bReset;
  BOOLEAN     bReadOnlyLastTime;
  BOOLEAN     bMediaPresentLastTime;
  
  bReset                = FALSE;
  bReadOnlyLastTime     = FdcDev->BlkIo.Media->ReadOnly;
  bMediaPresentLastTime = FdcDev->BlkIo.Media->MediaPresent;

  //
  // Check disk change
  //
  Status = DisketChanged (FdcDev);
  switch (Status) {
    case EFI_MEDIA_CHANGED :
      FdcDev->BlkIo.Media->MediaId++;
      FdcDev->BlkIo.Media->MediaPresent = TRUE;
      bReset = TRUE;    
      break;
            
    case EFI_NO_MEDIA :
      FdcDev->BlkIo.Media->MediaPresent = FALSE;
      break;
            
    case EFI_SUCCESS :
      break;
        
    default :
      MotorOff (FdcDev);
      return Status; //EFI_DEVICE_ERROR
  }
    
  if (FdcDev->BlkIo.Media->MediaPresent) {
    //
    // Check disk write protected
    //
    Status = SenseDrvStatus (FdcDev, 0);
    if (Status == EFI_WRITE_PROTECTED) {
      FdcDev->BlkIo.Media->ReadOnly = TRUE;
    } else {
      FdcDev->BlkIo.Media->ReadOnly = FALSE;
    }
  }
  
  if (FdcDev->BlkIo.Media->MediaPresent && (bReadOnlyLastTime != FdcDev->BlkIo.Media->ReadOnly)) {
    bReset = TRUE;
  }
  
  if (bMediaPresentLastTime != FdcDev->BlkIo.Media->MediaPresent) {
    bReset = TRUE;
  }

  if (bReset == TRUE) {
    Status = gBS->ReinstallProtocolInterface (
                    FdcDev->Handle, 
                    &gEfiBlockIoProtocolGuid, &FdcDev->BlkIo, &FdcDev->BlkIo
                    );

    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
Setup (
  IN FDC_BLK_IO_DEV  *FdcDev
  )
/*++

  Routine Description: Set the data rate and so on
  Parameters:
    None  
  Returns:
    EFI_SUCCESS:  

--*/
{
  EFI_STATUS Status;
  
  //
  // Set data rate 500kbs
  //
  FdcWritePort (FdcDev, FDC_REGISTER_CCR, 0x0);
  
  //
  // Io delay
  //
  gBS->Stall (50);
  
  Status = Specify (FdcDev);
  
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  return EFI_SUCCESS;
}



EFI_STATUS
ReadWriteDataSector(
  IN  FDC_BLK_IO_DEV  *FdcDev,
  IN  VOID            *HostAddress, 
  IN  EFI_LBA         Lba, 
  IN  UINTN           NumberOfBlocks,
  IN  BOOLEAN         Read
  )
/*++

  Routine Description: Read or Write a number of blocks in the same cylinder
  Parameters:
    FdcDev FDC_BLK_IO_DEV * : A pointer to Data Structure FDC_BLK_IO_DEV
    Buffer VOID *:
    Lba EFI_LBA:
    NumberOfBlocks UINTN:
    Read BOOLEAN:     
  Returns:
    EFI_SUCCESS:  

--*/
{
  EFI_STATUS                     Status;
  FDD_COMMAND_PACKET1            Command;
  FDD_RESULT_PACKET              Result;
  UINTN                          Index;
  UINTN                          Times;
  UINT8                          *CommandPointer;  
    
  EFI_PHYSICAL_ADDRESS           DeviceAddress;
  EFI_ISA_IO_PROTOCOL            *IsaIo;
  UINTN                          NumberofBytes;
  VOID                           *Mapping;
  EFI_ISA_IO_PROTOCOL_OPERATION  Operation;
  VOID                           *Buffer; 
  EFI_STATUS                     Status1;     

  Status = Seek (FdcDev, Lba);
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Map Dma
  //
  IsaIo = FdcDev->IsaIo;
  NumberofBytes = NumberOfBlocks * 512;
  if (Read == READ) {
    Operation = EfiIsaIoOperationSlaveWrite;
  } else {
    Operation = EfiIsaIoOperationSlaveRead;
  }
  
  Status1 = IsaIo->Map (
                     IsaIo,
                     Operation,
                     2,
                     EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_COMPATIBLE | EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_8 | EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SINGLE_MODE,
                     HostAddress,
                     &NumberofBytes,
                     &DeviceAddress,
                     &Mapping
                     );
  if (EFI_ERROR(Status1)) {
    return Status1;
  }
  Buffer = (UINT8*)(UINTN)DeviceAddress;                     
  
  //
  //Allocate Read or Write command packet
  //
  EfiZeroMem (&Command, sizeof (FDD_COMMAND_PACKET1));
  if (Read == READ) {
    Command.CommandCode = READ_DATA_CMD | CMD_MT | CMD_MFM | CMD_SK;
  } else {
    Command.CommandCode = WRITE_DATA_CMD | CMD_MT | CMD_MFM;
  }
  
  FillPara (FdcDev, Lba, &Command);
  
  //
  //Write command bytes to FDC
  //
  CommandPointer = (UINT8*)(&Command);
  for ( Index = 0; Index < sizeof (FDD_COMMAND_PACKET1); Index++) {
    if (EFI_ERROR(DataOutByte (FdcDev, CommandPointer++))) {
      return EFI_DEVICE_ERROR;
    }
  }

  //
  // wait for some time 
  //
  
  Times = (STALL_1_SECOND / 50) + 1;
  do  {
    if ((FdcReadPort (FdcDev, FDC_REGISTER_MSR) & 0xc0) == 0xc0) {
      break ;
    }
    
    gBS->Stall (50);
    Times = Times - 1;
  } while (Times);
    
  if (Times == 0) {
    return EFI_TIMEOUT;
  }
  
  //
  //Read result bytes from FDC
  //
  CommandPointer = (UINT8*)(&Result);
  for( Index = 0; Index < sizeof(FDD_RESULT_PACKET); Index++) {
    if (EFI_ERROR(DataInByte (FdcDev, CommandPointer++))) {
      return EFI_DEVICE_ERROR; 
    }
  } 
  
  //
  // Unmap Dma
  //
  Status1 = IsaIo->Unmap (IsaIo, Mapping);
  if (EFI_ERROR(Status1)) {
    return Status1;
  }
  
  return CheckResult(Result,FdcDev);
} 
  
VOID
FillPara(
  IN  FDC_BLK_IO_DEV       *FdcDev,
  IN  EFI_LBA              Lba,
  IN  FDD_COMMAND_PACKET1  *Command
  )
/*++

  Routine Description: Fill in Parameter
  Parameters:
  Returns:
    
--*/  
{
  UINT8              EndOfTrack;

  //
  // Get EndOfTrack from the Para table
  //
  EndOfTrack = DISK_1440K_EOT;

  //
  // Fill the command parameter
  //
  if (FdcDev->Disk == FDC_DISK0) {
    Command->DiskHeadSel = 0;
  } else {
    Command->DiskHeadSel = 1;
  }

  Command->Cylinder     = (UINT8)((UINTN)Lba / EndOfTrack / 2);
  Command->Head         = (UINT8)((UINTN)Lba / EndOfTrack % 2);
  Command->Sector       = (UINT8)((UINT8)((UINTN)Lba % EndOfTrack) + 1);
  Command->DiskHeadSel |= Command->Head << 2;
  Command->Number       = DISK_1440K_NUMBER;
  Command->EndOfTrack   = DISK_1440K_EOT;
  Command->GapLength    = DISK_1440K_GPL;
  Command->DataLength   = DISK_1440K_DTL;
}

EFI_STATUS
DataInByte (
  IN     FDC_BLK_IO_DEV  *FdcDev,
  IN OUT UINT8           *Pointer
  )
/*++

  Routine Description:  Read result byte from Data Register of FDC
  Parameters:
    Pointer UINT8 *: Be used to save result byte read from FDC   
  Returns:
    EFI_SUCCESS:    Read result byte from FDC successfully
    EFI_DEVICE_ERROR: The FDC is not ready to be read

--*/
{
  UINT8  data;
    
  //
  // wait for 1ms and detect the FDC is ready to be read
  //
  if (EFI_ERROR(FddDRQReady (FdcDev, DATA_IN, 1)))  {
    return EFI_DEVICE_ERROR; //is not ready
  }
  
  data = FdcReadPort (FdcDev, FDC_REGISTER_DTR);
  
  //
  // Io delay
  //
  gBS->Stall (50);
  
  *Pointer = data;
  return EFI_SUCCESS; 
}

EFI_STATUS
DataOutByte(
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINT8           *Pointer
  )
/*++

  Routine Description:  Write command byte to Data Register of FDC
  Parameters:
    Pointer UINT8 *: Be used to save command byte written to FDC   
  Returns:
    EFI_SUCCESS:    Write command byte to FDC successfully
    EFI_DEVICE_ERROR: The FDC is not ready to be written

--*/
{
  UINT8  data;
  
  //
  // wait for 1ms and detect the FDC is ready to be written
  //
  if (EFI_ERROR(FddDRQReady(FdcDev, DATA_OUT, 1)))  {
    return EFI_DEVICE_ERROR; //is not ready
  }
  
  data = *Pointer;

  FdcWritePort (FdcDev, FDC_REGISTER_DTR, data);
  
  //
  // Io delay
  //
  gBS->Stall (50);
  
  return EFI_SUCCESS;
}

EFI_STATUS
FddWaitForBSYClear (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINTN           TimeoutInSeconds
  )
/*++

  Routine Description:  Detect the specified floppy logic drive is busy or 
                        not within a period of time
  Parameters:
    Disk EFI_FDC_DISK:    Indicate it is drive A or drive B
    TimeoutInSeconds UINTN: the time period for waiting   
  Returns:
    EFI_SUCCESS:  The drive and command are not busy
    EFI_TIMEOUT:  The drive or command is still busy after a period time that 
                  set by TimeoutInSeconds

--*/
{
  UINTN  Delay; 
  UINT8  StatusRegister;
  UINT8  Mask;
    
  //
  // How to determine drive and command are busy or not: by the bits of 
  // Main Status Register
  // bit0: Drive 0 busy (drive A)
  // bit1: Drive 1 busy (drive B)
  // bit4: Command busy
  //
    
  //
  // set mask: for drive A set bit0 & bit4; for drive B set bit1 & bit4
  //
  Mask = (UINT8)((FdcDev->Disk == FDC_DISK0 ? MSR_DAB : MSR_DBB) | MSR_CB);
  
  Delay = ((TimeoutInSeconds * STALL_1_MSECOND) / 50) + 1;
  do {
    StatusRegister = FdcReadPort (FdcDev, FDC_REGISTER_MSR);  
    if ((StatusRegister & Mask) == 0x00) {
      break; // not busy
    }

    gBS->Stall (50);
    Delay = Delay - 1;
  } while (Delay);
  
  if (Delay == 0)  {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS  
FddDRQReady (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN BOOLEAN         Dio,
  IN  UINTN          TimeoutInSeconds
  )
/*++

  Routine Description:  Determine whether FDC is ready to write or read
  Parameters:
    Dio BOOLEAN:      Indicate the FDC is waiting to write or read
    TimeoutInSeconds UINTN: The time period for waiting   
  Returns:
    EFI_SUCCESS:  FDC is ready to write or read
    EFI_NOT_READY:  FDC is not ready within the specified time period

--*/
{
  UINTN  Delay; 
  UINT8  StatusRegister;
  UINT8  DataInOut;
  
  //
  //Before writing to FDC or reading from FDC, the Host must examine 
  //the bit7(RQM) and bit6(DIO) of the Main Status Register.
  //That is to say:
  //  command bytes can not be written to Data Register 
  //  unless RQM is 1 and DIO is 0
  //  result bytes can not be read from Data Register 
  //  unless RQM is 1 and DIO is 1
  //

  DataInOut = (UINT8)(Dio << 6); //in order to compare bit6
  Delay = ((TimeoutInSeconds * STALL_1_MSECOND) / 50) + 1;
  do {
    StatusRegister = FdcReadPort (FdcDev, FDC_REGISTER_MSR);  
    if ((StatusRegister & MSR_RQM) == MSR_RQM && (StatusRegister & MSR_DIO) == DataInOut) {
      break; //FDC is ready
    }

    gBS->Stall (50);   // Stall for 50 us
    Delay = Delay - 1;
  } while (Delay);
    
  if (Delay == 0) {
    return EFI_NOT_READY;//FDC is not ready within the specified time period
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
CheckResult (
  IN     FDD_RESULT_PACKET  Result,
  IN OUT FDC_BLK_IO_DEV     *FdcDev
  )
{
  //
  // Check Status Register0
  //
  if ((Result.Status0 & STS0_IC) != IC_NT) {
    if ((Result.Status0 & STS0_SE) == 0x20) {
      //seek error
      FdcDev->ControllerState->NeedRecalibrate = TRUE;
    }

    FdcDev->ControllerState->NeedRecalibrate = TRUE;
    return EFI_DEVICE_ERROR;
  }
    
  //
  // Check Status Register1
  //
  if (Result.Status1 & (STS1_EN | STS1_DE | STS1_OR | STS1_ND | STS1_NW | STS1_MA)) {
    FdcDev->ControllerState->NeedRecalibrate = TRUE;
    return EFI_DEVICE_ERROR;
  }

  //
  // Check Status Register2
  //
  if( Result.Status2 & ( STS2_CM | STS2_DD | STS2_WC | STS2_BC | STS2_MD ) ) {
    FdcDev->ControllerState->NeedRecalibrate = TRUE;
    return EFI_DEVICE_ERROR;
  } 
  
  return EFI_SUCCESS;
}

EFI_STATUS
CheckStatus3 (
  IN UINT8 StatusRegister3
  )
/*++

  Routine Description:  Check the drive status information
  Parameters:
    StatusRegister3 UINT8: the value of Status Register 3    
  Returns:
    EFI_SUCCESS:     
    EFI_WRITE_PROTECTED:  The disk is write protected

--*/
{
  if ( StatusRegister3 & STS3_WP ) {
    return EFI_WRITE_PROTECTED;
  }
  return EFI_SUCCESS;
}


UINTN
GetTransferBlockCount (
  IN  FDC_BLK_IO_DEV  *FdcDev,
  IN  EFI_LBA         LBA,
  IN  UINTN           NumberOfBlocks
  )
/*++

  Routine Description:  Calculate the number of block in the same cylinder 
                        according to LBA
  Parameters:
    FdcDev FDC_BLK_IO_DEV *: A pointer to Data Structure FDC_BLK_IO_DEV
    LBA EFI_LBA:      The starting logic block address            
    NumberOfBlocks UINTN: The number of blocks
  Returns:
    UINTN : The number of blocks in the same cylinder which the starting 
        logic block address is LBA

--*/
{
  UINT8              EndOfTrack;
  UINT8              Head;
  UINT8              SectorsInTrack;

  //
  // Calculate the number of block in the same cylinder
  //
  EndOfTrack = DISK_1440K_EOT;
  Head = (UINT8)((UINTN)LBA / EndOfTrack % 2);
  
  SectorsInTrack = (UINT8)(EndOfTrack * (2 - Head) - (UINT8)((UINTN)LBA % EndOfTrack));
  if (SectorsInTrack < NumberOfBlocks) {
    return SectorsInTrack;
  } else {
    return NumberOfBlocks;
  }
}

VOID
FddTimerProc ( 
  IN EFI_EVENT  Event, 
  IN VOID       *Context 
  )
/*++

  Routine Description:  When the Timer(2s) off, turn the drive's motor off
  Parameters:
    Event EFI_EVENT: Event(the timer) whose notification function is being 
                     invoked
    Context VOID *:  Pointer to the notification function�s context 
  Returns:
    VOID

--*/
{
  FDC_BLK_IO_DEV  *FdcDev;
  UINT8           data;

  FdcDev = (FDC_BLK_IO_DEV *)Context;
  
  //
  // Get the motor status
  //
  data = FdcReadPort (FdcDev, FDC_REGISTER_DOR);
  
  if (((FdcDev->Disk == FDC_DISK0) && ((data & 0x10) != 0x10))
      || ((FdcDev->Disk == FDC_DISK1) && ((data & 0x21) != 0x21))) {
    return;
  }    
        
  
  //
  //the motor is on, so need motor off
  //
  data = 0x0C;
  data |= (SELECT_DRV & FdcDev->Disk);
  FdcWritePort (FdcDev, FDC_REGISTER_DOR, data);
  gBS->Stall (500);
}    

UINT8 
FdcReadPort (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINT32          Offset
  )
/*++

  Routine Description: Read I/O port for FDC  
  Parameters:
  Returns:
    
--*/  
{
  UINT8      Data;
  EFI_STATUS Status;

  //
  // Call IsaIo
  //
  Status = FdcDev->IsaIo->Io.Read (
     FdcDev->IsaIo, 
     EfiIsaIoWidthUint8, 
     FdcDev->BaseAddress + Offset, 
     1, 
     &Data
     );

  return Data;
}

VOID 
FdcWritePort (
  IN FDC_BLK_IO_DEV  *FdcDev,
  IN UINT32          Offset,
  IN UINT8           Data
  )
/*++

  Routine Description: Write I/O port for FDC  
  Parameters:
  Returns:
    
--*/  
{
  EFI_STATUS  Status;
  
  //
  // Call IsaIo
  //
  Status = FdcDev->IsaIo->Io.Write (
     FdcDev->IsaIo, 
     EfiIsaIoWidthUint8, 
     FdcDev->BaseAddress + Offset, 
     1, 
     &Data
     );
}
