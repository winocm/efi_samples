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

    TerminalConOut.c
    
Abstract: 
    

Revision History
--*/

#include "Terminal.h"

//
// This list is used to define the valid extend chars.
// It also provides a mapping from Unicode to PCANSI or
// ASCII. The ASCII mapping we just made up.
//
//

STATIC UNICODE_TO_CHAR UnicodeToPcAnsiOrAscii[] = {
    BOXDRAW_HORIZONTAL,                 0xc4, L'-', 
    BOXDRAW_VERTICAL,                   0xb3, L'|',
    BOXDRAW_DOWN_RIGHT,                 0xda, L'/',
    BOXDRAW_DOWN_LEFT,                  0xbf, L'\\',
    BOXDRAW_UP_RIGHT,                   0xc0, L'\\',
    BOXDRAW_UP_LEFT,                    0xd9, L'/',
    BOXDRAW_VERTICAL_RIGHT,             0xc3, L'|',
    BOXDRAW_VERTICAL_LEFT,              0xb4, L'|',
    BOXDRAW_DOWN_HORIZONTAL,            0xc2, L'+',
    BOXDRAW_UP_HORIZONTAL,              0xc1, L'+',
    BOXDRAW_VERTICAL_HORIZONTAL,        0xc5, L'+',
    BOXDRAW_DOUBLE_HORIZONTAL,          0xcd, L'-',
    BOXDRAW_DOUBLE_VERTICAL,            0xba, L'|',
    BOXDRAW_DOWN_RIGHT_DOUBLE,          0xd5, L'/',
    BOXDRAW_DOWN_DOUBLE_RIGHT,          0xd6, L'/',
    BOXDRAW_DOUBLE_DOWN_RIGHT,          0xc9, L'/',
    BOXDRAW_DOWN_LEFT_DOUBLE,           0xb8, L'\\',
    BOXDRAW_DOWN_DOUBLE_LEFT,           0xb7, L'\\',
    BOXDRAW_DOUBLE_DOWN_LEFT,           0xbb, L'\\',
    BOXDRAW_UP_RIGHT_DOUBLE,            0xd4, L'\\',
    BOXDRAW_UP_DOUBLE_RIGHT,            0xd3, L'\\',
    BOXDRAW_DOUBLE_UP_RIGHT,            0xc8, L'\\',
    BOXDRAW_UP_LEFT_DOUBLE,             0xbe, L'/',
    BOXDRAW_UP_DOUBLE_LEFT,             0xbd, L'/',
    BOXDRAW_DOUBLE_UP_LEFT,             0xbc, L'/',
    BOXDRAW_VERTICAL_RIGHT_DOUBLE,      0xc6, L'|',
    BOXDRAW_VERTICAL_DOUBLE_RIGHT,      0xc7, L'|',
    BOXDRAW_DOUBLE_VERTICAL_RIGHT,      0xcc, L'|',
    BOXDRAW_VERTICAL_LEFT_DOUBLE,       0xb5, L'|',
    BOXDRAW_VERTICAL_DOUBLE_LEFT,       0xb6, L'|',
    BOXDRAW_DOUBLE_VERTICAL_LEFT,       0xb9, L'|',
    BOXDRAW_DOWN_HORIZONTAL_DOUBLE,     0xd1, L'+',
    BOXDRAW_DOWN_DOUBLE_HORIZONTAL,     0xd2, L'+',
    BOXDRAW_DOUBLE_DOWN_HORIZONTAL,     0xcb, L'+',
    BOXDRAW_UP_HORIZONTAL_DOUBLE,       0xcf, L'+',
    BOXDRAW_UP_DOUBLE_HORIZONTAL,       0xd0, L'+',
    BOXDRAW_DOUBLE_UP_HORIZONTAL,       0xca, L'+',
    BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE, 0xd8, L'+',
    BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL, 0xd7, L'+',
    BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL, 0xce, L'+',

    BLOCKELEMENT_FULL_BLOCK,            0xdb, L'*',
    BLOCKELEMENT_LIGHT_SHADE,           0xb0, L'+',

    GEOMETRICSHAPE_UP_TRIANGLE,         0x1e, L'^',
    GEOMETRICSHAPE_RIGHT_TRIANGLE,      0x10, L'>',
    GEOMETRICSHAPE_DOWN_TRIANGLE,       0x1f, L'v',
    GEOMETRICSHAPE_LEFT_TRIANGLE,       0x11, L'<',

    ARROW_LEFT,                         0x3c, L'<',

    ARROW_UP,                           0x18, L'^',
    
    ARROW_RIGHT,                        0x3e, L'>',

    ARROW_DOWN,                         0x19, L'v',
    
    0x0000, 0x00
};

CHAR16 mSetModeString[]           = { ESC, '[', '=','3', 'h' , 0 };
CHAR16 mSetAttributeString[]      = { ESC, '[', '0', 'm', ESC, '[', '4', '0', 'm', ESC, '[', '4', '0', 'm', 0 };
CHAR16 mClearScreenString[]       = { ESC, '[', '2', 'J' , 0 };
CHAR16 mSetCursorPositionString[] = { ESC, '[', '0', '0', ';', '0', '0', 'H', 0 };


//
// Body of the ConOut functions
//

EFI_STATUS 
EFIAPI
TerminalConOutReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       ExtendedVerification
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT_PROTOCOL.Reset().
    If ExtendeVerification is TRUE, then perform dependent serial device reset,
    and set display mode to mode 0.
    If ExtendedVerification is FALSE, only set display mode to mode 0.
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    BOOLEAN             IN    ExtendedVerification
        Indicates that the driver may perform a more exhaustive
        verification operation of the device during reset.
        
  Returns:
  
    EFI_SUCCESS
       The reset operation succeeds.   
    
    EFI_DEVICE_ERROR
      The terminal is not functioning correctly or the serial port reset fails.
                
--*/  
{
  EFI_STATUS   Status;
  TERMINAL_DEV *TerminalDevice;

  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(This);
  
  //
  // Perform a more exhaustive reset by resetting the serial port.
  //
  if (ExtendedVerification) {
    
    Status = TerminalDevice->SerialIo->Reset (TerminalDevice->SerialIo);      
    if(EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  //
  //reset the background and keep the foreground unchanged
  //
  This->SetAttribute(This,This->Mode->Attribute & 0xf);
  Status = This->SetMode (This, 0); 
  return Status;
}


EFI_STATUS 
EFIAPI
TerminalConOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT_PROTOCOL.OutputString().
    The Unicode string will be converted to terminal expressible data stream
    and send to terminal via serial port.
    
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    CHAR16                        IN    *WString
        The Null-terminated Unicode string to be displayed on 
        the terminal screen.
        
  Returns:
  
    EFI_SUCCESS
       The string is output successfully.   
    
    EFI_DEVICE_ERROR
      The serial port fails to send the string out.
      
    EFI_WARN_UNKNOWN_GLYPH
      Indicates that some of the characters in the Unicode string could not 
      be rendered and are skipped.          
                
--*/    
{
  TERMINAL_DEV                  *TerminalDevice;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   *Mode;
  UINTN                         MaxColumn;
  UINTN                         MaxRow;
  UINTN                         Length;
  UTF8_CHAR                     Utf8Char;
  CHAR8                         GraphicChar;
  CHAR8                         AsciiChar;
  EFI_STATUS                    Status;
  UINT8                         ValidBytes;
  //
  //  flag used to indicate whether condition happens which will cause
  //  return EFI_WARN_UNKNOWN_GLYPH
  //
  BOOLEAN                       Warning;  
  
  ValidBytes = 0;
  Warning = FALSE;
  
  //
  //  get Terminal device data structure pointer.
  //
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(This);

  //
  //  get current display mode
  //  Terminal driver only support mode 0
  //
  Mode = This->Mode;
  if (Mode->Mode != 0) {
    return EFI_UNSUPPORTED;
  }
  
  This->QueryMode(
      This, 
      Mode->Mode, 
      &MaxColumn, 
      &MaxRow
      );

  for (;*WString != CHAR_NULL; WString++) {

    switch (TerminalDevice->TerminalType) {
    
      case PcAnsiType:
      case VT100Type:
      case VT100PlusType:

        if (!TerminalIsValidTextGraphics (*WString, &GraphicChar, &AsciiChar)) {
          
          //
          // If it's not a graphic character convert Unicode to ASCII.
          //
          GraphicChar = (CHAR8)*WString;  
    
          if (!(TerminalIsValidAscii(GraphicChar) 
              || TerminalIsValidEfiCntlChar(GraphicChar))) {
            
            //            
            // when this driver use the OutputString to output control string,
            // TerminalDevice->OutputEscChar is set to let the Esc char
            // to be output to the terminal emulation software.
            // 
            if ((GraphicChar == 27) && TerminalDevice->OutputEscChar) {
              GraphicChar = 27;
            } else {
              GraphicChar = '?';
              Warning = TRUE;
            }
          }

          AsciiChar = GraphicChar;
             
        } 
        
        if (TerminalDevice->TerminalType != PcAnsiType) {
          GraphicChar = AsciiChar;
        }

        Length = 1;
        
        Status = TerminalDevice->SerialIo->Write(
                                    TerminalDevice->SerialIo, 
                                    &Length, 
                                    &GraphicChar
                                    );
        
        if (EFI_ERROR(Status)) {
          return EFI_DEVICE_ERROR;
        }
        
        break;
      
      case VTUTF8Type:
        UnicodeToUtf8 (*WString,&Utf8Char,&ValidBytes);
        Length = ValidBytes;
        Status = TerminalDevice->SerialIo->Write(
                                    TerminalDevice->SerialIo, 
                                    &Length, 
                                    (UINT8*)&Utf8Char
                                    );
        if (EFI_ERROR(Status)) {
          return EFI_DEVICE_ERROR;
        }
        break;
    }    
    
    //
    //  Update cursor position.
    //
    switch (*WString) {
      
      case CHAR_BACKSPACE : 
        if (Mode->CursorColumn > 0) {
          Mode->CursorColumn--;
        }
        break;
        
      case CHAR_LINEFEED :
        if (Mode->CursorRow < (INT32)(MaxRow-1)) {
          Mode->CursorRow++;
        }
        break;
        
      case CHAR_CARRIAGE_RETURN:
        Mode->CursorColumn = 0;
        break;
        
      default:
        if (Mode->CursorColumn < (INT32)(MaxColumn-1)) {
          
          Mode->CursorColumn++;
          
        } else {
          
          Mode->CursorColumn = 0;
          if (Mode->CursorRow < (INT32)(MaxRow-1)) {
            Mode->CursorRow++;
          }
          
        }
        break;
        
    };
    
  }
  

  if (Warning) {
    return EFI_WARN_UNKNOWN_GLYPH;
  }

  return EFI_SUCCESS;
}


EFI_STATUS 
EFIAPI
TerminalConOutTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT_PROTOCOL.TestString().
    If one of the characters in the *Wstring is
    neither valid Unicode drawing characters,
    not ASCII code, then this function will return
    EFI_UNSUPPORTED.
        
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    CHAR16                        IN    *WString
        The Null-terminated Unicode string to be tested.
        
  Returns:
  
    EFI_SUCCESS
       The terminal is capable of rendering the output string. 
    
    EFI_UNSUPPORTED
      Some of the characters in the Unicode string cannot be rendered.      
                
--*/      
{
  TERMINAL_DEV                  *TerminalDevice;
  EFI_STATUS                    Status;  
  
  //
  //  get Terminal device data structure pointer.
  //
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(This);
        
  switch (TerminalDevice->TerminalType) {
    
    case PcAnsiType:
    case VT100Type:
    case VT100PlusType:
      Status = AnsiTestString (TerminalDevice,WString);
      break;
      
    case VTUTF8Type:
      Status = VTUTF8TestString (TerminalDevice,WString);
      break;
      
    default:
      Status = EFI_UNSUPPORTED;
      break;
  }
  
  return Status;
}

EFI_STATUS 
EFIAPI
TerminalConOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber,
  OUT UINTN                         *Columns,
  OUT UINTN                         *Rows
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT_PROTOCOL.QueryMode().
    It returns information for an available text mode
    that the terminal supports.
    In this driver, we only support text mode 80x25, which is
    defined as mode 0.
        
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    UINTN                         IN    ModeNumber
        The mode number to return information on.
        
    UINTN                         OUT   *Columns
        The returned columns of the requested mode.
        
    UINTN                         OUT   *Rows
        The returned rows of the requested mode.                
        
  Returns:
  
    EFI_SUCCESS
      The requested mode information is returned. 
    
    EFI_UNSUPPORTED
      The mode number is not valid.   
                
--*/      
{ 
  if (ModeNumber == 0) {
    
    *Columns = MODE0_COLUMN_COUNT;
    *Rows = MODE0_ROW_COUNT;   
    
    return EFI_SUCCESS;
  } 
  
  return EFI_UNSUPPORTED;
}

EFI_STATUS 
EFIAPI
TerminalConOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT.SetMode().
    Set the terminal to a specified display mode.
    In this driver, we only support mode 0.        
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    UINTN                         IN    ModeNumber
        The text mode to set.
        
  Returns:
  
    EFI_SUCCESS
       The requested text mode is set.
       
    EFI_DEVICE_ERROR
      The requested text mode cannot be set because of serial device error.
    
    EFI_UNSUPPORTED
      The text mode number is not valid.       
                
--*/      
{ 
  EFI_STATUS    Status;  
  TERMINAL_DEV  *TerminalDevice;
  
  //
  //  get Terminal device data structure pointer.
  //
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(This);
  
  if (ModeNumber != 0) {
    return EFI_UNSUPPORTED;
  }
  
  This->Mode->Mode = 0;
    
  This->ClearScreen(This);
  
  TerminalDevice->OutputEscChar = TRUE;
  Status = This->OutputString(This, mSetModeString);
  TerminalDevice->OutputEscChar = FALSE;
  
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
    
  This->Mode->Mode = 0;

  Status = This->ClearScreen(This);
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
    
  return EFI_SUCCESS;
  
}

EFI_STATUS 
EFIAPI
TerminalConOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Attribute
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT_PROTOCOL.SetAttribute().       
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    UINTN                         IN    Attribute
        The attribute to set. Only bit0..6 are valid, all other bits
        are undefined and must be zero.
        
  Returns:
  
    EFI_SUCCESS
      The requested attribute is set. 
       
    EFI_DEVICE_ERROR
      The requested attribute cannot be set due to serial port error.
          
    EFI_UNSUPPORTED
      The attribute requested is not defined by EFI spec.   
                
--*/      
{
  UINT8           ForegroundControl;
  UINT8           BackgroundControl;
  UINT8           BrightControl;
  INT32           SavedColumn;
  INT32           SavedRow;
  EFI_STATUS      Status;
  TERMINAL_DEV    *TerminalDevice;
  
  SavedColumn = 0;
  SavedRow = 0;
  
  //
  //  get Terminal device data structure pointer.
  //
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(This);  

  //
  //  only the bit0..6 of the Attribute is valid
  //  
  if ((Attribute | 0x7f) != 0x7f) {
    return EFI_UNSUPPORTED;
  }
  
  //
  //  convert Attribute value to terminal emulator
  //  understandable foreground color
  //
  switch (Attribute & 0x07) {
      
    case EFI_BLACK      : ForegroundControl = 30; break;
    case EFI_BLUE       : ForegroundControl = 34; break;
    case EFI_GREEN      : ForegroundControl = 32; break;
    case EFI_CYAN       : ForegroundControl = 36; break;
    case EFI_RED        : ForegroundControl = 31; break;
    case EFI_MAGENTA    : ForegroundControl = 35; break;
    case EFI_BROWN      : ForegroundControl = 33; break;
    default:
    case EFI_LIGHTGRAY  : ForegroundControl = 37; break;

  }
  
  //
  //  bit4 of the Attribute indicates bright control
  //  of terminal emulator.
  //
  BrightControl = (UINT8)((Attribute>>3) & 1);

  //
  //  convert Attribute value to terminal emulator
  //  understandable background color.
  //
  switch ((Attribute>>4) & 0x07) {
      
    case EFI_BLACK     : BackgroundControl = 40; break;
    case EFI_BLUE      : BackgroundControl = 44; break;
    case EFI_GREEN     : BackgroundControl = 42; break;
    case EFI_CYAN      : BackgroundControl = 46; break;
    case EFI_RED       : BackgroundControl = 41; break;
    case EFI_MAGENTA   : BackgroundControl = 45; break;
    case EFI_BROWN     : BackgroundControl = 43; break;
    default:
    case EFI_LIGHTGRAY : BackgroundControl = 47; break;
  }
  
  //
  // terminal emulator's control sequence to set attributes
  //
  mSetAttributeString[BRIGHT_CONTROL_OFFSET]         = (CHAR16)('0' + BrightControl);
  mSetAttributeString[FOREGROUND_CONTROL_OFFSET + 0] = (CHAR16)('0' + (ForegroundControl / 10));
  mSetAttributeString[FOREGROUND_CONTROL_OFFSET + 1] = (CHAR16)('0' + (ForegroundControl % 10));
  mSetAttributeString[BACKGROUND_CONTROL_OFFSET + 0] = (CHAR16)('0' + (BackgroundControl / 10));
  mSetAttributeString[BACKGROUND_CONTROL_OFFSET + 1] = (CHAR16)('0' + (BackgroundControl % 10));
 
  // 
  // save current column and row 
  // for future scrolling back use.
  //
  SavedColumn = This->Mode->CursorColumn;
  SavedRow  = This->Mode->CursorRow;
  
  TerminalDevice->OutputEscChar = TRUE;
  Status = This->OutputString(This, mSetAttributeString);
  TerminalDevice->OutputEscChar = FALSE;
  
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  } 
  
  //
  //  scroll back to saved cursor position.
  //
  This->Mode->CursorColumn = SavedColumn;
  This->Mode->CursorRow  = SavedRow;

  This->Mode->Attribute = (INT32) Attribute; 

  return EFI_SUCCESS;
  
}

EFI_STATUS 
EFIAPI
TerminalConOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT_PROTOCOL.ClearScreen().
    It clears the ANSI terminal's display to the 
    currently selected background color.
        
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.

  Returns:
  
    EFI_SUCCESS
      The operation completed successfully.
       
    EFI_DEVICE_ERROR
      The terminal screen cannot be cleared due to serial port error.        
    
    EFI_UNSUPPORTED
      The terminal is not in a valid display mode.       
                
--*/      
{
  EFI_STATUS      Status;
  TERMINAL_DEV    *TerminalDevice;
  
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(This);
 
  //
  //  control sequence for clear screen request
  //
  TerminalDevice->OutputEscChar = TRUE;
  Status = This->OutputString(This, mClearScreenString);
  TerminalDevice->OutputEscChar = FALSE;
  
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }

  Status = This->SetCursorPosition(This, 0, 0);
  
  return Status ;
}

EFI_STATUS 
EFIAPI
TerminalConOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  )
/*++
  Routine Description:
  
    Implements EFI_SIMPLE_TEXT_OUT_PROTOCOL.SetCursorPosition().          
  
  Arguments:
  
    EFI_SIMPLE_TEXT_OUT_PROTOCOL  IN    *This
        Indicates the calling context.
        
    UINTN                         IN    Column
        The row to set cursor to.
        
    UINTN                         IN    Row
        The column to set cursor to.                

  Returns:
  
    EFI_SUCCESS
      The operation completed successfully.
       
    EFI_DEVICE_ERROR
      The request fails due to serial port error.        
    
    EFI_UNSUPPORTED
      The terminal is not in a valid text mode, or the cursor position
      is invalid for current mode.     
                
--*/        
{
  EFI_SIMPLE_TEXT_OUTPUT_MODE   *Mode;
  UINTN                         MaxColumn;
  UINTN                         MaxRow;
  EFI_STATUS                    Status;
  TERMINAL_DEV                  *TerminalDevice;
  
  TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(This);  

  //
  //  get current mode
  //
  Mode = This->Mode;
  
  //
  //  get geometry of current mode
  //
  Status = This->QueryMode(
        This, 
        Mode->Mode, 
        &MaxColumn, 
        &MaxRow
        );
  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;          
  }

  if (Column >= MaxColumn || Row >= MaxRow) {
    return EFI_UNSUPPORTED;
  }

  //
  // control sequence to move the cursor
  //

  mSetCursorPositionString[ROW_OFFSET + 0]    = (CHAR16)('0' + ((Row + 1) / 10));
  mSetCursorPositionString[ROW_OFFSET + 1]    = (CHAR16)('0' + ((Row + 1) % 10));
  mSetCursorPositionString[COLUMN_OFFSET + 0] = (CHAR16)('0' + ((Column + 1) / 10));
  mSetCursorPositionString[COLUMN_OFFSET + 1] = (CHAR16)('0' + ((Column + 1) % 10));
  
  TerminalDevice->OutputEscChar = TRUE;
  Status = This->OutputString(This, mSetCursorPositionString);
  TerminalDevice->OutputEscChar = FALSE;
  
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  //  update current cursor position
  //  in the Mode data structure.
  //
  Mode->CursorColumn = (INT32) Column;
  Mode->CursorRow    = (INT32) Row;
  
  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
TerminalConOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN             Visible
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.EnableCursor().
    In this driver, the cursor cannot be hidden.        
  
  Arguments:
  
    SERIAL_IO_PROTOCOL  IN    *This
        Indicates the calling context.
        
    BOOLEAN                       IN    Visible
        If TRUE, the cursor is set to be visible,
        If FALSE, the cursor is set to be invisible.        

  Returns:
  
    EFI_SUCCESS
      The request is valid.
       
    EFI_UNSUPPORTED
      The terminal does not support cursor hidden.   
                
--*/        
{
  if (Visible == FALSE) {
    return EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;
}


BOOLEAN
TerminalIsValidTextGraphics (
    IN  CHAR16  Graphic,
    OUT CHAR8   *PcAnsi,    OPTIONAL
    OUT CHAR8   *Ascii      OPTIONAL
    )
/*++

Routine Description:

    Detects if a Unicode char is for Box Drawing text graphics.

Arguments:

    Graphic  - Unicode char to test.

    PcAnsi  - Optional pointer to return PCANSI equivalent of Graphic.

    Ascii   - Optional pointer to return ASCII equivalent of Graphic.

Returns:

    TRUE if Graphic is a supported Unicode Box Drawing character.

--*/
{
  UNICODE_TO_CHAR     *Table;

  if ((((Graphic & 0xff00) != 0x2500) && ((Graphic & 0xff00) != 0x2100))) {
     
    //
    // Unicode drawing code charts are all in the 0x25xx range, 
    //  arrows are 0x21xx
    //
    return FALSE;
  }

  for (Table = UnicodeToPcAnsiOrAscii; Table->Unicode != 0x0000; Table++) {
    if (Graphic == Table->Unicode) {
      if (PcAnsi) {
        *PcAnsi = Table->PcAnsi; 
      }
      if (Ascii) {
        *Ascii = Table->Ascii;
      }
      return TRUE;
    }
  }
  
  return FALSE;
}

BOOLEAN
TerminalIsValidAscii (
    IN  CHAR16  Ascii
    )
{
  //
  // valid ascii code lies in the extent of 0x20 ~ 0x7f
  //
  if ((Ascii >= 0x20) && (Ascii <= 0x7f)) {
    return TRUE;
  }              
  return FALSE;
}

BOOLEAN
TerminalIsValidEfiCntlChar (
    IN  CHAR16  c
    )
{
  //
  // only support four control characters.
  //
  if (c == CHAR_NULL || c == CHAR_BACKSPACE 
      || c == CHAR_LINEFEED || c == CHAR_CARRIAGE_RETURN
      || c == CHAR_TAB) {
    return TRUE;
  }              
  return FALSE;
}
