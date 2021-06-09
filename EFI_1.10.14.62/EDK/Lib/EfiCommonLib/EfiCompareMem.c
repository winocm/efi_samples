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

  EfiCompareMem.c

Abstract:

  Generic compare-memory routine. 

--*/

#include "Efi.h"
#include "EfiDriverLib.h"


INTN
EfiCompareMem (
  IN VOID     *MemOne,
  IN VOID     *MemTwo,
  IN UINTN    Length
  )
/*++

Routine Description:

  Compares two memory buffers of a given length.

Arguments:

  MemOne - First memory buffer

  MemTwo - Second memory buffer

  Len    - Length of Mem1 and Mem2 memory regions to compare

Returns:

  = 0     if MemOne == MemTwo

--*/
{
  UINTN Size;
  CHAR8 *MemOne8;
  CHAR8 *MemTwo8;

  MemOne8 = (CHAR8 *)MemOne;
  MemTwo8 = (CHAR8 *)MemTwo;
  Size = Length;
  if (EFI_UINTN_ALIGNED (MemOne) || EFI_UINTN_ALIGNED (MemTwo) || EFI_UINTN_ALIGNED (Size)) {
    //
    // If Destination/Source/Length not aligned do byte compare
    //
    for (; Size > 0; Size--, MemOne8++, MemTwo8++) {
      if (*MemOne8 != *MemTwo8) {
        return *MemOne8 - *MemTwo8;
      }
    }
  } else {
    //
    // If Destination/Source/Length are aligned do UINTN conpare
    //
    for (; Size > 0; Size -= sizeof (UINTN)) {
      if (*(UINTN *)MemOne8 != *(UINTN *)MemTwo8) {
        return *(UINTN *)MemOne8 - *(UINTN *)MemTwo8;
      }
      MemOne8 += sizeof (UINTN);
      MemTwo8 += sizeof (UINTN);
    }
  }

  return 0;
}


