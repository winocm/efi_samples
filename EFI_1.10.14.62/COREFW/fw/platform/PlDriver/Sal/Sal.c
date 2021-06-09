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

    int.c

Abstract:

    Main SAL interface routine for IA-64 calls. 


Revision History

--*/


#include "efi.h"
#include "efilib.h"
#include "SalEfi.h"
#include "SalProc.h"
#include "PlSal.h"

VOID
PlInitSalPalProc (
    IN  UINT64  SALCallBackEntry,
    IN  UINT64  SALCallBackGP
    )
{
    UINT64  PalEntry;
    
    LibInitSalAndPalProc(&RtGlobalSalProcEntry, &PalEntry);

    RtGlobalSALCallBack.ProcEntryPoint =    SALCallBackEntry;
    RtGlobalSALCallBack.GP =                SALCallBackGP;
    PlInitSalIVA();
}

VOID
SalProc (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    )
{
    PFN_SAL_PROC    SalProc;
    rArg            ReturnValue;

    if (RtGlobalSalProcEntry.ProcEntryPoint == 0) {
        return;
    }
    SalProc = (PFN_SAL_PROC)&RtGlobalSalProcEntry;
    ReturnValue = SalProc (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    if (Results) {
        CopyMem (Results, &ReturnValue, sizeof(rArg));
    }
}

#pragma RUNTIME_CODE(RtSalCallBack) 
VOID
RUNTIMEFUNCTION
RtSalCallBack (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    )
{
    PFN_SAL_PROC    SalProc;
    rArg            ReturnValue;

    if (RtGlobalSALCallBack.ProcEntryPoint == 0) {
        return;
    }
    SalProc = (PFN_SAL_PROC)&RtGlobalSALCallBack;
    ReturnValue = SalProc (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    if (Results) {
        CopyMem (Results, &ReturnValue, sizeof(rArg));
    }
}

VOID
SalCallBack (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    )
{
    RtSalCallBack (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Results);
}
