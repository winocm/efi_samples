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

    tsc.c

Abstract: 

    TSC calibration and timestamp code.  Used to calculate intervals for FwTimerTick()


--*/

#include "efi.h"
#include "efilib.h"
#include "efifw.h"
#include "PlIntCtrl.h"
#include "CpuInterrupt.h"

typedef union
{
    UINT64  qw;
    struct
    {
        UINT32  low;
        UINT32  high;
    } dw;
} QUADWORD;
    
static QUADWORD PlCpuFreq, NumberOfCpuTicks;
static volatile BOOLEAN DoTscCalibration = FALSE;
UINT32 CpuFrequencyAdjustment;

//
// the __declspec (naked) attribute causes compiler to omit prelogue and
// epilogue code from function.  This allows for an explicit "iret" rather than
// an implicit "ret"
//
// also note that this function must be re-entrant by the time RestoreTPL is
// called.  That is to say, any non-stack based data operations must be complete
// by this time.
__declspec (naked)  
STATIC
VOID
SystemTimerHandler(
    VOID
    )
{
    EFI_TPL     OriginalTPL;
    QUADWORD    NewTickCount;
    QUADWORD    ElapsedTicks;
    QUADWORD    ElapsedTime;    // number of microseconds since last interrupt
    UINT32      Remainder;
    
    static QUADWORD OldTickCount;
    
    __asm {
        push    ebp
        mov     ebp, esp
        sub     esp, __LOCAL_SIZE
        pushad                              // save context

//        in        al, 0x61                    // system control port B
//        or        al, 0x80                    // set bit 7
//        out       0x61, al                    // clear interrupt from system timer

        _emit 0x0f                          // rdtsc = 0fh 31h
        _emit 0x31

        mov NewTickCount.dw.low, eax
        mov NewTickCount.dw.high, edx
    }

    ElapsedTicks.qw = NewTickCount.qw - OldTickCount.qw;
    ElapsedTime.qw = DivU64x32(MultU64x32 (ElapsedTicks.qw, 10000000 / CpuFrequencyAdjustment),PlCpuFreq.dw.low, &Remainder);  // ElapsedTime == 100 NS units since last call...
    OldTickCount.qw = NewTickCount.qw;


    __asm {
        call    PlEOI                       // Issue EOI to PIC
        sti                                 // enable interrupts
    }
    
    OriginalTPL = BS->RaiseTPL (TPL_HIGH_LEVEL);    // Implicitly disables interrupts
    FwTimerTick (ElapsedTime.dw.low);
    BS->RestoreTPL (OriginalTPL);           // Implicitly re-enables interrupts and causes queued events to fire
    
    __asm {
        popad                               // restore context
        add     esp, __LOCAL_SIZE
        pop     ebp
        iretd                               // return to interrupted task
    }   
}
    

#define NUM_CALIBRATION_TICKS 5
#define NS_PER_TIMER_TICK 54925415
#define CALIBRATION_DURATION_IN_NS (NUM_CALIBRATION_TICKS * NS_PER_TIMER_TICK)

__declspec (naked)  
STATIC
VOID
CalibrateTSC(
    VOID
    )
{
    static UINTN CalibrationCount = 0;
    static QUADWORD OrigTS;
    QUADWORD NewTS;

    __asm {
        push    ebp
        mov     ebp, esp
        sub     esp, __LOCAL_SIZE
        pushad                              // save context
    }
    
    if (DoTscCalibration == TRUE)
    {
        __asm {
            _emit 0x0f                      // rdtsc = 0fh 31h
            _emit 0x31
            mov     NewTS.dw.low, eax
            mov     NewTS.dw.high, edx
        }
        
        if (CalibrationCount == 0)
            OrigTS.qw = NewTS.qw;
        
        if (CalibrationCount < NUM_CALIBRATION_TICKS)
            CalibrationCount++;
        else
        {
            DoTscCalibration = FALSE;
            CalibrationCount = 0;
            NumberOfCpuTicks.qw = NewTS.qw - OrigTS.qw;
        }
    }

    PlEOI();    

    __asm {
        popad                               // restore context
        add     esp, __LOCAL_SIZE
        pop     ebp
        iretd                               // return to interrupted task
    }   
}
    

STATIC
VOID
PlCalibrateCpuFreq(
    VOID
    )

/*++

Routine Description:

    This routine computes the number of itterations of StallLoop() that need to be 
    use for a 1 uS delay, a 16 uS delay, and a 256 uS delay.  These values will 
    vary based on CPU speed, cache speed, and main memory speed.  These values are 
    later used by PlStall() to perform very accurate stalls.

Arguments:

    None

Returns:

    None

--*/

{
    UINT32 Remainder;
    
    NumberOfCpuTicks.qw = 0;
    DoTscCalibration = FALSE;
    PL->SetInterruptState(FALSE);
    InstallInterruptHandler(PlGetVectorFromIrq(0), CalibrateTSC);
    PlEnableTimerInterrupt();
    PL->SetInterruptState(TRUE);
    for (DoTscCalibration = TRUE; DoTscCalibration == TRUE; );
    PL->SetInterruptState(FALSE);
    PlDisableTimerInterrupt();
    
    
    // CpuFreq = TscClocks * 1,000,000,000 / CALIBRATION_DURATION_IN_NS
    PlCpuFreq.qw = DivU64x32 (MultU64x32 (NumberOfCpuTicks.qw, 1000000000), CALIBRATION_DURATION_IN_NS, &Remainder);
    
    // CpuFreq must fit within 31 bits for timer tick handler to work properly
    // so insure this is the case and adjust if necessary
    CpuFrequencyAdjustment = 1;
    while (PlCpuFreq.qw & 0xffffffff80000000) { 
        CpuFrequencyAdjustment <<= 1;
        PlCpuFreq.qw = RShiftU64 (PlCpuFreq.qw, 1);
    }
    
    __asm   mov eax, PlCpuFreq.dw.low
    __asm   mov edx, PlCpuFreq.dw.high
}

