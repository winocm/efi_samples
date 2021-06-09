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

  EfiNetworkInterfaceIdentifier.c

Abstract:


Revision History

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(EfiNetworkInterfaceIdentifier)


EFI_GUID gEfiNetworkInterfaceIdentifierProtocolGuid = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiNetworkInterfaceIdentifierProtocolGuid, "Network Interface Identifier Protocol", "EFI Network Interface Identifier Protocol");

EFI_GUID gEfiNetworkInterfaceIdentifierProtocolGuid_31 = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_GUID_31;

EFI_GUID_STRING(&gEfiNetworkInterfaceIdentifierProtocolGuid_31, "Network Interface Identifier Protocol_31", "EFI1.1 Network Interface Identifier Protocol");
