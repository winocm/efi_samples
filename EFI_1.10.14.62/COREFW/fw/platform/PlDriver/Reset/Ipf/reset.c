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

    plmisc.c

Abstract: 

    Miss Runtime Pl interfaces that are IA-64 specific


Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "plreset.h"
#include "SalEfi.h"
#include "SalProc.h"

#pragma RUNTIME_CODE(RtPlResetSystem)
STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlResetSystem (
    IN EFI_RESET_TYPE   ResetType,
    IN EFI_STATUS       ResetStatus,
    IN UINTN            DataSize,
    IN CHAR16           *ResetData OPTIONAL
    )
/*++

Routine Description:

    This routine handles reset system calls

Arguments:

    Reset type
    Reason for reset
    Size of data pointed to by ResetData
    Context Data

Returns:
 
    None

--*/
{
    rArg    Results = {-1,0,0,0};
    UINTN   Reset;

    if(ResetType == EfiResetCold)
        Reset = ID_SALCB_RESET_COLD;

    if(ResetType == EfiResetWarm)
        Reset = ID_SALCB_RESET_WARM;

    if(ResetType == EfiResetShutdown)
        Reset = ID_SALCB_RESET_COLD;

    //
    // call SAL callback to issue reset
    //

    RtSalCallBack(
        ID_SALCB_RESET, Reset, DataSize, (UINTN)ResetData,
        0,0,0,0,
        &Results
    );

    if(Results.p0) {
        return EFI_UNSUPPORTED;
    }

    return EFI_SUCCESS;

}

