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

  IsaIo.c
  
Abstract:

  The implementation for EFI_ISA_IO_PROTOCOL.

Revision History:

--*/
#include "IsaBus.h"

//
// ISA I/O Support Function Prototypes
//
EFI_STATUS
IsaIoVerifyAccess (
  IN     ISA_IO_DEVICE              *IsaIoDevice,
  IN     ISA_ACCESS_TYPE            Type,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINTN                      Count,
  IN OUT UINT32                     *Offset
  );

EFI_STATUS
IsaIoMemRead (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  );

EFI_STATUS
IsaIoMemWrite (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  );

EFI_STATUS
IsaIoIoRead (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  );

EFI_STATUS
IsaIoIoWrite (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  );

EFI_STATUS
IsaIoCopyMem (
  IN EFI_ISA_IO_PROTOCOL        *This,
  IN EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN UINT32                     DestOffset,
  IN UINT32                     SrcOffset,
  IN UINTN                      Count
  );

EFI_STATUS
IsaIoMap (
  IN     EFI_ISA_IO_PROTOCOL            *This,
  IN     EFI_ISA_IO_PROTOCOL_OPERATION  Operation,
  IN     UINT8                          ChannelNumber      OPTIONAL,
  IN     UINT32                         ChannelAttributes,
  IN     VOID                           *HostAddress,
  IN OUT UINTN                          *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
  OUT    VOID                           **Mapping
  );

EFI_STATUS
IsaIoUnmap (
  IN EFI_ISA_IO_PROTOCOL  *This,
  IN VOID                 *Mapping
  );

EFI_STATUS
IsaIoAllocateBuffer (
  IN  EFI_ISA_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE    Type,
  IN  EFI_MEMORY_TYPE      MemoryType,
  IN  UINTN                Pages,
  OUT VOID                 **HostAddress,
  IN  UINT64               Attributes
  );

EFI_STATUS
IsaIoFreeBuffer (
  IN EFI_ISA_IO_PROTOCOL  *This,
  IN UINTN                Pages,
  IN VOID                 *HostAddress
  );

EFI_STATUS
IsaIoFlush (
  IN EFI_ISA_IO_PROTOCOL  *This
  );

//
// Driver Support Global Variables
//

static EFI_ISA_IO_PROTOCOL IsaIoInterface = {
  { IsaIoMemRead,    
    IsaIoMemWrite } ,
  { IsaIoIoRead,     
    IsaIoIoWrite  } ,
  IsaIoCopyMem,
  IsaIoMap,
  IsaIoUnmap,
  IsaIoAllocateBuffer,
  IsaIoFreeBuffer,
  IsaIoFlush,
  NULL,
  0,
  NULL
};

static EFI_ISA_DMA_REGISTERS   DmaRegisters[8] = {
  {0x00, 0x87, 0x01},
  {0x02, 0x83, 0x03},
  {0x04, 0x81, 0x05},
  {0x06, 0x82, 0x07},
  {0x00, 0x00, 0x00},  //Channel 4 is invalid
  {0xC4, 0x8B, 0xC6},
  {0xC8, 0x89, 0xCA},
  {0xCC, 0x8A, 0xCE},
};


//
// Driver Support Functions
//
EFI_STATUS
InitializeIsaIoInstance (
  IN ISA_IO_DEVICE               *IsaIoDevice,
  IN EFI_ISA_ACPI_RESOURCE_LIST  *IsaDeviceResourceList
  )
/*++

Routine Description:

  Initializes an ISA I/O Instance

Arguments:
  
Returns:

  None

--*/
{
  //
  // Initializes an ISA I/O Instance
  //
  EfiCopyMem (
     &IsaIoDevice->IsaIo ,
     &IsaIoInterface, 
     sizeof(EFI_ISA_IO_PROTOCOL)
     );
     
  IsaIoDevice->IsaIo.ResourceList = IsaDeviceResourceList;
  return EFI_SUCCESS;
}

EFI_STATUS
IsaIoVerifyAccess (
  IN     ISA_IO_DEVICE              *IsaIoDevice,
  IN     ISA_ACCESS_TYPE            Type,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINTN                      Count,
  IN OUT UINT32                     *Offset
  )
/*++

Routine Description:

  Verifies access to an ISA device

Arguments:

Returns:

  None

--*/

{
  EFI_ISA_ACPI_RESOURCE        *Item;
  EFI_STATUS                   Status;

  if (Width <  EfiIsaIoWidthUint8        ||
      Width >= EfiIsaIoWidthMaximum      ||
      Width == EfiIsaIoWidthReserved     || 
      Width == EfiIsaIoWidthFifoReserved ||
      Width == EfiIsaIoWidthFillReserved)   {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((Type != IsaAccessTypeMem) && (Type != IsaAccessTypeIo)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If Width is EfiIsaIoWidthFifoUintX then convert to EfiIsaIoWidthUintX
  // If Width is EfiIsaIoWidthFillUintX then convert to EfiIsaIoWidthUintX
  //
  if (Width >= EfiIsaIoWidthFifoUint8 && Width <= EfiIsaIoWidthFifoReserved) {
    Count = 1;
  }     
  Width &= 0x03;

  Status = EFI_UNSUPPORTED;
  Item = IsaIoDevice->IsaIo.ResourceList->ResourceItem;
  while (Item->Type != EfiIsaAcpiResourceEndOfList) {
    if (Type == IsaAccessTypeMem && Item->Type == EfiIsaAcpiResourceMemory) {
      if (*Offset >= Item->StartRange &&
        (*Offset + Count * (1 << Width)) - 1 <= Item->EndRange) {
        return EFI_SUCCESS;
      }
      if (*Offset >= Item->StartRange && *Offset <= Item->EndRange) {
        Status = EFI_INVALID_PARAMETER;
      }
    }
    
    if (Type == IsaAccessTypeIo && Item->Type == EfiIsaAcpiResourceIo) {
      if (*Offset >= Item->StartRange &&
        (*Offset + Count * (1 << Width)) - 1 <= Item->EndRange) {
        return EFI_SUCCESS;
      }
      if (*Offset >= Item->StartRange && *Offset <= Item->EndRange) {
        Status = EFI_INVALID_PARAMETER;
      }
    }
    
    Item ++;
  }
  
  return Status;
}

EFI_STATUS
IsaIoMemRead (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  )
/*++

Routine Description:

  Performs an ISA Memory Read Cycle

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify the Isa Io Access
  //
  Status = IsaIoVerifyAccess (
              IsaIoDevice, 
              IsaAccessTypeMem, 
              Width, 
              Count, 
              &Offset
              );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Call PciIo->Mem.Read
  //
  Status = IsaIoDevice->PciIo->Mem.Read (
                                   IsaIoDevice->PciIo,
                                   (EFI_PCI_IO_PROTOCOL_WIDTH)Width,
                                   EFI_PCI_IO_PASS_THROUGH_BAR,
                                   Offset,
                                   Count,
                                   Buffer
                                   );
  
  return Status;
}

EFI_STATUS
IsaIoMemWrite (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  )
/*++

Routine Description:

  Performs an ISA Memory Write Cycle

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify Isa IO Access
  //
  Status = IsaIoVerifyAccess (
              IsaIoDevice, 
              IsaAccessTypeMem, 
              Width, 
              Count, 
              &Offset
              );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Call PciIo->Mem.Write
  //
  Status = IsaIoDevice->PciIo->Mem.Write (
                                   IsaIoDevice->PciIo,
                                   (EFI_PCI_IO_PROTOCOL_WIDTH)Width,
                                   EFI_PCI_IO_PASS_THROUGH_BAR,
                                   Offset,
                                   Count,
                                   Buffer
                                   );
                                   
  return Status;
}

EFI_STATUS
IsaIoIoRead (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  )
/*++

Routine Description:

  Performs an ISA I/O Read Cycle

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify Isa IO Access
  //
  Status = IsaIoVerifyAccess (
              IsaIoDevice, 
              IsaAccessTypeIo, 
              Width, 
              Count, 
              &Offset
              );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Call PciIo->Io.Read
  //
  Status = IsaIoDevice->PciIo->Io.Read (
                                  IsaIoDevice->PciIo,
                                  (EFI_PCI_IO_PROTOCOL_WIDTH)Width,
                                  EFI_PCI_IO_PASS_THROUGH_BAR,
                                  Offset,
                                  Count,
                                  Buffer
                                  );
                                  
   
  return Status;
}

EFI_STATUS
IsaIoIoWrite (
  IN     EFI_ISA_IO_PROTOCOL        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  )
/*++

Routine Description:

  Performs an ISA I/O Write Cycle

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify Isa IO Access
  //
  Status = IsaIoVerifyAccess (
              IsaIoDevice, 
              IsaAccessTypeIo, 
              Width, 
              Count, 
              &Offset
              );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Call PciIo->Io.Write
  //
  Status = IsaIoDevice->PciIo->Io.Write (
                                  IsaIoDevice->PciIo,
                                  (EFI_PCI_IO_PROTOCOL_WIDTH)Width,
                                  EFI_PCI_IO_PASS_THROUGH_BAR,
                                  Offset,
                                  Count,
                                  Buffer
                                  );
                                  
  
  return Status;
}

EFI_STATUS
IsaIoCopyMem (
  IN EFI_ISA_IO_PROTOCOL        *This,
  IN EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN UINT32                     DestOffset,
  IN UINT32                     SrcOffset,
  IN UINTN                      Count
  )
/*++

Routine Description:

  Performs an ISA I/O Copy Memory 

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify Isa IO Access for destination and source
  //
  Status = IsaIoVerifyAccess (
              IsaIoDevice, 
              IsaAccessTypeMem, 
              Width, 
              Count, 
              &DestOffset
              );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  Status = IsaIoVerifyAccess (
              IsaIoDevice, 
              IsaAccessTypeMem, 
              Width, 
              Count, 
              &SrcOffset
              );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Call PciIo->CopyMem
  //
  Status = IsaIoDevice->PciIo->CopyMem (
                               IsaIoDevice->PciIo,
                               (EFI_PCI_IO_PROTOCOL_WIDTH)Width,
                               EFI_PCI_IO_PASS_THROUGH_BAR,
                               DestOffset,
                               EFI_PCI_IO_PASS_THROUGH_BAR,
                               SrcOffset,
                               Count
                               );
                               
    
  return Status;
}

static
EFI_STATUS
WritePort (
  IN EFI_ISA_IO_PROTOCOL  *This,
  IN UINT32               Offset,
  IN UINT8                Value
  )
/*++

Routine Description:

  Writes an 8 bit I/O Port

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Call PciIo->Io.Write
  //
  Status = IsaIoDevice->PciIo->Io.Write (
                                  IsaIoDevice->PciIo,
                                  EfiPciIoWidthUint8,
                                  EFI_PCI_IO_PASS_THROUGH_BAR,
                                  Offset,
                                  1,
                                  &Value
                                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

//  This->Io.Write (This, EfiIsaIoWidthUint8, Offset, 1, &Value);
  gBS->Stall (50);

  return EFI_SUCCESS;
}

static
EFI_STATUS
WriteDmaPort (
  IN EFI_ISA_IO_PROTOCOL  *This,
  IN UINT32               AddrOffset,
  IN UINT32               PageOffset,
  IN UINT32               CountOffset,
  IN UINT32               BaseAddress,
  IN UINT16               Count
  )
/*++

Routine Description:

  Writes an 8 bit I/O Port

Arguments:

Returns:

  None

--*/
{
  WritePort (This, AddrOffset, (UINT8)(BaseAddress & 0xff));
  WritePort (This, AddrOffset, (UINT8)((BaseAddress >> 8) & 0xff));
  WritePort (This, PageOffset, (UINT8)((BaseAddress >> 16) & 0xff));
  WritePort (This, CountOffset, (UINT8)(Count & 0xff));
  WritePort (This, CountOffset, (UINT8)((Count >> 8) & 0xff));
  return EFI_SUCCESS;
}

EFI_STATUS
IsaIoMap (
  IN     EFI_ISA_IO_PROTOCOL            *This,
  IN     EFI_ISA_IO_PROTOCOL_OPERATION  Operation,
  IN     UINT8                          ChannelNumber,      
  IN     UINT32                         ChannelAttributes,  
  IN     VOID                           *HostAddress,
  IN OUT UINTN                          *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
  OUT    VOID                           **Mapping
  )
/*++

Routine Description:

  Maps a memory region for DMA

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS            Status;
  ISA_IO_DEVICE         *IsaIoDevice;
  BOOLEAN               Master;
  BOOLEAN               Read;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;
  ISA_MAP_INFO          *IsaMapInfo;
  UINT8                 DmaMode;
  UINTN                 MaxNumberOfBytes;
  UINT32                BaseAddress;
  UINT16                Count;
  
  UINT8                 DmaMask;
  UINT8                 DmaClear;
  UINT8                 DmaChannelMode;


  if ( (NULL == This)
     || (NULL == HostAddress)
     || (NULL == NumberOfBytes)
     || (NULL == DeviceAddress)
     || (NULL == Mapping) ) {
    return EFI_INVALID_PARAMETER;
  }

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Initialize the return values to their defaults
  //
  *Mapping = NULL;

  //
  // Make sure the Operation parameter is valid
  //
  if (Operation < 0 || Operation >= EfiIsaIoOperationMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // See if this is a Slave DMA Operation
  //
  Master = TRUE;
  Read   = FALSE;
  if (Operation == EfiIsaIoOperationSlaveRead) {
    Operation = EfiIsaIoOperationBusMasterRead;
    Master = FALSE;
    Read   = TRUE;
  }
  if (Operation == EfiIsaIoOperationSlaveWrite) {
    Operation = EfiIsaIoOperationBusMasterWrite;
    Master = FALSE;
    Read   = FALSE;
  }

  if (!Master) {
    //
    // Make sure that ChannelNumber is a valid channel number
    // Channel 4 is used to cascade, so it is illegal.
    //
    if (ChannelNumber == 4 || ChannelNumber > 7) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // This implementation only support COMPATIBLE DMA Transfers
    //
    if (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_COMPATIBLE)){
      return EFI_INVALID_PARAMETER;
    }
    if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_A) {
      return EFI_INVALID_PARAMETER;
    }
    if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_B) {
      return EFI_INVALID_PARAMETER;
    }
    if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_C) {
      return EFI_INVALID_PARAMETER;
    }

    if (ChannelNumber < 4) {
      //
      // If this is Channel 0..3, then the width must be 8 bit
      //
      if (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_8)) {
        return EFI_INVALID_PARAMETER;
      }
      if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_16) {
        return EFI_INVALID_PARAMETER;
      }
    } else {
      //
      // If this is Channel 4..7, then the width must be 16 bit
      //
      if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_8) {
        return EFI_INVALID_PARAMETER;
      }
      if (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_16)) {
        return EFI_INVALID_PARAMETER;
      }
    }

    //
    // Either Demand Mode or Single Mode must be selected, but not both
    //
    if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SINGLE_MODE) {
      if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_DEMAND_MODE) {
        return EFI_INVALID_PARAMETER;
      }
    } else {
      if (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_DEMAND_MODE)) {
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  //
  // Map the HostAddress to a DeviceAddress.
  //
  PhysicalAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) HostAddress;
  if ((PhysicalAddress + *NumberOfBytes) > ISA_MAX_MEMORY_ADDRESS) {

    //
    // Common Buffer operations can not be remapped.  If the common buffer
    // if above 16MB, then it is not possible to generate a mapping, so return 
    // an error.
    //
    if (Operation == EfiIsaIoOperationBusMasterCommonBuffer) {
      return EFI_UNSUPPORTED;
    }

    //
    // Allocate an ISA_MAP_INFO structure to remember the mapping when Unmap() 
    // is called later.
    //
    IsaMapInfo = EfiLibAllocatePool (sizeof (ISA_MAP_INFO));
    if (IsaMapInfo == NULL) {
      *NumberOfBytes = 0;
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Return a pointer to the MAP_INFO structure in Mapping
    //
    *Mapping = IsaMapInfo;

    //
    // Initialize the MAP_INFO structure
    //
    IsaMapInfo->Operation         = Operation;
    IsaMapInfo->NumberOfBytes     = *NumberOfBytes;
    IsaMapInfo->NumberOfPages     = EFI_SIZE_TO_PAGES (*NumberOfBytes);
    IsaMapInfo->HostAddress       = PhysicalAddress;
    IsaMapInfo->MappedHostAddress = ISA_MAX_MEMORY_ADDRESS - 1;

    //
    // Allocate a buffer below 4GB to map the transfer to.
    //
    Status = gBS->AllocatePages (
                    AllocateMaxAddress, 
                    EfiBootServicesData, 
                    IsaMapInfo->NumberOfPages,
                    &IsaMapInfo->MappedHostAddress
                    );
    if (EFI_ERROR(Status)) {
      gBS->FreePool (IsaMapInfo);
      *NumberOfBytes = 0;
      *Mapping = NULL;
      return Status;
    }

    //
    // If this is a read operation from the DMA agents's point of view,
    // then copy the contents of the real buffer into the mapped buffer
    // so the DAM agent can read the contents of the real buffer.
    //
    if (Operation == EfiIsaIoOperationBusMasterRead) {
      EfiCopyMem (
        (VOID *)(UINTN)IsaMapInfo->MappedHostAddress, 
        (VOID *)(UINTN)IsaMapInfo->HostAddress,
        IsaMapInfo->NumberOfBytes
        );
    }

    //
    // The DeviceAddress is the address of the maped buffer below 16 MB
    //
    *DeviceAddress = IsaMapInfo->MappedHostAddress;
  } else {
    //
    // The transfer is below 16 MB, so the DeviceAddress is simply the 
    // HostAddress
    //
    *DeviceAddress = PhysicalAddress;
  }

  //
  // If this is a Bus Master operation then return
  //
  if (Master) {
    return EFI_SUCCESS;
  }

  //
  // Figure out what to program into the DMA Channel Mode Register
  //
  DmaMode = (UINT8)(DMA_MODE_INCREMENT | (ChannelNumber & 0x03));
  if (Read) {
    DmaMode |= DMA_MODE_READ;
  } else {
    DmaMode |= DMA_MODE_WRITE;
  }
  if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_AUTO_INITIALIZE) {
    DmaMode |= DMA_MODE_AUTO_INITIALIZE;
  }
  if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_DEMAND_MODE) {
    DmaMode |= DMA_MODE_DEMAND;
  }
  if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SINGLE_MODE) {
    DmaMode |= DMA_MODE_SINGLE;
  }

  //
  // A Slave DMA transfer can not cross a 64K boundary.
  // Compute *NumberOfBytes based on this restriction.
  //
  MaxNumberOfBytes = 0x10000 - ((UINT32)(*DeviceAddress) & 0xffff);
  if (*NumberOfBytes > MaxNumberOfBytes) {
    *NumberOfBytes = MaxNumberOfBytes;
  }

  //
  // Compute the values to program into the BaseAddress and Count registers 
  // of the Slave DMA controller
  //
  if (ChannelNumber < 4) {
    BaseAddress = (UINT32)(*DeviceAddress);
    Count = (UINT16)(*NumberOfBytes - 1);
  } else {
    BaseAddress = (UINT32)(((UINT32)(*DeviceAddress) & 0xff0000) | (((UINT32)(*DeviceAddress) & 0xffff) >> 1));
    Count = (UINT16)((*NumberOfBytes - 1) >> 1);
  }

  //
  // Program the DMA Write Single Mask Register for ChannelNumber
  // Clear the DMA Byte Pointer Register
  //
  if (ChannelNumber < 4) {
    DmaMask         = DMA_SINGLE_MASK_0_3;
    DmaClear        = DMA_CLEAR_0_3;
    DmaChannelMode  = DMA_MODE_0_3;
  } else {
    DmaMask         = DMA_SINGLE_MASK_4_7;
    DmaClear        = DMA_CLEAR_4_7;
    DmaChannelMode  = DMA_MODE_4_7;
  }
  
  WritePort (
     This, 
     DmaMask, 
     (UINT8)(DMA_CHANNEL_MASK_SELECT | (ChannelNumber & 0x03))
     );
  WritePort (
     This, 
     DmaClear, 
     (UINT8)(DMA_CHANNEL_MASK_SELECT | (ChannelNumber & 0x03))
     );
  WritePort (This, DmaChannelMode, DmaMode);
  
  WriteDmaPort (
         This, 
         DmaRegisters[ChannelNumber].Address, 
         DmaRegisters[ChannelNumber].Page, 
         DmaRegisters[ChannelNumber].Count,
         BaseAddress,
         Count
         );
    
  WritePort (
     This, 
     DmaMask, 
     (UINT8)(ChannelNumber & 0x03)
     );

  return EFI_SUCCESS;
} 

EFI_STATUS
IsaIoUnmap (
  IN EFI_ISA_IO_PROTOCOL  *This,
  IN VOID                 *Mapping
  )
/*++

Routine Description:

  Unmaps a memory region for DMA

Arguments:

Returns:

  None

--*/
{
  ISA_MAP_INFO  *IsaMapInfo;

  //
  // See if the Map() operation associated with this Unmap() required a mapping 
  // buffer.If a mapping buffer was not required, then this function simply 
  // returns EFI_SUCCESS.
  //
  if (Mapping != NULL) {
    //
    // Get the MAP_INFO structure from Mapping
    //
    IsaMapInfo = (ISA_MAP_INFO *)Mapping;

    //
    // If this is a write operation from the Agent's point of view,
    // then copy the contents of the mapped buffer into the real buffer
    // so the processor can read the contents of the real buffer.
    //
    if (IsaMapInfo->Operation == EfiIsaIoOperationBusMasterWrite) {
      EfiCopyMem (
        (VOID *)(UINTN)IsaMapInfo->HostAddress, 
        (VOID *)(UINTN)IsaMapInfo->MappedHostAddress,
        IsaMapInfo->NumberOfBytes
        );
    }

    //
    // Free the mapped buffer and the MAP_INFO structure.
    //
    gBS->FreePages (IsaMapInfo->MappedHostAddress, IsaMapInfo->NumberOfPages);
    gBS->FreePool (IsaMapInfo);
  }
  return EFI_SUCCESS;
}

EFI_STATUS
IsaIoAllocateBuffer (
  IN  EFI_ISA_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE    Type,
  IN  EFI_MEMORY_TYPE      MemoryType,
  IN  UINTN                Pages,
  OUT VOID                 **HostAddress,
  IN  UINT64               Attributes
  )
/*++

Routine Description:

  Allocates a common buffer for DMA

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;

  if (HostAddress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (Type < AllocateAnyPages || Type >= MaxAllocateType) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // The only valid memory types are EfiBootServicesData and EfiRuntimeServicesData
  //
  if (MemoryType != EfiBootServicesData && MemoryType != EfiRuntimeServicesData) {
    return EFI_INVALID_PARAMETER;
  }

  if (Attributes & 
    ~(EFI_ISA_IO_ATTRIBUTE_MEMORY_WRITE_COMBINE | EFI_ISA_IO_ATTRIBUTE_MEMORY_CACHED)) {
    return EFI_UNSUPPORTED;
  }
  

  PhysicalAddress = (EFI_PHYSICAL_ADDRESS)(ISA_MAX_MEMORY_ADDRESS-1);
  if (Type == AllocateAddress) {
    if ((UINTN)(*HostAddress) >= ISA_MAX_MEMORY_ADDRESS) {
      return EFI_UNSUPPORTED;
    } else {
      PhysicalAddress = (UINTN)(*HostAddress);
    }
  }
  
  if (Type == AllocateAnyPages) {
    Type = AllocateMaxAddress;
  }
  
  Status = gBS->AllocatePages (Type, MemoryType, Pages, &PhysicalAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *HostAddress = (VOID *)(UINTN)PhysicalAddress;
  return Status;
}

EFI_STATUS
IsaIoFreeBuffer (
  IN EFI_ISA_IO_PROTOCOL  *This,
  IN UINTN                Pages,
  IN VOID                 *HostAddress
  )
/*++

Routine Description:

  Frees a common buffer 

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Call PciIo->FreeBuffer
  //
  Status = IsaIoDevice->PciIo->FreeBuffer (
                               IsaIoDevice->PciIo,
                               Pages,
                               HostAddress
                               );
  
  return Status;
}

EFI_STATUS
IsaIoFlush (
  IN EFI_ISA_IO_PROTOCOL  *This
  )
/*++

Routine Description:

  Flushes a DMA buffer

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Call PciIo->Flush
  //
  Status = IsaIoDevice->PciIo->Flush (IsaIoDevice->PciIo);
  
  return Status;
}
