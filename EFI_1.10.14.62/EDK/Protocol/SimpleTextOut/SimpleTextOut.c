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

  SimpleTextOut.c

Abstract:

  Simple Text Out protocol from the EFI 1.0 specification.

  Abstraction of a very simple text based output device like VGA text mode or
  a serial terminal.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(SimpleTextOut)


EFI_GUID gEfiSimpleTextOutProtocolGuid = EFI_SIMPLE_TEXT_OUT_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiSimpleTextOutProtocolGuid, "Simple Text Out Protocol", "EFI 1.0 Simple Text Out Protocol");

