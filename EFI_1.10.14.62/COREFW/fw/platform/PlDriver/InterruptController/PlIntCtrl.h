#ifndef _PLINTCTRL_H
#define _PLINTCTRL_H
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

    PlIntCtrl.h

Abstract: 

    Interrupt controller mask/unmask prototypes and functions.


Revision History

--*/

typedef enum 
{
    INT_CTRLR_EFIMODE,
    INT_CTRLR_BIOSMODE
} INT_CTRLR_MODE;

VOID
BOOTSERVICE
PlSetupInterruptControllerMask (
    IN INT_CTRLR_MODE Mode
    );

VOID
BOOTSERVICE
PlSetupInterruptController (
    IN INT_CTRLR_MODE Mode
    );

VOID
BOOTSERVICE
PlEnableTimerInterrupt(
    VOID
    );
    
VOID
BOOTSERVICE
PlDisableTimerInterrupt(
    VOID
    );

VOID
BOOTSERVICE
PlMapIrqToVect(
    INT_CTRLR_MODE
    );
                  
VOID
BOOTSERVICE
PlEnableTimerInterrupt(
    VOID
    );

VOID
BOOTSERVICE
PlDisableTimerInterrupt(
    VOID
    );

VOID
BOOTSERVICE
PlEOI(
    VOID
    );

UINT8
BOOTSERVICE
PlGetVectorFromIrq(
    UINT8 irq
    );

VOID
BOOTSERVICE
PlGenerateIrq(
    UINT8 irq
    );

#endif
