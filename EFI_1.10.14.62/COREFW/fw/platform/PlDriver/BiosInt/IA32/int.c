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

    init.c

Abstract:

    Main entry point on EFI 32-bit emulation environment. This envirnment layers 
    on top of the of a legacy BIOS



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Int86.h"
#include "PlIntCtrl.h"
#include "PlTpl.h"

//
// Interesting CR0 flags
//

#define CR0_PE          0x00000001


//
// Processor structres are packed
//

#pragma pack (1)

//
// Define what a processor GDT looks like
//

typedef struct {
    UINT32  LimitLo     : 16;
    UINT32  BaseLo      : 16;

    UINT32  BaseMid     : 8;
    UINT32  Type        : 4;
    UINT32  System      : 1;
    UINT32  Dpl         : 2;
    UINT32  Present     : 1;
    UINT32  LimitHi     : 4;
    UINT32  Software    : 1;
    UINT32  Reserved    : 1;
    UINT32  DefaultSize : 1;
    UINT32  Granularity : 1;
    UINT32  BaseHi      : 8;
} GDT ;


//
// Define what a processor descriptor looks like
//


typedef struct {
    UINT16  Limit;
    UINT32  Base;
} DESCRIPTOR;

#pragma pack ()

//
// Low stub lay out
//

#define LOW_STACK_SIZE      (8*1024)            // 8k?

typedef struct {
    //
    // Space for the code
    //

    CHAR8               Code[4096];             // ?

    //
    // Data for the code (cs releative)
    //

    DESCRIPTOR          GdtDesc;                // Protected mode GDT
    DESCRIPTOR          IdtDesc;                // Protected mode IDT
    UINT32              FlatSs;
    UINT32              FlatEsp;

    UINT32              LowCodeSelector;        // Low code selector in GDT
    UINT32              LowDataSelector;        // Low data selector in GDT
    UINT32              LowStack;
    DESCRIPTOR          RealModeIdtDesc;

    //
    // A low memory stack
    //

    CHAR8               Stack[LOW_STACK_SIZE];

} LOW_MEMORY_THUNK;


//
// Globals
//

LOW_MEMORY_THUNK        *IntThunk;

STATIC BOOLEAN BiosIntCallerInitialized = FALSE;

//
//
//

BOOLEAN
Int86 (
    IN  UINT8               BiosInt,
    IN  IA32_RegisterSet_t  *Regs
    )
// returns carry flag
{
    UINTN               Status;
    UINT16              *S16;
    BOOLEAN             SavedInterruptState;

    Regs->x.Flags.Reserved1 = 1;
    Regs->x.Flags.Reserved2 = 0;
    Regs->x.Flags.Reserved3 = 0;
    Regs->x.Flags.Reserved4 = 0;
    Regs->x.Flags.IOPL      = 3;
    Regs->x.Flags.NT        = 0;
    Regs->x.Flags.IF        = 1;
    Regs->x.Flags.TF        = 0;

    S16 = (UINT16 *) (IntThunk->Stack + LOW_STACK_SIZE);

    //
    // Copy regs to low memory stack
    //

    S16 -=  sizeof(IA32_RegisterSet_t) / sizeof(UINT16);
    CopyMem (S16, Regs, sizeof(IA32_RegisterSet_t));

    //
    // Provide low stack esp
    //

    IntThunk->LowStack = ((UINT32) S16) - ((UINT32) IntThunk);

    SavedInterruptState = PlSetInterruptState(FALSE);   // insure interrupts are turned off
    PlSetupInterruptControllerMask(INT_CTRLR_BIOSMODE); // Save/Setup interrupt controller mask
    
    //
    // Call the real mode int thunk code
    //

    _asm {
        movzx   ecx, BiosInt
        mov     edx, 0
        mov     eax, IntThunk
        call    eax
        mov     Status, eax

    }

    PlSetupInterruptControllerMask(INT_CTRLR_EFIMODE); // Save/Setup interrupt controller mask
    PlSetInterruptState(SavedInterruptState);          // Restore interrupt flag

    PlGenerateIrq (0);

    //
    // Return the resulting registers
    //

    CopyMem (Regs, S16, sizeof(IA32_RegisterSet_t));

    return Regs->x.Flags.CF ? TRUE : FALSE;
}

BOOLEAN
FarCall86 (
    IN  UINT16              Segment,
    IN  UINT16              Offset,
    IN  IA32_RegisterSet_t  *Regs,
    IN  VOID                *Stack,
    IN  UINTN               StackSize
    )
// returns carry flag
{
    UINT32              CallAddress;
    UINTN               Status;
    UINT16              *S16;
    BOOLEAN             SavedInterruptState;

    CallAddress = (Segment<<16) | Offset;

    Regs->x.Flags.Reserved1 = 1;
    Regs->x.Flags.Reserved2 = 0;
    Regs->x.Flags.Reserved3 = 0;
    Regs->x.Flags.Reserved4 = 0;
    Regs->x.Flags.IOPL      = 3;
    Regs->x.Flags.NT        = 0;
    Regs->x.Flags.IF        = 1;
    Regs->x.Flags.TF        = 0;

    S16 = (UINT16 *) (IntThunk->Stack + LOW_STACK_SIZE);

    if (Stack!=NULL && StackSize!=0) {

        //
        // Copy Stack to low memory stack
        //

        S16 -=  StackSize / sizeof(UINT16);
        CopyMem (S16, Stack, StackSize);
    }

    //
    // Copy regs to low memory stack
    //

    S16 -=  sizeof(IA32_RegisterSet_t) / sizeof(UINT16);
    CopyMem (S16, Regs, sizeof(IA32_RegisterSet_t));

    //
    // Provide low stack esp
    //

    IntThunk->LowStack = ((UINT32) S16) - ((UINT32) IntThunk);

    SavedInterruptState = PlSetInterruptState(FALSE);
    PlSetupInterruptControllerMask(INT_CTRLR_BIOSMODE); // Save/Setup interrupt controller
    
    //
    // Call the real mode int thunk code
    //

    _asm {
        mov     ecx, 0
        mov     edx, CallAddress
        mov     eax, IntThunk
        call    eax
        mov     Status, eax
    }

    PlSetupInterruptControllerMask(INT_CTRLR_EFIMODE);
    PlSetInterruptState(SavedInterruptState); 

    PlGenerateIrq (0);

    //
    // Return the resulting registers
    //

    CopyMem (Regs, S16, sizeof(IA32_RegisterSet_t));
    S16 +=  sizeof(IA32_RegisterSet_t) / sizeof(UINT16);

    if (Stack!=NULL && StackSize!=0) {

        //
        // Copy low memory stack to Stack
        //

        CopyMem (Stack, S16, StackSize);
        S16 +=  StackSize / sizeof(UINT16);
    }

    return Regs->x.Flags.CF ? TRUE : FALSE;
}

VOID
RealModeTemplate (
    OUT UINT32          *LowCodeStart,
    OUT UINT32          *LowCodeEnd
    )
{
    _asm {
        mov     eax, LowCodeStart
        mov     ecx, offset LCS0
        mov     [eax], ecx

        mov     eax, LowCodeEnd
        mov     ecx, offset LCS99
        mov     [eax], ecx

        ; patch in the far jump to 16bit protected mode
        mov     eax, offset LCSP3
        mov     ecx, offset LCS1
        sub     ecx, offset LCS0
        mov     [eax], ecx
        mov     ecx, IntThunk
        mov     ecx, LOW_MEMORY_THUNK [ecx].LowCodeSelector
        mov     [eax+4], cx

        ; patch in the far jump to real-mode
        mov     eax, offset LCSP4
        mov     ecx, offset LCS40
        sub     ecx, offset LCS0
        mov     [eax], cx
        mov     edx, IntThunk
        shr     edx, 4
        mov     [eax+2], dx

        ; patch in the real-mode ss
        mov     word ptr [LCSP5], dx

        ; patch in the far jump back to flat mode
        mov     eax, offset LCSP1
        mov     ecx, offset LCS90
        mov     [eax], ecx
        mov     cx, cs
        mov     [eax+4], cx

        ; path in flat ds value for restore
        mov     ax, ds
        mov     word ptr [LCSP2], ax

        ; patch in GdtDesc
        mov     ecx, IntThunk
        mov     ax, LOW_MEMORY_THUNK [ecx].GdtDesc
        mov     word ptr [LGDT0], ax
        mov     ax, LOW_MEMORY_THUNK [ecx+2].GdtDesc
        mov     word ptr [LGDT1], ax
        mov     ax, LOW_MEMORY_THUNK [ecx+4].GdtDesc
        mov     word ptr [LGDT2], ax
    
    }

    // always return
    if (*LowCodeStart) {
        return ;
    }

    //
    // real mode int thunk code
    //

    _asm {
LCS0:
    ;
    ; Save protected mode registers
    ; CL = BiosInt to invoke
    ;

        pushfd                          ; Save flags
        cli

        push    edi                     ; Save C regs
        push    esi
        push    ebp
        push    edx
        push    ebx

        mov     esi, IntThunk           ; esi = IntThunk

    ;
    ; Save flat 32bit stack location
    ;

        mov     LOW_MEMORY_THUNK [esi].FlatEsp, esp

    ;
    ; Patch in BiosInt value into low memory thunk
    ;

        cmp     edx,0
        jne     PatchFarCall
        mov     eax, esi
        add     eax, offset LCSP6
        sub     eax, offset LCS0
        mov     [eax-1], 0xcd       ; INT inst
        mov     [eax], cl           ; INT #
        mov     [eax+1], 0x90       ; NOP
        mov     [eax+2], 0x90       ; NOP
        mov     [eax+3], 0x90       ; NOP
        jmp     PatchComplete
PatchFarCall:
        mov     eax, esi
        add     eax, offset LCSP6
        sub     eax, offset LCS0
        mov     [eax-1], 0x9a       ; CALL inst
        mov     [eax], edx          ; Fill in SEG:OFF
PatchComplete:

    ;
    ; Set idt for real mode IVT
    ;

        lidt    fword ptr LOW_MEMORY_THUNK [esi].RealModeIdtDesc

    ;
    ; Load real-mode style selectors limits
    ;

        mov     eax, LOW_MEMORY_THUNK [esi].LowDataSelector
        mov     ebx, LOW_MEMORY_THUNK [esi].LowStack

        mov     gs, ax
        mov     fs, ax
        mov     es, ax
        mov     ds, ax
        mov     ss, ax
        mov     esp, ebx

        _emit   0xEA                    ; jmp far 16:32
LCSP3:  _emit   0x00                    ; offset
        _emit   0x00                        ; (LCS1)
        _emit   0x00
        _emit   0x00
        _emit   0x00                    ; selector
        _emit   0x00                        ; (16bit CS)

LCS1:

    ;
    ; We are now in 16bit protcted mode, on the low memory
    ;


    ;
    ; Turn protected mode off
    ;

        _emit   0x66
        mov     eax, cr0
        _emit   0x66
        and     eax, not CR0_PE
        _emit   0x66
        mov     cr0, eax

        _emit   0xEA                    ; jmp far 16:16
LCSP4:  _emit   0x00                    ; offset
        _emit   0x00                        ; (LCS40)
        _emit   0x00                    ; segment
        _emit   0x00                        ; (realmode cs)

LCS40:

    ;
    ; We are now in real mode
    ; Fix SS
    ;
        _emit   0xb8                    ; mov ax,
LCSP5:  _emit   0x00                        ; (readmode ss)
        _emit   0x00

        _emit   0x8e                    ; mov ss, ax
        _emit   0xd0

        sti
    ;
    ; Load regs with callers request
    ; NOTE1: that this pop sequence matches the
    ;        IA32_RegisterSet_t structure.
    ; NOTE2: the follow instructions are really executed
    ;        as 16 bit instructions as we are in real mode
    ;

        pop     eax
        pop     ebx
        pop     ecx
        pop     edx
        pop     esi
        pop     edi
        popfd
        pop     es
        pop     ebp         ; throw away cs
        pop     ebp         ; throw away ss
        pop     ds
        pop     ebp

        _emit   0xcd        ; issue NT or a FAR CALL
LCSP6:  _emit   00          ; INT # or OFFSET
        _emit   0x90        ; NOP or OFFSET
        _emit   0x90        ; NOP or SEGMENT
        _emit   0x90        ; NOP or SEGMENT

    ;
    ; Save the result regs
    ; See the above NOTEs
    ;

        push    ebp
        mov     ebp, eax    ; save eax

        push    ds
        push    eax         ; throw away ss
        push    eax         ; throw away cs
        push    es
        pushfd
        push    edi
        push    esi
        push    edx
        push    ecx
        push    ebx
        push    ebp         ; ax results are in bp


;
; Test to see if the A20 Gate needs to be enabled
;

TestA20Gate:
        cli
        push    es
        push    ds
        _emit   0x31        ; xor ax,ax         ax = 0
        _emit   0xc0
        _emit   0x89        ; mov si,ax         si = 0
        _emit   0xc6
        _emit   0x89        ; mov bp,ax         bp = 0
        _emit   0xc5
        _emit   0x8e        ; mov ds,ax         ds = 0
        _emit   0xd8
        _emit   0x48        ; dec ax            ax = ffff
        _emit   0x8e        ; mov es,ax         es = ffff
        _emit   0xc0
        _emit   0x8a        ; mov cl,[si]       cl = ds:[si] = 0000:0000
        _emit   0x0c
        _emit   0x26        ; es:
        _emit   0x8a        ; mov ch,[bp+10]    ch = es:[bp+10] = ffff:0000+10 = ffff:0010
        _emit   0x6e
        _emit   0x10
        _emit   0xc6        ; mov byte ptr [si],aa      ds:[si] = aa    0000:0000 = aa
        _emit   0x04
        _emit   0xaa
        _emit   0x26        ; es:
        _emit   0xc6        ; mov byte ptr [bp+10],55   es:[bp+10] = 55 ffff:0010 = 55
        _emit   0x46
        _emit   0x10
        _emit   0x55
        _emit   0x80        ; cmp byte ptr [si],aa      ds:[si] = 0000:0000
        _emit   0x3c
        _emit   0xaa
        _emit   0x88        ; mov [si],cl               ds:[si] = 0000:0000
        _emit   0x0c
        _emit   0x26        ; es:
        _emit   0x88        ; mov [bp+10],ch            es:[bp+10] = ffff:0010
        _emit   0x6e
        _emit   0x10
        jnz     EnableA20Gate        
        pop     ds
        pop     es
        sti
        jmp     A20GateEnabled
EnableA20Gate:
        pop     ds
        pop     es
        sti


;
; Enable A20 Mask
;


        _emit   0xb8                    ; mov ax,2401h
        _emit   0x01
        _emit   0x24
        _emit   0xcd                    ; INT 15h
        _emit   0x15
        jnc     TestA20Gate             ; Jump if it suceeded

        mov     ecx,0
Empty8042Loop0:
        _emit   0xe7            ; out DELAYPORT,ax
        _emit   0xed
        _emit   0xe4            ; in  al,KBD_STATUS_PORT
        _emit   0x64
        _emit   0x24            ; and al,02h
        _emit   0x02
        loopnz  Empty8042Loop0
        jnz     Timeout8042

        _emit   0xe7            ; out DELAYPORT,ax
        _emit   0xed
        _emit   0xb0            ; mov al,WRITE_DATA_PORT_CMD
        _emit   0xd1
        _emit   0xe6            ; out KBD_STATUS_PORT,al
        _emit   0x64

        mov     ecx,0
Empty8042Loop1:
        _emit   0xe7            ; out DELAYPORT,ax
        _emit   0xed
        _emit   0xe4            ; in  al,KBD_STATUS_PORT
        _emit   0x64
        _emit   0x24            ; and al,02h
        _emit   0x02
        loopnz  Empty8042Loop1
        jnz     Timeout8042

        _emit   0xb0            ; mov al,ENABLE_A20_CMD
        _emit   0xdf
        _emit   0xe6            ; out KBD_CONTROL_PORT,al
        _emit   0x60

        mov     ecx,0
Empty8042Loop2:
        _emit   0xe7            ; out DELAYPORT,ax
        _emit   0xed
        _emit   0xe4            ; in  al,KBD_STATUS_PORT
        _emit   0x64
        _emit   0x24            ; and al,02h
        _emit   0x02
        loopnz  Empty8042Loop2

        mov     ecx,25
Delay25uS:
        _emit   0xe7            ; out DELAYPORT,ax
        _emit   0xed
        loop    Delay25uS
        jmp     TestA20Gate
Timeout8042:
A20GateEnabled:

        cli

        _emit   0xb8                    ; mov ax,
LGDT2:  _emit   0x00                    ; GDTR
        _emit   0x00

        push    eax

        _emit   0xb8                    ; mov ax,
LGDT1:  _emit   0x00                    ; GDTR
        _emit   0x00

        push    eax

        _emit   0xb8                    ; mov ax,
LGDT0:  _emit   0x00                    ; GDTR
        _emit   0x00

        push    eax

        push    ss
        pop     ds
        mov     ebx,esp

        pop     eax
        pop     eax
        pop     eax

    ; lgdt    fword ptr [bx]
        _emit   0x66
        _emit   0x0f
        _emit   0x01
        _emit   0x17

    ;
    ; Turn protected mode on
    ;

        _emit   0x66
        mov     eax, cr0
        _emit   0x66
        or      eax, CR0_PE
        _emit   0x66
        mov     cr0, eax

    ;
    ; Return to 32 bit flat mode
   ; Put flat DS in ax
    ;
        _emit   0xb8                    ; mov ax,
LCSP2:  _emit   0x00                    ; flat ds
        _emit   0x00

    ;
    ; Return to 32 bit cs
    ;

        _emit   0x66
        _emit   0xEA                    ; jmp far 16:32
LCSP1:  _emit   0x00                    ; offset
        _emit   0x00                        ; (LCS90)
        _emit   0x00
        _emit   0x00
        _emit   0x00                    ; selector
        _emit   0x00                        ; (flat CS)

LCS90:
    ; restore data selector
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax

    ;
    ; Restore 32bit Idt
    ;

        mov     esi, IntThunk
        lidt    fword ptr LOW_MEMORY_THUNK [esi].IdtDesc

    ;
    ; Restore 32bit stack
    ;
        mov     ecx, LOW_MEMORY_THUNK [esi].FlatSs
        mov     ss, cx
        mov     esp, LOW_MEMORY_THUNK [esi].FlatEsp

        xor     eax, eax
        lldt    ax

    ;
    ; Restore c regs
    ;

        pop     ebx
        pop     edx
        pop     ebp
        pop     esi
        pop     edi
        popfd
        ret
LCS99:  nop
    }
}

VOID
InitializeBiosIntCaller (
    VOID
    )
{
    EFI_PHYSICAL_ADDRESS        MemPage;
    EFI_STATUS                  Status;
    GDT                         *CodeGdt, *DataGdt;
    UINT32                      LowCodeStart, LowCodeEnd, Base;

    //
    // Allocate 1 page below 1MB to put real mode thunk code in
    //

    MemPage = ONEMB - 1;
    Status = BS->AllocatePages (    
                    AllocateMaxAddress,
                    EfiBootServicesCode,
                    sizeof(LOW_MEMORY_THUNK) / 4096 + 1,
                    &MemPage
                    );

    ASSERT (!EFI_ERROR(Status));
    IntThunk = (LOW_MEMORY_THUNK *) MemPage;

    //
    // Capture the flat gdt, idt, and selector values
    //

    _asm {
        mov     ecx, IntThunk
        sgdt    [ecx].GdtDesc
        sidt    [ecx].IdtDesc
        mov     ax, ss
        movzx   eax, ax
        mov     [ecx].FlatSs, eax
    }

    //
    // Allocate a new GDT for real-mode code
    //
    // BUGBUG: we're just assuming that the first selector
    // that is not is use is something we can allocate
    //

    for (CodeGdt = ((GDT *) IntThunk->GdtDesc.Base) + 1; CodeGdt->Present; CodeGdt += 1) ;

    //
    // Fill in the new descriptor to by our real-mode segment value
    //

    CodeGdt->Type        = 0xA;         // code/read
    CodeGdt->System      = 1;
    CodeGdt->Dpl         = 0;
    CodeGdt->Present     = 1;
    CodeGdt->Software    = 0;
    CodeGdt->Reserved    = 0;
    CodeGdt->DefaultSize = 0;           // 16 bit operands
    CodeGdt->Granularity = 0;

    CodeGdt->LimitHi     = 0;
    CodeGdt->LimitLo     = 0xffff;

    Base = ((UINT32) IntThunk);
    CodeGdt->BaseHi      = (Base >> 24) & 0xFF;
    CodeGdt->BaseMid     = (Base >> 16) & 0xFF;
    CodeGdt->BaseLo      = Base & 0xFFFF;

    //
    // Allocate a new GDT for read-mode data
    //

    for (DataGdt = CodeGdt; DataGdt->Present; DataGdt += 1) ;
    CopyMem (DataGdt, CodeGdt, sizeof(GDT));
    DataGdt->Type = 0x2;                // read/write data

    //
    // Compute selector value
    //

    IntThunk->LowCodeSelector  = ((UINT32) CodeGdt) - IntThunk->GdtDesc.Base;
    IntThunk->LowDataSelector  = ((UINT32) DataGdt) - IntThunk->GdtDesc.Base;
    IntThunk->RealModeIdtDesc.Limit = 0xFFFF;
    IntThunk->RealModeIdtDesc.Base  = 0;

    //
    // Initialize low real-mode code thunk
    //

    RealModeTemplate (&LowCodeStart, &LowCodeEnd);
    CopyMem (IntThunk->Code, (VOID *) LowCodeStart, LowCodeEnd-LowCodeStart);

    BiosIntCallerInitialized = TRUE;

}

BOOLEAN
Int86Available (
    VOID
    )
{
    return BiosIntCallerInitialized;
}

VOID
PlGenerateIrq(
    UINT8 irq
    )
{
	UINT8 interrupt;
  
  interrupt = PlGetVectorFromIrq(irq);

	if (interrupt) _asm {
		mov		ecx, offset PatchInt
		mov 		al, [interrupt]
		mov		[ecx], al
		jmp		DoInterrupt
DoInterrupt:
		_emit	0xcd
PatchInt:	_emit	0
	}
}
