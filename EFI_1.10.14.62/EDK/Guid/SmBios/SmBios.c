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
  
    SmBios.c
    
Abstract:

  GUIDs used to locate the SMBIOS tables in the EFI 1.0 system table.

  This GUID in the system table is the only legal way to search for and 
  locate the SMBIOS tables. Do not search the 0xF0000 segment to find SMBIOS
  tables.

--*/

#include "Efi.h"
#include EFI_GUID_DEFINITION(Smbios)


EFI_GUID  gEfiSmbiosTableGuid = EFI_SMBIOS_TABLE_GUID;

EFI_GUID_STRING(&gEfiSmbiosTableGuid, "SMBIOS Table", "SMBIOS Table GUID in EFI System Table");
