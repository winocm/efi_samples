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

  BlockIo.h

Abstract:

  Block IO protocol as defined in the EFI 1.0 specification.

  The Block IO protocol is used to abstract block devices like hard drives,
  DVD-ROMs and floppy drives.

 
--*/

#ifndef __BLOCK_IO_H__
#define __BLOCK_IO_H__


#define EFI_BLOCK_IO_PROTOCOL_GUID \
  { 0x964e5b21, 0x6459, 0x11d2, 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

//
// Forward reference for pure ANSI compatability
//
EFI_INTERFACE_DECL(_EFI_BLOCK_IO_PROTOCOL);

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_RESET) (
  IN struct _EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                        ExtendedVerification
  )
/*++

  Routine Description:
    Reset the Block Device.

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES            - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_READ) (
  IN struct _EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  )
/*++

  Routine Description:
    Read BufferSize bytes from Lba into Buffer.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Lba        - The starting Logical Block Address to read from
    BufferSize - Size of Buffer, must be a multiple of device block size.
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was read correctly from the device.
    EFI_DEVICE_ERROR      - The device reported an error while performing the read.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHANGED     - The MediaId does not matched the current device.
    EFI_BAD_BUFFER_SIZE   - The Buffer was not a multiple of the block size of the 
                            device.
    EFI_INVALID_PARAMETER - The read request contains device addresses that are not 
                            valid for the device.

--*/
;


typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_WRITE) (
  IN struct _EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  )
/*++

  Routine Description:
    Write BufferSize bytes from Lba into Buffer.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Lba        - The starting Logical Block Address to read from
    BufferSize - Size of Buffer, must be a multiple of device block size.
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was written correctly to the device.
    EFI_WRITE_PROTECTED   - The device can not be written to.
    EFI_DEVICE_ERROR      - The device reported an error while performing the write.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_BAD_BUFFER_SIZE   - The Buffer was not a multiple of the block size of the 
                            device.
    EFI_INVALID_PARAMETER - The write request contains a LBA that is not 
                            valid for the device.

--*/
;


typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_FLUSH) (
  IN struct _EFI_BLOCK_IO_PROTOCOL  *This
  )
/*++

  Routine Description:
    Flush the Block Device.

  Arguments:
    This             - Protocol instance pointer.

  Returns:
    EFI_SUCCES       - All outstanding data was written to the device
    EFI_DEVICE_ERROR - The device reported an error while writting back the data
    EFI_NO_MEDIA     - There is no media in the device.

--*/
;


/*++

  Block IO read only mode data and updated only via members of BlockIO

  MediaId - The curent media Id. If the media changes, this value is changed.
  RemovableMedia - TRUE if the media is removable; otherwise, FALSE.
  MediaPresent   -  TRUE if there is a media currently present in the device;
                    othersise, FALSE. THis field shows the media present status
                    as of the most recent ReadBlocks() or WriteBlocks() call.
  LogicalPartition - TRUE if LBA 0 is the first block of a partition; otherwise
                     FALSE. For media with only one partition this would be TRUE.
  ReadOnly         - TRUE if the media is marked read-only otherwise, FALSE. This
                     field shows the read-only status as of the most recent 
                     WriteBlocks () call.
  WriteCaching     - TRUE if the WriteBlock () function caches write data.
  BlockSize - The intrinsic block size of the device. If the media changes, then
               this field is updated.
  IoAlign   - Supplies the alignment requirement for any buffer to read or write
               block(s).
  LastBlock - The last logical block address on the device. If the media changes,
               then this field is updated.

--*/

typedef struct {
  UINT32              MediaId;
  BOOLEAN             RemovableMedia;
  BOOLEAN             MediaPresent;

  BOOLEAN             LogicalPartition;
  BOOLEAN             ReadOnly;
  BOOLEAN             WriteCaching;

  UINT32              BlockSize;
  UINT32              IoAlign;

  EFI_LBA             LastBlock;
} EFI_BLOCK_IO_MEDIA;



#define EFI_BLOCK_IO_PROTOCOL_REVISION   0x00010000

typedef struct _EFI_BLOCK_IO_PROTOCOL {
  UINT64                  Revision;

  EFI_BLOCK_IO_MEDIA      *Media;

  EFI_BLOCK_RESET         Reset;
  EFI_BLOCK_READ          ReadBlocks;
  EFI_BLOCK_WRITE         WriteBlocks;
  EFI_BLOCK_FLUSH         FlushBlocks;

} EFI_BLOCK_IO_PROTOCOL;


extern EFI_GUID gEfiBlockIoProtocolGuid;

#endif
