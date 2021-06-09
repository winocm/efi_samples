/*++

Copyright (c) 2001 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DebugImageInfoTable.c
    
Abstract:

  GUID used to locate the Debug Image Info table in the EFI 1.0 system table.

--*/

#include "Efi.h"
#include EFI_GUID_DEFINITION(DebugImageInfoTable)


EFI_GUID  gEfiDebugImageInfoTableGuid = EFI_DEBUG_IMAGE_INFO_TABLE_GUID;

EFI_GUID_STRING(&gEfiDebugImageInfoTableGuid, "Debug Image Info Table", "Debug Image Info Table GUID in EFI System Table");