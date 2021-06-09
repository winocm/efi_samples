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

    console.c
    
Abstract:

    handles console redirection from bootmanager


Revision History

--*/

#include "menu.h"

typedef enum {
    UnusedMenuEntry,
    ConsoleMenuOut,
    ConsoleMenuIn,
    ConsoleMenuError
} TYPE_OF_DISPLAY_CONSOLE_MENU;

                
//
// BugBug: It smells like a lib function
//
//--------------------------------------
BOOLEAN
MatchDevicePaths (
    IN  EFI_DEVICE_PATH *Multi,
    IN  EFI_DEVICE_PATH *Single
    );

EFI_DEVICE_PATH *
DuplicateDevicePathInstance (
    IN EFI_DEVICE_PATH  *DevPath
    );
//----------------------------------------

EFI_STATUS
SetNvramForConsoleMenu (
    IN  EFI_MENU                        *Menu,
    IN  CHAR16                          *VarName,
    IN  TYPE_OF_DISPLAY_CONSOLE_MENU    MenuType
    );

BOOLEAN
CheckIfSelectionAllowed (
    IN  EFI_MENU                           *Menu,
    IN  TYPE_OF_DISPLAY_CONSOLE_MENU       CurrentMenuType,
    IN  CONSOLE_MENU_CONTEXT               *SelMenuContext
    );

BOOLEAN
VerifyDevicePaths (
    IN  EFI_DEVICE_PATH     *CurSelDevicePath,
    IN  EFI_DEVICE_PATH     *PrevSelDevicePath
);

VOID
ConsoleContextPrint (
     IN struct _MENU_OPTION *MenuOption,
     IN VOID                *Context,
     IN UINTN               *Row,
     IN UINTN               Column
     );

CONSOLE_MENU_CONTEXT *
AllocateSpecialMenuOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN UINTN        Option      
    );

EFI_STATUS
DisplayConsoleMenu (
    IN  TYPE_OF_DISPLAY_CONSOLE_MENU    MenuType,
    IN  CHAR16                          *Header,
    IN  CHAR16                          *Footer
    );

VOID
FreeConsoleMenu (
    IN EFI_MENU     *Menu
    );

EFI_DEVICE_PATH *
DuplicateDevicePathInstance (
    IN EFI_DEVICE_PATH  *DevPath
    );

VOID
ToggleSelectionStr(
    IN EFI_MENU             *Menu,
    IN CONSOLE_MENU_CONTEXT *SelMenuContext,
    IN BOOLEAN              Active
    );

EFI_MENU ConsoleMenu = {
    MENU_SIGNATURE, NULL,  
    4, CON_UNSELECTED, CON_ACTIVE_SELECTION, 
    TRUE, 10, 
    L"%ESelect an Operation\n\n", 
    L"    ", 4, 
    NULL, NULL, NULL, NULL, NULL, TRUE, 0, MENU_ENTRY_FORMAT
};

EFI_STATUS
Bmnt_DisplayConsoleOutMenu () 
{
    return DisplayConsoleMenu (
                ConsoleMenuOut, 
                L"Select the Console Output Device(s)\n\n",
                NULL
                );
}

EFI_STATUS
Bmnt_DisplayConsoleInMenu () 
{
    return DisplayConsoleMenu (
                ConsoleMenuIn,
                L"Select the Console Input Device(s)\n\n",
                NULL                
                );
}

EFI_STATUS
Bmnt_DisplayStdErrorMenu () 
{
    return DisplayConsoleMenu (
                ConsoleMenuError,
                L"Select the Standard Error Device\n\n",
                NULL
                );
}

EFI_DEVICE_PATH *
DuplicateDevicePathInstance (
    IN EFI_DEVICE_PATH  *DevPath
    )
{
    EFI_DEVICE_PATH     *NewDevPath,*DevicePathInst,*Temp;
    UINTN               Size;    

    //
    // get the size of an instance from the input
    //

    Temp = DevPath;
    DevicePathInst = DevicePathInstance (&Temp, &Size);
    
    //
    // Make a copy and set proper end type
    //
    NewDevPath = NULL;
    if (Size) { 
        NewDevPath = AllocatePool (Size + sizeof(EFI_DEVICE_PATH));
    }

    if (NewDevPath) {
        CopyMem (NewDevPath, DevicePathInst, Size);
        Temp = (EFI_DEVICE_PATH *) (((UINT8 *) NewDevPath) + Size);
        SetDevicePathEndNode(Temp);
    }

    return NewDevPath;
}

EFI_STATUS
InitializeConsoleMenuFromVariable (
    IN      TYPE_OF_DISPLAY_CONSOLE_MENU    MenuType,
    IN OUT  EFI_MENU                        *ConMenu
    )
{
    CHAR16                           Buffer[1];
    CONSOLE_MENU_CONTEXT             *Context;
    EFI_DEVICE_PATH                  *OutDevicePath;
    EFI_DEVICE_PATH                  *InpDevicePath;
    EFI_DEVICE_PATH                  *ErrDevicePath;
    EFI_DEVICE_PATH                  *PrevSelDevicePath, *AllDevicePath;
    EFI_DEVICE_PATH                  *Multi, *DevicePathInst;
    UINTN                            Size;
    CHAR16                           *VarString;
  
    OutDevicePath = LibGetVariable (VarConsoleOut, &EfiGlobalVariable);
    InpDevicePath = LibGetVariable (VarConsoleInp, &EfiGlobalVariable);
    ErrDevicePath = LibGetVariable (VarErrorOut, &EfiGlobalVariable);

    switch (MenuType) {
    case ConsoleMenuOut:
        PrevSelDevicePath = OutDevicePath;
        AllDevicePath = LibGetVariable (VarConsoleOutDev, &EfiGlobalVariable);
        VarString = VarConsoleOut;
        break;
    case ConsoleMenuIn:
        PrevSelDevicePath = InpDevicePath;
        AllDevicePath = LibGetVariable (VarConsoleInpDev, &EfiGlobalVariable);
        VarString = VarConsoleInp;
        break;
    case ConsoleMenuError:
        PrevSelDevicePath = ErrDevicePath;
        AllDevicePath = LibGetVariable (VarErrorOutDev, &EfiGlobalVariable);
        VarString = VarErrorOut;
        break;
    default:
        ASSERT(FALSE);
    };
   
    InitializeListHead (&ConMenu->Head);

    //
    // get all instances from the BS variable
    //
    Multi = AllDevicePath;    
    while (DevicePathInst = DevicePathInstance (&Multi, &Size)) {
        Context = AllocateZeroPool (sizeof(CONSOLE_MENU_CONTEXT));
        ASSERT (Context);

        Context->MenuOption = CON_MENU_CONSOLE;
        Context->DevicePath = DuplicateDevicePathInstance(DevicePathInst);
        Context->ActiveConsole = MatchDevicePaths (PrevSelDevicePath, Context->DevicePath);
        Context->Menu = AllocateMenuOption (ConMenu, Buffer, CON_UNSELECTED, Context);
    }

  
    if (AllDevicePath) {
      FreePool (AllDevicePath);
    }
    if (OutDevicePath) {
      FreePool (OutDevicePath);
    }
    if (InpDevicePath) {
      FreePool (InpDevicePath);
    }
    if (ErrDevicePath) {
      FreePool (ErrDevicePath);
    }
    return EFI_SUCCESS;
}

EFI_STATUS
DisplayConsoleMenu (
    IN  TYPE_OF_DISPLAY_CONSOLE_MENU    MenuType,
    IN  CHAR16                          *Header,
    IN  CHAR16                          *Footer
    ) 
{
    CHAR16                  Buffer[MAX_CHAR];
    CONSOLE_MENU_CONTEXT    *ResultContext;
    CONSOLE_MENU_CONTEXT    *PreviousResultContext;
    CONSOLE_MENU_CONTEXT    *Context;
    EFI_INPUT_KEY           Key;
    UINTN                   Attribute;
    EFI_DEVICE_PATH         *OutDevicePath;
    EFI_DEVICE_PATH         *InpDevicePath;
    EFI_DEVICE_PATH         *ErrDevicePath;
    EFI_DEVICE_PATH         *PrevSelDevicePath, *AllDevicePath;
    EFI_DEVICE_PATH         *Multi, *DevicePathInst;
    UINTN                   Size;
//    EFI_GUID                *CorGuid;
    BOOLEAN                 Done;
    UINTN                   ActiveConsoleCount;
//    EFI_GUID                *DeviceGuid;
    CHAR16                  *VarString;
    BOOLEAN                 NVRAMNeedsUpdating;
    UINTN                   MenuOption;
    EFI_STATUS              Status;

    OutDevicePath = LibGetVariable (VarConsoleOut, &EfiGlobalVariable);
    InpDevicePath = LibGetVariable (VarConsoleInp, &EfiGlobalVariable);
    ErrDevicePath = LibGetVariable (VarErrorOut, &EfiGlobalVariable);

    ActiveConsoleCount = 0;

    switch (MenuType) {
    case ConsoleMenuOut:
  //      DeviceGuid = &TextOutProtocol;
  //      CorGuid = &TextInProtocol;
        PrevSelDevicePath = OutDevicePath;
        AllDevicePath = LibGetVariable (VarConsoleOutDev, &EfiGlobalVariable);
        VarString = VarConsoleOut;
        break;
    case ConsoleMenuIn:
 //       DeviceGuid = &TextInProtocol, 
 //       CorGuid = &TextOutProtocol;
        PrevSelDevicePath = InpDevicePath;
        AllDevicePath = LibGetVariable (VarConsoleInpDev, &EfiGlobalVariable);
        VarString = VarConsoleInp;
        break;
    case ConsoleMenuError:
//        DeviceGuid = &TextOutProtocol, 
//        CorGuid = &TextInProtocol;
        PrevSelDevicePath = ErrDevicePath;
        AllDevicePath = LibGetVariable (VarErrorOutDev, &EfiGlobalVariable);
        VarString = VarErrorOut;
        break;
    default:
        ASSERT(FALSE);
    };
    
    ConsoleMenu.BottomOfPage = NULL;
    InitializeListHead (&ConsoleMenu.Head);

    //
    // get all instances from the BS variable
    //
    Multi = AllDevicePath;    
    while (DevicePathInst = DevicePathInstance (&Multi, &Size)) {
        Context = AllocateZeroPool (sizeof(CONSOLE_MENU_CONTEXT));
        ASSERT (Context);

        Context->MenuOption = CON_MENU_CONSOLE;
//        Context->Handle = Handles[Index];
        Context->DevicePath = DuplicateDevicePathInstance(DevicePathInst);
        Context->DevicePathStr = DevicePathToStr (Context->DevicePath);
        Context->ActiveConsole = MatchDevicePaths (PrevSelDevicePath, Context->DevicePath);
      
        Attribute = CON_UNSELECTED;

        Context->IsConIn  = MatchDevicePaths (InpDevicePath, Context->DevicePath);
        Context->IsConOut = MatchDevicePaths (OutDevicePath, Context->DevicePath);
        Context->IsStdErr = MatchDevicePaths (ErrDevicePath, Context->DevicePath);

        SPrint (Buffer, sizeof(Buffer), L"  %s", Context->DevicePathStr);
        Context->Menu = AllocateMenuOption (&ConsoleMenu, Buffer, Attribute, Context);
        Context->Menu->ContextPrint = ConsoleContextPrint;
        if (Context->ActiveConsole) { 
            ToggleSelectionStr(&ConsoleMenu,Context,Context->ActiveConsole);
////            Attribute = CON_SELECTED;
            ActiveConsoleCount++;
       } 
    }

    AllocateSpecialMenuOption (&ConsoleMenu, L"Save Settings to NVRAM", CON_UNSELECTED, CON_MENU_SAVE_NVRAM);
    AllocateSpecialMenuOption (&ConsoleMenu, L"Exit", CON_UNSELECTED, CON_MENU_EXIT);

    ConsoleMenu.Header = Header;
    ConsoleMenu.Footer = Footer;
    ConsoleMenu.Selection = NULL;
    NVRAMNeedsUpdating = FALSE;
    for (Done = FALSE, ResultContext = NULL; !Done;) {
        PreviousResultContext = ResultContext;
        ResultContext = MenuDisplay (&ConsoleMenu, &Key);
        if (!ResultContext) {
            Done = TRUE;
            break;
        }

        MenuOption = ResultContext->MenuOption;
        if (MenuOption == CON_MENU_EXIT) {
            if (NVRAMNeedsUpdating) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 2, L"NVRAM Not updated. Save NVRAM? [Y to save, N to ignore]");
                WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
                ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
                if (Key.UnicodeChar == 'Y' || Key.UnicodeChar == 'y') {
                    MenuOption = CON_MENU_SAVE_NVRAM;
                }
            } 
            Done = TRUE;            
        }

        switch (MenuOption) {
        case CON_MENU_EXIT:
            break;           
        case CON_MENU_SAVE_NVRAM:
            Status = SetNvramForConsoleMenu (&ConsoleMenu, VarString, MenuType);
            if (EFI_ERROR(Status)) {
                PrintAt (ResultContext->Menu->Col, ResultContext->Menu->Row + 3, L"NVRAM update failed");
            } else {
                NVRAMNeedsUpdating = FALSE;
            }
            break;
        case CON_MENU_CONSOLE:
            //
            // Toggle selection
            //
            if (ResultContext->ActiveConsole) {
                ResultContext->ActiveConsole = FALSE;
                ResultContext->Menu->Attribute = CON_UNSELECTED;
            } else {
                ResultContext->ActiveConsole = CheckIfSelectionAllowed(&ConsoleMenu, MenuType, ResultContext);
                if (ResultContext->ActiveConsole) {
                    ResultContext->Menu->Attribute = CON_UNSELECTED;
                }
                else
                    ResultContext->Menu->Attribute = CON_UNSELECTED;
            }
            ToggleSelectionStr(&ConsoleMenu,ResultContext,ResultContext->ActiveConsole);
/*            if (MenuType == ConsoleMenuError) {
                //
                // If the menu only supports a single selection.
                //  Inactivate the previous selection.
                //
                if (PreviousResultContext && (PreviousResultContext != ResultContext)) {
                    PreviousResultContext->ActiveConsole = FALSE;
                    PreviousResultContext->Menu->Attribute = CON_UNSELECTED;
                }
            }
*/
            //
            // This variable was set with the orignal NVRAM setting so update the settings
            //
            switch (MenuType) {
            case ConsoleMenuIn:
                ResultContext->IsConIn = ResultContext->ActiveConsole;
                break;
            case ConsoleMenuOut:
                ResultContext->IsConOut = ResultContext->ActiveConsole;
                break;
            case ConsoleMenuError:
                ResultContext->IsStdErr = ResultContext->ActiveConsole;
                break;
            default:
                break;
            }

            NVRAMNeedsUpdating = TRUE;
        default:
            break;
        }
    }

    if (OutDevicePath) {
        FreePool (OutDevicePath);
    }
    if (InpDevicePath) {
        FreePool (InpDevicePath);
    }
    if (ErrDevicePath) {
        FreePool (ErrDevicePath);
    }
    if (AllDevicePath) {
        FreePool (AllDevicePath);
    }

    FreeConsoleMenu (&ConsoleMenu);
    ST->ConOut->ClearScreen (ST->ConOut);
    return EFI_SUCCESS;
}

VOID
ConsoleContextPrint (
     IN struct _MENU_OPTION *MenuOption,
     IN VOID                *Context,
     IN UINTN               *Row,
     IN UINTN               Column
     )
{
    CONSOLE_MENU_CONTEXT   *ConContext;
    CHAR16                 *Str1, *Str2, *Str3;

    ConContext = (CONSOLE_MENU_CONTEXT *)Context;

    Str1 = Str2 = Str3 = L"";
    if (ConContext->MenuOption == CON_MENU_CONSOLE) {
        if (ConContext->IsConIn) {
            Str1 = L"Active Input Device. ";
        }
        if (ConContext->IsConOut) {
            Str2 = L"Active Output Device. ";
        }
        if (ConContext->IsStdErr) {
            Str3 = L"Active Standard Error Device.";
        } 
    } 
    PrintAt (Column, *Row + 1, L"%s%s%-.80s", Str1, Str2, Str3);
    *Row = *Row + 2;
}

CONSOLE_MENU_CONTEXT *
AllocateSpecialMenuOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN UINTN        Option        
    )
{
    CONSOLE_MENU_CONTEXT    *Context;

    Context = AllocateZeroPool (sizeof(CONSOLE_MENU_CONTEXT));
    ASSERT (Context);
    Context->MenuOption = Option;

    Context->Menu = AllocateMenuOption (Menu, String, Attribute, Context);
    Context->Menu->ContextPrint = ConsoleContextPrint;
    return Context;
}


EFI_STATUS
SetNvramForConsoleMenu (
    IN  EFI_MENU                        *Menu,
    IN  CHAR16                          *VarName,
    IN  TYPE_OF_DISPLAY_CONSOLE_MENU    MenuType
    )
{
    MENU_OPTION             *MenuOption;
    CONSOLE_MENU_CONTEXT    *MenuContext;
    LIST_ENTRY              *List;
    EFI_DEVICE_PATH         *DevicePath;
    EFI_STATUS              Status;
    UINTN                   Size;

    DevicePath = NULL;
    List = Menu->Head.Flink;
    while (List != &Menu->Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        MenuContext = (CONSOLE_MENU_CONTEXT *)MenuOption->Context;
        if (MenuContext != NULL) {
            if (MenuContext->ActiveConsole) {
                DevicePath = AppendDevicePathInstance (DevicePath, MenuContext->DevicePath);
            } 
            //
            // This variable was set with the orignal NVRAM setting so update the settings
            //
            switch (MenuType) {
            case ConsoleMenuIn:
                MenuContext->IsConIn = MenuContext->ActiveConsole;
                break;
            case ConsoleMenuOut:
                MenuContext->IsConOut = MenuContext->ActiveConsole;
                break;
            case ConsoleMenuError:
                MenuContext->IsStdErr = MenuContext->ActiveConsole;
                break;
            default:
                break;
            }
        }        
        List = List->Flink;
    }

    Size = 0;
    if (DevicePath) {
        Size = DevicePathSize(DevicePath);
    }

    Status = RT->SetVariable (
                VarName, &EfiGlobalVariable,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                Size, DevicePath 
                );

    if (DevicePath) {
        FreePool (DevicePath);
    }

    return Status;
}

VOID
FreeConsoleMenu (
    IN EFI_MENU     *Menu
    )
{
    MENU_OPTION             *MenuOption;
    CONSOLE_MENU_CONTEXT    *MenuContext;
    LIST_ENTRY              *List;   
    //
    // Delete the menu option's Context
    //
    List = Menu->Head.Flink;
    while (List != &Menu->Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        MenuContext = (CONSOLE_MENU_CONTEXT *)MenuOption->Context;
        if (MenuContext != NULL) {
            if (MenuContext->DevicePathStr) {
                FreePool (MenuContext->DevicePathStr);
            }
            FreePool (MenuOption->Context);
        }
        
        List = List->Flink;
    }
    MenuFree (Menu);
}


BOOLEAN
MatchDevicePaths (
    IN  EFI_DEVICE_PATH *Multi,
    IN  EFI_DEVICE_PATH *Single
    )
{
    EFI_DEVICE_PATH     *DevicePath, *DevicePathInst;
    UINTN               Size;

    if (!Multi || !Single) {
        return FALSE;
    }

    DevicePath = Multi;
    while (DevicePathInst = DevicePathInstance (&DevicePath, &Size)) {
        if (CompareMem (Single, DevicePathInst, Size) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN
CheckIfSelectionAllowed (
    IN  EFI_MENU                     *Menu,
    IN  TYPE_OF_DISPLAY_CONSOLE_MENU CurrentMenuType,
    IN  CONSOLE_MENU_CONTEXT         *SelMenuContext
    )
{
    MENU_OPTION                      *MenuOption;
    CONSOLE_MENU_CONTEXT             *MenuContext;
    LIST_ENTRY                       *List;
    EFI_MENU                         ConMenu;
    EFI_MENU                         *TempMenu;
    UINTN                            Index;
    TYPE_OF_DISPLAY_CONSOLE_MENU     MenuType;

    //
    // We should deal with two different cases here. The first case is we can not select a console
    // (terminal) which has the same hardware device path part with an active one in the *same* menu. 
    // The second case is if we want to select a console which has the same hardware device path part 
    // with an active one in *another* two menu, we should make sure these two consoles' device path
    // are identical.
    //
    for (Index = 0; Index < 3; Index++) {
      switch (Index) {
      case 0:
         MenuType = ConsoleMenuIn;
         break;
      case 1:
        MenuType = ConsoleMenuOut;
        break;
      case 2:
        MenuType = ConsoleMenuError;
        break;
      default:
        ASSERT(FALSE);
      }
      if (MenuType != CurrentMenuType) {
        //
        // Generate an undisplayed menu here to make the comparison easier.
        //
        InitializeConsoleMenuFromVariable (MenuType, &ConMenu);
        TempMenu = &ConMenu;
      } else {
        TempMenu = Menu;
      }
  
      List = TempMenu->Head.Flink;
      while (List != &TempMenu->Head) {
          MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
          
          MenuContext = (CONSOLE_MENU_CONTEXT *)MenuOption->Context;
          if ((MenuContext != NULL) && (MenuContext != SelMenuContext)) {
            if (MenuContext->ActiveConsole) {
              if (!VerifyDevicePaths (SelMenuContext->DevicePath, MenuContext->DevicePath)) {
                //
                // Both device paths point to the same hardware location here.
                // 
                if (MenuType != CurrentMenuType &&
                  ( 0 == CompareMem (SelMenuContext->DevicePath, MenuContext->DevicePath, DevicePathSize (SelMenuContext->DevicePath)))) {
                  return TRUE;
                } else {
                  return FALSE;
                }
              }
            } 
          }        
          List = List->Flink;
      } // end while
      if (MenuType != CurrentMenuType) {
        FreeConsoleMenu (&ConMenu);
      }
    }

    return TRUE;
}

BOOLEAN
VerifyDevicePaths (
    IN  EFI_DEVICE_PATH     *CurSelDevicePath,
    IN  EFI_DEVICE_PATH     *PrevSelDevicePath
)
{
    EFI_DEVICE_PATH     *Src1, *Src1Subset;
    EFI_DEVICE_PATH     *Src2, *Src2Subset;
    BOOLEAN             StopSearch1,StopSearch2,NoMatch;

    //
    // The logic here is to search till the messaging_device_path
    // is hit on both the inputs. Then a compare is made till that
    // point. If there is a match then both device paths point to
    // the same hardware location. Return TRUE if they don't match
    //
    
    Src1 = CurSelDevicePath;
    Src2 = PrevSelDevicePath;
    Src1Subset = NULL;
    Src2Subset = NULL;
    StopSearch1 = FALSE;
    StopSearch2 = FALSE;
    NoMatch = TRUE;

    while (!IsDevicePathEnd(Src1) && !IsDevicePathEnd(Src2)) {      
        if(DevicePathType(Src1) != MESSAGING_DEVICE_PATH) {
            Src1Subset = AppendDevicePathNode (Src1Subset, Src1);
            Src1 = NextDevicePathNode(Src1);
        }
        else
            StopSearch1 = TRUE;

        if(DevicePathType(Src2) != MESSAGING_DEVICE_PATH) {
            Src2Subset = AppendDevicePathNode (Src2Subset, Src2);
            Src2 = NextDevicePathNode(Src2);
        }
        else
            StopSearch2 = TRUE;

        if(StopSearch1 && StopSearch2) {
            NoMatch = (CompareMem(Src1Subset,Src2Subset,DevicePathSize(Src1Subset)) != 0);            
            FreePool(Src1Subset);
            FreePool(Src2Subset);
            break;
        }
    }// end while
   
    return NoMatch;
}

VOID
ToggleSelectionStr(
    IN EFI_MENU             *Menu,
    IN CONSOLE_MENU_CONTEXT *SelMenuContext,
    IN BOOLEAN              Active
    )
{
    MENU_OPTION             *MenuOption;
    CONSOLE_MENU_CONTEXT    *MenuContext;
    LIST_ENTRY              *List;

    List = Menu->Head.Flink;
    while (List != &Menu->Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        MenuContext = (CONSOLE_MENU_CONTEXT *)MenuOption->Context;
        if (MenuContext == SelMenuContext ) {
            if (Active) {
                MenuOption->Description[0] = '*';
            } else {
                MenuOption->Description[0] = ' ';
            }
            break;
        }
        List = List->Flink;
    } // end while
}
