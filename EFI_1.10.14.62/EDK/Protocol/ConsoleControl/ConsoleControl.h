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

  ConsoleControl.h

Abstract:

  Abstraction of a Text mode or UGA screen

--*/

#ifndef __CONSOLE_CONTROL_H__
#define __CONSOLE_CONTROL_H__

#define EFI_CONSOLE_CONTROL_PROTOCOL_GUID \
  { 0xf42f7782, 0x12e, 0x4c12, 0x99, 0x56, 0x49, 0xf9, 0x43, 0x4, 0xf7, 0x2e }

typedef struct _EFI_CONSOLE_CONTROL_PROTOCOL   EFI_CONSOLE_CONTROL_PROTOCOL;


typedef enum {
  EfiScreenText,
  EfiScreenGraphics,
  EfiScreenTextMaxValue
} EFI_SCREEN_MODE_ENUM;


typedef
EFI_STATUS
(EFIAPI *EFI_CONSOLE_CONTROL_PROTOCOL_GET_MODE) (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL  *This,
  OUT EFI_SCREEN_MODE_ENUM          *Mode,        OPTIONAL
  OUT BOOLEAN                       *UgaExists,   OPTIONAL  
  OUT BOOLEAN                       *StdInLocked  OPTIONAL
  )
/*++

  Routine Description:
    Return the current video mode information. Also returns info about existence
    of UGA Draw devices in system, and if the Std In device is locked. All the
    arguments are optional and only returned if a non NULL pointer is passed in.

  Arguments:
    This - Protocol instance pointer.
    Mode        - Are we in text of grahics mode.
    UgaExists   - TRUE if UGA Spliter has found a UGA device
    StdInLocked - TRUE if StdIn device is keyboard locked

  Returns:
    EFI_SUCCES      - Mode information returned.

--*/
;


typedef
EFI_STATUS
(EFIAPI *EFI_CONSOLE_CONTROL_PROTOCOL_SET_MODE) (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  OUT EFI_SCREEN_MODE_ENUM            Mode
  )
/*++

  Routine Description:
    Set the current mode to either text or graphics. Graphics is
    for Quiet Boot.

  Arguments:
    This  - Protocol instance pointer.
    Mode  - Mode to set the 

  Returns:
    EFI_SUCCES      - Mode information returned.

--*/
;


typedef
EFI_STATUS
(EFIAPI *EFI_CONSOLE_CONTROL_PROTOCOL_LOCK_STD_IN) (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  IN CHAR16                           *Password
  )
/*++

  Routine Description:
    Lock Std In devices until Password is typed.

  Arguments:
    This     - Protocol instance pointer.
    Password - Password needed to unlock screen. NULL means unlock keyboard

  Returns:
    EFI_SUCCES       - Mode information returned.
    EFI_DEVICE_ERROR - Std In not locked

--*/
;



typedef struct _EFI_CONSOLE_CONTROL_PROTOCOL {
  EFI_CONSOLE_CONTROL_PROTOCOL_GET_MODE           GetMode;
  EFI_CONSOLE_CONTROL_PROTOCOL_SET_MODE           SetMode;
  EFI_CONSOLE_CONTROL_PROTOCOL_LOCK_STD_IN        LockStdIn;
} EFI_CONSOLE_CONTROL_PROTOCOL;

extern EFI_GUID gEfiConsoleControlProtocolGuid;

#endif
