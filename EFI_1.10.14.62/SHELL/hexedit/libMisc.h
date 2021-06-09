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
    libMisc.h

  Abstract:
    Definitions for various line and string routines

--*/
#ifndef _LIB_MISC_H_
#define _LIB_MISC_H_

#include "Heditortype.h"


VOID HEditorClearLine ( UINTN );
HEFI_EDITOR_LINE* HLineDup (HEFI_EDITOR_LINE* );
VOID HLineFree (HEFI_EDITOR_LINE* );
VOID HEditorFreePool (VOID *);


HEFI_EDITOR_LINE* HMoveLine ( INTN );
HEFI_EDITOR_LINE* HMoveCurrentLine ( INTN );

UINTN HLineStrInsert ( HEFI_EDITOR_LINE *,CHAR16,UINTN,UINTN);

VOID HLineCat (HEFI_EDITOR_LINE*,HEFI_EDITOR_LINE*);

VOID HLineDeleteAt (HEFI_EDITOR_LINE*,UINTN);

UINTN HUnicodeToAscii  (CHAR16  *,UINTN ,CHAR8   *);

UINTN HStrStr  (CHAR16  *, CHAR16 *);

BOOLEAN HIsValidFileName ( CHAR16 * ) ;

EFI_STATUS HFreeLines (   EFI_LIST_ENTRY   *,  HEFI_EDITOR_LINE *);

INT32 HGetTextX ( INT32 ) ;
INT32 HGetTextY ( INT32 ) ;


EFI_STATUS HXtoi ( CHAR16  *,    UINTN *);

#endif
