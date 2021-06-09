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

    EfiLdrHandoff.h

Abstract: 

    Handoff from EFILDR to EFI.

Revision History

--*/

#ifndef _EFILDR_HANDOFF_H
#define _EFILDR_HANDOFF_H

#define EFILDR_BASE_SEGMENT 0x2000

#define EFILDR_CB_VA        0x00

typedef 
VOID
EFILDR_CALLBACK (
  UINTN id, 
  UINTN p1, 
  UINTN p2, 
  UINTN p3
  );

typedef struct _EFILDRHANDOFF {
    UINTN                    MemDescCount;
    EFI_MEMORY_DESCRIPTOR   *MemDesc;
    VOID                    *ImageBase;
    UINT64                  ImageSize;
    EFILDR_CALLBACK         *EfiLdrCallBack;
} EFILDRHANDOFF;

#endif
