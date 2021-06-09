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
    
      pci.c

Abstract:




Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Pldefio.h"
#include "pci22.h"

EFI_STATUS
DefPciRead (
    IN  EFI_DEVICE_IO_INTERFACE     *This,
    IN  EFI_IO_WIDTH                Width,
    IN  UINT64                      UserAddress,
    IN  UINTN                       Count,
    IN  OUT VOID                    *UserBuffer
    )
{
    if (Width < 0 || Width > IO_UINT32) {
      return EFI_INVALID_PARAMETER;
    }

    SetMem(UserBuffer,Count*(1<<Width),0xff);
    return EFI_SUCCESS;
}

EFI_STATUS
DefPciWrite (
    IN  EFI_DEVICE_IO_INTERFACE     *This,
    IN  EFI_IO_WIDTH                Width,
    IN  UINT64                      UserAddress,
    IN  UINTN                       Count,
    IN  OUT VOID                    *UserBuffer
    )
{
    if (Width < 0 || Width > IO_UINT32) {
      return EFI_INVALID_PARAMETER;
    }

    return EFI_SUCCESS;
}

