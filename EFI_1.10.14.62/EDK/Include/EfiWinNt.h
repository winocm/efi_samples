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

  EfiWinNt.h

Abstract:

  EFI master include file for WinNt components.

--*/

#ifndef _EFI_WIN_NT_H_

#ifdef EFI_NT_EMULATOR

#define _EFI_WIN_NT_H_

#pragma warning ( disable : 4115 )
#pragma warning ( disable : 4201 )
#pragma warning ( disable : 4214 )

#include "windows.h"
#include "stdio.h"

//
// Set the warnings back on so the EFI code must be /W4.
//
#pragma warning ( default : 4115 )
#pragma warning ( default : 4201 )
#pragma warning ( default : 4214 )

#endif

#include "Efi.h"

#endif

