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

  Debug.c

Abstract:

  Support for Debug primatives. 

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#ifdef EFI_DEBUG
#include "EfiPrintLib.h"
#include EFI_GUID_DEFINITION (GlobalVariable)

static BOOLEAN  EfiDebugVariableRead = FALSE; 
#endif

EFI_STATUS
EfiDebugAssertInit (
  VOID
  )
{
#ifdef EFI_DEBUG
  EFI_STATUS      Status;
  UINT32          Attributes;
  UINTN           DataSize;
  UINTN           Value;

  if (!EfiDebugVariableRead) {
    DataSize = sizeof(Value);
    Status = gRT->GetVariable(L"EFIDebug", &gEfiGlobalVariableGuid, &Attributes, &DataSize, &Value);
    if (!EFI_ERROR (Status)) {
      gErrorLevel = Value;
      EfiDebugVariableRead = TRUE;
    } 
  }
#endif
  return EFI_SUCCESS;
}


VOID
EfiDebugAssert (
  IN CHAR8    *FileName,
  IN INTN     LineNumber,
  IN CHAR8    *Description
  )
/*++

Routine Description:

  Worker function for ASSERT(). If Error Logging hub is loaded log ASSERT
  information. If Error Logging hub is not loaded BREAKPOINT().
  
Arguments:

  FileName    - File name of failing routine.

  LineNumber  - Line number of failing ASSERT().

  Description - Descritption, usally the assertion,
  
Returns:
  
  None

--*/
{
#ifdef EFI_DEBUG
  EfiDebugPrint (EFI_D_ERROR, "%EASSERT FAILED: %a(%d): %a%N\n", FileName, LineNumber, Description);
#endif
  EFI_BREAKPOINT();
}


VOID
EfiDebugPrint (
  IN  UINTN ErrorLevel,
  IN  CHAR8 *Format,
  ...
  )
/*++

Routine Description:

  Worker function for DEBUG(). If Error Logging hub is loaded log ASSERT
  information. If Error Logging hub is not loaded do nothing.
  
Arguments:

  ErrorLevel - If error level is set do the debug print.

  Format     - String to use for the print, followed by Print arguments.
  
Returns:
  
  None

--*/
{
#ifdef EFI_DEBUG
  VA_LIST   Marker;

  VA_START (Marker, Format);
  if (gErrorLevel & ErrorLevel) {
    ErrorPrint (L"", Format, Marker);
  }
  VA_END (Marker);
#endif
}

