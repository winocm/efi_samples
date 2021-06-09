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

    virtual.c

Abstract: 

    Go to virtual mode in RT. May the force be with you. I think
    you may need it.


Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlVirtual.h"
#include "PlEfiLdr.h"

//
//
//
#pragma RUNTIME_CODE(RtSetVirtualAddressMap)
EFI_STATUS
EFIAPI
RUNTIMEFUNCTION
RtSetVirtualAddressMap (
    IN UINTN                        MemoryMapSize,
    IN UINTN                        DescriptorSize,
    IN UINT32                       DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR        *VirtualMap
    )
{
  return RtSetVirtualAddressMapInternal (
           MemoryMapSize, 
           DescriptorSize, 
           DescriptorVersion, 
           VirtualMap
           );
}


#pragma RUNTIME_CODE(RtPlSetVirtualAddressMap)
STATIC
VOID
RUNTIMEFUNCTION
RtPlSetVirtualAddressMap (
    IN EFI_CONVERT_POINTER      ConvertPointer,
    IN UINTN                    MemoryMapSize,
    IN UINTN                    DescriptorSize,
    IN UINT32                   DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR    *VirtualMap
    )
/*++

Routine Description:

    This routine registers all virtual address translations

Arguments:

    Pointer to ConvertPointer function
    Total size of memory map
    Size of each descriptor within memory map
    Descriptor version
    Pointer to memory map

Returns:
   

--*/
{
    //
    // Callback to EFILDR to fixup all the addresses in the base image
    //
    RtPlEfiLdrCallBack (EFILDR_CB_VA, MemoryMapSize, DescriptorSize, (UINTN)VirtualMap);

    return;
}

