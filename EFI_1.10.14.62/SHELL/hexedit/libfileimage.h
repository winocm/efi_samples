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
    libFileImage.h

  Abstract:
    Defines FileImage - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file

--*/


#ifndef _LIB_FILE_IMAGE_H_
#define _LIB_FILE_IMAGE_H_

#include "heditortype.h"


EFI_STATUS  HFileImageInit (VOID);
EFI_STATUS  HFileImageCleanup (VOID);
EFI_STATUS  HFileImageBackup (VOID);

EFI_STATUS  HFileImageSetFileName ( IN CHAR16 * );


EFI_STATUS HFileImageGetFileInfo ( EFI_FILE_HANDLE , 
                                   CHAR16      *, 
                                   EFI_FILE_INFO ** 
                                 );

EFI_STATUS   HFileImageRead ( IN CHAR16*, IN BOOLEAN );
EFI_STATUS   HFileImageSave ( IN CHAR16 * );

#endif
