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

  EfiPrintLib.h

Abstract:

  Light weight lib to support EFI drivers.

--*/

#ifndef _EFI_PRINT_LIB_H_
#define _EFI_PRINT_LIB_H_

#include EFI_PROTOCOL_DEFINITION(UgaDraw)

UINTN
ErrorPrint (
  IN CONST CHAR16 *ErrorString,
  IN CONST CHAR8  *Format,
  IN VA_LIST      Marker
  );

UINTN
Aprint (
  IN CONST CHAR8  *Format,
  ...
  );

UINTN
UPrint (
  IN CONST CHAR16  *Format,
  ...
  );

UINTN
VSPrint (
  OUT CHAR16        *StartOfBuffer,
  IN  UINTN         StrLen,
  IN  CONST CHAR16  *Format,
  IN  VA_LIST       Marker
  );

//
// BoxDraw support
//
BOOLEAN
PrintLibIsValidEfiCntlChar (
  IN  CHAR16  c
  );

BOOLEAN
PrintLibIsValidAscii (
  IN  CHAR16  Ascii
  );

BOOLEAN
PrintLibIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi,    OPTIONAL
  OUT CHAR8   *Ascii      OPTIONAL
  );


#endif
