///*++
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
//Module Name:  
//
//	IpfMul.s
//  
//Abstract:
//
//  Low level routines for IPF multiply support
//  
//--*/  

.file "IpfMul.s"
.section .text

  .proc MulS64x64#
  .align 32
  .global MulS64x64#
  .align 32

///*++
//
//Routine Description:
//
//	Multiply two 64-bit signed numbers.
//
//    
//Arguments:
//
//	INT64 
//	MulS64x64 (
//	  IN INT64 Value1, 
//	  IN INT64 Value2, 
//	  OUT INT64 *ResultHigh);
//
//Returns:
//
//  64-bit signed result
//  
//--*/

MulS64x64:
  // signed 64x64->128-bit multiply
  // A in r32, B in r33, Q_hi stored in [r34], Q_lo returned in r8
{ .mfi
  alloc r31=ar.pfs,3,0,0,0  // r32-r34
  nop.f 0
  nop.i 0;;
}
{.mmi
  setf.sig f6=r32
  setf.sig f7=r33
  nop.i 0;;
}

{.mfi
  nop.m 0
  xma.h f8=f6,f7,f0
  nop.i 0
}
{.mfi
  nop.m 0
  xma.l f6=f6,f7,f0
  nop.i 0;;
}


{.mmb
  stf8 [r34]=f8
  getf.sig r8=f6
  br.ret.sptk b0;;
}

.endp MulS64x64

  .proc MulU64x64#
  .align 32
  .global MulU64x64#
  .align 32


///*++
//
//Routine Description:
//
//	Multiply two 64-bit unsigned numbers.
//
//    
//Arguments:
//
//	UINT64 
//	MulU64x64 (
//	  IN UINT64 Value1, 
//	  IN UINT64 Value2, 
//	  OUT UINT64 *ResultHigh);
//
//Returns:
//
//  64-bit unsigned result
//  
//--*/
MulU64x64:
  // A in r32, B in r33, Q_hi stored in [r34], Q_lo returned in r8
{ .mfi
  alloc r31=ar.pfs,3,0,0,0  // r32-r34
  nop.f 0
  nop.i 0;;
}
{.mmi
  setf.sig f6=r32
  setf.sig f7=r33
  nop.i 0;;
}

{.mfi
  nop.m 0
  xma.hu f8=f6,f7,f0
  nop.i 0
}
{.mfi
  nop.m 0
  xma.l f6=f6,f7,f0
  nop.i 0;;
}


{.mmb
  stf8 [r34]=f8
  getf.sig r8=f6
  br.ret.sptk b0;;
}

.endp MulU64x64


