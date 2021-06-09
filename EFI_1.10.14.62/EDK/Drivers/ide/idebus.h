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

    idebus.h

Abstract:

    Header file for IDE Bus Driver.

Revision History
++*/

#ifndef _IDE_BUS_H
#define _IDE_BUS_H

#include "Efi.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"
#include "pci22.h"
#include "idedata.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (BlockIo)

#define MAX_IDE_DEVICE 4

typedef struct {
  BOOLEAN               HaveScannedDevice[MAX_IDE_DEVICE];  
} IDE_BUS_DRIVER_PRIVATE_DATA;

#define IDE_BLK_IO_DEV_SIGNATURE   EFI_SIGNATURE_32('i','b','i','d')

typedef struct {
    UINT32                        Signature;
    
    EFI_HANDLE                    Handle;
    EFI_BLOCK_IO_PROTOCOL         BlkIo;
    EFI_BLOCK_IO_MEDIA            BlkMedia;
    EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
    EFI_PCI_IO_PROTOCOL           *PciIo;
    IDE_BUS_DRIVER_PRIVATE_DATA   *IdeBusDriverPrivateData;

    //
    // Local Data for IDE interface goes here
    //
    EFI_IDE_CHANNEL               Channel;
    EFI_IDE_DEVICE                Device;
    UINT16                        Lun;
    IDE_DEVICE_TYPE               Type;
    
    IDE_BASE_REGISTERS            *IoPort;
    UINT16                        AtapiError;

    INQUIRY_DATA                  *pInquiryData; 
    IDENTIFY                      *pIdData;
    ATA_PIO_MODE                  PioMode;
    CHAR8                         ModelName[41];
    REQUEST_SENSE_DATA            *SenseData;
    UINT8                         SenseDataNumber;
    UINT8                         *Cache;

    EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
} IDE_BLK_IO_DEV;

#define IDE_BLOCK_IO_DEV_FROM_THIS(a) CR(a, IDE_BLK_IO_DEV, BlkIo, IDE_BLK_IO_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_GUID                          gIDEBusDriverGuid;
extern EFI_DRIVER_BINDING_PROTOCOL       gIDEBusDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL       gIDEBusComponentName;
extern EFI_DRIVER_CONFIGURATION_PROTOCOL gIDEBusDriverConfiguration;
extern EFI_DRIVER_DIAGNOSTICS_PROTOCOL   gIDEBusDriverDiagnostics;

#include "ide.h"

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
IDEBusControllerDriverEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
IDEBusDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
IDEBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
IDEBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );

  
//
// Block I/O Protocol Interface
//
EFI_STATUS
IDEBlkIoReset(
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  BOOLEAN                 ExtendedVerification
  );
  
EFI_STATUS
IDEBlkIoReadBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  OUT VOID                    *Buffer
  );
  
EFI_STATUS
IDEBlkIoWriteBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  IN  VOID                    *Buffer
  );
  
EFI_STATUS
IDEBlkIoFlushBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL   *This
  );
  
EFI_STATUS  
IDERegisterDecodeEnableorDisable(
  IN  EFI_PCI_IO_PROTOCOL       *PciIo,
  IN  BOOLEAN                   Enable
  );  

#endif
