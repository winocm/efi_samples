#ifndef _BIOS32_H
#define _BIOS32_H
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

    Bios32.h

Abstract:

    Header defines for Bios32 build tip

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlDriver.h"

#include "int86.h"
#include "PlLegacyBoot.h"

VOID
EFIFirmwareBanner (
    VOID
    );

VOID
PlPrintLogonBanner (
    VOID
    );


extern EFI_PLATFORM_TABLE   PlTable;
extern EFI_FIRMWARE_TABLE   *FW;
extern EFI_DEVICE_PATH      BiosRootDevicePath[];

#endif
