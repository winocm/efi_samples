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
    libEditor.h

  Abstract:
    Defines the Main Editor data type - 
     - Global variables 
     - Instances of the other objects of the editor
     - Main Interfaces

--*/

#ifndef _LIB_EDITOR_H_
#define _LIB_EDITOR_H_

#include "heditortype.h"

  EFI_STATUS  HMainEditorInit (VOID);
  EFI_STATUS  HMainEditorCleanup (VOID);
  EFI_STATUS  HMainEditorRefresh (VOID);
  EFI_STATUS  HMainEditorKeyInput (VOID);
  EFI_STATUS  HMainEditorBackup (VOID);


#endif
