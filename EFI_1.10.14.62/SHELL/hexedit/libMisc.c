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

#include "heditor.h"


extern BOOLEAN HEditorMouseAction;

VOID 
HEditorClearLine ( 
  IN UINTN Row 
)
/*++

Routine Description: 

  Clear line at Row

Arguments:  

  Row -- row number to be cleared ( start from 1 )

Returns:  

  EFI_SUCCESS

--*/
{
  CHAR16 Line[200];
  UINTN  Index;
  UINTN  Limit;
  UINTN  StartCol;
  
  if ( HEditorMouseAction ) {
    Limit = 3 * 0x10;
    StartCol = HEX_POSITION;
  } else {
    Limit = DISPLAY_MAX_COLUMN;
    StartCol = 1;
  }
  
  
  //
  // prepare a blank line
  //
  for ( Index = 0; Index < Limit; Index++ ) {
    Line[Index] = ' ';
  }
  
  
  if ( Row == DISPLAY_MAX_ROW && Limit == DISPLAY_MAX_COLUMN) {
    //
    // if '\0' is still at position 80, it will cause first line error
    //
    Line[Limit - 1 ] = '\0';
  } else {
    Line[Limit ] = '\0';
  }
  
  //
  // print out the blank line
  //
  PrintAt ( StartCol - 1, Row -1 , Line );
}




HEFI_EDITOR_LINE*
HLineDup ( 
  IN  HEFI_EDITOR_LINE *Src
)
/*++

Routine Description: 

  Duplicate a line

Arguments:  

  Src -- line to be duplicated

Returns:  

  NULL -- wrong
  Not NULL -- line created

--*/
{
  HEFI_EDITOR_LINE *Dest;

  //
  // allocate for the line structure
  //  
  Dest = AllocatePool(sizeof(HEFI_EDITOR_LINE));
  if ( Dest == NULL ) {
    return NULL;
  }
  
  Dest->Signature = EFI_EDITOR_LINE_LIST;
  Dest->Size = Src->Size;

  CopyMem ( Dest->Buffer, Src->Buffer, 0x10 );

  Dest->Link = Src->Link;

  return Dest;
}






VOID
HLineFree ( 
  IN  HEFI_EDITOR_LINE *Src
)
/*++

Routine Description: 

  Free a line and it's internal buffer

Arguments:  

  Src -- line to be freed

Returns:  

  None

--*/
{
  if ( Src == NULL ) {
    return;
  }
  
  HEditorFreePool ( Src );
  
}




VOID
HEditorFreePool (
  IN VOID            *Buffer
)
/*++

Routine Description: 

  Free a pointed area.
  its different with FreePool is that it can free NULL pointer
  and it's internal buffer

Arguments:  

  Buffer -- Buffer to be freed

Returns:  

  None

--*/
{
  if ( Buffer == NULL ) {
    return ;
  }
  
  FreePool ( Buffer );
}






HEFI_EDITOR_LINE*
_HLineAdvance (
  IN  UINTN Count
)
/*++

Routine Description: 

  Advance to the next Count lines

Arguments:  

  Count -- line number to advance

Returns:  

  NULL -- wrong
  Not NULL -- line after advance

--*/
{
  UINTN             Index;
  HEFI_EDITOR_LINE  *Line;

  Line = HMainEditor.BufferImage->CurrentLine;
  if (  Line == NULL ) {
    return NULL;
  }

  for ( Index = 0 ; Index < Count ; Index++ ) {
    //
    // if already last line
    //
    if ( Line->Link.Flink == HMainEditor.BufferImage->ListHead ) {
      return NULL;
    }

    Line = CR(Line->Link.Flink,HEFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
  }

  return Line;
}






HEFI_EDITOR_LINE*
_HLineRetreat (
  IN  UINTN Count
)
/*++

Routine Description: 

  Retreat to the previous Count lines

Arguments:  

  Count -- line number to retreat

Returns:  

  NULL -- wrong
  Not NULL -- line after retreat

--*/
{ 
  UINTN              Index;
  HEFI_EDITOR_LINE   *Line;

  Line = HMainEditor.BufferImage->CurrentLine;
  if (  Line == NULL ) {
    return NULL;
  }

  for ( Index = 0 ; Index < Count ; Index++ ) {
  
    //
    // already the first line
    //
    if ( Line->Link.Blink == HMainEditor.BufferImage->ListHead ) {
      return NULL;
    }

    Line = CR(Line->Link.Blink,HEFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
  }

  return Line;
}






HEFI_EDITOR_LINE*
HMoveLine (
  IN  INTN Count
)
/*++

Routine Description: 

  Advance/Retreat lines

Arguments:  

  Count -- line number to advance/retreat
    >0 : advance
    <0: retreat  

Returns:  

  NULL -- wrong
  Not NULL -- line after advance

--*/

{ 
  HEFI_EDITOR_LINE  *Line;
  UINTN             AbsCount;
  
  //
  // difference with MoveCurrentLine
  //     just return Line
  //     do not set currentline to Line
  //
  if ( Count <=0 ) {
    AbsCount = -Count; 
    Line = _HLineRetreat ( AbsCount );
  } else {
    Line = _HLineAdvance ( Count );
  }
  
  return Line;
}






HEFI_EDITOR_LINE*
HMoveCurrentLine (
  IN  INTN Count
)
/*++

Routine Description: 

  Advance/Retreat lines and set CurrentLine in BufferImage to it

Arguments:  

  Count -- line number to advance/retreat
    >0 : advance
    <0: retreat

Returns:  

  NULL -- wrong
  Not NULL -- line after advance


--*/
{ 
  HEFI_EDITOR_LINE *Line;
  UINTN            AbsCount;
  
  //
  // <0: retreat
  // >0: advance
  //
  if ( Count <=0 ) {
    AbsCount = -Count; 
    Line = _HLineRetreat ( AbsCount );
  } else {
    Line = _HLineAdvance ( Count );
  }
  
  if ( Line == NULL ) {
    return NULL ;
  }
  
  HMainEditor.BufferImage->CurrentLine = Line;
  
  return Line;
}





BOOLEAN 
HIsValidFileNameChar ( 
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
  return   TRUE;
  
}


BOOLEAN 
HIsValidFileName ( 
  IN CHAR16 *Name 
) 
/*++

Routine Description: 

  check if file name has illegal character

Arguments:  

   Name -- file name

Returns:  

   TRUE : valid
   FALSE : invalid 

--*/
{

  UINTN Index;
  
  //
  // if one char in Name not appears ValidFileName
  // then the name is invalid
  //
  for ( Index = 0 ; Index < StrLen ( Name ); Index++ ) {
    if ( !HIsValidFileNameChar ( Name[Index] ) ) {
      return FALSE;
    }
  }
   
  
  return TRUE;
}









EFI_STATUS
HFreeLines (
  IN EFI_LIST_ENTRY   *ListHead,
  IN HEFI_EDITOR_LINE *Lines 
)
/*++

Routine Description: 

  Free all the lines in HBufferImage
    Fields affected:
    Lines
    CurrentLine
    NumLines
    ListHead 

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{ 
  EFI_LIST_ENTRY   *Link;
  HEFI_EDITOR_LINE *Line;
  
  //
  // release all the lines 
  //
  if ( Lines != NULL ) {
  
    Line = Lines;
    Link = &(Line->Link);
    do {
      Line = CR(Link,HEFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
      Link = Link -> Flink;
      HLineFree ( Line );
    } while ( Link != ListHead );
  }

  ListHead->Flink = ListHead;
  ListHead->Blink = ListHead;

  return EFI_SUCCESS; 
}



UINTN
HStrStr  (
  IN  CHAR16  *Str,
  IN  CHAR16  *Pat
  )
/*++

Routine Description: 

  Search Pat in Str

Arguments:  

  Str -- mother string
  Pat -- search pattern


Returns:  

  0 : not found
  >= 1 : found position + 1

--*/
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
  // >=1 : found position + 1
  //
  return ((j == Lenp) ? (i - Lenp) : -1)+1;

}

INT32
HGetTextX ( 
  IN INT32 GuidX 
 ) 
{
  INT32  Gap;

  HMainEditor.MouseAccumulatorX += GuidX;
  Gap = (HMainEditor.MouseAccumulatorX * (INT32)HMainEditor.ScreenSize.Column) / (INT32)(50 * (INT32)HMainEditor.MouseInterface->Mode->ResolutionX);
  HMainEditor.MouseAccumulatorX = (HMainEditor.MouseAccumulatorX * (INT32)HMainEditor.ScreenSize.Column) % (INT32)(50 * (INT32)HMainEditor.MouseInterface->Mode->ResolutionX);
  HMainEditor.MouseAccumulatorX = HMainEditor.MouseAccumulatorX / (INT32)HMainEditor.ScreenSize.Column;
  return Gap;
}

INT32
HGetTextY ( 
  IN INT32 GuidY 
  ) 
{
  INT32  Gap;

  HMainEditor.MouseAccumulatorY += GuidY;
  Gap = (HMainEditor.MouseAccumulatorY * (INT32)HMainEditor.ScreenSize.Row) / (INT32)(50 * (INT32)HMainEditor.MouseInterface->Mode->ResolutionY);
  HMainEditor.MouseAccumulatorY = (HMainEditor.MouseAccumulatorY * (INT32)HMainEditor.ScreenSize.Row) % (INT32)(50 * (INT32)HMainEditor.MouseInterface->Mode->ResolutionY);
  HMainEditor.MouseAccumulatorY = HMainEditor.MouseAccumulatorY / (INT32)HMainEditor.ScreenSize.Row;

  return Gap;
}

EFI_STATUS
HXtoi (
  IN  CHAR16  *Str,
  OUT UINTN   *Value
)
// convert hex string to uint
{
  UINT64       u;
  CHAR16       c;
  UINTN        Size;
  
  Size = sizeof ( UINTN );

  // skip leading white space
  while (*Str && *Str == ' ') {
    Str += 1;
  }

  if ( StrLen ( Str ) > Size * 2 ) {
    return EFI_LOAD_ERROR;
  }

  // convert hex digits
  u = 0;
  c = *Str;
  while (c ) {
    c = *Str;
    Str++;
    
    if ( c == 0 ) {
      break;
    }

    //
    // not valid char
    //
    if ( ! ( ( c >= 'a' && c <= 'f' ) || 
             ( c >= 'A' && c <= 'F' ) || 
             ( c >= '0' && c <= '9' ) ||  
             ( c == '\0') ) 
        ) {
      return EFI_LOAD_ERROR;
    }
      
    if (c >= 'a'  &&  c <= 'f') {
      c -= 'a' - 'A';
    }

    if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F')) {
      u = LShiftU64 ( u, 4 )   + (   c - (c >= 'A' ? 'A'-10 : '0') ) ;
    } else {
      //
      // '\0'
      //
      break;
    }
  }

  *Value = ( UINTN ) u;
  
  return EFI_SUCCESS;
}

#endif  
