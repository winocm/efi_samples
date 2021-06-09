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

  DebugImageInfo.h
    
Abstract:

  Support functions for managing debug image info table when loading and unloading
  images...

--*/

#ifndef __DEBUG_IMAGE_INFO_H__
#define __DEBUG_IMAGE_INFO_H__

#include "Efi.h"
#include "efifw.h"
#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(LoadedImage)
#include EFI_GUID_DEFINITION(DebugImageInfoTable)

#define EFI_DEBUG_IMAGE_ALLOCATION_SIZE  (EFI_PAGE_SIZE / sizeof (void *))

#define TABLE_SIZE_TO_PAGES(TableSize)  \
  (((TableSize) * sizeof (void *)) / EFI_PAGE_SIZE)
  
VOID
InitializeSystemTablePointerStructure (
  VOID
  );

VOID
InitializeDebugImageInfoTable (
  VOID
  );

VOID
NewDebugImageInfoEntry (
  UINTN ImageInfoType,
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage,
  EFI_HANDLE ImageHandle
  );

VOID
DeleteDebugImageInfoEntry (
  EFI_HANDLE ImageHandle
  );

#endif
