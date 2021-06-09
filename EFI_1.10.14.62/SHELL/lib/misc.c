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

  misc.c

Abstract:

  Initialize the shell library



Revision History

--*/

#include "shelllib.h"

EFI_STATUS
ShellExecute (
  IN EFI_HANDLE       ImageHandle,
  IN CHAR16           *CmdLine,
  IN BOOLEAN          Output
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  return SE->Execute (ImageHandle, CmdLine, Output);
}


BOOLEAN
ShellBatchIsActive ( 
  VOID
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  return SE->BatchIsActive ();
}


VOID
ShellFreeResources ( 
  VOID
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  SE->FreeResources ();
}


CHAR16 *
MemoryTypeStr (
  IN EFI_MEMORY_TYPE  Type
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  return (Type >= 0 && Type < EfiMaxMemoryType) ? ShellLibMemoryTypeDesc[Type] : NULL;
}
