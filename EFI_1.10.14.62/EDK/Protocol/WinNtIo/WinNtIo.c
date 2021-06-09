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

  WinNtIo.c

Abstract:

  This protocol allows an EFI driver (DLL) in the NT emulation envirnment
  to make Win32 API calls.

--*/

#include "EfiWinNt.h"
#include EFI_PROTOCOL_DEFINITION(WinNtIo)

EFI_GUID gEfiWinNtIoProtocolGuid = EFI_WIN_NT_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiWinNtIoProtocolGuid, "EFI Win NT I/O Protocol", "Win32 API I/O protocol");

EFI_GUID gEfiWinNtVirtualDisksGuid    = EFI_WIN_NT_VIRTUAL_DISKS_GUID;
EFI_GUID gEfiWinNtPhysicalDisksGuid   = EFI_WIN_NT_PHYSICAL_DISKS_GUID;
EFI_GUID gEfiWinNtFileSystemGuid      = EFI_WIN_NT_FILE_SYSTEM_GUID;
EFI_GUID gEfiWinNtSerialPortGuid      = EFI_WIN_NT_SERIAL_PORT_GUID;
EFI_GUID gEfiWinNtUgaGuid             = EFI_WIN_NT_UGA_GUID;
EFI_GUID gEfiWinNtConsoleGuid         = EFI_WIN_NT_CONSOLE_GUID;
