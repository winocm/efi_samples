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

  EbcMath.c

Abstract:

  Math worker functions for EBC. These are written to make driver code
  portable. There's already EBC instructions for doing most of these
  operations.

--*/

#include "Efi.h"

UINT64
DriverLibLShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  )
{
  return Operand << Count;
}

UINT64
DriverLibEbcMultU64x32 (
  IN UINT64   Multiplicand,
  IN UINTN    Multiplier
  )
{
  return Multiplicand * Multiplier;
}

UINT64
DriverLibRShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  )
{
  return Operand >> Count;
}

UINT64
DriverLibDivU64x32 (
  IN UINT64   Dividend,
  IN UINTN    Divisor,
  OUT UINTN   *Remainder OPTIONAL
  )
{
  ASSERT (Divisor != 0);
 
  //
  // Have to compute twice if remainder. No support for 
  // divide-with-remainder in VM.
  //
  if (Remainder != NULL) {
    *Remainder = Dividend % Divisor;
  }
  return Dividend / Divisor;
}
