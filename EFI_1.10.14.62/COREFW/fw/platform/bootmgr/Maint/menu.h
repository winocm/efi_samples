#ifndef _MENU_H
#define _MENU_H
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

    menu.h

Abstract:

    EFI menu system



Revision History

--*/
#include "efi.h"
#include "efilib.h"

#define NEWADDITION_OPTIONNUMBER    0xFFFF

#define MAX_CHAR        240
#define MAX_CHAR_SIZE   (MAX_CHAR * 2)

typedef struct _MENU_OPTION;

typedef
VOID
(*CONTEXT_PRINT) (
     IN struct _MENU_OPTION *MenuOption,
     IN VOID                *Context,
     IN UINTN               *Row,
     IN UINTN               Column
     );

#define MENU_OPTION_SIGNATURE    'mnue'

typedef struct _MENU_OPTION {
    UINTN                   Signature;
    LIST_ENTRY              Link;

    CHAR16                  *Description;
    UINTN                   Attribute;
    UINTN                   Skip;

    UINTN                   Row;
    UINTN                   Col;

    VOID                    *Context;
    CONTEXT_PRINT           ContextPrint;
} MENU_OPTION;

#define MENU_SIGNATURE    'menu'

typedef struct {
    UINTN               Signature;
    CHAR8               *LangCode;

    //
    // Start Menu at this location
    //
    UINTN               Col;

    UINTN               ScreenAttribute;
    UINTN               ReverseScreenAttribute;
    BOOLEAN             ClearScreen;
    UINTN               Timeout;

    CHAR16              *Header;
    CHAR16              *Footer;
    UINTN               FooterHeight;

    LIST_ENTRY          Head;

    MENU_OPTION         *TopOfPage;
    MENU_OPTION         *BottomOfPage;
    MENU_OPTION         *Selection;

    BOOLEAN             FooterUnderMenu;
    UINTN               MaxMenuRow;

    CHAR16              *FmtStr;
} EFI_MENU;

#define MENU_ENTRY_FORMAT   L"%-.64s"

//
// File menu context
//
typedef struct {
    EFI_HANDLE                        Handle;
    EFI_DEVICE_PATH                   *DevicePath;
    CHAR16                            *DevicePathStr;
    EFI_FILE_HANDLE                   FHandle;
    CHAR16                            *FileName;
    EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *Info;

    BOOLEAN                           RootContext;
    BOOLEAN                           IsDir;
    BOOLEAN                           IsRemovableMedia;
    BOOLEAN                           IsLoadFile;

    UINTN                             MenuOption;
    MENU_OPTION                       *Menu;
} FILE_MENU_CONTEXT;

//
// File MenuOption Defines
//
#define FILE_MENU_DYNAMIC_ENTRY     0x00
#define FILE_MENU_MONOLITHIC_APP    0x01
#define FILE_MENU_EXIT              0x02

//
// Console Menu Attributes 
//
#define FILE_UNSELECTED         EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK)
#define FILE_SELECTED           EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY)
#define FILE_ACTIVE_SELECTION   EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY)  
/*    
#define FILE_UNSELECTED          EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLUE)
#define FILE_SELECTED            EFI_TEXT_ATTR(EFI_WHITE, EFI_BLUE)  
#define FILE_ACTIVE_SELECTION    EFI_TEXT_ATTR(EFI_LIGHTBLUE, EFI_LIGHTGRAY)  
*/

//
// Console menu context
//
typedef struct {
    EFI_HANDLE              Handle;
    EFI_DEVICE_PATH         *DevicePath;
    CHAR16                  *DevicePathStr;
    BOOLEAN                 ActiveConsole;

    BOOLEAN                 IsConIn;
    BOOLEAN                 IsConOut;
    BOOLEAN                 IsStdErr;

    UINTN                   MenuOption;
    MENU_OPTION             *Menu;
} CONSOLE_MENU_CONTEXT;

//
// MenuOption Defines
//
#define CON_MENU_CONSOLE    0x00
#define CON_MENU_SAVE_NVRAM 0x01
#define CON_MENU_EXIT       0x02

//
// Console Menu Attributes 
//
#define CON_UNSELECTED          EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK)
#define CON_SELECTED            EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY)  
#define CON_ACTIVE_SELECTION    EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY)  
/*
#define CON_UNSELECTED          EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLUE)
#define CON_SELECTED            EFI_TEXT_ATTR(EFI_WHITE, EFI_BLUE)  
#define CON_ACTIVE_SELECTION    EFI_TEXT_ATTR(EFI_LIGHTBLUE , EFI_LIGHTGRAY)  
*/
//
// Boot menu context
//
typedef struct {
    UINTN               Signature;
    
    LIST_ENTRY          All;
    LIST_ENTRY          Order;

    MENU_OPTION         *Menu;
    UINTN               MenuOption;
    BOOLEAN             IsActive;

    UINT16              OptionNumber;           // option # part of name
    UINT8               *RawOption;
    UINTN               RawOptionSize;

    UINT32              Attributes;
    CHAR16              *Description;
    EFI_DEVICE_PATH     *FilePath;
    CHAR16              *FilePathString;

    UINT16              BootString[10];
    BOOLEAN             LoadOptionModified;
    VOID                *LoadOptions;
    UINTN               LoadOptionsSize;

    BOOLEAN             IsBootNext;
} BOOT_MENU_CONTEXT;


//
// Boot MenuOption Defines
//
#define BOOT_MENU_SELECTION         0x00
#define BOOT_MENU_SAVE_NVRAM        0x01
#define BOOT_MENU_HELP              0x02
#define BOOT_MENU_EXIT              0x03


//
// Delete MenuOption Defines
//
#define DELETE_MENU_SAVE_NVRAM  0x01
#define DELETE_MENU_HELP        0x02
#define DELETE_MENU_EXIT        0x03
#define DELETE_MENU_ALL         0x04


//
// BootNext MenuOption Defines
//
#define BOOTNEXT_MENU_SAVE_NVRAM  0x01
#define BOOTNEXT_MENU_HELP        0x02
#define BOOTNEXT_MENU_EXIT        0x03
#define BOOTNEXT_MENU_RESET       0x04

//
// TIMEOUT menu context
//
typedef struct {
    UINT16                  Timeout;
    UINTN                   MenuOption;
    MENU_OPTION             *Menu;
} TIMEOUT_MENU_CONTEXT;

//
// MenuOption Defines
//
#define TIMEOUT_MENU_MODIFY    0x00
#define TIMEOUT_MENU_DELETE    0x01
#define TIMEOUT_MENU_HELP      0x02
#define TIMEOUT_MENU_EXIT      0x03
//
// Timeout Menu Attributes 
//
#define TIMEOUT_UNSELECTED          EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK)
#define TIMEOUT_SELECTED            EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY)  
#define TIMEOUT_ACTIVE_SELECTION    EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY)  


typedef
EFI_STATUS
(*MENU_PICK) ();

VOID
PrintBanner (
    VOID
    );

//
// Menu functions
//
EFI_STATUS
DisplayFileSystemMenu (
    IN  BOOLEAN BootFromAFileSystem
    );

EFI_STATUS
Bmnt_DisplayBootMenuOption(); 

EFI_STATUS
Bmnt_DisplayDeleteBootMenu(); 

EFI_STATUS
Bmnt_DisplayFileSystemBootFromFsMenu(); 

EFI_STATUS
Bmnt_DisplayBootTimeOut();

MENU_OPTION *
AllocateMenuOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN VOID         *Context
    );

VOID
MenuDisplayWindow (
    IN EFI_MENU     *RootMenu,
    IN UINTN        StartRow,
    IN UINTN        ScreenHeight,
    IN UINTN        ScreenWidth
    );

VOID *
MenuDisplay (
    IN  EFI_MENU        *Menu,
    IN  EFI_INPUT_KEY   *Key
    );

VOID
MenuFree (
    IN  EFI_MENU    *Menu
    );

VOID
SkipMenuLine(
    IN EFI_MENU *Menu
    );


//
// FileMenu functions
//

BOOLEAN
FindAFileMenu (
    IN  FILE_MENU_CONTEXT   *MenuContext,
    IN  EFI_MENU            *DirMenu,
    IN  BOOLEAN             BootFromAFileSystem
    );

BOOLEAN
DirMenuBuilder (
    IN  FILE_MENU_CONTEXT   *MenuContext,
    IN  BOOLEAN             Filter,
    IN  EFI_MENU            *DirMenu
    );

VOID
FreeDirMenu (
    IN EFI_MENU     *Menu
    );

EFI_STATUS
Bmnt_DisplayConsoleInMenu(); 

EFI_STATUS
Bmnt_DisplayConsoleOutMenu(); 

EFI_STATUS
Bmnt_DisplayStdErrorMenu(); 

EFI_STATUS
Bmnt_DisplayBootOrderMenu (
    VOID
    );

EFI_STATUS
Bmnt_DisplayBootNextMenu (
    VOID
    ) ;

EFI_STATUS
BootTheOption (
    IN  FILE_MENU_CONTEXT   *Context,
    IN  BOOT_MENU_CONTEXT   *BootOption
    );

EFI_STATUS
DefaultBootTheBootOption (
    IN  FILE_MENU_CONTEXT   *Context,
    IN  BOOT_MENU_CONTEXT   *BootOption,
    OUT EFI_HANDLE          *ImageHandle
    );

UINTN
InitializeBootOrderMenuFromVariable (
    VOID
    );

VOID
FreeBootOrderMenuOfVariables (
    VOID
    );

EFI_STATUS
FindBootOption (
    IN  EFI_MENU            *Menu,
    IN OUT LIST_ENTRY   **List,
    IN  EFI_DEVICE_PATH     *DevicePath,
    OUT BOOT_MENU_CONTEXT   **ReturnBootContext
    );

BOOT_MENU_CONTEXT *
BuildBootMenuContext (
    IN  CHAR8   *BootOption,
    IN  UINTN   BootOptionSize,
    IN  UINT16  BootOptionNumber,
    IN  CHAR16  *BootString,
    IN  BOOLEAN BootNext,
    IN  BOOLEAN Modified
    );

BOOLEAN
MatchDevicePaths (
    IN  EFI_DEVICE_PATH *Multi,
    IN  EFI_DEVICE_PATH *Single
    );

EFI_STATUS
BootFromFileContext (
    IN  FILE_MENU_CONTEXT   *Context
    );

EFI_STATUS
ConvertFileContextToBootOption (
    IN  FILE_MENU_CONTEXT   *Context
    );

BOOLEAN
PrintEfiFileInfo (
    IN  FILE_MENU_CONTEXT    *Context
    );

VOID
BootPause (
    VOID
    );

VOID
BmntClearLine (
    VOID
    );

BOOLEAN
IsEfiImageName (
    CHAR16  *FileName
    );

BOOLEAN
PrintMenuHelp(
    IN  EFI_MENU    *Menu,
    IN  CHAR16      *Description
    );

CHAR16 *
ReturnUiString (
    EFI_HANDLE      Handle,
    EFI_DEVICE_PATH *DevicePath,
    CHAR16          *DevicePathStr
    );

extern CHAR16       *BootOrderMenuHelpStr;
extern CHAR16       *DeleteMenuHelpStr;
extern CHAR16       *BootNextMenuHelpStr;
extern CHAR16       *TimeoutMenuHelpStr;
extern EFI_HANDLE   GlobalImageHandle;
extern EFI_MENU     BootMenu;
#endif