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

    boot.c
    
Abstract:

    Boot Manger type menus



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "menu.h"
#include "legacyboot.h"

#define     DIRECTION_UP        1
#define     DIRECTION_DOWN      2

UINT16
AllocateBootOrder ( 
    IN  UINT16  *BootOrderList,
    IN  UINTN   BootOrderSize
    );

BOOLEAN
IsInVarBootOrder (  
    IN  UINT16  BootOption,
    IN  UINT16  *BootOrderList,
    IN  UINTN   BootOrderSize
    );

EFI_STATUS
SetNvramForBootMenu (
    IN  EFI_MENU    *Menu
    );

VOID
BootContextPrint (
     IN struct _MENU_OPTION *MenuOption,
     IN VOID                *Context,
     IN UINTN               *Row,
     IN UINTN               Column
     );

BOOT_MENU_CONTEXT *
AllocateSpecialBootOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN UINTN        Option  
    );

VOID
FreeBootMenu (
    IN EFI_MENU     *Menu
    );

VOID
FreeAllOfBootMenu (
    IN EFI_MENU     *Menu
    );

VOID
FreeSpecificBootMenuOption (
    IN EFI_MENU             *Menu,
    IN BOOT_MENU_CONTEXT    *Context
    );

BOOLEAN
PrintBootHelp(
    IN  CHAR16      *Description
    );

EFI_STATUS
DeleteAllBootOptions (
    OUT BOOLEAN *NVRAMNeedsUpdating
    );


EFI_STATUS
ResetBootNextOption (
    OUT BOOLEAN *NVRAMNeedsUpdating,
    IN  BOOLEAN Prompt
    );

EFI_STATUS
OrderOption(
    IN  BOOT_MENU_CONTEXT   *Context,
    IN  UINTN               Direction,
    OUT BOOLEAN             *UpdateNvram
    );

EFI_STATUS
UpdateLoadOption(
    IN  CHAR16  *LoadOptionStr, 
    IN  UINTN   SizeLoadOptionStr,
    OUT UINTN   *LoadOptionSize,
    IN  BOOLEAN EditMode);

    
EFI_MENU BootMenu = {
    MENU_SIGNATURE, NULL,  
    4, CON_UNSELECTED, CON_ACTIVE_SELECTION, 
    TRUE, 10, 
    NULL,
    NULL, 7, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};

EFI_MENU BootOrderHelpMenu = {
    MENU_SIGNATURE, NULL,  
    4, CON_UNSELECTED, CON_ACTIVE_SELECTION, 
    TRUE, 10, 
    L"Boot Order Menu Help Screen\n\n", 
    NULL, 7, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};

EFI_MENU DeleteHelpMenu = {
    MENU_SIGNATURE, NULL,  
    4, CON_UNSELECTED, CON_ACTIVE_SELECTION, 
    TRUE, 10, 
    L"Delete Menu Help Screen\n\n", 
    NULL, 7, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};

EFI_MENU BootNextHelpMenu = {
    MENU_SIGNATURE, NULL,  
    4, CON_UNSELECTED, CON_ACTIVE_SELECTION, 
    TRUE, 10, 
    L"BootNext Menu Help Screen\n\n", 
    NULL, 7, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};

UINTN
InitializeBootOrderMenuFromVariable (
    VOID
    )
{
    BOOT_MENU_CONTEXT       *Context;
    UINTN                   Index;
    UINT16                  BootString[10];
    UINT8                   *LoadOptionFromVar, *LoadOption;
    UINTN                   BootOptionSize;
    BOOLEAN                 BootNextFlag;

    UINT16                  *BootOrderList;
    UINTN                   BootOrderListSize;
    UINT16                  *BootNext;
    UINTN                   BootNextSize;


    BootOrderList = LibGetVariableAndSize (VarBootOrder, &EfiGlobalVariable, &BootOrderListSize);
    BootNext = LibGetVariableAndSize (VarBootNext, &EfiGlobalVariable, &BootNextSize);
    if (BootNext) {
        if (BootNextSize != sizeof(UINT16)) {
            FreePool (BootNext);
            BootNext = NULL;
        }
    }

    BootMenu.BottomOfPage = NULL;
    InitializeListHead (&BootMenu.Head);
    for (Index = 0; Index < BootOrderListSize/sizeof(UINT16); Index++) {
        SPrint (BootString, sizeof(BootString), VarBootOption, BootOrderList[Index]);
        LoadOptionFromVar = LibGetVariableAndSize (BootString, &EfiGlobalVariable, &BootOptionSize);
        if (!LoadOptionFromVar) {
            continue;
        }

        LoadOption = AllocateZeroPool(BootOptionSize);
        if (!LoadOption) {
            continue;
        }

        CopyMem(LoadOption,LoadOptionFromVar,BootOptionSize);
        FreePool(LoadOptionFromVar);

        if (BootNext) {
            BootNextFlag = *BootNext == BootOrderList[Index];
        } else {
            BootNextFlag = FALSE;
        }
        Context = BuildBootMenuContext (
                                LoadOption, 
                                BootOptionSize, 
                                BootOrderList[Index], 
                                BootString,
                                BootNextFlag,
                                FALSE
                                );
    }

    if(BootNext) {
        FreePool (BootNext);
    }
    if(BootOrderList) {
        FreePool (BootOrderList);
    }
    return Index;
}

VOID
FreeBootOrderMenuOfVariables (
    VOID
    )
{
    FreeAllOfBootMenu(&BootMenu);
}

    
BOOT_MENU_CONTEXT *
BuildBootMenuContext (
    IN  CHAR8   *BootOption,
    IN  UINTN   BootOptionSize,
    IN  UINT16  BootOptionNumber,
    IN  CHAR16  *BootString,
    IN  BOOLEAN BootNext,
    IN  BOOLEAN Modified
    )
{
    BOOT_MENU_CONTEXT       *Context;
    UINTN                   SizeDevicePath;
    UINT8                   *LoadOption;
    UINT8                   *End;
    UINT32                  FilePathListLength;
    UINTN                   StringSize;
    
    LoadOption = BootOption;

    Context = AllocateZeroPool (sizeof(BOOT_MENU_CONTEXT));
    if (!Context) {
        return NULL;
    }

    Context->OptionNumber = BootOptionNumber;
    Context->IsBootNext = BootNext;
    Context->LoadOptionModified = Modified;

    End = LoadOption + BootOptionSize;
    Context->RawOption = BootOption;
    Context->RawOptionSize = BootOptionSize;
    Context->Attributes = *(UINT32 *) LoadOption;
    Context->IsActive = (BOOLEAN)(Context->Attributes & LOAD_OPTION_ACTIVE);
    LoadOption += sizeof(UINT32);
    
    FilePathListLength = *(UINT16 *) LoadOption;
    LoadOption += sizeof(UINT16);

    StringSize = StrSize((CHAR16 *)LoadOption);
    Context->Description = AllocateZeroPool (StringSize);
    CopyMem (Context->Description, (CHAR16 *)LoadOption, StringSize);
    
    LoadOption += StringSize;

    SizeDevicePath = DevicePathSize ((EFI_DEVICE_PATH *)LoadOption);
    if (SizeDevicePath > FilePathListLength) {
        SizeDevicePath = FilePathListLength;
    }
    if ((LoadOption + SizeDevicePath) > End) {
        Context->IsActive = FALSE;
        SizeDevicePath = End - LoadOption;
    }

    //
    // To avoid alignment problems copy to an aligned buffer
    //
    Context->FilePath = AllocatePool (SizeDevicePath);
    CopyMem (Context->FilePath, (EFI_DEVICE_PATH *)LoadOption, SizeDevicePath);
    Context->FilePathString = DevicePathToStr (Context->FilePath);
    LoadOption += SizeDevicePath;

    Context->LoadOptionsSize = BootOptionSize - sizeof(UINT32) - sizeof(UINT16) - StringSize - SizeDevicePath;
    Context->LoadOptions = AllocatePool(Context->LoadOptionsSize);
    CopyMem (Context->LoadOptions, LoadOption, Context->LoadOptionsSize);

    StrCpy (Context->BootString, BootString);

    Context->MenuOption = BOOT_MENU_SELECTION;
    Context->Menu = AllocateMenuOption (&BootMenu, Context->Description, CON_UNSELECTED, Context);
    Context->Menu->ContextPrint = BootContextPrint;
    
    return Context;
}


EFI_STATUS
Bmnt_DisplayBootOrderMenu (
    VOID
    ) 
{
    BOOT_MENU_CONTEXT       *ResultContext;
    BOOT_MENU_CONTEXT       *PreviousResultContext;
    BOOT_MENU_CONTEXT       *SaveNvramContext;
    BOOT_MENU_CONTEXT       *BootHelpContext;
    BOOT_MENU_CONTEXT       *BootExitContext;
    BOOLEAN                 NVRAMNeedsUpdating;
    UINTN                   MenuOption;
    EFI_INPUT_KEY           Key;
    EFI_STATUS              Status;
    BOOLEAN                 Done;
    UINTN                   Direction;
    CHAR16                  *Header = L"%EChange boot order.  Select an Operation\n\n";

    // 
    // initialize header text
    //
    FreeBootOrderMenuOfVariables();
    InitializeBootOrderMenuFromVariable();

    BootMenu.Header = Header;

    SaveNvramContext = 
        AllocateSpecialBootOption (&BootMenu, L"Save Settings to NVRAM", CON_UNSELECTED, BOOT_MENU_SAVE_NVRAM);
    BootHelpContext =
        AllocateSpecialBootOption (&BootMenu, L"Help", CON_UNSELECTED, BOOT_MENU_HELP);
    BootExitContext =
        AllocateSpecialBootOption (&BootMenu, L"Exit", CON_UNSELECTED, BOOT_MENU_EXIT);

    BootMenu.Selection = NULL;
    NVRAMNeedsUpdating = FALSE;
    for (Done = FALSE, ResultContext = NULL; !Done;) {
        PreviousResultContext = ResultContext;
        ResultContext = MenuDisplay (&BootMenu, &Key);
        if (!ResultContext) {
            Done = TRUE;
            break;
        }
        
        //
        // default move option down
        //
        Direction = DIRECTION_DOWN; 

        switch (Key.UnicodeChar){
        case CHAR_CARRIAGE_RETURN:
            if (ResultContext->MenuOption != BOOT_MENU_SELECTION) {
                MenuOption = ResultContext->MenuOption;
            } else {
                MenuOption = -1;
            }
            break;
        case 'U':
        case 'u':
            Direction = DIRECTION_UP;
        case 'D':
        case 'd':
            if (ResultContext->MenuOption == BOOT_MENU_SELECTION) {
                MenuOption = BOOT_MENU_SELECTION;
            } else {
                MenuOption = -1;
            }
            break;
        default:
            MenuOption =  -1;
        } // end switch

        if (MenuOption == BOOT_MENU_EXIT) {
            if (NVRAMNeedsUpdating) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 2, L"NVRAM Not updated. Save NVRAM? [Y to save, N to ignore]");
                WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
                ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
                if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
                    MenuOption = BOOT_MENU_SAVE_NVRAM;
                }
            }
            Done = TRUE;
        }

        switch (MenuOption) {
        case BOOT_MENU_EXIT:
            break;           
        case BOOT_MENU_HELP:
            PrintMenuHelp(&BootOrderHelpMenu,BootOrderMenuHelpStr);           
            break;                 
        case BOOT_MENU_SAVE_NVRAM:
            Status = SetNvramForBootMenu (&BootMenu);
            if (EFI_ERROR(Status)) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 3, L"NVRAM update failed");
            } else {
                NVRAMNeedsUpdating = FALSE;
            }
            break;
        case BOOT_MENU_SELECTION:
            //
            // Toggle selection
            //
            OrderOption(ResultContext,Direction,&NVRAMNeedsUpdating);
            break;
        default:
            break;
        }
    }

    FreeSpecificBootMenuOption(&BootMenu, SaveNvramContext);
    FreeSpecificBootMenuOption(&BootMenu, BootHelpContext);
    FreeSpecificBootMenuOption(&BootMenu, BootExitContext);
    ST->ConOut->ClearScreen (ST->ConOut);
    return EFI_SUCCESS;
}

EFI_STATUS
OrderOption(
    IN  BOOT_MENU_CONTEXT   *Context,
    IN  UINTN               Direction,
    OUT BOOLEAN             *UpdateNvram
    )
{
    EFI_INPUT_KEY                   Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;
    UINTN                           EndRow;
    UINTN                           MaxColumn, ScreenSize;
    MENU_OPTION                     *MenuOption;
    MENU_OPTION                     *MenuOption_Next;
    MENU_OPTION                     *MenuOption_Prev;
    BOOT_MENU_CONTEXT               *BootContext;
    BOOT_MENU_CONTEXT               *BootContext_Next;
    BOOT_MENU_CONTEXT               *BootContext_Prev;
    LIST_ENTRY                      *List;   
    LIST_ENTRY                      *List_Next;   
    LIST_ENTRY                      *List_Prev;   


    ConOut = ST->ConOut;
    ConIn  = ST->ConIn;

    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
    EndRow = ScreenSize - BootMenu.FooterHeight - 1;
     
    List = BootMenu.Head.Flink;
    while (List != &BootMenu.Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        // get the next & previous members 
        List_Next = List->Flink;
        List_Prev = List->Blink;

        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        
        if ((BootContext != NULL) && (BootContext == Context)) {
            
            //
            // if direction is down and next one is not BOOT_MENU_SELECTION, then
            // we cannot move down
            //
            if (Direction == DIRECTION_DOWN) {
                if (List_Next != &BootMenu.Head) {
                    MenuOption_Next = CR(List_Next, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
                    BootContext_Next = (BOOT_MENU_CONTEXT *)MenuOption_Next->Context;
                    if(BootContext_Next->MenuOption == BOOT_MENU_SELECTION) {
                        // list_next before List    
                        SwapListEntries(List,List_Next);
                        *UpdateNvram = TRUE;
                        break;
                    } else {
                        ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
                        PrintAt(BootMenu.Col,EndRow,L"Selected option bottom of list. Cannot move down. Press any key...");
                        WaitForSingleEvent (ConIn->WaitForKey, 0);
                        ConIn->ReadKeyStroke (ConIn, &Key);
                        ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
                    } 
                } 
            } 

            //
            // if direction is up and above one is not BOOT_MENU_SELECTION, then
            // we cannot move down
            //
            if (Direction == DIRECTION_UP) {
                if (List_Prev != &BootMenu.Head) {
                    MenuOption_Prev = CR(List_Prev, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
                    BootContext_Prev = (BOOT_MENU_CONTEXT *)MenuOption_Prev->Context;
                    if(BootContext_Prev->MenuOption == BOOT_MENU_SELECTION) {
                        // list before List_Prev    
                        SwapListEntries(List_Prev,List);
                        *UpdateNvram = TRUE;
                        break;
                    } 
                } else {
                    ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
                    PrintAt(BootMenu.Col,EndRow,L"Selected option top of list. Cannot move up. Press any key...");
                    WaitForSingleEvent (ConIn->WaitForKey, 0);
                    ConIn->ReadKeyStroke (ConIn, &Key);
                    ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
                } 
            } 
        } 
        List = List->Flink;
    } 
    return EFI_SUCCESS;
}



VOID
BootContextPrint (
     IN MENU_OPTION         *MenuOption,
     IN BOOT_MENU_CONTEXT   *Context,
     IN UINTN               *Row,
     IN UINTN               Column
     )
{
    CHAR16  *OptionString;
    CHAR16  *OptionDevicePath;
    CHAR16  *VariableName;
    CHAR16  *LoadOption,*TempLoadOption;
    CHAR8   *AsciiChar;


    PrintAt (Column, *Row+1, L"%-.80s", L"");
    PrintAt (Column, *Row+2, L"%-.80s", L"");
    PrintAt (Column, *Row+3, L"%-.80s", L"");
    PrintAt (Column, *Row+4, L"%-.80s", L"");

    LoadOption = NULL;
    if (Context->MenuOption == BOOT_MENU_SELECTION) {
        OptionString = Context->Description;
        OptionDevicePath = Context->FilePathString;
        VariableName = &Context->BootString[0];
        if (Context->LoadOptionsSize) {
            LoadOption = Context->LoadOptions;
        }
    } else {
        OptionString = L"";
        OptionDevicePath = L"";
        VariableName = NULL;
    }

    *Row += 1;
    PrintAt (Column, *Row, L"%-.80s", OptionDevicePath);
    *Row += 1;
    if (VariableName) {
        PrintAt (Column, *Row, L"%-.80s", VariableName);
        *Row += 1;
    } 
    
    //
    // Crude check. Currently loadoption can be either
    // ascii or Unicode string only..
    // hence test 2nd byte to see if it is non-zero
    // if non-zero then string is ascii else, unicode
    // Bugbug: NEED TO GET  A BETTER METHOD (like a header)
    // to figure out type of data

    if (LoadOption) {
        AsciiChar = (UINT8 *)LoadOption;
        AsciiChar++;
        if(*AsciiChar) {
            // ascii string. hence form unicode string
            LoadOption = AllocateZeroPool(Context->LoadOptionsSize * sizeof(CHAR16));
            ASSERT(LoadOption);
            TempLoadOption = LoadOption;
            AsciiChar = (UINT8 *)Context->LoadOptions;
            for ( ; *AsciiChar != '\0';AsciiChar++) {
                *LoadOption = *AsciiChar;
                LoadOption++;
            }
            *LoadOption = '\0';
            PrintAt(Column, *Row,L"%-.80s",TempLoadOption);
            FreePool(TempLoadOption);
        } else {
            // unicode string
            PrintAt(Column, *Row,L"%-.80s",LoadOption);
        }
        *Row += 1;
    }

    if(Context->IsBootNext) {
        PrintAt (Column, *Row, L"%-.80s", L"BootNext Option");
    } else {
        PrintAt (Column, *Row, L"%-.80s", L"");
    }
    *Row += 1;
}

BOOT_MENU_CONTEXT *
AllocateSpecialBootOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN UINTN        Option
    )
{
    BOOT_MENU_CONTEXT    *Context;

    Context = AllocateZeroPool (sizeof(BOOT_MENU_CONTEXT));
    ASSERT (Context);
    Context->MenuOption = Option;

    Context->Menu = AllocateMenuOption (Menu, String, Attribute, Context);
    Context->Menu->ContextPrint = BootContextPrint;
    return Context;
}


EFI_STATUS
SetNvramForBootMenu (
    IN  EFI_MENU    *Menu
    )
{
    MENU_OPTION             *MenuOption;
    BOOT_MENU_CONTEXT       *BootContext;
    LIST_ENTRY              *List;
    EFI_STATUS              Status;
    BOOLEAN                 WriteBootOption, SavedBootNext;

    UINT16                  *VarBootOrderList;
    UINTN                   VarBootOrderListSize;
    UINT16                  *BootOrderList;
    UINTN                   BootOrderListSize,SaveListSize;


    VarBootOrderList = LibGetVariableAndSize (VarBootOrder, &EfiGlobalVariable, &VarBootOrderListSize);
    BootOrderList = AllocateZeroPool (0x1024);
    
    BootOrderListSize = 0;
    SavedBootNext = FALSE;

    for (List = Menu->Head.Flink ;List != &Menu->Head; List = List->Flink) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        if (BootContext == NULL) {
            continue;
        }
        // we want to add old ones only if we read a valid rawoption data for this
        // boot option
        if ((BootContext->OptionNumber != NEWADDITION_OPTIONNUMBER) && (BootContext->RawOption)) {
            BootOrderList[BootOrderListSize] = BootContext->OptionNumber;
            BootOrderListSize++;
        }

        if (BootContext->OptionNumber == NEWADDITION_OPTIONNUMBER) {
            BootOrderList[BootOrderListSize] = NEWADDITION_OPTIONNUMBER;
            BootOrderListSize++;
        }
    }

    SaveListSize = BootOrderListSize;
    BootOrderListSize = 0;
    for (List = Menu->Head.Flink ;List != &Menu->Head; List = List->Flink) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        if (BootContext == NULL) {
            continue;
        }
        // add any new boot option
        if (BootContext->OptionNumber == NEWADDITION_OPTIONNUMBER) {
            BootContext->OptionNumber = AllocateBootOrder (BootOrderList, SaveListSize);
            BootOrderList[BootOrderListSize] = BootContext->OptionNumber;
            SPrint (BootContext->BootString, sizeof(BootContext->BootString), VarBootOption, BootContext->OptionNumber);
        }

        if (BootContext->LoadOptionModified) {
            // may need to update MenuOption description as it might changed as part of
            // loadoptions
            FreePool(MenuOption->Description);
            MenuOption->Description = StrDuplicate(BootContext->Description);
            Status = RT->SetVariable (
                        BootContext->BootString, &EfiGlobalVariable,
                        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                        BootContext->RawOptionSize, (VOID*) (BootContext->RawOption)
                        );
            //
            // BugBug: Should not assert after code is debugged
            //
            ASSERT (!EFI_ERROR(Status));
        }

        //
        // Store BootNext option number
        //
        if(BootContext->IsBootNext) {
            Status = RT->SetVariable (
                        VarBootNext, &EfiGlobalVariable,
                        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                        sizeof(BootContext->OptionNumber), &(BootContext->OptionNumber)
                        );
            ASSERT (!EFI_ERROR(Status));
            SavedBootNext = TRUE;
        }
        BootOrderListSize++;
    }
    
    if (SaveListSize*sizeof(UINT16) != VarBootOrderListSize) {
        WriteBootOption = TRUE;
    } else {
        if (VarBootOrderList) {
            WriteBootOption = (CompareMem (VarBootOrderList, BootOrderList, VarBootOrderListSize) != 0);
        } else {
            //
            // No NVRAM variable case
            //
            WriteBootOption = TRUE;
        }
    }
    
    if (WriteBootOption) {
        Status = RT->SetVariable (
                    VarBootOrder, &EfiGlobalVariable,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    SaveListSize*sizeof(UINT16), BootOrderList
                    );        
    }      

    if (!SavedBootNext) {
        LibDeleteVariable (VarBootNext, &EfiGlobalVariable);
    }
    if (VarBootOrderList) {
        FreePool (VarBootOrderList);
    }
    if (BootOrderList) {
        FreePool (BootOrderList);
    }
    return EFI_SUCCESS;
}

UINT16
AllocateBootOrder ( 
    IN  UINT16  *BootOrderList,
    IN  UINTN   BootOrderSize
    )
{
    UINT16  BootOption;
    UINTN   Index;
    BOOLEAN Match;

    for (BootOption = 0; BootOption <= 0xffff; BootOption++) {
        for (Index = 0, Match = FALSE; (Index < BootOrderSize) && !Match; Index++) {
            Match = (BootOrderList[Index] == BootOption);
        }
        if (!Match) {
            return BootOption;
        }
    }
    return 0xffff;
}


BOOLEAN
IsInVarBootOrder (  
    IN  UINT16  BootOption,
    IN  UINT16  *BootOrderList,
    IN  UINTN   BootOrderSize
    )
{
    UINTN   Index;

    for (Index = 0; Index < BootOrderSize; Index++) {
        if (BootOrderList[Index] == BootOption) {
            return TRUE;
        }
    }
    return FALSE;
}

VOID
FreeAllOfBootMenu (
    IN EFI_MENU     *Menu
    )
{
    MENU_OPTION         *MenuOption;
    BOOT_MENU_CONTEXT   *BootContext;
    LIST_ENTRY          *List;   

    //
    // Delete the menu option's Context
    //
    List = Menu->Head.Flink;
    while (List != &Menu->Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        if (BootContext != NULL) {
            if (BootContext->FilePathString) {
                FreePool (BootContext->FilePathString);
            }
            if (BootContext->RawOption) {
                FreePool (BootContext->RawOption);
            }
            FreePool (MenuOption->Context);
        }
        
        List = List->Flink;
    }
    MenuFree (Menu);
}

VOID
FreeSpecificBootMenuOption (
    IN EFI_MENU             *Menu,
    IN BOOT_MENU_CONTEXT    *Context
    )
{
    MENU_OPTION         *MenuOption;
    BOOT_MENU_CONTEXT   *BootContext;
    LIST_ENTRY          *List;   

    //
    // Delete the menu option's Context
    //
    List = Menu->Head.Flink;
    while (List != &Menu->Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        // get the next list in case we end up freeing the current one
        List = List->Flink;

        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        if ((BootContext != NULL) && (BootContext == Context)) {
            if (BootContext->FilePathString) {
                FreePool (BootContext->FilePathString);
            }
            if (BootContext->Description) {
                FreePool (BootContext->Description);
            }
            if (BootContext->RawOption) {
                FreePool (BootContext->RawOption);
            }
            FreePool (MenuOption->Context);
            MenuOption->Context = NULL;
            FreePool (MenuOption->Description);
            MenuOption->Description = NULL;
            RemoveEntryList (&MenuOption->Link);
            ASSERT(MenuOption->Signature == MENU_OPTION_SIGNATURE);
            FreePool (MenuOption);
        }
    }
}

EFI_STATUS
FindBootOption (
    IN EFI_MENU             *Menu,
    IN OUT LIST_ENTRY       **List,
    IN  EFI_DEVICE_PATH     *DevicePath,
    OUT BOOT_MENU_CONTEXT   **ReturnBootContext
    )
{
    MENU_OPTION         *MenuOption;
    BOOT_MENU_CONTEXT   *BootContext;

    *ReturnBootContext = NULL;
    while (*List != &Menu->Head) {
        MenuOption = CR(*List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        if (BootContext != NULL) {
            if (BootContext->FilePath) {
                if (MatchDevicePaths (DevicePath, BootContext->FilePath)) {
                    *ReturnBootContext = BootContext;
                    *List = (*List)->Flink;
                    return EFI_SUCCESS;
                }
            }
        }
        *List = (*List)->Flink;
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS
BootFromFileContext (
    IN  FILE_MENU_CONTEXT   *Context
    )
{
    EFI_INPUT_KEY       Key;
    BOOT_MENU_CONTEXT   *BootOption;
    EFI_STATUS          Result;
    EFI_STATUS          Status;
    UINTN               ScreenSize, MaxColumn, EndRow;
    LIST_ENTRY          *List;

    ST->ConOut->ClearScreen (ST->ConOut);
    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
    EndRow = ScreenSize - BootMenu.FooterHeight - 1;

    PrintEfiFileInfo (Context);
    List = BootMenu.Head.Flink;
    Result = FindBootOption (&BootMenu, &List, Context->DevicePath, &BootOption);
    if (!EFI_ERROR(Result)) {
        Print (L"    Boot%04x: %s\n\n", BootOption->OptionNumber, BootOption->FilePathString);
    }
    Status = BootTheOption (Context, BootOption);
    if (!EFI_ERROR(Status) || Status == EFI_ALREADY_STARTED) {
        return EFI_SUCCESS;
    } else {
        ST->ConOut->SetAttribute (ST->ConOut, BootMenu.ReverseScreenAttribute);
        PrintAt(BootMenu.Col,EndRow,L"Load failed [%r]. Press any key to continue..", Status);
        WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
        ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
        ST->ConOut->SetAttribute (ST->ConOut, BootMenu.ScreenAttribute);
        return Status;
    }
}


EFI_STATUS
ConvertFileContextToBootOption (
    IN  FILE_MENU_CONTEXT   *Context
    )
{
    EFI_INPUT_KEY                  Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE   *ConOut;
    SIMPLE_INPUT_INTERFACE         *ConIn;   
    EFI_STATUS                     Result;
    UINT8                          *RawOption;
    CHAR16                         *Str;
    EFI_DEVICE_PATH                *DevicePath;
    UINTN                          Size,ScreenSize, MaxColumn, EndRow;
    CHAR16                         InputString[MAX_CHAR];
    BOOT_MENU_CONTEXT              *BootOption;
    BOOT_MENU_CONTEXT              *NextBootOption;
    BOOLEAN                        Edit, Exit, Next;
    CHAR16                         LoadOptionStr[MAX_CHAR];
    UINTN                          LoadOptionSize;
    UINT8                          *LoadOptionPtr;
    CHAR16                         *LoadOption,*TempLoadOption;
    CHAR8                          *AsciiChar;
    UINTN                          SizeDevicePath;
    LIST_ENTRY                     *List;
    LIST_ENTRY                     *NextList;
    EFI_STATUS                     NextResult;

    ConOut = ST->ConOut;
    ConIn  = ST->ConIn;
    ConOut->ClearScreen (ST->ConOut);
    PrintEfiFileInfo (Context);

    List = BootMenu.Head.Flink;
    Edit = FALSE;
    Exit = FALSE;
    Next = FALSE;

    while (TRUE) {
      Result = FindBootOption ( &BootMenu, &List, Context->DevicePath, &BootOption);
      if (!EFI_ERROR(Result)) {

        Edit = FALSE;
        Exit = FALSE;
        Next = FALSE;

        NextList = List;
        NextResult = FindBootOption ( &BootMenu, &NextList, Context->DevicePath, &NextBootOption);
        if (!EFI_ERROR(NextResult)) {
          Next = TRUE;
        }

        ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
        ConOut->ClearScreen (ST->ConOut);
        PrintEfiFileInfo (Context);
        Print (L"    Boot%04x: %s\n", BootOption->OptionNumber, BootOption->FilePathString);
        while (TRUE) {
          BmntClearLine();
          ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
          if (!Next) {
            Print(L" Edit this Boot Option or Create a new one? [E-Edit N-New]: ");
          } else {
            Print(L" Edit/Skip this Boot Option or Create a new one? [E-Edit S-Skip N-New]: ");
          }
          WaitForSingleEvent (ConIn->WaitForKey, 0);
          ConIn->ReadKeyStroke (ConIn, &Key);
         
          if (Key.UnicodeChar == 'E' || Key.UnicodeChar == 'e') {
            Edit = TRUE;
            Exit = TRUE;
            Print(L" Edit");
            break;
          }
            
          if (Key.UnicodeChar == 'N' || Key.UnicodeChar == 'n') {
            Exit = TRUE;
            Print(L" New");
            break;
          }

          if ( Next && (Key.UnicodeChar == 'S' || Key.UnicodeChar == 's')) {
            Exit = FALSE;
            Print(L"Skip");
            break;
          }
        }
        if (Exit) {
          break;
        }
      } else {
        break;
      }
    }
    
    //
    // Let's make a boot option
    //
    
    ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);

    //
    // Update Description Field
    //
    if (Edit) {
        Print (L"\n\n    Current Description-->%s", BootOption->Description);
    }
    Print(L"\n");
    do {
        BmntClearLine();
        ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
        Print (L"    Enter New Description: ");
        ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
        Print(L" ");
        do {
            Input (L"", InputString, sizeof(InputString));
        } while (StrLen(InputString) == 0);
    } while (InputString[0] == ' ');
    
    //
    // Update BootOption Field
    //
    LoadOption = NULL;
    if (Edit) {
        if (BootOption->MenuOption == BOOT_MENU_SELECTION) {
            if (BootOption->LoadOptionsSize)
                LoadOption = BootOption->LoadOptions;
        } 
   
        //
        // Crude check. Currently loadoption can be either
        // ascii or Unicode string only..
        // hence test 2nd byte to see if it is non-zero
        // if non-zero then string is ascii else, unicode
        // Bugbug: NEED TO GET  A BETTER METHOD (like a header)
        // to figure out type of data

        if (LoadOption) {
            AsciiChar = (UINT8 *)LoadOption;
            AsciiChar++;
            if(*AsciiChar) {
                // ascii string. hence form unicode string
                LoadOption = AllocateZeroPool(BootOption->LoadOptionsSize * sizeof(CHAR16));
                ASSERT(LoadOption);
                TempLoadOption = LoadOption;
                AsciiChar = (UINT8 *)BootOption->LoadOptions;
                for ( ; *AsciiChar != '\0';AsciiChar++) {
                    *LoadOption = *AsciiChar;
                    LoadOption++;
                }
                *LoadOption = '\0';
                Print (L"\n\n    Current BootOption-->%-s", TempLoadOption);
                FreePool(TempLoadOption);
            } else {
                // unicode string
                Print (L"\n\n    Current BootOption-->%-s", LoadOption);
            }
        } 
    } 

    // Get the new load option
    LoadOptionSize = 0;
    UpdateLoadOption(LoadOptionStr, sizeof(LoadOptionStr), &LoadOptionSize, FALSE);
    
    
    //
    // save the changes
    //
    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
    EndRow = ScreenSize - BootMenu.FooterHeight - 1;
    Exit = FALSE;
    while (!Exit) {
        ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
        PrintAt(BootMenu.Col,EndRow,L"Save changes to NVRAM [Y-Yes N-No]: ");
        WaitForSingleEvent (ConIn->WaitForKey, 0);
        ConIn->ReadKeyStroke (ConIn, &Key);
        ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
        
        if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
        
          //
          // Update BootOption Description
          //
          if (Edit) {
            FreePool (BootOption->Description);
            BootOption->Description = NULL;
            if(!StrLen(InputString)) {
              StrCpy(InputString, L"No Description");
            }
            BootOption->Description = AllocatePool (StrSize(InputString));
            StrCpy (BootOption->Description, InputString);
          }

          //
          // Update BootOption's Load Option
          //
          if (Edit) {
            FreePool (BootOption->LoadOptions);
            BootOption->LoadOptions = NULL;
            BootOption->LoadOptions = AllocateZeroPool(LoadOptionSize);
            CopyMem(BootOption->LoadOptions,LoadOptionStr,LoadOptionSize);
            BootOption->LoadOptionsSize = LoadOptionSize;
          }

          //
          // build a new menuoption or update the current one
          // allocate memory and copy all the options
          //
          SizeDevicePath = DevicePathSize (Context->DevicePath);
          Size = sizeof(UINT32) + sizeof(UINT16) + StrSize(InputString) + SizeDevicePath + LoadOptionSize;
          RawOption = AllocatePool (Size);
          *(UINT32 *)RawOption = LOAD_OPTION_ACTIVE;

          *(UINT16 *)(RawOption + sizeof(UINT32)) = (UINT16)SizeDevicePath;

          Str = (CHAR16 *)(RawOption + sizeof(UINT32) + sizeof(UINT16));
          StrCpy (Str, InputString);

          DevicePath = (EFI_DEVICE_PATH *)(Str + StrLen(Str) + 1);
          CopyMem (DevicePath, Context->DevicePath, DevicePathSize(Context->DevicePath));

          LoadOptionPtr = (UINT8 *) ((UINT8 *)(DevicePath) + DevicePathSize(Context->DevicePath));
          CopyMem(LoadOptionPtr,LoadOptionStr,LoadOptionSize);

          if(Edit) {
              FreePool(BootOption->RawOption);
              BootOption->RawOption = RawOption;
              BootOption->RawOptionSize = Size;
              BootOption->LoadOptionModified = TRUE;
          } else {
              // build the menu option
              BootOption = BuildBootMenuContext (RawOption, Size, NEWADDITION_OPTIONNUMBER, L"", FALSE, TRUE);
          }

          Result = SetNvramForBootMenu (&BootMenu);
            
          if (EFI_ERROR(Result)) {
              BmntClearLine();
              ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
              PrintAt(BootMenu.Col,EndRow,L"NVRAM update failed. Press any key to continue.... ");
              WaitForSingleEvent (ConIn->WaitForKey, 0);
              ConIn->ReadKeyStroke (ConIn, &Key);
              ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
          }
          Exit = TRUE;
        }
        if (Key.UnicodeChar == 'N' || Key.UnicodeChar == 'n') {
            Exit = TRUE;
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS
UpdateLoadOption(
    IN  CHAR16  *LoadOptionStr, 
    IN  UINTN   SizeLoadOptionStr,
    OUT UINTN   *LoadOptionSize,
    IN  BOOLEAN EditMode)
{
    EFI_INPUT_KEY                   Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;   
    BOOLEAN                         Ascii;
    CHAR16                          *TempStr,*TempStr1;
    CHAR8                           *AsciiChar;

    ConOut = ST->ConOut;
    ConIn  = ST->ConIn;
    
    *LoadOptionSize = 0;   
    Print (L"\n    New BootOption Data. ASCII/Unicode strings only, with max of %d characters\n", MAX_CHAR);
    
    Ascii = FALSE;
    
    BmntClearLine();
    ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
    Print (L"    Enter BootOption Data Type [A-Ascii U-Unicode N-No BootOption] : ");
    WaitForSingleEvent (ConIn->WaitForKey, 0);
    ConIn->ReadKeyStroke (ConIn, &Key);

    switch (Key.UnicodeChar) {
    case 'N':
    case 'n':
        Print(L" None\n");
        ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);   
        return (EFI_SUCCESS);
    case 'A':
    case 'a':
        Ascii = TRUE;
        Print(L" Ascii");
        Print (L"\n    Enter BootOption Data [Data will be stored as Ascii string]:");
        break;
    case 'U':
    case 'u':
    default :
        Print (L" Unicode");
        Print (L"\n    Enter BootOption Data [Data will be stored as Unicode string]: ");
        break;
    } 

    ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);   
    Print (L"\n");   

    TempStr = (CHAR16 *) AllocateZeroPool(SizeLoadOptionStr);
    ASSERT(TempStr);
    Input (L" ", TempStr, SizeLoadOptionStr);

    if (Ascii) {
        AsciiChar = (CHAR8 *)LoadOptionStr;
        TempStr1 = TempStr;
        for(;*TempStr1 != CHAR_NULL;TempStr1++) {
            // Convert to ASCII
            *AsciiChar = (CHAR8)*TempStr1;
            *LoadOptionSize += 1;
            AsciiChar++;
        }
        
        // Check to see if any boot options were added
        // LoadOptionSize would be non-zero if something was entered at input
        // if something was added then increment LoadOptionSize by 1 so that 
        // we can capture the Null string terminator character as well
        if (*LoadOptionSize) {
            *LoadOptionSize += 1;
            *AsciiChar = '\0';
        }
    } else {
        StrCpy(LoadOptionStr,TempStr);   
        *LoadOptionSize = StrSize(LoadOptionStr);
    }

    FreePool(TempStr);

    return EFI_SUCCESS;
}


EFI_STATUS
BootTheOption (
    IN  FILE_MENU_CONTEXT   *Context,
    IN  BOOT_MENU_CONTEXT   *BootOption
    )
{
    EFI_STATUS              Status;
    EFI_DEVICE_PATH         *DevicePath;
    EFI_HANDLE              ImageHandle;
    LEGACY_BOOT_INTERFACE   *BootLegacy;
    
    DevicePath = Context->DevicePath;
    if (DevicePath->Type == BBS_DEVICE_PATH && 
        DevicePath->SubType == BBS_BBS_DP    ) {
        //
        // If the device path starts with a BBS device path entry
        //  call into platform code to boot the legacy PC AT way
        //
        LibLocateProtocol (&LegacyBootProtocol, &BootLegacy);
        if (BootLegacy) {
            Status = BootLegacy->BootIt (DevicePath);
            if (EFI_ERROR(Status)) {
                Print (L"Start of %s failed: %r\n", BootOption->Description, Status);
                BootPause ();
                return Status;
            }
        } else {
            //
            // Booting from legacy devices not support on this platform
            //
            ASSERT(FALSE);
            return EFI_UNSUPPORTED;
        }
    }

    if (Context->IsRemovableMedia) {
        Status = DefaultBootTheBootOption (Context, BootOption, &ImageHandle);
        if (EFI_ERROR(Status)) {
            return Status;
        }
    } else {
        Status = BS->LoadImage(TRUE, GlobalImageHandle, DevicePath, NULL, 0, &ImageHandle);
        if (EFI_ERROR(Status)) {
            return Status;
        }
    }
    
    //
    // Before calling the image, enable the Watchdog Timer for the 5 Minute period
    //

    Status = BS->StartImage (ImageHandle, 0, NULL);

    if (EFI_ERROR(Status)) {
    }
    return Status;
}

EFI_STATUS
DefaultBootTheBootOption (
    IN  FILE_MENU_CONTEXT   *Context,
    IN  BOOT_MENU_CONTEXT   *BootOption,
    OUT EFI_HANDLE          *ImageHandle
    )
{
    UINTN                   PathSize;
    EFI_STATUS              Status;
    UINTN                   NoHandles, Index;
    EFI_HANDLE              *Handles;
    EFI_HANDLE              Handle;
    EFI_FILE_HANDLE         EfiBoot, Dir;
    UINTN                   FileInfoSize, BufferSize;
    EFI_FILE_INFO           *FileInfo;
    CHAR16                  *FileName;    
    EFI_DEVICE_PATH         *FilePath, *DevicePath;
    EFI_LOADED_IMAGE        *ImageInfo;
    EFI_BLOCK_IO            *BlkIo;
    VOID                    *Buffer;

    EfiBoot = NULL;
    Dir = NULL;

    ASSERT (Context->DevicePath);
    PathSize = DevicePathSize(Context->DevicePath) - sizeof(EFI_DEVICE_PATH);

    FileInfoSize = sizeof(EFI_FILE_HANDLE) + 1024;
    FileInfo = AllocatePool (FileInfoSize);
    ASSERT (FileInfo);

    //
    // Read a block just in case we booted without the media in place
    // This will allow for the FileSystemProtocol to be found when attempting
    // to boot from the device in that case.
    //
    LibLocateHandle (ByProtocol, &BlockIoProtocol, NULL, &NoHandles, &Handles);
    for (Index=0; Index < NoHandles; Index++) {
      Handle = Handles[Index];
      
      DevicePath=DevicePathFromHandle (Handle);

      if (CompareMem(DevicePath, Context->DevicePath, PathSize) != 0) {
        continue;
      }

      Status = BS->HandleProtocol(Handle, &BlockIoProtocol, (VOID **)&BlkIo);
        if (!EFI_ERROR(Status)) {
            if (!BlkIo->Media->LogicalPartition) {
                Buffer = AllocatePool (BlkIo->Media->BlockSize);
                BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 0, BlkIo->Media->BlockSize, Buffer);
                FreePool (Buffer);
            }
        }
    }
    //
    // Find all file system handles
    //

    LibLocateHandle (ByProtocol, &FileSystemProtocol, NULL, &NoHandles, &Handles);
    for (Index=0; Index < NoHandles; Index++) {
        Handle = Handles[Index];

        //
        // See if this is a file system on the same device
        //
        // BUGBUG: this doesn't handle instance paths

        DevicePath = DevicePathFromHandle (Handle);
        if (!DevicePath) {
            continue;
        }
        if (CompareMem(DevicePath, Context->DevicePath, PathSize) != 0) {
            //
            // Match on the first part of the device path
            //
            continue;
        }

        //
        // We found a file system on the device.  Open it.
        //
        Dir = LibOpenRoot(Handle);
        if (!Dir) {
            continue;
        }

        Status = Dir->Open (Dir, &EfiBoot, L"EFI", EFI_FILE_MODE_READ, 0);
        Dir->Close (Dir);
        if (EFI_ERROR(Status)) {
            continue;
        }
        Status = EfiBoot->Open (EfiBoot, &Dir, L"BOOT", EFI_FILE_MODE_READ, 0);
        EfiBoot->Close (EfiBoot);
        if (EFI_ERROR(Status)) {
            continue;
        }

        //
        // Check the Boot directory for a bootable image
        //
        Dir->SetPosition (Dir, 0);
        for (; ;) {

            BufferSize = FileInfoSize;
            Status = Dir->Read (Dir, &BufferSize, FileInfo);

            //
            // If at the end of the search, we're done
            //
            if (EFI_ERROR(Status) || BufferSize == 0) {
                break;
            }

            //
            // If it not an image name, skip it
            //
            if ((FileInfo->Attribute & EFI_FILE_DIRECTORY)  /*||
                !IsEfiImageName(FileInfo->FileName)*/) {
                continue;
            }

            //
            // Build a path to the image and see if we can load it
            //
            FileName = PoolPrint(L"\\EFI\\BOOT\\%s", FileInfo->FileName);
            ASSERT (FileName);
            FilePath = FileDevicePath (Handle, FileName);

            Status = BS->LoadImage (
                        TRUE,
                        GlobalImageHandle,
                        FilePath,
                        NULL,
                        0,
                        ImageHandle
                        );

            if (!EFI_ERROR(Status)) {

                //
                // Verify the image is an application (and not a driver)
                //
                Status = BS->HandleProtocol (*ImageHandle, &LoadedImageProtocol, &ImageInfo);
                ASSERT (!EFI_ERROR(Status));

                if (ImageInfo->ImageCodeType != EfiLoaderCode) {
                    DEBUG ((D_BM | D_INFO, "DefaultBootImage: Image %hs is not an application\n", FileName));
                    Status = BS->Exit(*ImageHandle, EFI_SUCCESS, 0, NULL);
                    ASSERT (!EFI_ERROR(Status));
                    Status = EFI_LOAD_ERROR;
                } else {
                    DEBUG ((D_BM | D_INFO, "BmDefaultBootImage: Loaded %hs\n", FileName));
                }
            }

            FreePool (FileName);
            FreePool (FilePath);

            if (!EFI_ERROR(Status)) {
                Dir->Close (Dir);
                FreePool (FileInfo);
                return Status;
            }
        }

        Dir->Close (EfiBoot);
    }

    *ImageHandle = NULL;
    if (FileInfo) {
        FreePool (FileInfo);
    }
    return EFI_NOT_FOUND;
}

BOOLEAN
IsEfiImageName (
    CHAR16  *FileName
    )
{
    // Search for ".efi" extension
    while (*FileName) {
        if (FileName[0] == '.' && StriCmp(FileName, L".EFI") == 0) {
            return TRUE;
        }
        FileName += 1;
    }
    return FALSE;
}

BOOLEAN
PrintBootHelp(
    IN  CHAR16      *Description
    )
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    EFI_STATUS                      Status;
    UINTN                           MaxColumn, ScreenSize;
    UINTN                           Row,Col;
    UINTN                           Size,PrintSize;
    BOOLEAN                         Done;
    CHAR16                          *TempStr;

    ConOut = ST->ConOut;

    InitializeListHead (&BootOrderHelpMenu.Head);

    Status = ConOut->SetAttribute (ConOut, BootOrderHelpMenu.ScreenAttribute);
    ASSERT (!EFI_ERROR (Status));
    ConOut->EnableCursor(ConOut, FALSE);
    if (BootOrderHelpMenu.ClearScreen) {
        Status = ConOut->ClearScreen (ConOut);
        ASSERT (!EFI_ERROR (Status));
    }
    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
    Print (BootOrderHelpMenu.Header);

    Size = StrSize(Description)/sizeof(CHAR16);
    TempStr = Description;

    Col = MaxColumn - BootOrderHelpMenu.Col*2;

    Done = FALSE;
    for (Row = 2;!Done; ) {
        if( Size > Col) {
            PrintSize = Col;
        } else {
            PrintSize = Size;
            Done = TRUE;
        }

        PrintAt(BootOrderHelpMenu.Col, Row, L"%-.*s", PrintSize, TempStr);
        Size -= PrintSize;
        TempStr = TempStr + PrintSize;   
        Row++;
    }

    return TRUE;
}

EFI_STATUS
Bmnt_DisplayDeleteBootMenu (
    VOID
    ) 
{
    BOOT_MENU_CONTEXT       *ResultContext;
    BOOT_MENU_CONTEXT       *PreviousResultContext;
    BOOT_MENU_CONTEXT       *SaveNvramContext;
    BOOT_MENU_CONTEXT       *DeleteAllContext;
    BOOT_MENU_CONTEXT       *DeleteHelpContext;
    BOOT_MENU_CONTEXT       *DeleteExitContext;
    BOOLEAN                 NVRAMNeedsUpdating;
    UINTN                   MenuOption;
    EFI_INPUT_KEY           Key;
    EFI_STATUS              Status;
    BOOLEAN                 Done;
    UINTN                   EndRow;
    UINTN                   MaxColumn, ScreenSize;
    BOOLEAN                 Exit;
    CHAR16                  *Header = L"%EDelete Boot Option(s).  Select an Option\n\n";

    // 
    // initialize header text
    //
    FreeBootOrderMenuOfVariables();
    InitializeBootOrderMenuFromVariable();

    BootMenu.Header = Header;
    DeleteAllContext =
        AllocateSpecialBootOption (&BootMenu, L"Delete All Boot Options", CON_UNSELECTED, DELETE_MENU_ALL);
    SaveNvramContext =
        AllocateSpecialBootOption (&BootMenu, L"Save Settings to NVRAM", CON_UNSELECTED, DELETE_MENU_SAVE_NVRAM);
    DeleteHelpContext =
        AllocateSpecialBootOption (&BootMenu, L"Help", CON_UNSELECTED, DELETE_MENU_HELP);
    DeleteExitContext =
        AllocateSpecialBootOption (&BootMenu, L"Exit", CON_UNSELECTED, DELETE_MENU_EXIT);

    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
    EndRow = ScreenSize - BootMenu.FooterHeight - 1;

    BootMenu.Selection = NULL;
    NVRAMNeedsUpdating = FALSE;
    for (Done = FALSE, ResultContext = NULL; !Done;) {
        PreviousResultContext = ResultContext;
        ResultContext = MenuDisplay (&BootMenu, &Key);
        if (!ResultContext) {
            Done = TRUE;
            break;
        }

        switch (Key.UnicodeChar){
        case CHAR_CARRIAGE_RETURN:
            MenuOption = ResultContext->MenuOption;
            break;
        case 'D':
        case 'd':
            if (ResultContext->MenuOption == BOOT_MENU_SELECTION)
                MenuOption = BOOT_MENU_SELECTION;
            else
                MenuOption = -1;
            break;
        case 'A':
        case 'a':
            MenuOption = DELETE_MENU_ALL;
            break;
        default:
            MenuOption =  -1;
        } // end switch

        if (MenuOption == DELETE_MENU_EXIT) {
            if (NVRAMNeedsUpdating) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 2, L"NVRAM Not updated. Save NVRAM? [Y to save, N to ignore]");
                WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
                ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
                if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
                    MenuOption = DELETE_MENU_SAVE_NVRAM;
                }
            }
            Done = TRUE;
        }


        switch (MenuOption) {
        case DELETE_MENU_ALL:
            BootMenu.Selection = NULL;
            DeleteAllBootOptions (&NVRAMNeedsUpdating);
            break;           
        case DELETE_MENU_EXIT:
            break;           
        case DELETE_MENU_HELP:
            PrintMenuHelp(&DeleteHelpMenu,DeleteMenuHelpStr);           
            break;                 
        case DELETE_MENU_SAVE_NVRAM:
            Status = SetNvramForBootMenu (&BootMenu);
            if (EFI_ERROR(Status)) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 3, L"NVRAM update failed");
            } else {
                NVRAMNeedsUpdating = FALSE;
            }
            break;
        case BOOT_MENU_SELECTION:
            Exit = FALSE;
            while (!Exit) {
                ST->ConOut->SetAttribute (ST->ConOut, BootMenu.ReverseScreenAttribute);
                PrintAt(BootMenu.Col,EndRow,L"Delete selected Boot Option [Y-Yes N-No]: ");
                WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
                ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
                ST->ConOut->SetAttribute (ST->ConOut, BootMenu.ScreenAttribute);
        
                if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
                    BootMenu.Selection = NULL;
                    if (ResultContext->OptionNumber != NEWADDITION_OPTIONNUMBER)
                        NVRAMNeedsUpdating = TRUE;
                    FreeSpecificBootMenuOption(&BootMenu, ResultContext);
                    Exit = TRUE;
                }
                if (Key.UnicodeChar == 'N' || Key.UnicodeChar == 'n') {
                    Exit = TRUE;
                }
            }
            break;
        default:
            break;
        }
    }

    FreeSpecificBootMenuOption(&BootMenu, DeleteAllContext);
    FreeSpecificBootMenuOption(&BootMenu, SaveNvramContext);
    FreeSpecificBootMenuOption(&BootMenu, DeleteHelpContext);
    FreeSpecificBootMenuOption(&BootMenu, DeleteExitContext);

    ST->ConOut->ClearScreen (ST->ConOut);
    return EFI_SUCCESS;
}

EFI_STATUS
DeleteAllBootOptions (
    OUT BOOLEAN *UpdateNvram)
{
    EFI_INPUT_KEY                   Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;
    UINTN                           EndRow;
    UINTN                           MaxColumn, ScreenSize;
    BOOLEAN                         Exit,DeleteAll;
    MENU_OPTION                     *MenuOption;
    BOOT_MENU_CONTEXT               *BootContext;
    LIST_ENTRY                      *List;   
    UINTN                           Count;

    ConOut = ST->ConOut;
    ConIn  = ST->ConIn;

    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxColumn, &ScreenSize);

    //
    // check to see if we have atleast one boot option to delete
    //
    Count = 0;
    List = BootMenu.Head.Flink;
    while (List != &BootMenu.Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        // get the next list in line
        List = List->Flink;

        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        if( BootContext->MenuOption == BOOT_MENU_SELECTION) {
            Count = 1;
            break;
        }
    } 
    
    //
    // Put prompt for delete
    //
    EndRow = ScreenSize - BootMenu.FooterHeight - 1;
    Exit = FALSE;
    DeleteAll = FALSE;
    *UpdateNvram = FALSE;
    while (!Exit && Count) {
        ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
        PrintAt(BootMenu.Col,EndRow,L"Delete ALL of above Boot Options [Y-Yes N-No]: ");
        WaitForSingleEvent (ConIn->WaitForKey, 0);
        ConIn->ReadKeyStroke (ConIn, &Key);
        ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
        
        if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
            DeleteAll = TRUE;
            Exit = TRUE;
        }
        if (Key.UnicodeChar == 'N' || Key.UnicodeChar == 'n') {
            Exit = TRUE;
        }
    } 

    //
    // Check and Delete the boot options
    //
    if ( DeleteAll) {
        List = BootMenu.Head.Flink;
        while (List != &BootMenu.Head) {
            MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
            // get the next list in case we end up freeing the current one
            List = List->Flink;

            BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
            if ((BootContext != NULL) && (BootContext->MenuOption == BOOT_MENU_SELECTION)) {
                //
                // Delete the variable from storage
                //
                if (BootContext->OptionNumber != NEWADDITION_OPTIONNUMBER) {
                    *UpdateNvram = TRUE;
                }
                FreeSpecificBootMenuOption(&BootMenu, BootContext);
            }
        } 

        //
        // Clear BootNext and BootOrder as well
        //
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
Bmnt_DisplayBootNextMenu (
    VOID
    ) 
{
    BOOT_MENU_CONTEXT       *ResultContext;
    BOOT_MENU_CONTEXT       *PreviousResultContext;
    BOOT_MENU_CONTEXT       *SaveNvramContext;
    BOOT_MENU_CONTEXT       *ResetBootNext;
    BOOT_MENU_CONTEXT       *BootNextHelpContext;
    BOOT_MENU_CONTEXT       *BootNextExitContext;
    BOOLEAN                 NVRAMNeedsUpdating;
    UINTN                   MenuOption;
    EFI_INPUT_KEY           Key;
    EFI_STATUS              Status;
    BOOLEAN                 Done;
    UINTN                   EndRow;
    UINTN                   MaxColumn, ScreenSize;
    BOOLEAN                 Exit;
    CHAR16                  *Header = L"%EManage BootNext setting.  Select an Operation\n\n";

    // 
    // initialize header text
    //
    FreeBootOrderMenuOfVariables();
    InitializeBootOrderMenuFromVariable();

    BootMenu.Header = Header;
   
    ResetBootNext =
        AllocateSpecialBootOption (&BootMenu, L"Reset BootNext Setting", CON_UNSELECTED, BOOTNEXT_MENU_RESET);
    SaveNvramContext =
        AllocateSpecialBootOption (&BootMenu, L"Save Settings to NVRAM", CON_UNSELECTED, BOOTNEXT_MENU_SAVE_NVRAM);
    BootNextHelpContext =
        AllocateSpecialBootOption (&BootMenu, L"Help", CON_UNSELECTED, BOOTNEXT_MENU_HELP);
    BootNextExitContext =
        AllocateSpecialBootOption (&BootMenu, L"Exit", CON_UNSELECTED, BOOTNEXT_MENU_EXIT);

    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
    EndRow = ScreenSize - BootMenu.FooterHeight - 1;

    BootMenu.Selection = NULL;
    NVRAMNeedsUpdating = FALSE;
    for (Done = FALSE, ResultContext = NULL; !Done;) {
        PreviousResultContext = ResultContext;
        ResultContext = MenuDisplay (&BootMenu, &Key);
        if (!ResultContext) {
            Done = TRUE;
            break;
        }

        switch (Key.UnicodeChar){
        case CHAR_CARRIAGE_RETURN:
            MenuOption = ResultContext->MenuOption;
            break;
        case 'B':
        case 'b':
            if (ResultContext->MenuOption == BOOT_MENU_SELECTION)
                MenuOption = BOOT_MENU_SELECTION;
            else
                MenuOption = -1;
            break;
        case 'R':
        case 'r':
            MenuOption = BOOTNEXT_MENU_RESET;
            break;
        default:
            MenuOption =  -1;
        } // end switch

        if (MenuOption == BOOTNEXT_MENU_EXIT) {
            if (NVRAMNeedsUpdating) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 2, L"NVRAM Not updated. Save NVRAM? [Y to save, N to ignore]");
                WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
                ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
                if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
                    MenuOption = BOOTNEXT_MENU_SAVE_NVRAM;
                }
            }
            Done = TRUE;
        }

        switch (MenuOption) {
        case BOOTNEXT_MENU_RESET:
            ResetBootNextOption (&NVRAMNeedsUpdating, TRUE);
            break;           
        case BOOTNEXT_MENU_EXIT:
            break;           
        case BOOTNEXT_MENU_HELP:
            PrintMenuHelp(&BootNextHelpMenu,BootNextMenuHelpStr);           
            break;                 
        case BOOTNEXT_MENU_SAVE_NVRAM:
            Status = SetNvramForBootMenu (&BootMenu);
            if (EFI_ERROR(Status)) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 3, L"NVRAM update failed");
            } else {
                NVRAMNeedsUpdating = FALSE;
            }
            break;
        case BOOT_MENU_SELECTION:
            Exit = FALSE;
            while (!Exit) {
                ST->ConOut->SetAttribute (ST->ConOut, BootMenu.ReverseScreenAttribute);
                PrintAt(BootMenu.Col,EndRow,L"Enter selected Boot Option as 'BootNext' [Y-Yes N-No]: ");
                WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
                ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
                ST->ConOut->SetAttribute (ST->ConOut, BootMenu.ScreenAttribute);
        
                if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
                    ResetBootNextOption (&NVRAMNeedsUpdating,FALSE);
                    if (ResultContext->OptionNumber != NEWADDITION_OPTIONNUMBER)
                        NVRAMNeedsUpdating = TRUE;
                    ResultContext->IsBootNext = TRUE;
                    Exit = TRUE;
                }
                if (Key.UnicodeChar == 'N' || Key.UnicodeChar == 'n') {
                    Exit = TRUE;
                }
            }
            break;
        default:
            break;
        }
    }

    FreeSpecificBootMenuOption(&BootMenu, ResetBootNext);
    FreeSpecificBootMenuOption(&BootMenu, SaveNvramContext);
    FreeSpecificBootMenuOption(&BootMenu, BootNextHelpContext);
    FreeSpecificBootMenuOption(&BootMenu, BootNextExitContext);

    ST->ConOut->ClearScreen (ST->ConOut);
    return EFI_SUCCESS;
}


EFI_STATUS
ResetBootNextOption (
    OUT BOOLEAN *UpdateNvram,
    IN  BOOLEAN Prompt
    )
{
    EFI_INPUT_KEY                   Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;
    UINTN                           EndRow, Count;
    UINTN                           MaxColumn, ScreenSize;
    MENU_OPTION                     *MenuOption;
    BOOT_MENU_CONTEXT               *BootContext;
    LIST_ENTRY                      *List;   


    ConOut = ST->ConOut;
    ConIn  = ST->ConIn;

    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
     
    Count = 0;

    List = BootMenu.Head.Flink;
    while (List != &BootMenu.Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        // get the next list in case we end up freeing the current one
        List = List->Flink;

        BootContext = (BOOT_MENU_CONTEXT *)MenuOption->Context;
        if ((BootContext != NULL) && (BootContext->MenuOption == BOOT_MENU_SELECTION)
                                  && (BootContext->IsBootNext)) {

            if (BootContext->OptionNumber != NEWADDITION_OPTIONNUMBER) {
                *UpdateNvram = TRUE;
            }
            
            Count = 1 ;
            BootContext->IsBootNext = FALSE;
        }
    }

    if(Count && Prompt) {    
        //
        // Print message
        //
        EndRow = ScreenSize - BootMenu.FooterHeight - 1;
        ConOut->SetAttribute (ConOut, BootMenu.ReverseScreenAttribute);
        PrintAt(BootMenu.Col,EndRow,L"'BootNext' variable cleared. Press any key...");
        WaitForSingleEvent (ConIn->WaitForKey, 0);
        ConIn->ReadKeyStroke (ConIn, &Key);
        ConOut->SetAttribute (ConOut, BootMenu.ScreenAttribute);
    }

    return EFI_SUCCESS;
}

