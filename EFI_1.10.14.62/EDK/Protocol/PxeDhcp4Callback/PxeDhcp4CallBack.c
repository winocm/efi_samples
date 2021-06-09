/*++

Copyright (c) 1999 - 2002 Intel Corporation.  All rights reserved.

This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  PxeDhcp4Callback.c

Abstract:
  PxeDhcp4Callback protocol GUID definition.

--*/

#include "Efi.h"

#include EFI_PROTOCOL_DEFINITION(PxeDhcp4CallBack)

EFI_GUID gEfiPxeDhcp4CallbackProtocolGuid =
  EFI_PXE_DHCP4_CALLBACK_PROTOCOL_GUID;

EFI_GUID_STRING(
  &gEfiPxeDhcp4CallbackProtocolGuid,
  "PXE DHCP4 Callback Protocol",
  "EFI 1.1 PXE DHCP IPv4 Callback Protocol");

/* EOF - PxeDhcp4Callback.c */
