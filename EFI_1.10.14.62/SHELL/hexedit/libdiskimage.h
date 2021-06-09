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
    libDiskImage.h

  Abstract:
    Defines DiskImage - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file

--*/


#ifndef _LIB_DISK_IMAGE_H_
#define _LIB_DISK_IMAGE_H_

#include "heditortype.h"


EFI_STATUS  HDiskImageInit (VOID);
EFI_STATUS  HDiskImageCleanup (VOID);
EFI_STATUS  HDiskImageBackup (VOID);

EFI_STATUS  HDiskImageSetDiskNameOffsetSize (  IN CHAR16 * , 
                                               IN UINTN , 
                                               IN UINTN 
                                             );

EFI_STATUS   HDiskImageRead ( IN CHAR16 *, IN UINTN, IN UINTN, IN BOOLEAN );
EFI_STATUS   HDiskImageSave ( IN CHAR16*, IN UINTN, IN UINTN );

#endif
