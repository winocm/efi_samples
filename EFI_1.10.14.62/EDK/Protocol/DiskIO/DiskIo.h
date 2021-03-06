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

  DiskIo.h

Abstract:

  Disk IO protocol as defined in the EFI 1.0 specification.

  The Disk IO protocol is used to convert block oriented devices into byte
  oriented devices. The Disk IO protocol is intended to layer on top of the
  Block IO protocol.
 
--*/

#ifndef __DISK_IO_H__
#define __DISK_IO_H__

#define EFI_DISK_IO_PROTOCOL_GUID \
    { 0xce345171, 0xba0b, 0x11d2,  0x8e, 0x4f, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

EFI_INTERFACE_DECL(_EFI_DISK_IO_PROTOCOL);

typedef
EFI_STATUS
(EFIAPI *EFI_DISK_READ) (
  IN struct _EFI_DISK_IO_PROTOCOL *This,
  IN UINT32                       MediaId,
  IN UINT64                       Offset,
  IN UINTN                        BufferSize,
  OUT VOID                        *Buffer
  )
/*++

  Routine Description:
    Read BufferSize bytes from Offset into Buffer.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Offset     - The starting byte offset to read from
    BufferSize - Size of Buffer
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was read correctly from the device.
    EFI_DEVICE_ERROR      - The device reported an error while performing the read.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_INVALID_PARAMETER - The read request contains device addresses that are not 
                            valid for the device.

--*/
;


typedef
EFI_STATUS
(EFIAPI *EFI_DISK_WRITE) (
  IN struct _EFI_DISK_IO_PROTOCOL *This,
  IN UINT32                       MediaId,
  IN UINT64                       Offset,
  IN UINTN                        BufferSize,
  IN VOID                         *Buffer
  )
/*++

  Routine Description:
    Read BufferSize bytes from Offset into Buffer.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced. 
    Offset     - The starting byte offset to read from
    BufferSize - Size of Buffer
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was written correctly to the device.
    EFI_WRITE_PROTECTED   - The device can not be written to.
    EFI_DEVICE_ERROR      - The device reported an error while performing the write.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_INVALID_PARAMETER - The write request contains device addresses that are not 
                            valid for the device.

--*/
;


#define EFI_DISK_IO_PROTOCOL_REVISION 0x00010000

typedef struct _EFI_DISK_IO_PROTOCOL {
  UINT64              Revision;
  EFI_DISK_READ       ReadDisk;
  EFI_DISK_WRITE      WriteDisk;
} EFI_DISK_IO_PROTOCOL;


extern EFI_GUID gEfiDiskIoProtocolGuid;

#endif
