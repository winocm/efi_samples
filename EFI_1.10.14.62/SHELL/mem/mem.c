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

  mem.c
  
Abstract: 

  shell command "dmem".

Revision History

--*/

#include "shelle.h"


EFI_STATUS
DumpMem (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

VOID
PrintOwnHelp(
  VOID
  );
  
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(DumpMem)
#endif

EFI_STATUS
DumpMem (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*+++

  mem [Address] [Size] ;MMIO
    if no Address default address is EFI System Table
    if no size default size is 512;
    if ;MMIO then use memory mapped IO and not system memory
--*/
{
  EFI_STATUS                        Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *PciRootBridgeIo;
  UINT64                            Address;
  UINTN                             Size;
  UINT8                             *Buffer;
  BOOLEAN                           MMIo;
  UINTN                             Index;
  CHAR16                            *AddressStr;
  CHAR16                            *SizeStr;
  CHAR16                            *Ptr;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   DumpMem,
    L"dmem",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // The End Device Path represents the Root of the tree, thus get the global IoDev
  //  for the system
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Get command arguments
  //
  MMIo = FALSE;
  AddressStr = SizeStr = NULL;
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == ';') {
      // Shortcut! assume MMIo if ; exists
      if (StriCmp (Ptr, L";MMIO") == 0) {
        MMIo = TRUE;
        continue;
      } else {
        Print (L"dmem: Unknown argument %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }
    } else if (*Ptr == '-') {
      switch (Ptr[1]) {
      case '?':
        PrintOwnHelp();
        return Status;
        
      default:
        Print (L"dmem: Uknown flag %hs\n", Ptr);
        Status = EFI_INVALID_PARAMETER;
        return Status;
      };
      continue;
    }
    if (!AddressStr) {
      AddressStr = Ptr;      
      continue;
    }
    if (!SizeStr) {
       SizeStr = Ptr;
       continue;
    }
  }

  Address = (AddressStr) ? xtoi(AddressStr) : (UINT64)SystemTable;
  Size = (SizeStr) ? xtoi(SizeStr) : 512;

  //
  // Get memory data
  //
  Print (L"  Memory Address %0*lx %0x Bytes\n", 2 * sizeof (UINTN), Address, Size);
  if (MMIo) {
    Status = BS->LocateProtocol (&gEfiPciRootBridgeIoProtocolGuid, NULL, &PciRootBridgeIo);
    if (EFI_ERROR(Status)) {
      Print (L"dmem: Locate protocol error - %r\n", Status);
      return Status;
    }

    //
    // Allocate buffer for memory io read
    //
    Buffer = AllocatePool (Size);
    if (Buffer == NULL) {
      Print (L"dmem: Out of memory\n");
      Status = EFI_OUT_OF_RESOURCES;
      return Status;
    }
    PciRootBridgeIo->Mem.Read (PciRootBridgeIo, IO_UINT8, Address, Size, Buffer);
  } else {
    Buffer = (UINT8 *)(UINTN)Address;
  }

  //
  // Dump data
  //
  DumpHex (2, (UINTN)Address, Size, Buffer);
  EFIStructsPrint (Buffer, Size, Address, NULL);

  if (MMIo) {
    FreePool (Buffer);
  }

  return Status;
}


VOID
PrintOwnHelp(
  VOID
  )
{
  Print (L"\n%Hmem%N [%HAddress%N] [%HSize%N] [%H;MMIO%N]\n");
  Print (L"  if no %HAddress%N is specified the EFI System Table is used\n");
  Print (L"  if no %HSize%N is specified 512 bytes is used\n");
  Print (L"  if %H;MMIO%N is specified memory is referenced with the DeviceIo Protocol\n");
}
