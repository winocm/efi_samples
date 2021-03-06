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

  LoadFile.h

Abstract:

  Load File protocol as defined in the EFI 1.0 specification.

  Load file protocol exists to supports the addition of new boot devices, 
  and to support booting from devices that do not map well to file system. 
  Network boot is done via a LoadFile protocol.

  EFI 1.0 can boot from any device that produces a LoadFile protocol.

--*/

#ifndef _LOAD_FILE_H_
#define _LOAD_FILE_H_

#define LOAD_FILE_PROTOCOL_GUID \
  { 0x56EC3091, 0x954C, 0x11d2, 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }

EFI_INTERFACE_DECL(_EFI_LOAD_FILE_PROTOCOL);

typedef
EFI_STATUS
(EFIAPI *EFI_LOAD_FILE) (
  IN struct _EFI_LOAD_FILE_PROTOCOL   *This,
  IN EFI_DEVICE_PATH_PROTOCOL         *FilePath,
  IN BOOLEAN                          BootPolicy,
  IN OUT UINTN                        *BufferSize,
  IN VOID                             *Buffer OPTIONAL
  )
/*++

  Routine Description:
    Causes the driver to load a specified file.

  Arguments:
    This     - Protocol instance pointer.
    FilePath - The device specific path of the file to load.
    BootPolicy - If TRUE, indicates that the request originates from the 
                 boot manager is attempting to load FilePath as a boot
                 selection. If FALSE, then FilePath must match as exact file
                 to be loaded.
    BufferSize - On input the size of Buffer in bytes. On output with a return
                  code of EFI_SUCCESS, the amount of data transferred to 
                  Buffer. On output with a return code of EFI_BUFFER_TOO_SMALL,
                  the size of Buffer required to retrieve the requested file.
    Buffer     - The memory buffer to transfer the file to. IF Buffer is NULL,
                  then no the size of the requested file is returned in 
                  BufferSize.

  Returns:
    EFI_SUCCES      - The file was loaded.
    EFI_UNSUPPORTED - The device does not support the provided BootPolicy
    EFI_INVALID_PARAMETER - FilePath is not a valid device path, or 
                             BufferSize is NULL.
    EFI_NO_SUCH_MEDIA - No medium was present to load the file.
    EFI_DEVICE_ERROR  - The file was not loaded due to a device error.
    EFI_NO_RESPONSE - The remote system did not respond.
    EFI_NOT_FOUND   - The file was not found
    EFI_ABORTED - The file load process was manually cancelled.

--*/
;

typedef struct _EFI_LOAD_FILE_PROTOCOL {
  EFI_LOAD_FILE       LoadFile;
} EFI_LOAD_FILE_PROTOCOL;

extern EFI_GUID gEfiLoadFileProtocolGuid;

#endif

