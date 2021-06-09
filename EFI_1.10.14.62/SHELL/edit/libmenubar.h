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
    libMenuBar.h

  Abstract:
    Definition of the Menu Bar for the text editor

--*/


#ifndef _LIB_MENU_BAR_H_
#define _LIB_MENU_BAR_H_

#include "editortype.h"

EFI_STATUS  MainMenuBarInit ( VOID);
EFI_STATUS  MainMenuBarCleanup ( VOID);
EFI_STATUS  MainMenuBarBackup( VOID);

EFI_STATUS  MainMenuBarHide ( VOID);
EFI_STATUS  MainMenuBarRefresh ( VOID);
EFI_STATUS  MainMenuBarHandleInput ( EFI_INPUT_KEY*);

EFI_STATUS  MainCommandOpenFile( VOID);
EFI_STATUS  MainCommandSaveFile( VOID);

EFI_STATUS  MainCommandExit( VOID);         

EFI_STATUS  MainCommandCutLine( VOID);         
EFI_STATUS  MainCommandPasteLine ( VOID);         

EFI_STATUS  MainCommandSearch( VOID);         
EFI_STATUS  MainCommandSearchReplace( VOID);         
EFI_STATUS  MainCommandGotoLine( VOID);         




#endif
