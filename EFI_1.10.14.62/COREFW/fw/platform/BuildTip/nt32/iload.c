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

    main.c

Abstract:

    Main entry point on EFI emulation application



Revision History

--*/


#include "ntemul.h"


EFI_STATUS
PlEmulateLoad (
    IN CHAR16                   *InternalName,
    IN OUT EFI_LOADED_IMAGE     *ImageInfo,
    OUT EFI_IMAGE_ENTRY_POINT   *ImageEntryPoint
    )
{
    HINSTANCE           Library;
    HANDLE              h;    
    CHAR16              DllName[80];
    BOOLEAN             f;
    ULONG               br, i;
    IMAGE_DOS_HEADER    DosHdr;
    IMAGE_NT_HEADERS    PeHdr;
    

    SPrint (DllName, sizeof(DllName), L"%s.dll", InternalName);

    Library = LoadLibraryEx (DllName, NULL, DONT_RESOLVE_DLL_REFERENCES);

    if (!Library) {
        DEBUG((D_ERROR, "LoadInternalDriver: failed to load: %s\n", DllName));
        return EFI_NOT_FOUND;
    }

    *ImageEntryPoint = (EFI_IMAGE_ENTRY_POINT) GetProcAddress (Library, "InitializeDriver");
    if (!*ImageEntryPoint) {
        DEBUG((D_ERROR, "LoadInternalDriver: driver doesn't have an entry point: %s\n", DllName));
        return EFI_UNSUPPORTED;
    }

    h = CreateFile (
            DllName,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    ASSERT (h != INVALID_HANDLE_VALUE);

    // supply image loaded info for ntldr
    ImageInfo->ImageSize    = GetFileSize(h, NULL);
    ImageInfo->ImageBase    = (VOID *) Library;
    ImageInfo->DeviceHandle = NULL;

    // read dos header
    f = ReadFile (h, &DosHdr, sizeof(DosHdr), &br, NULL);
    ASSERT (f && br == sizeof(DosHdr));
    ASSERT (DosHdr.e_magic == IMAGE_DOS_SIGNATURE);

    // read pe hdr
    i = SetFilePointer (h, DosHdr.e_lfanew, NULL, FILE_BEGIN);
    f = ReadFile (h, &PeHdr, sizeof(PeHdr), &br, NULL);
    ASSERT (i != 0xFFFFFFFF);
    ASSERT (f && br == sizeof(PeHdr));
    ASSERT (PeHdr.Signature == IMAGE_NT_SIGNATURE);

    // Verify ntldr is loaded at it's linked address.
    // (some link flags are causing it not to work if it's 
    // loaded at some other spot, but only when loaded via 
    // the NT api - if loaded by the EFI FW loader it works
    // even after relocation)

//   ASSERT ((UINT32) ImageInfo->ImageBase == PeHdr.OptionalHeader.ImageBase);

    // Done with special ntldr code
    CloseHandle (h);

    return EFI_SUCCESS;
}



