/*++

Copyright (c)  1999 - 2001 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Ps2Policy.c
    
Abstract:

  Protocol used for PS/2 Policy definition.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(Ps2Policy)

EFI_GUID gEfiPs2PolicyProtocolGuid = EFI_PS2_POLICY_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiPs2PolicyProtocolGuid, "PS2 Policy", "Policy for Configuring PS2");

