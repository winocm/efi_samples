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

    menu.c

Abstract:

    EFI Boot Manager



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "menu.h"

VOID
MenuFree (
    IN  EFI_MENU    *Menu
    )
{
    MENU_OPTION *MenuOption;
    
    //
    // Delete the menu options
    //
    while (!IsListEmpty(&Menu->Head)) {
        MenuOption = CR(Menu->Head.Flink, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        FreePool (MenuOption->Description);
        RemoveEntryList (&MenuOption->Link);
        ASSERT(MenuOption->Signature == MENU_OPTION_SIGNATURE);
        FreePool (MenuOption);
    }
}

MENU_OPTION *
AllocateMenuOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN VOID         *Context
    )
{
    MENU_OPTION     *MenuOption;

    MenuOption = AllocateZeroPool (sizeof(MENU_OPTION));
    ASSERT (MenuOption);

    MenuOption->Signature = MENU_OPTION_SIGNATURE;
    MenuOption->Description = StrDuplicate (String);
    MenuOption->Attribute = Attribute;
    MenuOption->Context = Context;
    MenuOption->Skip = 1;
    MenuOption->Col = Menu->Col;
    InsertTailList (&Menu->Head, &MenuOption->Link);
    return MenuOption;
}

VOID
MenuDisplayWindow (
    IN EFI_MENU     *RootMenu,
    IN UINTN        StartRow,
    IN UINTN        ScreenHeight,
    IN UINTN        ScreenWidth
    )
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    LIST_ENTRY                      *Link;
    MENU_OPTION                     *Menu;  
    UINTN                           Row, Col;
    UINTN                           EndRow;
    
    ConOut = ST->ConOut;

    ASSERT(RootMenu->TopOfPage);

    EndRow = ScreenHeight - RootMenu->FooterHeight;
    Row = StartRow;
    Link = &RootMenu->TopOfPage->Link;
    for (; Link != &RootMenu->Head; Link = Link->Flink) {
        Menu = CR(Link, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);

        if (RootMenu->Selection == Menu) {
            ConOut->SetAttribute (ConOut, RootMenu->ReverseScreenAttribute);

        } else {
            ConOut->SetAttribute (ConOut, Menu->Attribute);
        }
        PrintAt (Menu->Col, Row, RootMenu->FmtStr, Menu->Description);
        Menu->Row = Row;

        Col = Menu->Col;
        Row += Menu->Skip;
        RootMenu->MaxMenuRow = Row;
        if (Row > EndRow) {
            RootMenu->BottomOfPage = Menu;
            break;
        }
    }
    //
    // Restore Screen Attributes
    //
    ConOut->SetAttribute (ConOut, RootMenu->ScreenAttribute);   
    if (RootMenu->Selection->ContextPrint) {
        PrintAt (Col, Row, RootMenu->FmtStr, L"");
        RootMenu->Selection->ContextPrint (
                                    RootMenu->Selection, 
                                    RootMenu->Selection->Context, 
                                    &Row, Col
                                    );
    }

    for (; Row <= EndRow ; Row++) {
        //
        // Clear the rest of the screen
        //
        PrintAt (Col, Row, RootMenu->FmtStr, L"");
    }

    if (RootMenu->BottomOfPage == NULL) {
        RootMenu->BottomOfPage = Menu;
    }
}

VOID *
MenuDisplay (
    IN  EFI_MENU        *Menu,
    IN  EFI_INPUT_KEY   *Key
    )
{    
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;
    UINTN                           StartRow, EndRow;
    LIST_ENTRY                      *Link;
    MENU_OPTION                     *Selection;
    EFI_STATUS                      Status;
    BOOLEAN                         Scroll;
    UINTN                           MaxColumn, ScreenSize;
    BOOLEAN                         ExitNow;
    UINTN                           Row;
    UINTN                           SelectionOffset;
    UINTN                           SelectionStrLen;

    ConOut = ST->ConOut;
    ConIn = ST->ConIn;

    Status = ConOut->SetAttribute (ConOut, Menu->ScreenAttribute);
    ASSERT (!EFI_ERROR (Status));
    ConOut->EnableCursor(ConOut, FALSE);
    if (Menu->ClearScreen) {
        Status = ConOut->ClearScreen (ConOut);
        ASSERT (!EFI_ERROR (Status));
    }
    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxColumn, &ScreenSize);

    PrintBanner();
    Print (Menu->Header);

    StartRow = ConOut->Mode->CursorRow + 1;

    Menu->TopOfPage = CR(Menu->Head.Flink, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
    if (Menu->Selection == NULL) {
        Menu->Selection = Menu->TopOfPage;
    }
    Menu->BottomOfPage = NULL;
    Selection = Menu->TopOfPage;
    Scroll = TRUE;
    SelectionOffset = 0; 
    for (ExitNow = FALSE;!ExitNow;) {
        if (Scroll) {
            MenuDisplayWindow (Menu, StartRow, ScreenSize, MaxColumn);
            if (Menu->FooterUnderMenu) {
                EndRow = Menu->MaxMenuRow + 1;
            } else {
                EndRow = ScreenSize - Menu->FooterHeight - 1;    
            }
            if (Menu->Footer) {
                PrintAt (Menu->Col, EndRow, L"%s", Menu->Footer);
            }
            Scroll = FALSE;
        }
        //
        // Get user's selection
        //

        WaitForSingleEvent (ConIn->WaitForKey, 0);
        ConIn->ReadKeyStroke (ConIn, Key);

        if (Key->UnicodeChar ) {
            ExitNow = TRUE;
        } else {
            ConOut->SetAttribute (ConOut, Menu->Selection->Attribute);
            SelectionStrLen = StrLen (Menu->Selection->Description);
            PrintAt (Menu->Selection->Col, Menu->Selection->Row, Menu->FmtStr, Menu->Selection->Description + SelectionOffset);

            switch (Key->ScanCode) {
            case SCAN_UP:
               SelectionOffset = 0;
               Link = Menu->Selection->Link.Blink;
               if (Link != &Menu->Head) {
                    Selection = CR(Link, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
                    if (Menu->Selection == Menu->TopOfPage) {
                        Menu->TopOfPage = Selection;
                        Link = Menu->BottomOfPage->Link.Blink;
                        if (Link != &Menu->Head) {
                            Menu->BottomOfPage = CR(Link, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
                            Scroll = TRUE;
                        }
                    } 
                    Menu->Selection = Selection;
                }
                break;

            case SCAN_DOWN:
                SelectionOffset = 0;
                Link = Menu->Selection->Link.Flink;
                if (Link != &Menu->Head) {
                    Selection = CR(Link, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
                    if (Menu->Selection == Menu->BottomOfPage) {
                        Link = Menu->TopOfPage->Link.Flink;
                        if (Link != &Menu->Head) {
                            Menu->TopOfPage = CR(Link, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
                            Scroll = TRUE;
                        }
                        Menu->BottomOfPage = Selection;
                    }
                }
                Menu->Selection = Selection;
                break;

            case SCAN_RIGHT:
                if (SelectionStrLen > 64) {
                    if (SelectionOffset < (SelectionStrLen - 64)) {
                        SelectionOffset++;
                    }
                }
                break;

            case SCAN_LEFT:
                if (SelectionOffset > 0) {
                    SelectionOffset--;
                }
                break;

            case SCAN_HOME:
                // BugBug: Fix Me
                //  Start with first entry in at the top of the screen.
                //
                break;

            case SCAN_END:
                // BugBug: Fix Me
                //  Start with last entry in at the bottom of the screen.
                //
                break;

            case SCAN_PAGE_DOWN:
                // BugBug: Fix Me
                //  SCAN_DOWN for the size of the screen.
                //
                break;

            case SCAN_PAGE_UP:
                // BugBug: Fix Me
                //  SCAN_UP for the size of the screen.
                //
                break;

            default:
                Scroll = FALSE;
            } 

            if (!Scroll) {
                ConOut->SetAttribute (ConOut, Menu->ReverseScreenAttribute);
                SelectionStrLen = StrLen (Menu->Selection->Description);
                PrintAt (Menu->Selection->Col, Menu->Selection->Row, Menu->FmtStr, Menu->Selection->Description + SelectionOffset);
                ConOut->SetAttribute (ConOut, Menu->ScreenAttribute);
                if (Menu->Selection->ContextPrint) {
                    Row = Menu->MaxMenuRow;
                    PrintAt (Menu->Col, Row, Menu->FmtStr, L"");
                    Menu->Selection->ContextPrint (
                                                Menu->Selection, 
                                                Menu->Selection->Context, 
                                                &Row, Menu->Col
                                                );
                } 
            } 
        } 
    } 

    ConOut->SetAttribute (ConOut, Menu->ReverseScreenAttribute | EFI_BRIGHT);
    PrintAt (Menu->Selection->Col, Menu->Selection->Row, Menu->FmtStr, Menu->Selection->Description);
    ConOut->SetCursorPosition (ConOut, 0, Menu->Selection->Row);
    ConOut->EnableCursor(ConOut, TRUE);

    ConOut->SetAttribute (ConOut, Menu->ScreenAttribute);
    return Menu->Selection->Context;

}


VOID
BootPause (
    VOID
    )
// Print pause message and wait for a key
{
    SIMPLE_INPUT_INTERFACE      *ConIn;
    EFI_INPUT_KEY               Key;
    EFI_STATUS                  Status;

    ConIn = ST->ConIn;
    
    BmntClearLine ();
    Print (L"    Hit any key to continue ");
    Status = WaitForSingleEvent(ConIn->WaitForKey, 0);
    ASSERT (!EFI_ERROR(Status));
    Status = ConIn->ReadKeyStroke(ConIn, &Key);
    ASSERT (!EFI_ERROR(Status));
    BmntClearLine ();
}

VOID
BmntClearLine (
    VOID
    )
// Clear line before cursor, and return cursor to the left
{
    UINTN   Column;

    Column = ST->ConOut->Mode->CursorColumn;
    if (Column) {
        Print (L"\r%*a\r", Column, "");
    }
}

VOID
SkipMenuLine(
    IN EFI_MENU *Menu
    )
{
    MENU_OPTION     *MenuOption;

    if (!IsListEmpty(&Menu->Head)) {
        MenuOption = CR(Menu->Head.Blink, MENU_OPTION, Link, MENU_OPTION_SIGNATURE); 
        MenuOption->Skip += 1;
    }
}
