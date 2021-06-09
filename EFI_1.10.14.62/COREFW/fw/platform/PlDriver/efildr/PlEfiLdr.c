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

    EfiLdr.c

Abstract:

    Routine to transition to runtime

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlEfiLdr.h"

#pragma BEGIN_RUNTIME_DATA()
STATIC EFILDR_CALLBACK  *LocalEfiLdrCallBack = NULL;

VOID
PlInitEfiLdrCallBack (
    IN EFILDR_CALLBACK*EfiLdrCallBackEntry
    )
{
    LocalEfiLdrCallBack = EfiLdrCallBackEntry;
}

#pragma RUNTIME_CODE(RtPlEfiLdrCallBack) 
VOID RUNTIMEFUNCTION
RtPlEfiLdrCallBack (
    UINTN FuncId,
    UINTN parm1,
    UINTN parm2,
    UINTN parm3
    )
{
  if (LocalEfiLdrCallBack != NULL) {
      LocalEfiLdrCallBack(FuncId, parm1, parm2, parm3);
  }
}
