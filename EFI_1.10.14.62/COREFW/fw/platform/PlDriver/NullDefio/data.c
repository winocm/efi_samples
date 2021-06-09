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

    Global data in the DeviceIo Code



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlDefIo.h"


//
// Globals
//

//
// Global IO Fncs for PC AT class box
//
EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs; 

//
// PCI Root Bus List
//
LIST_ENTRY                  PciRootBusList;

//
// Device Path to Legacy Devices behind ISA Bridge
//
EFI_DEVICE_PATH             *LegacyDevicePath;
