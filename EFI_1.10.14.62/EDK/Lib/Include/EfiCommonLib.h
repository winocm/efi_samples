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

  EfiDriverLib.h

Abstract:

  Light weight lib to support EFI drivers.

--*/

#ifndef _EFI_COMMON_LIB_H_
#define _EFI_COMMON_LIB_H_

//
// Memory
//
#define EFI_UINTN_ALIGN_MASK  (sizeof (UINTN) - 1)
#define EFI_UINTN_ALIGNED(_v) (((UINTN)(_v)) & EFI_UINTN_ALIGN_MASK)

INTN
EfiCompareMem (
  IN VOID     *MemOne,
  IN VOID     *MemTwo,
  IN UINTN    Len
  );

BOOLEAN
EfiCompareGuid (
  IN EFI_GUID *Guid1,
  IN EFI_GUID *Guid2
  );


VOID
EfiSetMem (
  IN VOID   *Buffer,
  IN UINTN  Size,
  IN UINT8  Value    
  );

VOID
EfiCopyMem (
  IN VOID     *Destination,
  IN VOID     *Source,
  IN UINTN    Length
  );

INTN
EfiCompareMem (
  IN VOID     *MemOne,
  IN VOID     *MemTwo,
  IN UINTN    Len
  );

VOID
EfiZeroMem (
  IN VOID     *Buffer,
  IN UINTN    Size
  );



//
// Min Max
//
#define EFI_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define EFI_MAX(a,b) (((a) > (b)) ? (a) : (b))


//
// math.c
//

UINT64
DriverLibMultU64x32 (
  IN  UINT64  Multiplicand,
  IN  UINTN   Multiplier
  );

#define MultU64x32          DriverLibMultU64x32

UINT64
DriverLibDivU64x32 (
  IN  UINT64  Dividend,
  IN  UINTN   Divisor,
  OUT UINTN   *Remainder  OPTIONAL
  );

#define DivU64x32            DriverLibDivU64x32

UINT64
DriverLibRShiftU64 (
  IN  UINT64  Operand,
  IN  UINTN   Count
  );

#define RShiftU64             DriverLibRShiftU64

UINT64
DriverLibLShiftU64 (
  IN  UINT64  Operand,
  IN  UINTN   Count
  );

#define LShiftU64             DriverLibLShiftU64

//
// Unicode String primatives
//

VOID
EfiStrCpy (
  IN CHAR16   *Destination,
  IN CHAR16   *Source
  );

UINTN
EfiStrLen (
  IN CHAR16   *String
  );

UINTN
EfiStrSize (
  IN CHAR16   *String
  );

INTN
EfiStrCmp (
  IN CHAR16   *String,
  IN CHAR16   *String2
  );

VOID
EfiStrCat (
  IN CHAR16   *Destination,
  IN CHAR16   *Source
  );

UINTN
EfiAsciiStrLen (
  IN CHAR8   *String
  );

CHAR8 *
EfiAsciiStrCpy (
  IN CHAR8    *Destination,
  IN CHAR8    *Source
  );

UINTN
EfiAsciiStrnCmp (
    IN CHAR8    *s1,
    IN CHAR8    *s2,
    IN UINTN    len
  );


//
// Print primitives
//

#define LEFT_JUSTIFY    0x01
#define PREFIX_SIGN     0x02
#define PREFIX_BLANK    0x04
#define COMMA_TYPE      0x08
#define LONG_TYPE       0x10
#define PREFIX_ZERO     0x20

UINTN
EfiValueToHexStr (
  IN  OUT CHAR16  *Buffer, 
  IN  UINT64      Value, 
  IN  UINTN       Flags, 
  IN  UINTN       Width
  );

UINTN
EfiValueToString (
  IN  OUT CHAR16  *Buffer, 
  IN  INT64       Value, 
  IN  UINTN       Flags,
  IN  UINTN       Width
  );
  
VOID
EfiStrTrim (
  IN OUT CHAR16   *str,
  IN     CHAR16   c
  );

#endif
