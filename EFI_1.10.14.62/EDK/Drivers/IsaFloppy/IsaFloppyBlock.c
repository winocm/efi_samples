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

  IsaFloppyBlock.c

Abstract:

  ISA Floppy Driver
  1. Support two types diskette drive  
     1.44M drive and 2.88M drive (and now only support 1.44M)
  2. Support two diskette drives
  3. Use DMA channel 2 to transfer data
  4. Do not use interrupt
  5. Support diskette change line signal and write protect
  
  Implement the Block IO interface

Revision History:

--*/

#include "IsaFloppy.h"

EFI_STATUS 
FdcReset (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  BOOLEAN                ExtendedVerification
  )
/*++
  
  Routine Description:  Reset the Floppy Logic Drive, call the FddReset function   
  Parameters:
    This EFI_BLOCK_IO *: A pointer to the Block I/O protocol interface
    ExtendedVerification BOOLEAN: Indicate that the driver may perform a more 
                    exhaustive verification operation of the device during 
                    reset, now this par is ignored in this driver          
  Returns:
    EFI_SUCCESS:      The Floppy Logic Drive is reset
    EFI_DEVICE_ERROR: The Floppy Logic Drive is not functioning correctly 
                      and can not be reset

--*/

{
  FDC_BLK_IO_DEV  *FdcDev;

  //
  // Reset the Floppy Disk Controller
  //
  FdcDev = FDD_BLK_IO_FROM_THIS (This);

  return FddReset (FdcDev);
}


EFI_STATUS 
FddFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This
  )
/*++
  
  Routine Description:  
  Parameters:
    This EFI_BLOCK_IO *: A pointer to the Block I/O protocol interface
  Returns:
    EFI_SUCCESS:    

--*/
{
  //
  // Not supported yet
  //
  return EFI_SUCCESS;
}


EFI_STATUS
FddReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                LBA,
  IN  UINTN                  BufferSize,
  OUT VOID                   *Buffer
  )
/*++

  Routine Description:  Read the requested number of blocks from the device   
  Parameters:
    This EFI_BLOCK_IO *: A pointer to the Block I/O protocol interface
    MediaId UINT32:    The media id that the read request is for    
    LBA EFI_LBA:     The starting logic block address to read from on the device
    BufferSize UINTN:  The size of the Buffer in bytes
    Buffer VOID *:     A pointer to the destination buffer for the data
  Returns:
    EFI_SUCCESS:     The data was read correctly from the device
    EFI_DEVICE_ERROR:The device reported an error while attempting to perform
                     the read operation
    EFI_NO_MEDIA:    There is no media in the device
    EFI_MEDIA_CHANGED:   The MediaId is not for the current media
    EFI_BAD_BUFFER_SIZE: The BufferSize parameter is not a multiple of the 
                         intrinsic block size of the device
    EFI_INVALID_PARAMETER:The read request contains LBAs that are not valid, 
                          or the buffer is not on proper alignment 

--*/
{
  return FddReadWriteBlocks (This, MediaId, LBA, BufferSize, READ, Buffer);
}

EFI_STATUS
FddWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                LBA,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
  )
/*++

  Routine Description:  Write a specified number of blocks to the device   
  Parameters:
    This EFI_BLOCK_IO *: A pointer to the Block I/O protocol interface
    MediaId UINT32:    The media id that the write request is for   
    LBA EFI_LBA:     The starting logic block address to be written
    BufferSize UINTN:  The size in bytes in Buffer
    Buffer VOID *:     A pointer to the source buffer for the data
  Returns :
    EFI_SUCCESS:     The data were written correctly to the device
    EFI_WRITE_PROTECTED: The device can not be written to 
    EFI_NO_MEDIA:    There is no media in the device
    EFI_MEDIA_CHANGED:   The MediaId is not for the current media
    EFI_DEVICE_ERROR:  The device reported an error while attempting to perform 
                       the write operation 
    EFI_BAD_BUFFER_SIZE: The BufferSize parameter is not a multiple of the 
                         intrinsic block size of the device
    EFI_INVALID_PARAMETER:The write request contains LBAs that are not valid, 
                          or the buffer is not on proper alignment 

--*/
{
  return FddReadWriteBlocks (This, MediaId, LBA, BufferSize, WRITE, Buffer);
}

EFI_STATUS
FddReadWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                LBA,
  IN  UINTN                  BufferSize,
  IN  BOOLEAN                Operation,
  OUT VOID                   *Buffer
  )
{
  EFI_BLOCK_IO_MEDIA    *Media;
  FDC_BLK_IO_DEV        *FdcDev;  
  UINTN                 BlockSize;            
  UINTN                 NumberOfBlocks; 
  UINTN                 BlockCount;
  EFI_STATUS            Status;
  EFI_LBA               LBA0;
  UINT8                 *Pointer;
  
  //
  //Get the intrinsic block size
  //
  Media     = This->Media;
  BlockSize = Media->BlockSize;
  FdcDev = FDD_BLK_IO_FROM_THIS (This);

  //
  //Check buffer alignment
  //
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }

  if (Operation == WRITE) {
    if (LBA == 0) {
      FdcFreeCache (FdcDev);
    }
  }

  //
  // Check the Parameter is valid
  //
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }
  
  //
  // Set the drive motor on
  //
  Status = MotorOn (FdcDev);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Check to see if media can be detected
  //
  Status = DetectMedia (FdcDev);
  if (EFI_ERROR (Status)) {
    MotorOff (FdcDev);
    FdcFreeCache (FdcDev);
    return EFI_DEVICE_ERROR;
  }

  //
  // Check to see if media is present
  //
  if (!(Media->MediaPresent)) {
    MotorOff (FdcDev);
    FdcFreeCache (FdcDev);
/*
    if (FdcDev->Cache) {
      gBS->FreePool (FdcDev->Cache);
      FdcDev->Cache = NULL;
    }
*/
    return EFI_NO_MEDIA;
  }

  //
  // Check to see if media has been changed
  //
  if (MediaId != Media->MediaId) {
    MotorOff (FdcDev);
    FdcFreeCache (FdcDev);
    return EFI_MEDIA_CHANGED;
  }
  
  if (Operation == WRITE) {
    if (Media->ReadOnly) {
      MotorOff (FdcDev);
      return EFI_WRITE_PROTECTED;
    }
  }
  
  //
  // Check the parameters for this read/write operation
  //
  if (BufferSize % BlockSize != 0) {
    MotorOff (FdcDev);
    return EFI_BAD_BUFFER_SIZE;
  }
  
  if (LBA > Media->LastBlock) {
    MotorOff (FdcDev);
    return EFI_INVALID_PARAMETER;
  }
  
  if (((BufferSize / BlockSize) + LBA - 1) > Media->LastBlock) {
    MotorOff (FdcDev);
    return EFI_INVALID_PARAMETER;
  }

  if (Operation == READ) {
    //
    // See if the data that is being read is already in the cache
    //
    if (FdcDev->Cache) {
      if (LBA == 0 && BufferSize == BlockSize) {
        MotorOff (FdcDev);
        EfiCopyMem((UINT8*)Buffer, (UINT8*)FdcDev->Cache, BlockSize);
        return EFI_SUCCESS;
      }
    }
  }
  
  //
  // Set up Floppy Disk Controller
  //
  Status = Setup (FdcDev);
  if (EFI_ERROR (Status)) {
    MotorOff (FdcDev);
    return EFI_DEVICE_ERROR;
  }
  
  NumberOfBlocks = BufferSize / BlockSize;
  LBA0 = LBA;
  Pointer  = Buffer;

  //
  // read blocks in the same cylinder.
  // in a cylinder , there are 18 * 2 = 36 blocks
  //
  BlockCount = GetTransferBlockCount (FdcDev, LBA, NumberOfBlocks);
  while ((BlockCount != 0) && !EFI_ERROR(Status)) {
    Status = ReadWriteDataSector (FdcDev, Buffer, LBA, BlockCount, Operation);
    if (EFI_ERROR (Status)) {
      MotorOff (FdcDev);
      FddReset (FdcDev);
      return EFI_DEVICE_ERROR;
    }
    LBA += BlockCount;
    NumberOfBlocks -= BlockCount;
    Buffer = (VOID*)((UINTN)Buffer + BlockCount * BlockSize);
    BlockCount = GetTransferBlockCount (FdcDev, LBA, NumberOfBlocks);
  }
  
  Buffer = Pointer;

  //
  // Turn the motor off
  //
  MotorOff (FdcDev);

  if (Operation == READ) {
    //
    // Cache the data read
    //
    if (LBA0 == 0 && !FdcDev->Cache) {
      FdcDev->Cache = EfiLibAllocatePool (BlockSize);
      EfiCopyMem(FdcDev->Cache, Buffer, BlockSize);
    }
  }
  
  return EFI_SUCCESS;

}

VOID
FdcFreeCache (
  IN    FDC_BLK_IO_DEV  *FdcDev
)
{
  if (FdcDev->Cache) {
    gBS->FreePool (FdcDev->Cache);
    FdcDev->Cache = NULL;
  }
}
