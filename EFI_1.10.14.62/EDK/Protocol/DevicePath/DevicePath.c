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
  
    DevicePath.c

Abstract:

  The device path protocol as defined in EFI 1.0.

  The device path represents a programatic path to a device. It's the view
  from a software point of view. It also must persist from boot to boot, so 
  it can not contain things like PCI bus numbers that change from boot to boot.
  

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(DevicePath)


EFI_GUID gEfiDevicePathProtocolGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiDevicePathProtocolGuid, "Device Path Protocol", "EFI 1.0 Device Path protocol");
