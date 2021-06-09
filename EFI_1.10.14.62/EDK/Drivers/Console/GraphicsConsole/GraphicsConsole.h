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

  GraphicsConsole.h

Abstract:

  
Revision History

--*/

#ifndef _GRAPHICS_CONSOLE_H
#define _GRAPHICS_CONSOLE_H

#include "Efi.h"
#include "EfiDriverLib.h"

#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (UgaSplash)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)

//
// Glyph database
//
#define GLYPH_WIDTH  8
#define GLYPH_HEIGHT 19

typedef struct {
    CHAR16  UnicodeWeight;
    UINT8   Attributes;
    UINT8   Glyph[GLYPH_HEIGHT];
} NARROW_GLYPH;

typedef struct {
    CHAR16  UnicodeWeight;
    UINT8   Attributes;
    UINT8   Glyph2[GLYPH_HEIGHT*2];
    UINT8   Pad;
    CHAR16  SurrogateWeight;
} WIDE_GLYPH;

extern NARROW_GLYPH UsStdGlyphData[];

//
// Device Structure
//
#define GRAPHICS_CONSOLE_DEV_SIGNATURE   EFI_SIGNATURE_32('g','s','t','o')

typedef struct {
  UINTN           Columns;
  UINTN           Rows;
  INTN            DeltaX;
  INTN            DeltaY;
  UINTN           ClearWidth;
  UINTN           ClearHeight;
} GRAPHICS_CONSOLE_MODE_DATA;

#define GRAPHICS_MAX_MODE 3

typedef struct {
  UINTN                                Signature;
  EFI_HANDLE                           Handle;
  EFI_UGA_DRAW_PROTOCOL                *UgaDraw;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL         SimpleTextOutput;
  EFI_SIMPLE_TEXT_OUTPUT_MODE          SimpleTextOutputMode;
  GRAPHICS_CONSOLE_MODE_DATA           ModeData[GRAPHICS_MAX_MODE];
  EFI_UGA_PIXEL                        *LineBuffer;
} GRAPHICS_CONSOLE_DEV;

#define GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(a) CR(a, GRAPHICS_CONSOLE_DEV, SimpleTextOutput, GRAPHICS_CONSOLE_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gGraphicsConsoleDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gGraphicsConsoleComponentName;

//
// Prototypes
//

EFI_STATUS
InitializeGraphicsConsole (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         ExtendedVerification
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber,
  OUT UINTN                         *Columns,
  OUT UINTN                         *Rows
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Attribute
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  );

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       Visible
  );

VOID
GraphicsConsoleDrawLogo (
  IN  EFI_UGA_DRAW_PROTOCOL  *UgaDraw
  );

#endif




