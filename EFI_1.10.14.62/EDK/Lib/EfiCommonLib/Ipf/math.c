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

  64-bit Math worker functions for Intel® Itanium(TM) processors.

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
  return Operand << Count;
}

UINT64
DriverLibRShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  )
// Right shift 64bit by 32bit and get a 64bit result
{
  return Operand >> Count;
}


UINT64
DriverLibMultU64x32 (
  IN UINT64   Multiplicand,
  IN UINTN    Multiplier
  )
// Multiple 64bit by 32bit and get a 64bit result
{
  return Multiplicand * Multiplier;
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
  if (Remainder) {
    *Remainder = Dividend % Divisor;
  }

  return Dividend / Divisor;
}
