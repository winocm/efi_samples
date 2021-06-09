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

  data.c

Abstract:

  Global data in the FAT Filesystem driver

Revision History

--*/


#include "fat.h"


//
// Globals
//

//
// Unicode collation interface pointer
//
EFI_UNICODE_COLLATION_PROTOCOL   *gUnicodeCollationInterface;

//
// FatFsLock - Global lock for synchronizing all requests.
//
EFI_LOCK    FatFsLock = EFI_INITIALIZE_LOCK_VARIABLE (EFI_TPL_CALLBACK);

//
// Filesystem interface functions
//
EFI_FILE FATFileInterface = {
  EFI_FILE_HANDLE_REVISION,
  FatOpen,    
  FatClose,      
  FatDelete,  
  FatRead, 
  FatWrite, 
  FatGetPosition,
  FatSetPosition,
  FatGetInfo, 
  FatSetInfo,
  FatFlush    
};
