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

    ide.c
    
Abstract: 
    

Revision History
--*/

#include "idebus.h"

UINT8
IDEReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
{
  UINT8         Data = 0;
  //
  // perform 1-byte data read from register
  //
  PciIo->Io.Read(PciIo,
                 EfiPciIoWidthUint8,
                 EFI_PCI_IO_PASS_THROUGH_BAR,
                 (UINT64)Port,
                 1,
                 &Data
                 );
  return Data;                 
}   

VOID
IDEReadPortWMultiple (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINTN                 Count,
  IN  VOID                  *Buffer
  )
/*++

Routine Description:
  Reads multiple words of data from the IDE data port. 
  Call the IO abstraction once to do the complete read,
  not one word at a time
  

Arguments:
  PciIo   - Pointer to the EFI_PCI_IO instance
  Port    - IO port to read
  Count   - No. of UINT16's to read
  Buffer  - Pointer to the data buffer for read

++*/
{
  UINT16    *AllignedBuffer;
  UINTN     Size;
  
  //
  // Prepare an 16-bit alligned working buffer. CpuIo will return failure and
  // not perform actual I/O operations if buffer pointer passed in is not at
  // natural boundary. The "Buffer" argument is passed in by user and may not
  // at 16-bit natural boundary.
  //
  Size = sizeof(UINT16) * Count;
  gBS->AllocatePool (
         EfiBootServicesData,
         Size,
         (VOID**)&AllignedBuffer
         );
  
  //
  // Perform UINT16 data read from FIFO
  //
  PciIo->Io.Read (
                 PciIo,
                 EfiPciIoWidthFifoUint16,
                 EFI_PCI_IO_PASS_THROUGH_BAR,
                 (UINT64)Port,
                 Count,
                 AllignedBuffer
                 );
  
  //
  // Copy data to user buffer
  //
  EfiCopyMem (Buffer, AllignedBuffer, Size);
  gBS->FreePool (AllignedBuffer);
}


VOID
IDEWritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT8                 Data
  )
{
  //
  // perform 1-byte data write to register
  //  
  PciIo->Io.Write(PciIo,
                  EfiPciIoWidthUint8,
                  EFI_PCI_IO_PASS_THROUGH_BAR,
                  (UINT64)Port,
                  1,
                  &Data
                  );  

}   

VOID
IDEWritePortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT16                Data
  )
{
  //
  // perform 1-word data write to register
  //
  PciIo->Io.Write(PciIo,
                  EfiPciIoWidthUint16,
                  EFI_PCI_IO_PASS_THROUGH_BAR,
                  (UINT64)Port,
                  1,
                  &Data
                  );
}   

VOID
IDEWritePortWMultiple (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINTN                 Count,
  IN  VOID                  *Buffer
  )
/*++

Routine Description:
  Write multiple words of data to the IDE data port. 
  Call the IO abstraction once to do the complete read,
  not one word at a time
  

Arguments:
  PciIo   - Pointer to the EFI_PCI_IO instance
  Port    - IO port to read
  Count   - No. of UINT16's to read
  Buffer  - Pointer to the data buffer for read

++*/
{
  UINT16    *AllignedBuffer;
  UINTN     Size;
  
  //
  // Prepare an 16-bit alligned working buffer. CpuIo will return failure and
  // not perform actual I/O operations if buffer pointer passed in is not at
  // natural boundary. The "Buffer" argument is passed in by user and may not
  // at 16-bit natural boundary.
  //
  Size = sizeof(UINT16) * Count;
  gBS->AllocatePool (
         EfiBootServicesData,
         Size,
         (VOID**)&AllignedBuffer
         );
  
  //
  // Copy data from user buffer to working buffer
  //
  EfiCopyMem (AllignedBuffer, Buffer, Size);

  //
  // perform UINT16 data write to the FIFO
  //
  PciIo->Io.Write (
              PciIo,
                 EfiPciIoWidthFifoUint16,
                 EFI_PCI_IO_PASS_THROUGH_BAR,
                 (UINT64)Port,
                 Count,
              AllignedBuffer
                 );
  
  gBS->FreePool (AllignedBuffer);
}


BOOLEAN
BadIdeDeviceCheck (
  IN IDE_BLK_IO_DEV *IdeDev
  )
{
  //
  //  check whether all registers return 0xff,
  //  if so, deem the channel is disabled.
  //
  
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Data) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg1.Feature) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->SectorNumber) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->CylinderLsb) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->CylinderMsb) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Head) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Command) != 0xff) {
    return FALSE;
  }
  if (IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus) != 0xff) {
    return FALSE;
  }

  return TRUE;
}    


//****************************************************************************
// DiscoverIdeDevice
//****************************************************************************
EFI_STATUS
DiscoverIdeDevice (
  IN IDE_BLK_IO_DEV *IdeDev
  )
{
  EFI_STATUS      Status;
  
  //
  // Check whether all the IDE registers have the value of 0xFF,
  // which is invalid. TRUE means the registers are not valid.
  //
  if (BadIdeDeviceCheck (IdeDev) == TRUE) {
    return EFI_NOT_FOUND;
  }
  
  if( DetectIDEController(IdeDev) == FALSE ) {
    return EFI_NOT_FOUND;
  }

  //
  // test if it is an ATA device 
  //
  Status = ATAIdentify (IdeDev);
  if (EFI_ERROR(Status)) {
    //
    // if not ATA device, test if it is an ATAPI device
    //
    Status = ATAPIIdentify (IdeDev);
    if (EFI_ERROR(Status)) {
      //
      // if not ATAPI device either, return error.
      //
      return EFI_NOT_FOUND;
    }
  }
  
  //
  // configure timing registers in configuration space
  //
  IdeConfigureTiming (IdeDev);
  
  //
  // set device tranfer pio mode (using best PIO mode the device supported.)
  //
  Status = SetPioMode(IdeDev);
  if (EFI_ERROR(Status)) {    
    AtaSoftReset (IdeDev);
  }
  
  //
  // Init Block I/O interface
  //
  IdeDev->BlkIo.Revision    = EFI_BLOCK_IO_PROTOCOL_REVISION;
  IdeDev->BlkIo.Reset       = IDEBlkIoReset;
  IdeDev->BlkIo.ReadBlocks  = IDEBlkIoReadBlocks;
  IdeDev->BlkIo.WriteBlocks = IDEBlkIoWriteBlocks;
  IdeDev->BlkIo.FlushBlocks = IDEBlkIoFlushBlocks;  

  IdeDev->BlkMedia.LogicalPartition = FALSE ;
  IdeDev->BlkMedia.WriteCaching     = FALSE ;

  return EFI_SUCCESS;
}

//*******************************************************************************
EFI_STATUS
CheckPowerMode (
  IDE_BLK_IO_DEV    *IdeDev,
  UINT8             AtaCommand
  )
{
  UINT8       StatusRegister;
  UINT8       ErrorRegister;
  EFI_STATUS  Status;  
  UINT8       SectorCountRegister;
  
  //
  // select device
  //
  IDEWritePortB (IdeDev->PciIo,
                 IdeDev->IoPort->Head, 
                 (UINT8)((IdeDev->Device << 4) | 0xe0 )
                 );
  //
  // refresh the SectorCount register
  //
  SectorCountRegister = 0x55;
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount, SectorCountRegister);
  
  //
  // select device
  //
  IDEWritePortB (IdeDev->PciIo,
                 IdeDev->IoPort->Head, 
                 (UINT8)((IdeDev->Device << 4) | 0xe0 )
                 ); 
  Status = DRDYReady (IdeDev,100);
  
  //
  // select device
  //
  IDEWritePortB (IdeDev->PciIo,
                 IdeDev->IoPort->Head, 
                 (UINT8)((IdeDev->Device << 4) | 0xe0 )
                 );
  //
  // send 'check power' commandd via Command Register
  //
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Command, AtaCommand);
  
  Status = WaitForBSYClear (IdeDev,1000);
  if (EFI_ERROR(Status)) {
    return EFI_TIMEOUT;
  }
  
  StatusRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Status);
  
  //
  // command returned status is DRDY, indicating device supports the command,
  // so device is present. 
  //
  if ((StatusRegister & DRDY) == DRDY) {
    return EFI_SUCCESS;
  }  
  
  SectorCountRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount);
  
  //
  // command returned status is ERR & ABRT_ERR, indicating device does not support
  // the command, so device is present.
  //
  if ((StatusRegister & ERR) == ERR) {
    ErrorRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg1.Error);
    if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
      return EFI_ABORTED;
    } else {
      //
      // According to spec, no other error code is valid 
      //
      return EFI_NOT_FOUND;
    }
  }
  
  if (   (SectorCountRegister == 0x00) || (SectorCountRegister == 0x80)
      || (SectorCountRegister == 0xff) ) {
      //
    // Write SectorCount 0x55 but return valid state value. Maybe no device 
    // exists or some slow kind of ATAPI device exists. 
      //
      IDEWritePortB (IdeDev->PciIo,
                   IdeDev->IoPort->Head, 
                   (UINT8)((IdeDev->Device << 4) | 0xe0 )
                   );
      //
      // write 0x55 and 0xaa to SectorCounter register,
      // if the data could be written into the register,
      // indicating the device is present, otherwise the device is not present.
      //
      SectorCountRegister = 0x55;
      IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount, SectorCountRegister);
      gBS->Stall(1000);
      SectorCountRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount);
      if (SectorCountRegister != 0x55) {
        return EFI_NOT_FOUND;
      }
      
    //
    //Send a "ATAPI TEST UNIT READY" command ... slow but accurate
    //
    Status = AtapiTestUnitReady (IdeDev);
    return Status;
  }  
  
  return EFI_NOT_FOUND;
}

//********************************************************************************
BOOLEAN
DetectIDEController (
  IN  IDE_BLK_IO_DEV  *IdeDev
)
/*++
  
  Name: DetectIDEController


  Purpose: 
      This function is called by DiscoverIdeDevice(). It is used for detect 
      whether the IDE device exists in the specified Channel as the specified 
      Device Number.

      There is two IDE channels: one is Primary Channel, the other is 
      Secondary Channel.(Channel is the logical name for the physical "Cable".) 
      Different channel has different register group.

      On each IDE channel, at most two IDE devices attach, 
      one is called Device 0 (Master device), the other is called Device 1 
      (Slave device). The devices on the same channel co-use the same register 
      group, so before sending out a command for a specified device via command 
      register, it is a must to select the current device to accept the command 
      by set the device number in the Head/Device Register.
 
      The mechanism used for IDE device detection is as follows:
      1. Waits for BSY bit in the Status Register clear until 40 ms timeout. 
         If device exists, the BSY bit should initially stay in cleared status.
      2. Output the device number via the Head/Device Register to select the
         device.
      3. If device exists, then it should response by setting the DRDY bit in 
         the Status Register, indicating device ready. But there may be  
         exceptions, for example, in some platform, the ATAPI device will never
         set DRDY bit when device is selected during detection period. But it do 
         response to the ATAPI Test Unit Ready Packet Command which used to find 
         out if ATAPI device is in ready status.
      4. If device is found, then ATA Nop Command will be sent to make the 
         register group back to the normal status.
            
  Parameters:
      IDE_BLK_IO_DEV  IN    *IdeDev
            pointer pointing to IDE_BLK_IO_DEV data structure, used
            to record all the information of the IDE device.


  Returns:    
      TRUE      
            successfully detects device.

      FALSE
            any failure during detection process will return this
            value.


  Notes:
--*/
{
  EFI_STATUS    Status;  
  UINT8         DeviceControl;
  UINT8         AtaCommand;
  
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Head, 
                (UINT8)((IdeDev->Device << 4) | 0xe0)
                );   
    
  //
  // soft reset device
  //
  DeviceControl = 0;
  DeviceControl |= SRST;      // set SRST bit to initiate soft reset
  DeviceControl |= bit1;      // disable Interrupt
   
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Alt.DeviceControl, DeviceControl);
   
  gBS->Stall(30);     // Wait 30us 
   
  //
  // Clear SRST bit
  //
  DeviceControl &= 0xfb ;     // 0xfb:1111,1011
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Alt.DeviceControl, DeviceControl);

  
  //
  //  wait 31 seconds for BSY clear
  //
  Status = WaitForBSYClear (IdeDev, 31000);
  if (EFI_ERROR(Status)) {
    return FALSE;
  } 
  
  //
  // 'check power' command for ata device
  //
  AtaCommand = 0x98;
  Status = CheckPowerMode (IdeDev,AtaCommand);
  if ((Status == EFI_ABORTED) || (Status == EFI_SUCCESS)) {
    return TRUE;
  } else if (Status == EFI_NOT_FOUND) {
    return FALSE;
  }
  
  //
  // 'check power' command for atapi device
  //
  AtaCommand = 0xE5;
  Status = CheckPowerMode (IdeDev,AtaCommand);
  if ((Status == EFI_ABORTED) || (Status == EFI_SUCCESS)) {
    return TRUE;
  }
  
  return FALSE;
}

//****************************************************************************
EFI_STATUS  
DRQClear (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  )
/*++
  Name:   DRQClear


  Purpose: 
        This function is used to poll for the DRQ bit clear in the Status 
        Register. DRQ is cleared when the device is finished transferring data. 
        So this function is called after data transfer is finished.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
            pointer pointing to IDE_BLK_IO_DEV data structure, used
            to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
            used to designate the timeout for the DRQ clear.

        CHAR16      IN    *ErrorString
            string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
            DRQ bit clear within the time out.

        EFI_TIMEOUT
            DRQ bit not clear within the time out. 


  Notes:
        Read Status Register will clear interrupt status.
--*/  
{
  UINT32        Delay;
  UINT8         StatusRegister;
  UINT8         ErrorRegister;
  
  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);
  do {
    
    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status);
    
    //
    // wait for BSY == 0 and DRQ == 0
    //
    if ( (StatusRegister & (DRQ | BSY) ) == 0) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR ) {
      
      ErrorRegister =  IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    //
    //  Stall for 30 us
    //
    gBS->Stall(30);
    
    Delay --;

  } while (Delay);
  
  if (Delay == 0) {
    return  EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}




EFI_STATUS  
DRQClear2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  )
/*++
  Name:   DRQClear2


  Purpose: 
        This function is used to poll for the DRQ bit clear in the Alternate 
        Status Register. DRQ is cleared when the device is finished 
        transferring data. So this function is called after data transfer
        is finished.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
          used to designate the timeout for the DRQ clear.

        CHAR16      IN    *ErrorString
          string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
          DRQ bit clear within the time out.

        EFI_TIMEOUT
          DRQ bit not clear within the time out. 


  Notes:
        Read Alternate Status Register will not clear interrupt status.
--*/  
{
  UINT32      Delay;
  UINT8       AltRegister;
  UINT8       ErrorRegister;
  
  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);
  do {
     
    AltRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus);
    
    //
    //  wait for BSY == 0 and DRQ == 0
    //
    if ( (AltRegister & (DRQ | BSY) ) == 0) {
      break;
    }
    
    if ((AltRegister & (BSY | ERR)) == ERR ) {
      
      ErrorRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30); // Stall for 30 us
    
    Delay --;

  } while (Delay);
  
  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}



EFI_STATUS  
DRQReady (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  )
/*++
  Name:   DRQReady

  
  Purpose: 
        This function is used to poll for the DRQ bit set in the 
        Status Register.
        DRQ is set when the device is ready to transfer data. So this function
        is called after the command is sent to the device and before required 
        data is transferred.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
            pointer pointing to IDE_BLK_IO_DEV data structure,used
            to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
            used to designate the timeout for the DRQ ready.

        CHAR16      IN    *ErrorString
            string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
            DRQ bit set within the time out.

        EFI_TIMEOUT
            DRQ bit not set within the time out.
            
        EFI_ABORTED
            DRQ bit not set caused by the command abort.

  Notes:
        Read Status Register will clear interrupt status.

--*/ 
{
  UINT32      Delay; 
  UINT8       StatusRegister;
  UINT8       ErrorRegister;


  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);
  do {
    //
    //  read Status Register will clear interrupt
    //
    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status);   
    
    //
    //  BSY==0,DRQ==1
    //
    if ( (StatusRegister & (BSY | DRQ )) == DRQ) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR ) {
      
      ErrorRegister =  IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }
   
    gBS->Stall(30); // Stall for 30 us
    
    Delay --;
  } while (Delay);

  if (Delay == 0) {
    return  EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}


EFI_STATUS  
DRQReady2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  )
/*++
  Name:   DRQReady2


  Purpose: 
        This function is used to poll for the DRQ bit set in the 
        Alternate Status Register. DRQ is set when the device is ready to 
        transfer data. So this function is called after the command 
        is sent to the device and before required data is transferred.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
          used to designate the timeout for the DRQ ready.

        CHAR16      IN    *ErrorString
          string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
          DRQ bit set within the time out.

        EFI_TIMEOUT
          DRQ bit not set within the time out. 
        
        EFI_ABORTED
            DRQ bit not set caused by the command abort.

  Notes:
        Read Alternate Status Register will not clear interrupt status.
--*/  
{
  UINT32      Delay; 
  UINT8       AltRegister;
  UINT8       ErrorRegister;

  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);

  do {
    //
    //  Read Alternate Status Register will not clear interrupt status
    //
    AltRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus); 
    //
    //BSY == 0 , DRQ == 1
    //
    if ( (AltRegister & (BSY | DRQ )) == DRQ) {
      break;
    }    
    
    if ((AltRegister & (BSY | ERR)) == ERR ) {
      
      ErrorRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30); // Stall for 30 us
    
    Delay --;
  } while (Delay);
  
  if (Delay == 0) {
    return  EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}



EFI_STATUS  
WaitForBSYClear (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  )
/*++
  Name:
        WaitForBSYClear


  Purpose: 
        This function is used to poll for the BSY bit clear in the 
        Status Register. BSY is clear when the device is not busy.
        Every command must be sent after device is not busy.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
          used to designate the timeout for the DRQ ready.

        CHAR16      IN    *ErrorString
          string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
          BSY bit clear within the time out.

        EFI_TIMEOUT
          BSY bit not clear within the time out. 


  Notes:
        Read Status Register will clear interrupt status.
--*/ 
{
  UINT32        Delay; 
  UINT8         StatusRegister;
  
  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) +  1);
  do {

    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status); 
    if ( (StatusRegister & BSY) == 0x00) {
      break;
    }
    
    gBS->Stall(30); // Stall for 30 us
    
    Delay --;

  } while (Delay);

  if (Delay == 0){
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}


//****************************************************************************
// WaitForBSYClear2
//****************************************************************************
EFI_STATUS  
WaitForBSYClear2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  )
/*++
  Name:
        WaitForBSYClear2


  Purpose: 
        This function is used to poll for the BSY bit clear in the 
        Alternate Status Register. BSY is clear when the device is not busy.
        Every command must be sent after device is not busy.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
          used to designate the timeout for the DRQ ready.

        CHAR16      IN    *ErrorString
          string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
          BSY bit clear within the time out.

        EFI_TIMEOUT
          BSY bit not clear within the time out. 


  Notes:
        Read Alternate Status Register will not clear interrupt status.
--*/  
{
  UINT32        Delay; 
  UINT8         AltRegister;
  
  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) +  1);
  do {
    AltRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus); 
    if ( (AltRegister & BSY) == 0x00) {
      break;
    }

    gBS->Stall(30); // Stall for 30 us
    
    Delay --;

  } while (Delay);
  
  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

//****************************************************************************
// DRDYReady
//****************************************************************************
EFI_STATUS  
DRDYReady (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           DelayInMilliSeconds
  )
/*++
  Name:
        DRDYReady


  Purpose: 
        This function is used to poll for the DRDY bit set in the 
        Status Register. DRDY bit is set when the device is ready 
        to accept command. Most ATA commands must be sent after 
        DRDY set except the ATAPI Packet Command.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
          used to designate the timeout for the DRQ ready.

        CHAR16      IN    *ErrorString
          string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
          DRDY bit set within the time out.

        EFI_TIMEOUT
          DRDY bit not set within the time out. 


  Notes:
        Read Status Register will clear interrupt status.
--*/  
{
  UINT32        Delay; 
  UINT8         StatusRegister;
  UINT8       ErrorRegister;
  
  Delay = (UINT32)(((DelayInMilliSeconds * STALL_1_MILLI_SECOND)  / 30) + 1);
  do {
    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status); 
    //
    //  BSY == 0 , DRDY == 1
    //
    if ( (StatusRegister & (DRDY | BSY) ) == DRDY) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR ) {
      
      ErrorRegister =  IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }
// Todo...
    gBS->Stall(30); // Stall for 30 us

    Delay --;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}


//****************************************************************************
// DRDYReady2
//****************************************************************************
EFI_STATUS  
DRDYReady2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           DelayInMilliSeconds
  )
/*++
  Name:
        DRDYReady2


  Purpose: 
        This function is used to poll for the DRDY bit set in the 
        Alternate Status Register. DRDY bit is set when the device is ready 
        to accept command. Most ATA commands must be sent after 
        DRDY set except the ATAPI Packet Command.


  Parameters:
        IDE_BLK_IO_DEV  IN    *IdeDev
          pointer pointing to IDE_BLK_IO_DEV data structure, used
          to record all the information of the IDE device.

        UINTN     IN    TimeoutInMilliSeconds
          used to designate the timeout for the DRQ ready.

        CHAR16      IN    *ErrorString
          string used to identify the calling function's information.


  Returns:  
        EFI_SUCCESS
          DRDY bit set within the time out.

        EFI_TIMEOUT
          DRDY bit not set within the time out. 


  Notes:
        Read Alternate Status Register will clear interrupt status.
--*/  
{
  UINT32        Delay; 
  UINT8         AltRegister;
  UINT8       ErrorRegister;
  
  Delay = (UINT32)(((DelayInMilliSeconds * STALL_1_MILLI_SECOND)  / 30) + 1);
  do {
    AltRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus); 
    //
    //  BSY == 0 , DRDY == 1
    //
    if ( (AltRegister & (DRDY | BSY) ) == DRDY){
      break;
    }
    
    if ((AltRegister & (BSY | ERR)) == ERR ) {
      
      ErrorRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }
    
    gBS->Stall(30); // Stall for 30 us
    
    Delay --;
  } while (Delay);
  
  if (Delay == 0){
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

//****************************************************************************
// SwapStringChars
//****************************************************************************
VOID
SwapStringChars ( 
  IN CHAR8  *Destination,  
  IN CHAR8  *Source,   
  IN UINT32 Size  
  )         
/*++
  Name:
        SwapStringChars


  Purpose: 
        This function is a helper function used to change the char order in a 
        string. It is designed specially for the PrintAtaModuleName() function.
        After the IDE device is detected, the IDE driver gets the device module
        name by sending ATA command called ATA Identify Command or ATAPI 
        Identify Command to the specified IDE device. The module name returned 
        is a string of ASCII characters: the first character is bit8--bit15 
        of the first word, the second character is bit0--bit7 of the first word 
        and so on. Thus the string can not be print directly before it is 
        preprocessed by this func to change the order of characters in 
        each word in the string.


  Parameters:
        CHAR8 IN    *Destination
          Indicates the destination string.

        CHAR8 IN    *Source
          Indicates the source string.

        UINT8 IN    Size
          the length of the string


  Returns:  
        none

  Notes:

--*/  
{
  UINT32  Index;
  CHAR8   Temp;

  for (Index = 0; Index < Size; Index += 2) {
    
    Temp = Source[Index + 1];
    Destination[Index + 1] = Source[Index];
    Destination[Index] = Temp;
  }
}

//****************************************************************************
// ReleaseIdeResources
//****************************************************************************
VOID
ReleaseIdeResources (
  IN  IDE_BLK_IO_DEV  *IdeBlkIoDevice
  )
/*
  Release all the resourses occupied by the IDE_BLK_IO_DEV
*/
{
  if (IdeBlkIoDevice == NULL) {
    return;
  }
  
  if (IdeBlkIoDevice->SenseData != NULL) {
    gBS->FreePool (IdeBlkIoDevice->SenseData);
    IdeBlkIoDevice->SenseData = NULL;
  }
  
  if (IdeBlkIoDevice->Cache != NULL) {
    gBS->FreePool (IdeBlkIoDevice->Cache);
    IdeBlkIoDevice->Cache = NULL;
  }
  
  if (IdeBlkIoDevice->pIdData != NULL) {
    gBS->FreePool (IdeBlkIoDevice->pIdData);
    IdeBlkIoDevice->pIdData = NULL;
  }
  
  if (IdeBlkIoDevice->pInquiryData != NULL) {
    gBS->FreePool (IdeBlkIoDevice->pInquiryData);
    IdeBlkIoDevice->pInquiryData = NULL;
  }
  
  if (IdeBlkIoDevice->ControllerNameTable != NULL) {
    EfiLibFreeUnicodeStringTable (IdeBlkIoDevice->ControllerNameTable);
    IdeBlkIoDevice->ControllerNameTable = NULL;
  }
  
  gBS->FreePool (IdeBlkIoDevice);
  IdeBlkIoDevice = NULL;
  
  return;
}

VOID
GetDeviceBestPIOMode (
  IN  IDE_BLK_IO_DEV  *IdeDev
  )
{
  UINT16      PioMode,AdvancedPioMode,Temp;
  UINT16      Index;
  UINT16      MinimumPioCycleTime;
  
  if (IdeDev->pIdData == NULL) {
    return;
  }
  
  Temp = 0xff;
  
  PioMode = (UINT8)(IdeDev->pIdData->pio_cycle_timing >> 8);
  
  //
  // see whether Identify Data word 64 - 70 are valid
  //
  if ((IdeDev->pIdData->field_validity & bit1) == bit1) {
    
    AdvancedPioMode = IdeDev->pIdData->advanced_pio_modes;
    
    for (Index = 0; Index < 8; Index ++) {        
      if ((AdvancedPioMode & bit0) != 0) {
        Temp = Index;
      }
      AdvancedPioMode >>= 1;
    }
    
    //
    // if Temp is modified, meant the advanced_pio_modes is not zero;
    // if Temp is not modified, meant the no advanced PIO Mode supported,
    // the best PIO Mode is the value in pio_cycle_timing.
    //
    if (Temp != 0xff) {
      AdvancedPioMode = (UINT16)(Temp + 3);
    } else {
      AdvancedPioMode = PioMode;
    }

    //
    // Limit the PIO mode to at most PIO4.
    //
    PioMode = (UINT16)EFI_MIN (AdvancedPioMode,4);
    
    MinimumPioCycleTime = IdeDev->pIdData->min_pio_cycle_time_with_flow_control;
    
    if (MinimumPioCycleTime <= 120) {
      PioMode = (UINT16)EFI_MIN(4,PioMode);
    } else if (MinimumPioCycleTime <= 180) {
      PioMode = (UINT16)EFI_MIN(3,PioMode);
    } else if (MinimumPioCycleTime <= 240) {
      PioMode = (UINT16)EFI_MIN(2,PioMode);
    } else {
      PioMode = 0;
    }
    
    switch (PioMode) {
      case 0:
      case 1: // fall through
        IdeDev->PioMode = ATA_PIO_MODE_BELOW_2;
        break;
      
      case 2:
        IdeDev->PioMode = ATA_PIO_MODE_2;
        break;
      
      case 3:
        IdeDev->PioMode = ATA_PIO_MODE_3;
        break;
        
      case 4:
        IdeDev->PioMode = ATA_PIO_MODE_4;
        break;
    }
    
  } else {
    
    if (PioMode < 2) {
      IdeDev->PioMode = ATA_PIO_MODE_BELOW_2;
    } else {
      IdeDev->PioMode = ATA_PIO_MODE_2;
    }
  }
  
}