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

Abstract:




Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlUnicode.h"

//
// Globals
//

CHAR8   *EngUpperMap;
CHAR8   *EngLowerMap;
CHAR8   *EngInfoMap;

STATIC CHAR8 OtherChars[] = {'0','1','2','3','4','5','6','7',
                             '8','9','\\','.','_','^','$','~',
                             '!','#','%','&','-','{','}','(',
                             ')','@','`','\''};

//
// Defines
//

#define CHAR_FAT_VALID      0x01


#define ToUpper(a)  (a <= 0xFF ? EngUpperMap[a] : a)
#define ToLower(a)  (a <= 0xFF ? EngLowerMap[a] : a)

//
// Prototypes
//

INTN
EngStriColl (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *s1,
    IN CHAR16                           *s2
    );

BOOLEAN
EngMetaiMatch (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *String,
    IN CHAR16                           *Pattern
    );


VOID
EngStrLwr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN OUT CHAR16                       *Str
    );

VOID
EngStrUpr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN OUT CHAR16                       *Str
    );

VOID
EngFatToStr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN UINTN                            FatSize,
    IN CHAR8                            *Fat,
    OUT CHAR16                          *String
    );

BOOLEAN
EngStrToFat (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *String,
    IN UINTN                            FatSize,
    OUT CHAR8                           *Fat
    );

//
// Globals
//

EFI_UNICODE_COLLATION_INTERFACE     UnicodeEng = {
    EngStriColl,
    EngMetaiMatch,
    EngStrLwr,
    EngStrUpr,
    EngFatToStr,
    EngStrToFat,
    "eng"
} ;


//
//
//

INTN
EngStriColl (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *s1,
    IN CHAR16                           *s2
    )

{
    while (*s1) {
        if (ToUpper(*s1) != ToUpper(*s2)) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return ToUpper(*s1) - ToUpper(*s2);
}

VOID
EngStrLwr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN OUT CHAR16                       *Str
    )

{
    while (*Str) {
        *Str = ToLower(*Str);
        Str += 1;
    }
}

VOID
EngStrUpr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN OUT CHAR16                       *Str
    )

{
    while (*Str) {
        *Str = ToUpper(*Str);
        Str += 1;
    }
}


BOOLEAN
EngMetaiMatch (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *String,
    IN CHAR16                           *Pattern
    )

{
    CHAR16  c, p, l;

    for (; ;) {
        p = *Pattern;
        Pattern += 1;

        switch (p) {
        case 0:    
            // End of pattern.  If end of string, TRUE match
            return *String ? FALSE : TRUE;     

        case '*':                               
            // Match zero or more chars
            while (*String) {
                if (EngMetaiMatch (This, String, Pattern)) {
                    return TRUE;
                }
                String += 1;
            }
            return EngMetaiMatch (This, String, Pattern);

        case '?':                               
            // Match any one char
            if (!*String) {
                return FALSE;
            }
            String += 1;
            break;

        case '[':                               
            // Match char set
            c = *String;
            if (!c) {
                return FALSE;                       // syntax problem
            }

            l = 0;
            while ( p = *Pattern++ ) {
                if (p == ']') {
                    return FALSE;
                }

                if (p == '-') {                     // if range of chars,
                    p = *Pattern;                   // get high range
                    if (p == 0 || p == ']') {
                        return FALSE;               // syntax problem
                    }

                    if (ToUpper(c) >= ToUpper(l) && ToUpper(c) <= ToUpper(p)) {         // if in range, 
                        break;                      // it's a match
                    }
                }
                
                l = p;
                if (ToUpper(c) == ToUpper(p)) {     // if char matches
                    break;                          // move on
                }
            }
            
            // skip to end of match char set
            while (p && p != ']') {
                p = *Pattern;
                Pattern += 1;
            }

            String += 1;
            break;

        default:
            c = *String;
            if (ToUpper(c) != ToUpper(p)) {
                return FALSE;
            }

            String += 1;
            break;
        }
    }
}


VOID
EngFatToStr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN UINTN                            FatSize,
    IN CHAR8                            *Fat,
    OUT CHAR16                          *String
    )
// Function has to expand dbcs fat chars.
{
    // No dbcs issues, just expand and null terminate
    while (*Fat  && FatSize) {
        *String = *Fat;
        String += 1;
        Fat += 1;
        FatSize -= 1;
    }
    *String = 0;
}


BOOLEAN
EngStrToFat (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *String,
    IN UINTN                            FatSize,
    OUT CHAR8                           *Fat
    )
// Functions has to crunch string to a fat string. Replacing
// any chars that can't be represeted in the fat name.
{
    BOOLEAN             LfnNeeded;

    LfnNeeded = FALSE;
    while (*String && FatSize) {

        //
        // Skip '.' or ' ' when making a fat name
        //

        if (*String != '.' &&  *String != ' ') {
            //
            // If this is a valid fat char, move it.
            // Otherwise, move a '_' and flag the fact that the name needs an Lfn
            //

            if (*String < 0x100  &&  (EngInfoMap[*String] & CHAR_FAT_VALID)) {
                *Fat = EngUpperMap[*String];
            } else {
                *Fat = '_';
                LfnNeeded = TRUE;
            }

            Fat += 1;
            FatSize -= 1;
        }

        String += 1;
    }

    // do not terminate that fat string
    return LfnNeeded;
}

EFI_STATUS
PlInitializeUnicodeStringDevice(
    VOID                                
    )

{
    EFI_HANDLE              Handle;
    EFI_STATUS              Status;
    UINTN                   Index, Index2;
    CHAR8                   *CaseMap;

    //
    // Initialize mapping tables for the supported languages
    //

    CaseMap = AllocatePool (0x300);
    ASSERT (CaseMap);
    EngUpperMap = CaseMap + 0;
    EngLowerMap = CaseMap + 0x100;
    EngInfoMap  = CaseMap + 0x200;

    for (Index=0; Index < 0x100; Index++) {
        EngUpperMap[Index] = (CHAR8) Index;
        EngLowerMap[Index] = (CHAR8) Index;
        EngInfoMap[Index] = 0;

        if ((Index >= 'a'   &&  Index <= 'z')  ||
            (Index >= 0xe0  &&  Index <= 0xf6) ||
            (Index >= 0xf8  &&  Index <= 0xfe)) {

            Index2 = Index - 0x20;
            EngUpperMap[Index] = (CHAR8) Index2;
            EngLowerMap[Index2] = (CHAR8) Index;

            EngInfoMap[Index] |= CHAR_FAT_VALID;
            EngInfoMap[Index2] |= CHAR_FAT_VALID;
        }
    }

    for (Index=0; OtherChars[Index]; Index++) {
        Index2 = OtherChars[Index];
        EngInfoMap[Index2] |= CHAR_FAT_VALID;
    }


    //
    // Create a handle for the device
    //

    Handle = NULL;
    Status = LibInstallProtocolInterfaces (&Handle, &UnicodeCollationProtocol, &UnicodeEng, NULL);
    InitializeUnicodeSupport ("eng");
    return Status;
}
