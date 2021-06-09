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

  PrintWidth.h

Abstract:

  Unicde option for generic width.
  CHAR_W is Unicode
  STRING_W is L""

--*/

#ifndef _PRINT_WIDTH_H_
#define _PRINT_WIDTH_H_

typedef CHAR16        CHAR_W;
#define STRING_W(_s)  L##_s

#define USPrint(Buffer, BufferSize, Format)           SPrint (Buffer, BufferSize, Format)
#define UvSPrint(Buffer, BufferSize, Format, Marker)  VSPrint (Buffer, BufferSize, Format, Marker)


UINTN
AvSPrint (
  OUT CHAR8         *StartOfBuffer,
  IN  UINTN         StrLen,
  IN  CONST CHAR8   *Format,
  IN  VA_LIST       Marker
  );

UINTN
ASPrint (
  OUT CHAR8       *Buffer,
  IN UINTN        BufferSize,
  IN CONST CHAR8  *Format,
  ...
  );

#endif
