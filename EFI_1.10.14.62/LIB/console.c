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

    console.c

Abstract:




Revision History

--*/

#include "lib.h"



VOID
Output (
    IN CHAR16   *Str
    )
// Write a string to the console at the current cursor location
{
    ST->ConOut->OutputString (ST->ConOut, Str);
}


VOID
Input (
    IN CHAR16    *Prompt OPTIONAL,
    OUT CHAR16   *InStr,
    IN UINTN     StrLen
    )
// Input a string at the current cursor location, for StrLen
{
    IInput (
        ST->ConOut,
        ST->ConIn,
        Prompt,
        InStr,
        StrLen
        );
}

VOID
IInput (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *ConOut,
    IN SIMPLE_INPUT_INTERFACE           *ConIn,
    IN CHAR16                           *Prompt OPTIONAL,
    OUT CHAR16                          *InStr,
    IN UINTN                            StrLen
    )
// Input a string at the current cursor location, for StrLen
{
    EFI_INPUT_KEY                   Key;
    EFI_STATUS                      Status;
    UINTN                           Len;

    if (Prompt) {
        ConOut->OutputString (ConOut, Prompt);
    }

    Len = 0;
    for (; ;) {
        WaitForSingleEvent (ConIn->WaitForKey, 0);

        Status = ConIn->ReadKeyStroke(ConIn, &Key);
        if (EFI_ERROR(Status)) {
            DEBUG((D_ERROR, "Input: error return from ReadKey %x\n", Status));
            break;
        }

        if (Key.UnicodeChar == '\n' ||
            Key.UnicodeChar == '\r') {
            break;
        }
        
        if (Key.UnicodeChar == '\b') {
            if (Len) {
                ConOut->OutputString(ConOut, L"\b \b");
                Len -= 1;
            }
            continue;
        }

        if (Key.UnicodeChar >= ' ') {
            if (Len < StrLen-1) {
                InStr[Len] = Key.UnicodeChar;

                InStr[Len+1] = 0;
                ConOut->OutputString(ConOut, &InStr[Len]);

                Len += 1;
            }
            continue;
        }
    }

    InStr[Len] = 0;
}
