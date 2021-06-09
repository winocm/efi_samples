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
    Definition of the user input bar (the same position with the Status Bar)

--*/

#ifndef _LIB_INPUT_BAR_H_
#define _LIB_INPUT_BAR_H_

#include "heditortype.h"

  EFI_STATUS  HMainInputBarInit (VOID);
  EFI_STATUS  HMainInputBarCleanup (VOID);
  EFI_STATUS  HMainInputBarRefresh (VOID);
  EFI_STATUS  HMainInputBarSetPrompt (CHAR16*);
  EFI_STATUS  HMainInputBarSetStringSize ( UINTN );

#endif
