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

  DebugSupport.c

Abstract:

  DebugSupport protocol as defined in the EFI 1.1 specification.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(DebugSupport)

EFI_GUID  gEfiDebugSupportProtocolGuid = EFI_DEBUG_SUPPORT_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiDebugSupportProtocolGuid, "DebugSupport Protocol", "EFI 1.1 DebugSupport Protocol");
