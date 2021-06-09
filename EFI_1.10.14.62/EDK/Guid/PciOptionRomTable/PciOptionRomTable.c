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

  PciOptionRomTable.c
    
Abstract:

  GUID and data structure used to describe the list of PCI Option ROMs present in a system.

--*/

#include "Efi.h"
#include EFI_GUID_DEFINITION(PciOptionRomTable)

EFI_GUID gEfiPciOptionRomTableGuid = EFI_PCI_OPTION_ROM_TABLE_GUID;

EFI_GUID_STRING(&gEfiPciOptionRomTableGuid, "PCI Option ROM Table", "PCI Option ROM Table GUID in EFI System Table");
