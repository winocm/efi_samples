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

    data.c

Abstract:

    Global data in the Bios Platform code



Revision History

--*/


#include "ia-32emb.h"


//
// Globals
//

//
// PlTable - Platform table.  Provides the core firwmare component
// callouts to the environment.
//

EFI_PLATFORM_TABLE   PlTable;

//
// FW - Internal firmware table.  Provides the emulator with intenral firmware
// functions.  (mostly for firmware initialization)
//

EFI_FIRMWARE_TABLE   *FW;

//
// BiosRootDevicePath - The path to install the default device_io device
//

EFI_DEVICE_PATH BiosRootDevicePath[] = {
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, END_DEVICE_PATH_LENGTH 
};

