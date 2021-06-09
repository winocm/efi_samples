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
    libInputBar.h

  Abstract:
    Definition of the user input bar (same position with the Status Bar)

--*/

#ifndef _LIB_INPUT_BAR_H_
#define _LIB_INPUT_BAR_H_

#include "editortype.h"

EFI_STATUS  MainInputBarInit (VOID);
EFI_STATUS  MainInputBarCleanup (VOID);
EFI_STATUS  MainInputBarRefresh (VOID);
EFI_STATUS  MainInputBarSetPrompt (CHAR16*);
EFI_STATUS  MainInputBarSetStringSize ( UINTN );

#endif
