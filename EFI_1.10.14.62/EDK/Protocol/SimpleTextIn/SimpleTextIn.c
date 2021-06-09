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

  SimpleTextIn.c

Abstract:

  Simple Text In protocol from the EFI 1.0 specification.

  Abstraction of a very simple input device like a keyboard or serial
  terminal.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(SimpleTextIn)


EFI_GUID gEfiSimpleTextInProtocolGuid = EFI_SIMPLE_TEXT_IN_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiSimpleTextInProtocolGuid, "Simple Text In Protocol", "EFI 1.0 Simple Text In Protocol");
