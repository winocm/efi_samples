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

    CacheFlush.c

Abstract:

    Flush cache to maintain coherence in after loading a PE image.
    Not needed on IA-64 systems


Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlCacheFlush.h"

#pragma RUNTIME_CODE(RtPlCacheFlush)
VOID
RUNTIMEFUNCTION
RtPlCacheFlush (
    IN  VOID    *StartAddress,
    IN  VOID    *EndAddress
    )
{
    return;
}
