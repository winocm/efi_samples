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
// BmLanguageStrings - points to the current string table for the selected language
//

CHAR16  **BmLanguageStrings;

//
// BmDriverOptions - list of all DriverXXXX variables
// BmOrderedDriverOptions - same list, but ordered
//

LIST_ENTRY  BmDriverOptions;
LIST_ENTRY  BmOrderedDriverOptions;

//
// BmBootOptions - list of all BootXXXX variables
// BmOrderedBootOptions - same list, but ordered
//

LIST_ENTRY  BmBootOptions;
LIST_ENTRY  BmOrderedBootOptions;

//
// BmImageHandle - Image handle for the boot manager
//

EFI_HANDLE  BmImageHandle;

//
// Callout back to firware to support booting via legacy BIOS
//  if this function is NULL booting the PC AT way is not supported
//  on this platform. This is implementation and is not part of the
//  EFI specification
//
LEGACY_BOOT_INTERFACE *BmBootLegacy;

//
// BmUpdateVariables - TRUE if the variable updates are to be written back
// to the variable store.
//

BOOLEAN BmUpdateVariables = FALSE;


//
// The different variables found in the f/w's variable store
//

BM_VARIABLE BmLanguage = { VarLanguage, 3, FALSE, TRUE, NULL, 0, 0, FALSE, TRUE };
BM_VARIABLE BmLanguageCodes = { VarLanguageCodes, 3, TRUE, FALSE, NULL, 0, 0, FALSE, FALSE };
BM_VARIABLE BmTimeout = { VarTimeout, sizeof(UINT16), FALSE, TRUE, NULL, 0, 0, TRUE, TRUE };
BM_VARIABLE BmBootNext = { VarBootNext, sizeof(UINT16), FALSE, TRUE, NULL, 0, 0, TRUE, TRUE };
BM_VARIABLE BmBootOrder = { VarBootOrder, sizeof(UINT16), TRUE, TRUE, NULL, 0, 0, TRUE, TRUE };
BM_VARIABLE BmDriverOrder = { VarDriverOrder, sizeof(UINT16), TRUE, TRUE, NULL, 0, 0, TRUE, TRUE };


BM_VARIABLE *BmVariables[] = {
    &BmLanguage,
    &BmLanguageCodes,
    &BmTimeout,
    &BmBootNext,
    &BmBootOrder,
    &BmDriverOrder,
    NULL
} ;

