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

  CirrusLogic5430.h
    
Abstract:

  Cirrus Logic 5430 Controller Driver

Revision History

--*/

//
// Cirrus Logic 5430 Controller Driver
//

#ifndef _CIRRUS_LOGIC_5430_H_
#define _CIRRUS_LOGIC_5430_H_

#include "Efi.h"
#include "Pci22.h"
#include "EfiDriverLib.h"
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (UgaDraw)

//
// Cirrus Logic 5430 PCI Configuration Header values
//
#define CIRRUS_LOGIC_VENDOR_ID                0x1013
#define CIRRUS_LOGIC_5430_DEVICE_ID           0x00a8
#define CIRRUS_LOGIC_5430_ALTERNATE_DEVICE_ID 0x00a0
#define CIRRUS_LOGIC_5446_DEVICE_ID           0x00b8

//
// Cirrus Logic Graphical Mode Data
//
#define CIRRUS_LOGIC_5430_UGA_DRAW_MODE_COUNT  3  

typedef struct {
  UINT32  HorizontalResolution;
  UINT32  VerticalResolution;
  UINT32  ColorDepth;
  UINT32  RefreshRate;
} CIRRUS_LOGIC_5430_UGA_DRAW_MODE_DATA;

//
// Cirrus Logic 5440 Private Data Structure
//
#define CIRRUS_LOGIC_5430_PRIVATE_DATA_SIGNATURE EFI_SIGNATURE_32('C','L','5','4')

typedef struct {
  UINT64                              Signature;
  EFI_HANDLE                          Handle;
  EFI_PCI_IO_PROTOCOL                 *PciIo;
  EFI_UGA_DRAW_PROTOCOL               UgaDraw;

  //
  // UGA Draw Private Data
  //
  BOOLEAN                               HardwareNeedsStarting;
  UINTN                                 CurrentMode;
  UINTN                                 MaxMode;
  CIRRUS_LOGIC_5430_UGA_DRAW_MODE_DATA  ModeData[CIRRUS_LOGIC_5430_UGA_DRAW_MODE_COUNT];
  UINT8                                 *LineBuffer;
} CIRRUS_LOGIC_5430_PRIVATE_DATA;

#define CIRRUS_LOGIC_5430_PRIVATE_DATA_FROM_UGA_DRAW_THIS(a)  \
  CR(a, CIRRUS_LOGIC_5430_PRIVATE_DATA, UgaDraw, CIRRUS_LOGIC_5430_PRIVATE_DATA_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gCirrusLogic5430DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gCirrusLogic5430ComponentName;

//
// Io Registers defined by VGA
//
#define CRTC_ADDRESS_REGISTER    0x3d4
#define CRTC_DATA_REGISTER       0x3d5
#define SEQ_ADDRESS_REGISTER     0x3c4
#define SEQ_DATA_REGISTER        0x3c5
#define GRAPH_ADDRESS_REGISTER   0x3ce
#define GRAPH_DATA_REGISTER      0x3cf
#define ATT_ADDRESS_REGISTER     0x3c0
#define MISC_OUTPUT_REGISTER     0x3c2
#define INPUT_STATUS_1_REGISTER  0x3da
#define DAC_PIXEL_MASK_REGISTER  0x3c6
#define PALETTE_INDEX_REGISTER   0x3c8
#define PALETTE_DATA_REGISTER    0x3c9

//
// UGA Draw Hardware abstraction internal worker functions
//
EFI_STATUS
CirrusLogic5430UgaDrawConstructor (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  );

EFI_STATUS
CirrusLogic5430UgaDrawDestructor (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  );

//
// EFI 1.1 driver model prototypes for Cirrus Logic 5430 UGA Draw
//
EFI_STATUS
CirrusLogic5430DriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

//
// EFI_DRIVER_BINDING_PROTOCOL Protocol Interface
//
EFI_STATUS
CirrusLogic5430ControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
CirrusLogic5430ControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
CirrusLogic5430ControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  );

#endif

