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
  
    Gpt.h
    
Abstract:

  Guids used for the GPT as defined in EFI 1.0

  GPT defines a new disk partitioning scheme and also describes 
  usage of the legacy Master Boot Record (MBR) partitioning scheme. 

--*/

#ifndef _GPT_GUID_H_
#define _GPT_GUID_H_

#define EFI_PART_TYPE_UNUSED_GUID   \
  { 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
    
#define EFI_PART_TYPE_EFI_SYSTEM_PART_GUID  \
  { 0xc12a7328, 0xf81f, 0x11d2, 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b }

#define EFI_PART_TYPE_LEGACY_MBR_GUID   \
  { 0x024dee41, 0x33e7, 0x11d3, 0x9d, 0x69, 0x00, 0x08, 0xc7, 0x81, 0xf3, 0x9f }

extern EFI_GUID gEfiPartTypeUnusedGuid;
extern EFI_GUID gEfiPartTypeSystemPartGuid;
extern EFI_GUID gEfiPartTypeLegacyMbrGuid;

#endif
