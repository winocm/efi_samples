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

  Efi.h

Abstract:

  EFI master include file.

  This is the main include file for EFI components. There should be
  no defines or macros added to this file, other than the EFI version 
  information already in this file.

  Don't add include files to the list for convenience, only add things
  that are architectural. Don't add Protocols or GUID include files here

--*/

#ifndef _EFI_H_
#define _EFI_H_

//
// EFI Specification Revision information
//
#define EFI_SPECIFICATION_MAJOR_REVISION 1
#define EFI_SPECIFICATION_MINOR_REVISION 10

#define EFI_FIRMWARE_VENDOR         L"INTEL"
#define EFI_FIRMWARE_MAJOR_REVISION 14
#define EFI_FIRMWARE_MINOR_REVISION 62
#define EFI_FIRMWARE_REVISION ( (EFI_FIRMWARE_MAJOR_REVISION <<16) | \
                                (EFI_FIRMWARE_MINOR_REVISION) )

#include "EfiBind.h"
#include "EfiTypes.h"
#include "EfiBuild.h"
#include "EfiApi.h"
#include "EfiStdArg.h"
#include "EfiError.h"
#include "EfiDebug.h"
#include "EfiDevicePath.h"


#endif

