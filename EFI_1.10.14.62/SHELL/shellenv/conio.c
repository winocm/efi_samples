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

  conio.c
  
Abstract:

  Shell Environment driver

Revision History

--*/

#include "shelle.h"

//
//
//
#define MAX_HISTORY       20

#define MODE_MIN_COLUMN   80
#define MODE_MIN_ROW      25

#define INPUT_LINE_SIGNATURE     EFI_SIGNATURE_32('i','s','i','g')

typedef struct {
  UINTN           Signature;
  EFI_LIST_ENTRY  Link;
  CHAR16          *Buffer;
} INPUT_LINE;

//
// Globals
//
static BOOLEAN          SEnvInsertMode;
static EFI_LIST_ENTRY   SEnvLineHistory;
static UINTN            SEnvNoHistory;

//
//
//
VOID
SEnvConIoInitDosKey (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  InitializeListHead (&SEnvLineHistory);
  SEnvInsertMode = FALSE;
  SEnvNoHistory = 0;
}


EFI_STATUS
SEnvConIoOpen (
  IN struct _EFI_FILE_HANDLE  *File,
  OUT struct _EFI_FILE_HANDLE **NewHandle,
  IN CHAR16                   *FileName,
  IN UINT64                   OpenMode,
  IN UINT64                   Attributes
  )
/*++

Routine Description:

  Functions used to access the console interface via a file handle
  Used if the console is not being redirected to a file

Arguments:

Returns:

--*/
{
  return EFI_NOT_FOUND;
}


EFI_STATUS
SEnvConIoNop (
  IN struct _EFI_FILE_HANDLE  *File
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvConIoGetPosition (
  IN struct _EFI_FILE_HANDLE  *File,
  OUT UINT64                  *Position
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
SEnvConIoSetPosition (
  IN struct _EFI_FILE_HANDLE  *File,
  OUT UINT64                  Position
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
SEnvConIoGetInfo (
  IN struct _EFI_FILE_HANDLE  *File,
  IN EFI_GUID                 *InformationType,
  IN OUT UINTN                *BufferSize,
  OUT VOID                    *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
SEnvConIoSetInfo (
  IN struct _EFI_FILE_HANDLE  *File,
  IN EFI_GUID                 *InformationType,
  IN UINTN                    BufferSize,
  OUT VOID                    *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
SEnvConIoWrite (
  IN struct _EFI_FILE_HANDLE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  Print (L"%.*s", *BufferSize, Buffer);
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvErrIoWrite (
  IN struct _EFI_FILE_HANDLE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  IPrint (ST->StdErr, L"%.*s", *BufferSize, Buffer);
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvErrIoRead (
  IN struct _EFI_FILE_HANDLE  *File,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}


VOID
SEnvPrintHistory(
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_LIST_ENTRY  *Link;
  INPUT_LINE      *Line;
  UINTN           Index;
  UINTN           LineNumber;
  UINTN           StartColumn;
  UINTN           LineLength;
  UINTN           TotalRow;
  UINTN           LineCount;
  CHAR16          InputStr[1];

  Print (L"\n");
  Index = 0;
  LineNumber = 0;
  StartColumn = 4;
  ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, 
                         &LineLength, &TotalRow);
  
  //
  // Print history
  //
  for (Link=SEnvLineHistory.Flink; Link != &SEnvLineHistory; Link=Link->Flink) {
    Index += 1;
    Line = CR(Link, INPUT_LINE, Link, INPUT_LINE_SIGNATURE);
    LineCount = (StrLen (Line->Buffer) + StartColumn + 1) / LineLength + 1;

    if (LineNumber + LineCount >= TotalRow) {
      Print (L"%NPress ENTER to continue:%E");
      Input (NULL, InputStr, 1);
      Print (L"\n");
      LineNumber = 0;
    }
    Print (L"%2d. %s\n", Index, Line->Buffer);
    LineNumber += LineCount;
  }
}

VOID
MoveCursorBackward (
  IN     UINTN                   LineLength,
  IN OUT UINTN                   *Column,
  IN OUT UINTN                   *Row
)
/*++

Routine Description:
  Move the cursor position one character backward.

Arguments:
  LineLength       Length of a line. Get it by calling QueryMode
  Column           Current column of the cursor position
  Row              Current row of the cursor position

Returns:

--*/
{
  //
  // If current column is 0, move to the last column of the previous line,
  // otherwise, just decrement column.
  //
  if (*Column == 0) {
    (*Column) = LineLength - 1;
    if (*Row > 0) {
      (*Row) --;
    }
  } else {
    (*Column) --;
  }
}

VOID
MoveCursorForward (
  IN     UINTN                   LineLength,
  IN     UINTN                   TotalRow,
  IN OUT UINTN                   *Column,
  IN OUT UINTN                   *Row
)
/*++

Routine Description:
  Move the cursor position one character backward.

Arguments:
  LineLength       Length of a line. Get it by calling QueryMode
  TotalRow         Total row of a screen, get by calling QueryMode
  Column           Current column of the cursor position
  Row              Current row of the cursor position

Returns:

--*/
{
  //
  // If current column is at line end, move to the first column of the nest
  // line, otherwise, just increment column.
  //
  (*Column) ++;
  if (*Column >= LineLength) {
    (*Column) = 0;
    if ((*Row) < TotalRow - 1) {
      (*Row) ++;
    }
  }
}

EFI_STATUS
SEnvConIoRead (
  IN struct _EFI_FILE_HANDLE      *File,
  IN OUT UINTN                    *BufferSize,
  IN VOID                         *Buffer
  )
/*++

Routine Description:
  Get a line from the console.

Arguments:
  File             File handle, actually not used in this function
  BufferSize       Size of the buffer
  Buffer           Buffer to hold the line from console

Returns:
  EFI_SUCCESS      The function finished successfully

--*/
{
  CHAR16                          *Str;
  BOOLEAN                         Done;
  UINTN                           Column;
  UINTN                           Row;
  UINTN                           StartColumn;
  UINTN                           Update;
  UINTN                           Delete;
  UINTN                           Len;
  UINTN                           StrPos;
  UINTN                           MaxStr;
  UINTN                           Index;
  UINTN                           LineLength;
  UINTN                           TotalRow;
  UINTN                           SkipLength;
  UINTN                           OutputLength;
  UINTN                           TailRow;
  UINTN                           TailColumn;
  EFI_INPUT_KEY                   Key;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *ConOut;
  EFI_SIMPLE_TEXT_IN_PROTOCOL     *ConIn;
  INPUT_LINE                      *NewLine;
  INPUT_LINE                      *LineCmd;
  EFI_LIST_ENTRY                  *LinePos;
  EFI_LIST_ENTRY                  *NewPos;

  ConOut = ST->ConOut;
  ConIn = ST->ConIn;
  Str = Buffer;
  Len = 0;
  StrPos = 0;
  OutputLength = 0;
  Update = 0;
  Delete = 0;
  LinePos = NewPos = &SEnvLineHistory;

  //
  // If buffer is not large enough to hold a CHAR16, do nothing.
  //
  if (*BufferSize < sizeof(CHAR16)*2) {
    *BufferSize = 0;
    return EFI_SUCCESS;
  }

  //
  // Get the screen setting and the current cursor location
  //
  StartColumn = ConOut->Mode->CursorColumn;
  Column = StartColumn;
  Row = ConOut->Mode->CursorRow;
  ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &LineLength, &TotalRow);

  //
  // Limit the line length to the buffer size or the minimun size of the
  // screen. (The smaller takes effect)
  //
  MaxStr = LineLength * (TotalRow - 1) - StartColumn;
  if (MaxStr > *BufferSize / sizeof(CHAR16)) {
    MaxStr = *BufferSize / sizeof(CHAR16);
  }

  ZeroMem (Str, MaxStr * sizeof(CHAR16));
  Done = FALSE;
  do {
    //
    // Read a key
    //
    WaitForSingleEvent(ConIn->WaitForKey, 0);
    ConIn->ReadKeyStroke(ConIn, &Key);

    switch (Key.UnicodeChar) {
    case CHAR_CARRIAGE_RETURN:
      //
      // All done, print a newline at the end of the string
      //
      TailRow = Row + (Len - StrPos + Column) / LineLength;
      TailColumn = (Len - StrPos + Column) % LineLength;
      PrintAt (TailColumn, TailRow, L"\n");
      Done = TRUE;
      break;

    case CHAR_BACKSPACE:
      if (StrPos) {
        //
        // If not move back beyond string beginning, move all characters behind
        // the current position one character forward
        //
        StrPos -= 1;
        Update = StrPos;
        Delete = 1;
        CopyMem (Str+StrPos, Str+StrPos+1, sizeof(CHAR16) * (Len-StrPos));

        //
        // Adjust the current column and row
        //
        MoveCursorBackward (LineLength, &Column, &Row);
      }
      break;

    default:
      if (Key.UnicodeChar >= ' ') {
        //
        // If we are at the buffer's end, drop the key
        //
        if (Len == MaxStr-1 && (SEnvInsertMode || StrPos == Len)) {
          break;
        }

        //
        // If in insert mode, move all characters behind the current position
        // one character backward to make space for this character. Then store
        // the character.
        //
        if (SEnvInsertMode) {
          for (Index = Len; Index > StrPos; Index -= 1) {
            Str[Index] = Str[Index-1];
          }
        }

        Str[StrPos] = Key.UnicodeChar;
        Update = StrPos;

        StrPos += 1;
        OutputLength = 1;
      }
      break;

    case 0:
      switch (Key.ScanCode) {
      case SCAN_DELETE:
        //
        // Move characters behind current position one character forward
        //
        if (Len) {
          Update = StrPos;
          Delete = 1;
          CopyMem (Str+StrPos, Str+StrPos+1, sizeof(CHAR16) * (Len-StrPos));
        }
        break;

      case SCAN_UP:
        //
        // Prepare to print the previous command
        //
        NewPos = LinePos->Blink;
        if (NewPos == &SEnvLineHistory) {
          NewPos = NewPos->Blink;
        }
        break;

      case SCAN_DOWN:
        //
        // Prepare to print the next command
        //
        NewPos = LinePos->Flink;
        if (NewPos == &SEnvLineHistory) {
          NewPos = NewPos->Flink;
        }
        break;

      case SCAN_LEFT:
        //
        // Adjust current cursor position
        //
        if (StrPos) {
          StrPos -= 1;
          MoveCursorBackward (LineLength, &Column, &Row);
        }
        break;

      case SCAN_RIGHT:
        //
        // Adjust current cursor position
        //
        if (StrPos < Len) {
          StrPos += 1;
          MoveCursorForward (LineLength, TotalRow, &Column, &Row);
        }
        break;

      case SCAN_HOME:
        //
        // Move current cursor position to the beginning of the command line
        //
        Row -= (StrPos + StartColumn) / LineLength;
        Column = StartColumn;
        StrPos = 0;
        break;

      case SCAN_END:
        //
        // Move current cursor position to the end of the command line
        //
        TailRow = Row + (Len - StrPos + Column) / LineLength;
        TailColumn = (Len - StrPos + Column) % LineLength;
        Row = TailRow;
        Column = TailColumn;
        StrPos = Len;
        break;

      case SCAN_ESC:
        //
        // Prepare to clear the current command line
        //
        Str[0] = 0;
        Update = 0;
        Delete = Len;
        Row -= (StrPos + StartColumn) / LineLength;
        Column = StartColumn;
        OutputLength = 0;
        break;

      case SCAN_INSERT:
        //
        // Toggle the SEnvInsertMode flag
        //
        SEnvInsertMode = (BOOLEAN)!SEnvInsertMode;
        break;

      case SCAN_F7:
        //
        // Print command history
        //
        SEnvPrintHistory();
        *Str = 0;
        Done = TRUE;    
        break;
      }       
    }

    if (Done) {
      break;
    }

    //
    // If we have a new position, we are preparing to print a previous or next
    // command. 
    //
    if (NewPos != &SEnvLineHistory) {
      Column = StartColumn;
      Row -= (StrPos + StartColumn) / LineLength;

      LineCmd = CR(NewPos, INPUT_LINE, Link, INPUT_LINE_SIGNATURE);
      LinePos = NewPos;
      NewPos  = &SEnvLineHistory;

      OutputLength = StrLen (LineCmd->Buffer) < MaxStr - 1 ? 
        StrLen (LineCmd->Buffer) : MaxStr - 1;
      CopyMem (Str, LineCmd->Buffer, OutputLength * sizeof(CHAR16));
      Str[OutputLength] = 0;

      StrPos = OutputLength;
      
      //
      // Draw new input string
      //
      Update = 0;                 
      if (Len > OutputLength) {
        //
        // If old string was longer, blank its tail
        //
        Delete = Len - OutputLength;   
      }
    }

    //
    // If we need to update the output do so now
    //
    if (Update != -1) {
      PrintAt (Column, Row, L"%s%.*s", Str + Update, Delete, L"");
      Len = StrLen (Str);

      if (Delete) {
        SetMem(Str+Len, Delete * sizeof(CHAR16), 0x00);
      }

      if (StrPos > Len) {
        StrPos = Len;
      }

      Update = (UINTN)-1;

      //
      // After using print to reflect newly updates, if we're not using
      // BACKSPACE and DELETE, we need to move the cursor position forward,
      // so adjust row and column here.
      //
      if (Key.UnicodeChar != CHAR_BACKSPACE && 
        ! (Key.UnicodeChar == 0 && Key.ScanCode == SCAN_DELETE)) {
        //
        // Calulate row and column of the tail of current string
        //
        TailRow = Row + (Len - StrPos + Column + OutputLength) / LineLength;
        TailColumn = (Len - StrPos + Column + OutputLength) % LineLength;
        
        //
        // If the tail of string reaches screen end, screen rolls up, so if
        // Row does not equal TailRow, Row should be decremented
        //
        // (if we are recalling commands using UPPER and DOWN key, and if the
        // old command is too long to fit the screen, TailColumn must be 79.
        //
        if (TailColumn == 0 && TailRow >= TotalRow && Row != TailRow) {
          Row --;
        }

        //
        // Calculate the cursor position after current operation. If cursor
        // reaches line end, update both row and column, otherwise, only 
        // column will be changed.
        //
        if (Column + OutputLength >= LineLength) {
          SkipLength = OutputLength - (LineLength - Column);
 
          Row += SkipLength / LineLength + 1;
          if (Row > TotalRow - 1) {
            Row = TotalRow - 1;
          }
          Column = SkipLength % LineLength;
        } else {
          Column += OutputLength;
        }
      }
      Delete = 0;
    }

    //
    // Set the cursor position for this key
    //
    ConOut->SetCursorPosition (ConOut, Column, Row);
  } while (!Done);

  //
  // Copy the line to the history buffer
  //
  if (Str[0]) {
    //
    // Allocate and initalize a new key entry for command history
    //
    NewLine = AllocateZeroPool (sizeof(INPUT_LINE));
    if (!NewLine) {
      return EFI_OUT_OF_RESOURCES;
    }

    NewLine->Signature = INPUT_LINE_SIGNATURE;
    NewLine->Buffer = AllocateZeroPool ((Len + 1) * sizeof(CHAR16));
    if (NewLine->Buffer == NULL) {
      FreePool (NewLine);
      return EFI_OUT_OF_RESOURCES;
    }

    StrCpy (NewLine->Buffer, Str);
    InsertTailList (&SEnvLineHistory, &NewLine->Link);
    SEnvNoHistory ++;
  }
  
  //
  // If there's too much in the history buffer free an entry
  //
  if (SEnvNoHistory > MAX_HISTORY) {
    LineCmd = CR (SEnvLineHistory.Flink, INPUT_LINE, Link, 
      INPUT_LINE_SIGNATURE);
    RemoveEntryList (&LineCmd->Link);
    SEnvNoHistory --;
    if (LineCmd->Buffer) {
      FreePool (LineCmd->Buffer);
    }
    FreePool (LineCmd);
  }

  //
  // Return the data to the caller
  //
  *BufferSize = Len * sizeof(CHAR16);
  return EFI_SUCCESS;
}

//
//
//
EFI_STATUS
SEnvReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{ 
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS              Status;         
  ENV_SHELL_REDIR_FILE    *Redir;
  UINTN                   Len;
  UINTN                   Size;
  UINTN                   WriteSize;
  UINTN                   Index;
  UINTN                   Start;
  CHAR8                   Buffer[100];
  CHAR16                  UnicodeBuffer[100];
  BOOLEAN                 InvalidChar;
  
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *TextIn;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;

  TextIn  = NULL;
  TextOut = NULL;
  
  //
  // Get Shell redirection device
  //
  Redir = CR(This, ENV_SHELL_REDIR_FILE, Out, ENV_REDIR_SIGNATURE);
  if (EFI_ERROR(Redir->WriteError)) {
    return(Redir->WriteError);
  }
  Status = EFI_SUCCESS;
  InvalidChar = FALSE;

  if (Redir->Ascii) {
    //
    // Handle for ASCII
    //
    Start = 0;
    Len   = StrLen (String);
    while (Len) {
      Size = Len > sizeof(Buffer) ? sizeof(Buffer) : Len;
      for (Index = 0; Index < Size; Index += 1) {
        if (String[Start+Index] > 0xff) {
          Buffer[Index] = '_';
          InvalidChar = TRUE;
        } else {
          Buffer[Index] = (CHAR8) String[Start+Index];
        }  
      }

      WriteSize = Size;
      if (WriteSize > 0) {
        Status = Redir->File->Write (Redir->File, &WriteSize, Buffer);
        if (EFI_ERROR(Status)) {
          break;
        }
      }

      Len   -= Size;
      Start += Size;
    }


  } else {
    //
    // Unicode
    //
    Len = StrSize (String) - sizeof(CHAR16);
    if (Len > 0)  {
      Status = Redir->File->Write (Redir->File, &Len, String);
    } 
  }

  //
  // Error check
  //
  if (EFI_ERROR(Status)) {
    Redir->WriteError = Status;
    SEnvBatchGetConsole( &TextIn, &TextOut );
    SPrint(UnicodeBuffer,100,L"write error: %r\n\r",Status);
    Status = TextOut->OutputString( TextOut, UnicodeBuffer);
  }

  if (InvalidChar && !EFI_ERROR(Status)) {
    Status = EFI_WARN_UNKNOWN_GLYPH;
  }

  return Status;
}


EFI_STATUS
SEnvTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS              Status;         
  ENV_SHELL_REDIR_FILE    *Redir;

  Redir = CR(This, ENV_SHELL_REDIR_FILE, Out, ENV_REDIR_SIGNATURE);
  Status = ST->ConOut->TestString (ST->ConOut, String);

  //
  // Test String
  //
  if (!EFI_ERROR(Status) && Redir->Ascii) {
    while (*String && *String < 0x100) {
      String += 1;
    }

    if (*String > 0xff) {
      Status = EFI_UNSUPPORTED;
    }
  }

  return Status;
}


EFI_STATUS 
SEnvQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber,
  OUT UINTN                       *Columns,
  OUT UINTN                       *Rows
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  if (ModeNumber > 0) {
    return EFI_INVALID_PARAMETER;
  }

  *Columns = 0;
  *Rows = 0;
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvSetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return ModeNumber > 0 ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
}

EFI_STATUS
SEnvSetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN UINTN                            Attribute
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  This->Mode->Attribute = (UINT32) Attribute;
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}


EFI_STATUS
SEnvSetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        Column,
  IN UINTN                        Row
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
SEnvEnableCursor (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                      Enable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  This->Mode->CursorVisible = Enable;
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDummyReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{ 
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDummyOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDummyTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS 
SEnvDummyQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber,
  OUT UINTN                       *Columns,
  OUT UINTN                       *Rows
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDummySetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDummySetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN UINTN                            Attribute
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDummyClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDummySetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        Column,
  IN UINTN                        Row
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
SEnvDummyEnableCursor (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                      Enable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  return EFI_SUCCESS;
}
