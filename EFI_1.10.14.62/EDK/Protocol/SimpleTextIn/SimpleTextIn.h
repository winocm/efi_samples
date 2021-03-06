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

  SimpleTextIn.h

Abstract:

  Simple Text In protocol from the EFI 1.0 specification.

  Abstraction of a very simple input device like a keyboard or serial
  terminal.

--*/

#ifndef _SIMPLE_TEXT_IN_H_
#define _SIMPLE_TEXT_IN_H_

#define EFI_SIMPLE_TEXT_IN_PROTOCOL_GUID \
  { 0x387477c1, 0x69c7, 0x11d2, 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

EFI_INTERFACE_DECL(_EFI_SIMPLE_TEXT_IN_PROTOCOL);


//
// Data structures
//

typedef struct {
  UINT16                              ScanCode;
  CHAR16                              UnicodeChar;
} EFI_INPUT_KEY;

//
// Required unicode control chars
//
#define CHAR_NULL                       0x0000
#define CHAR_BACKSPACE                  0x0008
#define CHAR_TAB                        0x0009
#define CHAR_LINEFEED                   0x000A
#define CHAR_CARRIAGE_RETURN            0x000D

//
// EFI Scan codes 
//
#define SCAN_NULL                       0x0000
#define SCAN_UP                         0x0001
#define SCAN_DOWN                       0x0002
#define SCAN_RIGHT                      0x0003
#define SCAN_LEFT                       0x0004
#define SCAN_HOME                       0x0005
#define SCAN_END                        0x0006
#define SCAN_INSERT                     0x0007
#define SCAN_DELETE                     0x0008
#define SCAN_PAGE_UP                    0x0009
#define SCAN_PAGE_DOWN                  0x000A
#define SCAN_F1                         0x000B
#define SCAN_F2                         0x000C
#define SCAN_F3                         0x000D
#define SCAN_F4                         0x000E
#define SCAN_F5                         0x000F
#define SCAN_F6                         0x0010
#define SCAN_F7                         0x0011
#define SCAN_F8                         0x0012
#define SCAN_F9                         0x0013
#define SCAN_F10                        0x0014
#define SCAN_ESC                        0x0017


typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_RESET) (
  IN struct _EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN BOOLEAN                              ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES            - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_READ_KEY) (
  IN struct _EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                       *Key
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This   - Protocol instance pointer.
    Key    - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES        - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to 
                        hardware errors.

--*/
;


typedef struct _EFI_SIMPLE_TEXT_IN_PROTOCOL {
  EFI_INPUT_RESET       Reset;
  EFI_INPUT_READ_KEY    ReadKeyStroke;
  EFI_EVENT             WaitForKey;
} EFI_SIMPLE_TEXT_IN_PROTOCOL;

extern EFI_GUID gEfiSimpleTextInProtocolGuid;

#endif
