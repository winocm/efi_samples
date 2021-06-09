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

    Flush cache to maintain coherence in L0 I cace. The L0 I does
    not maintain coherency with the L0 D. So manual flushing is 
    required after loading more moving code in memory.


Revision History

--*/
#include "efi.h"
#include "efilib.h"
#include "PlCacheFlush.h"

#define ICACHE_LINE_SIZE 32

VOID
RUNTIMEFUNCTION
RtPioICacheFlush (
    IN  VOID    *StartAddress,
    IN  UINTN   SizeInBytes
    );

#pragma RUNTIME_CODE(RtPlCacheFlush)
VOID
RUNTIMEFUNCTION
RtPlCacheFlush (
    IN  VOID    *StartAddress,
    IN  VOID    *EndAddress
    )
{
    ASSERT(StartAddress <= EndAddress);

    RtPioICacheFlush ((UINT8 *)StartAddress, 
                      (UINTN) ((CHAR8 *)EndAddress - (CHAR8 *)StartAddress));
}
