#ifndef _CACHE_FLUSH_H
#define _CACHE_FLUSH_H
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

    CacheFlush.h

Abstract:

    Flush cache to maintain coherence in L0 I cace. The L0 I does
    not maintain coherency with the L0 D. So manual flushing is 
    required after loading more moving code in memory.


Revision History

--*/

VOID
RUNTIMEFUNCTION
RtPlCacheFlush (
    IN  VOID    *StartAddress,
    IN  VOID    *EndAddress
    );

#endif