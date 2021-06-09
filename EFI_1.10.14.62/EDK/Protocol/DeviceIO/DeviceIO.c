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

  DeviceIo.c

Abstract:

  Device IO protocol as defined in the EFI 1.0 specification.

  Device IO is used to abstract hardware access to devices. It includes
  memory mapped IO, IO, PCI Config space, and DMA.

 
--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(DeviceIo)


EFI_GUID gEfiDeviceIoProtocolGuid = EFI_DEVICE_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiDeviceIoProtocolGuid, "DeviceIo Protocol", "EFI 1.0 Device IO Protocol");
