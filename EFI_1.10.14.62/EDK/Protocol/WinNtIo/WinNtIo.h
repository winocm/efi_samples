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

  WinNtIo.h

Abstract:

--*/

#ifndef _WIN_NT_IO_H_
#define _WIN_NT_IO_H_

#define EFI_WIN_NT_IO_PROTOCOL_GUID \
  { 0x96eb4ad6, 0xa32a, 0x11d4, 0xbc, 0xfd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

//
// The following APIs require EfiWinNT.h. In some environmnets the GUID 
// definitions are needed but the EfiWinNT.h is not included. 
// EfiWinNT.h is needed to support WINDOWS API requirements.
//
#ifdef _EFI_WIN_NT_H_

#include EFI_PROTOCOL_DEFINITION (WinNtThunk)

typedef struct {
  EFI_WIN_NT_THUNK_PROTOCOL  *WinNtThunk;
  EFI_GUID                   *TypeGuid;
  CHAR16                     *EnvString;
  UINT16                     InstanceNumber;
} EFI_WIN_NT_IO_PROTOCOL;

#endif

extern EFI_GUID gEfiWinNtIoProtocolGuid;

//
// The following GUIDs are used in EFI_WIN_NT_IO_PROTOCOL_GUID 
// Device paths. They map 1:1 with NT envirnment variables. The variables
// define what virtual hardware the emulator/WinNtBusDriver will produce.
//

//
// EFI_WIN_NT_VIRTUAL_DISKS
//
#define EFI_WIN_NT_VIRTUAL_DISKS_GUID \
  { 0xc95a928, 0xa006, 0x11d4, 0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

extern EFI_GUID gEfiWinNtVirtualDisksGuid;

//
// EFI_WIN_NT_PHYSICAL_DISKS
//
#define EFI_WIN_NT_PHYSICAL_DISKS_GUID \
  { 0xc95a92f, 0xa006, 0x11d4, 0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

extern EFI_GUID gEfiWinNtPhysicalDisksGuid;

//
// EFI_WIN_NT_FILE_SYSTEM
//
#define EFI_WIN_NT_FILE_SYSTEM_GUID \
  { 0xc95a935, 0xa006, 0x11d4, 0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

extern EFI_GUID gEfiWinNtFileSystemGuid;

//
// EFI_WIN_NT_SERIAL_PORT
//
#define EFI_WIN_NT_SERIAL_PORT_GUID \
  { 0xc95a93d, 0xa006, 0x11d4, 0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

extern EFI_GUID gEfiWinNtSerialPortGuid;

//
// EFI_WIN_NT_UGA
//
#define EFI_WIN_NT_UGA_GUID \
  { 0xab248e99, 0xabe1, 0x11d4, 0xbd, 0xd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

extern EFI_GUID gEfiWinNtUgaGuid;

//
// EFI_WIN_NT_CONSOLE
//
#define EFI_WIN_NT_CONSOLE_GUID \
  { 0xba73672c, 0xa5d3, 0x11d4, 0xbd, 0x0, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

extern EFI_GUID gEfiWinNtConsoleGuid;


#endif
