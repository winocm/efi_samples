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

  pci.h

Abstract:

  Header file for EFI shell command pci. Defined some useful data structures 
  such as header format for pci devices, p2p bridges, and cardbuses.

Revision History

--*/

#ifndef _PCI_H_
#define _PCI_H_

#include "Efi.h"

typedef enum {
  PciDevice,
  PciP2pBridge,
  PciCardBusBridge,
  PciUndefined
} PCI_HEADER_TYPE;

#define    HEADER_TYPE_MULTI_FUNCTION 0x80

#define CALC_EFI_PCI_ADDRESS(Bus,Dev,Func,Reg) \
  ( (UINT64) ( (((UINTN)Bus) << 24) + (((UINTN)Dev) << 16) + \
  (((UINTN)Func) << 8) + ((UINTN)Reg) ))

#define INDEX_OF(Field) \
  (  (UINT8 *)(Field) - (UINT8 *)mConfigSpace  )

#define    PCI_BIT_0                  0x00000001
#define    PCI_BIT_1                  0x00000002
#define    PCI_BIT_2                  0x00000004
#define    PCI_BIT_3                  0x00000008
#define    PCI_BIT_4                  0x00000010
#define    PCI_BIT_5                  0x00000020
#define    PCI_BIT_6                  0x00000040
#define    PCI_BIT_7                  0x00000080
#define    PCI_BIT_8                  0x00000100
#define    PCI_BIT_9                  0x00000200
#define    PCI_BIT_10                 0x00000400
#define    PCI_BIT_11                 0x00000800
#define    PCI_BIT_12                 0x00001000
#define    PCI_BIT_13                 0x00002000
#define    PCI_BIT_14                 0x00004000
#define    PCI_BIT_15                 0x00008000


#pragma pack(1)

//
// Common part of the PCI configuration space header for devices, P2P bridges, 
// and cardbus bridges
//
typedef struct {
  UINT16        VendorId;
  UINT16        DeviceId;
  
  UINT16        Command;
  UINT16        Status;
  
  UINT8         RevisionId;
  UINT8         ClassCode[3];
  
  UINT8         CacheLineSize;
  UINT8         PrimaryLatencyTimer;
  UINT8         HeaderType;
  UINT8         BIST;
  
} PCI_COMMON_HEADER;

//
// PCI configuration space header for devices(after the common part)
//  
typedef struct {
  UINT32        Bar[6];                   // Base Address Registers
  UINT32        CardBusCISPtr;            // CardBus CIS Pointer
  
  UINT16        SubVendorId;              // Subsystem Vendor ID
  UINT16        SubSystemId;              // Subsystem ID
  
  UINT32        ROMBar;                   // Expansion ROM Base Address
  
  UINT8         CapabilitiesPtr;          // Capabilities Pointer
  UINT8         Reserved[3];
  
  UINT32        Reserved1;
  
  UINT8         InterruptLine;            // Interrupt Line
  UINT8         InterruptPin;             // Interrupt Pin
  UINT8         MinGnt;                   // Min_Gnt
  UINT8         MaxLat;                   // Max_Lat
  
} PCI_DEVICE_HEADER;

//
// PCI configuration space header for pci-to-pci bridges(after the common part)
//  
typedef struct {
  UINT32        Bar[2];                   // Base Address Registers
  
  UINT8         PrimaryBus;               // Primary Bus Number
  UINT8         SecondaryBus;             // Secondary Bus Number
  UINT8         SubordinateBus;           // Subordinate Bus Number
  UINT8         SecondaryLatencyTimer;    // Secondary Latency Timer
  
  UINT8         IoBase;                   // I/O Base
  UINT8         IoLimit;                  // I/O Limit
  UINT16        SecondaryStatus;          // Secondary Status
  
  UINT16        MemoryBase;               // Memory Base
  UINT16        MemoryLimit;              // Memory Limit
  
  UINT16        PrefetchableMemBase;      // Pre-fetchable Memory Base
  UINT16        PrefetchableMemLimit;     // Pre-fetchable Memory Limit 
  
  UINT32        PrefetchableBaseUpper;    // Pre-fetchable Base Upper 32 bits
  
  UINT32        PrefetchableLimitUpper;   // Pre-fetchable Limit Upper 32 bits
  
  UINT16        IoBaseUpper;              // I/O Base Upper 16 bits
  UINT16        IoLimitUpper;             // I/O Limit Upper 16 bits
  
  UINT8         CapabilitiesPtr;          // Capabilities Pointer
  UINT8         Reserved[3];            
  
  UINT32        ROMBar;                   // Expansion ROM Base Address
  
  UINT8         InterruptLine;            // Interrupt Line
  UINT8         InterruptPin;             // Interrupt Pin
  UINT16        BridgeControl;            // Bridge Control
  
} PCI_BRIDGE_HEADER;
  
//
// PCI configuration space header for cardbus bridges(after the common part)
//  
typedef struct {
  UINT32        CardBusSocketReg;          // Cardus Socket/ExCA Base 
                                           //Address Register
  UINT16        Reserved;  
  UINT16        SecondaryStatus;           // Secondary Status
  
  UINT8         PciBusNumber;              // PCI Bus Number
  UINT8         CardBusBusNumber;          // CardBus Bus Number
  UINT8         SubordinateBusNumber;      // Subordinate Bus Number
  UINT8         CardBusLatencyTimer;       // CardBus Latency Timer
  
  UINT32        MemoryBase0;               // Memory Base Register 0
  UINT32        MemoryLimit0;              // Memory Limit Register 0
  UINT32        MemoryBase1;              
  UINT32        MemoryLimit1;              
  UINT32        IoBase0;                   
  UINT32        IoLimit0;                  // I/O Base Register 0
  UINT32        IoBase1;                   // I/O Limit Register 0
  UINT32        IoLimit1;
  
  UINT8         InterruptLine;             // Interrupt Line
  UINT8         InterruptPin;              // Interrupt Pin
  UINT16        BridgeControl;             // Bridge Control
} PCI_CARDBUS_HEADER;

//
// Data region after PCI configuration header(for cardbus bridge)
//  
typedef struct {
  UINT16        SubVendorId;               // Subsystem Vendor ID
  UINT16        SubSystemId;               // Subsystem ID
  UINT32        LegacyBase;                // Optional 16-Bit PC Card Legacy
                                           // Mode Base Address
  UINT32        Data[46];
} PCI_CARDBUS_DATA;

typedef struct {
  PCI_COMMON_HEADER     Common;
  union {
    PCI_DEVICE_HEADER   Device;
    PCI_BRIDGE_HEADER   Bridge;
    PCI_CARDBUS_HEADER  CardBus;
  } NonCommon;
  UINT32                Data[48];
} PCI_CONFIG_SPACE;
  
#pragma pack()

#endif //_PCI_H_
 
  
