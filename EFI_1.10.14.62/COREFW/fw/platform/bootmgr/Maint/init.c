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

    init.c
    
Abstract:

    init file for boot maintenance menu option of boot manager.



Revision History

--*/

#include "menu.h"

EFI_MENU MainMenu = {
    MENU_SIGNATURE, NULL,  
    4, FILE_UNSELECTED, FILE_ACTIVE_SELECTION, 
    TRUE, 10, 
    L"%EMain Menu. Select an Operation\n\n", 
    NULL, 3, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};

EFI_MENU FileSystemMenu = {
    MENU_SIGNATURE, NULL,  
    4, FILE_UNSELECTED, FILE_ACTIVE_SELECTION,  
    TRUE, 10, 
//    L"%ESelect a volume\n\n", 
    NULL,
    NULL, 2, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};

EFI_MENU DirMenu = {
    MENU_SIGNATURE, NULL,  
    4, FILE_UNSELECTED, FILE_ACTIVE_SELECTION,  
    TRUE, 10, 
    L"%ESelect file or change to new directory:\n", 
    NULL, 2, 
    NULL, NULL, NULL, NULL, NULL, FALSE, 0, MENU_ENTRY_FORMAT
};


VOID
FreeFileSystemMenu (
    IN EFI_MENU     *Menu
    );

EFI_STATUS
Bmnt_DisplayWarmReset ();

EFI_STATUS
Bmnt_DisplayColdReset (); 


VOID
InitializeMainMenuFooter();

VOID
FreeMainMenuFooter();

VOID
AddLegacyBootOption (
    IN  UINT16  BbsType,
    IN  UINT16  BbsFlag,
    IN  UINT8   *BbsString,
    IN  UINT16  *String,
    IN  BOOLEAN Removable
    );

FILE_MENU_CONTEXT *
AllocateSpecialFileMenuOption (
    IN EFI_MENU     *Menu,
    IN CHAR16       *String,
    IN UINTN        Attribute,
    IN UINTN        Option
    );

VOID
FreeSpecificFileMenuOption (
    IN EFI_MENU             *Menu,
    IN FILE_MENU_CONTEXT    *Context
    );
//
//
//

EFI_STATUS
InitializeBootMaintenance (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

//
//
//
EFI_HANDLE   GlobalImageHandle;


EFI_DRIVER_ENTRY_POINT(InitializeBootMaintenance)

EFI_STATUS
InitializeBootMaintenance (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    MENU_PICK       MenuPick;
    UINTN           PreviousAttribute;
    EFI_INPUT_KEY   Key;
    
    //
    // Initialize EFI library
    //

    InitializeLib (ImageHandle, SystemTable);

    //
    // Disable the watchdog timer
    //

    BS->SetWatchdogTimer ( 0x0000, 0x0000, 0x0000, NULL );

    GlobalImageHandle = ImageHandle;

    PreviousAttribute = ST->ConOut->Mode->Attribute;

    //
    // Init Boot Order menu to allow other menus to access the Boot variables 
    //
    InitializeBootOrderMenuFromVariable ();


    InitializeListHead (&MainMenu.Head);
    AllocateMenuOption (&MainMenu, L"    Boot from a File", FILE_UNSELECTED, Bmnt_DisplayFileSystemBootFromFsMenu);
    AllocateMenuOption (&MainMenu, L"    Add a Boot Option", FILE_UNSELECTED, Bmnt_DisplayBootMenuOption);
    AllocateMenuOption (&MainMenu, L"    Delete Boot Option(s)", FILE_UNSELECTED, Bmnt_DisplayDeleteBootMenu);
    AllocateMenuOption (&MainMenu, L"    Change Boot Order", FILE_UNSELECTED, Bmnt_DisplayBootOrderMenu);

    SkipMenuLine(&MainMenu);

    AllocateMenuOption (&MainMenu, L"    Manage BootNext setting", FILE_UNSELECTED, Bmnt_DisplayBootNextMenu);
    AllocateMenuOption (&MainMenu, L"    Set Auto Boot TimeOut", FILE_UNSELECTED, Bmnt_DisplayBootTimeOut);

    SkipMenuLine(&MainMenu);

    AllocateMenuOption (&MainMenu, L"    Select Active Console Output Devices", FILE_UNSELECTED, Bmnt_DisplayConsoleOutMenu);
    AllocateMenuOption (&MainMenu, L"    Select Active Console Input Devices", FILE_UNSELECTED, Bmnt_DisplayConsoleInMenu);
    AllocateMenuOption (&MainMenu, L"    Select Active Standard Error Devices", FILE_UNSELECTED, Bmnt_DisplayStdErrorMenu);

    SkipMenuLine(&MainMenu);

    //    AllocateMenuOption (&MainMenu, L"    Warm Reset", FILE_UNSELECTED, DisplayWarmReset);
    AllocateMenuOption (&MainMenu, L"    Cold Reset", FILE_UNSELECTED, Bmnt_DisplayColdReset);
    AllocateMenuOption (&MainMenu, L"    Exit", FILE_UNSELECTED, NULL);

    MainMenu.Selection = NULL;
    for (;;) {
        //
        // bugbug: Not the best way to update timeout on main menu screen
        // Init main menu footer with SystemGuid and SerialNumber and Timeout value
        //
        InitializeMainMenuFooter ();
        
        MenuPick = MenuDisplay (&MainMenu, &Key);
        if (MenuPick == NULL) {
            break;
        }
        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            MenuPick();
        }

        //
        // Free main menu footer 
        //
        FreeMainMenuFooter ();
    }
    
    MenuFree (&MainMenu);

    //
    // Free Boot Order menu 
    //
    FreeBootOrderMenuOfVariables();


    ST->ConOut->SetAttribute (ST->ConOut, PreviousAttribute);
    ST->ConOut->ClearScreen (ST->ConOut);

    return EFI_SUCCESS;
}

EFI_STATUS
Bmnt_DisplayFileSystemBootFromFsMenu() 
{
    CHAR16 *Header = L"%EBoot From a File.  Select a Volume\n\n";

    // 
    // initialize header text
    //
    FileSystemMenu.Header = Header;
    return DisplayFileSystemMenu (TRUE);
}

EFI_STATUS
Bmnt_DisplayBootMenuOption() 
{
    CHAR16 *Header = L"%EAdd a Boot Option.  Select a Volume\n\n";

    // 
    // initialize header text
    //
    FileSystemMenu.Header = Header;
    return DisplayFileSystemMenu (FALSE);
}


EFI_STATUS
DisplayFileSystemMenu (
    IN  BOOLEAN BootFromAFileSystem
    ) 
{
    CHAR16              Buffer[MAX_CHAR];
    UINTN               Index;
    UINTN               NumberFsHandles;
    EFI_HANDLE          *FsHandles;
    UINTN               NumberBlkIoHandles;
    EFI_HANDLE          *BlkIoHandles;
    UINTN               NumberLoadFileHandles;
    EFI_HANDLE          *LoadFileHandles;
    FILE_MENU_CONTEXT   *ResultContext;
    FILE_MENU_CONTEXT   *Context;
    CHAR16              *VolLabel;
    EFI_INPUT_KEY       Key;
    EFI_BLOCK_IO        *BlkIo;
    EFI_STATUS          Status;
    CHAR16              *Str;
    EFI_FILE_HANDLE     FileHandle;
    VOID                *Interface;

    InitializeListHead (&FileSystemMenu.Head);

    // 
    // add filesystem devices 
    //
    LibLocateHandle (ByProtocol, &FileSystemProtocol, NULL, &NumberFsHandles, &FsHandles);

    FileSystemMenu.BottomOfPage = NULL;
    for (Index = 0; Index < NumberFsHandles; Index++) {
        Status = BS->HandleProtocol (FsHandles[Index], &BlockIoProtocol, &BlkIo);
        //
        // add searching of files for non-removable media only
        // removable media has a fixed file path (\efi\boot) for searching
        //
        if(!EFI_ERROR(Status) && BlkIo->Media->RemovableMedia) {
            if (!BootFromAFileSystem) {
                continue;
            }
        }

        FileHandle = LibOpenRoot(FsHandles[Index]);
        if (FileHandle == NULL) {
            continue;
        }

        Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
        ASSERT (Context);
        Context->Handle = FsHandles[Index];
        Context->FHandle = FileHandle;
        Context->DevicePathStr = DevicePathToStr (DevicePathFromHandle (Context->Handle));
        Context->Info = LibFileSystemVolumeLabelInfo (Context->FHandle);
        Context->FileName = StrDuplicate (L"\\");
        Context->DevicePath = FileDevicePath (Context->Handle, Context->FileName);

        Context->IsDir = TRUE;
        Context->RootContext = TRUE;
        Context->IsRemovableMedia = FALSE;
        if (Context->Info == NULL) {
            VolLabel = L"NO FILE SYSTEM INFO";         
        } else {
            if (Context->Info->VolumeLabel == NULL) {
                VolLabel = L"NULL VOLUME LABEL"; 
            } else {
                VolLabel = Context->Info->VolumeLabel;
                if (*VolLabel == 0x0000) {
                    VolLabel = L"NO VOLUME LABEL"; 
                }
            }
        } 
        Str = LibGetUiString (Context->Handle, UiDeviceString, LanguageCodeEnglish, FALSE);
        if (!Str) {
            Str = Context->DevicePathStr;
        }
        SPrint (Buffer, MAX_CHAR, L"%s [%s]", VolLabel, Str);
        AllocateMenuOption (&FileSystemMenu, Buffer, FILE_UNSELECTED, Context);
    }
    if(NumberFsHandles) {
        FreePool (FsHandles);
    }

    // 
    // add removable media block devices 
    // (Includes fixed media with removable media boot algorithm
    //
    LibLocateHandle (ByProtocol, &BlockIoProtocol, NULL, &NumberBlkIoHandles, &BlkIoHandles);
    for (Index = 0; Index < NumberBlkIoHandles; Index++) {
        Status = BS->HandleProtocol (BlkIoHandles[Index], &BlockIoProtocol, &BlkIo);
        if (EFI_ERROR(Status)) {
            continue;
        }
        if (BlkIo->Media->LogicalPartition || !BlkIo->Media->RemovableMedia) {
            // Skip Logical Paritions and NON removable media devices
            continue;
        }

        Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
        ASSERT (Context);
        Context->IsRemovableMedia = TRUE;
        Context->Handle = BlkIoHandles[Index];

        Context->DevicePath = DevicePathFromHandle (Context->Handle);
        Context->DevicePathStr = DevicePathToStr (Context->DevicePath);

        Str = LibGetUiString (Context->Handle, UiDeviceString, LanguageCodeEnglish, TRUE);
        SPrint (Buffer, MAX_CHAR, L"Removable Media Boot [%s]", Str);
        AllocateMenuOption (&FileSystemMenu, Buffer, FILE_UNSELECTED, Context);
    }
    if(NumberBlkIoHandles) {
        FreePool (BlkIoHandles);
    }

    // 
    // add loadfileprotocol devices as removable media
    //
    LibLocateHandle (ByProtocol, &LoadFileProtocol, NULL, &NumberLoadFileHandles, &LoadFileHandles);
    for (Index = 0; Index < NumberLoadFileHandles; Index++) {

        Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
        ASSERT (Context);
        Context->IsRemovableMedia = FALSE;
        Context->IsLoadFile = TRUE;
        Context->Handle = LoadFileHandles[Index];

        Context->DevicePath = DevicePathFromHandle (Context->Handle);
        Context->DevicePathStr = DevicePathToStr (Context->DevicePath);

        Str = LibGetUiString (Context->Handle, UiDeviceString, LanguageCodeEnglish, TRUE);
        SPrint (Buffer, MAX_CHAR, L"Load File [%s]", Str);
        AllocateMenuOption (&FileSystemMenu, Buffer, FILE_UNSELECTED, Context);
    }

    if(NumberLoadFileHandles) {
        FreePool (LoadFileHandles);
    }

    //
    // Add Legacy boot support as well
    //
    Status = LibLocateProtocol (&LegacyBootProtocol, &Interface);
    if (!EFI_ERROR (Status)) {
      AddLegacyBootOption (BBS_TYPE_FLOPPY, 0, "", L"Legacy Boot", TRUE);
    }

    AllocateMenuOption (&FileSystemMenu, L"Exit", FILE_UNSELECTED, NULL);

    FileSystemMenu.Selection = NULL;
    for (;;) {
        ResultContext = MenuDisplay (&FileSystemMenu, &Key);
        if (ResultContext == NULL) {
            break;
        }       

        if (ResultContext->IsRemovableMedia || ResultContext->IsLoadFile) {
            if (BootFromAFileSystem) {
                if (!EFI_ERROR(BootFromFileContext (ResultContext))) {
                    break;
                }
            } else {
                ConvertFileContextToBootOption (ResultContext);
            }
        } else if (!FindAFileMenu (ResultContext, &DirMenu, BootFromAFileSystem)) {
            break;
        }
    }

    FreeFileSystemMenu (&FileSystemMenu);

    ST->ConOut->ClearScreen (ST->ConOut);
    Print (L"\n");
    return EFI_SUCCESS;
}

VOID
AddLegacyBootOption (
    IN  UINT16  BbsType,
    IN  UINT16  BbsFlag,
    IN  UINT8   *BbsString,
    IN  UINT16  *String,
    IN  BOOLEAN Removable
    )
{
    EFI_DEVICE_PATH     *DevicePath;
    UINTN               BbsStrLen;
    EFI_DEV_PATH        Node;
    FILE_MENU_CONTEXT   *Context;    

    ZeroMem (&Node, sizeof(Node));
    Node.DevPath.Type = BBS_DEVICE_PATH;
    Node.DevPath.SubType = BBS_BBS_DP;

    BbsStrLen = strlena(BbsString);
    SetDevicePathNodeLength (&Node.DevPath, sizeof(BBS_BBS_DEVICE_PATH) + BbsStrLen);
    Node.Bbs.DeviceType = BbsType;
    Node.Bbs.StatusFlag = BbsFlag;
    CopyMem (&Node.Bbs.String[0], BbsString, BbsStrLen + 1);
   
    DevicePath = AppendDevicePathNode (EndDevicePath, &Node.DevPath);

    Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
    ASSERT (Context);
    Context->IsRemovableMedia = Removable;
    Context->IsLoadFile = TRUE;
    Context->DevicePath = DuplicateDevicePath(DevicePath);
    Context->DevicePathStr = DevicePathToStr (Context->DevicePath);
    AllocateMenuOption (&FileSystemMenu, String, FILE_UNSELECTED, Context);

    FreePool (DevicePath);
}    
VOID
FreeFileSystemMenu (
    IN EFI_MENU     *Menu
    )
{
    MENU_OPTION         *MenuOption;
    FILE_MENU_CONTEXT   *MenuContext;
    LIST_ENTRY          *List;   
    //
    // Delete the menu option's Context
    //
    List = Menu->Head.Flink;
    while (List != &Menu->Head) {
        MenuOption = CR(List, MENU_OPTION, Link, MENU_OPTION_SIGNATURE);
        
        MenuContext = (FILE_MENU_CONTEXT *)MenuOption->Context;
        if (MenuContext != NULL) {
            if (MenuContext->FHandle) {
                MenuContext->FHandle->Close (MenuContext->FHandle);
            }
            FreePool (MenuContext->DevicePathStr);
            if (MenuContext->FileName) {
                FreePool (MenuContext->FileName);
            }
            FreePool (MenuOption->Context);
        }
        
        List = List->Flink;
    }
    MenuFree (Menu);
}


EFI_STATUS
Bmnt_DisplayWarmReset () 
{
    RT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
    return EFI_DEVICE_ERROR;
}

EFI_STATUS
Bmnt_DisplayColdReset () 
{
    RT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    return EFI_DEVICE_ERROR;
}

VOID
InitializeMainMenuFooter()
{
    EFI_STATUS  Status;
    EFI_GUID    SystemGuid;
    CHAR8       *SerialNumber;
    UINT16      Timeout;
    UINTN       TimoutSize;
    UINTN       SprintSize;
    UINT16      *Str;
    CHAR16      Buffer[240];

    Str = Buffer;
    *Str = L'\0';

    TimoutSize = sizeof(Timeout);
    Status = RT->GetVariable (
                VarTimeout,
                &EfiGlobalVariable,
                NULL,
                &TimoutSize,
                &Timeout
                );
    if (!EFI_ERROR(Status)) {
        SprintSize = SPrint (Str, 240, L"Timeout-->[%hd] sec ", Timeout);
        Str += SprintSize;
    }

    Status = LibGetSmbiosSystemGuidAndSerialNumber (&SystemGuid, &SerialNumber);
    if (!EFI_ERROR(Status)) {
        SprintSize = SPrint (Str, 240, L"SystemGuid-->[%hg]\n    SerialNumber-->[%ha] ", &SystemGuid, SerialNumber);
        Str += SprintSize;    
    }

    MainMenu.Footer = PoolPrint(Buffer);
}

VOID
FreeMainMenuFooter()
{
    if (MainMenu.Footer) {
        FreePool(MainMenu.Footer);
        MainMenu.Footer = NULL;
    }
}

VOID
PrintBanner (
    VOID
    )
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *Con;

    //
    // Clear the display
    //

    Con = ST->ConOut;
    Con->SetAttribute (Con, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    Con->ClearScreen (Con);

    Print(L"%EEFI Boot Maintenance Manager ver %01d.%02d [%d.%d]\n\n", ST->Hdr.Revision >> 16, ST->Hdr.Revision & 0xffff, ST->FirmwareRevision >> 16,ST->FirmwareRevision & 0xffff);
}


