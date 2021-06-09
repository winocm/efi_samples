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

  IsaBus.h
  
Abstract:
  
  The header file for ISA bus driver
  
Revision History:

--*/

#ifndef _EFI_ISA_BUS_H
#define _EFI_ISA_BUS_H

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (IsaAcpi)
#include EFI_PROTOCOL_DEFINITION (PciIo)

//
// Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (IsaIo)

typedef enum {
  IsaAccessTypeUnknown,
  IsaAccessTypeIo,
  IsaAccessTypeMem,
  IsaAccessTypeMaxType
} ISA_ACCESS_TYPE;

#define ISA_MAX_MEMORY_ADDRESS  0x1000000  // 16 MB Memory Range
#define ISA_MAX_IO_ADDRESS      0x10000    // 64K I/O Range

//
// ISA DMA Write Single Mask Register
//
#define DMA_SINGLE_MASK_0_3        0x0A 
#define DMA_SINGLE_MASK_4_7        0xD4 
  #define DMA_CHANNEL_MASK_SELECT  0x04

//
// ISA DMA Clear Byte Pointer Register
//
#define DMA_CLEAR_0_3              0x0C 
#define DMA_CLEAR_4_7              0xD8 

//
// ISA DMA Channel Mode Register
//
#define DMA_MODE_0_3                0x0B 
#define DMA_MODE_4_7                0xD6 
  #define DMA_MODE_READ             0x08
  #define DMA_MODE_WRITE            0x04
  #define DMA_MODE_AUTO_INITIALIZE  0x10
  #define DMA_MODE_INCREMENT        0x00
  #define DMA_MODE_DECREMENT        0x20
  #define DMA_MODE_SINGLE           0x40
  #define DMA_MODE_DEMAND           0x00


typedef struct {
  UINT8                     Address;
  UINT8                     Page;
  UINT8                     Count;
} EFI_ISA_DMA_REGISTERS;


//
// ISA I/O Device Structure
//
#define ISA_IO_DEVICE_SIGNATURE   EFI_SIGNATURE_32('i','s','a','i')

typedef struct {
  UINT32                    Signature;
  EFI_HANDLE                Handle;
  EFI_ISA_IO_PROTOCOL       IsaIo;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_PCI_IO_PROTOCOL       *PciIo;
} ISA_IO_DEVICE;

#define ISA_IO_DEVICE_FROM_ISA_IO_THIS(a) \
  CR(a, ISA_IO_DEVICE, IsaIo, ISA_IO_DEVICE_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gIsaBusControllerDriver;
extern EFI_COMPONENT_NAME_PROTOCOL gIsaBusComponentName;


//
// Mapping structure for performing ISA DMA to a buffer above 16 MB
//
typedef struct {
  EFI_ISA_IO_PROTOCOL_OPERATION  Operation;
  UINTN                          NumberOfBytes;
  UINTN                          NumberOfPages;
  EFI_PHYSICAL_ADDRESS           HostAddress;
  EFI_PHYSICAL_ADDRESS           MappedHostAddress;
} ISA_MAP_INFO;

//
// EFI Driver Binding Protocol Interface Functions
//
EFI_STATUS
IsaBusControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
IsaBusControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
IsaBusControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

//
// Function Prototypes
//
EFI_STATUS
IsaCreateDevice (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  IN EFI_DEVICE_PATH_PROTOCOL     *ParentDevicePath,
  IN EFI_ISA_ACPI_RESOURCE_LIST   *IsaDeviceResourceList
  );

EFI_STATUS
InitializeIsaIoInstance (
  ISA_IO_DEVICE                  *IsaIoDevice,
  IN EFI_ISA_ACPI_RESOURCE_LIST  *IsaDevice
  );

#endif
