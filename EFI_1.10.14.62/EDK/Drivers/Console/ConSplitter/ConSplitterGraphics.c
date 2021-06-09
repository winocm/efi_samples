/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Abstract:

  Support for ConsoleControl protocol. Support for UGA Draw spliter.
  Support for DevNull Console Out. This console uses memory buffers
  to represnt the console. It allows a console to start very early and
  when a new console is added it is synced up with the current console


--*/

#include "ConSplitter.h"

static CHAR16 mCrLfString[3] = {CHAR_CARRIAGE_RETURN, CHAR_LINEFEED, CHAR_NULL};

EFI_STATUS
EFIAPI
ConSpliterConsoleControlGetMode (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  OUT EFI_SCREEN_MODE_ENUM            *Mode,
  OUT BOOLEAN                         *UgaExists,
  OUT BOOLEAN                         *StdInLocked
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
{
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  
  Private = CONSOLE_CONTROL_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  if (Mode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Mode = Private->UgaMode;

  if (UgaExists != NULL) {
    *UgaExists = FALSE;
    for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
      if (Private->TextOutList[Index].UgaDraw != NULL) {
        *UgaExists = TRUE;
        break;
      }
    }
  }

  if (StdInLocked != NULL) {
    *StdInLocked = ConSpliterConssoleControlStdInLocked ();
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSpliterConsoleControlSetMode (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  IN  EFI_SCREEN_MODE_ENUM            Mode
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
{
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  TEXT_OUT_AND_UGA_DATA               *TextAndUga;
  BOOLEAN                             Supported;

  Private = CONSOLE_CONTROL_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  if (Mode >= EfiScreenTextMaxValue) {
    return EFI_INVALID_PARAMETER;
  }

  Supported  = FALSE;
  TextAndUga = &Private->TextOutList[0];
  for (Index = 0; Index < Private->CurrentNumberOfConsoles;
        Index++, TextAndUga++) {
    if (TextAndUga->UgaDraw != NULL) {
      Supported = TRUE;
      break;
    }
  }
  if ( ( Supported == FALSE ) && ( Mode == EfiScreenGraphics ) ) {
    return EFI_UNSUPPORTED;
  }

  Private->UgaMode = Mode;

  TextAndUga = &Private->TextOutList[0];
  for (Index = 0; Index < Private->CurrentNumberOfConsoles;
        Index++, TextAndUga++) {

    TextAndUga->TextOutEnabled  = TRUE;
    //
    // If we are going into Graphics mode disable ConOut to any UGA device
    //
    if ( ( Mode == EfiScreenGraphics ) && ( TextAndUga->UgaDraw != NULL ) ) { 
      TextAndUga->TextOutEnabled = FALSE;
      DevNullUgaSync (Private, TextAndUga->UgaDraw);
    }
  }

  if (Mode == EfiScreenText) {
    DevNullSyncUgaStdOut (Private);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSpliterUgaDrawGetMode (
  IN  EFI_UGA_DRAW_PROTOCOL           *This,
  OUT UINT32                          *HorizontalResolution,
  OUT UINT32                          *VerticalResolution,
  OUT UINT32                          *ColorDepth,
  OUT UINT32                          *RefreshRate
  )
/*++

  Routine Description:
    Return the current video mode information.

  Arguments:
    This                  - Protocol instance pointer.
    HorizontalResolution  - Current video horizontal resolution in pixels
    VerticalResolution    - Current video vertical resolution in pixels
    ColorDepth            - Current video color depth in bits per pixel
    RefreshRate           - Current video refresh rate in Hz.

  Returns:
    EFI_SUCCES      - Mode information returned.
    EFI_NOT_STARTED - Video display is not initialized. Call SetMode () 
    EFI_INVALID_PARAMETER - One of the input args was NULL.

--*/
{
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  
  if ( !(HorizontalResolution && VerticalResolution 
      && RefreshRate && ColorDepth) ) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // retrieve private data
  //
  Private = UGA_DRAW_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  *HorizontalResolution = Private->UgaHorizontalResolution;
  *VerticalResolution   = Private->UgaVerticalResolution;
  *ColorDepth           = Private->UgaColorDepth;
  *RefreshRate          = Private->UgaRefreshRate;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSpliterUgaDrawSetMode (
  IN  EFI_UGA_DRAW_PROTOCOL           *This,
  IN UINT32                           HorizontalResolution,
  IN UINT32                           VerticalResolution,
  IN UINT32                           ColorDepth,
  IN UINT32                           RefreshRate
  )
/*++

  Routine Description:
    Return the current video mode information.

  Arguments:
    This                  - Protocol instance pointer.
    HorizontalResolution  - Current video horizontal resolution in pixels
    VerticalResolution    - Current video vertical resolution in pixels
    ColorDepth            - Current video color depth in bits per pixel
    RefreshRate           - Current video refresh rate in Hz.

  Returns:
    EFI_SUCCES      - Mode information returned.
    EFI_NOT_STARTED - Video display is not initialized. Call SetMode () 

--*/
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;
  UINTN                               Size;

  Private = UGA_DRAW_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // UgaDevNullSetMode ()
  //
  ReturnStatus = EFI_SUCCESS;
  if ((HorizontalResolution != Private->UgaHorizontalResolution) && 
      (VerticalResolution != Private->UgaVerticalResolution)) {
    //
    // Free the old version
    //
    gBS->FreePool (Private->UgaBlt);

    //
    // Allocate the virtual Blt buffer
    //
    Size = HorizontalResolution * VerticalResolution * sizeof (EFI_UGA_PIXEL);
    Private->UgaBlt = EfiLibAllocateZeroPool (Size);
    if ( Private->UgaBlt == NULL ) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  // 
  // Update the Mode data
  //
  Private->UgaHorizontalResolution  = HorizontalResolution;
  Private->UgaVerticalResolution    = VerticalResolution;
  Private->UgaColorDepth            = ColorDepth;
  Private->UgaRefreshRate           = RefreshRate;

  if (Private->UgaMode != EfiScreenGraphics) {
    return ReturnStatus;
  }

  //
  // return the worst status met
  //
  for ( Index = 0; Index < Private->CurrentNumberOfConsoles; Index++ ) {
    if ( Private->TextOutList[Index].UgaDraw ) {
      Status = Private->TextOutList[Index].UgaDraw->SetMode (
                                      Private->TextOutList[Index].UgaDraw,
                                      HorizontalResolution,
                                      VerticalResolution,
                                      ColorDepth,
                                      RefreshRate
                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
   
  return ReturnStatus;
}

EFI_STATUS
DevNullUgaBlt (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_UGA_PIXEL                   *BltBuffer,   OPTIONAL
  IN  EFI_UGA_BLT_OPERATION           BltOperation,
  IN  UINTN                           SourceX,
  IN  UINTN                           SourceY,
  IN  UINTN                           DestinationX,
  IN  UINTN                           DestinationY,
  IN  UINTN                           Width,
  IN  UINTN                           Height,
  IN  UINTN                           Delta         OPTIONAL
  )
{
  UINTN                               SrcY;
  UINTN                               Index;
  EFI_UGA_PIXEL                       *BltPtr;
  EFI_UGA_PIXEL                       *ScreenPtr;
  UINT32                              HorizontalResolution, VerticalResolution;

  if( ( BltOperation >= EfiUgaBltMax ) || ( BltOperation < 0 ) ) {
    return EFI_INVALID_PARAMETER;
  }


  if ( Width == 0 || Height == 0 ) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DestinationX != 0 || DestinationY != 0) && Delta == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ( Delta == 0 ) {
    Delta = Width * sizeof (EFI_UGA_PIXEL);
  }

  HorizontalResolution  = Private->UgaHorizontalResolution;
  VerticalResolution    = Private->UgaVerticalResolution;

  //
  // We need to fill the Virtual Screen buffer with the blt data.
  //

  if ( BltOperation == EfiUgaVideoToBltBuffer ) {
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if ( ( SourceY + Height ) > VerticalResolution ) {
      return EFI_INVALID_PARAMETER;
    }
    if ( ( SourceX + Width ) > HorizontalResolution ) {
      return EFI_INVALID_PARAMETER;
    }

    BltPtr      = (EFI_UGA_PIXEL *) ((UINT8 *)BltBuffer + DestinationY * Delta + DestinationX * sizeof (EFI_UGA_PIXEL));
    ScreenPtr   = &Private->UgaBlt[SourceY * HorizontalResolution + SourceX];
    while ( Height ) {
      EfiCopyMem (BltPtr, ScreenPtr, Width * sizeof (EFI_UGA_PIXEL) );
      BltPtr    = (EFI_UGA_PIXEL *) ((UINT8 *)BltPtr + Delta);
      ScreenPtr += HorizontalResolution;
      Height --;
    }
  } else {
    //
    // BltBuffer to Video: Source is BltBuffer, destination is Video
    //
    if (DestinationY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }
    if (DestinationX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    ScreenPtr   = &Private->UgaBlt[DestinationY * HorizontalResolution + DestinationX];
    SrcY        = SourceY;
    while ( Height ) {
      if ( BltOperation == EfiUgaVideoFill ) {
        for (Index = 0; Index < Width; Index++) {
          ScreenPtr[Index] = *(EFI_UGA_PIXEL *)((UINT8 *)BltBuffer + SourceY * Delta + SourceX * sizeof (EFI_UGA_PIXEL));
        }
      } else {
        if ( BltOperation == EfiUgaBltBufferToVideo ) {
          BltPtr = (EFI_UGA_PIXEL *) ((UINT8 *)BltBuffer + SrcY * Delta + SourceX * sizeof (EFI_UGA_PIXEL));
        } else  {
          BltPtr = &Private->UgaBlt[SrcY * HorizontalResolution + SourceX];
        }
        EfiCopyMem (ScreenPtr, BltPtr, Width * sizeof (EFI_UGA_PIXEL));
      }
      ScreenPtr += HorizontalResolution;
      SrcY ++;
      Height --;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSpliterUgaDrawBlt (
  IN  EFI_UGA_DRAW_PROTOCOL           *This,
  IN  EFI_UGA_PIXEL                   *BltBuffer,   OPTIONAL
  IN  EFI_UGA_BLT_OPERATION           BltOperation,
  IN  UINTN                           SourceX,
  IN  UINTN                           SourceY,
  IN  UINTN                           DestinationX,
  IN  UINTN                           DestinationY,
  IN  UINTN                           Width,
  IN  UINTN                           Height,
  IN  UINTN                           Delta         OPTIONAL
  )
/*++

  Routine Description:
    The following table defines actions for BltOperations:
    EfiUgaVideoFill - Write data from the  BltBuffer pixel (SourceX, SourceY) 
      directly to every pixel of the video display rectangle 
      (DestinationX, DestinationY) 
      (DestinationX + Width, DestinationY + Height).
      Only one pixel will be used from the BltBuffer. Delta is NOT used.
    EfiUgaVideoToBltBuffer - Read data from the video display rectangle 
      (SourceX, SourceY) (SourceX + Width, SourceY + Height) and place it in 
      the BltBuffer rectangle (DestinationX, DestinationY ) 
      (DestinationX + Width, DestinationY + Height). If DestinationX or 
      DestinationY is not zero then Delta must be set to the length in bytes 
      of a row in the BltBuffer.
    EfiUgaBltBufferToVideo - Write data from the  BltBuffer rectangle 
      (SourceX, SourceY) (SourceX + Width, SourceY + Height) directly to the 
      video display rectangle (DestinationX, DestinationY) 
      (DestinationX + Width, DestinationY + Height). If SourceX or SourceY is 
      not zero then Delta must be set to the length in bytes of a row in the 
      BltBuffer.
    EfiUgaVideoToVideo - Copy from the video display rectangle 
      (SourceX, SourceY) (SourceX + Width, SourceY + Height) .
      to the video display rectangle (DestinationX, DestinationY) 
      (DestinationX + Width, DestinationY + Height). 
     The BltBuffer and Delta  are not used in this mode.

  Arguments:
    This          - Protocol instance pointer.
    BltBuffer     - Buffer containing data to blit into video buffer. This 
                    buffer has a size of Width*Height*sizeof(EFI_UGA_PIXEL)
    BltOperation  - Operation to perform on BlitBuffer and video memory
    SourceX       - X coordinate of source for the BltBuffer.
    SourceY       - Y coordinate of source for the BltBuffer.
    DestinationX  - X coordinate of destination for the BltBuffer.
    DestinationY  - Y coordinate of destination for the BltBuffer.
    Width         - Width of rectangle in BltBuffer in pixels.
    Height        - Hight of rectangle in BltBuffer in pixels.
    Delta         -
  
  Returns:
    EFI_SUCCES            - The Blt operation completed.
    EFI_INVALID_PARAMETER - BltOperation is not valid.
    EFI_DEVICE_ERROR      - A hardware error occured writting to the video 
                             buffer.

--*/
{
  EFI_STATUS                          Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private;
  UINTN                               Index;
  EFI_STATUS                          ReturnStatus;

  Private = UGA_DRAW_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  if( ( BltOperation >= EfiUgaBltMax ) || ( BltOperation < 0 ) ) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Sync up DevNull UGA device
  //  
  ReturnStatus = DevNullUgaBlt (
                    Private, 
                    BltBuffer,
                    BltOperation,
                    SourceX,
                    SourceY,
                    DestinationX,
                    DestinationY,
                    Width,
                    Height,
                    Delta 
                    );
  if (Private->UgaMode != EfiScreenGraphics) {
    return ReturnStatus;
  }
  
  //
  // return the worst status met
  //
  for ( Index = 0; Index < Private->CurrentNumberOfConsoles; Index++ ) {
    if ( Private->TextOutList[Index].UgaDraw ) {
      Status = Private->TextOutList[Index].UgaDraw->Blt (
                                      Private->TextOutList[Index].UgaDraw,
                                      BltBuffer,
                                      BltOperation,
                                      SourceX,
                                      SourceY,
                                      DestinationX,
                                      DestinationY,
                                      Width,
                                      Height,
                                      Delta 
                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      } else if (BltOperation == EfiUgaVideoToBltBuffer) {
        //
        // Only need to read the data into buffer one time
        //
        return EFI_SUCCESS;
      }
    }
  }
   
  return ReturnStatus;
}

EFI_STATUS
DevNullUgaSync (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_UGA_DRAW_PROTOCOL           *UgaDraw
  )
{
  return UgaDraw->Blt (
           UgaDraw, 
           Private->UgaBlt,
           EfiUgaBltBufferToVideo,
           0,
           0,
           0,
           0,
           Private->UgaHorizontalResolution,
           Private->UgaVerticalResolution,
           Private->UgaHorizontalResolution * sizeof (EFI_UGA_PIXEL) 
           );
}

EFI_STATUS
DevNullTextOutOutputString (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  CHAR16                          *WString
  )
/*++

  Routine Description:
    Write a Unicode string to the output device.

  Arguments:
    This    - Protocol instance pointer.
    String  - The NULL-terminated Unicode string to be displayed on the output
              device(s). All output devices must also support the Unicode 
              drawing defined in this file.

  Returns:
    EFI_SUCCES        - The string was output to the device.
    EFI_DEVICE_ERROR  - The device reported an error while attempting to output
                         the text.
    EFI_UNSUPPORTED        - The output device's mode is not currently in a 
                              defined text mode.
    EFI_WARN_UNKNOWN_GLYPH - This warning code indicates that some of the 
                              characters in the Unicode string could not be 
                              rendered and were skipped.

--*/
{
  UINTN                               SizeScreen;
  UINTN                               SizeAttribute;
  UINTN                               Index;
  EFI_SIMPLE_TEXT_OUTPUT_MODE         *Mode;
  CHAR16                              *Screen, *NullScreen;
  INT32                               *Attribute, *NullAttributes;
  UINTN                               LastRow, MaxColumn;

  Mode            = &Private->TextOutMode;
  NullScreen      = Private->DevNullScreen;
  NullAttributes  = Private->DevNullAttributes;
  LastRow         = Private->DevNullRows - 1;
  MaxColumn       = Private->DevNullColumns;
  
  while ( *WString ) {

    if ( *WString == CHAR_BACKSPACE ) {

      //  
      // If the cursor is not at the left edge of the display, 
      // then move the cursor left one column.
      //
      if ( Mode->CursorColumn > 0 ) {
        Mode->CursorColumn --;
      }
      WString++;

    } else if ( *WString == CHAR_LINEFEED ) {
      //
      // If the cursor is at the bottom of the display, 
      // then scroll the display one row, and do not update 
      // the cursor position. Otherwise, move the cursor down one row.
      //
      if ( Mode->CursorRow == (INT32) (LastRow) ) {
        //
        // Scroll Screen Up One Row 
        //
        SizeAttribute = LastRow * MaxColumn;
        EfiCopyMem (
          NullAttributes,
          NullAttributes + MaxColumn,
          SizeAttribute * sizeof (INT32)
          );

        //
        // Each row has an ending CHAR_NULL. So one more character each line
        // for DevNullScreen than DevNullAttributes
        //
        SizeScreen = SizeAttribute + LastRow;
        EfiCopyMem (
          NullScreen,
          NullScreen + (MaxColumn + 1),
          SizeScreen * sizeof (CHAR16)
          );

        //
        // Print Blank Line at last line
        //
        Screen = NullScreen + SizeScreen;
        Attribute = NullAttributes + SizeAttribute;

        for ( Index = 0; Index < MaxColumn; Index++, Screen++, Attribute++ ) {
          *Screen = ' ';
          *Attribute = Mode->Attribute;
        }
      } else {
        Mode->CursorRow ++;
      }
      WString++;
    } else if ( *WString == CHAR_CARRIAGE_RETURN ) {
      //
      // Move the cursor to the beginning of the current row.
      //
      Mode->CursorColumn = 0;
      WString++;
    } else {
      //  
      // Print the character at the current cursor position and 
      // move the cursor right one column. If this moves the cursor 
      // past the right edge of the display, then the line should wrap to 
      // the beginning of the next line. This is equivalent to inserting 
      // a CR and an LF. Note that if the cursor is at the bottom of the 
      // display, and the line wraps, then the display will be scrolled
      // one line.
      //
      Index = Mode->CursorRow * MaxColumn + Mode->CursorColumn;

      while ( Mode->CursorColumn < (INT32)MaxColumn ) {
        if ( *WString == CHAR_NULL ) {
          break;
        }
        if ( *WString == CHAR_BACKSPACE ) {
          break;
        }
        if ( *WString == CHAR_LINEFEED ) {
          break;
        }
        if ( *WString == CHAR_CARRIAGE_RETURN ) {
          break;
        }

        NullScreen[Index + Mode->CursorRow] = *WString;
        NullAttributes[Index]               = Mode->Attribute;
        Index ++;
        WString ++;
        Mode->CursorColumn ++;
      }

      //
      // At the end of line, output carriage return and line feed
      //
      if ( Mode->CursorColumn >= (INT32) (MaxColumn) ) {
        DevNullTextOutOutputString (Private, mCrLfString);
      } 
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DevNullTextOutSetMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  UINTN                           ModeNumber
  )
/*++

  Routine Description:
    Sets the output device(s) to a specified mode.

  Arguments:
    This       - Protocol instance pointer.
    ModeNumber - The mode number to set.

  Returns:
    EFI_SUCCES       - The requested text mode was set.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The mode number was not valid.

--*/
{
  UINTN                           Size;
  UINTN                           Row, Column;
  TEXT_OUT_SPLITTER_QUERY_DATA    *Mode;

  //
  // No extra check for ModeNumber here, as it has been checked in
  // ConSplitterTextOutSetMode. And mode 0 should always be supported.
  //

  Mode    = & (Private->TextOutQueryData[ModeNumber]);
  Row     = Mode->Rows;
  Column  = Mode->Columns;

  if (Private->DevNullColumns != Column ||
      Private->DevNullRows    != Row     ) {

    Private->TextOutMode.Mode = (INT32)ModeNumber;
    Private->DevNullColumns   = Column;
    Private->DevNullRows      = Row;

    gBS->FreePool (Private->DevNullScreen);

    Size = (Row * (Column + 1)) * sizeof (CHAR16);
    Private->DevNullScreen = EfiLibAllocateZeroPool (Size);
    if ( Private->DevNullScreen == NULL ) {
      return EFI_OUT_OF_RESOURCES;
    }

    gBS->FreePool (Private->DevNullAttributes);

    Size = Row * Column * sizeof (INT32);
    Private->DevNullAttributes = EfiLibAllocateZeroPool (Size);
    if ( Private->DevNullAttributes == NULL ) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  DevNullTextOutClearScreen (Private);

  return EFI_SUCCESS;
}

EFI_STATUS  
DevNullTextOutClearScreen (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
/*++

  Routine Description:
    Clears the output device(s) display to the currently selected background 
    color.

  Arguments:
    This      - Protocol instance pointer.

  Returns:
    EFI_SUCCES       - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode.

--*/  
{  
  UINTN                               Row;
  UINTN                               Column;
  CHAR16                              *Screen;
  INT32                               *Attributes;
  INT32                               CurrentAttribute;

  //
  // Clear the DevNull Text Out Buffers.
  // The screen is filled with spaces.
  // The attributes are all synced with the current Simple Text Out Attribute
  //
  Screen = Private->DevNullScreen;
  Attributes = Private->DevNullAttributes;
  CurrentAttribute = Private->TextOutMode.Attribute;

  for (Row = 0; Row < Private->DevNullRows; Row++) {
    for (Column = 0; Column < Private->DevNullColumns; 
          Column++, Screen++, Attributes++) {
      *Screen = ' ';
      *Attributes = CurrentAttribute;
    }
    //
    // Each line of the screen has a NULL on the end so we must skip over it
    //
    Screen++;
  }

  DevNullTextOutSetCursorPosition (Private, 0, 0);

  return DevNullTextOutEnableCursor (Private, TRUE);
}

EFI_STATUS  
DevNullTextOutSetCursorPosition (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  )
/*++

  Routine Description:
    Sets the current coordinates of the cursor position

  Arguments:
    This        - Protocol instance pointer.
    Column, Row - the position to set the cursor to. Must be greater than or
                  equal to zero and less than the number of columns and rows
                  by QueryMode ().

  Returns:
    EFI_SUCCES       - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode, or the 
                       cursor position is invalid for the current mode.

--*/  
{
  //
  // No need to do extra check here as whether (Column, Row) is valid has 
  // been checked in ConSplitterTextOutSetCursorPosition. And (0, 0) should
  // always be supported.
  //

  Private->TextOutMode.CursorColumn = (INT32) Column;
  Private->TextOutMode.CursorRow    = (INT32) Row;

  return EFI_SUCCESS;
}

EFI_STATUS 
DevNullTextOutEnableCursor (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  BOOLEAN                         Visible
  )
/*++
  Routine Description:
  
    Implements SIMPLE_TEXT_OUTPUT.EnableCursor().
    In this driver, the cursor cannot be hidden.        
  
  Arguments:
  
    This - Indicates the calling context.
        
    Visible - If TRUE, the cursor is set to be visible, If FALSE, the cursor 
              is set to be invisible.        

  Returns:
  
    EFI_SUCCESS - The request is valid.
       
               
--*/        
{
  Private->TextOutMode.CursorVisible = Visible;
  
  return EFI_SUCCESS;
}

EFI_STATUS
DevNullSyncUgaStdOut (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
/*++
  Routine Description:
    Take the DevNull TextOut device and update the Simple Text Out on every
    UGA device. 
 
  Arguments:
    Private - Indicates the calling context.

  Returns:
    EFI_SUCCESS - The request is valid.
    other       - Return status of TextOut->OutputString ()
               
--*/        
{
  EFI_STATUS                          Status;
  EFI_STATUS                          ReturnStatus;
  UINTN                               Row, Column, List;
  UINTN                               MaxColumn;
  UINTN                               CurrentColumn;
  UINTN                               StartRow, StartColumn;
  INT32                               StartAttribute;
  BOOLEAN                             StartCursorState;
  CHAR16                              *Screen;
  CHAR16                              *Str;
  CHAR16                              *ScreenStart;
  CHAR16                              RememberTempNull;
  INT32                               CurrentAttribute;
  INT32                               *Attributes;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL        *Sto;

  //
  // Save the devices Attributes, Cursor enable state and location
  //
  StartColumn       = Private->TextOutMode.CursorColumn;
  StartRow          = Private->TextOutMode.CursorRow;
  StartAttribute    = Private->TextOutMode.Attribute;
  StartCursorState  = Private->TextOutMode.CursorVisible;

  for ( List = 0; List < Private->CurrentNumberOfConsoles; List++ ) {

    Sto = Private->TextOutList[List].TextOut;

    //
    // Skip non UGA devices
    //
    if ( Private->TextOutList[List].UgaDraw ) {
      Sto->EnableCursor (Sto, FALSE);
      Sto->ClearScreen (Sto);
    }
  }

  ReturnStatus  = EFI_SUCCESS;
  Screen        = Private->DevNullScreen;
  Attributes    = Private->DevNullAttributes;
  MaxColumn     = Private->DevNullColumns;
  for ( Row = 0; Row < Private->DevNullRows; 
        Row++, Screen += (MaxColumn + 1), Attributes += MaxColumn ) {
          
    if (Row == (Private->DevNullRows - 1)) {
      //
      // Don't ever sync the last character as it will scroll the screen
      //
      Screen[MaxColumn - 1] = 0x00;
    }
    Column      = 0;
    while ( Column < MaxColumn ) {
      if ( Screen[Column] ) {
        CurrentAttribute  = Attributes[Column];
        CurrentColumn     = Column;
        ScreenStart       = &Screen[Column];

        //
        // the line end is alway 0x0. So Column should be less than MaxColumn
        // It should be still in the same row
        //
        for ( Str = ScreenStart, RememberTempNull = 0; 
              *Str != 0; Str++, Column++ ) {

          if ( Attributes[Column] != CurrentAttribute ) {
            RememberTempNull = *Str;
            *Str = 0;
            break;
          }
        }

        //
        // BugBug: One character at a time is 
        //         a bad optimization algorithm!!!!!!!!!!!!!!
        //
        for ( List = 0; List < Private->CurrentNumberOfConsoles; List++ ) {

          Sto = Private->TextOutList[List].TextOut;

          //
          // Skip non UGA devices
          //
          if ( Private->TextOutList[List].UgaDraw ) {
            Sto->SetAttribute (Sto, CurrentAttribute);
            Sto->SetCursorPosition (Sto, CurrentColumn, Row);
            Status = Sto->OutputString (Sto, ScreenStart);
            if (EFI_ERROR (Status)) {
              ReturnStatus = Status;
            }
          }
        }
        
        if ( RememberTempNull ) {
          *Str = RememberTempNull;
          //
          // Make sure we process the character with the new attributes
          //
          Column --;
        }
      }
      Column ++;
    }
  }

  //
  // Restore the devices Attributes, Cursor enable state and location
  //
  for ( List = 0; List < Private->CurrentNumberOfConsoles; List++ ) {
    Sto = Private->TextOutList[List].TextOut;

    //
    // Skip non UGA devices
    //
    if ( Private->TextOutList[List].UgaDraw ) {
      Sto->SetAttribute (Sto, StartAttribute);
      Sto->SetCursorPosition (Sto, StartColumn, StartRow);
      Status = Sto->EnableCursor (Sto, StartCursorState);
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  return ReturnStatus;
}
