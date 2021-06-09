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

    help.c
    
Abstract:

    Contains help menu descriptions



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "menu.h"


CHAR16 *BootOrderMenuHelpStr = L"\
This menu helps in setting the boot order of options. Select a boot option \
using the arrow keys. Pressing 'U' or 'u' moves it up the order chain and \
pressing 'D' or 'd' moves it down the order chain. Save the settings to NVRAM \
(if needed) before exiting. ";

CHAR16 *DeleteMenuHelpStr = L"\
To delete a boot option, highlight that option using arrow keys and press 'ENTER'.\
Alternatively, highlight a boot option and press 'D' or 'd' on the keyboard. \
To confirm delete press 'Y' or 'N' at the prompt. To delete all boot options, \
select 'Delete All Boot Options' from the menu or 'A' or 'a' on the keyboard. \
To confirm delete press 'Y' or 'N' at the prompt. Save the settings to NVRAM \
(if needed) before exiting. ";

CHAR16 *BootNextMenuHelpStr = L"\
This menu helps in managing the 'BootNext' variable of options. \
Select a boot option using the arrow keys. Press 'ENTER' or 'B' or 'b' \
to make this option as 'BootNext'. To remove 'BootNext' setting , \
select 'Reset BootNext Setting' menu option or 'R' or 'r'. Save the \
settings to NVRAM (if needed) before exiting. ";

CHAR16 *TimeoutMenuHelpStr = L"\
This menu helps in managing the 'Timeout' variable. Use the 'Set Timeout Value'\
option to set time(in seconds) to boot the default (first in list) option. If \
a value of '0' is specified there is no wait to boot the default option. \
To disable use of the timeout variable, following three choices are available: \
1. Use the 'Delete/Disable Timeout' menu option to delete the timeout variable \
2. Set the timeout value to be 65535 (0xFFFF) \
3. Press a key when EFI is booting up and the timeout count down is disabled. \
The timeout value is saved to NVRAM when 'Set Timeout Value' menu option \
is chosen. " ;


BOOLEAN
PrintMenuHelp(
    IN  EFI_MENU    *Menu,
    IN  CHAR16      *Description
    )
{

    EFI_INPUT_KEY                   Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;
    UINTN                           EndRow;
    EFI_STATUS                      Status;
    UINTN                           MaxColumn, ScreenSize;
    UINTN                           Row,Col;
    UINTN                           Size, PrintSize;
    CHAR16                          *TempStr;
    CHAR16                          InputString[MAX_CHAR];

    ConOut = ST->ConOut;
    ConIn  = ST->ConIn;

    Status = ConOut->SetAttribute (ConOut, Menu->ScreenAttribute);
    ASSERT (!EFI_ERROR (Status));
    ConOut->EnableCursor(ConOut, FALSE);
    if (Menu->ClearScreen) {
        Status = ConOut->ClearScreen (ConOut);
        ASSERT (!EFI_ERROR (Status));
    }
    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxColumn, &ScreenSize);
    PrintAt(Menu->Col,1,L"%E%s\n",Menu->Header);
    ConOut->SetAttribute (ConOut, Menu->ScreenAttribute);

    Size = StrSize(Description)/sizeof(CHAR16);
    TempStr = Description;

    Col = MaxColumn - Menu->Col*2;
    EndRow = ScreenSize - Menu->FooterHeight - 1;

    for (Row = 3;Size > 0; ) {
        if (Size > Col) {
            PrintSize =  Col;

            //
            // Don't break up words
            //
            while (TempStr[PrintSize] != ' ') {
                PrintSize--;
                if (!PrintSize) {
                    break;
                }
            }
        } else {
            PrintSize = Size;
        }

        PrintAt(Menu->Col, Row, L"%-.*s", PrintSize, TempStr);
        Size -= PrintSize;
        TempStr = TempStr + PrintSize;   
        Row++;
        if ( Row == EndRow) {
            Status = ConOut->SetAttribute (ConOut, Menu->ReverseScreenAttribute);
            PrintAt(Menu->Col,Row,L"Press 'E' to Exit or any other key to Continue: ");
            Input (L"", InputString, sizeof(InputString));
            ConOut->SetAttribute (ConOut, Menu->ScreenAttribute);

            if (InputString[0] == 'E' || InputString[0] == 'e') { 
                return TRUE;
            } else {
                for (Row = 3; Row <= EndRow ; Row++) { 
                    PrintAt (Col, Row, L"");
                }
                Row = 3;
            }
        }           
    }

    Status = ConOut->SetAttribute (ConOut, Menu->ReverseScreenAttribute);
    PrintAt(Menu->Col,EndRow,L"Press any key to Continue: ");
    WaitForSingleEvent (ConIn->WaitForKey, 0);
    ConIn->ReadKeyStroke (ConIn, &Key);
    ConOut->SetAttribute (ConOut, Menu->ScreenAttribute);
    return TRUE;

}
