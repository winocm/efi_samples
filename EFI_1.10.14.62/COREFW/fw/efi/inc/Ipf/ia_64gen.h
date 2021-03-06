// 
// 
// 
// Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
// This software and associated documentation (if any) is furnished
// under a license and may only be used or copied in accordance
// with the terms of the license. Except as permitted by such
// license, no part of this software or documentation may be
// reproduced, stored in a retrieval system, or transmitted in any
// form or by any means without the express written consent of
// Intel Corporation.
// 
// 
//
//Module Name: ia_64gen.h
//
//
//Abstract:
//
//
//
//
//Revision History
//
//
#ifndef _IA64GEN_H
#define _IA64GEN_H

#define TT_UNAT     0
#define C_PSR       0
#define J_UNAT      0
#define T_TYPE      0
#define T_IPSR      0x8
#define T_ISR       0x10
#define T_IIP       0x18
#define T_IFA       0x20
#define T_IIPA      0x28
#define T_IFS       0x30
#define T_IIM       0x38
#define T_RSC       0x40
#define T_BSP       0x48
#define T_BSPSTORE      0x50
#define T_RNAT      0x58
#define T_PFS       0x60
#define T_KBSPSTORE     0x68
#define T_UNAT      0x70
#define T_CCV       0x78
#define T_DCR       0x80
#define T_PREDS     0x88
#define T_NATS      0x90
#define T_R1        0x98
#define T_GP        0x98
#define T_R2        0xa0
#define T_R3        0xa8
#define T_R4        0xb0
#define T_R5        0xb8
#define T_R6        0xc0
#define T_R7        0xc8
#define T_R8        0xd0
#define T_R9        0xd8
#define T_R10       0xe0
#define T_R11       0xe8
#define T_R12       0xf0
#define T_SP        0xf0
#define T_R13       0xf8
#define T_R14       0x100
#define T_R15       0x108
#define T_R16       0x110
#define T_R17       0x118
#define T_R18       0x120
#define T_R19       0x128
#define T_R20       0x130
#define T_R21       0x138
#define T_R22       0x140
#define T_R23       0x148
#define T_R24       0x150
#define T_R25       0x158
#define T_R26       0x160
#define T_R27       0x168
#define T_R28       0x170
#define T_R29       0x178
#define T_R30       0x180
#define T_R31       0x188
#define T_F2        0x1f0
#define T_F3        0x200
#define T_F4        0x210
#define T_F5        0x220
#define T_F6        0x230
#define T_F7        0x240
#define T_F8        0x250
#define T_F9        0x260
#define T_F10       0x270
#define T_F11       0x280
#define T_F12       0x290
#define T_F13       0x2a0
#define T_F14       0x2b0
#define T_F15       0x2c0
#define T_F16       0x2d0
#define T_F17       0x2e0
#define T_F18       0x2f0
#define T_F19       0x300
#define T_F20       0x310
#define T_F21       0x320
#define T_F22       0x330
#define T_F23       0x340
#define T_F24       0x350
#define T_F25       0x360
#define T_F26       0x370
#define T_F27       0x380
#define T_F28       0x390
#define T_F29       0x3a0
#define T_F30       0x3b0
#define T_F31       0x3c0
#define T_FPSR      0x1e0
#define T_B0        0x190
#define T_B1        0x198
#define T_B2        0x1a0
#define T_B3        0x1a8
#define T_B4        0x1b0
#define T_B5        0x1b8
#define T_B6        0x1c0
#define T_B7        0x1c8
#define T_EC        0x1d0
#define T_LC        0x1d8
#define J_NATS      0x8
#define J_PFS       0x10
#define J_BSP       0x18
#define J_RNAT      0x20
#define J_PREDS     0x28
#define J_LC        0x30
#define J_R4        0x38
#define J_R5        0x40
#define J_R6        0x48
#define J_R7        0x50
#define J_SP        0x58
#define J_F2        0x60
#define J_F3        0x70
#define J_F4        0x80
#define J_F5        0x90
#define J_F16       0xa0
#define J_F17       0xb0
#define J_F18       0xc0
#define J_F19       0xd0
#define J_F20       0xe0
#define J_F21       0xf0
#define J_F22       0x100
#define J_F23       0x110
#define J_F24       0x120
#define J_F25       0x130
#define J_F26       0x140
#define J_F27       0x150
#define J_F28       0x160
#define J_F29       0x170
#define J_F30       0x180
#define J_F31       0x190
#define J_FPSR      0x1a0
#define J_B0        0x1a8
#define J_B1        0x1b0
#define J_B2        0x1b8
#define J_B3        0x1c0
#define J_B4        0x1c8
#define J_B5        0x1d0
#define TRAP_FRAME_LENGTH       0x3d0
#define C_UNAT      0x28
#define C_NATS      0x30
#define C_PFS       0x8
#define C_BSPSTORE      0x10
#define C_RNAT      0x18
#define C_RSC       0x20
#define C_PREDS     0x38
#define C_LC        0x40
#define C_DCR       0x48
#define C_R1        0x50
#define C_GP        0x50
#define C_R4        0x58
#define C_R5        0x60
#define C_R6        0x68
#define C_R7        0x70
#define C_SP        0x78
#define C_R13       0x80
#define C_F2        0x90
#define C_F3        0xa0
#define C_F4        0xb0
#define C_F5        0xc0
#define C_F16       0xd0
#define C_F17       0xe0
#define C_F18       0xf0
#define C_F19       0x100
#define C_F20       0x110
#define C_F21       0x120
#define C_F22       0x130
#define C_F23       0x140
#define C_F24       0x150
#define C_F25       0x160
#define C_F26       0x170
#define C_F27       0x180
#define C_F28       0x190
#define C_F29       0x1a0
#define C_F30       0x1b0
#define C_F31       0x1c0
#define C_FPSR      0x1d0
#define C_B0        0x1d8
#define C_B1        0x1e0
#define C_B2        0x1e8
#define C_B3        0x1f0
#define C_B4        0x1f8
#define C_B5        0x200
#define TT_R2       0x8
#define TT_R3       0x10
#define TT_R8       0x18
#define TT_R9       0x20
#define TT_R10      0x28
#define TT_R11      0x30
#define TT_R14      0x38

#endif _IA64GEN_H
