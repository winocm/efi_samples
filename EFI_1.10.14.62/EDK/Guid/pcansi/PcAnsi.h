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
  
    PcAnsi.h
    
Abstract:

  Terminal Device Path Vendor Guid. Defined in EFI 1.0.

--*/

#ifndef _PC_ANSI_H_
#define _PC_ANSI_H_

#define EFI_PC_ANSI_GUID  \
  { 0xe0c14753, 0xf9be, 0x11d2, 0x9a, 0x0c, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d }
  
#define EFI_VT_100_GUID  \
    { 0xdfa66065, 0xb419, 0x11d3,  0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d  }

#define EFI_VT_100_PLUS_GUID  \
  { 0x7baec70b, 0x57e0, 0x4c76, 0x8e, 0x87, 0x2f, 0x9e, 0x28, 0x08, 0x83, 0x43 }
  
#define EFI_VT_UTF8_GUID  \
  { 0xad15a0d6, 0x8bec, 0x4acf, 0xa0, 0x73, 0xd0, 0x1d, 0xe7, 0x7e, 0x2d, 0x88 }

extern EFI_GUID gEfiPcAnsiGuid;
extern EFI_GUID gEfiVT100Guid;
extern EFI_GUID gEfiVT100PlusGuid;
extern EFI_GUID gEfiVTUTF8Guid;

#endif
