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

    VgaMiniPort.h
    
Abstract:

    Vga Mini port binding for a VGA controller

Revision History

--*/

#ifndef _VGA_MINI_PORT_H
#define _VGA_MINI_PORT_H

#define EFI_VGA_MINI_PORT_PROTOCOL_GUID \
  { 0xc7735a2f, 0x88f5, 0x4882, 0xae, 0x63, 0xfa, 0xac, 0x8c, 0x8b, 0x86, 0xb3 }

EFI_INTERFACE_DECL(_EFI_VGA_MINI_PORT_PROTOCOL);

typedef
EFI_STATUS
(EFIAPI *EFI_VGA_MINI_PORT_SET_MODE) (
  IN struct _EFI_VGA_MINI_PORT_PROTOCOL  *This,
  IN UINTN                               ModeNumber
  );
/*++

  Routine Description:
    Sets the text display mode of a VGA controller

  Arguments:
    This                 - Protocol instance pointer.
    Mode                 - Mode number.  0 - 80x25   1-80x50

  Returns:
    EFI_SUCCES            - The mode was set
    EFI_DEVICE_ERROR      - The device is not functioning properly.

--*/

typedef struct _EFI_VGA_MINI_PORT_PROTOCOL {
  EFI_VGA_MINI_PORT_SET_MODE  SetMode;

  UINT64                      VgaMemoryOffset;
  UINT64                      CrtcAddressRegisterOffset;
  UINT64                      CrtcDataRegisterOffset;

  UINT8                       VgaMemoryBar;
  UINT8                       CrtcAddressRegisterBar;
  UINT8                       CrtcDataRegisterBar;

  UINT8                       MaxMode;
} EFI_VGA_MINI_PORT_PROTOCOL;

extern EFI_GUID gEfiVgaMiniPortProtocolGuid;

#endif
