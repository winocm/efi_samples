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

  ConsoleInDevice.c
    
Abstract:


--*/

#include "Efi.h"
#include EFI_GUID_DEFINITION(ConsoleInDevice)


EFI_GUID  gEfiConsoleInDeviceGuid = EFI_CONSOLE_IN_DEVICE_GUID;

EFI_GUID_STRING(&gEfiConsoleInDeviceGuid, "Console In Device Guid", "EFI Conosle In Device Guid");
