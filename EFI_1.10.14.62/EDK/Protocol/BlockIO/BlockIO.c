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

  BlockIo.c

Abstract:

  Block IO protocol as defined in the EFI 1.0 specification.

  The Block IO protocol is used to abstract block devices like hard drives,
  DVD-ROMs and floppy drives.

 
--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(BlockIo)
 

EFI_GUID gEfiBlockIoProtocolGuid = EFI_BLOCK_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiBlockIoProtocolGuid, "BlockIo Protocol", "EFI 1.0 Block IO protocol");

