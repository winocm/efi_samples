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

    load.c

Abstract:

    EFI Boot Manager



Revision History

--*/

#include "bm.h"

//
// Internal prototypes
//

EFI_STATUS
BmLocateHandleByDiskSignatureAndPartition (
    IN UINT8                        MBRType,
    IN UINT8                        SignatureType,
    IN VOID                         *Signature,
    IN UINT32                       *PartitionNumber OPTIONAL,
    IN OUT UINTN                    *NoHandles,
    OUT EFI_HANDLE                  **Buffer
    );

EFI_STATUS
BmDefaultBootImage (
    IN EFI_DEVICE_PATH      *DevicePath,
    OUT EFI_HANDLE          *ImageHandle
    );


BOOLEAN
BmIsEfiImageName (
    CHAR16  *FileName
    );

//
//
//


VOID
BmLoadDrivers (
    VOID
    )
{
    LIST_ENTRY          *Link;
    BM_LOAD_OPTION      *Option;
    EFI_INPUT_KEY       Key;
    BOOLEAN             Reconnect;

    ConnectAll ();

    //
    // Display loading drivers
    //
    BmDisplay (BM_LOADING_DEVICE_DRIVERS);

    //
    // Load all images in order
    //
    Reconnect = FALSE;
    for (Link = BmOrderedDriverOptions.Flink; Link != &BmOrderedDriverOptions; Link = Link->Flink) {
        Option = CR(Link, BM_LOAD_OPTION, Order, BM_LOAD_OPTION_SIGNATURE);

        //
        // See if the user want to skip Driver Load by pressing F10
        //
        if (ST->ConIn != NULL) {
          while (ST->ConIn->ReadKeyStroke (ST->ConIn, &Key) == EFI_SUCCESS) {
            if (Key.ScanCode == SCAN_F10) {
              return;
            }
          }
        }

        if (Option->Attributes & LOAD_OPTION_FORCE_RECONNECT) {
            Reconnect = TRUE;
        }

        BmLoad (Option, FALSE, TRUE);
    }

    //
    // See if the user want to skip Driver Load by pressing F10
    //
    if (ST->ConIn != NULL) {
      while (ST->ConIn->ReadKeyStroke (ST->ConIn, &Key) == EFI_SUCCESS) {
        if (Key.ScanCode == SCAN_F10) {
          return;
        }
      }
    }

    //
    // If one of the drivers requested a FORCE_RECONNECT, then disconnect all the drivers
    //
    if (Reconnect) {
      DisconnectAll ();
    }

    //
    // Always connect all the drivers after all the driver have been loaded and initialized
    //
    ConnectAllConsoles ();
}

EFI_STATUS
BmLoad (
    IN BM_LOAD_OPTION   *Option,
    IN BOOLEAN          BootLoad,
    IN BOOLEAN          AutoBoot
    )
{
    EFI_STATUS              Status;
    EFI_HANDLE              ImageHandle;
    EFI_LOADED_IMAGE        *ImageInfo;
    CHAR16                  *LoadImage, *LoadImageFailed;
    CHAR16                  *StartImage, *StartImageFailed;
    EFI_DEVICE_PATH         *FilePath, *HardDriveFilePath;
    UINT32                  *PartitionNumber;
    UINTN                   NoHandles;
    EFI_HANDLE              *HandleBuffer;
    EFI_BLOCK_IO            *BlockIo;
    UINTN                   Index;
    HARDDRIVE_DEVICE_PATH   *HardDrive;

    //
    // Lookup some strings
    //
    LoadImage = BmString (BM_LOAD_IMAGE);
    LoadImageFailed = BmString (BM_LOAD_IMAGE_FAILED);

    StartImage = BmString (BM_START_IMAGE);
    StartImageFailed = BmString (BM_START_IMAGE_FAILED);

    //
    // Connect the drivers required to access the device specified by Option->FilePath
    //
    ConnectDevicePath (Option->FilePath);

    //
    // Load the image
    //
    DEBUG((D_BM | D_INFO, "BM: Loading driver %s:%s\n", 
                Option->Name,
                Option->Description
                ));

    //
    // Set BootCurrent variable
    //
    BmSetBootCurrent(TRUE, (UINT16)Option->OptionNumber);

    //
    // Load this image
    //
    if (Option->FilePath->Type == BBS_DEVICE_PATH && 
        Option->FilePath->SubType == BBS_BBS_DP    ) {
        //
        // If the device path starts with a BBS device path entry
        //  call into platform code to boot the legacy PC AT way
        //
        if (BmBootLegacy != NULL) {
            Status = BmBootLegacy->BootIt (Option->FilePath);
            if (EFI_ERROR(Status)) {
                Print (LoadImageFailed, Option->Description, Status);
                if (!AutoBoot) {
                    BmPause ();
                }
                goto Done;
            }
        } else {
            //
            // Booting from legacy devices not support on this platform
            //
            Status = EFI_UNSUPPORTED;
            goto Done;
        }
    }

    //
    // If the device path starts with a Media Hard drive node expand it
    //  to be a full device path. Add in the pointer to the hard disk controller.
    //  this allows a disk to move around and we can still boot from it.
    //
    FilePath = Option->FilePath;
    HardDriveFilePath = NULL;
    if (FilePath->Type == MEDIA_DEVICE_PATH &&
        FilePath->SubType == MEDIA_HARDDRIVE_DP) {

        HardDrive = (HARDDRIVE_DEVICE_PATH *)FilePath;

        PartitionNumber = (HardDrive->MBRType == SIGNATURE_TYPE_MBR) ? &HardDrive->PartitionNumber : NULL;
        BmLocateHandleByDiskSignatureAndPartition (
            HardDrive->MBRType,
            HardDrive->SignatureType,
            HardDrive->Signature,
            PartitionNumber,
            &NoHandles,
            &HandleBuffer
            );

        if (HandleBuffer) {
            for (Index = 0; Index < NoHandles; Index++) {
                Status = BS->HandleProtocol (HandleBuffer[Index], &BlockIoProtocol, (VOID*)&BlockIo);
                if (EFI_ERROR(Status)) {
                    continue;
                }
                if (BlockIo->Media->RemovableMedia) {
                    //
                    // Skip removeable media devices
                    //
                    continue;
                }

                HardDriveFilePath = DevicePathFromHandle (HandleBuffer[Index]);
                if (HardDriveFilePath) {
                    //
                    // We have now converted the Media Hard drive node to a 
                    //  device path that ends in the Media Hard drive node.
                    //  So add the file name at the end and we are done
                    //
                    FilePath = AppendDevicePath (
                                    HardDriveFilePath, 
                                    NextDevicePathNode (Option->FilePath)
                                    );

                    //
                    // Save the pointer so we can free it later
                    //
                    HardDriveFilePath = FilePath;
                    break;
                }
            }

            FreePool (HandleBuffer);
        }
    }

    Print (LoadImage, Option->Description);
    Status = BS->LoadImage(
                BootLoad,
                BmImageHandle,
                FilePath,
                NULL,
                0,
                &ImageHandle
                );

    if (Status == EFI_ALREADY_STARTED) {
        //
        // If image is linked in it will retun EFI_ALREADY_STARTED
        //  thus emulate a load
        //
        Status = EFI_SUCCESS;
        goto Done;
    }

    //
    // If we didn't find an image, and this is a BootLoad request we
    // may need to implement the default boot behaviour for the device.
    // The boot manager only implements the default boot behaviour for
    // filesystem devices.  Other default behaviour is handled by the 
    // device itself in the previous LoadImage request.
    //

    if (EFI_ERROR(Status) && BootLoad) {
        Status = BmDefaultBootImage (FilePath, &ImageHandle);
    }

    BmClearLine ();

    //
    // If image failed to load print a message
    //
    if (EFI_ERROR(Status)) {
        Print (LoadImageFailed, Option->Description, Status);
        if (!AutoBoot) {
            BmPause ();
        }
        goto Done;
    }    

    //
    // Provide the image with it's load optins
    //
    Status = BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, &ImageInfo);
    ASSERT (!EFI_ERROR(Status));
    if (Option->LoadOptionsSize) {
        ImageInfo->LoadOptionsSize = (UINT32) Option->LoadOptionsSize;
        ImageInfo->LoadOptions = Option->LoadOptions;
    }

    //
    // Start the image
    //
    Print (StartImage, Option->Description);

    //
    // Before calling the image, enable the Watchdog Timer for the 5 Minute period
    //

    BS->SetWatchdogTimer ( 5 * 60, 0x0000, 0x00, NULL );
    
    Status = BS->StartImage (ImageHandle, 0, NULL);

    BS->SetWatchdogTimer ( 0x0000, 0x0000, 0x0000, NULL );

    //
    // If image failed to start print a message
    //
    if (EFI_ERROR(Status)) {
        Print (StartImageFailed, Option->Description, Status);
        if (!AutoBoot) {
            BmPause ();
        }
        goto Done;
    }    

Done:    

    if (HardDriveFilePath) {
        FreePool (HardDriveFilePath);
    }

    //
    // Delete BootCurrent variable
    //
    BmSetBootCurrent(FALSE, 0);

    BmNewLine();
    return Status;
}


EFI_STATUS
BmDefaultBootImage (
    IN EFI_DEVICE_PATH      *TargetDevice,
    OUT EFI_HANDLE          *ImageHandle
    )
{
    UINTN                   PathSize;
    EFI_STATUS              Status;
    UINTN                   NoHandles, Index;
    EFI_HANDLE              *Handles;
    EFI_HANDLE              Handle;
    EFI_FILE_HANDLE         Root, Dir;
    UINTN                   FileInfoSize, BufferSize;
    EFI_FILE_INFO           *FileInfo, *DirInfo;
    CHAR16                  *FileName;    
    EFI_DEVICE_PATH         *FilePath, *DevicePath;
    EFI_LOADED_IMAGE        *ImageInfo;

    Root = NULL;
    Dir = NULL;

    PathSize = DevicePathSize(TargetDevice) - sizeof(EFI_DEVICE_PATH);

    FileInfoSize = sizeof(EFI_FILE_HANDLE) + 1024;
    FileInfo = AllocatePool (FileInfoSize);
    DirInfo = AllocatePool (FileInfoSize);
    ASSERT (FileInfo && DirInfo);

    
    //
    // Find all file system handles
    //
    LibLocateHandle (ByProtocol, &FileSystemProtocol, NULL, &NoHandles, &Handles);
    for (Index=0; Index < NoHandles; Index++) {
        Handle = Handles[Index];

        //
        // See if this is a file system on the same device
        //
        // BUGBUG: this doesn't handle instance paths
        DevicePath = DevicePathFromHandle (Handle);
        if (TargetDevice && CompareMem(DevicePath, TargetDevice, PathSize) == 0) {
            //
            // Do the removable Media thing. \EFI\BOOT\boot{machinename}.EFI
            //  machinename is ia32, ia64, ...
            //
            FilePath = FileDevicePath (Handle, DEFAULT_REMOVABLE_FILE_NAME);
            Status = BS->LoadImage (
                            TRUE,
                            BmImageHandle,
                            FilePath,
                            NULL,
                            0,
                            ImageHandle
                            );
            if (!EFI_ERROR(Status)) {
                //
                // Verify the image is an application (and not a driver)
                //
                Status = BS->HandleProtocol (*ImageHandle, &LoadedImageProtocol, &ImageInfo);
                ASSERT (!EFI_ERROR(Status));

                if (ImageInfo->ImageCodeType != EfiLoaderCode) {
                    DEBUG ((D_BM | D_INFO, "BmDefaultBootImage: Image %hs is not an application\n", FileName));
                    Status = BS->Exit(*ImageHandle, EFI_SUCCESS, 0, NULL);
                    ASSERT (!EFI_ERROR(Status));
                    Status = EFI_LOAD_ERROR;
                } else {
                    DEBUG ((D_BM | D_INFO, "BmDefaultBootImage: Loaded %hs\n", FileName));
                }
            }

            FreePool (FilePath);
            if (!EFI_ERROR(Status)) {
                //
                // If we got here we are done.
                //
                goto Done;
            }

            //
            // BugBug: This is the old code. Get rid of this when the protoype 
            //          OSes start following the EFI specifiction!
            //

            //
            // We found a file system on the device.  Open it.
            //
            Dir = LibOpenRoot(Handle);
            if (!Dir) {
                continue;
            }

            Status = Dir->Open (Dir, &Root, L"EFI", EFI_FILE_MODE_READ, 0);
            Dir->Close (Dir);
            Dir = NULL;
            if (EFI_ERROR(Status)) {
                continue;
            }

            //
            // Open the directory \EFI\BOOT on removable media
            //
            Root->SetPosition (Root, 0);
            StrCpy(DirInfo->FileName, L"BOOT");
            Status = Root->Open (Root, &Dir, DirInfo->FileName, EFI_FILE_MODE_READ, 0);
            if (EFI_ERROR(Status)) {
                goto Done;
            }

            Dir->SetPosition (Dir, 0);
            for (;;) {
                BufferSize = FileInfoSize;
                Status = Dir->Read (Dir, &BufferSize, FileInfo);

                //
                // If no more directory entries, we're done
                //
                if (EFI_ERROR(Status) || BufferSize == 0) {
                    break;
                }

                if ((FileInfo->Attribute & EFI_FILE_DIRECTORY)  ||
                        !BmIsEfiImageName(FileInfo->FileName)) {
                        continue;
                }

                //
                // Build a path to the image and see if we can load it
                //
                FileName = PoolPrint(L"EFI\\%s\\%s", DirInfo->FileName, FileInfo->FileName);
                ASSERT (FileName);
                FilePath = FileDevicePath (Handle, FileName);
                Status = BS->LoadImage (
                            TRUE,
                            BmImageHandle,
                            FilePath,
                            NULL,
                            0,
                            ImageHandle
                            );

                if (!EFI_ERROR(Status)) {
                    //
                    // Verify the image is an application (and not a driver)
                    //
                    Status = BS->HandleProtocol (*ImageHandle, &LoadedImageProtocol, &ImageInfo);
                    ASSERT (!EFI_ERROR(Status));

                    if (ImageInfo->ImageCodeType != EfiLoaderCode) {
                        DEBUG ((D_BM | D_INFO, "BmDefaultBootImage: Image %hs is not an application\n", FileName));
                        Status = BS->Exit(*ImageHandle, EFI_SUCCESS, 0, NULL);
                        ASSERT (!EFI_ERROR(Status));
                        Status = EFI_LOAD_ERROR;
                    } else {
                        DEBUG ((D_BM | D_INFO, "BmDefaultBootImage: Loaded %hs\n", FileName));
                    }
                }

                FreePool (FileName);
                FreePool (FilePath);

                if (!EFI_ERROR(Status)) {
                    goto Done;
                }
            }
            Dir->Close (Dir);
            Dir = NULL;

            Root->Close (Root);
            Root = NULL;
        }
    }

    Status = EFI_NOT_FOUND;
    *ImageHandle = NULL;

Done:

    if (Root) {
        Root->Close(Root);
    }

    if (Dir) {
        Dir->Close (Dir);
    }

    if (FileInfo) {
        FreePool (FileInfo);
    }

    if (DirInfo) {
        FreePool (DirInfo);
    }

    return Status;
}

BOOLEAN
BmIsEfiImageName (
    CHAR16  *FileName
    )
{
    // Search for ".efi" extension
    while (*FileName) {
        if (FileName[0] == '.' && StriCmp(FileName, L".EFI") == 0) {
            return TRUE;
        }

        FileName += 1;
    }

    return FALSE;
}

EFI_STATUS
BmBootNextOption(
    VOID
    )

{
    EFI_STATUS     Status;
    UINT16         BootNext;
    LIST_ENTRY     *Link;
    BM_LOAD_OPTION *Option;

    if (BmBootNext.DataSize != 0) {

        //
        // Get the BootNext value
        //
        BootNext = *(UINT16 *)BmBootNext.u.Data;

        //
        // Delete the BootNext environment variable
        //
        Status = RT->SetVariable (
                    VarBootNext,
                    &EfiGlobalVariable, 
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    0,
                    NULL
                    );

        for (Link = BmOrderedBootOptions.Flink; Link != &BmOrderedBootOptions; Link = Link->Flink) {
            Option = CR(Link, BM_LOAD_OPTION, Order, BM_LOAD_OPTION_SIGNATURE);
            if (Option->OptionNumber == BootNext) {
                Status = BmLoad (Option, TRUE, FALSE);
            }
        }
    }
    return Status;
}

EFI_STATUS
BmLocateHandleByDiskSignatureAndPartition (
    IN UINT8                        MBRType,
    IN UINT8                        SignatureType,
    IN VOID                         *Signature,
    IN UINT32                       *PartitionNumber OPTIONAL,
    IN OUT UINTN                    *NoHandles,
    OUT EFI_HANDLE                  **Buffer
    )

{
    EFI_STATUS            Status;
    UINTN                 BufferSize;
    UINTN                 NoBlockIoHandles;
    EFI_HANDLE            *BlockIoBuffer;
    EFI_DEVICE_PATH       *DevicePath;
    UINTN                 Index;
    EFI_DEVICE_PATH       *Start, *Next, *DevPath;
    HARDDRIVE_DEVICE_PATH *HardDriveDevicePath;
    HARDDRIVE_DEVICE_PATH HardDriveDevicePathData;
    BOOLEAN               Match;
    BOOLEAN               PreviousNodeIsHardDriveDevicePath;

    //
    // Initialize for GrowBuffer loop
    //
    BlockIoBuffer = NULL;
    BufferSize = 50 * sizeof(EFI_HANDLE);

    //
    // Call the real function
    //
    while (GrowBuffer (&Status, (VOID **)&BlockIoBuffer, BufferSize)) {

        //
        // Get list of device handles that support the BLOCK_IO Protocol.
        //
        Status = BS->LocateHandle (
                        ByProtocol,
                        &BlockIoProtocol,
                        NULL,
                        &BufferSize,
                        BlockIoBuffer
                        );

    }

    NoBlockIoHandles = BufferSize / sizeof (EFI_HANDLE);
    if (EFI_ERROR(Status)) {
        NoBlockIoHandles = 0;
    }

    //
    // If there was an error or there are no device handles that support 
    // the BLOCK_IO Protocol, then return.
    //
    if (NoBlockIoHandles == 0) {
        FreePool(BlockIoBuffer);
        *NoHandles = 0;
        *Buffer = NULL;
        return Status;
    }

    //
    // Loop through all the device handles that support the BLOCK_IO Protocol
    //
    *NoHandles = 0;

    for(Index=0;Index<NoBlockIoHandles;Index++) {

        Status = BS->HandleProtocol (BlockIoBuffer[Index], 
                                     &DevicePathProtocol, 
                                     (VOID*)&DevicePath
                                     );

        //
        // Search DevicePath for a Hard Drive Media Device Path node.
        // If one is found, then see if it matches the signature that was
        // passed in.  If it does match, and the next node is the End of the
        // device path, and the previous node is not a Hard Drive Media Device
        // Path, then we have found a match.
        //

        Match = FALSE;

        if (DevicePath != NULL) {

            PreviousNodeIsHardDriveDevicePath = FALSE;

            DevPath = DevicePath;
            Start = DevPath;

            //
            // Check for end of device path type
            //    
            for (; ;) {

                if ((DevicePathType(DevPath) == MEDIA_DEVICE_PATH) &&
                    (DevicePathSubType(DevPath) == MEDIA_HARDDRIVE_DP)) {

                    //
                    // Ensure data structure is properly aligned.
                    //
                    CopyMem (&HardDriveDevicePathData, DevPath, sizeof (HARDDRIVE_DEVICE_PATH));
                    HardDriveDevicePath = &HardDriveDevicePathData;

                    if (PreviousNodeIsHardDriveDevicePath == FALSE) {

                        Next = NextDevicePathNode(DevPath);
                        if (IsDevicePathEndType(Next)) {
                            if ((HardDriveDevicePath->MBRType == MBRType) &&
                                (HardDriveDevicePath->SignatureType == SignatureType)) {
                                switch(SignatureType) {
                                    case SIGNATURE_TYPE_MBR:
                                        if (*((UINT32 *)(Signature)) == *(UINT32 *)(&(HardDriveDevicePath->Signature[0]))) {
                                            Match = TRUE;
                                        }
                                        if (PartitionNumber) {
                                            //
                                            // If the PartitionNumber exists match on it too
                                            //
                                            if (*PartitionNumber != HardDriveDevicePath->PartitionNumber) {
                                                Match = FALSE;
                                            }
                                        }
                                        break;
                                    case SIGNATURE_TYPE_GUID:
                                        if (CompareGuid((EFI_GUID *)Signature,(EFI_GUID *)(&(HardDriveDevicePath->Signature[0]))) == 0) {
                                            Match = TRUE;
                                        }
                                        break;
                                }
                            }
                        }
                    }
                    PreviousNodeIsHardDriveDevicePath = TRUE;
                } else {
                    PreviousNodeIsHardDriveDevicePath = FALSE;
                }

                if (IsDevicePathEnd(DevPath)) {
                    break;
                }

                DevPath = NextDevicePathNode(DevPath);
            }

        }

        if (Match == FALSE) {
            BlockIoBuffer[Index] = NULL;
        } else {
            *NoHandles = *NoHandles + 1;
        }
    }

    //
    // If there are no matches, then return
    //
    if (*NoHandles == 0) {
        FreePool(BlockIoBuffer);
        *NoHandles = 0;
        *Buffer = NULL;
        return EFI_SUCCESS;
    }

    //
    // Allocate space for the return buffer of device handles.
    //
    *Buffer = AllocatePool(*NoHandles * sizeof(EFI_HANDLE));

    if (*Buffer == NULL) {
        FreePool(BlockIoBuffer);
        *NoHandles = 0;
        *Buffer = NULL;
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Build list of matching device handles.
    //
    *NoHandles = 0;
    for(Index=0;Index<NoBlockIoHandles;Index++) {
        if (BlockIoBuffer[Index] != NULL) {
            (*Buffer)[*NoHandles] = BlockIoBuffer[Index];
            *NoHandles = *NoHandles + 1;
        }
    }

    FreePool(BlockIoBuffer);

    return EFI_SUCCESS;
}

