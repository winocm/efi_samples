#ifndef _SAL64_H
#define _SAL64_H
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

    BootBlock.h

Abstract:

    Boot Block include file for IA-64 SAL platform

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "SalEfi.h"
#include "SalProc.h"
#include "PlDriver.h"
#include "PlSal.h"
#include "PlTimer.h"

#include "int86.h"
#include "PlLegacyBoot.h"

//
// Define the timer period for the periodic timer interrupt to be 500 ms
//

#ifdef SOFT_SDV
#define  TIMER_PERIOD   500
#else
#define  TIMER_PERIOD   10
#endif

//
//
//

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
