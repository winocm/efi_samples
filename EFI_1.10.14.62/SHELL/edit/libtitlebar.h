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
    libTitleBar.h

--*/

#ifndef _LIB_TITLE_BAR_H_
#define _LIB_TITLE_BAR_H_

#include "editortype.h"

EFI_STATUS    MainTitleBarInit ( VOID);
EFI_STATUS    MainTitleBarCleanup ( VOID);

EFI_STATUS    MainTitleBarRefresh ( VOID);

EFI_STATUS    MainTitleBarSetTitle ( CHAR16*);
EFI_STATUS    MainTitleBarBackup ( VOID);

#endif
