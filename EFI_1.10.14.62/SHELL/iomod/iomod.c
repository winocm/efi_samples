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

  iomod.c
  
Abstract: 

  shell command "mm". Modify memory/IO/PCI/MMIO.

Revision History

--*/

#include "shelle.h"

#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)

typedef enum {
  EfiMemory,
  EFIMemoryMappedIo,
  EfiIo,
  EfiPciConfig
} EFI_ACCESS_TYPE;

EFI_STATUS
DumpIoModify (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

VOID
ReadMem (
  IN  EFI_IO_WIDTH  Width, 
  IN  UINT64        Address, 
  IN  UINTN         Size, 
  IN  VOID          *Buffer
  );

VOID
WriteMem (
  IN  EFI_IO_WIDTH  Width, 
  IN  UINT64        Address, 
  IN  UINTN         Size, 
  IN  VOID          *Buffer
  );

static 
BOOLEAN
GetHex (
  IN  UINT16  *str,
  OUT UINT64  *data
  );

UINT64  MaxNum[9] = {0xff, 0xffff, 0xffffffff, 0xffffffffffffffff};

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(DumpIoModify)
#endif

EFI_STATUS
DumpIoModify (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Dump or modify MEM/IO/PCI/MMIO.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

Notes:
  MM Address [Width] [;[MEM | MMIO | IO | PCI]] [:Value]
    1|2|4|8 supported byte widths - Default is 1 byte. 
    ;MEM = Memory, 
    ;MMIO = Memory Mapped IO, 
    ;IO = in/out, 
    ;PCI = PCI Config space
    Default access type is memory (MEM)

--*/
{
  EFI_STATUS                        Status;
  UINTN                             PciRootBridgeIoHandleCount;
  EFI_HANDLE                        *PciRootBridgeIoBuffer;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *IoDev;
  UINT64                            Address;
  UINT64                            Value;
  EFI_IO_WIDTH                      Width;
  EFI_ACCESS_TYPE                   AccessType;
  UINT64                            Buffer;
  UINTN                             Index;
  UINTN                             Size;
  CHAR16                            *AddressStr;
  CHAR16                            *WidthStr;
  CHAR16                            *Ptr; 
  CHAR16                            *ValueStr;
  BOOLEAN                           Done;
  CHAR16                            InputStr[80];
  BOOLEAN                           Interactive;
  
  Address = 0;
  IoDev = NULL;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   DumpIoModify,
    L"mm",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Parse arguments
  //
  Width = EfiPciWidthUint8;
  Size = 1;
  AccessType = EfiMemory;
  AddressStr = WidthStr = NULL;
  ValueStr = NULL;
  Interactive = TRUE;
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    //
    // access type
    // 
    if (*Ptr == L';') {
      if (StriCmp (Ptr, L";IO") == 0) {
        AccessType = EfiIo;
        continue;
      } else if (StriCmp (Ptr, L";PCI") == 0) {
        AccessType = EfiPciConfig;
        continue;
      } else if (StriCmp (Ptr, L";MEM") == 0) {
        AccessType = EfiMemory;
        continue;  
      } else if (StriCmp (Ptr, L";MMIO") == 0) {
        AccessType = EFIMemoryMappedIo;
        continue;
      } else {
        Print (L"mm: Unknown access type %hs\n", Ptr);
        return EFI_INVALID_PARAMETER;
      }
    } else if (*Ptr == ':') {
      //
      // value
      //
      ValueStr = &Ptr[1];
      Value = xtoi(ValueStr);
      continue;
    } else if (*Ptr == '-') {
      //
      // command option switch
      //
      switch (Ptr[1]) {
      case 'n':
      case 'N': Interactive = FALSE; 
        break;

      default:
        Print (L"mm: Unknown flag %hs\n", Ptr); 
        return EFI_INVALID_PARAMETER;
      };
      continue;
    }
    // 
    // Start Address 
    //
    if (!AddressStr) {
      AddressStr = Ptr;
      Address = xtoi(AddressStr);
      continue;
    }
    //
    // access width
    //
    if (!WidthStr) {
       WidthStr = Ptr;
       switch (xtoi(WidthStr)) {
       case 1:
         Width = EfiPciWidthUint8;
         Size = 1;
         continue;
       case 2:
         Width = EfiPciWidthUint16;
         Size = 2;
         continue;
       case 4:
         Width = EfiPciWidthUint32;
         Size = 4;
         continue;
       case 8:
         Width = EfiPciWidthUint64;
         Size = 8;
         continue;
       default:
         Print (L"mm: Invalid data width\n");
         return EFI_INVALID_PARAMETER;
       }
    }
  }

  if (! AddressStr) {
    Print (L"mm: Too few arguments\n"); 
    return EFI_INVALID_PARAMETER;
  }

  if ((Address & (Size - 1)) != 0) {
    Print (L"mm: Data address %hd not aligned %hd\n", Address, Size);
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // locate DeviceIO protocol interface
  //
  if (AccessType != EfiMemory) {
    PciRootBridgeIoBuffer = NULL;
    Status = BS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiPciRootBridgeIoProtocolGuid, 
                    NULL,
                    &PciRootBridgeIoHandleCount,  
                    (VOID ***)&PciRootBridgeIoBuffer
                    );
    if (!EFI_ERROR(Status) && PciRootBridgeIoHandleCount > 0) {
      Status = BS->HandleProtocol (PciRootBridgeIoBuffer[0], &gEfiPciRootBridgeIoProtocolGuid, (VOID*)&IoDev);
      if (PciRootBridgeIoBuffer != NULL) {
        BS->FreePool(PciRootBridgeIoBuffer);
      }
    } else {
      Print (L"mm: Handle PciRootBridgeIO protocol error - %r", Status);
      return Status;
    }
  }

  if (AccessType == EfiIo && Address+Size > 0x10000) {
    Print(L"mm: IO address out of range 0 - 0x10000\n");
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Set value
  //
  if (ValueStr) {
    if (AccessType == EFIMemoryMappedIo) {
      IoDev->Mem.Write (IoDev, Width, Address, 1, &Value);
    } else if (AccessType == EfiIo) {
      IoDev->Io.Write (IoDev, Width, Address, 1, &Value);
    } else if (AccessType == EfiPciConfig) {
      IoDev->Pci.Write (IoDev, Width, Address, 1, &Value);
    } else {
      WriteMem (Width, Address, 1, &Value);
    }
    return EFI_SUCCESS;
  }

  //
  // non-interactive mode
  //
  if (Interactive == FALSE) {
    Buffer = 0;
    if (AccessType == EFIMemoryMappedIo) {
      Print (L"%HMMIO%N");
      IoDev->Mem.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiIo) {
      Print (L"%HIO%N");
      IoDev->Io.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiPciConfig) {
      Print (L"%HPCI%N");
      IoDev->Pci.Read (IoDev, Width, Address, 1, &Buffer);
    } else {
      Print (L"%HMEM%N");
      ReadMem (Width, Address, 1, &Buffer);
    }

    Print (L"  0x%016lx : 0x", Address);
    if (Size == 1) {
      Print (L"%02x", Buffer);
    } else if (Size == 2) {
      Print (L"%04x", Buffer);
    } else if (Size == 4) {
      Print (L"%08x", Buffer);
    } else if (Size == 8) {
      Print (L"%016lx", Buffer);
    }
    Print(L"\n");
    return EFI_SUCCESS;
  }

  //
  // interactive mode
  //
  Done = FALSE;
  do {
    if (AccessType == EfiIo && Address+Size > 0x10000) {
      Print(L"\nmm: IO address out of range\n");
      break;
    }
    
    Buffer = 0;
    if (AccessType == EFIMemoryMappedIo) {
      Print (L"%HMMIO%N");
      IoDev->Mem.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiIo) {
      Print (L"%HIO%N");
      IoDev->Io.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiPciConfig) {
      Print (L"%HPCI%N");
      IoDev->Pci.Read (IoDev, Width, Address, 1, &Buffer);
    } else {
      Print (L"%HMEM%N");
      ReadMem (Width, Address, 1, &Buffer);
    }

    Print (L"  0x%016lx : 0x", Address);

    if (Size == 1) {
      Print (L"%02x", Buffer);
    } else if (Size == 2) {
      Print (L"%04x", Buffer);
    } else if (Size == 4) {
      Print (L"%08x", Buffer);
    } else if (Size == 8) {
      Print (L"%016lx", Buffer);
    }

    //
    // wait user input to modify
    //
    Input (L" > ", InputStr, sizeof(InputStr)/sizeof(CHAR16));
    
    //
    // skip space characters
    //      
    for ( Index = 0; InputStr[Index] == ' '; Index++ );
    
    //
    // parse input string
    //
    if (InputStr[Index] == '.' || InputStr[Index] == 'q' || InputStr[Index] == 'Q' ) {
      Done = TRUE;
    } else if (InputStr[Index] == CHAR_NULL) {
      
      //Continue to next address
      
    } else if (GetHex(InputStr+Index, &Buffer) && Buffer <= MaxNum[Width]) {       
      if (AccessType == EFIMemoryMappedIo) {
        IoDev->Mem.Write (IoDev, Width, Address, 1, &Buffer);
      } else if (AccessType == EfiIo) {
        IoDev->Io.Write (IoDev, Width, Address, 1, &Buffer);
      } else if (AccessType == EfiPciConfig) {
        IoDev->Pci.Write (IoDev, Width, Address, 1, &Buffer);
      } else {
        WriteMem (Width, Address, 1, &Buffer);
      }
    }
    else {
      Print(L"\n ^ Error\n");
      continue;
    }
    
    Address += Size;
    Print (L"\n");
  } while (!Done);

  return EFI_SUCCESS;
}


VOID
ReadMem (
  IN  EFI_IO_WIDTH  Width, 
  IN  UINT64        Address, 
  IN  UINTN         Size, 
  IN  VOID          *Buffer
  )
{
  do {
    if (Width == EfiPciWidthUint8) {
      *(UINT8 *)Buffer = *(UINT8 *)(UINTN)Address;
      Address -= 1;
    } else if (Width == EfiPciWidthUint16) {
      *(UINT16 *)Buffer = *(UINT16 *)(UINTN)Address;
      Address -= 2;
    } else if (Width == EfiPciWidthUint32) { 
      *(UINT32 *)Buffer = *(UINT32 *)(UINTN)Address;
      Address -= 4;
    } else if (Width == EfiPciWidthUint64) {
      *(UINT64 *)Buffer = *(UINT64 *)(UINTN)Address;
      Address -= 8;
    } else {
      Print (L"mm: Read mem error\n");
      break;
    }
    //
    //
    //
    Size--;
  }while (Size > 0);
}

VOID
WriteMem (
  IN  EFI_IO_WIDTH  Width, 
  IN  UINT64        Address, 
  IN  UINTN         Size, 
  IN  VOID          *Buffer
  )
{
  do {
    if (Width == EfiPciWidthUint8) {
      *(UINT8 *)(UINTN)Address = *(UINT8 *)Buffer;
      Address += 1;
    } else if (Width == EfiPciWidthUint16) {
      *(UINT16 *)(UINTN)Address = *(UINT16 *)Buffer;
      Address += 2;
    } else if (Width == EfiPciWidthUint32) { 
      *(UINT32 *)(UINTN)Address = *(UINT32 *)Buffer;
      Address += 4;
    } else if (Width == EfiPciWidthUint64) {
      *(UINT64 *)(UINTN)Address = *(UINT64 *)Buffer;
      Address += 8;
    }  else {
      ASSERT(FALSE);
    }
    //
    //
    //
    Size--;
  } while(Size > 0);
}

static 
BOOLEAN
GetHex (
  IN  UINT16  *str,
  OUT UINT64  *data
  )
{
  UINTN   u;
  CHAR16    c;
  BOOLEAN   Find = FALSE;
  
  //
  // convert hex digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= 'a'  &&  c <= 'f') {
      c -= 'a' - 'A';
    }
    
    if (c == ' ') {
      break;
    }
    
    if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F')) {
      u = u << 4  |  c - (c >= 'A' ? 'A'-10 : '0');
      
      Find = TRUE;
    } else {
      return FALSE;
    }
    c = *(str++);
  }
  *data = u;
  return Find;  
}
