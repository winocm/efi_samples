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

  UgaIo.c

Abstract:

  UGA IO protocol from the EFI 1.1 specification.

  Abstraction of a very simple graphics device.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION (UgaIo)


EFI_GUID gEfiUgaIoProtocolGuid = EFI_UGA_IO_PROTOCOL_GUID;

EFI_GUID_STRING (&gEfiUgaIoProtocolGuid, "UGA Protocol", "EFI 1.1 UGA Protocol");

