#ifndef _VIRTUAL_H
#define _VIRTUAL_H
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

    PlVirtual.h

Abstract: 

    Go to virtual mode in RT. May the force be with you. I think
    you may need it.


Revision History

--*/

EFI_STATUS
EFIAPI
RUNTIMEFUNCTION
RtSetVirtualAddressMap (
    IN UINTN                        MemoryMapSize,
    IN UINTN                        DescriptorSize,
    IN UINT32                       DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR        *VirtualMap
    );

EFI_STATUS
EFIAPI
RUNTIMEFUNCTION
RtSetVirtualAddressMapInternal (
    IN UINTN                        MemoryMapSize,
    IN UINTN                        DescriptorSize,
    IN UINT32                       DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR        *VirtualMap
    );

STATIC
VOID
RUNTIMEFUNCTION
RtPlSetVirtualAddressMap (
    IN EFI_CONVERT_POINTER      ConvertPointer,
    IN UINTN                    MemoryMapSize,
    IN UINTN                    DescriptorSize,
    IN UINT32                   DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR    *VirtualMap
    );

#endif
