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
  ComponentName.c

Abstract:
  Component name protocol member functions for DebugPort...
  
--*/

#include "DebugPort.h"

static EFI_UNICODE_STRING_TABLE mDebugPortDriverNameTable[] = {
  { "eng", L"DebugPort Driver" },
  { NULL, NULL }
};

EFI_STATUS
DebugPortComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
/*++

  Routine Description:
    Retrieves a Unicode string that is the user readable name of the EFI Driver.

  Arguments:
    This       - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
    Language   - A pointer to a three character ISO 639-2 language identifier.
                 This is the language of the driver name that that the caller 
                 is requesting, and it must match one of the languages specified
                 in SupportedLanguages.  The number of languages supported by a 
                 driver is up to the driver writer.
    DriverName - A pointer to the Unicode string to return.  This Unicode string
                 is the name of the driver specified by This in the language 
                 specified by Language.

  Returns:
    EFI_SUCCES            - The Unicode string for the Driver specified by This
                            and the language specified by Language was returned 
                            in DriverName.
    EFI_INVALID_PARAMETER - Language is NULL.
    EFI_INVALID_PARAMETER - DriverName is NULL.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            language specified by Language.

--*/
{
  return EfiLibLookupUnicodeString (
           Language,
           gDebugPortDevice->ComponentNameInterface.SupportedLanguages,
           mDebugPortDriverNameTable, 
           DriverName
           );
}

EFI_STATUS
DebugPortComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_HANDLE                   ChildHandle        OPTIONAL,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **ControllerName
  )
/*++

  Routine Description:
    The debug port driver does not support GetControllerName, so this function
    is just stubbed and returns EFI_UNSUPPORTED.

  Arguments:
    Per EFI 1.10 driver model

  Returns:
    EFI_UNSUPPORTED

--*/
{
  return EFI_UNSUPPORTED;
}
