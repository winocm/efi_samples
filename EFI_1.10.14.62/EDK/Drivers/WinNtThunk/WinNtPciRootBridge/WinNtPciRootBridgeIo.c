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

  WinNtPciRootBridgeIo.c
    
Abstract:

  EFI WinNt Emulated PCI Root Bridge Controller

Revision History

--*/

#include "WinNtPciRootBridge.h"
#include "pci22.h"
#include "acpi.h"
#include "EfiDriverLib.h"

//
// Protocol Member Function Prototypes
//

EFI_STATUS
RootBridgeIoPollMem ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  );
  
EFI_STATUS
RootBridgeIoPollIo ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  );
  
EFI_STATUS
RootBridgeIoMemRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
RootBridgeIoMemWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
RootBridgeIoIoRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

EFI_STATUS
RootBridgeIoIoWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

EFI_STATUS
RootBridgeIoPciRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
RootBridgeIoPciWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
RootBridgeIoCopyMem (
  IN     struct _EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH    Width,
  IN     UINT64                                   DestAddress,
  IN     UINT64                                   SrcAddress,
  IN     UINTN                                    Count
  );

EFI_STATUS
RootBridgeIoMap (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                                       *HostAddress,
  IN OUT UINTN                                      *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                       *DeviceAddress,
  OUT    VOID                                       **Mapping
  );

EFI_STATUS
RootBridgeIoUnmap (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  VOID                             *Mapping
  );

EFI_STATUS
RootBridgeIoAllocateBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE                Type,
  IN  EFI_MEMORY_TYPE                  MemoryType,
  IN  UINTN                            Pages,
  OUT VOID                             **HostAddress,
  IN  UINT64                           Attributes
  );

EFI_STATUS
RootBridgeIoFreeBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  UINTN                            Pages,
  OUT VOID                             *HostAddress
  );

EFI_STATUS
RootBridgeIoFlush (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This
  );

EFI_STATUS
RootBridgeIoGetAttributes (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT UINT64                           *Supported,
  OUT UINT64                           *Attributes
  );

EFI_STATUS
RootBridgeIoSetAttributes (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     UINT64                           Attributes,
  IN OUT UINT64                           *ResourceBase,
  IN OUT UINT64                           *ResourceLength
  ); 

EFI_STATUS
RootBridgeIoConfiguration (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT VOID                             **Resources
  );

//
// Sub Function Prototypes
//

typedef union {
    UINT8       VOLATILE    *buf;
    UINT8       VOLATILE    *ui8;
    UINT16      VOLATILE    *ui16;
    UINT32      VOLATILE    *ui32;
    UINT64      VOLATILE    *ui64;
    UINTN       VOLATILE    ui;
} PTR;


STATIC
EFI_STATUS
RootBridgeIoPciRW (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     BOOLEAN                                Write,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

//
// PCI COnfig Header for a PCI-ISA Bridge at Bus #9, Dev #1f, Func 0
//
UINT8 PciToIsaBridge[] = {
    0x86, 0x80, 0x40, 0x24, 0x0f, 0x01, 0x80, 0x02,
    0x01, 0x00, 0x01, 0x06, 0x00, 0x00, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

typedef struct {
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  Bus;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  Io;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  Memory;
  EFI_ACPI_END_TAG_DESCRIPTOR        End;
} WIN_NT_PCI_ROOT_BRIDGE_CONFIGURATION;

WIN_NT_PCI_ROOT_BRIDGE_CONFIGURATION WinNtPciRootBridgeConfiguration = {
  { ACPI_ADDRESS_SPACE_DESCRIPTOR,                // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR),   // Len
    ACPI_ADDRESS_SPACE_TYPE_BUS,                  // ResType
    0,                                            // GenFlag
    0,                                            // SpecificFlag    
    0,                                            // AddrSpaceGranularity
    0,                                            // AddrRangeMin
    0,                                            // AddrRangeMax
    0,                                            // AddrTranslationOffset
    1                                             // AddrLen
  },
  { ACPI_ADDRESS_SPACE_DESCRIPTOR,                // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR),   // Len
    ACPI_ADDRESS_SPACE_TYPE_IO,                   // ResType
    0,                                            // GenFlag
    1,                                            // SpecificFlag    
    0,                                            // AddrSpaceGranularity
    0x0000,                                       // AddrRangeMin
    0xffff,                                       // AddrRangeMax
    0,                                            // AddrTranslationOffset
    0x10000                                       // AddrLen
  },
  { ACPI_ADDRESS_SPACE_DESCRIPTOR,                // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR),   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                  // ResType
    0,                                            // GenFlag
    0,                                            // SpecificFlag    
    32,                                           // AddrSpaceGranularity
    0x00000000,                                   // AddrRangeMin
    0xffffffff,                                   // AddrRangeMax
    0,                                            // AddrTranslationOffset
    0x100000000                                   // AddrLen
  },
  { ACPI_END_TAG_DESCRIPTOR,
    0
  }
};

EFI_STATUS
WinNtRootBridgeIoConstructor (
	IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *Protocol
  )
/*++

Routine Description:

    Contruct the Pci Root Bridge Io protocol

Arguments:

    Protocol - protocol to initialize
    
Returns:

    None

--*/
{
  WIN_NT_PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(Protocol);

  //
  // The WinNt host to pci bridge, the host memory and io addresses are
  // direct mapped to pci addresses, so no need translate, set bases to 0.
  //
  PrivateData->MemBase = 0;
  PrivateData->IoBase  = 0;

  //
  // The WinNt host bridge only supports 32bit addressing for memory
  // and standard IA32 16bit io
  //
  PrivateData->MemLimit = 1 << 32;
  PrivateData->IoLimit  = 1 << 16;

  EfiInitializeLock (&PrivateData->PciLock, EFI_TPL_HIGH_LEVEL);
  PrivateData->PciAddress  = 0xCF8;
  PrivateData->PciData     = 0xCFC;

  Protocol->ParentHandle   = NULL;

  Protocol->PollMem        = RootBridgeIoPollMem;
  Protocol->PollIo         = RootBridgeIoPollIo;

  Protocol->Mem.Read       = RootBridgeIoMemRead;
  Protocol->Mem.Write      = RootBridgeIoMemWrite;

  Protocol->Io.Read        = RootBridgeIoIoRead;
  Protocol->Io.Write       = RootBridgeIoIoWrite;

  Protocol->Pci.Read       = RootBridgeIoPciRead;
  Protocol->Pci.Write      = RootBridgeIoPciWrite;

  Protocol->CopyMem        = RootBridgeIoCopyMem;

  Protocol->Map            = RootBridgeIoMap;
  Protocol->Unmap          = RootBridgeIoUnmap;

  Protocol->AllocateBuffer = RootBridgeIoAllocateBuffer;
  Protocol->FreeBuffer     = RootBridgeIoFreeBuffer;

  Protocol->Flush          = RootBridgeIoFlush;

  Protocol->GetAttributes  = RootBridgeIoGetAttributes;
  Protocol->SetAttributes  = RootBridgeIoSetAttributes;

  Protocol->Configuration  = RootBridgeIoConfiguration;
  Protocol->SegmentNumber  = 0;

  return EFI_SUCCESS;
}


EFI_STATUS
RootBridgeIoPollMem ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  )
{
  EFI_STATUS  Status;
  UINT64      NumberOfTicks;
  UINTN       Remainder;

  //
  // No matter what, always do a single poll.
  //
  Status = This->Mem.Read (This, Width, Address, 1, Result);
  if ( EFI_ERROR(Status) ) {
    return Status;
  }    
  if ( (*Result & Mask) == Value ) {
    return EFI_SUCCESS;
  }

  if ( Delay != 0 ) {

    NumberOfTicks = DriverLibDivU64x32 (Delay, 100, &Remainder);
    if ( Remainder !=0 ) {
      NumberOfTicks += 1;
    }
    NumberOfTicks += 1;
  
    while ( NumberOfTicks ) {

      gBS->Stall(10);
    
      Status = This->Mem.Read (This, Width, Address, 1, Result);
      if ( EFI_ERROR(Status) ) {
        return Status;
      }
    
      if ( (*Result & Mask) == Value ) {
        return EFI_SUCCESS;
      }

      NumberOfTicks -= 1;
    }
  }
  return EFI_TIMEOUT;
}
  
EFI_STATUS
RootBridgeIoPollIo ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  )
{

  EFI_STATUS  Status;
  UINT64      NumberOfTicks;
  UINTN       Remainder;

  //
  // No matter what, always do a single poll.
  //
  Status = This->Io.Read (This, Width, Address, 1, Result);
  if ( EFI_ERROR(Status) ) {
    return Status;
  }    
  if ( (*Result & Mask) == Value ) {
    return EFI_SUCCESS;
  }

  if (Delay != 0) {

    NumberOfTicks = DriverLibDivU64x32 (Delay, 100, &Remainder);
    if ( Remainder !=0 ) {
      NumberOfTicks += 1;
    }
    NumberOfTicks += 1;
  
    while ( NumberOfTicks ) {

      gBS->Stall(10);
    
      Status = This->Io.Read (This, Width, Address, 1, Result);
      if ( EFI_ERROR(Status) ) {
        return Status;
      }
    
      if ( (*Result & Mask) == Value ) {
        return EFI_SUCCESS;
      }

      NumberOfTicks -= 1;
    }
  }
  return EFI_TIMEOUT;
}

EFI_STATUS
RootBridgeIoMemRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  EfiSetMem (Buffer, Count * (1<<Width), 0);
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoMemWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoIoRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  )
{
  EfiSetMem (UserBuffer, Count * (1 << Width), 0);
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoIoWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoPciRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  return RootBridgeIoPciRW (This, FALSE, Width, Address, Count, Buffer);
}

EFI_STATUS
RootBridgeIoPciWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  return RootBridgeIoPciRW (This, TRUE, Width, Address, Count, Buffer);
}

EFI_STATUS
RootBridgeIoCopyMem (
  IN     struct _EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH    Width,
  IN     UINT64                                   DestAddress,
  IN     UINT64                                   SrcAddress,
  IN     UINTN                                    Count
  )

{
  EFI_STATUS  Status;
  BOOLEAN     Direction;
  UINTN       Stride;
  UINTN       Index;
  UINT64      Result;

  if (Width > EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }    

  if (DestAddress == SrcAddress) {
    return EFI_SUCCESS;
  }

  Stride = 1 << Width;

  Direction = TRUE;
  if ((DestAddress > SrcAddress) && (DestAddress < (SrcAddress + Count * Stride))) {
    Direction   = FALSE;
    SrcAddress  = SrcAddress  + (Count-1) * Stride;
    DestAddress = DestAddress + (Count-1) * Stride;
  }

  for (Index = 0;Index < Count;Index++) {
    Status = RootBridgeIoMemRead (
               This,
               Width,
               SrcAddress,
               1,
               &Result
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = RootBridgeIoMemWrite (
               This,
               Width,
               DestAddress,
               1,
               &Result
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (Direction) {
      SrcAddress  += Stride;
      DestAddress += Stride;
    } else {
      SrcAddress  -= Stride;
      DestAddress -= Stride;
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoMap (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                                       *HostAddress,
  IN OUT UINTN                                      *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                       *DeviceAddress,
  OUT    VOID                                       **Mapping
  )

{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;
  MAP_INFO              *MapInfo;

  //
  // Initialize the return values to their defaults
  //
  *Mapping = NULL;

  //
  // Make sure that Operation is valid
  //
  if (Operation < 0 || Operation > EfiPciOperationBusMasterCommonBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Most PCAT like chipsets can not handle performing DMA above 4GB.
  // If any part of the DMA transfer being mapped is above 4GB, then
  // map the DMA transfer to a buffer below 4GB.
  //
  PhysicalAddress = (EFI_PHYSICAL_ADDRESS)HostAddress;
  if ((PhysicalAddress + *NumberOfBytes) > 0x100000000) {

    //
    // Common Buffer operations can not be remapped.  If the common buffer
    // if above 4GB, then it is not possible to generate a mapping, so return 
    // an error.
    //
    if (Operation == EfiPciOperationBusMasterCommonBuffer) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Allocate a MAP_INFO structure to remember the mapping when Unmap() is
    // called later.
    //
    Status = gBS->AllocatePool (
                    EfiBootServicesData, 
                    sizeof(MAP_INFO), 
                    &MapInfo
                    );
    if (EFI_ERROR (Status)) {
      *NumberOfBytes = 0;
      return Status;
    }

    //
    // Return a pointer to the MAP_INFO structure in Mapping
    //
    *Mapping = MapInfo;

    //
    // Initialize the MAP_INFO structure
    //
    MapInfo->Operation         = Operation;
    MapInfo->NumberOfBytes     = *NumberOfBytes;
    MapInfo->NumberOfPages     = EFI_SIZE_TO_PAGES(*NumberOfBytes);
    MapInfo->HostAddress       = PhysicalAddress;
    MapInfo->MappedHostAddress = 0x00000000ffffffff;

    //
    // Allocate a buffer below 4GB to map the transfer to.
    //
    Status = gBS->AllocatePages (
                    AllocateMaxAddress, 
                    EfiBootServicesData, 
                    MapInfo->NumberOfPages,
                    &MapInfo->MappedHostAddress
                    );
    if (EFI_ERROR(Status)) {
      gBS->FreePool (MapInfo);
      *NumberOfBytes = 0;
      return Status;
    }

    //
    // If this is a read operation from the Bus Master's point of view,
    // then copy the contents of the real buffer into the mapped buffer
    // so the Bus Master can read the contents of the real buffer.
    //
    if (Operation == EfiPciOperationBusMasterRead) {
      EfiCopyMem (
        (VOID *)(UINTN)MapInfo->MappedHostAddress, 
        (VOID *)(UINTN)MapInfo->HostAddress,
        MapInfo->NumberOfBytes
        );
    }

    //
    // The DeviceAddress is the address of the maped buffer below 4GB
    //
    *DeviceAddress = MapInfo->MappedHostAddress;
  } else {
    //
    // The transfer is below 4GB, so the DeviceAddress is simply the HostAddress
    //
    *DeviceAddress = PhysicalAddress;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoUnmap (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  VOID                             *Mapping
  )

{
  MAP_INFO    *MapInfo;

  //
  // See if the Map() operation associated with this Unmap() required a mapping buffer.
  // If a mapping buffer was not required, then this function simply returns EFI_SUCCESS.
  //
  if (Mapping != NULL) {
    //
    // Get the MAP_INFO structure from Mapping
    //
    MapInfo = (MAP_INFO *)Mapping;

    //
    // If this is a write operation from the Bus Master's point of view,
    // then copy the contents of the mapped buffer into the real buffer
    // so the processor can read the contents of the real buffer.
    //
    if (MapInfo->Operation == EfiPciOperationBusMasterWrite) {
      EfiCopyMem (
        (VOID *)(UINTN)MapInfo->HostAddress, 
        (VOID *)(UINTN)MapInfo->MappedHostAddress,
        MapInfo->NumberOfBytes
        );
    }

    //
    // Free the mapped buffer and the MAP_INFO structure.
    //
    gBS->FreePages (MapInfo->MappedHostAddress, MapInfo->NumberOfPages);
    gBS->FreePool (Mapping);
  }
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoAllocateBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE                Type,
  IN  EFI_MEMORY_TYPE                  MemoryType,
  IN  UINTN                            Pages,
  OUT VOID                             **HostAddress,
  IN  UINT64                           Attributes
  )

{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;

  //
  // The only valid memory types are EfiBootServicesData and EfiRuntimeServicesData
  //
  if (MemoryType != EfiBootServicesData && MemoryType != EfiRuntimeServicesData) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Limit allocations to memory below 4GB
  //
  PhysicalAddress = (EFI_PHYSICAL_ADDRESS)*HostAddress;
  if (Type == AllocateAddress) {
    if ((PhysicalAddress + DriverLibLShiftU64(Pages, EFI_PAGE_SHIFT)) > 0x00000000ffffffff) {
      return EFI_UNSUPPORTED;
    }
  }

  if (Type == AllocateAnyPages)  {
    Type = AllocateMaxAddress;
    PhysicalAddress = 0x00000000ffffffff;
  }

  if (Type == AllocateMaxAddress) {
    if (PhysicalAddress > 0x00000000ffffffff) {
      PhysicalAddress = 0x00000000ffffffff;
    }
  }

  Status = gBS->AllocatePages (Type, MemoryType, Pages, &PhysicalAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *HostAddress = (VOID *)(UINTN)PhysicalAddress;
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoFreeBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  UINTN                            Pages,
  OUT VOID                             *HostAddress
  )

{
  return gBS->FreePages ((EFI_PHYSICAL_ADDRESS)HostAddress, Pages);
}

EFI_STATUS
RootBridgeIoFlush (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This
  )

{
  return EFI_SUCCESS;
}

EFI_STATUS
RootBridgeIoGetAttributes (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT UINT64                           *Supported,
  OUT UINT64                           *Attributes
  )

{
  *Supported  = 0;
  *Attributes = 0;
  return EFI_UNSUPPORTED;
}

EFI_STATUS
RootBridgeIoSetAttributes (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     UINT64                           Attributes,
  IN OUT UINT64                           *ResourceBase,
  IN OUT UINT64                           *ResourceLength
  )

{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
RootBridgeIoConfiguration (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT VOID                             **Resources
  )

{
  *Resources = &WinNtPciRootBridgeConfiguration;
  return EFI_SUCCESS;
}

//
// Internal function
//

STATIC
EFI_STATUS
RootBridgeIoPciRW (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     BOOLEAN                                Write,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
    )
{
  PCI_CONFIG_ACCESS_CF8             Pci;

  if (Write) {
    return EFI_SUCCESS;
  }

  EfiSetMem (UserBuffer, Count * (1 << Width), 0xff);

  Pci.Reg      = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS*) &UserAddress)->Register;
  Pci.Func     = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS*) &UserAddress)->Function;
  Pci.Dev      = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS*) &UserAddress)->Device;
  Pci.Bus      = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS*) &UserAddress)->Bus;
  Pci.Reserved = 0;
  Pci.Enable   = 1;

  if (Pci.Bus == 0 && Pci.Dev == 0x1f && Pci.Func == 0) {
    EfiCopyMem(UserBuffer, &(PciToIsaBridge[Pci.Reg]), Count*(1<<Width));
  } 
  return EFI_SUCCESS;
}
