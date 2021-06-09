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

    Definition for the Status Bar - 
    the display for the status of the editor

--*/


#ifndef _LIB_STATUS_BAR_H_
#define _LIB_STATUS_BAR_H_

#include "heditortype.h"

  EFI_STATUS  HMainStatusBarInit (VOID);
  EFI_STATUS  HMainStatusBarCleanup (VOID);
  EFI_STATUS  HMainStatusBarRefresh (VOID);
  EFI_STATUS  HMainStatusBarHide (VOID);
  EFI_STATUS  HMainStatusBarSetStatusString (CHAR16*);
  EFI_STATUS  HMainStatusBarSetMode (BOOLEAN);
  EFI_STATUS  HMainStatusBarBackup (VOID);

#endif
