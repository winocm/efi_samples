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

    int.c

Abstract:

    Main SAL interface routine for IA-64 calls. 


Revision History

--*/


#include "efi.h"
#include "efilib.h"
#include "SalEfi.h"
#include "SalProc.h"
#include "PlSal.h"
#include "Int86.h"

//
// BugBug: Move me to a more common place
//

#define INT86_STACK_PAGES   4
#define INTERRUPT_FLAG      0x09

STATIC VOID *GlobalInt86Stack;
    
BOOLEAN
Int86 (
    IN  UINT8               BiosInt,
    IN  IA32_RegisterSet_t  *Regs
    )
// returns carry flag
{                         
    rArg                        Return;
    IA32_BIOS_REGISTER_STATE    SAL_Regs;
    UINTN                       ModBiosInt;
    EFI_TPL                     Tpl;

    Tpl = BS->RaiseTPL (TPL_NOTIFY);

    //
    // BugBug: cs is being hard coded. This is bad.
    //  may need to capture these value at hand off time. 
    //
    SAL_Regs.eax    = Regs->x.AX;
    SAL_Regs.ecx    = Regs->x.CX;
    SAL_Regs.edx    = Regs->x.DX;
    SAL_Regs.ebx    = Regs->x.BX;
    SAL_Regs.ebp    = Regs->x.BP;
    SAL_Regs.esi    = Regs->x.SI;
    SAL_Regs.edi    = Regs->x.DI;
    SAL_Regs.eflags = 1 << INTERRUPT_FLAG;
    SAL_Regs.eip    = 0;
    SAL_Regs.cs     = 0xf000;
    SAL_Regs.ds     = Regs->x.DS;
    SAL_Regs.es     = Regs->x.ES;
    SAL_Regs.fs     = 0x0;
    SAL_Regs.gs     = 0x0;
    SAL_Regs.ss     = _FP_SEG(GlobalInt86Stack);
    SAL_Regs.esp    = _FP_OFF(GlobalInt86Stack); 

    ModBiosInt = ID_SALCB_BIOSCALL(BiosInt);

    SalCallBack(   
            (UINT64)ModBiosInt, (UINT64)&SAL_Regs, 
            0, 0, 0, 0, 0, 0,
            &Return
            );

    Regs->x.AX       = (UINT16)SAL_Regs.eax;
    Regs->x.CX       = (UINT16)SAL_Regs.ecx;
    Regs->x.DX       = (UINT16)SAL_Regs.edx;
    Regs->x.BX       = (UINT16)SAL_Regs.ebx;
    Regs->x.SI       = (UINT16)SAL_Regs.esi;
    Regs->x.DI       = (UINT16)SAL_Regs.edi;
    Regs->x.Flags.CF = ((UINT16)SAL_Regs.eflags >>  0) & 1;
    Regs->x.Flags.PF = ((UINT16)SAL_Regs.eflags >>  2) & 1;
    Regs->x.Flags.AF = ((UINT16)SAL_Regs.eflags >>  4) & 1;
    Regs->x.Flags.ZF = ((UINT16)SAL_Regs.eflags >>  6) & 1;
    Regs->x.Flags.SF = ((UINT16)SAL_Regs.eflags >>  7) & 1;
    Regs->x.Flags.DF = ((UINT16)SAL_Regs.eflags >> 10) & 1;
    Regs->x.Flags.OF = ((UINT16)SAL_Regs.eflags >> 11) & 1;
    Regs->x.DS       = SAL_Regs.ds;
    Regs->x.ES       = SAL_Regs.es;
    Regs->x.BP       = (UINT16)SAL_Regs.ebp;


    BS->RestoreTPL(Tpl);

    return Regs->x.Flags.CF ? TRUE : FALSE;
} 

VOID
InitializeBiosIntCaller (
    VOID
    )

{
    UINTN                   PageSize;
    EFI_PHYSICAL_ADDRESS    MemPage;
    EFI_STATUS              Status;

    //
    // Allocate 16K bellow 1MB.
    //
    PageSize = INT86_STACK_PAGES;
    MemPage = 0xfffff;

    Status = BS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, PageSize, &MemPage);
    ASSERT (!EFI_ERROR(Status));

    //
    // Stack Grows down, so point to the end of the device. Make it - 0x10 from the
    //  top of the buffer. The 0x10 is safty margin.
    //
    GlobalInt86Stack = (VOID *)(MemPage + (INT86_STACK_PAGES*EFI_PAGE_SIZE) - 0x10);
}

BOOLEAN
Int86Available (
    VOID
    )
{
    //
    // Assume if the SAL_B SAL API's exist that we can make a SAL Callback to do Int86
    //  Note: We really use the SalCallBack() to do this, but we may use SalCallBack()
    //        even if we do not support Int86 callbacks.
    //

    return RtGlobalSalProcEntry.ProcEntryPoint != 0x00;   
}



BOOLEAN
FarCall86 (
    IN  UINT16              Segment,
    IN  UINT16              Offset,
    IN  IA32_RegisterSet_t  *Regs,
    IN OUT VOID             *Stack,   // we need to retrieve our stack state
    IN  UINTN               StackSize
    )
// returns carry flag
{
    rArg                        Return;
    IA32_BIOS_REGISTER_STATE    SAL_Regs;
    UINT8                       *S16;       // possibly byte-aligned Real-mode stack
    EFI_TPL                     Tpl;

    Tpl = BS->RaiseTPL (TPL_NOTIFY);

    // build the arguments to call in
    SAL_Regs.eax    = Regs->x.AX;
    SAL_Regs.ecx    = Regs->x.CX;
    SAL_Regs.edx    = Regs->x.DX;
    SAL_Regs.ebx    = Regs->x.BX;
    SAL_Regs.ebp    = Regs->x.BP;
    SAL_Regs.esi    = Regs->x.SI;
    SAL_Regs.edi    = Regs->x.DI;
    SAL_Regs.eflags = 1 << INTERRUPT_FLAG;
    SAL_Regs.eip    = Offset;
    SAL_Regs.cs     = Segment;
    SAL_Regs.ds     = Regs->x.DS;
    SAL_Regs.es     = Regs->x.ES;
    SAL_Regs.fs     = 0x0;
    SAL_Regs.gs     = 0x0;
    SAL_Regs.ss     = _FP_SEG(GlobalInt86Stack);
    SAL_Regs.esp    = _FP_OFF(GlobalInt86Stack); 
    
    
    if (Stack!=NULL && StackSize!=0) {

        //
        // Copy Stack to low memory stack
        //

        S16 = GlobalInt86Stack;
        S16 -=  StackSize / sizeof(UINT8);
        CopyMem (S16, Stack, StackSize);
        SAL_Regs.ss     = _FP_SEG(S16);
        SAL_Regs.esp    = _FP_OFF(S16); 
    }
    

    SalCallBack(
        ID_SALCB_REALMODE,                     // ID_SALCB_REALMODE
        (UINT64)&SAL_Regs,
        INT86_STACK_PAGES * EFI_PAGE_SIZE,  //  Right now, this will be 16KB.  This will allow the SAL to perform some checking for stack overflows if that became important,
        StackSize,                      //  OPTIONAL - Will be 0 for INT calls.  Will be 0 for FAR CALLs that do not require a stack frame.
        0,
        0,
        0,
        0,
        &Return
        );

    // retrieve the stack in order to parse any return arguments
    Regs->x.AX      = (UINT16)SAL_Regs.eax ;
    Regs->x.BX      = (UINT16)SAL_Regs.ebx;
    Regs->x.CX      = (UINT16)SAL_Regs.ecx ;
    Regs->x.DX      = (UINT16)SAL_Regs.edx;
    Regs->x.BP      = (UINT16)SAL_Regs.ebp;
    Regs->x.SI      = (UINT16)SAL_Regs.esi;
    Regs->x.DI      = (UINT16)SAL_Regs.edi;
    Regs->x.DS      = (UINT16)SAL_Regs.ds;
    Regs->x.ES      = (UINT16)SAL_Regs.es;


    if (Stack!=NULL && StackSize!=0) {
    
        S16 = (UINT8 *)((SAL_Regs.ss << 4) | (SAL_Regs.esp & 0xffff));

        // Put the data back for the caller
        CopyMem (Stack, S16, StackSize);
    }

    BS->RestoreTPL(Tpl);

    return SAL_Regs.eflags & CARRY_FLAG ? TRUE : FALSE;
}
