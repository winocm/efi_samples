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

  Sprint.c

Abstract:

  Basic Ascii AvSPrintf() function named VSPrint(). VSPrint() enables very
  simple implemenation of SPrint() and Print() to support debug. 

  You can not Print more than EFI_DRIVER_LIB_MAX_PRINT_BUFFER characters at a 
  time. This makes the implementation very simple.

  VSPrint, Print, SPrint format specification has the follwoing form

  %[flags][width]type

  flags:
    '-' - Left justify
    '+' - Prefix a sign
    ' ' - Prefix a blank
    ',' - Place commas in numberss
    '0' - Prefix for width with zeros
    'l' - UINT64
    'L' - UINT64

  width:
    '*' - Get width from a UINTN argumnet from the argument list
    Decimal number that represents width of print

  type:
    'X' - argument is a UINTN hex number, prefix '0'
    'x' - argument is a hex number
    'd' - argument is a decimal number
    'a' - argument is an ascii string 
    'S','s' - argument is an Unicode string
    'g' - argument is a pointer to an EFI_GUID
    't' - argument is a pointer to an EFI_TIME structure
    'c' - argument is an ascii character
    'r' - argument is EFI_STATUS
    '%' - Print a %

--*/

#include "EfiCommon.h"
#include "PrintWidth.h"
#include "EfiPrintLib.h"
#include "Print.h"


UINTN
USPrint (
  OUT CHAR16        *Buffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *Format,
  ...
  )
{
  UINTN   Return;
  VA_LIST Marker;

  VA_START (Marker, Format);
  Return = UnicodeVSPrint (Buffer, BufferSize, Format, Marker);
  VA_END (Marker);
  
  return Return;
}


UINTN
UvSPrint (
  OUT CHAR16        *Buffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *FormatString,
  IN  VA_LIST       Marker
  )
{
  UINTN   Index;
  CHAR8   AsciiFormat[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];
  CHAR8   AsciiResult[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];

  for (Index = 0; Index < EFI_DRIVER_LIB_MAX_PRINT_BUFFER && FormatString[Index] != '\0'; Index++) {
    AsciiFormat[Index] = (CHAR8)FormatString[Index];
  }
  AsciiFormat[Index] = '\0';

  Index = VSPrint (AsciiResult, EFI_DRIVER_LIB_MAX_PRINT_BUFFER, AsciiFormat, Marker);

  for (Index = 0; (Index < (BufferSize - 1)) && AsciiResult[Index] != '\0'; Index++) {
    Buffer[Index] = (CHAR16)AsciiResult[Index];
  }
  Buffer[Index] = '\0';

  return Index++;
}

