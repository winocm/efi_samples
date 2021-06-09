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
  
    UgaSplash.h

Abstract:

  UGA Splash screen protocol.

  Abstraction of a very simple graphics device.

--*/

#ifndef __UGA_SPLASH_H__
#define __UGA_SPLASH_H__

#include EFI_PROTOCOL_DEFINITION (UgaDraw)


#define EFI_UGA_SPLASH_PROTOCOL_GUID \
  { 0xa45b3a0d, 0x2e55, 0x4c03, 0xad, 0x9c, 0x27, 0xd4, 0x82, 0xb, 0x50, 0x7e }

typedef struct _EFI_UGA_SPLASH_PROTOCOL   EFI_UGA_SPLASH_PROTOCOL;


typedef struct _EFI_UGA_SPLASH_PROTOCOL {
  UINT32          PixelWidth;
  UINT32          PixelHeight;
  EFI_UGA_PIXEL   *Image;
} EFI_UGA_SPLASH_PROTOCOL;

extern EFI_GUID gEfiUgaSplashProtocolGuid;

#endif
