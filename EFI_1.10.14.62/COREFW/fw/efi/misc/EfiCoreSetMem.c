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

  EfiCoreSetMem.c

Abstract:

  Implementation of the EfiSetMem routine. This function is broken
  out into its own source file so that it can be excluded from a
  build for a particular platform easily if an optimized version 
  is desired.

--*/

#include "Efi.h"

VOID
EfiCoreZeroMem (
  IN VOID     *Buffer,
  IN UINTN    Size
  );

VOID
EfiCoreSetMem (
  IN VOID   *Buffer,
  IN UINTN  Size,
  IN UINT8  Value    
  )
/*++

Routine Description:

  Set Buffer to Value for Size bytes.

Arguments:

  Buffer  - Memory to set.

  Size    - Number of bytes to set

  Value   - Value of the set operation.

Returns:

  None

--*/
{
  INT8  *Ptr;

  if (Value == 0) {
    EfiCoreZeroMem(Buffer, Size);
    return;
  } 
  Ptr = Buffer;
  while (Size--) {
    *(Ptr++) = Value;
  }
}


