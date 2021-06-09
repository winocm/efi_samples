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

    strings.c

Abstract:

    EFI Boot Manager



Revision History

--*/

#include "bm.h"

//
// Our default language code
//
CHAR8   BmDefaultLanguage[] = { 'e', 'n', 'g', 0 };

//
// Strings for English
//
CHAR16  *BmEngStrings[BM_MAX_STRING] = {
    L"%EEFI Boot Manager ver %01d.%02d [%d.%d]\n\n",
    L"Loading device drivers\n\n",
    L"Loading.: %s\n",
    L"Starting: %s\n",
    L"Load of %s failed: %r\n",
    L"Start of %s failed: %r\n",
    L"Paused - press any key to continue ",
    L"%HNo boot options were found\n",
    L"Boot option maintenance menu",
    L"Please select a boot option",
    L"Enter the filename to search for: ",
    L"File '%hs' not found\n",
    L"Use %c and %c to change option(s). Use Enter to select an option\n",
};

//
// Strings for ???
//


//
// Table of all the supported languages
//


BM_SUPPORTED_LANGUAGES BmSupportedLanguages[] = {
    "engenmang",    NULL,       BmEngStrings,

    // End of list
    NULL,           NULL
} ;
