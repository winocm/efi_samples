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


Abstract:




Revision History

--*/


#include "efifw.h"


#define LOADED_IMAGE_SIGNATURE   EFI_SIGNATURE_32('l','d','r','i')

typedef struct {
    UINTN                       Signature;
    EFI_HANDLE                  Handle;         // Image handle
    UINTN                       Type;           // Image type

    CHAR16                      *Name;          // Displayable name

    BOOLEAN                     Started;        // If entrypoint has been called
    VOID                        *StartImageContext;

    EFI_IMAGE_ENTRY_POINT       EntryPoint;     // The image's entry point
    EFI_LOADED_IMAGE            Info;           // loaded image protocol

    // 
    EFI_PHYSICAL_ADDRESS        ImageBasePage;  // Location in memory
    UINTN                       NumberOfPages;  // Number of pages 
    CHAR8                       *ImageBase;     // As a char pointer
    CHAR8                       *ImageEof;      // End of memory image
    LIST_ENTRY                  ImageLink;      // External image list

    // relocate info
    CHAR8                       *ImageAdjust;   // Bias for reloc calculations
    CHAR8                       *FixupData;     // Original fixup data
    UINT16                      Machine;        // Machine type from PE image
} LOADED_IMAGE;


//
//
//

EFI_STATUS
LoadPeImage (
    IN SIMPLE_READ_FILE             FHand,
    IN LOADED_IMAGE                 *Image
    );

VOID
ConvertPeImage (
    IN LOADED_IMAGE                 *Image
    );

INTERNAL
EFI_STATUS
CheckImageMachineType (
    IN UINTN                        MachineType
    );

INTERNAL
EFI_STATUS
SetImageType (
    IN OUT LOADED_IMAGE             *Image,
    IN UINTN                        ImageType
    );

EFI_STATUS
RUNTIMEFUNCTION 
LoadPeRelocate_Ex (
    IN UINT16      *Reloc,
    IN OUT CHAR8   *Fixup, 
    IN OUT CHAR8   **FixupData,
    IN UINT64      Adjust
    );

STATIC
EFI_STATUS
RUNTIMEFUNCTION INTERNAL
ConvertPeImage_Ex (
    IN UINT16      *Reloc,
    IN OUT CHAR8   *Fixup, 
    IN OUT CHAR8   **FixupData,
    IN UINT64      Adjust
    );

//
// Globals
//

extern INTERNAL FLOCK       ImageListLock;
extern INTERNAL LIST_ENTRY  BootImageList;
extern INTERNAL LIST_ENTRY  RuntimeImageList;
