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

    init.c

Abstract:

    EFI Boot Manager



Revision History

--*/

#include "bm.h"

//
// Internal prototype
//

BM_SUPPORTED_LANGUAGES *
BmFindLanguage (
    IN CHAR8            *LangCode
    );


VOID
BmInitLangCodes(
    VOID
    )
{
    UINTN                   Index, Size;
    CHAR8                   *Buffer, *Pos;
    BM_SUPPORTED_LANGUAGES  *Language;

    //
    // Compute & set are supported languages
    //
    Buffer = AllocatePool(1024);
    Pos = Buffer;

    for (Index=0; BmSupportedLanguages[Index].LangCodes; Index++) {
        Size = strlena(BmSupportedLanguages[Index].LangCodes);
        CopyMem (Pos, BmSupportedLanguages[Index].LangCodes, Size);
        Pos += Size;
    }

    BmSetVariable (&BmLanguageCodes, Buffer, Pos - Buffer);
    FreePool (Buffer);

    //
    // Set the language
    //
    Language = BmFindLanguage(BmLanguage.u.LangCode);

    //
    // If the language was not set, set the selection to be the default
    //
    if (!Language) {
        DEBUG((D_BM|D_INFO, "BmInitLangCodes: Setting default language type\n"));
        BmSetVariable (&BmLanguage, BmDefaultLanguage, ISO_639_2_ENTRY_SIZE);
        Language = BmFindLanguage(BmLanguage.u.LangCode);
        ASSERT (Language);
    }

    BmLanguageStrings = Language->Strings;
}



BM_SUPPORTED_LANGUAGES *
BmFindLanguage (
    IN CHAR8            *LangCode
    )
{
    CHAR8               *Codes;
    UINTN               Index;
    UINTN               Length, Position;

    if (LangCode) {

        //
        // Check all languages in the table
        //
        for (Index=0; BmSupportedLanguages[Index].LangCodes; Index++) {

            Codes = BmSupportedLanguages[Index].LangCodes;
            Length = strlena(Codes);

            //
            // Check each language for this entry
            //
            for (Position=0; Position < Length; Position += ISO_639_2_ENTRY_SIZE) {
                if (CompareMem (LangCode, Codes+Position, ISO_639_2_ENTRY_SIZE) == 0) {
                    return &BmSupportedLanguages[Index];
                }
            }
        }
    }

    return NULL;
}


CHAR16 *
BmString (
    IN UINTN    StringNo
    )
{
    CHAR16      *String;

    ASSERT (StringNo < BM_MAX_STRING);
    String = StringNo < BM_MAX_STRING ? BmLanguageStrings[StringNo] : L"Invalid String";
    ASSERT (String);
    String = String ? String : L"Invalid String";

    return String;
}

