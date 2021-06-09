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

  PlatformDriverOverride.c

Abstract:

  Platform Driver Override protocol as defined in the EFI 1.1 specification.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(PlatformDriverOverride)

EFI_GUID gEfiPlatformDriverOverrideProtocolGuid = EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiPlatformDriverOverrideProtocolGuid, "Platform Driver Override Protocol", "EFI 1.1 Platform Driver Override Protocol");
