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
  
    Gpt.c
    
Abstract:

  Guids used for the GPT as defined in EFI 1.0

  GPT defines a new disk partitioning scheme and also describes 
  usage of the legacy Master Boot Record (MBR) partitioning scheme. 

--*/

#include "Efi.h"
#include EFI_GUID_DEFINITION(Gpt)


EFI_GUID gEfiPartTypeUnusedGuid     = EFI_PART_TYPE_UNUSED_GUID;

EFI_GUID_STRING(&gEfiPartTypeUnusedGuid, "G0", "Null Partition Type GUID");


EFI_GUID gEfiPartTypeSystemPartGuid = EFI_PART_TYPE_EFI_SYSTEM_PART_GUID;

EFI_GUID_STRING(&gEfiPartTypeSystemPartGuid, "ESP", "EFI System Partition GUID");


EFI_GUID gEfiPartTypeLegacyMbrGuid  = EFI_PART_TYPE_LEGACY_MBR_GUID;

EFI_GUID_STRING(&gEfiPartTypeLegacyMbrGuid, "Legacy MBR", "Legacy Master Boot Record Partition GUID");
