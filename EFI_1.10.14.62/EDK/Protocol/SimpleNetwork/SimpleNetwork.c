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

  SimpleNetwork.c

Abstract:

  Simple Network protocol as defined in the EFI 1.0 specification.

  Basic network device abstraction.

  Rx    - Received
  Tx    - Transmit
  MCast - MultiCast
  ...

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(SimpleNetwork)


EFI_GUID gEfiSimpleNetworkProtocolGuid = EFI_SIMPLE_NETWORK_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiSimpleNetworkProtocolGuid, "Simple Network Protocol", "EFI 1.0 Simple Network Protocol");
