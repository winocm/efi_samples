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

    FileMenu.c

Abstract:

    EFI File manager



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "menu.h"
#include "pe.h"

CHAR16 *
AppendFileName (
    IN  CHAR16  *Str1,
    IN  CHAR16  *Str2
    );



BOOLEAN
FindAFileMenu (
    IN  FILE_MENU_CONTEXT   *MenuContext,
    IN  EFI_MENU            *DirMenu,
    IN  BOOLEAN             BootFromAFileSystem
    )
{
    FILE_MENU_CONTEXT   *ResultContext;
    FILE_MENU_CONTEXT   *Context;
    FILE_MENU_CONTEXT   ContextCopy;
    EFI_INPUT_KEY       Key;

    InitializeListHead (&DirMenu->Head);

    DirMenuBuilder (MenuContext, TRUE, DirMenu);

    Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
    ASSERT (Context);
    Context->MenuOption = FILE_MENU_EXIT;
    AllocateMenuOption (DirMenu, L"Exit", FILE_UNSELECTED, Context);

    for (;;) {
        DirMenu->Selection = NULL;
        ResultContext = MenuDisplay (DirMenu, &Key);
        if (ResultContext == NULL) {
            break;
        }
        if (ResultContext->MenuOption == FILE_MENU_EXIT) {
            Print (L"\n");
            break;
        }
        if (!ResultContext->IsDir) {
            if (BootFromAFileSystem) {
                if (!EFI_ERROR(BootFromFileContext (ResultContext))) {
                    break;
                }
            } else {
                ConvertFileContextToBootOption (ResultContext);
            }
        }
        CopyMem (&ContextCopy, ResultContext, sizeof(FILE_MENU_CONTEXT));
        ContextCopy.FileName = StrDuplicate (ResultContext->FileName);
        FreeDirMenu (DirMenu);

        if (!DirMenuBuilder (&ContextCopy, TRUE, DirMenu)) {
            break;            
        } else {
            Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
            ASSERT (Context);
            Context->MenuOption = FILE_MENU_EXIT;
            AllocateMenuOption (DirMenu, L"Exit", FILE_UNSELECTED, Context);
        }
        FreePool (ContextCopy.FileName);
    }
    return TRUE;
}

 
VOID
FreeDirMenu (
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
            if (MenuContext->FileName) {
                FreePool (MenuContext->FileName);
            }
            if (MenuOption->Context) {
                FreePool (MenuOption->Context);
            }
        }
        
        List = List->Flink;
    }
    MenuFree (Menu);
}

BOOLEAN
DirMenuBuilder (
    IN  FILE_MENU_CONTEXT   *MenuContext,
    IN  BOOLEAN             Filter,
    IN  EFI_MENU            *DirMenu
    )
{
    EFI_FILE_HANDLE         NewDir, Dir;
    EFI_FILE_INFO           *DirInfo;
    UINTN                   BufferSize, DirBufferSize;
    CHAR16                  Buffer[MAX_CHAR];
    EFI_STATUS              Status;
    FILE_MENU_CONTEXT       *Context;
    UINTN                   Pass;

    Dir = MenuContext->FHandle;
    Status = Dir->Open (Dir, &NewDir, MenuContext->FileName, EFI_FILE_MODE_READ, 0);
    if (!MenuContext->RootContext) {
        Dir->Close (Dir);
    }
    if (EFI_ERROR(Status)) {
        return FALSE;
    }

    DirInfo = LibFileInfo (NewDir);
    if (!DirInfo) {
        return FALSE;
    }
    if (!(DirInfo->Attribute & EFI_FILE_DIRECTORY)) {
        return FALSE;
    }
    MenuContext->DevicePath = FileDevicePath (MenuContext->Handle, MenuContext->FileName);

    DirBufferSize = sizeof(EFI_FILE_INFO) + 1024;
    DirInfo = AllocatePool (DirBufferSize);
    if (!DirInfo) {
        return FALSE;
    }

    DirMenu->BottomOfPage = NULL;
    for (Pass = 1; Pass <= 2; Pass++) {
        NewDir->SetPosition (NewDir, 0);
        for (;;) {
            BufferSize = DirBufferSize;
            Status = NewDir->Read (NewDir, &BufferSize, DirInfo);
            if (EFI_ERROR(Status) || BufferSize == 0) {
                break;
            }

            if ((DirInfo->Attribute & EFI_FILE_DIRECTORY && Pass == 2) ||
                (!(DirInfo->Attribute & EFI_FILE_DIRECTORY) && Pass == 1)) {
                //
                // Pass 1 is for Directories
                // Pass 2 is for file names
                //
                continue;
            }

            if (Filter) {
                if (!(IsEfiImageName (DirInfo->FileName) ||
                      DirInfo->Attribute & EFI_FILE_DIRECTORY) ) {
                    //
                    // Slip file unless it is a directory entry or a .EFI file
                    //
                    continue;
                }
            }

            SPrint (Buffer, MAX_CHAR, L"   %t %s %11,ld %s", 
                                &DirInfo->ModificationTime,
                                DirInfo->Attribute & EFI_FILE_DIRECTORY ? L"<DIR>" : L"     ",
                                DirInfo->FileSize,
                                DirInfo->FileName
                                );

            Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
            ASSERT (Context);
            Context->MenuOption = FILE_MENU_DYNAMIC_ENTRY;

            Context->Handle = MenuContext->Handle;
            Context->Info = MenuContext->Info;
            Context->FileName = AppendFileName (MenuContext->FileName ,DirInfo->FileName);
            Context->FHandle = NewDir;
            Context->DevicePath = FileDevicePath (Context->Handle, Context->FileName);
            Context->DevicePathStr = DevicePathToStr (Context->DevicePath);
            Context->IsDir = (DirInfo->Attribute & EFI_FILE_DIRECTORY) == EFI_FILE_DIRECTORY;
            AllocateMenuOption (DirMenu, Buffer, FILE_UNSELECTED, Context);
        }
    }

    if (MenuContext->RootContext) {
        //
        // For the root menu add an option to try a removable media 
        // device behavior. That is \efi\boot\boot[ia32].efi
        //
        Context = AllocateZeroPool (sizeof(FILE_MENU_CONTEXT));
        ASSERT (Context);
        Context->MenuOption = FILE_MENU_DYNAMIC_ENTRY;

        Context->Handle = MenuContext->Handle;
        Context->Info = MenuContext->Info;
        Context->IsRemovableMedia = TRUE;
        Context->FileName = StrDuplicate (L"");
        Context->FHandle = NewDir;
        Context->DevicePath = DevicePathFromHandle (Context->Handle);
        Context->DevicePathStr = DevicePathToStr (Context->DevicePath);
        AllocateMenuOption (DirMenu, L"   [Treat like Removable Media Boot]", FILE_UNSELECTED, Context);
    }

    FreePool (DirInfo);
    return TRUE;
}

CHAR16 *
AppendFileName (
    IN  CHAR16  *Str1,
    IN  CHAR16  *Str2
    )
{
    UINTN   Size1, Size2;
    CHAR16  *Str;
    CHAR16  *Ptr;
    CHAR16  *LastSlash;

    Size1 = StrSize(Str1);
    Size2 = StrSize(Str2);
    Str = AllocateZeroPool (Size1 + Size2 + sizeof(CHAR16));

    StrCat (Str, Str1);
    if ( !((*Str == '\\') && (*(Str + 1) == 0)) ) {
        StrCat (Str, L"\\");
    }
    StrCat (Str, Str2);

    Ptr = Str;
    LastSlash = Str;
    while (*Ptr != 0) {
        if (*Ptr == '\\' && *(Ptr+1) == '.' && *(Ptr+2) == '.' && *(Ptr+3) != 0) {
            //
            // Convert \Name\..\ to \
            //  DO NOT convert the .. if it is at the end of the string. This will
            //  break the .. behavior in chaning directories.
            //
            StrCpy (LastSlash, Ptr+3);
            Ptr = LastSlash;
        } else if (*Ptr == '\\' && *(Ptr+1) == '.' && *(Ptr + 2) == '\\') {
            //
            // Convert a \.\ to a \
            //
            StrCpy (Ptr, Ptr+2);
            Ptr = LastSlash;
        } else if (*Ptr == '\\') {
            LastSlash = Ptr;
        }
        Ptr++;
    }
    return Str;
}


BOOLEAN
PrintEfiFileInfo (
    IN  FILE_MENU_CONTEXT    *Context
    )
{
    EFI_FILE_HANDLE         NewDir, Dir;
    EFI_STATUS              Status;
    UINTN                   BufferSize;
    EFI_FILE_INFO           *FileInfo;
    IMAGE_DOS_HEADER        DosHdr;
    IMAGE_NT_HEADERS        PeHdr;

    Dir = Context->FHandle;
    if (!Dir) {
        Print (L"\n    Device Path %s\n", Context->DevicePathStr);
        return FALSE;
    }

    if (Context->FileName == NULL) {
      return FALSE;
    }
    Status = Dir->Open (Dir, &NewDir, Context->FileName, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        return FALSE;
    }
    BufferSize = sizeof(IMAGE_DOS_HEADER);
    NewDir->Read (NewDir, &BufferSize, &DosHdr);
    if (DosHdr.e_magic != IMAGE_DOS_SIGNATURE) {
        NewDir->Close (NewDir);
        return FALSE;
    }

    NewDir->SetPosition (NewDir, DosHdr.e_lfanew);
    BufferSize = sizeof(IMAGE_NT_HEADERS);
    NewDir->Read (NewDir, &BufferSize, &PeHdr);
    if (PeHdr.Signature != IMAGE_NT_SIGNATURE) {
        NewDir->Close (NewDir);
        return FALSE;
    }

    Print (L"\n    Filename: %s \n  DevicePath: [%s]\n    ", Context->FileName, Context->DevicePathStr);
    switch (PeHdr.FileHeader.Machine) {
    case EFI_IMAGE_MACHINE_IA32:
        Print (L"IA-32 ");
        break;
    case EFI_IMAGE_MACHINE_IA64:
        Print (L"IA-64 ");
        break;
    case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
        Print (L"fcode ");
        break;
    default:
        Print (L"??-?? ");
    }

    switch (PeHdr.OptionalHeader.Subsystem) {
    case IMAGE_SUBSYSTEM_EFI_APPLICATION:
        Print (L"EFI Application ");
        break;
    case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
        Print (L"EFI Boot Service Driver ");
        break;
    case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
        Print (L"EFI Runtime Driver ");
        break;
    default:
        Print (L"NON EFI IMAGE ");
    }

    FileInfo = LibFileInfo (NewDir);
    if (FileInfo) {
        Print (L"%N%t %11,ld bytes\n\n", 
                                &FileInfo->ModificationTime,
                                FileInfo->FileSize
                                );
        FreePool (FileInfo);
    }
    NewDir->Close (NewDir);
    return TRUE;
}
    