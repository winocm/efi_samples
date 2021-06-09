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
    libstatusbar.h

  Abstract:
    Definition for the Status Bar - the display for the 
    status of the editor

--*/

#ifndef _LIB_STATUS_BAR_H_
#define _LIB_STATUS_BAR_H_

#include "editortype.h"

EFI_STATUS  MainStatusBarInit (VOID);
EFI_STATUS  MainStatusBarCleanup (VOID);
EFI_STATUS  MainStatusBarRefresh (VOID);
EFI_STATUS  MainStatusBarHide (VOID);
EFI_STATUS  MainStatusBarSetStatusString (CHAR16*);
EFI_STATUS  MainStatusBarSetMode (BOOLEAN);
EFI_STATUS  MainStatusBarBackup (VOID);

#endif
