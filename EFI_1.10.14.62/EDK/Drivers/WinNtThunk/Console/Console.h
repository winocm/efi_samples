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

  Console.h

Abstract:

  Console based on Win32 APIs.

  This file attaches a SimpleTextIn protocol to a previously open window.
  
  The constructor for this protocol depends on an open window. Currently
  the SimpleTextOut protocol creates a window when it's constructor is called.
  Thus this code must run after the constructor for the SimpleTextOut 
  protocol
  
--*/

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "EfiWinNt.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (WinNtIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)

#define WIN_NT_SIMPLE_TEXT_PRIVATE_DATA_SIGNATURE \
          EFI_SIGNATURE_32('N','T','s','c')

typedef struct {
  UINT64                            Signature;
                                    
  EFI_HANDLE                        Handle;
                                    
  EFI_SIMPLE_TEXT_OUT_PROTOCOL      SimpleTextOut;
  EFI_SIMPLE_TEXT_OUTPUT_MODE       SimpleTextOutMode;

  EFI_WIN_NT_IO_PROTOCOL            *WinNtIo;
  EFI_WIN_NT_THUNK_PROTOCOL         *WinNtThunk;

  //
  // SimpleTextOut Private Data including Win32 types.
  //
  HANDLE                            NtOutHandle;
  HANDLE                            NtInHandle;
                                    
  COORD                             MaxScreenSize;
  COORD                             Possition;
  WORD                              Attribute;
  BOOLEAN                           CursorEnable;

  EFI_SIMPLE_TEXT_IN_PROTOCOL       SimpleTextIn;

  EFI_UNICODE_STRING_TABLE           *ControllerNameTable;
  
} WIN_NT_SIMPLE_TEXT_PRIVATE_DATA;

#define WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, WIN_NT_SIMPLE_TEXT_PRIVATE_DATA, SimpleTextOut, WIN_NT_SIMPLE_TEXT_PRIVATE_DATA_SIGNATURE)

#define WIN_NT_SIMPLE_TEXT_IN_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, WIN_NT_SIMPLE_TEXT_PRIVATE_DATA, SimpleTextIn, WIN_NT_SIMPLE_TEXT_PRIVATE_DATA_SIGNATURE)

//
// Console Globale Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gWinNtConsoleDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gWinNtConsoleComponentName;

typedef struct {
  UINTN   ColumnsX;
  UINTN   RowsY;
} WIN_NT_SIMPLE_TEXT_OUT_MODE;


//
// Simple Text Out protocol member functions
//

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutReset (
  IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                          ExtendedVerification
  );


STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutOutputString (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN CHAR16                     *String
    );

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutTestString (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN CHAR16                     *String
    );

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutQueryMode (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                      ModeNumber,
    OUT UINTN                     *Columns,
    OUT UINTN                     *Rows
    );

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutSetMode (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                      ModeNumber
    );

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutSetAttribute (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                      Attribute
    );

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutClearScreen (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This
    );

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutSetCursorPosition (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                      Column,
    IN UINTN                      Row
    );

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutEnableCursor (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN BOOLEAN                    Enable
    );

//
// Simple Text Out constructor and destructor.
//

EFI_STATUS
WinNtSimpleTextOutOpenWindow (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private
  );

EFI_STATUS
WinNtSimpleTextOutCloseWindow (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Console
  );




//
// Simple Text In protocol member functions.
//


STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextInReset (
  IN struct _EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN BOOLEAN                              ExtendedVerification
  );

STATIC
EFI_STATUS
EFIAPI 
WinNtSimpleTextInReadKeyStroke (
    IN  struct _EFI_SIMPLE_TEXT_IN_PROTOCOL *This,
    OUT EFI_INPUT_KEY                       *Key
    );

STATIC
VOID
EFIAPI
WinNtSimpleTextInWaitForKey (
    IN EFI_EVENT          Event,
    IN VOID               *Context
  );

//
// Simple Text In constructor
//

EFI_STATUS
WinNtSimpleTextInAttachToWindow (
  IN  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private
  );


//
// Main Entry Point
//

EFI_STATUS
InitializeWinNtConsole (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  );

EFI_STATUS
AppendDevicePathInstanceToVar (
  IN  CHAR16                    *VariableName,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInstance
  );


#endif
