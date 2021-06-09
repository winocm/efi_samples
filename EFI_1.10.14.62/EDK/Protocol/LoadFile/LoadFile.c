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

  LoadFile.c

Abstract:

  Load File protocol as defined in the EFI 1.0 specification.

  Load file protocol exists to supports the addition of new boot devices, 
  and to support booting from devices that do not map well to file system. 
  Network boot is done via a LoadFile protocol.

  EFI 1.0 can boot from any device that produces a LoadFile protocol.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(LoadFile)

EFI_GUID gEfiLoadFileProtocolGuid = LOAD_FILE_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiLoadFileProtocolGuid, "LoadFile Protocol", "EFI 1.0 Load File Protocol");
