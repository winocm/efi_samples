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

  Bis.c

Abstract:

  This file defines the Bis protocol

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION (Bis)

EFI_GUID gEfiBisProtocolGuid = EFI_BIS_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiBisProtocolGuid, "BIS Base Protocol", "BIS Base protocol");

