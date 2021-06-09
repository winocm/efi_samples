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
    libMemImage.h

  Abstract:
    Defines MemImage - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file

--*/


#ifndef _LIB_MEM_IMAGE_H_
#define _LIB_MEM_IMAGE_H_

#include "heditortype.h"


EFI_STATUS  HMemImageInit (VOID);
EFI_STATUS  HMemImageCleanup (VOID);
EFI_STATUS  HMemImageBackup (VOID);

EFI_STATUS  HMemImageSetMemOffsetSize (  IN UINTN , IN UINTN );

EFI_STATUS   HMemImageRead ( IN UINTN, IN UINTN, IN BOOLEAN );
EFI_STATUS   HMemImageSave ( IN UINTN, IN UINTN );

#endif
