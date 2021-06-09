#ifndef _INT86_H
#define _INT86_H
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

    Int86.h

Abstract:

    Bios include file to do Legacy BIOS INT


Revision History

--*/

//
// Convert Flat Address to Seg Offset
//
#define _FP_SEG(fp) ((UINT16) (((UINT32)(fp)) >> 4) & 0xf000)
#define _FP_OFF(fp) (((UINT16)(fp)) & 0xffff) 

//
// Defines for Int86() functions
//

#define CARRY_FLAG  0x01

struct _FlagsReg {
    UINT16 CF:1;
    UINT16 Reserved1:1;
    UINT16 PF:1;
    UINT16 Reserved2:1;
    UINT16 AF:1;
    UINT16 Reserved3:1;
    UINT16 ZF:1;
    UINT16 SF:1;
    UINT16 TF:1;
    UINT16 IF:1;
    UINT16 DF:1;
    UINT16 OF:1;
    UINT16 IOPL:2;
    UINT16 NT:1;
    UINT16 Reserved4:1;
};

/* word registers */
struct __WordRegs {
    UINT16           AX;
    UINT16           BX;
    UINT16           CX;
    UINT16           DX;
    UINT16           SI;
    UINT16           DI;
    struct _FlagsReg Flags;
};

struct _WordRegs {
    UINT16           AX;
    UINT16           BX;
    UINT16           CX;
    UINT16           DX;
    UINT16           SI;
    UINT16           DI;
    struct _FlagsReg Flags;
    UINT16           ES;
    UINT16           CS;
    UINT16           SS;
    UINT16           DS;

    UINT16           BP;
};

/* byte registers */

struct _ByteRegs {
    UINT8 AL, AH;
    UINT8 BL, BH;
    UINT8 CL, CH;
    UINT8 DL, DH;
};

/* general purpose registers union -
 *  overlays the corresponding word and byte registers.
 */

union _Regs {
    struct _WordRegs x;
    struct _ByteRegs h;
};

struct _SREGS {
    UINT16 es;
    UINT16 cs;
    UINT16 ss;
    UINT16 ds;
};

/* segment registers */

typedef union _Regs IA32_RegisterSet_t;   

//
//
//

#define ONEMB     (1024*1024)

BOOLEAN
Int86 (
    IN  UINT8               BiosInt,
    IN  IA32_RegisterSet_t  *Regs
    );


BOOLEAN
FarCall86 (
    IN  UINT16              Segment,
    IN  UINT16              Offset,
    IN  IA32_RegisterSet_t  *Regs,
    IN  VOID                *Stack,
    IN  UINTN               StackSize
    );

VOID
InitializeBiosIntCaller (
    VOID
    );

BOOLEAN
Int86Available (
    VOID
    );

#endif
