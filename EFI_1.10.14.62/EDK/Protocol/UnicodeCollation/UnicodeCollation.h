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

  UnicodeCollation.h

Abstract:

  Unicode Collation protocol that follows the EFI 1.0 specification.

--*/

#ifndef _UNICODE_COLLATION_H_
#define _UNICODE_COLLATION_H_


#define EFI_UNICODE_COLLATION_PROTOCOL_GUID \
  { 0x1d85cd7f, 0xf43d, 0x11d2, 0x9a, 0xc,  0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

EFI_INTERFACE_DECL (_EFI_UNICODE_COLLATION_PROTOCOL);

//
// Protocol data structures and defines
//

#define EFI_UNICODE_BYTE_ORDER_MARK (CHAR16)(0xfeff)

//
// Protocol member functions
//

typedef
INTN
(EFIAPI *EFI_UNICODE_COLLATION_STRICOLL) (
  IN struct _EFI_UNICODE_COLLATION_PROTOCOL *This,
  IN CHAR16                                 *Str1,
  IN CHAR16                                 *Str2
  )
/*++

  Routine Description:
    Performs a case-insensitive comparison of two Null-terminated Unicode 
    strings.

  Arguments:
    This - Protocol instance pointer.
    Str1 - A pointer to a Null-terminated Unicode string.
    Str2 - A pointer to a Null-terminated Unicode string.

  Returns:
    0   - Str1 is equivalent to Str2
    > 0 - Str1 is lexically greater than Str2
    < 0 - Str1 is lexically less than Str2

--*/
;


typedef
BOOLEAN
(EFIAPI *EFI_UNICODE_COLLATION_METAIMATCH) (
  IN struct _EFI_UNICODE_COLLATION_PROTOCOL *This,
  IN CHAR16                                 *String,
  IN CHAR16                                 *Pattern
  )
/*++

  Routine Description:
    Performs a case-insensitive comparison of a Null-terminated Unicode 
    pattern string and a Null-terminated Unicode string.

  Arguments:
    This - Protocol instance pointer.
    String  - A pointer to a Null-terminated Unicode string.
    Pattern - A pointer to a Null-terminated Unicode pattern string.

  Returns:
    TRUE -  Pattern was found in String.
    FALSE - Pattern was not found in String.

--*/
;


typedef
VOID
(EFIAPI *EFI_UNICODE_COLLATION_STRLWR) (
  IN struct _EFI_UNICODE_COLLATION_PROTOCOL *This,
  IN OUT CHAR16                             *Str
  )
/*++

  Routine Description:
    Converts all the Unicode characters in a Null-terminated Unicode string to 
    lower case Unicode characters.

  Arguments:
    This - Protocol instance pointer.
    String  - A pointer to a Null-terminated Unicode string.

  Returns:
    NONE

--*/
;

typedef
VOID
(EFIAPI *EFI_UNICODE_COLLATION_STRUPR) (
  IN struct _EFI_UNICODE_COLLATION_PROTOCOL *This,
  IN OUT CHAR16                             *Str
  )
/*++

  Routine Description:
   Converts all the Unicode characters in a Null-terminated Unicode string to upper
   case Unicode characters.
  
  Arguments:
    This   - Protocol instance pointer.
    String - A pointer to a Null-terminated Unicode string.

  Returns:
    NONE

--*/
;

typedef
VOID
(EFIAPI *EFI_UNICODE_COLLATION_FATTOSTR) (
  IN struct _EFI_UNICODE_COLLATION_PROTOCOL *This,
  IN UINTN                                  FatSize,
  IN CHAR8                                  *Fat,
  OUT CHAR16                                *String
  )
/*++

  Routine Description:
   Converts an 8.3 FAT file name in an OEM character set to a Null-terminated 
   Unicode string.
  
  Arguments:
    This    - Protocol instance pointer.
    FatSize - The size of the string Fat in bytes.
    Fat     - A pointer to a Null-terminated string that contains an 8.3 file
               name using an OEM character set.
    String  - A pointer to a Null-terminated Unicode string. The string must
               be preallocated to hold FatSize Unicode characters.
  Returns:
    NONE

--*/
;

typedef
BOOLEAN
(EFIAPI *EFI_UNICODE_COLLATION_STRTOFAT) (
  IN struct _EFI_UNICODE_COLLATION_PROTOCOL *This,
  IN CHAR16                                 *String,
  IN UINTN                                  FatSize,
  OUT CHAR8                                 *Fat
  )
/*++

  Routine Description:
    Converts a Null-terminated Unicode string to legal characters in a FAT 
    filename using an OEM character set. 

  Arguments:
    This    - Protocol instance pointer.
    String  - A pointer to a Null-terminated Unicode string. The string must
               be preallocated to hold FatSize Unicode characters.
    FatSize - The size of the string Fat in bytes.
    Fat     - A pointer to a Null-terminated string that contains an 8.3 file
               name using an OEM character set.
  Returns:
    TRUE  - Fat is a Long File Name
    FALSE - Fat is an 8.3 file name

--*/;


typedef struct _EFI_UNICODE_COLLATION_PROTOCOL {

  //
  // general
  //
  EFI_UNICODE_COLLATION_STRICOLL                StriColl;
  EFI_UNICODE_COLLATION_METAIMATCH              MetaiMatch;
  EFI_UNICODE_COLLATION_STRLWR                  StrLwr;
  EFI_UNICODE_COLLATION_STRUPR                  StrUpr;

  //
  // for supporting fat volumes
  //
  EFI_UNICODE_COLLATION_FATTOSTR                FatToStr;
  EFI_UNICODE_COLLATION_STRTOFAT                StrToFat;

  CHAR8                               *SupportedLanguages;
} EFI_UNICODE_COLLATION_PROTOCOL;


extern EFI_GUID gEfiUnicodeCollationProtocolGuid;

#endif
