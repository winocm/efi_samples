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

  String.c

Abstract:

  Unicode string primatives

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

static CHAR16 mHexStr[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
                            L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };

VOID
EfiStrCpy (
  IN CHAR16   *Destination,
  IN CHAR16   *Source
  )
/*++

Routine Description:
  Copy the Unicode string Source to Destination.

Arguments:
  Destination - Location to copy string
  Source      - String to copy

Returns:
  NONE

--*/
{
  while (*Source) {
    *(Destination++) = *(Source++);
  }
  *Destination = 0;
}

UINTN
EfiStrLen (
  IN CHAR16   *String
  )
/*++

Routine Description:
  Return the number of Unicode characters in String. This is not the same as
  the length of the string in bytes.

Arguments:
  String - String to process

Returns:
  Number of Unicode characters in String

--*/
{
  UINTN Length;
  
  for (Length=0; *String; String++, Length++);
  return Length;
}


UINTN
EfiStrSize (
  IN CHAR16   *String
  )
/*++

Routine Description:
  Return the number bytes in the Unicode String. This is not the same as
  the length of the string in characters. The string size includes the NULL

Arguments:
  String - String to process

Returns:
  Number of bytes in String

--*/
{
  return ((EfiStrLen (String) + 1) * sizeof (CHAR16));
}


INTN
EfiStrCmp (
  IN CHAR16   *String,
  IN CHAR16   *String2
  )
/*++

Routine Description:
  Return the number bytes in the Unicode String. This is not the same as
  the length of the string in characters. The string size includes the NULL

Arguments:
  String - String to process

Returns:
  Number of bytes in String

--*/
{
  while (*String) {
    if (*String != *String2) {
      break;
    }

    String += 1;
    String2 += 1;
  }

  return *String - *String2;
}


VOID
EfiStrCat (
  IN CHAR16   *Destination,
  IN CHAR16   *Source
  )
/*++

Routine Description:
  Concatinate Source on the end of Destination

Arguments:
  Destination - String to added to the end of.
  Source      - String to concatinate.

Returns:
  NONE

--*/
{   
  EfiStrCpy (Destination + EfiStrLen (Destination), Source);
}


UINTN
EfiAsciiStrLen (
  IN CHAR8   *String
  )
/*++

Routine Description:
  Return the number of Ascii characters in String. This is not the same as
  the length of the string in bytes.

Arguments:
  String - String to process

Returns:
  Number of Unicode characters in String

--*/
{
  UINTN Length;
  
  for (Length=0; *String; String++, Length++);
  return Length;
}


CHAR8 *
EfiAsciiStrCpy (
  IN CHAR8    *Destination,
  IN CHAR8    *Source
  )
/*++

Routine Description:
  Copy the Ascii string Source to Destination.

Arguments:
  Destination - Location to copy string
  Source      - String to copy

Returns:
  Pointer just pass the end of Destination

--*/
{
  while (*Source) {
    *(Destination++) = *(Source++);
  }
  *Destination = 0;
  return Destination + 1;
}

UINTN
EfiAsciiStrnCmp (
    IN CHAR8    *s1,
    IN CHAR8    *s2,
    IN UINTN    len
  )
{
    while (*s1  &&  len) {
        if (*s1 != *s2) {
            break;
        }

        s1  += 1;
        s2  += 1;
        len -= 1;
    }

    return len ? *s1 - *s2 : 0;

}

UINTN
EfiValueToHexStr (
  IN  OUT CHAR16  *Buffer, 
  IN  UINT64      Value, 
  IN  UINTN       Flags, 
  IN  UINTN       Width
  )
/*++

Routine Description:

  VSPrint worker function that prints a Value as a hex number in Buffer

Arguments:

  Buffer - Location to place ascii hex string of Value.

  Value  - Hex value to convert to a string in Buffer.

  Flags  - Flags to use in printing Hex string, see file header for details.

  Width  - Width of hex value.

Returns: 

  Number of characters printed.  

--*/
{
  CHAR16  TempBuffer[30];
  CHAR16  *TempStr;
  CHAR16  Prefix;
  CHAR16  *BufferPtr;
  UINTN   Count;
  UINTN   Index;

  TempStr = TempBuffer;
  BufferPtr = Buffer;

  //
  // Count starts at one since we will null terminate. Each iteration of the
  // loop picks off one nibble. Oh yea TempStr ends up backwards
  //
  Count = 0;
  do {
    Index = ((UINTN)Value & 0xf);
    *(TempStr++) = mHexStr[Index];
    Value = DriverLibRShiftU64 (Value, 4);
    Count++;
  } while (Value != 0);

  if (Flags & PREFIX_ZERO) {
    Prefix = '0';
  } else if (!(Flags & LEFT_JUSTIFY)) { 
    Prefix = ' ';
  } else {
    Prefix = 0x00;
  }                    
  for (Index = Count; Index < Width; Index++) {
    *(TempStr++) = Prefix;
  }

  //
  // Reverse temp string into Buffer.
  //
  while (TempStr != TempBuffer) {
    *(BufferPtr++) = *(--TempStr);
  }  
    
  *BufferPtr = 0;
  return Index;
}


UINTN
EfiValueToString (
  IN  OUT CHAR16  *Buffer, 
  IN  INT64       Value, 
  IN  UINTN       Flags,
  IN  UINTN       Width
  )
/*++

Routine Description:

  VSPrint worker function that prints a Value as a decimal number in Buffer

Arguments:

  Buffer - Location to place ascii decimal number string of Value.

  Value  - Decimal value to convert to a string in Buffer.

  Flags  - Flags to use in printing decimal string, see file header for details.

  Width  - Width of hex value.

Returns: 

  Number of characters printed.  

--*/
{
  CHAR16  TempBuffer[30];
  CHAR16  *TempStr;
  CHAR16  *BufferPtr;
  UINTN   Count;
  UINTN   Remainder;
  CHAR16  Prefix;
  UINTN   Index;

  TempStr = TempBuffer;
  BufferPtr = Buffer;
  Count = 0;

  if (Value < 0) {
    *(BufferPtr++) = '-';
    Value = -Value;
    Count++;
  }

  do {
    Value = (INT64)DriverLibDivU64x32 ((UINT64)Value, 10, &Remainder);
    *(TempStr++) = (CHAR16)(Remainder + '0');
    Count++;
    if ((Flags & COMMA_TYPE) == COMMA_TYPE) {
      if (Count % 3 == 0) {
        *(TempStr++) = ',';
      }
    }
  } while (Value != 0);

 if (Flags & PREFIX_ZERO) {
    Prefix = '0';
  } else if (!(Flags & LEFT_JUSTIFY)) { 
    Prefix = ' ';
  } else {
    Prefix = 0x00;
  }                    
  for (Index = Count; Index < Width; Index++) {
    *(TempStr++) = Prefix;
  }


  //
  // Reverse temp string into Buffer.
  //
  while (TempStr != TempBuffer) {
    *(BufferPtr++) = *(--TempStr);
  }  

  *BufferPtr = 0;
  return Index;
}

VOID
EfiStrTrim (
  IN OUT CHAR16   *str,
  IN     CHAR16   c
  )
/*++

Routine Description:
  
  Removes (trims) specified leading and trailing characters from a string.
  
Arguments: 
  
  str     - Pointer to the null-terminated string to be trimmed. On return, 
            str will hold the trimmed string. 
  c       - Character will be trimmed from str.
  
Returns:

--*/
{
  CHAR16  *p1, *p2;
  
  if (*str == 0) {
    return;
  }
  
  //
  // Trim off the leading and trailing characters c
  //
  for (p1 = str; *p1 && *p1 == c; p1++) {
    ;
  }
  
  p2 = str;
  
  while (*p1) {
    *p2 = *p1;
    p1++;
    p2++;
  }
  
  *p2 = 0;
  
  for (p1 = str + EfiStrLen(str) - 1; p1 >= str && *p1 == c; p1--) {
    ;
  }
  
  *(p1 + 1) = 0;
}
