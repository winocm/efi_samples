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

  Print.c

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
#include "EfiCommonLib.h"
#include "PrintWidth.h"
#include "EfiPrintLib.h"
#include "Print.h"


static
CHAR_W *
GetFlagsAndWidth (
  IN  CHAR_W      *Format, 
  OUT UINTN       *Flags, 
  OUT UINTN       *Width,
  IN OUT  VA_LIST *Marker
  );

static
UINTN
GuidToString (
  IN  EFI_GUID  *Guid,
  IN OUT CHAR_W *Buffer,
  IN  UINTN     BufferSize
  );

static
UINTN
TimeToString (
  IN  EFI_TIME  *Time,
  IN OUT CHAR_W *Buffer,
  IN  UINTN     BufferSize
  );

static
UINTN
EfiStatusToString (
  IN EFI_STATUS   Status,
  OUT CHAR_W      *Buffer,
  IN  UINTN       BufferSize
  );

static
UINTN
Atoi (
  CHAR_W  *String
  );


static
UINTN
SPrint (
  OUT CHAR_W        *Buffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR_W  *Format,
  ...
  )
/*++

Routine Description:

  SPrint function to process format and place the results in Buffer.

Arguments:

  Buffer     - Ascii buffer to print the results of the parsing of Format into.

  BufferSize - Maximum number of characters to put into buffer. Zero means no 
               limit.

  Format - Ascii format string see file header for more details.

  ...    - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  UINTN   Return;
  VA_LIST Marker;

  VA_START (Marker, Format);
  Return = VSPrint (Buffer, BufferSize, Format, Marker);
  VA_END (Marker);
  
  return Return;
}


UINTN
VSPrint (
  OUT CHAR_W        *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR_W  *FormatString,
  IN  VA_LIST       Marker
  )
/*++

Routine Description:

  VSPrint function to process format and place the results in Buffer. Since a 
  VA_LIST is used this rountine allows the nesting of Vararg routines. Thus 
  this is the main print working routine

Arguments:

  StartOfBuffer - Unicode buffer to print the results of the parsing of Format into.

  BufferSize    - Maximum number of characters to put into buffer. Zero means 
                  no limit.

  FormatString  - Unicode format string see file header for more details.

  Marker        - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  CHAR_W    *Buffer;
  CHAR8     *AsciiStr;
  CHAR16    *UnicodeStr;
  CHAR_W    *Format;
  UINTN     Index;
  UINTN     Flags;
  UINTN     Width;
  UINTN     Count;
  UINT64    Value;
  EFI_GUID  *TmpGUID;

  //
  // Process the format string. Stop if Buffer is over run.
  //

  Buffer = StartOfBuffer;
  Format = (CHAR_W *)FormatString; 
  for (Index = 0; (*Format != '\0') && (Index < BufferSize); Format++) {
    if (*Format != '%') {
      if (*Format == '\n') {
        //
        // If carage return add line feed
        //
        Buffer[Index++] = '\r';
      }
      Buffer[Index++] = *Format;
    } else {
      
      //
      // Now it's time to parse what follows after %
      //
      Format = GetFlagsAndWidth (Format, &Flags, &Width, &Marker);
      switch (*Format) {
      case 'X':
        Flags |= PREFIX_ZERO;
        Width = sizeof (UINT64) * 2;
        //
        // break skiped on purpose
        //
      case 'x':
        if ((Flags & LONG_TYPE) == LONG_TYPE) {
          Value = VA_ARG (Marker, UINT64);
        } else {
          Value = VA_ARG (Marker, UINTN);
        }
        Index += EfiValueToHexStr (&Buffer[Index], Value, Flags, Width);
        break;

      case 'd':
        if ((Flags & LONG_TYPE) == LONG_TYPE) {
          Value = VA_ARG (Marker, UINT64);
        } else {
          Value = (UINTN)VA_ARG (Marker, UINTN);
        }
        Index += EfiValueToString (&Buffer[Index], Value, Flags, Width);
        break;

      case 's':
      case 'S':
        UnicodeStr = (CHAR16 *)VA_ARG (Marker, CHAR_W *);
        if (UnicodeStr == NULL) {
          UnicodeStr = L"<null string>";
        }
        for (Count = 0 ;*UnicodeStr != '\0'; UnicodeStr++, Count++) {
          Buffer[Index++] = *UnicodeStr;
        }
        //
        // Add padding if needed
        //
        for (; Count < Width; Count++) {
          Buffer[Index++] = ' ';
        }

        break;

      case 'a':
        AsciiStr = (CHAR8 *)VA_ARG (Marker, CHAR8 *);
        if (AsciiStr == NULL) {
          AsciiStr = "<null string>";
        }
        for (Count = 0 ;*AsciiStr != '\0'; AsciiStr++, Count++) {
          Buffer[Index++] = (CHAR_W)*AsciiStr;
        }
        //
        // Add padding if needed
        //
        for (;Count < Width; Count++) {
          Buffer[Index++] = ' ';
        }
        break;

      case 'c':
        Buffer[Index++] = (CHAR_W)VA_ARG (Marker, UINTN);
        break;

      case 'g':
        TmpGUID = VA_ARG (Marker, EFI_GUID *); 
        if (TmpGUID != NULL) {
          Index += GuidToString (
                    TmpGUID, 
                    &Buffer[Index], 
                    BufferSize
                    );
        }
        break;

      case 't':
        Index += TimeToString (
                  VA_ARG (Marker, EFI_TIME *), 
                  &Buffer[Index], 
                  BufferSize
                  );
        break;

      case 'r':
        Index += EfiStatusToString (
                  VA_ARG (Marker, EFI_STATUS), 
                  &Buffer[Index], 
                  BufferSize
                  );
        break;

      case '%':
        Buffer[Index++] = *Format;
        break;
    
      default:
        //
        // if the type is unknown print it to the screen
        //
        Buffer[Index++] = *Format;
      }

    } 
  }
  Buffer[Index++] = '\0'; 
   
  return &Buffer[Index] - StartOfBuffer;
}



static
CHAR_W *
GetFlagsAndWidth (
  IN  CHAR_W      *Format, 
  OUT UINTN       *Flags, 
  OUT UINTN       *Width,
  IN OUT  VA_LIST *Marker
  )
/*++

Routine Description:

  VSPrint worker function that parses flag and width information from the 
  Format string and returns the next index into the Format string that needs
  to be parsed. See file headed for details of Flag and Width.

Arguments:

  Format - Current location in the VSPrint format string.

  Flags  - Returns flags

  Width  - Returns width of element

  Marker - Vararg list that may be paritally consumed and returned.

Returns: 

  Pointer indexed into the Format string for all the information parsed
  by this routine.

--*/
{
  UINTN   Count;
  BOOLEAN Done;

  *Flags = 0;
  *Width = 0;
  for (Done = FALSE; !Done; ) {
    Format++;

    switch (*Format) {

    case '-': *Flags |= LEFT_JUSTIFY; break;
    case '+': *Flags |= PREFIX_SIGN;  break;
    case ' ': *Flags |= PREFIX_BLANK; break;
    case ',': *Flags |= COMMA_TYPE;   break;
    case 'L':
    case 'l': *Flags |= LONG_TYPE;    break;

    case '*':
      *Width = VA_ARG (*Marker, UINTN);
      break;

    case '0':
      *Flags |= PREFIX_ZERO;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      Count = 0;
      do {
        Count = (Count * 10) + *Format - '0';
        Format++;
      } while ((*Format >= '0')  &&  (*Format <= '9'));
      Format--;
      *Width = Count;
      break;

    default:
      Done = TRUE;
    }
  }
  return Format;
}

static
UINTN
GuidToString (
  IN  EFI_GUID  *Guid,
  IN  CHAR_W    *Buffer,
  IN  UINTN     BufferSize
  )
/*++

Routine Description:

  VSPrint worker function that prints an EFI_GUID.

Arguments:

  Guid       - Pointer to GUID to print.

  Buffer     - Buffe to print Guid into.
  
  BufferSize - Size of Buffer.

Returns: 

  Number of characters printed.  

--*/
{
  UINTN Size;

  Size = SPrint (
            Buffer, BufferSize, 
            STRING_W ("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
            Guid->Data1,                    
            Guid->Data2,
            Guid->Data3,
            Guid->Data4[0],
            Guid->Data4[1],
            Guid->Data4[2],
            Guid->Data4[3],
            Guid->Data4[4],
            Guid->Data4[5],
            Guid->Data4[6],
            Guid->Data4[7]
            );

  //
  // SPrint will null terminate the string. The -1 skips the null
  //
  return Size - 1;
}


static
UINTN
TimeToString (
  IN EFI_TIME   *Time,
  OUT CHAR_W    *Buffer,
  IN  UINTN     BufferSize
  )
/*++

Routine Description:

  VSPrint worker function that prints EFI_TIME.

Arguments:

  Time       - Pointer to EFI_TIME sturcture to print.

  Buffer     - Buffer to print Time into.
  
  BufferSize - Size of Buffer.

Returns: 

  Number of characters printed.  

--*/
{ 
  UINTN Size;

  Size = SPrint (
            Buffer, BufferSize, 
            STRING_W ("%02d/%02d/%04d  %02d:%02d"),
            Time->Month,
            Time->Day,
            Time->Year,
            Time->Hour,
            Time->Minute
            );

  //
  // SPrint will null terminate the string. The -1 skips the null
  //
  return Size - 1;
} 


static
UINTN
EfiStatusToString (
  IN EFI_STATUS   Status,
  OUT CHAR_W      *Buffer,
  IN  UINTN       BufferSize
  )
/*++

Routine Description:

  VSPrint worker function that prints EFI_STATUS as a string. If string is
  not known a hex value will be printed.

Arguments:

  Status     -  EFI_STATUS sturcture to print.

  Buffer     - Buffer to print EFI_STATUS message string into.
  
  BufferSize - Size of Buffer.

Returns: 

  Number of characters printed.  

--*/
{
  UINTN   Size;
  CHAR8   *Desc;
  
  if (Status == EFI_SUCCESS) {
    Desc = "Success";
  } else if (Status == EFI_LOAD_ERROR) {
    Desc = "Load Error";
  } else if (Status == EFI_INVALID_PARAMETER) {
    Desc = "Invalid Parameter";
  } else if (Status == EFI_UNSUPPORTED) {
    Desc = "Unsupported";
  } else if (Status == EFI_BAD_BUFFER_SIZE) {
    Desc = "Bad Buffer Size";
  } else if (Status == EFI_BUFFER_TOO_SMALL) {
    Desc = "Buffer Too Small";
  } else if (Status == EFI_NOT_READY) {
    Desc = "Not Ready";
  } else if (Status == EFI_DEVICE_ERROR) {
    Desc = "Device Error";
  } else if (Status == EFI_WRITE_PROTECTED) {
    Desc = "Write Protected";
  } else if (Status == EFI_OUT_OF_RESOURCES) {
    Desc = "Out of Resources";
  } else if (Status == EFI_VOLUME_CORRUPTED) {
    Desc = "Volume Corrupt";
  } else if (Status == EFI_VOLUME_FULL) {
    Desc = "Volume Full";
  } else if (Status == EFI_NO_MEDIA) {
    Desc = "No Media";
  } else if (Status == EFI_MEDIA_CHANGED) {
    Desc = "Media changed";
  } else if (Status == EFI_NOT_FOUND) {
    Desc = "Not Found";
  } else if (Status == EFI_ACCESS_DENIED) {
    Desc = "Access Denied";
  } else if (Status == EFI_NO_RESPONSE) {
    Desc = "No Response";
  } else if (Status == EFI_NO_MAPPING) {
    Desc = "No mapping";
  } else if (Status == EFI_TIMEOUT) {
    Desc = "Time out";
  } else if (Status == EFI_NOT_STARTED) {
    Desc = "Not started";
  } else if (Status == EFI_ALREADY_STARTED) {
    Desc = "Already started";
  } else if (Status == EFI_ABORTED) {
    Desc = "Aborted";
  } else if (Status == EFI_ICMP_ERROR) {
    Desc = "ICMP Error";
  } else if (Status == EFI_TFTP_ERROR) {
    Desc = "TFTP Error";
  } else if (Status == EFI_PROTOCOL_ERROR) {
    Desc = "Protocol Error";
  } else if (Status == EFI_WARN_UNKNOWN_GLYPH) {
    Desc = "Warning Unknown Glyph";
  } else if (Status == EFI_WARN_DELETE_FAILURE) {
    Desc = "Warning Delete Failure";
  } else if (Status == EFI_WARN_WRITE_FAILURE) {
    Desc = "Warning Write Failure";
  } else if (Status == EFI_WARN_BUFFER_TOO_SMALL) {
    Desc = "Warning Buffer Too Small";
  } else {
    Desc = NULL;
  }
  //
  // If we found a match, copy the message to the user's buffer. Otherwise
  // sprint the hex status code to their buffer.
  //
  if (Desc != NULL) {
    Size = SPrint (Buffer, BufferSize, STRING_W ("%a"), Desc);
  } else {
    Size = SPrint (Buffer, BufferSize, STRING_W ("%X"), Status);
  }
  return Size - 1;
}

