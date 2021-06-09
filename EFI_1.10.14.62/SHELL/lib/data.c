
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

  data.c

Abstract:

  Shell library global data



Revision History

--*/

#include "shelllib.h"

//
//
//
EFI_SHELL_INTERFACE     *SI;
EFI_SHELL_ENVIRONMENT   *SE;

//
//
//
EFI_GUID ShellInterfaceProtocol = SHELL_INTERFACE_PROTOCOL;
EFI_GUID ShellEnvProtocol = SHELL_ENVIRONMENT_INTERFACE_PROTOCOL;

//
//
//
CHAR16 *ShellLibMemoryTypeDesc[EfiMaxMemoryType] = {
            L"reserved  ",
            L"LoaderCode",
            L"LoaderData",
            L"BS_code   ",
            L"BS_data   ",
            L"RT_code   ",
            L"RT_data   ",
            L"available ",
            L"Unusable  ",
            L"ACPI_recl ",
            L"ACPI_NVS  ",
            L"MemMapIO  ",
            L"MemPortIO ",
            L"PAL_code  "
  };
