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

  GraphicsConsole.c
  
Abstract:

  This is the main routine for initializing the Graphics Console support routines.

Revision History

Remaining Tasks
  Add all standard Glyphs from EFI 1.02 Specification
  Implement optimal automatic Mode creation algorithm
  Solve palette issues for mixed graphics and text
  When does this protocol reset the palette?

--*/

#include "GraphicsConsole.h"

//
// Function Prototypes
//
EFI_STATUS
GraphicsConsoleControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
GraphicsConsoleControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
GraphicsConsoleControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );


EFI_STATUS
ConvertUnicodeWeightToGlyph (
  IN  CHAR16        UnicodeWeight,
  OUT NARROW_GLYPH  **Glyph  OPTIONAL
  );

EFI_STATUS
GetTextColors (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  OUT EFI_UGA_PIXEL                 *Foreground,
  OUT EFI_UGA_PIXEL                 *Background
  );

EFI_STATUS 
DrawUnicodeWeightAtCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        UnicodeWeight
  );

EFI_STATUS 
DrawUnicodeWeightAtCursorN (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *UnicodeWeight,
  IN  UINTN                         Count
  );

EFI_STATUS
DrawCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  );

EFI_STATUS
EraseCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  );

//
// Globals
//
static CHAR16 mCrLfString[3] = { CHAR_CARRIAGE_RETURN, CHAR_LINEFEED, CHAR_NULL };

static EFI_UGA_PIXEL mEfiColors[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x98, 0x00, 0x00, 0x00,
  0x00, 0x98, 0x00, 0x00,
  0x98, 0x98, 0x00, 0x00,
  0x00, 0x00, 0x98, 0x00,
  0x98, 0x00, 0x98, 0x00,
  0x00, 0x98, 0x98, 0x00,
  0x98, 0x98, 0x98, 0x00,
  0x10, 0x10, 0x10, 0x00,
  0xff, 0x10, 0x10, 0x00,
  0x10, 0xff, 0x10, 0x00,
  0xff, 0xff, 0x10, 0x00,
  0x10, 0x10, 0xff, 0x00,
  0xf0, 0x10, 0xff, 0x00,
  0x10, 0xff, 0xff, 0x00,
  0xff, 0xff, 0xff, 0x00,
};

static NARROW_GLYPH mCursorGlyph = 
{ 0x0000, 0x00, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF}};

static NARROW_GLYPH mUnknownGlyph = 
{ 0x0000, 0x00, {0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0x00}};

EFI_DRIVER_BINDING_PROTOCOL gGraphicsConsoleDriverBinding = {
  GraphicsConsoleControllerDriverSupported,
  GraphicsConsoleControllerDriverStart,
  GraphicsConsoleControllerDriverStop,
  0x10,
  NULL,
  NULL
};

//
// Body of the driver
//

EFI_DRIVER_ENTRY_POINT (InitializeGraphicsConsole)

EFI_STATUS
InitializeGraphicsConsole (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
  Routine Description:
  
    This function is the entry point of Graphics Console driver. It will install a Graphics Console.
  
  Arguments:
  
    EFI_HANDLE        IN    ImageHandle
         A handle for the image that is initializing this driver.
         
    EFI_SYSTEM_TABLE  IN    *SystemTable
        A pointer to the EFI system table.
        
  Returns:
  
    EFI_SUCCESS
       only returns this status code.

--*/
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle, 
          SystemTable, 
          &gGraphicsConsoleDriverBinding, 
          ImageHandle,
          &gGraphicsConsoleComponentName,
          NULL,
          NULL
          );
}

EFI_STATUS
GraphicsConsoleControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS            Status;
  EFI_UGA_DRAW_PROTOCOL *UgaDraw;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,       
                  &gEfiDevicePathProtocolGuid,  
                  NULL,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,       
                  &gEfiUgaDrawProtocolGuid,  
                  (VOID **)&UgaDraw,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Controller,       
         &gEfiUgaDrawProtocolGuid,  
         This->DriverBindingHandle,   
         Controller   
         );

  return Status;
}


EFI_STATUS
GraphicsConsoleControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS                           Status;
  GRAPHICS_CONSOLE_DEV                 *Private;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL         *Sto;
  UINT32                               HorizontalResolution;
  UINT32                               VerticalResolution;
  UINT32                               ColorDepth;
  UINT32                               RefreshRate;
  UINTN                                MaxMode;
  UINTN                                Columns;
  UINTN                                Rows;

  //
  // Initialize the Graphics Console device instance
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (GRAPHICS_CONSOLE_DEV),
                  (VOID **)&Private
                  );
  if (EFI_ERROR (Status)) {
    Private = NULL;
    goto Error;
  }
  EfiZeroMem(Private, sizeof (GRAPHICS_CONSOLE_DEV));

  Private->Signature = GRAPHICS_CONSOLE_DEV_SIGNATURE;
  Private->Handle    = Controller;

  Status = gBS->OpenProtocol (
                  Private->Handle,
                  &gEfiDevicePathProtocolGuid,  
                  NULL,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Private->Handle,       
                  &gEfiUgaDrawProtocolGuid,  
                  (VOID **)&Private->UgaDraw,
                  This->DriverBindingHandle,   
                  Private->Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }


  //
  // Atempt to set the 800x600, 32 bit color, 60 Hz refresh mode
  //
  HorizontalResolution = 800;
  VerticalResolution   = 600;
  ColorDepth           = 32;
  RefreshRate          = 60;
  Status = Private->UgaDraw->SetMode (
                               Private->UgaDraw, 
                               HorizontalResolution, 
                               VerticalResolution, 
                               ColorDepth, 
                               RefreshRate
                               );
  if (EFI_ERROR (Status)) {
    //
    // If the SetMode() fails, then get the current mode information from 
    // the UGA Draw Protocol
    //
    Status = Private->UgaDraw->GetMode (
                                 Private->UgaDraw, 
                                 &HorizontalResolution, 
                                 &VerticalResolution, 
                                 &ColorDepth, 
                                 &RefreshRate
                                 );
    if (EFI_ERROR (Status)) {
      goto Error;
    }
  }

  GraphicsConsoleDrawLogo (Private->UgaDraw);

  //
  // Compute the maximum number of text Rows and Columns that this current graphics mode can support
  //
  Columns = HorizontalResolution / GLYPH_WIDTH;
  Rows    = VerticalResolution / GLYPH_HEIGHT;

  //
  // See if the mode is too small to support the required 80x25 text mode
  //
  if (Columns < 80 || Rows < 25) {
    goto Error;
  }

  //
  // Add Mode #0 that must be 80x25
  //
  MaxMode=0;
  Private->ModeData[MaxMode].Columns     = 80;
  Private->ModeData[MaxMode].Rows        = 25;
  Private->ModeData[MaxMode].ClearWidth  = HorizontalResolution;
  Private->ModeData[MaxMode].ClearHeight = VerticalResolution;
  Private->ModeData[MaxMode].DeltaX      = (HorizontalResolution - (80 * GLYPH_WIDTH)) >> 1;
  Private->ModeData[MaxMode].DeltaY      = (VerticalResolution - (25 * GLYPH_HEIGHT)) >> 1;
  MaxMode++;

  //
  // If it is possible to support Mode #1 - 80x50, than add it as an active mode
  //
  if (Rows >= 50) {
    Private->ModeData[MaxMode].Columns     = 80;
    Private->ModeData[MaxMode].Rows        = 50;
    Private->ModeData[MaxMode].ClearWidth  = HorizontalResolution;
    Private->ModeData[MaxMode].ClearHeight = VerticalResolution;
    Private->ModeData[MaxMode].DeltaX      = (HorizontalResolution - (80 * GLYPH_WIDTH)) >> 1;
    Private->ModeData[MaxMode].DeltaY      = (VerticalResolution - (50 * GLYPH_HEIGHT)) >> 1;
    MaxMode++;
  }

  //
  // If the graphics mode is 800x600, than add a text mode that uses the entire display
  //
  if (HorizontalResolution == 800 && VerticalResolution == 600) {
    Private->ModeData[MaxMode].Columns     = 800 / GLYPH_WIDTH;
    Private->ModeData[MaxMode].Rows        = 600 / GLYPH_HEIGHT;
    Private->ModeData[MaxMode].ClearWidth  = 800;
    Private->ModeData[MaxMode].ClearHeight = 600;
    Private->ModeData[MaxMode].DeltaX      = (800 % GLYPH_WIDTH) >> 1;
    Private->ModeData[MaxMode].DeltaY      = (600 % GLYPH_HEIGHT) >> 1;
    MaxMode++;
  }

  //
  // Update the maximum number of modes
  //
  Private->SimpleTextOutputMode.MaxMode = (INT32)MaxMode;

  //
  // Simple Text Output Protocol
  //
  Sto = &Private->SimpleTextOutput;
  Sto->Reset             = GraphicsConsoleConOutReset;
  Sto->OutputString      = GraphicsConsoleConOutOutputString;
  Sto->TestString        = GraphicsConsoleConOutTestString;
  Sto->QueryMode         = GraphicsConsoleConOutQueryMode;
  Sto->SetMode           = GraphicsConsoleConOutSetMode;
  Sto->SetAttribute      = GraphicsConsoleConOutSetAttribute;
  Sto->ClearScreen       = GraphicsConsoleConOutClearScreen;
  Sto->SetCursorPosition = GraphicsConsoleConOutSetCursorPosition; 
  Sto->EnableCursor      = GraphicsConsoleConOutEnableCursor;
  Sto->Mode              = &Private->SimpleTextOutputMode;

  Private->SimpleTextOutputMode.Mode          = 0;
  Private->SimpleTextOutputMode.Attribute     = EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK);
  Private->SimpleTextOutputMode.CursorColumn  = 0;
  Private->SimpleTextOutputMode.CursorRow     = 0;
  Private->SimpleTextOutputMode.CursorVisible = TRUE;

  Status = Private->SimpleTextOutput.Reset (&Private->SimpleTextOutput, FALSE);
  if (EFI_ERROR (Status)) {   
    goto Error;
  }

  Status = Private->SimpleTextOutput.OutputString (&Private->SimpleTextOutput, L"Graphics Console Started\n\r");

  //
  // Install protocol interfaces for the Graphics Console device.
  //
  Status = gBS->InstallProtocolInterface (
                  &Private->Handle,            
                  &gEfiSimpleTextOutProtocolGuid,   
                  EFI_NATIVE_INTERFACE,
                  &Private->SimpleTextOutput
                  );

Error:
  if (EFI_ERROR (Status)) {

    //
    // Close the UGA IO Protocol
    //
    gBS->CloseProtocol (
           Private->Handle,       
           &gEfiUgaDrawProtocolGuid, 
           This->DriverBindingHandle, 
           Private->Handle       
           );

    //
    // Free private data
    //
    if (Private != NULL) {
      gBS->FreePool (Private);
    }
  }
  return Status;
}

EFI_STATUS
GraphicsConsoleControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )

/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS                    Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SimpleTextOutput;
  GRAPHICS_CONSOLE_DEV          *Private;

  Status = gBS->OpenProtocol (
                  Controller, 
                  &gEfiSimpleTextOutProtocolGuid, 
                  (VOID **)&SimpleTextOutput, 
                  This->DriverBindingHandle, 
                  Controller, 
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_STARTED;
  }

  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(SimpleTextOutput);

  Status = gBS->UninstallProtocolInterface (
                 Private->Handle,       
                 &gEfiSimpleTextOutProtocolGuid,   
                 &Private->SimpleTextOutput
                 );
  if (!EFI_ERROR (Status)) {
    //
    // Close the UGA IO Protocol
    //
    gBS->CloseProtocol (
           Private->Handle,       
           &gEfiUgaDrawProtocolGuid, 
           This->DriverBindingHandle, 
           Private->Handle       
           );

    //
    // Free our instance data
    //
    if (Private != NULL) {
      gBS->FreePool (Private);
    }
  }
  return Status;
}

//
// Body of the STO functions
//

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       ExtendedVerification
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.Reset().
    If ExtendeVerification is TRUE, then perform dependent Graphics Console 
    device reset, and set display mode to mode 0.
    If ExtendedVerification is FALSE, only set display mode to mode 0.
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    BOOLEAN             IN    ExtendedVerification
        Indicates that the driver may perform a more exhaustive
        verification operation of the device during reset.
        
  Returns:
  
    EFI_SUCCESS
       The reset operation succeeds.   
    
    EFI_DEVICE_ERROR
      The Graphics Console is not functioning correctly 
                
--*/  
{
  //
  //reset the background and keep the foreground unchanged
  //
  This->SetAttribute(This,This->Mode->Attribute & 0xf);
  
  return This->SetMode (This, 0); 
}


EFI_STATUS 
EFIAPI
GraphicsConsoleConOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.OutputString().
    The Unicode string will be converted to Glyphs and will be 
    sent to the Graphics Console.
    
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    CHAR16                        IN    *WString
        The Null-terminated Unicode string to be displayed on 
        the Graphics Console.
        
  Returns:
  
    EFI_SUCCESS
       The string is output successfully.   
    
    EFI_DEVICE_ERROR
      The Graphics Console failed to send the string out.
      
    EFI_WARN_UNKNOWN_GLYPH
      Indicates that some of the characters in the Unicode string could not 
      be rendered and are skipped.          
                
--*/    
{
  GRAPHICS_CONSOLE_DEV                  *Private;
  EFI_UGA_DRAW_PROTOCOL                 *UgaDraw;
  UINTN                                 MaxColumn;
  UINTN                                 MaxRow;
  EFI_STATUS                            Status;
  BOOLEAN                               Warning;
  EFI_UGA_PIXEL                         Foreground;
  EFI_UGA_PIXEL                         Background;
  UINTN                                 DeltaX;
  UINTN                                 DeltaY;
  UINTN                                 Count;
  
  Status = This->QueryMode(
                   This, 
                   This->Mode->Mode, 
                   &MaxColumn, 
                   &MaxRow
                   );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(This);
  UgaDraw = Private->UgaDraw;
  
  DeltaX = Private->ModeData[This->Mode->Mode].DeltaX, 
  DeltaY = Private->ModeData[This->Mode->Mode].DeltaY,  

  EraseCursor (This);

  Warning = FALSE;
  for (;*WString != CHAR_NULL; ) {

    switch (*WString) {

      case CHAR_NULL :
        //
        //Ignore the character, and do not move the cursor.
        //
        WString++;
        break;

      case CHAR_BACKSPACE : 
        //  
        //If the cursor is not at the left edge of the display, then move the cursor
        //left one column.
        //
        if (This->Mode->CursorColumn > 0) {
            This->Mode->CursorColumn--;
        }
        WString++;
        break;
  
      case CHAR_LINEFEED :
        //
        //If the cursor is at the bottom of the display, then scroll the display one
        //row, and do not update the cursor position. Otherwise, move the cursor
        //down one row.
        //
        if (This->Mode->CursorRow == (INT32)(MaxRow-1)) {
          //
          // Scroll Screen Up One Row 
          //
          Status = UgaDraw->Blt (
                              UgaDraw, 
                              NULL,
                              EfiUgaVideoToVideo,
                              DeltaX,
                              DeltaY + GLYPH_HEIGHT,
                              DeltaX, 
                              DeltaY,  
                              MaxColumn * GLYPH_WIDTH, 
                              (MaxRow-1) * GLYPH_HEIGHT, 
                              MaxColumn * GLYPH_WIDTH * sizeof (EFI_UGA_PIXEL)
                              );

          //
          // Print Blank Line at last line
          //
          Status = GetTextColors (This, &Foreground, &Background);
          Status = UgaDraw->Blt (
                              UgaDraw,
                              &Background,
                              EfiUgaVideoFill,
                              0,
                              0,
                              DeltaX, 
                              DeltaY + ((MaxRow - 1) * GLYPH_HEIGHT),
                              MaxColumn * GLYPH_WIDTH, 
                              GLYPH_HEIGHT, 
                              MaxColumn * GLYPH_WIDTH * sizeof (EFI_UGA_PIXEL)
                              );

        }
        
        if (This->Mode->CursorRow < (INT32)(MaxRow-1)) {
            This->Mode->CursorRow++;
        }
        WString++;
        break;
      
      case CHAR_CARRIAGE_RETURN:
        //
        //Move the cursor to the beginning of the current row.
        //
        This->Mode->CursorColumn = 0;
        WString++;
        break;

      default:
        //  
        //Print the character at the current cursor position and move the cursor
        //right one column. If this moves the cursor past the right edge of the
        //display, then the line should wrap to the beginning of the next line. This
        //is equivalent to inserting a CR and an LF. Note that if the cursor is at the
        //bottom of the display, and the line wraps, then the display will be scrolled
        //one line.
        //
        for (Count = 0; (This->Mode->CursorColumn + Count) <= (MaxColumn - 1); Count++) {
          if (WString[Count] == CHAR_NULL) {
            break;
          }
          if (WString[Count] == CHAR_BACKSPACE) {
            break;
          }
          if (WString[Count] == CHAR_LINEFEED) {
            break;
          }
          if (WString[Count] == CHAR_CARRIAGE_RETURN) {
            break;
          }
        }

        Status = DrawUnicodeWeightAtCursorN (This, WString, Count);
        if (EFI_ERROR (Status)) {
          Warning = TRUE;
        }

        //
        // At the end of line, output carriage return and line feed
        //
        This->Mode->CursorColumn += (INT32)Count;
        WString += Count;
        if (This->Mode->CursorColumn > (INT32)(MaxColumn-1)) {
          DrawCursor (This);
          This->OutputString (This, mCrLfString);
          EraseCursor (This);
        } 
        break;
    }
  }
  
  DrawCursor (This);

  if (Warning) {
    return EFI_WARN_UNKNOWN_GLYPH;
  }

  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.TestString().
    If one of the characters in the *Wstring is
    neither valid valid Unicode drawing characters,
    not ASCII code, then this function will return
    EFI_UNSUPPORTED.
        
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    CHAR16                        IN    *WString
        The Null-terminated Unicode string to be tested.
        
  Returns:
  
    EFI_SUCCESS
       The Graphics Console is capable of rendering the output string. 
    
    EFI_UNSUPPORTED
      Some of the characters in the Unicode string cannot be rendered.      
                
--*/      
{  
  EFI_STATUS Status;

  for (;*WString != CHAR_NULL; WString++) {
    Status = ConvertUnicodeWeightToGlyph (*WString, NULL);
    if (EFI_ERROR (Status)) {
      return EFI_UNSUPPORTED;
    }
  }
  return EFI_SUCCESS;
}


EFI_STATUS 
EFIAPI
GraphicsConsoleConOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber,
  OUT UINTN                         *Columns,
  OUT UINTN                         *Rows
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.QueryMode().
    It returnes information for an available text mode
    that the Graphics Console supports.
    In this driver,we only support text mode 80x25, which is
    defined as mode 0.
        
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
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
  GRAPHICS_CONSOLE_DEV  *Private;

  if (ModeNumber >= (UINTN)This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(This);

  *Columns = Private->ModeData[ModeNumber].Columns;
  *Rows    = Private->ModeData[ModeNumber].Rows;

  return EFI_SUCCESS;
}


EFI_STATUS 
EFIAPI
GraphicsConsoleConOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.SetMode().
    Set the Graphics Console to a specified mode.
    In this driver, we only support mode 0.        
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    UINTN                         IN    ModeNumber
        The text mode to set.
        
  Returns:
  
    EFI_SUCCESS
       The requested text mode is set.
       
    EFI_DEVICE_ERROR
      The requested text mode cannot be set because of Graphics Console device error.
    
    EFI_UNSUPPORTED
      The text mode number is not valid.       
                
--*/      
{ 
  EFI_STATUS                Status;
  GRAPHICS_CONSOLE_DEV      *Private;
  
  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(This);

  if (Private->LineBuffer) {
    This->ClearScreen (This);
    This->EnableCursor (This, FALSE);

    gBS->FreePool (Private->LineBuffer);
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(EFI_UGA_PIXEL) * Private->ModeData[ModeNumber].Columns * GLYPH_WIDTH * GLYPH_HEIGHT,
                  (VOID **)&Private->LineBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
    
  This->Mode->Mode = (INT32) ModeNumber;

  Status = This->SetCursorPosition (This, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = This->EnableCursor (This, TRUE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


EFI_STATUS 
EFIAPI
GraphicsConsoleConOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Attribute
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.SetAttribute().       
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.
    
    UINTN                         IN    Attrubute
        The attribute to set. Only bit0..6 are valid, all other bits
        are undefined and must be zero.
        
  Returns:
  
    EFI_SUCCESS
      The requested attribute is set. 
       
    EFI_DEVICE_ERROR
      The requested attribute cannot be set due to Graphics Console port error.
          
    EFI_UNSUPPORTED
      The attribute requested is not defined by EFI spec.   
                
--*/      
{
  if ((Attribute | 0x7f) != 0x7f) {
    return EFI_UNSUPPORTED;
  }

  if ((INT32)Attribute == This->Mode->Attribute) {
    return EFI_SUCCESS;
  }

  EraseCursor(This);

  This->Mode->Attribute = (INT32) Attribute; 

  DrawCursor(This);

  return EFI_SUCCESS;
}


EFI_STATUS 
EFIAPI
GraphicsConsoleConOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.ClearScreen().
    It clears the Graphics Console's display to the 
    currently selected background color.
        
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.

  Returns:
  
    EFI_SUCCESS
      The operation completed successfully.
       
    EFI_DEVICE_ERROR
      The Graphics Console cannot be cleared due to Graphics Console device error.        
    
    EFI_UNSUPPORTED
      The Graphics Console is not in a valid text mode.       
                
--*/      
{
  EFI_STATUS              Status;
  GRAPHICS_CONSOLE_DEV    *Private;
  EFI_UGA_DRAW_PROTOCOL   *UgaDraw;
  UINTN                   MaxColumn;
  UINTN                   MaxRow;
  EFI_UGA_PIXEL           Foreground;
  EFI_UGA_PIXEL           Background;
  UINTN                   DeltaX;
  UINTN                   DeltaY;
   
  Status = This->QueryMode(
                   This, 
                   This->Mode->Mode, 
                   &MaxColumn, 
                   &MaxRow
                   );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(This);
  UgaDraw = Private->UgaDraw;
  DeltaX = Private->ModeData[This->Mode->Mode].DeltaX; 
  DeltaY = Private->ModeData[This->Mode->Mode].DeltaY;  

  Status = GetTextColors (This, &Foreground, &Background);
  Status = UgaDraw->Blt (
                      UgaDraw, 
                      &Background,
                      EfiUgaVideoFill,
                      0,
                      0,
                      0,
                      0,
                      Private->ModeData[This->Mode->Mode].ClearWidth, 
                      Private->ModeData[This->Mode->Mode].ClearHeight, 
                      0
                      );

  This->Mode->CursorColumn = 0;
  This->Mode->CursorRow    = 0;
  
  DrawCursor(This);
  
  return Status ;
}


EFI_STATUS 
EFIAPI
GraphicsConsoleConOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.SetCursorPosition().          
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.
        
    UINTN                         IN    Column
        The row to set cursor to.
        
    UINTN                         IN    Row
        The column to set cursor to.                

  Returns:
  
    EFI_SUCCESS
      The operation completed successfully.
       
    EFI_DEVICE_ERROR
      The request fails due to Graphics Console device error.        
    
    EFI_UNSUPPORTED
      The Graphics Console is not in a valid text mode, or the cursor position
      is invalid for current mode.     
                
--*/        
{
  EFI_STATUS  Status;
  UINTN       MaxColumn;
  UINTN       MaxRow;

  Status = This->QueryMode(
        This, 
        This->Mode->Mode, 
        &MaxColumn, 
        &MaxRow
        );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;          
  }

  if (Column >= MaxColumn || Row >= MaxRow) {
    return EFI_UNSUPPORTED;
  }

  if (((INT32)Column == This->Mode->CursorColumn) && ((INT32)Row == This->Mode->CursorRow)) {
    return EFI_SUCCESS;
  }

  EraseCursor (This);

  This->Mode->CursorColumn = (INT32) Column;
  This->Mode->CursorRow    = (INT32) Row;
  
  DrawCursor (This);

  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
GraphicsConsoleConOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       Visible
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.EnableCursor().
    In this driver, the cursor cannot be hidden.        
  
  Arguments:
  
    SIMPLE_TEXT_OUTPUT_PROTOCOL  IN    *This
        Indicates the calling context.
        
    BOOLEAN                       IN    Visible
        If TRUE, the cursor is set to be visible,
        If FALSE, the cursor is set to be invisible.        

  Returns:
  
    EFI_SUCCESS
      The request is valid.
       
    EFI_UNSUPPORTED
      The Graphics Console does not support a hidden cursor.   
                
--*/        
{
  EraseCursor (This);

  This->Mode->CursorVisible = Visible;

  DrawCursor (This);
  
  return EFI_SUCCESS;
}


EFI_STATUS
GetTextColors (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  OUT EFI_UGA_PIXEL                 *Foreground,
  OUT EFI_UGA_PIXEL                 *Background
  )

{
  *Foreground = mEfiColors[This->Mode->Attribute & 0x0f];
  *Background = mEfiColors[This->Mode->Attribute >> 4];
  return EFI_SUCCESS;
}



EFI_STATUS
ConvertUnicodeWeightToGlyph (
  IN  CHAR16        UnicodeWeight,
  OUT NARROW_GLYPH  **Glyph  OPTIONAL
  )

{
  UINTN  Index;

  //
  // Search Glyph database for the Unicode Character UnicodeWeight
  //
  for (Index = 0; UsStdGlyphData[Index].UnicodeWeight != UnicodeWeight; Index++) {
    if (UsStdGlyphData[Index].UnicodeWeight == 0x0000) {
      *Glyph = &mUnknownGlyph;
      return EFI_NOT_FOUND;
    }
  }
  if (Glyph != NULL) {
    *Glyph = &UsStdGlyphData[Index];
  }
  return EFI_SUCCESS;
}


EFI_STATUS 
DrawUnicodeWeightAtCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        UnicodeWeight
  )

{
  GRAPHICS_CONSOLE_DEV    *Private;
  EFI_STATUS              Status;
  EFI_STATUS              ReturnStatus;
  NARROW_GLYPH            *Glyph;
  INTN                    GlyphX;
  INTN                    GlyphY;
  EFI_UGA_DRAW_PROTOCOL   *UgaDraw;
  EFI_UGA_PIXEL           Foreground;
  EFI_UGA_PIXEL           Background;
  EFI_UGA_PIXEL           BltChar[GLYPH_HEIGHT][GLYPH_WIDTH];
  UINTN                   X;
  UINTN                   Y;

  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS (This);

  ReturnStatus = EFI_SUCCESS;
  Status = ConvertUnicodeWeightToGlyph (UnicodeWeight, &Glyph);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  GetTextColors (This, &Foreground, &Background);

  //
  // Convert Monochrome bitmap of the Glyph to BltBuffer structure
  //
  for (Y = 0; Y < GLYPH_HEIGHT; Y++) {
    for (X = 0; X < GLYPH_WIDTH; X++) {
      if ((Glyph->Glyph[Y] & (1 << X)) != 0) {
        BltChar[Y][GLYPH_WIDTH - X - 1] = Foreground;
      } else {
        BltChar[Y][GLYPH_WIDTH - X - 1] = Background;
      }
    }
  }

  //
  // Blt a character to the screen
  //
  GlyphX = This->Mode->CursorColumn * GLYPH_WIDTH;
  GlyphY = This->Mode->CursorRow    * GLYPH_HEIGHT;
  UgaDraw = Private->UgaDraw;
  Status = UgaDraw->Blt (
                      UgaDraw, 
                      (EFI_UGA_PIXEL *)BltChar,
                      EfiUgaBltBufferToVideo, 
                      0,
                      0,
                      GlyphX + Private->ModeData[This->Mode->Mode].DeltaX, 
                      GlyphY + Private->ModeData[This->Mode->Mode].DeltaY, 
                      GLYPH_WIDTH, 
                      GLYPH_HEIGHT, 
                      GLYPH_WIDTH * sizeof (EFI_UGA_PIXEL)
                      );

  return ReturnStatus;
}

EFI_STATUS 
DrawUnicodeWeightAtCursorN (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *UnicodeWeight,
  IN  UINTN                         Count
  )

{
  GRAPHICS_CONSOLE_DEV    *Private;
  EFI_STATUS              Status;
  EFI_STATUS              ReturnStatus;
  NARROW_GLYPH            *Glyph;
  INTN                    GlyphX;
  INTN                    GlyphY;
  EFI_UGA_DRAW_PROTOCOL   *UgaDraw;
  EFI_UGA_PIXEL           Foreground;
  EFI_UGA_PIXEL           Background;
  UINTN                   X;
  UINTN                   Y;
  UINTN                   Index;

  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS (This);

  ReturnStatus = EFI_SUCCESS;

  GetTextColors (This, &Foreground, &Background);

  for (Index = 0; Index < Count; Index++, UnicodeWeight++) {

    Status = ConvertUnicodeWeightToGlyph (*UnicodeWeight, &Glyph);
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }

    //
    // Convert Monochrome bitmap of the Glyph to BltBuffer structure
    //
    for (Y = 0; Y < GLYPH_HEIGHT; Y++) {
      for (X = 0; X < GLYPH_WIDTH; X++) {
        if ((Glyph->Glyph[Y] & (1 << X)) != 0) {
          Private->LineBuffer[Y * GLYPH_WIDTH * Count + Index * GLYPH_WIDTH + (GLYPH_WIDTH - X - 1)] = Foreground;
        } else {
          Private->LineBuffer[Y * GLYPH_WIDTH * Count + Index * GLYPH_WIDTH + (GLYPH_WIDTH - X - 1)] = Background;
        }
      }
    }
  }

  //
  // Blt a character to the screen
  //
  GlyphX = This->Mode->CursorColumn * GLYPH_WIDTH;
  GlyphY = This->Mode->CursorRow    * GLYPH_HEIGHT;
  UgaDraw = Private->UgaDraw;
  Status = UgaDraw->Blt (
                      UgaDraw, 
                      Private->LineBuffer,
                      EfiUgaBltBufferToVideo, 
                      0,
                      0,
                      GlyphX + Private->ModeData[This->Mode->Mode].DeltaX, 
                      GlyphY + Private->ModeData[This->Mode->Mode].DeltaY, 
                      GLYPH_WIDTH * Count, 
                      GLYPH_HEIGHT, 
                      GLYPH_WIDTH * Count * sizeof (EFI_UGA_PIXEL)
                      );

  return ReturnStatus;
}

EFI_STATUS
DrawCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  )

{
  return EraseCursor (This);
}


EFI_STATUS
EraseCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  )

{
  GRAPHICS_CONSOLE_DEV    *Private;
  EFI_STATUS              Status;
  INTN                    GlyphX;
  INTN                    GlyphY;
  EFI_UGA_DRAW_PROTOCOL   *UgaDraw;
  EFI_UGA_PIXEL_UNION     Foreground;
  EFI_UGA_PIXEL_UNION     Background;
  EFI_UGA_PIXEL_UNION     BltChar[GLYPH_HEIGHT][GLYPH_WIDTH];
  UINTN                   X;
  UINTN                   Y;

  if (!This->Mode->CursorVisible) {
    return EFI_SUCCESS;
  }

  Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS (This);

  //
  // Blt a character to the screen
  //
  GlyphX = (This->Mode->CursorColumn * GLYPH_WIDTH) + Private->ModeData[This->Mode->Mode].DeltaX;
  GlyphY = (This->Mode->CursorRow    * GLYPH_HEIGHT) + Private->ModeData[This->Mode->Mode].DeltaY;
  UgaDraw = Private->UgaDraw;
  Status = UgaDraw->Blt (
                      UgaDraw, 
                      (EFI_UGA_PIXEL *)BltChar,
                      EfiUgaVideoToBltBuffer, 
                      GlyphX, 
                      GlyphY, 
                      0,
                      0,
                      GLYPH_WIDTH, 
                      GLYPH_HEIGHT, 
                      GLYPH_WIDTH * sizeof (EFI_UGA_PIXEL)
                      );

  GetTextColors (This, &Foreground.Pixel, &Background.Pixel);

  //
  // Convert Monochrome bitmap of the Glyph to BltBuffer structure
  //
  for (Y = 0; Y < GLYPH_HEIGHT; Y++) {
    for (X = 0; X < GLYPH_WIDTH; X++) {
      if ((mCursorGlyph.Glyph[Y] & (1 << X)) != 0) {
        BltChar[Y][GLYPH_WIDTH - X - 1].Raw ^= Foreground.Raw;
      }
    }
  }

  Status = UgaDraw->Blt (
                      UgaDraw, 
                      (EFI_UGA_PIXEL *)BltChar,
                      EfiUgaBltBufferToVideo, 
                      0,
                      0,
                      GlyphX, 
                      GlyphY, 
                      GLYPH_WIDTH, 
                      GLYPH_HEIGHT, 
                      GLYPH_WIDTH * sizeof (EFI_UGA_PIXEL)
                      );
  return EFI_SUCCESS;
}
