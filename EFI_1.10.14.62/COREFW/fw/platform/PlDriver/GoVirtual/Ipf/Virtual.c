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
#include "Plvirtual.h"
#include "SalEfi.h"
#include "SalProc.h"
#include "PlSal.h"

extern UINT64 ReadIVA();

UINT64
EFIAPI
RtPlGetRtSalIVA (
    )
/*++

Routine Description:

    This routine returns the value of the global RtSalIVA
    in R8 for the assembler to patch the IVA.
    This is easier than accessing global data from assembler.

Arguments:

    None

Returns:

    None
   

--*/
{
    return( RtSalIVA ) ;
}


VOID
PlInitSalIVA (
    )
/*++

Routine Description:

    This routine saves the SAL IVA value to be restored during the 
    RtSetVirutalAddressMap call.  The IVA is saved into a global.

Arguments:

    None

Returns:

    None
   

--*/
{
    RtSalIVA = ReadIVA() ;
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

    This routine registers all virtual address translations with SAL

Arguments:

    Pointer to ConvertPointer function
    Total size of memory map
    Size of each descriptor within memory map
    Descriptor version
    Pointer to memory map

Returns:
   

--*/
{
    rArg    Results = {-1,0,0,0};
    UINT64  *LocalProcEntryPointPointer;

    // fyi.. can't use DEBUG print in here because everything but
    // the base firmware has been fixed up for virtual mode

    //
    // Place address of global in local variable on the stack, because the global might get
    // fixed up by the SalCallBack()
    //
    LocalProcEntryPointPointer = &RtGlobalSALCallBack.ProcEntryPoint;

    //
    // callback SAL to fixup all addresses (including EFI)
    //
    RtSalCallBack(
        ID_SALCB_VA, MemoryMapSize,DescriptorSize,(UINTN)VirtualMap,
        0,0,0,0,
        &Results
        );
    
    //
    // If callback succeeded then r9 will contain new
    // fixed up value of salcallback
    //
    if(!Results.p0) {
        *LocalProcEntryPointPointer = Results.p1;
    }
}

