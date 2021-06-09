/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  WinNtBusDriver.h

Abstract:

This following section documents the envirnoment variables for the Win NT 
build.  These variables are used to define the (virtual) hardware 
configuration of the NT environment

A ! can be used to seperate multiple instances in a variable. Each 
instance represents a seperate hardware device. 

EFI_WIN_NT_PHYSICAL_DISKS - maps to drives on your system
EFI_WIN_NT_VIRTUAL_DISKS  - maps to a device emulated by a file
EFI_WIN_NT_FILE_SYSTEM    - mouts a directory as a file system
EFI_WIN_NT_CONSOLE        - make a logical comand line window (only one!)
EFI_WIN_NT_UGA            - Builds UGA Windows of Width and Height
EFI_WIN_NT_SERIAL_PORT    - maps physical serial ports

 <F>ixed       - Fixed disk like a hard drive.
 <R>emovable   - Removable media like a floppy or CD-ROM.
 Read <O>nly   - Write protected device.
 Read <W>rite  - Read write device.
 <block count> - Decimal number of blocks a device supports.
 <block size>  - Decimal number of bytes per block.

 NT envirnonment variable contents. '<' and '>' are not part of the variable, 
 they are just used to make this help more readable. There should be no 
 spaces between the ';'. Extra spaces will break the variable. A '!' is  
 used to seperate multiple devices in a variable.

 EFI_WIN_NT_VIRTUAL_DISKS = 
   <F | R><O | W>;<block count>;<block size>[!...]

 EFI_WIN_NT_PHYSICAL_DISKS =
   <drive letter>:<F | R><O | W>;<block count>;<block size>[!...]

 Virtual Disks: These devices use a file to emulate a hard disk or removable
                media device. 
                
   Thus a 20 MB emulated hard drive would look like:
   EFI_WIN_NT_VIRTUAL_DISKS=FW;40960;512

   A 1.44MB emulated floppy with a block size of 1024 would look like:
   EFI_WIN_NT_VIRTUAL_DISKS=RW;1440;1024

 Physical Disks: These devices use NT to open a real device in your system

   Thus a 120 MB floppy would look like:
   EFI_WIN_NT_PHYSICAL_DISKS=B:RW;245760;512

   Thus a standard CD-ROM floppy would look like:
   EFI_WIN_NT_PHYSICAL_DISKS=Z:RO;307200;2048

 EFI_WIN_NT_FILE_SYSTEM = 
   <directory path>[!...]

   Mounting the two directories C:\FOO and C:\BAR would look like:
   EFI_WIN_NT_FILE_SYSTEM=c:\foo!c:\bar

 EFI_WIN_NT_CONSOLE = 
   <window title>

   Declaring a text console window with the title "My EFI Console" woild look like:
   EFI_WIN_NT_CONSOLE=My EFI Console

 EFI_WIN_NT_UGA = 
   <width> <height>[!...]

   Declaring a two UGA windows with resolutions of 800x600 and 1024x768 would look like:
   Example : EFI_WIN_NT_UGA=800 600!1024 768

 EFI_WIN_NT_SERIAL_PORT = 
   <port name>[!...]

   Declaring two serial ports on COM1 and COM2 would look like:
   Example : EFI_WIN_NT_SERIAL_PORT=COM1!COM2

---*/

#ifndef _NT_BUS_DRIVER_H_
#define _NT_BUS_DRIVER_H_

#include "EfiWinNt.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (WinNtThunk)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (WinNtIo)

//
// WinNt Bus Driver Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gWinNtBusDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gWinNtBusDriverComponentName;

//
// WinNt Bus Controller Structure
//
#define WIN_NT_BUS_DEVICE_SIGNATURE  EFI_SIGNATURE_32('N','T','B','D')

typedef struct {
  UINT64                    Signature;
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;
} WIN_NT_BUS_DEVICE;

//
// WinNt Child Device Controller Structure
//
#define WIN_NT_IO_DEVICE_SIGNATURE  EFI_SIGNATURE_32('N','T','V','D')

typedef struct {
  UINT64                            Signature;
  EFI_HANDLE                        Handle;
  EFI_WIN_NT_IO_PROTOCOL            WinNtIo;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;

  //
  // Private data about the parent
  //
  EFI_HANDLE                        ControllerHandle;
  EFI_DEVICE_PATH_PROTOCOL          *ParentDevicePath;

  EFI_UNICODE_STRING_TABLE          *ControllerNameTable;

} WIN_NT_IO_DEVICE;

#define WIN_NT_IO_DEVICE_FROM_THIS(a) \
  CR(a, WIN_NT_IO_DEVICE, WinNtIo, WIN_NT_IO_DEVICE_SIGNATURE)

//
// This is the largest env variable we can parse
//
#define MAX_NT_ENVIRNMENT_VARIABLE_LENGTH 512

typedef struct _NT_ENVIRONMENT_VARIABLE_ENTRY {
  CHAR16      *Variable;
  EFI_GUID    *DevicePathGuid;
} NT_ENVIRONMENT_VARIABLE_ENTRY; 

typedef struct {
  VENDOR_DEVICE_PATH  VendorDevicePath;
  UINT32              Instance;
} WIN_NT_VENDOR_DEVICE_PATH_NODE;

//
// Driver Binding Protocol function prototypes
//
EFI_STATUS
WinNtBusDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Handle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
WinNtBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ParentHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
WinNtBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

//
// WinNt Bus Driver private worker functions
//
EFI_DEVICE_PATH_PROTOCOL *
WinNtBusCreateDevicePath ( 
  IN  EFI_DEVICE_PATH_PROTOCOL  *RootDevicePath,
  IN  EFI_GUID                  *Guid,
  IN  UINT16                    InstanceNumber
  );

#endif
