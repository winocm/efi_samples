#ifndef _LANG_H
#define _LANG_H
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

    lang.h

Abstract:

    Boot Manager 



Revision History

--*/


//
// Table of language strings
//

typedef struct {
    CHAR8       *LangCodes;
    CHAR8       *Parent;
    CHAR16      **Strings;
} BM_SUPPORTED_LANGUAGES;

extern BM_SUPPORTED_LANGUAGES BmSupportedLanguages[];
extern CHAR8 BmDefaultLanguage[];
extern CHAR16 **BmLanguageStrings;

//
// 
//

#define BM_BANNER                               0
#define BM_LOADING_DEVICE_DRIVERS               1
#define BM_LOAD_IMAGE                           2
#define BM_START_IMAGE                          3
#define BM_LOAD_IMAGE_FAILED                    4
#define BM_START_IMAGE_FAILED                   5
#define BM_PAUSED                               6
#define BM_NO_BOOT_OPTIONS_FOUND                7                
#define BM_MAINTENANCE_MENU                     8
#define BM_SELECT_BOOT_OPTION                   9
#define BM_ENTER_FILE_NAME                      10
#define BM_FILE_NOT_FOUND                       11        
#define BM_HELP_FOOTER                          12 
#define BM_MAX_STRING                           13

#endif













