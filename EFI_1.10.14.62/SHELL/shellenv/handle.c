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

  handle.c
  
Abstract:

  Shell environment handle information management

Revision History

--*/

#include "shelle.h"

UINTN       SEnvNoHandles;
EFI_HANDLE  *SEnvHandles;

VOID
SEnvInitHandleGlobals (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  SEnvNoHandles   = 0;
  SEnvHandles     = NULL;
}

  
VOID
SEnvLoadHandleTable (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // For ease of use the shell maps handle #'s to short numbers.
  //
  // This is only done on request for various internal commands and
  // the references are immediately freed when the internal command
  // completes.
  //

  //
  // Free any old info
  //
  SEnvFreeHandleTable();

  //
  // Load new info
  //
  SEnvHandles = NULL;
  LibLocateHandle (AllHandles, NULL, NULL, &SEnvNoHandles, &SEnvHandles);
}


VOID
SEnvFreeHandleTable (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  if (SEnvNoHandles) {
    SEnvFreeHandleProtocolInfo();

    FreePool (SEnvHandles);
    SEnvHandles = NULL;
    SEnvNoHandles = 0;
  }
}


UINTN
SEnvHandleNoFromStr(
  IN CHAR16       *Str
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN           HandleNo;

  HandleNo = xtoi(Str);
  HandleNo = HandleNo > SEnvNoHandles ? 0 : HandleNo;
  return HandleNo;
}


EFI_HANDLE
SEnvHandleFromStr(
  IN CHAR16       *Str
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN           HandleNo;
  EFI_HANDLE      Handle;

  HandleNo = xtoi(Str) - 1;
  Handle = HandleNo > SEnvNoHandles - 1 ? NULL : SEnvHandles[HandleNo];
  return Handle;
}


UINTN
SEnvHandleToNumber (
  IN  EFI_HANDLE  Handle
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN Index;

  for (Index = 0; Index < SEnvNoHandles; Index++) {
    if (SEnvHandles[Index] == Handle) {
      return Index + 1;
    }
  }
  return 0;
}
