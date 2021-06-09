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

  PciBus.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#ifndef _EFI_PCI_BUS_H
#define _EFI_PCI_BUS_H

#include "efi.h"
#include "EfiDriverLib.h"
#include "EfiImage.h"
#include "pci22.h"
#include "acpi.h"
#include "linkedlist.h"


//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)
#include EFI_PROTOCOL_DEFINITION (UgaIo)

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (BusSpecificDriverOverride)



//
// Driver Produced Protocol Prototypes
//


#define VGABASE1               0x3B0
#define VGALIMIT1              0x3BB

#define VGABASE2               0x3C0
#define VGALIMIT2              0x3DF

#define ISABASE                0x100
#define ISALIMIT               0x3FF

typedef enum {
  PciBarTypeUnknown = 0,
  PciBarTypeIo16,
  PciBarTypeIo32,
  PciBarTypeMem32,
  PciBarTypePMem32,
  PciBarTypeMem64,
  PciBarTypePMem64,
  PciBarTypeIo,
  PciBarTypeMem,
  PciBarTypeMaxType
} PCI_BAR_TYPE;

typedef struct {
  UINT64        BaseAddress;
  UINT64        Length;
  UINT64        Alignment;
  PCI_BAR_TYPE  BarType;
  BOOLEAN       Prefetchable;
  UINT8         MemType;
  UINT8         Offset;
} PCI_BAR;

#define PCI_IO_DEVICE_SIGNATURE   EFI_SIGNATURE_32 ('p','c','i','o')

#define EFI_BRIDGE_IO32_DECODE_SUPPORTED        0x0001 
#define EFI_BRIDGE_PMEM32_DECODE_SUPPORTED      0x0002 
#define EFI_BRIDGE_PMEM64_DECODE_SUPPORTED      0x0004 
#define EFI_BRIDGE_IO16_DECODE_SUPPORTED        0x0008  
#define EFI_BRIDGE_PMEM_MEM_COMBINE_SUPPORTED   0x0010  
#define EFI_BRIDGE_MEM64_DECODE_SUPPORTED       0x0020
#define EFI_BRIDGE_MEM32_DECODE_SUPPORTED       0x0040


typedef struct _PCI_IO_DEVICE {
  UINT32                                      Signature;
  EFI_HANDLE                                  Handle;
  EFI_PCI_IO_PROTOCOL                         PciIo;
  EFI_LIST_ENTRY                              Link;
 
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL   PciDriverOverride;
  EFI_DEVICE_PATH_PROTOCOL                    *DevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL             *PciRootBridgeIo;

  //
  // PCI configuration space header type               
  //
  PCI_TYPE00                                  Pci;

  //
  // Bus number, Device number, Function number
  //
  UINT8                                       BusNumber;
  UINT8                                       DeviceNumber;
  UINT8                                       FunctionNumber;

  //
  // BAR for this PCI Device
  //
  PCI_BAR                                     PciBar[PCI_MAX_BAR];

  //
  // The bridge device this pci device is subject to
  //
  struct _PCI_IO_DEVICE                       *Parent;

  //
  // A linked list for children Pci Device if it is bridge device
  //
  EFI_LIST_ENTRY                              ChildList;


  //
  // TURE if the PCI bus driver creates the handle for this PCI device
  //
  BOOLEAN                                     Registered;

  //
  // TRUE if the PCI bus driver successfully allocates the resource required by
  // this PCI device
  //
  BOOLEAN                                     Allocated;

  //
  // The attribute this PCI device currently set
  //
  UINT64                                      Attributes;


  //
  // The attributes this PCI device actually supports
  //
  UINT64                                      Supports;

  //
  // The resource decode the bridge supports
  //
  UINT32                                      Decodes;

  //
  // The OptionRom Size
  //
  UINT64                                      RomSize;

  //
  // TRUE if there is any EFI driver in the OptionRom
  //
  BOOLEAN                                     BusOverride;

  //
  //  A list tracking reserved resource on a bridge device
  //
  EFI_LIST_ENTRY                              ReservedResourceList;
  
  //
  // A list tracking image handle of platform specific overriding driver
  //
  EFI_LIST_ENTRY                              OptionRomDriverList;
} PCI_IO_DEVICE;


#define PCI_IO_DEVICE_FROM_PCI_IO_THIS(a) \
  CR (a, PCI_IO_DEVICE, PciIo, PCI_IO_DEVICE_SIGNATURE)

#define PCI_IO_DEVICE_FROM_PCI_DRIVER_OVERRIDE_THIS(a) \
  CR (a, PCI_IO_DEVICE, PciDriverOverride, PCI_IO_DEVICE_SIGNATURE)

#define PCI_IO_DEVICE_FROM_LINK(a) \
  CR (a, PCI_IO_DEVICE, Link, PCI_IO_DEVICE_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gPciBusDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPciBusComponentName;
extern BOOLEAN                     gFullEnumeration;
static UINT64                      gAllOne = 0xFFFFFFFFFFFFFFFF;
static UINT64                      gAllZero   = 0;

#include "PciIo.h"
#include "PciCommand.h"
#include "PciDeviceSupport.h"
#include "PciEnumerator.h"
#include "PciEnumeratorSupport.h"
#include "PciDriverOverride.h"
#include "PciRomTable.h"
#include "PciOptionRomSupport.h"
#include "PciPowerManagement.h"


#endif
