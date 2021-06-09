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

  UsbMassStorage.c
    
Abstract:

  USB Mass Storage Driver

Revision History

--*/

#include "UsbMassStorage.h"
#include "UsbMassStorageHelper.h"

extern EFI_COMPONENT_NAME_PROTOCOL gUsbMassStorageComponentName;

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
USBMassStorageDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
USBFloppyDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
USBFloppyDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
USBFloppyDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Block I/O Protocol Interface
//
STATIC
EFI_STATUS 
EFIAPI 
USBFloppyReset (
    IN  EFI_BLOCK_IO_PROTOCOL  *This, 
    IN  BOOLEAN                 ExtendedVerification
    );

STATIC
EFI_STATUS 
EFIAPI 
USBFloppyReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32            MediaId,
  IN  EFI_LBA           LBA,
  IN  UINTN           BufferSize,
  OUT VOID            *Buffer
    );

STATIC
EFI_STATUS 
EFIAPI 
USBFloppyWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32            MediaId,
  IN  EFI_LBA           LBA,
  IN  UINTN           BufferSize,
  IN  VOID            *Buffer
    );
    
STATIC
EFI_STATUS 
EFIAPI 
USBFloppyFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This
  );    


 

//
// USB Floppy Driver Global Variables
//

EFI_DRIVER_BINDING_PROTOCOL gUSBFloppyDriverBinding = {
  USBFloppyDriverBindingSupported,
  USBFloppyDriverBindingStart,
  USBFloppyDriverBindingStop,
  0x10,
  NULL,
  NULL
};


EFI_DRIVER_ENTRY_POINT(USBMassStorageDriverBindingEntryPoint)

EFI_STATUS
USBMassStorageDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
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
  EFI_STATUS  Status;

  //
  // Install driver binding protocol
  //
  Status = EfiLibInstallAllDriverProtocols (
            ImageHandle, 
            SystemTable, 
            &gUSBFloppyDriverBinding, 
            ImageHandle,
            &gUsbMassStorageComponentName,
            NULL,
            NULL
           );

  return Status;
}


EFI_STATUS
USBFloppyDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
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
  EFI_STATUS                OpenStatus;
  EFI_USB_ATAPI_PROTOCOL    *AtapiProtocol;

  //
  // check whether EFI_USB_ATAPI_PROTOCOL exists, if it does,
  // then the controller must be a USB Mass Storage Controller
  //
  OpenStatus = gBS->OpenProtocol (
                          Controller,       
                          &gEfiUsbAtapiProtocolGuid, 
                          &AtapiProtocol,
                          This->DriverBindingHandle,   
                          Controller,   
                          EFI_OPEN_PROTOCOL_BY_DRIVER
                          );
  if (EFI_ERROR(OpenStatus)) {
    return OpenStatus;
  }
  
  gBS->CloseProtocol (
         Controller,  
         &gEfiUsbAtapiProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
  
  return EFI_SUCCESS;
} 


EFI_STATUS
USBFloppyDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Start.
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
  
--*/       
{ 
  EFI_STATUS          Status; 
  EFI_USB_ATAPI_PROTOCOL    *AtapiProtocol;
  USB_FLOPPY_DEV      *UsbFloppyDevice = NULL;
  
  //
  // Check whether Usb Atapi Protocol attached on the controller handle.
  //  
  Status = gBS->OpenProtocol (
                      Controller,   
                      &gEfiUsbAtapiProtocolGuid,  
                      &AtapiProtocol,
                      This->DriverBindingHandle,     
                      Controller,   
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                    );                              
  if(EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(USB_FLOPPY_DEV),
                  &UsbFloppyDevice
                  );
  if(EFI_ERROR(Status)) {
    gBS->CloseProtocol (  
        Controller,
        &gEfiUsbAtapiProtocolGuid,
        This->DriverBindingHandle,
        Controller
        ); 
    return Status;
  }

  EfiZeroMem(UsbFloppyDevice,sizeof(USB_FLOPPY_DEV));   
  
  UsbFloppyDevice->Handle             = Controller;
  UsbFloppyDevice->BlkIo.Media        = &UsbFloppyDevice->BlkMedia;
  UsbFloppyDevice->Signature          = USB_FLOPPY_DEV_SIGNATURE;
  UsbFloppyDevice->BlkIo.Reset        = USBFloppyReset;
  UsbFloppyDevice->BlkIo.ReadBlocks   = USBFloppyReadBlocks;
  UsbFloppyDevice->BlkIo.WriteBlocks  = USBFloppyWriteBlocks;
  UsbFloppyDevice->BlkIo.FlushBlocks  = USBFloppyFlushBlocks;
  
  UsbFloppyDevice->AtapiProtocol = AtapiProtocol;

  //
  // Identify drive type and retrieve media information.
  //
  Status = USBFloppyIdentify(UsbFloppyDevice);
  if(EFI_ERROR(Status)) {
    if (UsbFloppyDevice->SenseData != NULL) {
      gBS->FreePool(UsbFloppyDevice->SenseData);
    }
    gBS->FreePool(UsbFloppyDevice);    
    gBS->CloseProtocol (  
        Controller,
        &gEfiUsbAtapiProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );
    return Status;
  }    

  //
  // Install Block I/O protocol for the usb floppy device.
  //
  Status = gBS->InstallProtocolInterface(
                  &Controller,
                  &gEfiBlockIoProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UsbFloppyDevice->BlkIo
                  );
  if (EFI_ERROR(Status)) {
    if (UsbFloppyDevice->SenseData != NULL) {
      gBS->FreePool(UsbFloppyDevice->SenseData);
    }
    gBS->FreePool(UsbFloppyDevice);    
    gBS->CloseProtocol (  
        Controller,
        &gEfiUsbAtapiProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );
    return Status;
  }

  return EFI_SUCCESS;
  
}


EFI_STATUS
USBFloppyDriverBindingStop (
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
  EFI_STATUS              Status;
  USB_FLOPPY_DEV          *UsbFloppyDevice;
  EFI_BLOCK_IO_PROTOCOL   *BlkIo;

  //
  // First find USB_FLOPPY_DEV
  //
  gBS->OpenProtocol(
             Controller,   
             &gEfiBlockIoProtocolGuid,  
             &BlkIo,
             This->DriverBindingHandle,     
             Controller,   
             EFI_OPEN_PROTOCOL_GET_PROTOCOL
             );

  UsbFloppyDevice = USB_FLOPPY_DEV_FROM_THIS(BlkIo);

  //
  // Uninstall Block I/O protocol from the device handle
  //
  Status = gBS->UninstallProtocolInterface(
                  Controller,
                  &gEfiBlockIoProtocolGuid,    
                  &UsbFloppyDevice->BlkIo
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //  
  // Stop using EFI_USB_ATAPI_PROTOCOL
  //
  gBS->CloseProtocol (
           Controller, 
           &gEfiUsbAtapiProtocolGuid, 
           This->DriverBindingHandle,   
           Controller   
           );

  if (UsbFloppyDevice->SenseData != NULL) {
    gBS->FreePool (UsbFloppyDevice->SenseData);
  }

  gBS->FreePool (UsbFloppyDevice);
      
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS 
EFIAPI 
USBFloppyReset (
    IN  EFI_BLOCK_IO_PROTOCOL   *This, 
    IN  BOOLEAN                 ExtendedVerification
    )
/*++

  Routine Description:
    Implements EFI_BLOCK_IO_PROTOCOL.Reset() function.
  
  Arguments:
    This:     The EFI_BLOCK_IO_PROTOCOL instance.
    ExtendedVerification:
              Indicates that the driver may perform a more exhaustive
              verification operation of the device during reset.
              (This parameter is ingored in this driver.)
    
  Returns:  

--*/      
{
  USB_FLOPPY_DEV              *UsbFloppyDevice;
  EFI_USB_ATAPI_PROTOCOL      *UsbAtapiInterface;
  EFI_STATUS                  Status;
  
  UsbFloppyDevice = USB_FLOPPY_DEV_FROM_THIS(This);
  
  UsbAtapiInterface = UsbFloppyDevice->AtapiProtocol;
  
  //
  // directly calling EFI_USB_ATAPI_PROTOCOL.Reset() to implement reset.
  //
  Status = UsbAtapiInterface->UsbAtapiReset(UsbAtapiInterface, TRUE);
  
  return Status;
} 

STATIC
EFI_STATUS 
EFIAPI 
USBFloppyReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  OUT VOID                    *Buffer
  )
/*++

  Routine Description:
    Implements EFI_BLOCK_IO_PROTOCOL.ReadBlocks() function.
  
  Arguments:
    This:     The EFI_BLOCK_IO_PROTOCOL instance.
    MediaId:  The media id that the read request is for.
    LBA:      The starting logical block address to read from on the device.
    BufferSize:
              The size of the Buffer in bytes. This must be a multiple of 
              the intrinsic block size of the device.
    Buffer:   A pointer to the destination buffer for the data. The caller 
              is responsible for either having implicit or explicit ownership
              of the buffer.                               
  
  Returns:  

--*/      
{
  USB_FLOPPY_DEV        *UsbFloppyDevice;
  EFI_STATUS            Status;  
  EFI_BLOCK_IO_MEDIA    *Media;
  UINTN                 BlockSize;            
  UINTN                 NumberOfBlocks ;
  BOOLEAN               MediaChange;
  
  Status = EFI_SUCCESS;
  MediaChange = FALSE;
  
  UsbFloppyDevice = USB_FLOPPY_DEV_FROM_THIS(This);
  
  //
  // Check parameters
  //
  
  Media     = UsbFloppyDevice->BlkIo.Media ;
  BlockSize = Media->BlockSize ;
  NumberOfBlocks = BufferSize / BlockSize ;

  //
  //Check buffer alignment
  //
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }
  
  if ( !(Media -> MediaPresent)) {
    Status = EFI_NO_MEDIA;
  }    
  
  if (MediaId != Media->MediaId) {
    Status = EFI_MEDIA_CHANGED;
  }
  
  if (LBA > Media->LastBlock) {
    Status = EFI_INVALID_PARAMETER;
  }
  
  if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {
    Status = EFI_INVALID_PARAMETER;
  }   
  
  if (BufferSize % BlockSize != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
  }
    
  if (Buffer == NULL) {    
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0 ) {    
    return EFI_SUCCESS;
  }  
  
  if (!EFI_ERROR(Status)) {
  
    Status = UsbFloppyTestUnitReady (UsbFloppyDevice);
    if (!EFI_ERROR(Status)) {
      Status = USBFloppyRead10 (UsbFloppyDevice, Buffer, LBA, 1);
    }
  } else {
    //
    // To generate sense data for DetectMedia use.
    //
    UsbFloppyTestUnitReady (UsbFloppyDevice);
  }
    
  if (EFI_ERROR(Status)) {
    //
    // if any error encountered, detect what happened to the media and
    // update the media info accordingly.
    //
    Status = UsbFloppyDetectMedia (UsbFloppyDevice,&MediaChange);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR ;
    }
    if (MediaChange) {
      gBS->ReinstallProtocolInterface (UsbFloppyDevice->Handle,
                                      &gEfiBlockIoProtocolGuid,
                                      &UsbFloppyDevice->BlkIo,
                                      &UsbFloppyDevice->BlkIo
                                      );                                
    }
  
    //
    // Get the intrinsic block size
    //
    Media     = UsbFloppyDevice->BlkIo.Media ;
    BlockSize = Media->BlockSize ;
  
    NumberOfBlocks = BufferSize / BlockSize ;

    //
    //Check buffer alignment
    //
    if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
      return EFI_INVALID_PARAMETER;
    }

    
    if ( !(Media -> MediaPresent)) {
      return EFI_NO_MEDIA;
    }
  
    if (MediaId != Media->MediaId) {
      return EFI_MEDIA_CHANGED;
    }
  
    if (BufferSize % BlockSize != 0) {
      return EFI_BAD_BUFFER_SIZE;
    }
    
    if (LBA > Media->LastBlock) {
      return EFI_INVALID_PARAMETER;
    }
  
    if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {
      return EFI_INVALID_PARAMETER;
    }   
    
    Status = USBFloppyRead10 (UsbFloppyDevice, Buffer, LBA, NumberOfBlocks) ;
    if (EFI_ERROR(Status)) {
      This->Reset (This,TRUE);
      return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
  
  } else {
    
    LBA += 1;
    NumberOfBlocks -= 1;
    Buffer = (UINT8*)Buffer + This->Media->BlockSize;
    
    if (NumberOfBlocks == 0) {
      return EFI_SUCCESS;
    }
    
    Status = USBFloppyRead10 ( UsbFloppyDevice, Buffer, LBA, NumberOfBlocks ) ;
    if (EFI_ERROR(Status)) {
      This->Reset (This,TRUE);
      return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
  } 

}     

STATIC
EFI_STATUS 
EFIAPI 
USBFloppyWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  IN  VOID                    *Buffer
  )
/*++

  Routine Description:
    Implements EFI_BLOCK_IO_PROTOCOL.WriteBlocks() function.
  
  Arguments:
    This:     The EFI_BLOCK_IO_PROTOCOL instance.
    MediaId:  The media id that the write request is for.
    LBA:      The starting logical block address to be written.
              The caller is responsible for writing to only 
              legitimate locations.
    BufferSize:
              The size of the Buffer in bytes. This must be a multiple of 
              the intrinsic block size of the device.
    Buffer:   A pointer to the source buffer for the data. The caller 
              is responsible for either having implicit or explicit ownership
              of the buffer.                               
  
  Returns:  

--*/        
{
  USB_FLOPPY_DEV          *UsbFloppyDevice;
  EFI_STATUS              Status;  
  EFI_BLOCK_IO_MEDIA      *Media;
  UINTN                   BlockSize;            
  UINTN                   NumberOfBlocks ;  
  BOOLEAN                 MediaChange;
  
  Status = EFI_SUCCESS;
  MediaChange = FALSE;
  
  UsbFloppyDevice = USB_FLOPPY_DEV_FROM_THIS(This);
  
  //
  // Check parameters.
  //  
  Media     = UsbFloppyDevice->BlkIo.Media ;
  BlockSize = Media->BlockSize ;
  NumberOfBlocks = BufferSize / BlockSize ;
  
  //
  //Check buffer alignment
  //
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }

  if ( !(Media -> MediaPresent)) {
    Status = EFI_NO_MEDIA;
  }    
  
  if (MediaId != Media->MediaId) {
    Status = EFI_MEDIA_CHANGED;
  }

  if (Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }
  
  if (LBA > Media->LastBlock) {
    Status = EFI_INVALID_PARAMETER;
  }
  
  if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {
    Status = EFI_INVALID_PARAMETER;
  }   
  
  if (BufferSize % BlockSize != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
  }
    
  if (Buffer == NULL) {    
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0 ) {    
    return EFI_SUCCESS;
  }
  
  if (!EFI_ERROR(Status)) {
    Status = UsbFloppyTestUnitReady (UsbFloppyDevice);
    if (!EFI_ERROR(Status)) {
      Status = USBFloppyWrite10 ( UsbFloppyDevice, Buffer, LBA, 1) ;
    }
  } else {
    //
    // To generate sense data for DetectMedia use.
    //
    UsbFloppyTestUnitReady (UsbFloppyDevice);
  }
  
  if (EFI_ERROR(Status)) {
    //
    // if any error encountered, detect what happened to the media and
    // update the media info accordingly.
    //
    Status = UsbFloppyDetectMedia (UsbFloppyDevice,&MediaChange);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR ;
    }
    if (MediaChange) {
      gBS->ReinstallProtocolInterface (UsbFloppyDevice->Handle,
                                      &gEfiBlockIoProtocolGuid,
                                      &UsbFloppyDevice->BlkIo,
                                      &UsbFloppyDevice->BlkIo
                                      );                                
    }
  
    //
    //Get the intrinsic block size
    //
    Media     = UsbFloppyDevice->BlkIo.Media ;
    BlockSize = Media->BlockSize ;
  
    NumberOfBlocks = BufferSize/BlockSize ;
    
    if ( !(Media -> MediaPresent)) {
      return EFI_NO_MEDIA;
    }
  
    if (MediaId != Media->MediaId) {
     return EFI_MEDIA_CHANGED;
    }
  
    if (BufferSize % BlockSize != 0) {
     return EFI_BAD_BUFFER_SIZE;
    }
    
    if (LBA > Media->LastBlock) {
     return EFI_INVALID_PARAMETER;
    }
  
    if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {
      return EFI_INVALID_PARAMETER;
    }
    
    //
    // Write blocks of data to device using command Write10.
    //
    Status = USBFloppyWrite10 ( UsbFloppyDevice, Buffer, LBA, NumberOfBlocks ) ;
    if (EFI_ERROR(Status)) {
      This->Reset (This,TRUE);
      return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
    
  } else {
    LBA += 1;
    NumberOfBlocks -= 1;
    Buffer = (UINT8*)Buffer + This->Media->BlockSize;
    
    if (NumberOfBlocks == 0) {
      return EFI_SUCCESS;
    }
    
    Status = USBFloppyWrite10 ( UsbFloppyDevice, Buffer, LBA, NumberOfBlocks ) ;
    if (EFI_ERROR(Status)) {
      This->Reset (This,TRUE);
      return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
  }  
}   
    
STATIC
EFI_STATUS 
EFIAPI 
USBFloppyFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This
  )
/*++

  Routine Description:
    Implements EFI_BLOCK_IO_PROTOCOL.FlushBlocks() function.
    (In this driver, this function just returns EFI_SUCCESS.)
  
  Arguments:
    This:     The EFI_BLOCK_IO_PROTOCOL instance.
  
  Returns:  

--*/    
{
  return EFI_SUCCESS;
}   
