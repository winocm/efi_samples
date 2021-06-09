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

  EfiCompareGuid.c

Abstract:

  Driver library routine to compare two GUIDs.

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

BOOLEAN
EfiCompareGuid (
  IN EFI_GUID *Guid1,
  IN EFI_GUID *Guid2
  )
/*++

Routine Description:

  Compares two GUIDs

Arguments:

  Guid1 - guid to compare

  Guid2 - guid to compare

Returns:
  TRUE     if Guid1 == Guid2
  FALSE    if Guid1 != Guid2

--*/
{
  INT32 Result;
  INT32 *IntGuid1;
  INT32 *IntGuid2;

  //
  // Compare 32 bits at a time
  //

  IntGuid1 = (INT32 *) Guid1;
  IntGuid2 = (INT32 *) Guid2;

  Result  = IntGuid1[0] - IntGuid2[0];
  Result |= IntGuid1[1] - IntGuid2[1];
  Result |= IntGuid1[2] - IntGuid2[2];
  Result |= IntGuid1[3] - IntGuid2[3];

  return (BOOLEAN)(Result == 0);
}

