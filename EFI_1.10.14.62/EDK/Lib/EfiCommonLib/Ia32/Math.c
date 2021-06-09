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

  math.c

Abstract:

  64-bit Math worker functions for IA-32

--*/

#include "Efi.h"
#include "EfiDriverLib.h"


UINT64
DriverLibLShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  )
// Left shift 64bit by 32bit and get a 64bit result
{
  UINT64      Result;

  _asm {
    mov     eax, dword ptr Operand[0]
    mov     edx, dword ptr Operand[4]
    mov     ecx, Count
    and     ecx, 63

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
DriverLibRShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
// Right shift 64bit by 32bit and get a 64bit result
{
  UINT64      Result;

  _asm {
    mov     eax, dword ptr Operand[0]
    mov     edx, dword ptr Operand[4]
    mov     ecx, Count
    and     ecx, 63

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


UINT64
DriverLibMultU64x32 (
  IN UINT64   Multiplicand,
  IN UINTN    Multiplier
  )
// Multiple 64bit by 32bit and get a 64bit result
{
  UINT64      Result;

  _asm {
    mov     eax, dword ptr Multiplicand[0]
    mul     Multiplier
    mov     dword ptr Result[0], eax
    mov     dword ptr Result[4], edx
    mov     eax, dword ptr Multiplicand[4]
    mul     Multiplier
    add     dword ptr Result[4], eax
  }

  return Result;
}

UINT64
DriverLibDivU64x32 (
  IN UINT64   Dividend,
  IN UINTN    Divisor,
  OUT UINTN   *Remainder OPTIONAL
  )
// divide 64bit by 32bit and get a 64bit result
// N.B. only works for 31bit divisors!!
{
  UINT32      Rem;
  UINT32      Bit;        

  //
  // For each bit in the dividend
  //

  Rem = 0;
  for (Bit=0; Bit < 64; Bit++) {
      _asm {
      shl     dword ptr Dividend[0], 1    ; shift rem:dividend left one
      rcl     dword ptr Dividend[4], 1    
      rcl     dword ptr Rem, 1            

      mov     eax, Rem
      cmp     eax, Divisor                ; Is Rem >= Divisor?
      cmc                                 ; No - do nothing
      sbb     eax, eax                    ; Else, 
      sub     dword ptr Dividend[0], eax  ;   set low bit in dividen
      and     eax, Divisor                ; and
      sub     Rem, eax                    ;   subtract divisor 
    }
  }

  if (Remainder) {
    *Remainder = Rem;
  }

  return Dividend;
}
