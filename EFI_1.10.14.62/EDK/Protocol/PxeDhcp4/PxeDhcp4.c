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
  PxeDhcp4.c

Abstract:
  PxeDhcp4 GUID declaration

--*/

#include "Efi.h"

#include EFI_PROTOCOL_DEFINITION(PxeDhcp4)

EFI_GUID gEfiPxeDhcp4ProtocolGuid = EFI_PXE_DHCP4_PROTOCOL_GUID;

EFI_GUID_STRING(
  &gEfiPxeDhcp4ProtocolGuid,
  "PXE DHCP4 Protocol",
  "EFI 1.1 PXE DHCPv4 Protocol");

/* EOF - PxeDhcp4.c */
