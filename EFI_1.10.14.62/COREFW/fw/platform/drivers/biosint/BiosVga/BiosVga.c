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

  BiosVga.c
    
Abstract:

  ConsoleOut Routines that speak VGA.

Revision History

--*/

#include "BiosVga.h"

//
// EFI Driver Binding Protocol Instance
//
//   This driver has a version value of 0x00000000.  This is the
//   lowest possible priority for a driver.  This is done on purpose to help
//   the developers of UGA drivers.  This driver can bind if no UGA driver 
//   is present, so a console is available.  Then, when a UGA driver is loaded
//   this driver can be disconnected, and the UGA driver can be connected.
//   As long as the UGA driver has a version value greater than 0x00000000, it
//   will be connected first and will block this driver from connecting.
//
EFI_DRIVER_BINDING_PROTOCOL gBiosVgaDriverBinding = {
  BiosVgaDriverBindingSupported,
  BiosVgaDriverBindingStart,
  BiosVgaDriverBindingStop,
  0x00000000,
  NULL,
  NULL
};  

//
// Private driver variables
//
static CHAR16 CrLfString[3] = { CHAR_CARRIAGE_RETURN, CHAR_LINEFEED, CHAR_NULL };

//
// Private worker functions
//
static 
CHAR16 *
UnicodeToASCII (
  IN  CHAR16  *WString,
  OUT CHAR8   *String,
  IN  UINTN   MaxSize
  );

static
VOID
SetVideoRows(
  INTN Mode
  );

static 
VOID
SetVideoMode(
  INTN Mode
  );

static 
VOID 
GetVideoCursorPosition(
  OUT INT32   *Column,
  OUT INT32   *Row
  );

static 
VOID
WriteVgaString(
  IN  INT32   Column,
  IN  INT32   Row,
  IN  INT32   Color,
  IN  CHAR8   *String
  );

static 
VOID 
SetVideoCursorPosition(
  IN  UINTN   Column,
  IN  UINTN   Row
  );

static
BOOLEAN
BiosVgaLibIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi,    OPTIONAL
  OUT CHAR8   *Ascii      OPTIONAL
  );

static
BOOLEAN
BiosVgaLibIsValidAscii (
  IN  CHAR16  Ascii
  );

static
BOOLEAN
BiosVgaLibIsValidEfiCntlChar (
  IN  CHAR16  c
  );

static
UINTN
strlena (
  IN CHAR8    *s1
  );

//
// Driver Entry Point
//  

EFI_DRIVER_ENTRY_POINT(BiosVgaDriverEntryPoint)
    
EFI_STATUS
BiosVgaDriverEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  ) 
/*++
  
  Routine Description:
    Driver Entry Point.
        
  Arguments:
    (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)
  
  Returns:
    EFI_STATUS
--*/                
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gBiosVgaDriverBinding,
           ImageHandle,
           &gBiosVgaComponentName,
           NULL,
           NULL
           );
} 

//
// EFI Driver Binding Protocol Functions
//

EFI_STATUS
BiosVgaDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Supported.
    
  Arguments:
    (Standard DriverBinding Protocol Supported() function)
    
  Returns:
    EFI_STATUS
  
--*/                
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PCI_TYPE00           Pci;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,           
                  &gEfiPciIoProtocolGuid, 
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // See if this is a PCI VGA Controller by looking at the Command register and 
  // Class Code Register
  //
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  
  Status = EFI_UNSUPPORTED;
#ifdef SOFT_SDV
    if(Pci.Hdr.ClassCode[2] == 1 && Pci.Hdr.ClassCode[1] == 1) {
      Status = EFI_SUCCESS;
    }
#else
  if ((Pci.Hdr.Command & 0x03) == 0x03) {
    if (IS_PCI_VGA (&Pci)) {
      Status = EFI_SUCCESS;
    }
  }
#endif

Done:
  gBS->CloseProtocol (
         Controller,  
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );

  return Status;
}   

EFI_STATUS
BiosVgaDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Install VGA Mini Port Protocol onto VGA device handles
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
    
--*/                
{
  EFI_STATUS            Status;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  BIOS_VGA_DEV          *BiosVgaPrivate;

  BiosVgaPrivate = NULL;

  //
  // Open the IO Abstraction(s) needed 
  //
  Status = gBS->OpenProtocol (
                  Controller,           
                  &gEfiPciIoProtocolGuid, 
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
      
  //
  // Allocate the private device structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(BIOS_VGA_DEV),
                  &BiosVgaPrivate
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EfiZeroMem (BiosVgaPrivate, sizeof (BIOS_VGA_DEV)) ;
  
  //
  // Initialize the private device structure
  //
  BiosVgaPrivate->Signature = BIOS_VGA_DEV_SIGNATURE;
  BiosVgaPrivate->Handle    = Controller;

  BiosVgaPrivate->SimpleTextOut.Reset             = BiosVgaReset;
  BiosVgaPrivate->SimpleTextOut.OutputString      = BiosVgaOutputString;
  BiosVgaPrivate->SimpleTextOut.TestString        = BiosVgaTestString;
  BiosVgaPrivate->SimpleTextOut.ClearScreen       = BiosVgaClearScreen;
  BiosVgaPrivate->SimpleTextOut.SetAttribute      = BiosVgaSetAttribute;
  BiosVgaPrivate->SimpleTextOut.SetCursorPosition = BiosVgaSetCursorPosition;
  BiosVgaPrivate->SimpleTextOut.EnableCursor      = BiosVgaEnableCursor;
  BiosVgaPrivate->SimpleTextOut.QueryMode         = BiosVgaQueryMode;
  BiosVgaPrivate->SimpleTextOut.SetMode           = BiosVgaSetMode;

  BiosVgaPrivate->SimpleTextOut.Mode              = &BiosVgaPrivate->SimpleTextOutputMode;
#ifdef SOFT_SDV
  BiosVgaPrivate->SimpleTextOutputMode.MaxMode    = 1;
#else
  BiosVgaPrivate->SimpleTextOutputMode.MaxMode    = 2;
#endif

  //
  // Allocate a one page buffer below 1 MB to support BIOS calls that require 
  // data in an input buffer.
  //
  BiosVgaPrivate->BufferPhysicalAddress = 0x000fffff;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress, 
                  EfiBootServicesData, 
                  1, 
                  &BiosVgaPrivate->BufferPhysicalAddress
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  BiosVgaPrivate->Buffer = (UINT8 *)BiosVgaPrivate->BufferPhysicalAddress;

  Status = BiosVgaPrivate->SimpleTextOut.SetAttribute (
                                           &BiosVgaPrivate->SimpleTextOut,
                                           EFI_TEXT_ATTR (EFI_WHITE, EFI_BLACK)
                                           );

  Status = BiosVgaPrivate->SimpleTextOut.Reset (
                                           &BiosVgaPrivate->SimpleTextOut,
                                           FALSE
                                           );

  Status = BiosVgaPrivate->SimpleTextOut.EnableCursor (
                                           &BiosVgaPrivate->SimpleTextOut,
                                           TRUE
                                           );

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleTextOutProtocolGuid, &BiosVgaPrivate->SimpleTextOut,
                  NULL
                  );

Done:
  if (EFI_ERROR (Status)) {

    gBS->CloseProtocol (
           Controller,  
           &gEfiPciIoProtocolGuid, 
           This->DriverBindingHandle,   
           Controller   
           );

    if (BiosVgaPrivate != NULL) {
      if (BiosVgaPrivate->Buffer != NULL) {
        gBS->FreePages (BiosVgaPrivate->BufferPhysicalAddress, 1);
      }
      gBS->FreePool (BiosVgaPrivate);
    }
  }

  return Status;
}

EFI_STATUS
BiosVgaDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop.
  
  Arguments:
    (Standard DriverBinding Protocol Stop() function)
  
  Returns:
    EFI_STATUS
  
--*/                
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SimpleTextOut;
  BIOS_VGA_DEV                  *BiosVgaPrivate;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextOutProtocolGuid,  
                  (VOID **)&SimpleTextOut,
                  This->DriverBindingHandle,             
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BiosVgaPrivate = BIOS_VGA_DEV_FROM_THIS (SimpleTextOut);

  Status = gBS->UninstallProtocolInterface (
                  Controller, 
                  &gEfiSimpleTextOutProtocolGuid, &BiosVgaPrivate->SimpleTextOut
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
    
  //
  // Release PCI I/O and VGA Mini Port Protocols on the controller handle.
  //
  gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle, 
         Controller
         );

  gBS->FreePages (BiosVgaPrivate->BufferPhysicalAddress, 1);
  gBS->FreePool (BiosVgaPrivate);

  return EFI_SUCCESS;
}

//
// EFI Simple Text Output Protocol Functions
//
EFI_STATUS 
EFIAPI
BiosVgaReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL        *This,
  IN  BOOLEAN                             ExtendedVerification
  )
{
  //
  //reset the background and keep the foreground unchanged
  //
  This->SetAttribute(This,This->Mode->Attribute & 0xf);
  
  return This->SetMode (This, 0); 
}

EFI_STATUS 
EFIAPI
BiosVgaOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
{
  BIOS_VGA_DEV                 *BiosVgaPrivate;
  EFI_SIMPLE_TEXT_OUTPUT_MODE  *Mode;
  UINTN                        MaxColumn;
  UINTN                        MaxRow;
  UINTN                        Index;
  CHAR8                        GraphicChar;

  BiosVgaPrivate = BIOS_VGA_DEV_FROM_THIS (This);

  Mode = This->Mode;
  This->QueryMode (
          This, 
          Mode->Mode, 
          &MaxColumn, 
          &MaxRow
          );

  for(;*WString != CHAR_NULL; WString++) {
      
    switch (*WString) {
    case CHAR_NULL :
      break;
    case CHAR_BACKSPACE : 
      if (Mode->CursorColumn > 0) {
          Mode->CursorColumn--;
      }
      break;
    case CHAR_LINEFEED :
      if (Mode->CursorRow == (INT32)(MaxRow-1)) {
        //
        // Scroll Screen and Print Blank Line
        //
        BiosVgaPrivate->Buffer[0] = '\r';
        BiosVgaPrivate->Buffer[1] = '\n';
#ifdef SOFT_SDV
        for (Index = 0; Index < MaxColumn ; Index++) {
#else
        for (Index = 0; Index < MaxColumn - 1 ; Index++) {
#endif
          BiosVgaPrivate->Buffer[Index + 2] = ' ';
        }
        BiosVgaPrivate->Buffer[Index + 2] = '\r';
        BiosVgaPrivate->Buffer[Index + 3] = 0;
        WriteVgaString (
          0, 
          Mode->CursorRow, 
          Mode->Attribute, 
          BiosVgaPrivate->Buffer
          );
      }
      if (Mode->CursorRow < (INT32)(MaxRow-1)) {
        Mode->CursorRow++;
      }
      break;
    case CHAR_CARRIAGE_RETURN:
      Mode->CursorColumn = 0;
      break;
    default:
      for (Index=0; (INT32)(Index + Mode->CursorColumn) < (INT32)(MaxColumn) && !BiosVgaLibIsValidEfiCntlChar(*WString) ; WString++, Index++) {
        if (!BiosVgaLibIsValidTextGraphics (*WString, &GraphicChar, NULL)) {
          // Just convert to ASCII
          GraphicChar = (CHAR8)*WString;
          if (!BiosVgaLibIsValidAscii (GraphicChar)) {
            //
            // Keep the API from supporting PCANSI Graphics chars
            //
            GraphicChar = '?';
          }
        }
        BiosVgaPrivate->Buffer[Index] = GraphicChar;  
      }
      BiosVgaPrivate->Buffer[Index] = 0;
      WriteVgaString (
        Mode->CursorColumn, 
        Mode->CursorRow, 
        Mode->Attribute, 
        BiosVgaPrivate->Buffer
        );

      Mode->CursorColumn += (UINT32)Index;
#ifdef SOFT_SDV
      if (Mode->CursorColumn >= (INT32)(MaxColumn)) {
        This->OutputString (This, CrLfString);
      } 
#else
      if (Mode->CursorColumn >= (INT32)(MaxColumn)) {
        if (Mode->CursorRow < (INT32)(MaxRow - 1)) {
          This->OutputString (This, CrLfString);
        } else {
          Mode->CursorColumn = 0;
        }
      } 
#endif
      WString--;
      break;
    }
  }    

  SetVideoCursorPosition ((UINTN)Mode->CursorColumn, (UINTN)Mode->CursorRow);

  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
BiosVgaTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
{
  while (*WString != 0x0000) {
    if ( ! (BiosVgaLibIsValidAscii (*WString) || 
            BiosVgaLibIsValidEfiCntlChar (*WString) || 
            BiosVgaLibIsValidTextGraphics (*WString, NULL, NULL) )) {
      return EFI_UNSUPPORTED;
    }
    WString++;
  }   
  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
BiosVgaQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  )
{
  switch (ModeNumber) {
    case 0 :
      *Columns = 80;
      *Rows    = 25;
      return EFI_SUCCESS;
    case 1 :
      *Columns = 80;
      *Rows    = 50;
      return EFI_SUCCESS;
  } 
  return EFI_UNSUPPORTED;
}

EFI_STATUS 
EFIAPI
BiosVgaSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber
  )
{
  if ((INT32)ModeNumber <= This->Mode->MaxMode) {
    This->Mode->Mode = (INT32)ModeNumber;
    SetVideoMode (0x83);
    SetVideoRows (ModeNumber);
    This->ClearScreen (This);
    return EFI_SUCCESS;
  }
  return EFI_UNSUPPORTED;
}

EFI_STATUS 
EFIAPI
BiosVgaSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Attribute
  )
{
  if (Attribute >= 0 && Attribute <= EFI_MAX_ATTRIBUTE) {
    This->Mode->Attribute = (INT32) Attribute; 
    return EFI_SUCCESS;
  } else {
    return EFI_UNSUPPORTED;
  }
}

EFI_STATUS 
EFIAPI
BiosVgaClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This
  )
{
  BIOS_VGA_DEV        *BiosVgaPrivate;
  UINTN               MaxRow;
  UINTN               MaxColumn;
  UINT32              Index;
  
  BiosVgaPrivate = BIOS_VGA_DEV_FROM_THIS (This);

  This->QueryMode (
          This, 
          This->Mode->Mode, 
          &MaxColumn, 
          &MaxRow
          );

  for (Index = 0; Index < MaxColumn; Index++) {
    BiosVgaPrivate->Buffer[Index] = ' ';
  }
  BiosVgaPrivate->Buffer[Index] = 0;
  for (Index = 0; Index < MaxRow; Index++) {
    WriteVgaString (0, Index, This->Mode->Attribute, BiosVgaPrivate->Buffer);
  }

  This->SetCursorPosition (This, 0,0);
  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
BiosVgaSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  )
{
  EFI_SIMPLE_TEXT_OUTPUT_MODE  *Mode;
  UINTN                        MaxColumn;
  UINTN                        MaxRow;

  Mode = This->Mode;
  This->QueryMode (
          This, 
          Mode->Mode, 
          &MaxColumn, 
          &MaxRow
          );
  if (Column >= MaxColumn || Row >= MaxRow) {
    return EFI_UNSUPPORTED;
  }

  SetVideoCursorPosition (Column, Row);
  Mode->CursorColumn = (INT32) Column;
  Mode->CursorRow = (INT32) Row;
  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
BiosVgaEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         Visible
  )
{
  IA32_RegisterSet_t  Regs;
  UINT16              CursorValue;

  if (Visible == TRUE) {
    switch (This->Mode->Mode) {
    case 0:
      CursorValue = 0x0607; 
      break;
    case 1:
      CursorValue = 0x0e0f; 
      break;
    default:
      return EFI_UNSUPPORTED;  
    }
  } else {
    CursorValue = 0x2000; 
  }

  Regs.h.AH = 0x01;
  Regs.x.CX = CursorValue;
  Int86(0x10, &Regs);

  This->Mode->CursorVisible = Visible;
  return EFI_SUCCESS;
}


static 
CHAR16 *
UnicodeToASCII (
  IN  CHAR16  *WString,
  OUT CHAR8   *String,
  IN  UINTN   MaxSize
  )
{    
  UINTN i;
  
  MaxSize -= 1;
  for (i=0; i<MaxSize && *WString != '\0'; i++) {
    *String++ = (CHAR8)*WString++;
  } 
  *String = '\0';
  return (WString);
}

static
VOID
SetVideoRows(
  INTN Mode
  )
{
  IA32_RegisterSet_t  Regs;
  UINT8               FontType;

  switch (Mode) {
  case 0: 
    FontType = 0x14; 
    break;
  case 1:
    FontType = 0x12; 
    break;
  default:
    FontType = 0x14;
  };
  
  Regs.h.AH = 0x11;
  Regs.h.AL = FontType;
  Regs.h.BL = 0;
  Int86(0x10, &Regs);
}

static 
VOID
SetVideoMode(
  INTN Mode
  )
{
  IA32_RegisterSet_t Regs;

  Regs.h.AH = 0x00;
  Regs.h.AL = (UINT8)Mode;
  Int86(0x10, &Regs);
}

static 
VOID 
GetVideoCursorPosition(
  OUT INT32   *Column,
  OUT INT32   *Row
  )
{
  IA32_RegisterSet_t Regs;
  
  Regs.h.AH = 0x03;
  Regs.h.BH = 0x00; /* Page Zero */
  Int86(0x10, &Regs);
  *Column = (INT32)Regs.h.DL;
  *Row = (INT32)Regs.h.DH;
}

static 
VOID
WriteVgaString(
  IN  INT32   Column,
  IN  INT32   Row,
  IN  INT32   Color,
  IN  CHAR8   *String
  )
{                         
  UINT16              RowColumn;
  IA32_RegisterSet_t  Regs;
  
  RowColumn = (UINT16) (Row << 8 | Column);
  Regs.x.AX = 0x1301;
  Regs.x.BX = (UINT16)Color;
  Regs.x.CX = (UINT16)strlena(String);
  Regs.x.DX = (UINT16)RowColumn;
  Regs.x.ES = (UINT16) _FP_SEG(String);
  Regs.x.BP = (UINT16) _FP_OFF(String);
  Int86(0x10, &Regs);
}

static 
VOID 
SetVideoCursorPosition(
  IN  UINTN   Column,
  IN  UINTN   Row
  )
{
  IA32_RegisterSet_t Regs;
  
  Regs.h.AH = 0x02;
  Regs.h.BH = 0x00; /* Page Zero */
  Regs.h.DL = (UINT8)Column;
  Regs.h.DH = (UINT8)Row;
  Int86(0x10, &Regs);
}

typedef struct {
    CHAR16  Unicode;
    CHAR8   PcAnsi;
    CHAR8   Ascii;
} UNICODE_TO_CHAR;

//
// This list is used to define the valid extend chars.
// It also provides a mapping from Unicode to PCANSI or
// ASCII. The ASCII mapping we just made up.
//
//

static UNICODE_TO_CHAR UnicodeToPcAnsiOrAscii[] = {
  BOXDRAW_HORIZONTAL,                 0xc4, L'-', 
  BOXDRAW_VERTICAL,                   0xb3, L'|',
  BOXDRAW_DOWN_RIGHT,                 0xda, L'/',
  BOXDRAW_DOWN_LEFT,                  0xbf, L'\\',
  BOXDRAW_UP_RIGHT,                   0xc0, L'\\',
  BOXDRAW_UP_LEFT,                    0xd9, L'/',
  BOXDRAW_VERTICAL_RIGHT,             0xc3, L'|',
  BOXDRAW_VERTICAL_LEFT,              0xb4, L'|',
  BOXDRAW_DOWN_HORIZONTAL,            0xc2, L'+',
  BOXDRAW_UP_HORIZONTAL,              0xc1, L'+',
  BOXDRAW_VERTICAL_HORIZONTAL,        0xc5, L'+',
  BOXDRAW_DOUBLE_HORIZONTAL,          0xcd, L'-',
  BOXDRAW_DOUBLE_VERTICAL,            0xba, L'|',
  BOXDRAW_DOWN_RIGHT_DOUBLE,          0xd5, L'/',
  BOXDRAW_DOWN_DOUBLE_RIGHT,          0xd6, L'/',
  BOXDRAW_DOUBLE_DOWN_RIGHT,          0xc9, L'/',
  BOXDRAW_DOWN_LEFT_DOUBLE,           0xb8, L'\\',
  BOXDRAW_DOWN_DOUBLE_LEFT,           0xb7, L'\\',
  BOXDRAW_DOUBLE_DOWN_LEFT,           0xbb, L'\\',
  BOXDRAW_UP_RIGHT_DOUBLE,            0xd4, L'\\',
  BOXDRAW_UP_DOUBLE_RIGHT,            0xd3, L'\\',
  BOXDRAW_DOUBLE_UP_RIGHT,            0xc8, L'\\',
  BOXDRAW_UP_LEFT_DOUBLE,             0xbe, L'/',
  BOXDRAW_UP_DOUBLE_LEFT,             0xbd, L'/',
  BOXDRAW_DOUBLE_UP_LEFT,             0xbc, L'/',
  BOXDRAW_VERTICAL_RIGHT_DOUBLE,      0xc6, L'|',
  BOXDRAW_VERTICAL_DOUBLE_RIGHT,      0xc7, L'|',
  BOXDRAW_DOUBLE_VERTICAL_RIGHT,      0xcc, L'|',
  BOXDRAW_VERTICAL_LEFT_DOUBLE,       0xb5, L'|',
  BOXDRAW_VERTICAL_DOUBLE_LEFT,       0xb6, L'|',
  BOXDRAW_DOUBLE_VERTICAL_LEFT,       0xb9, L'|',
  BOXDRAW_DOWN_HORIZONTAL_DOUBLE,     0xd1, L'+',
  BOXDRAW_DOWN_DOUBLE_HORIZONTAL,     0xd2, L'+',
  BOXDRAW_DOUBLE_DOWN_HORIZONTAL,     0xcb, L'+',
  BOXDRAW_UP_HORIZONTAL_DOUBLE,       0xcf, L'+',
  BOXDRAW_UP_DOUBLE_HORIZONTAL,       0xd0, L'+',
  BOXDRAW_DOUBLE_UP_HORIZONTAL,       0xca, L'+',
  BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE, 0xd8, L'+',
  BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL, 0xd7, L'+',
  BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL, 0xce, L'+',

  BLOCKELEMENT_FULL_BLOCK,            0xdb, L'*',
  BLOCKELEMENT_LIGHT_SHADE,           0xb0, L'+',

  GEOMETRICSHAPE_UP_TRIANGLE,         0x1e, L'^',
  GEOMETRICSHAPE_RIGHT_TRIANGLE,      0x10, L'>',
  GEOMETRICSHAPE_DOWN_TRIANGLE,       0x1f, L'v',
  GEOMETRICSHAPE_LEFT_TRIANGLE,       0x11, L'<',

  /* BugBug: Left Arrow is an ESC. We can not make it print
              on a PCANSI terminal. If we can make left arrow 
              come out on PC ANSI we can add it back.

  ARROW_LEFT,                         0x1b, L'<',
  */

  ARROW_UP,                           0x18, L'^',
  
  /* BugBut: Took out left arrow so right has to go too.
  ARROW_RIGHT,                        0x1a, L'>',
  */      
  ARROW_DOWN,                         0x19, L'v',
  
  0x0000, 0x00
};

static
BOOLEAN
BiosVgaLibIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi,    OPTIONAL
  OUT CHAR8   *Ascii      OPTIONAL
  )
/*++

Routine Description:

    Detects if a Unicode char is for Box Drawing text graphics.

Arguments:

    Grphic  - Unicode char to test.

    PcAnsi  - Optional pointer to return PCANSI equivalent of Graphic.

    Asci    - Optional pointer to return Ascii equivalent of Graphic.

Returns:

    TRUE if Gpaphic is a supported Unicode Box Drawing character.

--*/
{
  UNICODE_TO_CHAR     *Table;

  if ((((Graphic & 0xff00) != 0x2500) && ((Graphic & 0xff00) != 0x2100))) {
   
    //
    // Unicode drawing code charts are all in the 0x25xx range, 
    //  arrows are 0x21xx
    //
    return FALSE;
  }

  for (Table = UnicodeToPcAnsiOrAscii; Table->Unicode != 0x0000; Table++) {
    if (Graphic == Table->Unicode) {
      if (PcAnsi) {
        *PcAnsi = Table->PcAnsi; 
      }
      if (Ascii) {
        *Ascii = Table->Ascii;
      }
      return TRUE;
    }
  }
  return FALSE;
}

static
BOOLEAN
BiosVgaLibIsValidAscii (
  IN  CHAR16  Ascii
  )
{
  if ((Ascii >= 0x20) && (Ascii <= 0x7f)) {
    return TRUE;
  }              
  return FALSE;
}

static
BOOLEAN
BiosVgaLibIsValidEfiCntlChar (
    IN  CHAR16  c
    )
{
  if (c == CHAR_NULL || c == CHAR_BACKSPACE || c == CHAR_LINEFEED || c == CHAR_CARRIAGE_RETURN) {
    return TRUE;
  }              
  return FALSE;
}

static
UINTN
strlena (
  IN CHAR8    *s1
  )
// string length
{
  UINTN        len;
  
  for (len=0; *s1; s1+=1, len+=1) ;
  return len;
}

