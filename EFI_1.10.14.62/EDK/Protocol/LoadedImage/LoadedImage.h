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

 LoadedImage.h

Abstract:

  EFI 1.0 Loaded image protocol definition.

  Every EFI driver and application is passed an image handle when it is loaded.
  This image handle will contain a Loaded Image Protocol.

--*/
#ifndef _LOADED_IMAGE_H_
#define _LOADED_IMAGE_H_

#define EFI_LOADED_IMAGE_PROTOCOL_GUID  \
  { 0x5B1B31A1, 0x9562, 0x11d2, 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }


//
// EFI_SYSTEM_TABLE & EFI_IMAGE_UNLOAD are defined in EfiApi.h
//

#define EFI_LOADED_IMAGE_INFORMATION_REVISION      0x1000

typedef struct {
  UINT32                          Revision;
  EFI_HANDLE                      ParentHandle;
  EFI_SYSTEM_TABLE                *SystemTable;

  //
  // Source location of image
  //
  EFI_HANDLE                      DeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL        *FilePath;
  VOID                            *Reserved;

  //
  // Images load options
  //
  UINT32                          LoadOptionsSize;
  VOID                            *LoadOptions;

  //
  // Location of where image was loaded
  //
  VOID                            *ImageBase;
  UINT64                          ImageSize;
  EFI_MEMORY_TYPE                 ImageCodeType;
  EFI_MEMORY_TYPE                 ImageDataType;

  //
  // If the driver image supports a dynamic unload request
  //
  EFI_IMAGE_UNLOAD                Unload;

} EFI_LOADED_IMAGE_PROTOCOL;


extern EFI_GUID gEfiLoadedImageProtocolGuid;

#endif
