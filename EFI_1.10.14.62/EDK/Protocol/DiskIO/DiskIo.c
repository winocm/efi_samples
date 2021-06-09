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
  
    DiskIo.c

Abstract:

  Disk IO protocol as defined in the EFI 1.0 specification.

  The Disk IO protocol is used to convert block oriented devices into byte
  oriented devices. The Disk IO protocol is intended to layer on top of the
  Block IO protocol.
 
--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(DiskIo)

EFI_GUID gEfiDiskIoProtocolGuid = EFI_DISK_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiDiskIoProtocolGuid, "DiskIo Protocol", "EFI 1.0 Disk IO Protocol");
