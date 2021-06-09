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
    libInputBar.c

  Abstract:
    Definition of the user input bar (the same position with the Status Bar)

--*/

#ifndef _LIB_INPUT_BAR
#define _LIB_INPUT_BAR

#include "editor.h"


EFI_EDITOR_INPUT_BAR MainInputBar;

// basic initialization for MainInputBar
EFI_EDITOR_INPUT_BAR MainInputBarConst = {
  NULL,
  NULL,
  0
};

extern BOOLEAN StatusBarNeedRefresh;




//
// Name:
//   MainInputBarInit -- Initialize function for MainInputBar;
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//    

EFI_STATUS
MainInputBarInit (
  VOID
  )
{
  //
  // initialize inputbar structure
  //
  CopyMem (&MainInputBar, &MainInputBarConst, sizeof(MainInputBar));

  return EFI_SUCCESS;
}





//
// Name:
//   MainInputBarCleanup -- cleanup function for MainInputBar;
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//    

EFI_STATUS
MainInputBarCleanup (
  VOID
  )
{
  //
  // free input bar's prompt and input string
  //
  EditorFreePool ((VOID*)MainInputBar.Prompt);

  EditorFreePool ((VOID*)MainInputBar.ReturnString);

  return EFI_SUCCESS;
}


MainInputBarPrintInput ( 
   VOID
 ) 
{
  UINTN  Limit;
  UINTN  Size;
  CHAR16 *Buffer;
  UINTN  Index;
  UINTN  PromptLen;

  PromptLen = StrLen ( MainInputBar.Prompt );
  Limit = MainEditor.ScreenSize.Column - PromptLen - 1;
  Size = StrLen ( MainInputBar.ReturnString );

  //
  // check whether the prompt length and input length will
  // exceed limit
  //
  if ( Size <= Limit ) {
    Buffer = MainInputBar.ReturnString;
  }  else {
    Buffer = MainInputBar.ReturnString + Size - Limit;
  }

   PrintAt( PromptLen , INPUT_BAR_LOCATION - 1 ,L"%s",Buffer);
   Size = StrLen ( Buffer );
   
   //
   // print " " after prompt
   //
   for ( Index = Size ; Index < Limit; Index++ ) {
      PrintAt( PromptLen + Size, INPUT_BAR_LOCATION - 1 ,L" ");
   }

   Out->SetCursorPosition (Out,Size + PromptLen ,INPUT_BAR_LOCATION - 1 );
}

    

//
// Name:
//  MainInputRefresh --  refresh function for MainInputBar;
//  it will wait for user input
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//  

EFI_STATUS
MainInputBarRefresh (
  VOID
  )
{
  EFI_EDITOR_COLOR_UNION  Orig;
  EFI_EDITOR_COLOR_UNION  New;
  EFI_INPUT_KEY           Key;
  UINTN                   Size;
  EFI_STATUS              Status;
  BOOLEAN                 NoDisplay;
  UINTN                   Limit ;
  UINTN                   PromptLen;
  
  
  //
  // variable initialization
  //
  Size = 0;
  Status = EFI_SUCCESS;

  
  // free status string
  EditorFreePool (MainEditor.StatusBar->StatusString);
  MainEditor.StatusBar->StatusString = PoolPrint (L"");
  if ( MainEditor.StatusBar->StatusString == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }

  // back up the old screen attributes
  Orig = MainEditor.ColorAttributes;
  New.Colors.Foreground = Orig.Colors.Background;
  New.Colors.Background = Orig.Colors.Foreground;

  Out->SetAttribute (Out,New.Data);

  // clear input bar
  EditorClearLine(INPUT_BAR_LOCATION);
  
  Out->SetCursorPosition(Out,0,INPUT_BAR_LOCATION - 1 );
  Print(L"%s",MainInputBar.Prompt);
  
  // that's the maximum input length that can be displayed on screen
  PromptLen = StrLen ( MainInputBar.Prompt );
  Limit = MainEditor.ScreenSize.Column - PromptLen;

  // this is a selection prompt, cursor will stay in edit area
  // actually this is for search , search/replace
  if ( StrStr ( MainInputBar.Prompt, L"Yes/No" ) ) {
    NoDisplay = TRUE;
    FileBufferRestorePosition ();
  }
  else {
    NoDisplay = FALSE;
  }
   
  // wait for user input
  for ( ;; ) {
    WaitForSingleEvent(In->WaitForKey,0);
    Status = In->ReadKeyStroke(In,&Key);
    if ( EFI_ERROR(Status) ) {
      continue;
    }
    
    // pressed ESC
    if ( Key.ScanCode == SCAN_CODE_ESC ) {
      Size = 0;
      Status = EFI_NOT_READY;
      break;
    } 
     
    // return pressed
    if ( Key.UnicodeChar == CHAR_LF || Key.UnicodeChar == CHAR_CR ) {
      break;
    } else if (Key.UnicodeChar == CHAR_BS) {  // backspace
      if (Size > 0) {
        Size--;
        MainInputBar.ReturnString [Size ] = L'\0';
        if ( !NoDisplay ) {
          MainInputBarPrintInput ();
        }
      }
    } else if (Key.UnicodeChar <= 127 && Key.UnicodeChar >= 32) {
      // VALID ASCII char pressed
      MainInputBar.ReturnString[Size] = Key.UnicodeChar;
     
      // should be less than specified length
      if ( Size >= MainInputBar.StringSize ) {
        continue;
      }
        
      Size++;
       
      
      MainInputBar.ReturnString [Size ] = L'\0';

      if ( !NoDisplay ) {
      
        MainInputBarPrintInput ();
      } else {
        // if just choose yes/no
        break;
      }
       
    }
  }

  MainInputBar.ReturnString[Size] = 0;
  
  FileBufferRestorePosition ();

  // restore screen attributes
  Out->SetAttribute (Out,Orig.Data);
  
  StatusBarNeedRefresh = TRUE;

  return Status;
}






//
// Name:
//   MainInputHide --  Clear input bar and restore file buffer cursor position
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   

EFI_STATUS
MainInputBarHide (
  VOID
  )
{
  //
  // Clear the input bar line
  //
  EditorClearLine(INPUT_BAR_LOCATION);
  FileBufferRestorePosition();
  
  return  EFI_SUCCESS;
}













//
// Name:
//   MainInputSetPrompt --  Set Prompt field for MainInputBar;
//   it will wait for user input
// In:
//   Str -- Prompt string to set
// Out:
//   EFI_SUCCESS
//   EFI_OUT_OF_RESOURCES
//    

EFI_STATUS
MainInputBarSetPrompt (
  IN  CHAR16* Str
  )
{
  // FREE the old prompt string
  EditorFreePool (MainInputBar.Prompt);
  
  MainInputBar.Prompt = PoolPrint (L"%s ",Str);
  if ( MainInputBar.Prompt == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  return EFI_SUCCESS;
}






//
// Name:
//   MainInputSetStringSize --  
//       Set StringSize field for MainInputBar and allocate space for it
// In:
//   Size -- string size ( it's unit is Unicode character)
// Out:
//   EFI_SUCCESS
//    

EFI_STATUS
MainInputBarSetStringSize   (
  UINTN   Size
  )
{
  // free the old ReturnStirng
  EditorFreePool ( MainInputBar.ReturnString );

  
  MainInputBar.StringSize = Size;

  MainInputBar.ReturnString = AllocatePool ( 2 * (Size + 1 ));
  if ( MainInputBar.ReturnString == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}


#endif  //_LIB_INPUT_BAR
