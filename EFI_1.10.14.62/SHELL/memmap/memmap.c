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

  memmap.c
  
Abstract:

  Shell app "memmap"


Revision History

--*/

#include "shell.h"

//
//
//

EFI_STATUS
InitializeMemmap (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

BOOLEAN
IsRealMemory (
  IN  EFI_MEMORY_TYPE   Type
  );
//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeMemmap)
#endif

EFI_STATUS
InitializeMemmap (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Displays memory map.

Arguments:

  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:

  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Memory map not found

--*/  
{
  EFI_STATUS            Status;
  CHAR16                *Ptr;
  UINTN                 Index;
  UINTN                 DescriptorSize;
  UINT32                DescriptorVersion;
  UINTN                 NoDesc, MapKey;
  UINT64                Bytes;
  UINT64                NoPages[EfiMaxMemoryType];
  UINT64                OtherPages;
  UINT64                TotalMemory;
  EFI_MEMORY_DESCRIPTOR   *Desc, *MemMap;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeMemmap,
    L"memmap",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      if (Ptr[1] == 'b' || Ptr[1] == 'B') {
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
      } else {
        Print (L"memmap: Unknown flag %hs\n", Ptr);
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  //
  // Get memory map and print out
  //
  MemMap = LibMemoryMap (&NoDesc, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (!MemMap) {
    Print (L"memmap: Memory map table not found\n");
    return EFI_NOT_FOUND;
  }

  ASSERT (DescriptorVersion == EFI_MEMORY_DESCRIPTOR_VERSION);
  for (Index=0; Index < EfiMaxMemoryType; Index += 1) {
    NoPages[Index] = 0;
  }
  OtherPages = 0;

  Desc = MemMap;
  Print(L"\n%HType       Start            End               # Pages          Attributes%N\n");
  for (Index=0; Index < NoDesc; Index += 1) {
    Bytes = LShiftU64(Desc->NumberOfPages, 12);
    Ptr = MemoryTypeStr(Desc->Type);
    if (Ptr == NULL) {
      Print(L"%08x   ", Desc->Type);
    } else {
      Print(L"%s ", Ptr);
    }
    Print(L"%lX-%lX  %lX %lX\n", Desc->PhysicalStart, Desc->PhysicalStart + Bytes - 1, Desc->NumberOfPages, Desc->Attribute);
    //
    // count pages of each type memory
    //
    if (Desc->Type >=0 && Desc->Type < EfiMaxMemoryType) {
      NoPages[Desc->Type] += Desc->NumberOfPages;
    } else {
      OtherPages += Desc->NumberOfPages;
    }
    Desc = NextMemoryDescriptor(Desc, DescriptorSize);
  }

  Print(L"\n");

  //
  // Print each memory type summary
  //
  for (Index=0, TotalMemory = 0; Index < EfiMaxMemoryType; Index += 1) {
    if (NoPages[Index]) {
      Print(L"  %s: %,7ld Pages (%,ld)\n", 
        MemoryTypeStr((EFI_MEMORY_TYPE)Index),
        NoPages[Index], 
        LShiftU64(NoPages[Index], 12)
        );

      //
      // Count total memory
      //
      if (IsRealMemory((EFI_MEMORY_TYPE)(Index))) {
        TotalMemory += NoPages[Index];
      }
    }
  }
  if (OtherPages > 0) {
    Print(L"  other     : %,7ld Pages (%,ld)\n", 
      OtherPages, 
      LShiftU64(OtherPages, 12)
      );
    TotalMemory += OtherPages;
  }

  Print(L"Total Memory: %,ld MB (%,ld) Bytes\n",  
    RShiftU64(TotalMemory, 8),
    LShiftU64(TotalMemory, 12)
    );

  FreePool(MemMap);

  return Status;
}

BOOLEAN
IsRealMemory (
  IN  EFI_MEMORY_TYPE   Type
  )
{
  if (Type == EfiLoaderCode ||
    Type == EfiLoaderData ||
    Type == EfiBootServicesCode ||
    Type == EfiBootServicesData ||
    Type == EfiRuntimeServicesCode ||
    Type == EfiRuntimeServicesData ||
    Type == EfiConventionalMemory ||
    Type == EfiACPIReclaimMemory ||
    Type == EfiACPIMemoryNVS ||
    Type == EfiPalCode ) {
    //
    // BugBug can EfiPalCode point to ROM?
    //
    return TRUE;
  } else {
    return FALSE;
  }
}
