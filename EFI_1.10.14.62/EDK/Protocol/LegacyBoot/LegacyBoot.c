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

  LegacyBoot.C

Abstract:


Revision History

--*/
#ifndef _LEGACY_BOOT_C_
#define _LEGACY_BOOT_C_

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(LegacyBoot)


EFI_GUID gEfiLegacyBootProtocolGuid = EFI_LEGACY_BOOT_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiLegacyBootProtocolGuid, "Legacy Boot Protocol", "EFI Legacy Boot Protocol");

#endif
