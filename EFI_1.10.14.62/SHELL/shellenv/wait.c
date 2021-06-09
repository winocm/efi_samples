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

  wait.c
  
Abstract:

  Shell Environment batch wait command

Revision History

--*/

#include "shelle.h"

//
//  Statics
//
STATIC CHAR16 *TargetLabel;

EFI_STATUS
SEnvCmdWait(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Built-in shell command "wait" to wait for specified microseconds.

Arguments:

Returns:

--*/
{
  InitializeShellApplication (ImageHandle, SystemTable);
  
  if ( !SEnvBatchIsActive() ) {
    Print( L"wait: Only supported in script files\n" );
    return EFI_UNSUPPORTED;
  }

  BS->Stall(3000000); 
  return EFI_SUCCESS;
}
