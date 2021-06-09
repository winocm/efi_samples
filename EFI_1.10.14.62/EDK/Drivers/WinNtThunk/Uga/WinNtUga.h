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

  WinNtUga.h

Abstract:

  Private data for the Uga driver that is bound to the WinNt Thunk protocol 

--*/

#ifndef _WIN_NT_UGA_H_
#define _WIN_NT_UGA_H_

#include "EfiWinNT.h"
#include "Efi.h"
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
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (UgaIo)
#include "LinkedList.h"

#define MAX_Q 256

typedef struct {
  UINTN         Front;
  UINTN         Rear;
  UINTN         Count;
  EFI_INPUT_KEY Q[MAX_Q];
} UGA_QUEUE_FIXED;

#define WIN_NT_UGA_CLASS_NAME  L"WinNtUgaWindow"


#define UGA_PRIVATE_DATA_SIGNATURE EFI_SIGNATURE_32('S','g','o','N')
typedef struct {
  UINT64                              Signature;

  EFI_HANDLE                          Handle;
  EFI_UGA_DRAW_PROTOCOL               UgaDraw;
  EFI_UGA_IO_PROTOCOL                 UgaIo;
  EFI_SIMPLE_TEXT_IN_PROTOCOL         SimpleTextIn;

  EFI_WIN_NT_THUNK_PROTOCOL           *WinNtThunk;

  EFI_UNICODE_STRING_TABLE            *ControllerNameTable;
 
  //
  // UGA Private Data for GetMode ()
  //
  UINT32                              HorizontalResolution;
  UINT32                              VerticalResolution;
  UINT32                              ColorDepth;
  UINT32                              RefreshRate;

  //
  // UGA Private Data knowing when to start hardware
  //
  BOOLEAN                             HardwareNeedsStarting;


  CHAR16                              *WindowName;
  CHAR16                              Buffer[160];

  HANDLE                              ThreadInited;   // Semaphore
  HANDLE                              ThreadHandle;   // Thread
  DWORD                               ThreadId;

  HWND                                WindowHandle;
  WNDCLASSEX                          WindowsClass;

  //
  // This screen is used to redraw the scree when windows events happen. It's 
  // updated in the main thread and displayed in the windows thread.
  //
  BITMAPV4HEADER                      *VirtualScreenInfo;
  RGBQUAD                             *VirtualScreen;

  EFI_UGA_PIXEL                       *FillLine;

  //
  // Keyboard Queue used by Simple Text In. WinProc thread adds, and main 
  // thread removes.
  //
  CRITICAL_SECTION                    QCriticalSection;
  UGA_QUEUE_FIXED                     Queue;

} UGA_PRIVATE_DATA;

#define UGA_DRAW_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, UGA_PRIVATE_DATA, UgaDraw, UGA_PRIVATE_DATA_SIGNATURE)

#define UGA_PRIVATE_DATA_FROM_TEXT_IN_THIS(a)  \
         CR(a, UGA_PRIVATE_DATA, SimpleTextIn, UGA_PRIVATE_DATA_SIGNATURE)

//
// Global Protocol Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gWinNtUgaDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gWinNtUgaComponentName;

//
// Uga Hardware abstraction internal worker functions
//

EFI_STATUS
WinNtUgaSupported (
  IN  EFI_WIN_NT_IO_PROTOCOL  *WinNtIo
  );

EFI_STATUS
WinNtUgaConstructor (
  IN  UGA_PRIVATE_DATA    *Private
  );

EFI_STATUS
WinNtUgaDestructor (
  IN  UGA_PRIVATE_DATA    *Private
  );

//
// EFI 1.1 driver model prototypes for Win NT UGA
//

EFI_STATUS
WinNtUgaInitialize (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  );

EFI_STATUS
WinNtUgaDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  );

EFI_STATUS
WinNtUgaDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  );

EFI_STATUS
WinNtUgaDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

EFI_STATUS
UgaPrivateAddQ (
  IN  UGA_PRIVATE_DATA    *Private,
  IN  EFI_INPUT_KEY       Key
  );

EFI_STATUS
WinNtUgaInitializeSimpleTextInForWindow (
  IN  UGA_PRIVATE_DATA    *Private
  );

EFI_STATUS
WinNtUgaDestroySimpleTextInForWindow (
  IN  UGA_PRIVATE_DATA    *Private
  );

UINTN
Atoi (
  IN  CHAR16  *String
  );

#endif

