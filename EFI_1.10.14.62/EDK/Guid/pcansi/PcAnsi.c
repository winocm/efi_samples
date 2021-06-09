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
  
    PcAnsi.c
    
Abstract:

  Terminal Device Path Vendor Guid. Defined in EFI 1.0.

--*/

#include "Efi.h"

#include EFI_GUID_DEFINITION(PcAnsi)

EFI_GUID gEfiPcAnsiGuid     = EFI_PC_ANSI_GUID;
EFI_GUID gEfiVT100Guid      =  EFI_VT_100_GUID;
EFI_GUID gEfiVT100PlusGuid  =  EFI_VT_100_PLUS_GUID;
EFI_GUID gEfiVTUTF8Guid     =  EFI_VT_UTF8_GUID;

EFI_GUID_STRING(&gEfiPcAnsiGuid, "Efi", "Efi PC ANSI Device Path Vendor GUID")
EFI_GUID_STRING(&gEfiVT100Guid, "Efi", "Efi VT100 Device Path Vendor GUID")
EFI_GUID_STRING(&gEfiVT100PlusGuid, "Efi", "Efi VT100Plus Device Path Vendor GUID")
EFI_GUID_STRING(&gEfiVTUTF8Guid, "Efi", "Efi VTUTF8 Device Path Vendor GUID")

