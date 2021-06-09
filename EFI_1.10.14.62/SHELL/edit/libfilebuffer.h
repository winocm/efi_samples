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
    libFileBuffer.h

  Abstract:
    Defines FileBuffer - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file
--*/


#ifndef _LIB_FILE_BUFFER_H_
#define _LIB_FILE_BUFFER_H_

#include "editortype.h"


EFI_STATUS  FileBufferInit (VOID);
EFI_STATUS  FileBufferCleanup (VOID);
EFI_STATUS  FileBufferRefresh (VOID);
EFI_STATUS  FileBufferHide (VOID);
EFI_STATUS  FileBufferHandleInput (EFI_INPUT_KEY*);
EFI_STATUS  FileBufferBackup (VOID);

EFI_STATUS  FileBufferRestorePosition (VOID);
EFI_STATUS  FileBufferSetFileName ( CHAR16 *);

EFI_STATUS  FileBufferHandleInput ( EFI_INPUT_KEY *);
    
EFI_STATUS  FileBufferRead ( CHAR16 * , BOOLEAN);
EFI_STATUS  FileBufferSave ( CHAR16 * );

EFI_EDITOR_LINE*  FileBufferCreateLine  ( VOID);

EFI_STATUS FileBufferDoCharInput (CHAR16 );
EFI_STATUS FileBufferAddChar ( CHAR16 );

BOOLEAN InCurrentScreen (UINTN ,UINTN ); 
BOOLEAN AboveCurrentScreen (UINTN );
BOOLEAN UnderCurrentScreen (UINTN );
BOOLEAN LeftCurrentScreen (UINTN );
BOOLEAN RightCurrentScreen (UINTN );


VOID FileBufferMovePosition ( UINTN ,UINTN);

EFI_STATUS FileBufferScrollRight ();
EFI_STATUS FileBufferScrollLeft ();
EFI_STATUS FileBufferScrollDown ();
EFI_STATUS FileBufferScrollUp ();
EFI_STATUS FileBufferPageUp ();
EFI_STATUS FileBufferPageDown ();
EFI_STATUS FileBufferHome ();
EFI_STATUS FileBufferEnd ();

EFI_STATUS FileBufferDoReturn ();
EFI_STATUS FileBufferDoBackspace ();
EFI_STATUS FileBufferDoDelete ();

EFI_STATUS FileBufferChangeMode ();

EFI_STATUS FileBufferCutLine ( EFI_EDITOR_LINE ** );
EFI_STATUS FileBufferPasteLine ( );

EFI_STATUS FileBufferGetFileInfo (  EFI_FILE_HANDLE ,
                                    CHAR16 *,
                                    EFI_FILE_INFO ** );

EFI_STATUS FileBufferSearch ( CHAR16 * , UINTN);
EFI_STATUS FileBufferReplace ( CHAR16 * , UINTN);

VOID FileBufferAdjustMousePosition ( INT32 , INT32  );

#endif
