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

  StdErr.c

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

#include "Efi.h"
#include "EfiDriverLib.h"
#include "EfiCommonLib.h"
#include "EfiPrintLib.h"
#include "Print.h"


UINTN
ErrorPrint (
  IN CONST CHAR16 *ErrorString,
  IN CONST CHAR8  *Format,
  IN VA_LIST      Marker
  )
/*++

Routine Description:

  Print function for a maximum of EFI_DRIVER_LIB_MAX_PRINT_BUFFER ascii 
  characters.

Arguments:

  Format - Ascii format string see file header for more details.

  ...    - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  UINTN   Return;
  UINTN   Index;
  UINTN   MaxIndex;
  CHAR16  Buffer[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];
  CHAR16  UnicodeFormat[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];
  
  MaxIndex = EfiAsciiStrLen ((CHAR8 *)Format);
  if (MaxIndex > EFI_DRIVER_LIB_MAX_PRINT_BUFFER) {
    //
    // Format string was too long for use to process.
    //
    return 0;
  }
  
  if (ErrorString != '\0') {
    if (gST->StdErr != NULL) {
      //
      // To be extra safe make sure StdErr has been initialized
      //
      gST->StdErr->SetAttribute (gST->StdErr, EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));
      gST->StdErr->OutputString (gST->StdErr, (CHAR16 *)ErrorString);
      gST->StdErr->SetAttribute (gST->StdErr, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
    }
  }

  for (Index = 0; Index < MaxIndex; Index++) {
    UnicodeFormat[Index] = (CHAR16)Format[Index];
  }
  UnicodeFormat[Index] = 0;

  Return = VSPrint (Buffer, sizeof(Buffer), UnicodeFormat, Marker);

  //
  // Need to convert to Unicode to do an OutputString
  //

  if (gST->StdErr != NULL) {
    //
    // To be extra safe make sure StdErr has been initialized
    //
    gST->StdErr->OutputString (gST->StdErr, Buffer);
  }

  return Return;
}


UINTN
Aprint (
  IN CONST CHAR8  *Format,
  ...
  )
/*++

Routine Description:

  Print function for a maximum of EFI_DRIVER_LIB_MAX_PRINT_BUFFER ascii 
  characters.

Arguments:

  Format - Ascii format string see file header for more details.

  ...    - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  UINTN   Return;
  VA_LIST Marker;
  UINTN   Index;
  UINTN   MaxIndex;
  CHAR16  Buffer[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];
  CHAR16  UnicodeFormat[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];
  
  MaxIndex = EfiAsciiStrLen ((CHAR8 *)Format);
  if (MaxIndex > EFI_DRIVER_LIB_MAX_PRINT_BUFFER) {
    //
    // Format string was too long for use to process.
    //
    return 0;
  }
  
  for (Index = 0; Index < EFI_DRIVER_LIB_MAX_PRINT_BUFFER; Index++) {
    UnicodeFormat[Index] = (CHAR16)Format[Index];
  }

  VA_START(Marker, Format);
  Return = VSPrint (Buffer, sizeof(Buffer), UnicodeFormat, Marker);
  VA_END (Marker);

  //
  // Need to convert to Unicode to do an OutputString
  //

  if (gST->ConOut != NULL) {
    //
    // To be extra safe make sure ConOut has been initialized
    //
    gST->ConOut->OutputString (gST->ConOut, Buffer);
  }

  return Return;
}
 
static           
UINTN
Print (
  IN CONST CHAR16  *Format,
  ...
  )
/*++

Routine Description:

  Print function for a maximum of EFI_DRIVER_LIB_MAX_PRINT_BUFFER ascii 
  characters.

Arguments:

  Format - Ascii format string see file header for more details.

  ...    - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  UINTN   Return;
  VA_LIST Marker;
  CHAR16  Buffer[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];
  
  VA_START(Marker, Format);
  Return = VSPrint (Buffer, sizeof(Buffer), Format, Marker);
  VA_END (Marker);

  if (gST->ConOut != NULL) {
    //
    // To be extra safe make sure ConOut has been initialized
    //
    gST->ConOut->OutputString (gST->ConOut, Buffer);
  }

  return Return;
}

UINTN
UPrint (
  IN CONST CHAR16  *Format,
  ...
  )
/*++

Routine Description:

  Print function for a maximum of EFI_DRIVER_LIB_MAX_PRINT_BUFFER ascii 
  characters.

Arguments:

  Format - Ascii format string see file header for more details.

  ...    - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  UINTN   Return;
  VA_LIST Marker;
  CHAR16  Buffer[EFI_DRIVER_LIB_MAX_PRINT_BUFFER];
  
  VA_START(Marker, Format);
  Return = VSPrint (Buffer, sizeof(Buffer), Format, Marker);
  VA_END (Marker);

  if (gST->ConOut != NULL) {
    //
    // To be extra safe make sure ConOut has been initialized
    //
    gST->ConOut->OutputString (gST->ConOut, Buffer);
  }

  return Return;
}

