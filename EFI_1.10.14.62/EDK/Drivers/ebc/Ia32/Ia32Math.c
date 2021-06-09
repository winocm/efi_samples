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

	Ia32math.c
  
Abstract:

  Generic math routines for EBC interpreter running on IA32 processor.
  
--*/  

#include "Efi.h"

UINT64
LeftShiftU64 (
  IN UINT64   Operand,
  IN UINT64   CountIn
  )
/*++

Routine Description:
  
  Left-shift a 64-bit value.

Arguments:

  Operand - the value to shift
  Count   - shift count

Returns:

  Operand << Count

--*/
{
  UINT64      Result;
  UINT32      Count;
  
  if (CountIn > 63) {
    return 0;
  }
  Count = (UINT32)CountIn;
  _asm {
      mov     eax, dword ptr Operand[0]
      mov     edx, dword ptr Operand[4]
      mov     ecx, Count
      shld    edx, eax, cl
      shl     eax, cl
      cmp     ecx, 32
      jc      short ls10
      mov     edx, eax
      xor     eax, eax
ls10:
      mov     dword ptr Result[0], eax
      mov     dword ptr Result[4], edx
  }
  return Result;
}

UINT64
RightShiftU64 (
  IN UINT64   Operand,
  IN UINT64   CountIn
  )
/*++

Routine Description:
  
  Right-shift an unsigned 64-bit value.

Arguments:

  Operand - the value to shift
  Count   - shift count

Returns:

  Operand >> Count


--*/
{
  UINT64      Result;
  UINT32      Count;
  
  if (CountIn > 63) {
    return 0;
  }
  Count = (UINT32)CountIn;
  
  _asm {
      mov     eax, dword ptr Operand[0]
      mov     edx, dword ptr Operand[4]
      mov     ecx, Count
      shrd    eax, edx, cl
      shr     edx, cl
      cmp     ecx, 32
      jc      short rs10
      mov     eax, edx
      xor     edx, edx
rs10:
      mov     dword ptr Result[0], eax
      mov     dword ptr Result[4], edx
  }

  return Result;
}


INT64
ARightShift64 (
  IN INT64  Operand,
  IN UINT64 CountIn
  )
/*++

Routine Description:
  
  Arithmatic shift a 64 bit signed value.

Arguments:

  Operand - the value to shift
  Count   - shift count

Returns:

  (INT64)Operand >> Count

--*/
{
  INT64   Result;
  UINT32  Count;
  //
  // If they exceeded the max shift count, then return either 0 or all F's
  // depending on the sign bit. Assume the compiler simplifies the << 63.
  //
  if (CountIn > 63) {
    if (Operand & (0x01 << 63)) {
      return (INT64)~0;
    }
    return 0;
  }
  Count = (UINT32)CountIn;
  _asm {
    mov  eax, dword ptr Operand[0]
    mov  edx, dword ptr Operand[4]
    mov  ecx, Count
//    and  ecx, 63 // ecx in {0, 1, ..., 63}

    shrd eax, edx, cl // edx undefined if cl > 31
    sar  edx, cl // shift right by cl & 0x1f, in {0, 1, ..., 31}

    cmp  ecx, 32
    jc   short ARShift64_done

    mov  eax, edx // if ecx >= 32, then eax = edx, and edx = sign bit
    sar  edx, 31 // fill in with sign bit; 31 - (cl & 0x1f) would be ebnough

ARShift64_done:
    mov     dword ptr Result[0], eax
    mov     dword ptr Result[4], edx
  }

  return (Result);
}


UINT64 
MulU64x64 (
  UINT64 Value1, 
  UINT64 Value2, 
  UINT64 *ResultHigh
  )
/*++

Routine Description:
  
  Multiply two unsigned 64-bit values.

Arguments:

  Value1      - first value to multiply
  Value2      - value to multiply by Value1
  ResultHigh  - result to flag overflows

Returns:

  Value1 * Value2

Note:

  The 128-bit result is the concatenation of *ResultHigh and the return value 
  The product fits in 64 bits if *ResultHigh == 0x0000000000000000

  Method 1. Use four 32-bit multiplications:
      a * b = 2^64 * a_h * b_h + 2^32 * (a_h * b_l + b_h * a_l) + a_l * b_l
  Method 2. An alternative using only three multiplications is:
      a * b = (2^64 + 2^32) * a_h * b_h + 
               2^32 * (a_h - a_l) * (b_h - b_l) + (2^32 + 1) * a_l * b_l
  The first method was implemented, because of the overhead in the second one

--*/
{
  UINT64  Result;
  UINT64  ResHi;
  UINT32  SavedEbx;
  
  _asm {
    mov dword ptr SavedEbx, ebx     // save across function calls
    mov eax, dword ptr Value1[0] // a_l -> eax
    mov ebx, dword ptr Value2[0] // b_l -> ebx
    mul ebx // a_lxb_l -> edx:eax
    mov dword ptr Result[0], eax // (a_lxb_l)_LOW -> LL; done LL
    mov ecx, edx // (a_lxb_l)_HIGH -> ecx
    mov eax, dword ptr Value1[4] // a_h -> eax (b_l is in ebx)
    mul ebx // a_hxb_l -> edx:eax
    add eax, ecx // (a_lxb_l)_HIGH + (a_hxb_l)_LOW -> CF:eax
    adc edx, 0x0 // (a_lxb_l)_HIGH + CF -> edx (no carry over, so CF = 0)
    mov dword ptr ResHi[0], edx // (a_hxb_l)_HIGH + CF -> HL; partial result
    mov ecx, eax // (a_lxb_l)_HIGH + (a_hxb_l)_LOW -> ecx
    mov eax, dword ptr Value1[0] // a_l -> eax
    mov ebx, dword ptr Value2[4] // b_h -> ebx
    mul ebx // b_hxa_l -> edx:eax
    add eax, ecx // (a_lxb_l)_HIGH + (a_hxb_l)_LOW + (b_hxa_l)_LOW -> CF:eax
    mov dword ptr Result[4], eax // eax -> LH; done LH
    adc edx, 0x0 // add CF to (b_hxa_l)_HIGH (no carry over, so CF = 0)
    mov ecx, edx // (b_hxa_l)_HIGH -> ecx
    mov eax, dword ptr Value1[4] // a_h -> eax (b_h is in ebx)
    mul ebx // a_hxb_h -> edx:eax
    add eax, ecx // (a_hxb_h)_LOW + (b_hxa_l)_HIGH -> CF:eax
    adc edx, 0x0 // (a_hxb_h)_HIGH + CF -> edx (no carry over, so CF = 0)
    add dword ptr ResHi[0], eax // HL; done HL
    adc edx, 0x0
    mov dword ptr ResHi[4], edx // HH; done HH
    mov ebx, dword ptr SavedEbx
  }

  *ResultHigh = ResHi;
  return (Result);

}


INT64
MulS64x64 (
  INT64 Value1,
  INT64 Value2,
  INT64 *ResultHigh
  )
/*++

Routine Description:
  
  Multiply two signed 64-bit values.

Arguments:

  Value1      - first value to multiply
  Value2      - value to multiply by Value1
  ResultHigh  - result to flag overflows

Returns:

  Value1 * Value2

Note:

  The 128-bit result is the concatenation of *ResultHigh and the return value.
  The product fits in 64 bits if 
     (*ResultHigh == 0x0000000000000000 AND *ResultLow_bit63 == 0)
                                     OR
     (*ResultHigh == 0xffffffffffffffff AND *ResultLow_bit63 == 1)

  Method 1. Use four 32-bit multiplications:
      a * b = 2^64 * a_h * b_h + 2^32 * (a_h * b_l + b_h * a_l) + a_l * b_l
  Method 2. An alternative using only three multiplications is:
      a * b = (2^64 + 2^32) * a_h * b_h +
               2^32 * (a_h - a_l) * (b_h - b_l) + (2^32 + 1) * a_l * b_l
  The first method was implemented, because of the overhead in the second one

--*/
{
  INT64   Result;
  INT64   ResHi;
  UINT32  SavedEbx;
  UINT32  Sign = 0;

  _asm {
    mov dword ptr SavedEbx, ebx     // save across function calls
    mov ebx, dword ptr Value1[4] // a_h -> ebx
    bt  ebx, 0x1f // CF = 1 if a < 0
    jnc short MulS64xU64_a_positive
    //
    // a is negative
    //
    mov eax, dword ptr Value1[0] // a_l -> eax
    not ebx
    not eax
    add eax, 0x1
    adc ebx, 0x0
    mov dword ptr Value1[0], eax // eax -> a_l
    mov dword ptr Value1[4], ebx // ebx -> a_h
    btc dword ptr Sign[0], 0x0 // complement sign
MulS64xU64_a_positive:
    mov ebx, dword ptr Value2[4] // b_h -> ebx
    bt  ebx, 0x1f // CF = 1 if b < 0
    jnc short MulS64xU64_b_positive
    //
    // b is negative
    //
    mov eax, dword ptr Value2[0] // b_l -> eax
    not ebx
    not eax
    add eax, 0x1
    adc ebx, 0x0
    mov dword ptr Value2[0], eax // eax -> b_l
    mov dword ptr Value2[4], ebx // ebx -> b_h
    btc dword ptr Sign[0], 0x0 // complement sign
MulS64xU64_b_positive:
    //
    // multiply |a| and |b|
    //
    mov eax, dword ptr Value1[0] // a_l -> eax
    mov ebx, dword ptr Value2[0] // b_l -> ebx
    mul ebx // a_lxb_l -> edx:eax
    mov dword ptr Result[0], eax // (a_lxb_l)_LOW -> LL; done LL
    mov ecx, edx // (a_lxb_l)_HIGH -> ecx
    mov eax, dword ptr Value1[4] // a_h -> eax (b_l is in ebx)
    mul ebx // a_hxb_l -> edx:eax
    add eax, ecx // (a_lxb_l)_HIGH + (a_hxb_l)_LOW -> CF:eax
    adc edx, 0x0 // (a_lxb_l)_HIGH + CF -> edx (no carry over, so CF = 0)
    mov dword ptr ResHi[0], edx // (a_hxb_l)_HIGH + CF -> HL; partial result
    mov ecx, eax // (a_lxb_l)_HIGH + (a_hxb_l)_LOW -> ecx
    mov eax, dword ptr Value1[0] // a_l -> eax
    mov ebx, dword ptr Value2[4] // b_h -> ebx
    mul ebx // b_hxa_l -> edx:eax
    add eax, ecx // (a_lxb_l)_HIGH + (a_hxb_l)_LOW + (b_hxa_l)_LOW -> CF:eax
    mov dword ptr Result[4], eax // eax -> LH; done LH
    adc edx, 0x0 // add CF to (b_hxa_l)_HIGH (no carry over, so CF = 0)
    mov ecx, edx // (b_hxa_l)_HIGH -> ecx
    mov eax, dword ptr Value1[4] // a_h -> eax (b_h is in ebx)
    mul ebx // a_hxb_h -> edx:eax
    add eax, ecx // (a_hxb_h)_LOW + (b_hxa_l)_HIGH -> CF:eax
    adc edx, 0x0 // (a_hxb_h)_HIGH + CF -> edx (no carry over, so CF = 0)
    add dword ptr ResHi[0], eax // HL; done HL
    adc edx, 0x0
    mov dword ptr ResHi[4], edx // HH; done HH
    bt  dword ptr Sign[0], 0x0 // sign -> CF
    jnc short MulS64xU64_done
    mov eax, dword ptr Result[0] // LL
    mov ebx, dword ptr Result[4] // LH
    mov ecx, dword ptr ResHi[0] // HL
    mov edx, dword ptr ResHi[4] // HH
    not edx
    not ecx
    not ebx
    not eax
    add eax, 0x1
    adc ebx, 0x0
    adc ecx, 0x0
    adc edx, 0x0
    mov dword ptr Result[0], eax // LL
    mov dword ptr Result[4], ebx // LH
    mov dword ptr ResHi[0], ecx // HL
    mov dword ptr ResHi[4], edx // HH
MulS64xU64_done:
    mov ebx, dword ptr SavedEbx
  }

  *ResultHigh = ResHi;
  return (Result);

}

UINT64
DivU64x64 (
  IN  UINT64   Dividend,
  IN  UINT64   Divisor,
  OUT UINT64   *Remainder OPTIONAL,
  OUT UINT32   *Error
  )
{
  UINT64      Rem;
  UINT64      Bit;        

  *Error = 0;
  if (Divisor == 0) {
    *Error = 1;
    if (Remainder) {
      *Remainder = 0x8000000000000000;
    }
    return 0x8000000000000000;
  }

  //
  // For each bit in the dividend
  //

  Rem = 0;
  for (Bit=0; Bit < 64; Bit++) {
      _asm {
      shl     dword ptr Dividend[0], 1    ; shift dividend left one
      rcl     dword ptr Dividend[4], 1    
      rcl     dword ptr Rem[0], 1         ; shift rem left one
      rcl     dword ptr Rem[4], 1    
      }
      if (Rem >= Divisor) {
        Dividend |= 1;
        Rem -= Divisor;
      }
  }

  if (Remainder) {
    *Remainder = Rem;
  }

  return Dividend;
}


INT64
DivS64x64 (
  IN  INT64   Dividend,
  IN  INT64   Divisor,
  OUT INT64   *Remainder OPTIONAL,
  OUT UINT32  *Error
  )
{
  UINT64   Quotient;
  BOOLEAN  Negative;
  BOOLEAN  RemainderNegative;

  Negative = FALSE;
  RemainderNegative = FALSE;
  if (Dividend < 0) {
    Dividend = -Dividend;
    Negative = (BOOLEAN)!Negative;
    RemainderNegative = TRUE;
  }
  if (Divisor < 0) {
    Divisor = -Divisor;
    Negative = (BOOLEAN)!Negative;
  }
  Quotient = DivU64x64 ((UINT64)Dividend, (UINT64)Divisor, (INT64 *)Remainder, Error);
  if (*Error) {
    return (INT64)Quotient;
  }
  if (Negative) {
    Quotient = -((INT64)Quotient);
  }
  if (RemainderNegative) {
    if (Remainder) {
      *Remainder = -*Remainder;
    }
  }
  return (INT64)Quotient;
}
