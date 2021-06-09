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

  LibGlobals.c

Abstract:

  Globals used in EFI Driver Lib. They are initialized in EfiDriverLib.c.
  Each seperatly linked module has it's own copy of these globals.

  gBS       - Boot Services table pointer
  gRT       - Runt Time services table pointer
  gST       - System Table pointer
  
  gErrorLevel     - Debug error level.

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

EFI_BOOT_SERVICES         *gBS;
EFI_RUNTIME_SERVICES      *gRT;
EFI_SYSTEM_TABLE          *gST;
UINTN                     gErrorLevel = EFI_DBUG_MASK;

