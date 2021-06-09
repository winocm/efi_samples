/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name: load.c


Abstract:  PE32+ loader.


--*/

#include "loader.h"

#include EFI_PROTOCOL_DEFINITION(Ebc)

//
//
//

typedef struct _START_IMAGE {
    LOADED_IMAGE            *Image;
    EFI_TPL                 Tpl;
    EFI_STATUS              Status;
    UINTN                   ExitDataSize;
    VOID                    *ExitData;
    EFI_JUMP_BUFFER         Exit;
} START_IMAGE;

STATIC START_IMAGE          *CurrentStartImage;
STATIC EFI_HANDLE           EfiCoreImageHandle;


//
// Internal data
//

EFI_GUID InternalImageStructure = 
    { 0x280a011, 0x9ab0, 0x11d2, 0x8e, 0x40, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b };


//
// Prototypes 
//

STATIC
EFI_STATUS
LoadedImageProtocolHandler (
    IN VOID             *HandlerContext,
    IN EFI_GUID         *Protocol,
    OUT VOID            **ProtocolInterface
    );

VOID
UnloadImage (
    IN LOADED_IMAGE     *Image
    );


//
//
//

VOID
InitializeLoader (
    VOID
    )
{
	EfiCoreImageHandle = NULL;
    InitializeListHead (&BootImageList);
    InitializeListHead (&RuntimeImageList);
    InitializeLock (&ImageListLock, TPL_NOTIFY);
}


STATIC
LOADED_IMAGE *
LoadedImageInfo (
    IN EFI_HANDLE       ImageHandle
    )
{
    LOADED_IMAGE        *Image;
    EFI_STATUS          Status;

    Status = BS->HandleProtocol (
                    ImageHandle,
                    &InternalImageStructure,
                    &Image
                    );

    if (EFI_ERROR(Status)) {
        DEBUG ((D_LOAD, "LoadedImageInfo: Not an ImageHandle %x\n", ImageHandle));
        Image = NULL;
    }

    return Image;
}

INTERNAL
EFI_STATUS
SetImageType (
    IN OUT LOADED_IMAGE             *Image,
    IN UINTN                        ImageType
    )
{
    EFI_MEMORY_TYPE                 CodeType, DataType;

    switch (ImageType) {
    case IMAGE_SUBSYSTEM_EFI_APPLICATION:
        CodeType = EfiLoaderCode;
        DataType = EfiLoaderData;
        break;

    case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
        CodeType = EfiBootServicesCode;
        DataType = EfiBootServicesData;
        break;

    case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
        CodeType = EfiRuntimeServicesCode;
        DataType = EfiRuntimeServicesData;
        break;
    default:
        return EFI_INVALID_PARAMETER;
    }

    Image->Type = ImageType;
    Image->Info.ImageCodeType = CodeType;    
    Image->Info.ImageDataType = DataType;
    return EFI_SUCCESS;
}

INTERNAL
EFI_STATUS
CheckImageMachineType (
    IN UINTN            MachineType
    )
// Determine if machine type is supported by the local machine
{
    EFI_STATUS          Status;

    Status = EFI_UNSUPPORTED;

#if EFI32
    if (MachineType == EFI_IMAGE_MACHINE_IA32) {
        Status = EFI_SUCCESS;
    }
#endif
    
#if EFI64
    if (MachineType == EFI_IMAGE_MACHINE_IA64) {
        Status = EFI_SUCCESS;
    }
#endif

    if (MachineType == EFI_IMAGE_MACHINE_EBC) {
        Status = EFI_SUCCESS;
    }

    return Status;
}


//
//
//

EFI_STATUS
FwLoadInternal (
    IN UINTN                        ImageType,    
    IN CHAR16                       *InternalName,
    IN EFI_IMAGE_ENTRY_POINT        ImageEntryPoint OPTIONAL
    )
{
    LOADED_IMAGE                    *Image;
    EFI_STATUS                      Status;

    //
    // Allocate a new image structure
    //

    Image = AllocateZeroPool(sizeof(LOADED_IMAGE));
    if (!Image) {
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Initialize image info fields
    //

    Image->Signature = LOADED_IMAGE_SIGNATURE;
    Image->Info.Revision = EFI_LOADED_IMAGE_INFORMATION_REVISION;
    Image->Info.SystemTable = ST;
    Image->Info.FilePath = FileDevicePath (NULL, InternalName);
	Image->Info.ParentHandle = EfiCoreImageHandle;
    Image->Name = DevicePathToStr(Image->Info.FilePath);    // for debugging
    Image->EntryPoint = ImageEntryPoint;
    SetImageType (Image, ImageType);

    //
    // If there's an entry point then this image is directly
    // linked in; otherwise, it's being emulated as a linked
    // together driver.   This is only done on builds that
    // have EmulateLoad
    //

    if (!ImageEntryPoint) {

        //
        // Call the emulator to load the internal image
        //

        ASSERT (PL->EmulateLoad);
        Status = PL->EmulateLoad (
                        InternalName, 
                        &Image->Info,
                        &Image->EntryPoint
                        );

        //
        // If ther was an error, clean up and exit
        //

        if (EFI_ERROR(Status)) {
            UnloadImage (Image);
            goto Done;
        }
    }

    //
    // Install the protocol interfaces for the image
    //

    Status = LibInstallProtocolInterfaces (
                &Image->Handle, 
                &LoadedImageProtocol,       &Image->Info,    
                &InternalImageStructure,    Image,
                NULL
                );

    ASSERT (!EFI_ERROR(Status));

    //
    // Start the image
    //
    
    Status = StartImage (Image->Handle, 0, NULL);

Done:
    return Status;
}

EFI_STATUS
BuildEfiCoreImageHandle (
  IN  VOID                 *EntryPoint,
  IN  VOID                 *BaseAddress,
  IN  UINT64               Size,
  OUT EFI_HANDLE           *CoreImageHandle
  )

{
  EFI_STATUS                  Status;
  LOADED_IMAGE                *Image;

  *CoreImageHandle = NULL;

  Image = AllocateZeroPool(sizeof(LOADED_IMAGE));
  if (Image == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Initialize the fields for an internal driver
  //
  Image->Signature     = LOADED_IMAGE_SIGNATURE;
  Image->Info.Revision = EFI_LOADED_IMAGE_INFORMATION_REVISION;
  Image->Type          = IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
  Image->Started       = TRUE;
  Image->EntryPoint    = (EFI_IMAGE_ENTRY_POINT)(UINTN)EntryPoint;
  Image->ImageBasePage = (EFI_PHYSICAL_ADDRESS)BaseAddress;
  Image->NumberOfPages = EFI_SIZE_TO_PAGES((UINT32)Size);

  Image->Info.SystemTable = ST;
  Image->Info.DeviceHandle = NULL;

  Image->Info.FilePath        = FileDevicePath(NULL, L"EFI Core");

  Image->Info.ParentHandle    = NULL;
  Image->Info.SystemTable     = ST;

  Image->Info.ImageBase       = BaseAddress;
  Image->Info.ImageSize       = Size;
  Image->Info.ImageCodeType   = EfiBootServicesCode;
  Image->Info.ImageDataType   = EfiBootServicesData;

  //
  // Install the protocol interfaces for this image
  //
  Status = LibInstallProtocolInterfaces (
              &Image->Handle, 
              &LoadedImageProtocol,       &Image->Info,    
              &InternalImageStructure,    Image,
              NULL
              );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  *CoreImageHandle   = Image->Handle;
  EfiCoreImageHandle = Image->Handle;

  //
  // Add new image handle to debug image info table
  // This step is optional and not required by architecture.  It is
  // used by debuggers to determine image information
  //
  
  NewDebugImageInfoEntry (
    EFI_DEBUG_IMAGE_INFO_TYPE_NORMAL, 
    (EFI_LOADED_IMAGE_PROTOCOL *) &Image->Info, 
    Image->Handle
    );

  return EFI_SUCCESS;
}

EFI_STATUS
LoadImage (
    IN BOOLEAN                  BootPolicy,
    IN EFI_HANDLE               ParentImageHandle,
    IN EFI_DEVICE_PATH          *FilePath,
    IN VOID                     *SourceBuffer   OPTIONAL,
    IN UINTN                    SourceSize,
    OUT EFI_HANDLE              *ImageHandle
    )
{
    LOADED_IMAGE                *Image, *ParentImage;
    LOADED_IMAGE                *RuntimeImage;
    SIMPLE_READ_FILE            FHand;
    EFI_STATUS                  Status;
    EFI_HANDLE                  DeviceHandle;

    ASSERT (CurrentTPL() < TPL_NOTIFY);
    ParentImage = NULL;

    //
    // The caller must always pass in a valid ImageHandle and ParentImageHandle
    //
    if (ImageHandle == NULL || ParentImageHandle == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // If there's a parent image handle, look it up
    //
    ParentImage = LoadedImageInfo(ParentImageHandle);
    if (!ParentImage) {
        DEBUG((D_LOAD|D_ERROR, "LoadImage: Parent handle not an image handle\n"));
        return EFI_INVALID_PARAMETER;
    }

    //
    // If SourceBuffer is NULL, then FilePath must be valid
    //
    if (SourceBuffer == NULL && FilePath == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Get simple read access to the source file
    //
    Status = OpenSimpleReadFile (
                BootPolicy,
                SourceBuffer, 
                SourceSize,
                &FilePath,
                &DeviceHandle,
                &FHand
                );

    if (Status == EFI_ALREADY_STARTED) {
        Image = NULL;
        goto Done;
    } else if (EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Allocate a new image structure
    //
    Image = AllocateZeroPool(sizeof(LOADED_IMAGE));
    if (!Image) {
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Initialize the fields for an internal driver
    //
    Image->Signature = LOADED_IMAGE_SIGNATURE;
    Image->Info.Revision = EFI_LOADED_IMAGE_INFORMATION_REVISION;
    Image->Info.SystemTable = ST;
    Image->Info.DeviceHandle = DeviceHandle;

    Image->Info.FilePath = DuplicateDevicePath (FilePath);
    Image->Name = DevicePathToStr(Image->Info.FilePath);

    if (ParentImage) {
        Image->Info.ParentHandle = ParentImageHandle;
        Image->Info.SystemTable = ParentImage->Info.SystemTable;
    }

    //
    // Load Pe image types
    //
    Status = LoadPeImage (FHand, Image);

    if (EFI_ERROR(Status)) {
        goto Done;
    }

    //
    // If the image was sucessfully loaded, and it's a runtime image 
    // move its loaded_image structure to runtime memory
    //
    AcquireLock (&ImageListLock);

    if (Image->Type == IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER) {

        // Allocate a new buffer
        Status = BSAllocatePool (EfiRuntimeServicesData, sizeof(LOADED_IMAGE), &RuntimeImage);
        if (EFI_ERROR(Status)) {
            goto Done;
        }

        // Move the buffer to the runtime copy, and free the non-runtime buffer        
        CopyMem (RuntimeImage, Image, sizeof(LOADED_IMAGE));
        FreePool (Image);
        Image = RuntimeImage;

        // Keep track of all the runtime driver images
        InsertTailList (&RuntimeImageList, &Image->ImageLink);

    } else {

        //
        // Image is not a runtime driver.. If there's a fixup table free it
        //

        if (Image->FixupData) {
           FreePool (Image->FixupData);
           Image->FixupData = NULL;
        }
    }

    //
    // If the image is a boot service driver keep it in a list
    //

    if (Image->Type == IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER) {
        InsertTailList (&BootImageList, &Image->ImageLink);
    }

    ReleaseLock (&ImageListLock);

    //
    // Install the protocol interfaces for this image
    //
    Status = LibInstallProtocolInterfaces (
                &Image->Handle, 
                &LoadedImageProtocol,       &Image->Info,    
                &InternalImageStructure,    Image,
                NULL
                );

    if (EFI_ERROR(Status)) {
        goto Done;
    }

    //
    // Success.  Return the image handle
    // 

    *ImageHandle = Image->Handle;

    //
    // Add new image handle to debug image info table
    // This step is optional and not required by architecture.  It is
    // used by debuggers to determine image information
    //
    
    NewDebugImageInfoEntry (
      EFI_DEBUG_IMAGE_INFO_TYPE_NORMAL, 
      (EFI_LOADED_IMAGE_PROTOCOL *) &Image->Info, 
      Image->Handle
      );

Done:
    //
    // All done accessing the source file
    //

    CloseSimpleReadFile (FHand);
    
    //
    // There was an error.  If there's an Image structure, free it
    //

    if (EFI_ERROR(Status)) {
        if (Image) {
            UnloadImage (Image);
            *ImageHandle = NULL;
        }
    }

    return Status;
}
    
EFI_STATUS
BOOTSERVICE
StartImage (
    IN EFI_HANDLE           ImageHandle,
    OUT UINTN               *ExitDataSize,
    OUT CHAR16              **ExitData  OPTIONAL
    )
{
    START_IMAGE             Si;
    START_IMAGE             *LastSi;
    BOOLEAN                 Flag;
    UINT64                  HandleDatabaseKey;

    ZeroMem (&Si, sizeof(Si));
    Si.Image = LoadedImageInfo(ImageHandle);
    if (!Si.Image  ||  Si.Image->Started) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Save some information about the handle database
    //
    HandleDatabaseKey = CoreGetHandleDatabaseKey();

    //
    // Push the current start image context, and 
    // link the current image to the head.   This is the
    // only image that can call Exit()
    // 

    LastSi = CurrentStartImage;
    CurrentStartImage = &Si;
    Si.Image->StartImageContext = &Si;
    Si.Tpl = CurrentTPL();

    //
    // Set long jump for Exit() support
    //

    Flag = SetJump (&Si.Exit);
    if (!Flag) {

        //
        // Call platform code to set handoff state
        //
        
        if (PL->SI_HandoffState) {
            PL->SI_HandoffState();
        }
        
        //
        // Call the image's entry point
        //

        Si.Image->Started = TRUE;
        DEBUG ((D_LOAD, "StartImage: Calling %hs entry point\n", Si.Image->Name));
        Si.Status = Si.Image->EntryPoint (ImageHandle, Si.Image->Info.SystemTable);

        //
        // If the image returns, exit it through Exit()
        //

        BS->Exit(ImageHandle, Si.Status, 0, NULL);

        //
        // Call platform code to verify state on exit
        //
        
        if (PL->EI_ReturnState) {
            PL->EI_ReturnState();
        }
    }

    //
    // Image has completed.  Verify the tpl is the same
    //

    ASSERT (Si.Tpl == CurrentTPL());
    RestoreTPL (Si.Tpl);
    
    //
    // Pop the current start image context
    //
    
    Si.Image->StartImageContext = NULL;
    CurrentStartImage = LastSi;
    DEBUG ((D_LOAD, "StartImage: Image %hs returned %x\n", Si.Image->Name, Si.Status));

    //
    // Connect any handles that were created or modified while the image executed.
    //
    CoreConnectHandlesByKey (HandleDatabaseKey);

    //
    // Handle the image's returned ExitData
    //

    if (ExitData != NULL) {

        DEBUG ((D_LOAD, "StartImage: ExitDataSize %d, ExitData %x (%hs)\n", 
                            Si.ExitDataSize,
                            Si.ExitData,
                            Si.ExitData
                            ));

        //
        // Return the exit data to the caller
        //

        if (ExitDataSize != NULL) {

            *ExitDataSize = Si.ExitDataSize;
            *ExitData = Si.ExitData;

        } else {

            //
            // Caller doesn't want the exit data, free it
            //
            
            FreePool (ExitData);
        }
    }

    //
    // If the image returned an error, of if the image is an application
    // unload it
    //

    if (EFI_ERROR(Si.Status) || Si.Image->Type == IMAGE_SUBSYSTEM_EFI_APPLICATION) {
        UnloadImage (Si.Image);
    }

    //
    // Done
    //

    return Si.Status;
}


VOID
UnloadImage (
    IN LOADED_IMAGE     *Image
    )
{
    EFI_STATUS                          Status;
    UINTN                               HandleCount;
    EFI_HANDLE                          *HandleBuffer;
    UINTN                               HandleIndex;
    EFI_GUID                            **ProtocolGuidArray;
    UINTN                               ArrayCount;
    UINTN                               ProtocolIndex;
    EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
    UINTN                               OpenInfoCount;
    UINTN                               OpenInfoIndex;

    DEBUG((D_LOAD, "UnloadImage: unloading %s\n", Image->Name));

    //
    // If this image is on the runtime list, free it
    //

    if (Image->ImageLink.Flink) {
        AcquireLock (&ImageListLock);
        RemoveEntryList (&Image->ImageLink);
        ReleaseLock (&ImageListLock);
    }

    //
    // Free our references to the image handle
    //

    if (Image->Handle) {

        Status = LibLocateHandle (
                        AllHandles,   
                        NULL,
                        NULL,
                        &HandleCount, 
                        &HandleBuffer
                        );
        if (!EFI_ERROR (Status)) {
          for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    			  Status = EFI_UNSUPPORTED;
            ArrayCount = 0;
            ProtocolGuidArray = NULL;
            Status = BS->ProtocolsPerHandle (
                           HandleBuffer[HandleIndex], 
                            &ProtocolGuidArray, 
                            &ArrayCount
                            );
            if (!EFI_ERROR (Status)) {
              for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
                Status = BS->OpenProtocolInformation (
                                HandleBuffer[HandleIndex], 
                                ProtocolGuidArray[ProtocolIndex],
                                &OpenInfo,
                                &OpenInfoCount
                                );
                if (!EFI_ERROR (Status)) {
                  for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
                    if (OpenInfo[OpenInfoIndex].AgentHandle == Image->Handle) {
                      Status = BS->CloseProtocol (
                                      HandleBuffer[HandleIndex],
                                      ProtocolGuidArray[ProtocolIndex],
                                      OpenInfo[OpenInfoIndex].AgentHandle,
                                      OpenInfo[OpenInfoIndex].ControllerHandle
                                      );
                    }
                  }
                  if (OpenInfo != NULL) {
                    BS->FreePool(OpenInfo);
                  }
                }
              }
              if (ProtocolGuidArray != NULL) {
                BS->FreePool(ProtocolGuidArray);
              }
            }
          }
          if (HandleBuffer != NULL) {
            BS->FreePool (HandleBuffer);
          }
        }

        //
        // Remove image handle from debug image info table
        // This step is optional and not required by architecture.  It is
        // used by debuggers to determine image information
        //
    
        DeleteDebugImageInfoEntry (Image->Handle);
        
        LibUninstallProtocolInterfaces (
            Image->Handle, 
            &LoadedImageProtocol,       &Image->Info,    
            &InternalImageStructure,    Image,
            NULL
            );
    }

    //
    // Free the Image from memory
    //

    if (Image->ImageBasePage) {
        FreePages (Image->ImageBasePage, Image->NumberOfPages);
    }

    //
    // Done with the Image structure
    //

    if (Image->Info.FilePath) {
        FreePool (Image->Info.FilePath);
    }

    if (Image->Name) {
        FreePool (Image->Name);
    }

    if (Image->FixupData) {
        FreePool (Image->FixupData);
    }

    FreePool (Image);
}


EFI_STATUS
BOOTSERVICE
Exit (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_STATUS       Status,
    IN UINTN            ExitDataSize,
    IN CHAR16           *ExitData  OPTIONAL
    )
{
    LOADED_IMAGE        *Image;
    START_IMAGE         *Si;
    
    Image = LoadedImageInfo(ImageHandle);
    if (!Image) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // If this Image has been started, then unload it
    //
    
    if (!Image->Started) {
        UnloadImage (Image);
        return EFI_SUCCESS;
    }

    //
    // Image has been started, verify this image can exit
    //

    if (!Image->StartImageContext || 
        Image->StartImageContext != CurrentStartImage) {

        DEBUG ((D_LOAD|D_ERROR, "Exit: Image is not exitable image\n"));
        return EFI_INVALID_PARAMETER;
    }

    //
    //  Set status
    //

    Si = (START_IMAGE *) Image->StartImageContext;
    Si->Status = Status;

    //
    // If there's ExitData info, move it
    //

    if (ExitData) {
        Si->ExitData = ReallocatePool (ExitData, ExitDataSize, ExitDataSize);
        Si->ExitDataSize = ExitDataSize;
    }

    //
    // return to StartImage
    //

    LongJump (&Si->Exit);
    ASSERT (FALSE);
    return EFI_ACCESS_DENIED;
}

#pragma RUNTIME_CODE(RtLoaderExitBootServices)
VOID
RUNTIMEFUNCTION
RtLoaderExitBootServices (
    VOID
    )
{
    LIST_ENTRY          *Link;
    LOADED_IMAGE        *Image;

    //
    // Walk all the boot service driver images
    //

    Link = BootImageList.Flink;
    while (Link != &BootImageList) {
        Image = CR(Link, LOADED_IMAGE, ImageLink, LOADED_IMAGE_SIGNATURE);
        Link = Link->Flink;

        //
        // Clear memory of old components
        //

        ZeroMem (Image->ImageBase, Image->NumberOfPages << EFI_PAGE_SHIFT);
        ZeroMem (Image, sizeof(LOADED_IMAGE));
    }

    //
    // Walk all the runtime images
    //

    for(Link=RuntimeImageList.Flink; Link != &RuntimeImageList; Link=Link->Flink) {
        Image = CR(Link, LOADED_IMAGE, ImageLink, LOADED_IMAGE_SIGNATURE);

        // 
        // Clear non-runtime pointers
        //

        Image->Handle = NULL;
        Image->Name = NULL;
        Image->Info.ParentHandle = NULL;
        Image->Info.SystemTable = NULL;
        Image->Info.DeviceHandle = NULL;
        Image->Info.FilePath = NULL;
        Image->Info.LoadOptions = NULL;
    }
}

#pragma RUNTIME_CODE(RtLoaderVirtualAddressFixup)    
VOID
RUNTIMEFUNCTION
RtLoaderVirtualAddressFixup (
    VOID
    )
// Switching into virtual mode. reapply fix ups to all runtime images
{
    LIST_ENTRY              *Link;
    LOADED_IMAGE            *Image;

    for(Link=RuntimeImageList.Flink; Link != &RuntimeImageList; Link=Link->Flink) {
        Image = CR(Link, LOADED_IMAGE, ImageLink, LOADED_IMAGE_SIGNATURE);
        ConvertPeImage (Image);
        RT->ConvertPointer (0, &Image->ImageBase);
        RT->ConvertPointer (0, &Image->ImageEof);
        RT->ConvertPointer (0, &Image->Info.ImageBase);
    }

    RtConvertList(EFI_INTERNAL_PTR, &RuntimeImageList);
}



EFI_STATUS
BSUnloadImage (
    IN EFI_HANDLE                   ImageHandle
    )
// Unload a driver
{
  EFI_STATUS              Status;
  LOADED_IMAGE            *Image;
  EFI_EBC_PROTOCOL        *EbcProtocol;

  //
  // Make sure ImageHandle is a valid handle
  //
  if (ImageHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the Loaded Image Protocol that is installed on ImageHandle
  //
  Image = LoadedImageInfo (ImageHandle);

  //
  // If ImageHandle does not support a Loaded Image Protocol, then return an error
  //
  if (Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If this Image hasn't been started, then it can be unloaded immediately
  //
  if (!Image->Started) {
    UnloadImage (Image);
    return EFI_SUCCESS;
  }

  //
  // The image has been started, request it to unload.
  // If the Loaded Image Protocol does not have an Unload() function, then the 
  // image can not be unloaded, so return an error.
  //
  if (Image->Info.Unload == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Call the image specific Unload() function.  If it fails, then return an error
  //
  Status = Image->Info.Unload (ImageHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Determine if this is an EBC image. If so, then make a call into the
  // EBC protocol to perform any cleanup for this image.
  //
  if (Image->Machine == EFI_IMAGE_MACHINE_EBC) {
    //
    // Try to get the EBC protocol
    //
    Status = BS->LocateProtocol (&gEfiEbcProtocolGuid, NULL, &EbcProtocol);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Free the thunks that were created when the EBC image was loaded and initialized
    //
    Status = EbcProtocol->UnloadImage (EbcProtocol, ImageHandle);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  //
  // Unload the image from system memory
  //
  UnloadImage (Image);

  return EFI_SUCCESS;
}
