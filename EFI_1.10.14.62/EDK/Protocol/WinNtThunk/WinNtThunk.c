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

  WinNtThunk.c

Abstract:

  This protocol allows an EFI driver (DLL) in the NT emulation envirnment
  to make Win32 API calls.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION (WinNtThunk)


EFI_GUID gEfiWinNtThunkProtocolGuid = EFI_WIN_NT_THUNK_PROTOCOL_GUID;

EFI_GUID_STRING (&gEfiWinNtThunkProtocolGuid, "EFI Win NT Thunk", "Win32 API thunk protocol");

