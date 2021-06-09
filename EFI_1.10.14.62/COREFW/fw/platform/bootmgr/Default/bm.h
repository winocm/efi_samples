#ifndef _BM_H
#define _BM_H
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

    bm.h

Abstract:

    Boot Manager 



Revision History

--*/


#include "efi.h"
#include "efilib.h"
#include "lang.h"
#include "legacyboot.h"
#include "intload.h"
#include "BmMachine.h"
#include "PlatformLib.h"

//
// Load options
//

#define BM_LOAD_OPTION_SIGNATURE    EFI_SIGNATURE_32('b','m','l','d')

typedef struct {
    UINTN               Signature;
    
    LIST_ENTRY          All;
    LIST_ENTRY          Order;

    CHAR16              *Name;                  // variable name
    UINTN               OptionNumber;           // option # part of name

    UINT32              Attributes;
    UINT32              FilePathListLength;
    CHAR16              *Description;
    EFI_DEVICE_PATH     *FilePath;
    VOID                *LoadOptions;
    UINTN               LoadOptionsSize;

    BOOLEAN             Valid;
} BM_LOAD_OPTION;


//
// Data variables
//


typedef struct {
    // Define the data variable
    CHAR16              *Name;
    UINTN               IntegralSize;
    BOOLEAN             Array;
    BOOLEAN             RuntimeAccess;

    // The contents of the variable
    union {
        VOID            *Data;
        CHAR8           *LangCode;
        CHAR16          *String;
        UINT16          *Value;
    } u;

    UINTN               DataSize;
    UINTN               MaxIndex;
    BOOLEAN             BootDefault;
    BOOLEAN             NonVolatile;

} BM_VARIABLE;


typedef enum {
    BmNoOperation,
    BmSelect,
    BmUp,
    BmDown,
    BmLeft,
    BmRight,
    BmMaxOperation
} BM_SCREEN_OPERATION;

//
// Prototypes from lang.c
//

VOID
BmInitLangCodes(
    VOID
    );

CHAR16 *
BmString (
    IN UINTN    StringNo
    );

//
// Prototypes from display.c
//

VOID
BmInitDisplay(
    VOID
    );

VOID
BmDisplay (
    IN UINTN            StringNo
    );


VOID
BmDisplayString (
    IN UINTN    StringNo
    );

VOID
BmNewLine(
    VOID
    );

VOID
BmClearLine (
    VOID
    );

VOID
BmPause (
    VOID
    );

EFI_STATUS
BmBootMenu (
    OUT BOOLEAN *Maintenance
    );

VOID
BmBanner (
    VOID
    );

//
// Prototypes from load.c
//

VOID
BmLoadDrivers (
    VOID
    );

EFI_STATUS
BmLoad (
    IN BM_LOAD_OPTION   *Option,
    IN BOOLEAN          BootLoad,
    IN BOOLEAN          AutoBoot
    );

VOID
BmAddDefaultBootOptions(
    VOID
    );

VOID
BmGetImageOption(
    IN BOOLEAN          AddBootOption
    );

EFI_STATUS
BmBootNextOption(
    VOID
    );

//
// Prototypes from var.c
//

VOID
BmReadVariables (
    VOID
    );

VOID
BmRemoveVariable (
    IN CHAR16           *Name
    );

VOID
BmDeleteAllLoadOptions (
    IN BM_VARIABLE      *Order,
    IN LIST_ENTRY       *OrderList
    );

VOID
BmSetVariable (
    IN OUT BM_VARIABLE  *Var,
    IN VOID             *Data,
    IN UINTN            DataSize
    );
 
VOID
BmUpdateOrder (
    IN BM_VARIABLE      *Order,
    IN LIST_ENTRY       *OptionList
    );

VOID
BmSetTimeout(
    VOID
    );

VOID
BmSetBootCurrent(
    BOOLEAN Set,
    UINT16  Value
    );

BOOLEAN
BmParseBootOption (
    IN CHAR16       *Name,
    IN UINT8        *Data,
    IN UINTN        DataSize,
    IN CHAR16       *OptionName,
    IN LIST_ENTRY   *OptionList
    );

//
// Prototypes from Default.c
//

EFI_STATUS
BmSetDefaultBootOptions (
    LIST_ENTRY  *BootOptions
    );

EFI_STATUS
BmSetDefaultDriverOptions (
    LIST_ENTRY  *DriverOptions
    );

//
// Externals
//

extern LIST_ENTRY  BmDriverOptions;
extern LIST_ENTRY  BmBootOptions;
extern LIST_ENTRY  BmOrderedDriverOptions;
extern LIST_ENTRY  BmOrderedBootOptions;
extern LEGACY_BOOT_INTERFACE *BmBootLegacy;

extern EFI_HANDLE  BmImageHandle;
extern BOOLEAN BmUpdateVariables;
extern BM_VARIABLE BmLanguage;
extern BM_VARIABLE BmLanguageCodes;
extern BM_VARIABLE BmTimeout;
extern BM_VARIABLE BmBootNext;
extern BM_VARIABLE BmBootOrder;
extern BM_VARIABLE BmDriverOrder;
extern BM_VARIABLE *BmVariables[];

#endif
