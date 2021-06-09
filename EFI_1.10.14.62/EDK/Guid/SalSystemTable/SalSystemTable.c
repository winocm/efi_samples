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
  
    SalSystemTable.c
    
Abstract:

  GUIDs used for SAL system table entries in the in the EFI 1.0 system table.

  SAL System Table contains Itanium-based processor centric information about
  the system.

--*/

#include "Efi.h"
#include EFI_GUID_DEFINITION(SalSystemTable)


EFI_GUID gEfiSalSystemTableGuid = EFI_SAL_SYSTEM_TABLE_GUID;

EFI_GUID_STRING(&gEfiSalSystemTableGuid, "SAL System Table", "SAL System Table GUID in EFI System Table");
