/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    VgaMiniPort.c
    
Abstract:

    EFI VGA Mini Port Protocol

Revision History

*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION (VgaMiniPort)

EFI_GUID gEfiVgaMiniPortProtocolGuid = EFI_VGA_MINI_PORT_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiVgaMiniPortProtocolGuid, "VGA Mini Port Protocol", "EFI VGA Mini Port Protocol");

