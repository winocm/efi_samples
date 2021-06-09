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
  
    GlobalVariable.c
    
Abstract:

  GUID for EFI (NVRAM) Variables. Defined in EFI 1.0.

--*/

#include "Efi.h"
#include EFI_GUID_DEFINITION(GlobalVariable)

EFI_GUID gEfiGlobalVariableGuid = EFI_GLOBAL_VARIABLE_GUID;

EFI_GUID_STRING(&gEfiGlobalVariableGuid, "Efi", "Efi Variable GUID")

