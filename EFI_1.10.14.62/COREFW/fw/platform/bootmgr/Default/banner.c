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

    banner.c

Abstract:

    Banner for EFI Boot Manager



Revision History

--*/

#include "bm.h"


VOID
BmBanner (
    VOID
    )
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *Con;
    CHAR16                          *Banner;

    //
    // Clear the display
    //
    Con = ST->ConOut;
    Con->SetAttribute (Con, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    Con->ClearScreen (Con);

    Banner = BmString (BM_BANNER);
    Print(Banner, ST->Hdr.Revision >> 16, ST->Hdr.Revision & 0xffff, ST->FirmwareRevision >> 16,ST->FirmwareRevision & 0xffff);
}
