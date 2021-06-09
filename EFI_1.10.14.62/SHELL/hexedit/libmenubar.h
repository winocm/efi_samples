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

#include "Heditortype.h"

  EFI_STATUS  HMainMenuBarInit (VOID);
  EFI_STATUS  HMainMenuBarCleanup (VOID);
  EFI_STATUS  HMainMenuBarHide (VOID);
  EFI_STATUS  HMainMenuBarRefresh (VOID);
  EFI_STATUS  HMainMenuBarHandleInput (EFI_INPUT_KEY*);
  EFI_STATUS  HMainMenuBarBackup(VOID);

  EFI_STATUS  HMainCommandOpenFile(VOID);
  EFI_STATUS  HMainCommandOpenDisk(VOID);
  EFI_STATUS  HMainCommandOpenMemory(VOID);

  EFI_STATUS  HMainCommandSaveBuffer(VOID);

  EFI_STATUS  HMainCommandSelectStart(VOID);
  EFI_STATUS  HMainCommandSelectEnd (VOID);

  EFI_STATUS  HMainCommandCut (VOID);
  EFI_STATUS  HMainCommandPaste (VOID);

  EFI_STATUS  HMainCommandGoToOffset(VOID);

  EFI_STATUS  HMainCommandExit(VOID);


#endif
