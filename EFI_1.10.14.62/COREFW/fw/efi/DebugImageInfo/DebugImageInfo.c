/*++

Copyright (c) 2001 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DebugImageInfo.c
    
Abstract:

  Support functions for managing debug image info table when loading and unloading
  images...

--*/

#include "DebugImageInfo.h"

static EFI_DEBUG_IMAGE_INFO_TABLE_HEADER *mDebugInfoTableHeader;

VOID
InitializeSystemTablePointerStructure (
  VOID
  )
/*++

Routine Description:

  Creates and initializes the aligned system table pointer structure.

Arguments:

Returns:

--*/
{
  EFI_STATUS                          Status;
  EFI_PHYSICAL_ADDRESS                Mem;
  EFI_SYSTEM_TABLE_POINTER            *STPointerStruct;
  
  //
  // Allocate boot services page for the structure.  Insure it's 4M aligned.
  //
  Status = BS->AllocatePages (AllocateAnyPages, EfiBootServicesData, 0x400, &Mem);
  if (!EFI_ERROR (Status)) {

    BS->FreePages (Mem, 0x400);

    Mem = (Mem + 0x003fffff) & (~0x3fffff);

    Status = BS->AllocatePages (AllocateAddress, EfiBootServicesData, 1, &Mem);
    if (!EFI_ERROR (Status)) {
    
      STPointerStruct = (EFI_SYSTEM_TABLE_POINTER *) Mem;
      STPointerStruct->Signature = EFI_SYSTEM_TABLE_SIGNATURE;
      STPointerStruct->EfiSystemTableBase = (EFI_PHYSICAL_ADDRESS) ST;
      STPointerStruct->Crc32 = 0;
      BS->CalculateCrc32((UINT8 *)STPointerStruct,
        sizeof (EFI_SYSTEM_TABLE_POINTER), &STPointerStruct->Crc32);
    }
  }
}

    
VOID
InitializeDebugImageInfoTable (
  VOID
  )
/*++

Routine Description:

  Creates and initializes the DebugImageInfo Table.  Also creates the configuration
  table and registers it into the system table.

Arguments:

Returns:

--*/
{
  EFI_STATUS                          Status;
  EFI_PHYSICAL_ADDRESS                Mem;
  
  //
  // Allocate and init boot services memory for the DebugInfoTable and header
  //
  mDebugInfoTableHeader = AllocateZeroPool (sizeof (EFI_DEBUG_IMAGE_INFO_TABLE_HEADER));
  if (mDebugInfoTableHeader != NULL) {
    mDebugInfoTableHeader->TableSize = EFI_DEBUG_IMAGE_ALLOCATION_SIZE;
  
    Status = BS->AllocatePages (AllocateAnyPages, EfiBootServicesData, 1, &Mem);
    if (!EFI_ERROR (Status)) {
      mDebugInfoTableHeader->EfiDebugImageInfoTable = (EFI_DEBUG_IMAGE_INFO *) Mem;
      BS->SetMem (mDebugInfoTableHeader->EfiDebugImageInfoTable, EFI_PAGE_SIZE, 0);

      //
      // Create a configuration table to tie this all back to the system table...
      //
      Status = BS->InstallConfigurationTable (&gEfiDebugImageInfoTableGuid, mDebugInfoTableHeader);
      if (EFI_ERROR (Status)) {
        BS->FreePages ((EFI_PHYSICAL_ADDRESS) mDebugInfoTableHeader->EfiDebugImageInfoTable, 1);
        BS->FreePool (mDebugInfoTableHeader);
        mDebugInfoTableHeader = NULL;
      }
    } else {
      BS->FreePool (mDebugInfoTableHeader);
      mDebugInfoTableHeader = NULL;
    }
  }
}


VOID
NewDebugImageInfoEntry (
  UINTN ImageInfoType,
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage,
  EFI_HANDLE ImageHandle
  )
/*++

Routine Description:

  Adds a new DebugImageInfo structure to the DebugImageInfo Table.  Re-Allocates
  the table if it's not large enough to accomidate another entry.  The table is
  allocated from page space and the entry is allocated from pool.

Arguments:

Returns:

--*/
{    
  EFI_DEBUG_IMAGE_INFO  *Table, *NewTable;
  UINTN                 Index;
  UINTN                 TableSize;
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS                Mem;
  
  if (mDebugInfoTableHeader != NULL) {
    mDebugInfoTableHeader->UpdateStatus |= EFI_DEBUG_IMAGE_INFO_UPDATE_IN_PROGRESS;
  
    Table = mDebugInfoTableHeader->EfiDebugImageInfoTable;
    TableSize = mDebugInfoTableHeader->TableSize;
  
    Index = 0;
    while (Index < TableSize && Table[Index].NormalImage != NULL) {
      Index++;
    }
    if (Index == TableSize) {
      //
      //  Table is full, so re-allocate another page for a larger table...
      //
      Status = BS->AllocatePages (
        AllocateAnyPages, 
        EfiBootServicesData, 
        TABLE_SIZE_TO_PAGES (TableSize) + 1,
        &Mem);

      if (EFI_ERROR (Status)) {
        mDebugInfoTableHeader->UpdateStatus &= ~EFI_DEBUG_IMAGE_INFO_UPDATE_IN_PROGRESS;
        return;
      }

      NewTable = (EFI_DEBUG_IMAGE_INFO *) Mem;
      BS->CopyMem (
        NewTable, 
        Table,
        TableSize * sizeof (void *));
      BS->SetMem (&NewTable[Index], EFI_PAGE_SIZE, 0);

      BS->FreePages ((EFI_PHYSICAL_ADDRESS)Table, TABLE_SIZE_TO_PAGES (TableSize));
      Table = NewTable;
      mDebugInfoTableHeader->EfiDebugImageInfoTable = NewTable;
      mDebugInfoTableHeader->TableSize += EFI_DEBUG_IMAGE_ALLOCATION_SIZE;
    }

    Table[Index].NormalImage = AllocateZeroPool (sizeof (EFI_DEBUG_IMAGE_INFO_NORMAL));
    if (Table[Index].NormalImage == NULL) {
      mDebugInfoTableHeader->UpdateStatus &= ~EFI_DEBUG_IMAGE_INFO_UPDATE_IN_PROGRESS;
      return;
    }
    Table[Index].NormalImage->ImageInfoType = ImageInfoType;
    Table[Index].NormalImage->LoadedImageProtocolInstance = LoadedImage;
    Table[Index].NormalImage->ImageHandle = ImageHandle;
    mDebugInfoTableHeader->UpdateStatus &= ~EFI_DEBUG_IMAGE_INFO_UPDATE_IN_PROGRESS;
  }
}


VOID
DeleteDebugImageInfoEntry (
  EFI_HANDLE ImageHandle
  )
/*++

Routine Description:

  Deletes and frees an entry from the DebugImageInfo Table.

Arguments:

Returns:

--*/
{    
  EFI_DEBUG_IMAGE_INFO  *Table;
  UINTN                 Index;
  UINTN                 TableSize;

  if (mDebugInfoTableHeader != NULL) {
    mDebugInfoTableHeader->UpdateStatus |= EFI_DEBUG_IMAGE_INFO_UPDATE_IN_PROGRESS;

    Table = mDebugInfoTableHeader->EfiDebugImageInfoTable;
    TableSize = mDebugInfoTableHeader->TableSize;
  
    Index = 0;
    while (Index < TableSize) {
      if (Table[Index].NormalImage != NULL && Table[Index].NormalImage->ImageHandle == ImageHandle) {
        BS->FreePool (Table[Index].NormalImage);
        Table[Index].NormalImage = NULL;
        break;
      } else {
        Index++;
      }
    }
  
    mDebugInfoTableHeader->UpdateStatus &= ~EFI_DEBUG_IMAGE_INFO_UPDATE_IN_PROGRESS;
  }
}

