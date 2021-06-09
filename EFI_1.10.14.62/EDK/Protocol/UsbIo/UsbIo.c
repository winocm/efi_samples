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

    UsbIo.c
    
Abstract:

    EFI Usb I/O Protocol

Revision History

--*/

#include "Efi.h"

#include EFI_PROTOCOL_DEFINITION(UsbIo)

EFI_GUID gEfiUsbIoProtocolGuid = EFI_USB_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiUsbIoProtocolGuid, "Usb IO Protocol", "EFI 1.1 Usb IO Protocol");

