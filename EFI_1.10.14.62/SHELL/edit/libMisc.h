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

#include "editortype.h"


VOID EditorClearLine ( UINTN );
EFI_EDITOR_LINE* LineDup (EFI_EDITOR_LINE* );
VOID LineFree (EFI_EDITOR_LINE* );
VOID EditorFreePool (VOID *);
    

EFI_EDITOR_LINE* MoveLine ( INTN );
EFI_EDITOR_LINE* MoveCurrentLine ( INTN );

UINTN LineStrInsert ( EFI_EDITOR_LINE *,CHAR16,UINTN,UINTN);

VOID    LineCat (EFI_EDITOR_LINE*,EFI_EDITOR_LINE*);
    
VOID    LineDeleteAt (EFI_EDITOR_LINE*,UINTN);

UINTN UnicodeToAscii  (CHAR16  *,UINTN ,CHAR8   *);

UINTN StrStr  (CHAR16  *, CHAR16 *);

BOOLEAN IsValidFileName ( CHAR16 * ) ;

INT32 GetTextX ( INT32 ) ;
INT32 GetTextY ( INT32 ) ;

#endif
