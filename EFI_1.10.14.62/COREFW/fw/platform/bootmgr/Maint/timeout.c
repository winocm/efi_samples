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

    timeout.c
    
Abstract:

    manages timeout variable



Revision History

--*/

#include "menu.h"

EFI_MENU TimeoutMenu = {
    MENU_SIGNATURE, NULL,  
    4, FILE_UNSELECTED, FILE_ACTIVE_SELECTION,  
    TRUE, 10, 
    NULL,
    NULL, 2, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};

EFI_MENU TimeoutHelpMenu = {
    MENU_SIGNATURE, NULL,  
    4, CON_UNSELECTED, CON_ACTIVE_SELECTION, 
    TRUE, 10, 
    L"Timeout Menu Help Screen\n\n", 
    L"    ", 4, 
    NULL, NULL, NULL, NULL, NULL, TRUE, 0, MENU_ENTRY_FORMAT
};

EFI_STATUS
Bmnt_DisplayBootTimeOut();

EFI_STATUS
ModifyBootTimeOut (
    IN BOOLEAN  ClearVariable
    ) ;

TIMEOUT_MENU_CONTEXT *
AllocateSpecialTimeoutOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN UINTN        Option
    );

VOID
FreeSpecificTimeoutMenuOption (
    IN EFI_MENU             *Menu,
    IN TIMEOUT_MENU_CONTEXT *Context
    );


//
//
//

EFI_STATUS
Bmnt_DisplayBootTimeOut () 
{
    
    CHAR16                  *Header = L"%ESet Auto Boot Timeout. Select an Option\n\n";
    TIMEOUT_MENU_CONTEXT    *SetContext;
    TIMEOUT_MENU_CONTEXT    *DeleteContext;
    TIMEOUT_MENU_CONTEXT    *TimeOutHelpContext;
    TIMEOUT_MENU_CONTEXT    *ExitContext;
    TIMEOUT_MENU_CONTEXT    *ResultContext;
    BOOLEAN                 Done;
    UINTN                   MenuOption;
    EFI_INPUT_KEY           Key;
    UINTN                   EndRow;
    UINTN                   MaxColumn, ScreenSize;

    // 
    // initialize header text
    //
    TimeoutMenu.Header = Header;
 
    InitializeListHead (&TimeoutMenu.Head);

    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &MaxColumn, &ScreenSize);

    SetContext =
        AllocateSpecialTimeoutOption (&TimeoutMenu, L"Set Timeout Value", TIMEOUT_UNSELECTED, TIMEOUT_MENU_MODIFY);
    DeleteContext =
        AllocateSpecialTimeoutOption (&TimeoutMenu, L"Delete/Disable Timeout", TIMEOUT_UNSELECTED, TIMEOUT_MENU_DELETE);
    TimeOutHelpContext =
        AllocateSpecialTimeoutOption (&TimeoutMenu, L"Help", TIMEOUT_UNSELECTED, TIMEOUT_MENU_HELP);
    ExitContext =
        AllocateSpecialTimeoutOption (&TimeoutMenu, L"Exit", TIMEOUT_UNSELECTED, TIMEOUT_MENU_EXIT);

    TimeoutMenu.Selection = NULL;
    for (Done = FALSE, ResultContext = NULL; !Done;) {
        ResultContext = MenuDisplay (&TimeoutMenu, &Key);
        if (!ResultContext) {
            Done = TRUE;
            break;
        }

        switch (Key.UnicodeChar){
        case CHAR_CARRIAGE_RETURN:
            MenuOption = ResultContext->MenuOption;
            break;
        case 'S':
        case 's':
            MenuOption = TIMEOUT_MENU_MODIFY;
            break;
        case 'D':
        case 'd':
            MenuOption = TIMEOUT_MENU_DELETE;
            break;
        default:
            MenuOption =  -1;
        } // end switch


        switch (MenuOption) {
        case TIMEOUT_MENU_MODIFY:
            ModifyBootTimeOut (FALSE);
            break;           
        case TIMEOUT_MENU_DELETE:
            if (ModifyBootTimeOut (TRUE) == EFI_SUCCESS) {
                //
                // Print message
                //
                EndRow = ScreenSize - TimeoutMenu.FooterHeight - 1;
                ST->ConOut->SetAttribute (ST->ConOut, TimeoutMenu.ReverseScreenAttribute);
                PrintAt(TimeoutMenu.Col,EndRow,L"'Timeout' variable cleared. Press any key...");
                WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
                ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
                ST->ConOut->SetAttribute (ST->ConOut, TimeoutMenu.ScreenAttribute);
            }
            break;                 
        case TIMEOUT_MENU_HELP:
            PrintMenuHelp(&TimeoutHelpMenu,TimeoutMenuHelpStr);           
            break;                 
        case TIMEOUT_MENU_EXIT:
            Done = TRUE;
            break;           
        default:
            break;
        }
    }

    FreeSpecificTimeoutMenuOption(&TimeoutMenu, SetContext);
    FreeSpecificTimeoutMenuOption(&TimeoutMenu, DeleteContext);
    FreeSpecificTimeoutMenuOption(&TimeoutMenu, TimeOutHelpContext);
    FreeSpecificTimeoutMenuOption(&TimeoutMenu, ExitContext);

    ST->ConOut->ClearScreen (ST->ConOut);
    return EFI_SUCCESS;
}

TIMEOUT_MENU_CONTEXT *
AllocateSpecialTimeoutOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN UINTN        Option
    )
{
    TIMEOUT_MENU_CONTEXT    *Context;

    Context = AllocateZeroPool (sizeof(TIMEOUT_MENU_CONTEXT));
    ASSERT (Context);
    Context->MenuOption = Option;

    Context->Menu = AllocateMenuOption (Menu, String, Attribute, Context);
    Context->Menu->ContextPrint = NULL;
    return Context;
}

VOID
FreeSpecificTimeoutMenuOption (
    IN EFI_MENU             *Menu,
    IN TIMEOUT_MENU_CONTEXT *Context
    )
{
    MENU_OPTION         *MenuOption;
    TIMEOUT_MENU_CONTEXT   *TimeoutContext;
    LIST_ENTRY          *List;   
    //
    // Delete the menu option's Context
    //
    List = Menu->Head.Flink;
    while (List != &Menu->Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        // get the next list in case we end up freeing the current one
        List = List->Flink;

        TimeoutContext = (TIMEOUT_MENU_CONTEXT *)MenuOption->Context;
        if ((TimeoutContext != NULL) && (TimeoutContext == Context)) {
            FreePool (MenuOption->Context);
            MenuOption->Context = NULL;
            RemoveEntryList (&MenuOption->Link);
            ASSERT(MenuOption->Signature == MENU_OPTION_SIGNATURE);
            FreePool (MenuOption);
        }
    }
}

EFI_STATUS
ModifyBootTimeOut (
    IN BOOLEAN  ClearVariable
    ) 
{
    
    CHAR16      InputString[MAX_CHAR];
    UINT16      *Timeout;
    UINTN       TimeoutSize,NewTimeOut;
    EFI_STATUS  Status;

    //
    // check if timeout variable is to be cleared
    //
    if (ClearVariable) {
        LibDeleteVariable(VarTimeout, &EfiGlobalVariable);
        return EFI_SUCCESS;
    }

    ST->ConOut->ClearScreen (ST->ConOut);

    PrintBanner();
    Print (L"%EChange Auto Boot TimeOut value%N\n");

    Timeout = LibGetVariableAndSize (VarTimeout, &EfiGlobalVariable, &TimeoutSize);   
    if (Timeout) {
        if (TimeoutSize != sizeof(UINT16)) {
            FreePool (Timeout);
            Timeout = NULL;
        }
    }

    Print(L"\n\n");

    if (Timeout)
        Print (L"    Current TimeOut is : %d seconds\n\n\n",*Timeout);
   
    Print(L"\n\n");

    do {
        BmntClearLine();
        Input (L"    New TimeOut in seconds (<= 65535) is : ", InputString, sizeof(InputString));
        if (StrLen(InputString) == 0) {
            NewTimeOut = 65536;
        } else if (InputString[0]<'0' || InputString[0]>'9') {
            NewTimeOut = 65536;
        } else {
            NewTimeOut = Atoi(InputString);
        }
    } while (NewTimeOut > 65535);

    Status = RT->SetVariable (
                        VarTimeout, &EfiGlobalVariable,
                        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                        sizeof(UINT16),&NewTimeOut
                        );
    ASSERT (!EFI_ERROR(Status));
    
    if(Timeout)
        FreePool (Timeout);

    return(Status);    
}

