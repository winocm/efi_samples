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

  SimpleFileSystem.h

Abstract:

  SimpleFileSystem protocol as defined in the EFI 1.0 specification.

  The SimpleFileSystem protocol is the programatic access to the FAT (12,16,32) 
  file system specified in EFI 1.0. It can also be used to abstract a file  
  system other than FAT.

  EFI 1.0 can boot from any valid EFI image contained in a SimpleFileSystem
 
--*/

#ifndef _SIMPLE_FILE_SYSTEM_H_
#define _SIMPLE_FILE_SYSTEM_H_

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
  { 0x964e5b22, 0x6459, 0x11d2, 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

EFI_INTERFACE_DECL(_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL);
EFI_INTERFACE_DECL(_EFI_FILE);

typedef
EFI_STATUS
(EFIAPI *EFI_VOLUME_OPEN) (
  IN struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *This,
  OUT struct _EFI_FILE                   **Root
  )
/*++

  Routine Description:
    Open the root directory on a volume.

  Arguments:
    This - Protocol instance pointer.
    Root - Returns an Open file handle for the root directory

  Returns:
    EFI_SUCCES           - The device was opened.
    EFI_UNSUPPORTED      - This volume does not suppor the file system.
    EFI_NO_MEDIA         - The device has no media.
    EFI_DEVICE_ERROR     - The device reported an error.
    EFI_VOLUME_CORRUPTED - The file system structures are corrupted
    EFI_ACCESS_DENIED    - The service denied access to the file
    EFI_OUT_OF_RESOURCES - The volume was not opened due to lack of resources

--*/
;

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION   0x00010000

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
  UINT64                  Revision;
  EFI_VOLUME_OPEN         OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;


typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
  IN struct _EFI_FILE  *File,
  OUT struct _EFI_FILE **NewHandle,
  IN CHAR16                   *FileName,
  IN UINT64                   OpenMode,
  IN UINT64                   Attributes
  )
/*++

  Routine Description:
    Open the root directory on a volume.

  Arguments:
    File       - Protocol instance pointer.
    NewHandle  - Returns File Handle for FileName
    FileName   - Null terminated string. "\", ".", and ".." are supported
    OpenMode   - Open mode for file.
    Attributes - Only used for EFI_FILE_MODE_CREATE

  Returns:
    EFI_SUCCES           - The device was opened.
    EFI_NOT_FOUND        - The specified file could not be found on the device
    EFI_NO_MEDIA         - The device has no media.
    EFI_MEDIA_CHANGED    - The media has changed
    EFI_DEVICE_ERROR     - The device reported an error.
    EFI_VOLUME_CORRUPTED - The file system structures are corrupted
    EFI_ACCESS_DENIED    - The service denied access to the file
    EFI_OUT_OF_RESOURCES - The volume was not opened due to lack of resources
    EFI_VOLUME_FULL      - The volume is full.

--*/
;

// Open modes
#define EFI_FILE_MODE_READ      0x0000000000000001
#define EFI_FILE_MODE_WRITE     0x0000000000000002
#define EFI_FILE_MODE_CREATE    0x8000000000000000

// File attributes
#define EFI_FILE_READ_ONLY      0x0000000000000001
#define EFI_FILE_HIDDEN         0x0000000000000002
#define EFI_FILE_SYSTEM         0x0000000000000004
#define EFI_FILE_RESERVED       0x0000000000000008
#define EFI_FILE_DIRECTORY      0x0000000000000010
#define EFI_FILE_ARCHIVE        0x0000000000000020
#define EFI_FILE_VALID_ATTR     0x0000000000000037

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_CLOSE) (
  IN struct _EFI_FILE  *File
  )
/*++

  Routine Description:
    Close the file handle

  Arguments:
    File       - Protocol instance pointer.

  Returns:
    EFI_SUCCES - The device was opened.

--*/
;


typedef
EFI_STATUS
(EFIAPI *EFI_FILE_DELETE) (
  IN struct _EFI_FILE  *File
  )
/*++

  Routine Description:
    Close and delete the file handle

  Arguments:
    File       - Protocol instance pointer.

  Returns:
    EFI_SUCCES              - The device was opened.
    EFI_WARN_DELETE_FAILURE - The handle was closed but the file was not 
                              deleted

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) (
  IN struct _EFI_FILE  *File,
  IN OUT UINTN                *BufferSize,
  OUT VOID                    *Buffer
  )
/*++

  Routine Description:
    Read data from the file.

  Arguments:
    File       - Protocol instance pointer.
    BufferSize - On input size of buffer, on output amount of data in 
                 buffer.
    Buffer     - The buffer in which data is read.

  Returns:
    EFI_SUCCES           - Data was read.
    EFI_NO_MEDIA         - The device has no media
    EFI_DEVICE_ERROR     - The device reported an error
    EFI_VOLUME_CORRUPTED - The file system structures are corrupted
    EFI_BUFFER_TO_SMALL  - BufferSize is too small. BufferSize contains 
                           required size

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_WRITE) (
  IN struct _EFI_FILE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  )
/*++

  Routine Description:
    Write data from to the file.

  Arguments:
    File       - Protocol instance pointer.
    BufferSize - On input size of buffer, on output amount of data in buffer.
    Buffer     - The buffer in which data to write.

  Returns:
    EFI_SUCCES           - Data was written.
    EFI_UNSUPPORT        - Writes to Open directory are not supported
    EFI_NO_MEDIA         - The device has no media
    EFI_DEVICE_ERROR     - The device reported an error
    EFI_VOLUME_CORRUPTED - The file system structures are corrupted
    EFI_WRITE_PROTECTED  - The device is write protected
    EFI_ACCESS_DENIED    - The file was open for read only
    EFI_VOLUME_FULL      - The volume is full

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_POSITION) (
  IN struct _EFI_FILE  *File,
  IN UINT64                   Position
  )
/*++

  Routine Description:
    Set a files current position

  Arguments:
    File     - Protocol instance pointer.
    Position - Byte possition from the start of the file

  Returns:
    EFI_SUCCES      - Data was written.
    EFI_UNSUPPORTED - Seek request for non-zero is not valid on open.

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_POSITION) (
  IN struct _EFI_FILE  *File,
  OUT UINT64                  *Position
  )
/*++

  Routine Description:
    Get a files current position

  Arguments:
    File     - Protocol instance pointer.
    Position - Byte possition from the start of the file

  Returns:
    EFI_SUCCES      - Data was written.
    EFI_UNSUPPORTED - Seek request for non-zero is not valid on open.

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_INFO) (
  IN struct _EFI_FILE  *File,
  IN EFI_GUID                 *InformationType,
  IN OUT UINTN                *BufferSize,
  OUT VOID                    *Buffer
  )
/*++

  Routine Description:
    Get information about a file

  Arguments:
    File            - Protocol instance pointer.
    InformationType - Type of info to return in Buffer
    BufferSize      - On input size of buffer, on output amount of data in
                      buffer.
    Buffer          - The buffer to return data.

  Returns:
    EFI_SUCCES           - Data was returned.
    EFI_UNSUPPORT        - InformationType is not supported
    EFI_NO_MEDIA         - The device has no media
    EFI_DEVICE_ERROR     - The device reported an error
    EFI_VOLUME_CORRUPTED - The file system structures are corrupted
    EFI_WRITE_PROTECTED  - The device is write protected
    EFI_ACCESS_DENIED    - The file was open for read only
    EFI_BUFFER_TOO_SMALL - Buffer was too small, required size returned in 
                           BufferSize
--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_INFO) (
  IN struct _EFI_FILE  *File,
  IN EFI_GUID                 *InformationType,
  IN UINTN                    BufferSize,
  IN VOID                     *Buffer
  )
/*++

  Routine Description:
    Set information about a file

  Arguments:
    File            - Protocol instance pointer.
    InformationType - Type of info in Buffer
    BufferSize      - Size of buffer.
    Buffer          - The data to write.

  Returns:
    EFI_SUCCES           - Data was returned.
    EFI_UNSUPPORT        - InformationType is not supported
    EFI_NO_MEDIA         - The device has no media
    EFI_DEVICE_ERROR     - The device reported an error
    EFI_VOLUME_CORRUPTED - The file system structures are corrupted
    EFI_WRITE_PROTECTED  - The device is write protected
    EFI_ACCESS_DENIED    - The file was open for read only
--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_FLUSH) (
  IN struct _EFI_FILE  *File
  )
/*++

  Routine Description:
    Flush data back for the file handle

  Arguments:
    File  - Protocol instance pointer.

  Returns:
    EFI_SUCCES           - Data was written.
    EFI_UNSUPPORT        - Writes to Open directory are not supported
    EFI_NO_MEDIA         - The device has no media
    EFI_DEVICE_ERROR     - The device reported an error
    EFI_VOLUME_CORRUPTED - The file system structures are corrupted
    EFI_WRITE_PROTECTED  - The device is write protected
    EFI_ACCESS_DENIED    - The file was open for read only
    EFI_VOLUME_FULL      - The volume is full

--*/
;

#define EFI_FILE_HANDLE_REVISION         0x00010000
typedef struct _EFI_FILE {
  UINT64                  Revision;
  EFI_FILE_OPEN           Open;
  EFI_FILE_CLOSE          Close;
  EFI_FILE_DELETE         Delete;
  EFI_FILE_READ           Read;
  EFI_FILE_WRITE          Write;
  EFI_FILE_GET_POSITION   GetPosition;
  EFI_FILE_SET_POSITION   SetPosition;
  EFI_FILE_GET_INFO       GetInfo;
  EFI_FILE_SET_INFO       SetInfo;
  EFI_FILE_FLUSH          Flush;
} EFI_FILE;

//
// File information types
//

#define EFI_FILE_INFO_ID   \
  { 0x9576e92, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

typedef struct {
  UINT64                  Size;
  UINT64                  FileSize;
  UINT64                  PhysicalSize;
  EFI_TIME                CreateTime;
  EFI_TIME                LastAccessTime;
  EFI_TIME                ModificationTime;
  UINT64                  Attribute;
  CHAR16                  FileName[1];
} EFI_FILE_INFO;

//
// The FileName field of the EFI_FILE_INFO data structure is variable length.
// Whenever code needs to know the size of the EFI_FILE_INFO data structure, it needs to
// be the size of the data structure without the FileName field.  The following macro 
// computes this size correctly no matter how big the FileName array is declared.
// This is required to make the EFI_FILE_INFO data structure ANSI compilant. 
//
#define SIZE_OF_EFI_FILE_INFO EFI_FIELD_OFFSET(EFI_FILE_INFO,FileName)

#define EFI_FILE_SYSTEM_INFO_ID_GUID  \
  { 0x9576e93, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

typedef struct {
  UINT64                  Size;
  BOOLEAN                 ReadOnly;
  UINT64                  VolumeSize;
  UINT64                  FreeSpace;
  UINT32                  BlockSize;
  CHAR16                  VolumeLabel[1];
} EFI_FILE_SYSTEM_INFO;

//
// The VolumeLabel field of the EFI_FILE_SYSTEM_INFO data structure is variable length.
// Whenever code needs to know the size of the EFI_FILE_SYSTEM_INFO data structure, it needs
// to be the size of the data structure without the VolumeLable field.  The following macro 
// computes this size correctly no matter how big the VolumeLable array is declared.
// This is required to make the EFI_FILE_SYSTEM_INFO data structure ANSI compilant. 
//
#define SIZE_OF_EFI_FILE_SYSTEM_INFO EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_INFO,VolumeLabel)

#define EFI_FILE_SYSTEM_VOLUME_LABEL_INFO_ID_GUID \
  { 0xDB47D7D3,0xFE81, 0x11d3, 0x9A, 0x35, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }

typedef struct {
  CHAR16                  VolumeLabel[1];
} EFI_FILE_SYSTEM_VOLUME_LABEL_INFO;

#define SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_VOLUME_LABEL_INFO,VolumeLabel)

extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;

extern EFI_GUID gEfiFileInfoGuid;
extern EFI_GUID gEfiFileInfoIdGuid;
extern EFI_GUID gEfiFileSystemVolumeLabelInfoIdGuid;

#endif
