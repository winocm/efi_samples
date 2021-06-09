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

  EfiLibAllocate.c

Abstract:

  Support routines for memory allocation routines for use with drivers.

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

VOID  *
EfiLibAllocatePool (
  IN  UINTN   AllocationSize
  )
{
  EFI_STATUS    Status;
  VOID          *Memory;

  Status = gBS->AllocatePool (EfiBootServicesData, AllocationSize, &Memory);
  if (EFI_ERROR (Status)) {
    return NULL;
  }
  return Memory;
}


VOID  *
EfiLibAllocateZeroPool (
  IN  UINTN   AllocationSize
  )
{
  VOID          *Memory;

  Memory = EfiLibAllocatePool (AllocationSize);
  if (Memory == NULL) {
    return Memory;
  }
  EfiZeroMem (Memory, AllocationSize);
  return Memory;
}
