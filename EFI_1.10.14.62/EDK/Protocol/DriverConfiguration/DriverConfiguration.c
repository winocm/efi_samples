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

    DriverConfiguration.c
    
Abstract:

    EFI Driver Configuration Protocol

Revision History

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(DriverConfiguration)

EFI_GUID gEfiDriverConfigurationProtocolGuid = EFI_DRIVER_CONFIGURATION_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiDriverConfigurationProtocolGuid, "Driver Configuration Protocol", "EFI 1.1 Driver Configuration Protocol");
