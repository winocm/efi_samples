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

    display.c

Abstract:

    EFI Boot Manager



Revision History

--*/

#include "bm.h"

//
// Internal prototypes
//

#define BM_MENU_OPTION_SIGNATURE    EFI_SIGNATURE_32('b','m','m','n')

typedef struct {
    UINTN               Signature;
    LIST_ENTRY          Link;

    UINTN               Row;
    UINTN               Col;
    CHAR16              *Description;
    UINTN               Skip;

    VOID                *Context;
} BM_MENU_OPTION;


//
// Globals
//

LIST_ENTRY  BmMenu;

//
//
//

VOID
BmInitDisplay(
    VOID
    )
{
    InitializeListHead (&BmMenu);
}


VOID
BmDisplayString (
    IN UINTN    StringNo
    )
{
    Print (BmString(StringNo));
}


VOID
BmNewLine(
    VOID
    )
{
    Print (L"\n");
}


VOID
BmDisplay (
    IN UINTN            StringNo
    )
// Clear display and put up first lines
{
    BmBanner ();
    BmDisplayString (StringNo);
}

VOID
BmClearLine (
    VOID
    )
// Clear line before cursor, and return cursor to the left
{
    UINTN               Column;

    Column = ST->ConOut->Mode->CursorColumn;
    if (Column) {
        Print (L"\r%*a\r", Column, "");
    }
}


VOID
BmPause (
    VOID
    )
// Print pause message and wait for a key
{
    SIMPLE_INPUT_INTERFACE      *ConIn;
    EFI_INPUT_KEY               Key;
    EFI_STATUS                  Status;

    ConIn = ST->ConIn;
    
    BmClearLine ();
    BmDisplayString (BM_PAUSED);
    Status = WaitForSingleEvent(ConIn->WaitForKey, 0);
    ASSERT (!EFI_ERROR(Status));
    Status = ConIn->ReadKeyStroke(ConIn, &Key);
    ASSERT (!EFI_ERROR(Status));
    BmClearLine ();
}

BM_MENU_OPTION *
BmDisplayMenu (
    IN UINTN        StringNo,
    IN OUT BOOLEAN  *AutoBoot,
    IN UINTN        Timeout
    )
{    
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    UINTN                           Row, Col;
    LIST_ENTRY                      *Link, *NewPos;
    BM_MENU_OPTION                  *Menu, *Selection, *SavedMenu;
    SIMPLE_INPUT_INTERFACE          *ConIn;
    EFI_INPUT_KEY                   Key;
    EFI_STATUS                      Status;
    BM_SCREEN_OPERATION             ScreenOperation;
    UINTN                           SelectionOffset;
    UINTN                           SelectionStrLen;
    UINTN                           ScreenWidth;
    UINTN                           ScreenHeight;
    UINTN                           TopRow;
    UINTN                           BottomRow;
    LIST_ENTRY                      *TopOfScreen;
    BOOLEAN                         Repaint;

    //
    // Display banner
    //
    BmDisplay (StringNo);
    Print (L"\n");

    ConOut = ST->ConOut;
    ConIn = ST->ConIn;
    TopRow = ConOut->Mode->CursorRow + 1;
    Row = ConOut->Mode->CursorRow + 1;
    Col = 4;

    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &ScreenWidth, &ScreenHeight);
    BottomRow = ScreenHeight - 8;

    ASSERT (!IsListEmpty (&BmMenu));
    TopOfScreen = BmMenu.Flink;
    Repaint = TRUE;
    Menu = NULL;

    //
    // Get user's selection
    //
    Selection = NULL;
    NewPos = BmMenu.Flink;
    SelectionOffset = 0;
    ConOut->EnableCursor(ConOut, FALSE);
    while (!Selection) {

        if (Repaint == TRUE) {
          //
          // Display menu
          //
          SavedMenu = Menu;
          Row = TopRow;
          for (Link=TopOfScreen; Link != &BmMenu; Link=Link->Flink) {
              Menu = CR(Link, BM_MENU_OPTION, Link, BM_MENU_OPTION_SIGNATURE);
              Menu->Row = Row;
              Menu->Col = Col;
              PrintAt (Col, Row, L"%N%-.64s", Menu->Description);

              // BUGBUG: need to handle the bottom of the display
              Row += Menu->Skip;

              if (Row > BottomRow) {
                Row = BottomRow+1;
                break;
              }
          }

          Row += 2;
          PrintAt (Col, Row, BmString (BM_HELP_FOOTER), ARROW_UP, ARROW_DOWN);
          Repaint = FALSE;
          if (SavedMenu != NULL) {
              Menu = SavedMenu;
          }
        }

        if (NewPos) {
            PrintAt (Menu->Col, Menu->Row, L"%-.64s", Menu->Description);
            Menu = CR(NewPos, BM_MENU_OPTION, Link, BM_MENU_OPTION_SIGNATURE);

            // Set reverse attribute
            ConOut->SetAttribute (ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY));
            PrintAt (Menu->Col, Menu->Row, L"%-.64s", Menu->Description + SelectionOffset);
            SelectionStrLen = StrLen(Menu->Description);

            // Clear reverse attribute
            ConOut->SetAttribute (ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
        }

        //
        // handle situation where timeout==0 and autoboot
        // implying start the default choice immediately
        //
        if (Timeout==0 && *AutoBoot) {
            //
            // if timeout is zero, check for any keystroke
            // this will prevent a failing program from hanging
            // the system on successive boots.
            //
            // delay a couple of sec so as to give a chance for
            // pressing a key to halt default load
            //
            Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);
            if(EFI_ERROR(Status)) {
                Status = EFI_TIMEOUT;
            } else {
                *AutoBoot=FALSE;
            }
        }            

        if (Status != EFI_TIMEOUT)
            do {
                if (Timeout>0 && *AutoBoot) {
                    PrintAt (Col, Row + 1, L"Default boot selection will be booted in %d seconds  ", Timeout);
                    Status = WaitForSingleEvent(ConIn->WaitForKey, 10000000);
                    if (Status == EFI_TIMEOUT) {
                        Timeout--;
                    } else {
                        ASSERT (!EFI_ERROR(Status));
                        *AutoBoot = FALSE;
                    }
                } else {
                    Status = WaitForSingleEvent(ConIn->WaitForKey, 0);
                    ASSERT (!EFI_ERROR(Status));
                }
            } while (Status==EFI_TIMEOUT && Timeout>0 && *AutoBoot);

        PrintAt (Col, Row + 1, L"%-.64s",L"");


        ScreenOperation = BmNoOperation;
        if (Status==EFI_TIMEOUT) {
            Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
        } else {
            Status = ConIn->ReadKeyStroke (ConIn, &Key);
            ASSERT (!EFI_ERROR(Status));
        }

        switch (Key.UnicodeChar) {
        case CHAR_CARRIAGE_RETURN:
            ScreenOperation = BmSelect;
        break;
        case '^':
            ScreenOperation = BmUp;
        break;
        case 'V':
        case 'v':
            ScreenOperation = BmDown;
        break;
        case 0:
            if (Key.ScanCode == SCAN_UP) {
                ScreenOperation = BmUp;
            } else if (Key.ScanCode == SCAN_DOWN) {
                ScreenOperation = BmDown;
            } else if (Key.ScanCode == L' ') {
                ScreenOperation = BmSelect;
            } else if (Key.ScanCode == SCAN_LEFT) {
                ScreenOperation = BmLeft;
            } else if (Key.ScanCode == SCAN_RIGHT) {
                ScreenOperation = BmRight;
            }
        break;
        }

        switch (ScreenOperation) {
        case BmSelect:
            Selection = Menu;
            PrintAt (Menu->Col, Menu->Row, L"%H%-.64s", Menu->Description);
        break;
        case BmUp:
            SelectionOffset = 0;
            if (NewPos->Blink != &BmMenu) {
                NewPos = NewPos->Blink;
                if (Menu->Row <= TopRow) {
                    TopOfScreen = TopOfScreen->Blink;
                    Repaint = TRUE;
                }
            }
        break;
        case BmDown:
            SelectionOffset = 0;
            if (NewPos->Flink != &BmMenu) {
                NewPos = NewPos->Flink;
                if (Menu->Row >= BottomRow) {
                    TopOfScreen = TopOfScreen->Flink;
                    Repaint = TRUE;
                }
            }
        break;
        case BmLeft:
            if (SelectionOffset > 0) {
                SelectionOffset--;
            }
        break;
        case BmRight:
            if (SelectionStrLen > 64) {
                if (SelectionOffset < (SelectionStrLen - 64)) {
                    SelectionOffset++;
                }
            }
        break;
        case BmNoOperation:
        default:
            break;
        }
    }

    ConOut->SetCursorPosition (ConOut, 0, Row);
    ConOut->EnableCursor(ConOut, TRUE);
    BmNewLine();

    return Selection;
}


VOID
BmFreeMenu (
    VOID
    )
{
    BM_MENU_OPTION                  *Menu;

    while (!IsListEmpty (&BmMenu)) {
        Menu = CR(BmMenu.Flink, BM_MENU_OPTION, Link, BM_MENU_OPTION_SIGNATURE);
        RemoveEntryList (&Menu->Link);
        FreePool (Menu);
    }
}
    

VOID
BmMenuOption (
    IN CHAR16                       *String,
    IN VOID                         *Context
    )
{
    BM_MENU_OPTION                  *Menu;

    Menu = AllocateZeroPool (sizeof(BM_MENU_OPTION));
    ASSERT (Menu);

    Menu->Signature = BM_MENU_OPTION_SIGNATURE;
    Menu->Description = String;
    Menu->Context = Context;
    Menu->Skip = 1;
    InsertTailList (&BmMenu, &Menu->Link);
}


VOID
BmInternalMenuOption (
    IN UINTN            StringNo
    )
{
    BmMenuOption (BmString(StringNo), (VOID *) StringNo);
}


VOID
BmSkipMenuLine (
    VOID
    )
{
    BM_MENU_OPTION                  *Menu;

    if (!IsListEmpty(&BmMenu)) {
        Menu = CR(BmMenu.Blink, BM_MENU_OPTION, Link, BM_MENU_OPTION_SIGNATURE);
        Menu->Skip += 1;
    }
}


STATIC BOOLEAN BmFirstBootMenu = TRUE;

EFI_STATUS
BmBootMenu (
    OUT BOOLEAN     *Maintenance
    )
{
    EFI_STATUS          Status;
    LIST_ENTRY          *Link;
    BM_MENU_OPTION      *Selection;
    BM_LOAD_OPTION      *Option;
    BOOLEAN             AutoBoot;
    UINTN               Timeout;


    Status = EFI_SUCCESS;
    *Maintenance = FALSE;

    //
    // Build the selection list
    //

    BmFreeMenu ();
    for (Link = BmOrderedBootOptions.Flink; Link != &BmOrderedBootOptions; Link = Link->Flink) {
        Option = CR(Link, BM_LOAD_OPTION, Order, BM_LOAD_OPTION_SIGNATURE);
        BmMenuOption (Option->Description, Option);
    }

    BmInternalMenuOption (BM_MAINTENANCE_MENU);

    //
    // Check for autoboot feature
    //
    AutoBoot = FALSE;
    Timeout  = 0;
    if (BmFirstBootMenu) {
        Timeout = 0xFFFF;
        if (BmTimeout.DataSize != 0) {
            Timeout = *(UINT16 *)BmTimeout.u.Data;
        }

        if (Timeout != 0xffff) {
            AutoBoot = TRUE;
        }
        BmFirstBootMenu = FALSE;
    }

    //
    // Ask the user for the selection
    //
    Selection = BmDisplayMenu (BM_SELECT_BOOT_OPTION, &AutoBoot, Timeout);
    Option = Selection->Context;

    //
    // parse the selected option
    //
    do {
        switch ((UINTN)Option) {
        case BM_MAINTENANCE_MENU :
            if (AutoBoot) {
                BmPause ();
            }
            AutoBoot = FALSE;
            *Maintenance = TRUE;
            break;
        default:
            Status = BmLoad (Option, TRUE, AutoBoot);
            if (!EFI_ERROR(Status)) {
                AutoBoot = FALSE;
            }
            if (EFI_ERROR(Status) && AutoBoot) {
                Link = Selection->Link.Flink;
                if (Link == &BmMenu) {
                    Link = Link->Flink;
                }
                Selection = CR(Link, BM_MENU_OPTION, Link, BM_MENU_OPTION_SIGNATURE);
                Option = Selection->Context;
            }
            break;
        } 
    } while (AutoBoot);

    return Status;
}
