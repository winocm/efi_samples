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

    Banner.c

Abstract: 

    Platform logon banner.


Revision History

--*/
 
#include "efi.h"
#include "efilib.h"

VOID
PlPrintLogonBanner(
    VOID
    )
{
    Print (L"%HEFI IA-64 SDV/FDK (BIOS CallBacks)%N [%a] - %s\n", __TIMESTAMP__, ST->FirmwareVendor);
}

