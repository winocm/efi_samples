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
    libMisc.c

  Abstract:
    Implementation of various string and line routines

--*/


#ifndef _LIB_MISC
#define _LIB_MISC

#include "editor.h"



 
//
// Name:
//   EditorClearLine -- Clear line at Row
// In:
//   Row -- row number to be cleared ( start from 1 );
// Out:
//   EFI_SUCCESS
//
VOID 
EditorClearLine ( 
  IN UINTN Row 
)
{
  CHAR16 Line[200];
  UINTN  Index;
  
  // prepare a blank line
  for ( Index = 0; Index < DISPLAY_MAX_COLUMN; Index++ ) {
    Line[Index] = ' ';
  }
  
  if ( Row == DISPLAY_MAX_ROW ) {
    // if '\0' is still at position 80, it will cause first line error
    Line[DISPLAY_MAX_COLUMN - 1 ] = '\0';
  } else {
    Line[DISPLAY_MAX_COLUMN ] = '\0';
  }
  
  // print out the blank line
  PrintAt ( 0, Row -1 , Line );
}




//
// Name:
//   LineDup -- Duplicate a line
// In:
//   Src -- line to be duplicated
// Out:
//   NULL -- wrong
//   Not NULL -- line created
//
EFI_EDITOR_LINE*
LineDup ( 
  IN  EFI_EDITOR_LINE *Src
  )
{
  EFI_EDITOR_LINE *Dest;
  
  // 
  // allocate for the line structure
  //
  Dest = AllocatePool(sizeof(EFI_EDITOR_LINE));
  if ( Dest == NULL ) {
    return NULL;
  }
  
  Dest->Signature = EFI_EDITOR_LINE_LIST;
  Dest->Size = Src->Size;
  Dest->TotalSize = Dest->Size;
  Dest->Type = Src->Type;
  
  // 
  // set the line buffer
  //
  Dest->Buffer = PoolPrint(L"%s\0",Src->Buffer);
  if ( Dest -> Buffer == NULL ) {
    FreePool ( Dest );
    return NULL;
  }

  Dest->Link = Src->Link;

  return Dest;
}






VOID
LineFree ( 
  IN  EFI_EDITOR_LINE *Src
  )
//
// Name:
//   LineFree -- Free a line and it's internal buffer
// In:
//   Src -- line to be freed
// Out:
//   EFI_SUCCESS
//

{
  if ( Src == NULL ) {
    return;
  }
  
  //
  // free line buffer and line itself
  //
  EditorFreePool ( Src -> Buffer );
  EditorFreePool ( Src );
  
}




VOID
EditorFreePool (
  IN VOID            *Buffer
  )
//
// Name:
//   EditorFreePool -- Free a pointed area.
//   its different with FreePool is that it can free NULL pointer
// In:
//   Buffer -- Buffer to be freed
// Out:
//   VOID
//

{
  //
  // if buffer is null,
  // directly returns
  //
  if ( Buffer == NULL ) {
    return ;
  }
  
  FreePool ( Buffer );
}






EFI_EDITOR_LINE*
_LineAdvance (
    IN  UINTN Count
    )
//
// Name:
//    _LineAdvance -- Advance to the next Count lines
// In:
//    Count -- line number to advance
// Out:
//    NULL -- wrong
//    Not NULL -- line after advance
//

{
  UINTN             Index;
  EFI_EDITOR_LINE   *Line;

  Line = MainEditor.FileBuffer->CurrentLine;
  if (  Line == NULL ) {
    return NULL;
  }

  for ( Index = 0 ; Index < Count ; Index++ ) {
    //
    // if already last line
    //
    if ( Line->Link.Flink == MainEditor.FileBuffer->ListHead ) {
      return NULL;
    }

    Line = CR(Line->Link.Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
  }

  return Line;
}






EFI_EDITOR_LINE*
_LineRetreat (
  IN  UINTN Count
  )
//
// Name:
//    _LineRetreat -- Retreat to the previous Count lines
// In:
//    Count -- line number to retreat
// Out:
//    NULL -- wrong
//    Not NULL -- line after retreat
//

{ 
  UINTN             Index;
  EFI_EDITOR_LINE   *Line;

  Line = MainEditor.FileBuffer->CurrentLine;
  if (  Line == NULL ) {
    return NULL;
  }

  for ( Index = 0 ; Index < Count ; Index++ ) {
    
    //
    // already the first line
    //
    if ( Line->Link.Blink == MainEditor.FileBuffer->ListHead ) {
      return NULL;
    }

    Line = CR(Line->Link.Blink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
  }

    return Line;
}






EFI_EDITOR_LINE*
MoveLine (
  IN  INTN Count
  )
//
// Name:
//    MoveLine -- Advance/Retreat lines
// In:
//    Count -- line number to advance/retreat
//      >0 : advance
//      <0: retreat
// Out:
//    NULL -- wrong
//    Not NULL -- line after advance
//

{ 
  EFI_EDITOR_LINE *Line;
  UINTN           AbsCount;
  
  //
  // if < 0, then retreat
  // if > 0, the advance
  //
  if ( Count <=0 ) {
    AbsCount = -Count; 
    Line = _LineRetreat ( AbsCount );
  } else {
    Line = _LineAdvance ( Count );
  }
  
  return Line;
}







EFI_EDITOR_LINE*
MoveCurrentLine (
  IN  INTN Count
  )
//
// Name:
//    MoveLine -- Advance/Retreat lines and set CurrentLine in FileBuffer to it
// In:
//    Count -- line number to advance/retreat
//      >0 : advance
//      <0: retreat
// Out:
//    NULL -- wrong
//    Not NULL -- line after advance
//
{ 
  EFI_EDITOR_LINE *Line;
  UINTN           AbsCount;
  
  if ( Count <=0 ) {
    AbsCount = -Count; 
    Line = _LineRetreat ( AbsCount );
  } else {
    Line = _LineAdvance ( Count );
  }
  
  if ( Line == NULL ) {
    return NULL ;
  };
  
  MainEditor.FileBuffer->CurrentLine = Line;
  
  return Line;
}






//
// Name:
//    LineStrInsert -- Insert a char into line
// In:
//    Line -- line to insert into
//    Char -- char to insert
//    Pos -- Position to insert the char at ( start from 0 )
//    StrSize -- current string size ( include '\0' ) 
//    ( unit is Unicode character )
// Out:
//    new string size ( include '\0' ) ( unit is Unicode character )
//

UINTN
LineStrInsert (
  IN      EFI_EDITOR_LINE  *Line,
  IN      CHAR16           Char,
  IN      UINTN            Pos,
  IN      UINTN            StrSize
  )
{
  UINTN   Index;
  CHAR16  *s;
  CHAR16  *Str;

  Index = (StrSize)*2;

  Str = Line -> Buffer;
  
  //
  // do not have free space
  //
  if ( Line -> TotalSize <= Line -> Size ) {
    Str = ReallocatePool( Str, Index , Index + 16);
    if ( Str == NULL ){
      return 0;
    }
      
    Line -> TotalSize += 8;
  }

  //
  // move the later part of the string one character right 
  //
  s = Str;
  for ( Index = StrSize; Index > Pos; Index--) {
    s[Index] = s[Index-1];
  }
  
  //
  // insert char into it.
  //
  s[Index] = Char;

  Line -> Buffer = Str;
  Line -> Size++;


  return (StrSize+1);
}






//
// Name:
//    LineDeleteAt -- Delete a char in line
// In:
//    Line -- line to delete in
//    Pos -- Position to delete the char at ( start from 0 )
// Out:
//    VOID
//

VOID
LineDeleteAt (
  IN  OUT EFI_EDITOR_LINE*    Line,
  IN      UINTN               Pos
  )
{
  UINTN   Index;

  //
  // move the latter characters front
  //
  for ( Index = Pos - 1; Index < Line->Size; Index++) {
    Line->Buffer[Index] = Line->Buffer[Index+1];
  }

  Line->Size--;
}






//
// Name:
//    LineCat -- Cat Src into Dest
// In:
//    Dest -- Destination string
//    Src -- Src String
// Out:
//    VOID
//

VOID
LineCat (
  IN  OUT EFI_EDITOR_LINE* Dest,
  IN      EFI_EDITOR_LINE* Src
  )
{
  CHAR16  *Str;
  UINTN   Size;

  Size = Dest->Size ;

  Dest->Buffer[Size] = 0;

  //
  // concatenate the two strings
  //
  Str = PoolPrint (L"%s%s",Dest->Buffer,Src->Buffer);
  if ( Str == NULL ) {
    Dest -> Buffer = NULL;
    return;
  }

  Dest->Size = Size + Src->Size;
  Dest->TotalSize = Dest->Size;

  FreePool (Dest->Buffer);
  FreePool (Src->Buffer);

  // 
  // put str to dest->buffer
  //
  Dest->Buffer = Str;
}







//
// Name:
//    UnicodeToAscii -- change a Unicode string t ASCII string
// In:
//    UStr -- Unicode string 
//    Lenght -- most possible length of AStr
//    AStr -- ASCII string to pass out
// Out:
//    Actuall length
//

UINTN
UnicodeToAscii  (
  IN  CHAR16  *UStr,
  IN  UINTN   Length,
  OUT CHAR8   *AStr
  )
{
  UINTN   Index;
  
  //
  // just buffer copy, not character copy
  //
  for (Index = 0; Index < Length; Index++) {
    *AStr++ = (CHAR8)*UStr++;
  }

  return Index;
}







//
// Name:
//    StrStr -- Search Pat in Str
// In:
//    Str -- mother string
//    Pat -- search pattern
// Out:
//     0 : not found
//    >= 1 : found position + 1
//
UINTN
StrStr  (
  IN  CHAR16  *Str,
  IN  CHAR16  *Pat
  )
{
  INTN    *Failure;
  INTN    i;
  INTN    j;
  INTN    Lenp;
  INTN    Lens;

  //
  // this function copies from some lib
  //
  
  Lenp = StrLen(Pat);
  Lens = StrLen(Str);

  Failure = AllocatePool(Lenp*sizeof(INTN));
  Failure[0] = -1;

  for (j=1; j< Lenp; j++ ) {
    i = Failure[j-1];
    while ( (Pat[j] != Pat[i+1]) && (i >= 0)) {
      i = Failure[i];
    }
    
    if ( Pat[i] == Pat[i+1]) {
      Failure[j] = i+1;
    } else {
      Failure[j] = -1;
    }
  }

  i = 0;
  j = 0;
  while (i < Lens && j < Lenp) {
    if (Str[i] == Pat[j]) {
      i++;
      j++;
    } else if (j == 0) {
      i++;
    } else {
      j = Failure[j-1] + 1;
    }
  }

  FreePool(Failure);

  //
  // 0: not found
  // >= 1: found position + 1
  //
  return ((j == Lenp) ? (i - Lenp) : -1)+1;

}




BOOLEAN 
IsValidFileNameChar ( 
  IN CHAR16 Ch 
  )
{
/*  
  BOOLEAN Valid;
   
  Valid = TRUE;
  
  //
  // See if there is any illegal characters within the name
  //  
  if (Ch < 0x20 ||
        Ch == '\"' ||
        Ch == ' ' ||
        Ch == '*' ||
        Ch == '/' ||
        Ch == ':' ||
        Ch == '<' ||
        Ch == '>' ||
        Ch == '?' ||
        Ch == '|' ) {
      Valid = FALSE;
    }
  
  return Valid;
  
  */
  
  // 
  // to deal with MetaArg
  if ( Ch == '*' ||
       Ch == '?' ||
       Ch == '[' ||
       Ch == ']'
   ) {
    return FALSE;
  }
  
  return  TRUE;
}


//
// Name:
//   IsValidFileName -- check if file name has illegal character
// In:
//   Name -- file name
// Out:
//   TRUE : valid
//   FALSE : invalid 
//

BOOLEAN 
IsValidFileName ( 
  IN CHAR16 *Name 
  ) 
{

  UINTN Index;

  //
  // check whether any char in Name not appears in valid file name char
  //
  for ( Index = 0 ; Index < StrLen ( Name ); Index++ ){
    if ( !IsValidFileNameChar ( Name[Index] ) ){
      return FALSE;
    }
  }
  
  return TRUE;
}

INT32
GetTextX ( 
  IN INT32 GuidX 
 ) 
{
  INT32  Gap;

  MainEditor.MouseAccumulatorX += GuidX;
  Gap = (MainEditor.MouseAccumulatorX * (INT32)MainEditor.ScreenSize.Column) / (INT32)(50 * (INT32)MainEditor.MouseInterface->Mode->ResolutionX);
  MainEditor.MouseAccumulatorX = (MainEditor.MouseAccumulatorX * (INT32)MainEditor.ScreenSize.Column) % (INT32)(50 * (INT32)MainEditor.MouseInterface->Mode->ResolutionX);
  MainEditor.MouseAccumulatorX = MainEditor.MouseAccumulatorX / (INT32)MainEditor.ScreenSize.Column;
  return Gap;
}

INT32
GetTextY ( 
  IN INT32 GuidY 
  ) 
{
  INT32  Gap;

  MainEditor.MouseAccumulatorY += GuidY;
  Gap = (MainEditor.MouseAccumulatorY * (INT32)MainEditor.ScreenSize.Row) / (INT32)(50 * (INT32)MainEditor.MouseInterface->Mode->ResolutionY);
  MainEditor.MouseAccumulatorY = (MainEditor.MouseAccumulatorY * (INT32)MainEditor.ScreenSize.Row) % (INT32)(50 * (INT32)MainEditor.MouseInterface->Mode->ResolutionY);
  MainEditor.MouseAccumulatorY = MainEditor.MouseAccumulatorY / (INT32)MainEditor.ScreenSize.Row;

  return Gap;
}

#endif  
