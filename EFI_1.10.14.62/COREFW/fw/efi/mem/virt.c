/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    virt.c

Abstract:


Revision History

--*/

#include "imem.h"
#include EFI_PROTOCOL_DEFINITION (UgaIo)

#pragma BEGIN_RUNTIME_DATA()
EFI_GUID  LocalEfiUgaIoProtocolGuid= EFI_UGA_IO_PROTOCOL_GUID;
#pragma END_RUNTIME_DATA()

//
// This procedure name was changed to allow an assembler front end to
// intercept the call.  This is necessary to guarantee that no
// speculative loads are performed before the IVA is setup.  If we
// wrote the code to restore the IVA in 'C', we couldn't be sure
// that it wouldn't try something speculative.
// 
// We could try the #pragma optimize("",off) command but we have
// no real guarantee that this really disables speculative code.
// probably safer going with the assembler routine.  Only danger
// is if the input/output of this routine changes, the assmbler
// routine may have issues.  We helped mitigate this by declaring
// 8 outs from the assembler routine to ensure all params were passed
// through, this should have no bad side-effects.
//

EFI_STATUS
EFIAPI
RUNTIMEFUNCTION
RtSetVirtualAddressMapInternal (
    IN UINTN                        MemoryMapSize,
    IN UINTN                        DescriptorSize,
    IN UINT32                       DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR        *VirtualMap
    );

//
//
//
#pragma RUNTIME_CODE(RtSetVirtualAddressMapInternal)
EFI_STATUS
EFIAPI
RUNTIMEFUNCTION
RtSetVirtualAddressMapInternal (
    IN UINTN                        MemoryMapSize,
    IN UINTN                        DescriptorSize,
    IN UINT32                       DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR        *VirtualMap
    )
{
    LIST_ENTRY                      *Link;
    MEMORY_MAP                      *Entry;
    EFI_MEMORY_DESCRIPTOR           *VirtEntry;
    UINT64                          PhysicalEnd;
    EFI_STATUS                      Status;
    UINTN                           Index, MaxIndex;
    UINTN                           Index1;
    UINT32                          Crc;
    EFI_DRIVER_OS_HANDOFF_HEADER    *DriverOsHandoffHeader;
    EFI_DRIVER_OS_HANDOFF           *DriverOsHandoff;
    
    //
    // Can only switch to virtual addresses once the memory map is locked down,
    // and can only set it once
    // 
    
    if (!EfiAtRuntime  ||  EfiVirtualMode) {
        return EFI_UNSUPPORTED;
    }

    //
    // Only understand the original descriptor format
    //

    if (DescriptorVersion != EFI_MEMORY_DESCRIPTOR_VERSION ||
        DescriptorSize < sizeof (EFI_MEMORY_DESCRIPTOR) ) {
        return EFI_INVALID_PARAMETER;
    }

    DEBUG ((D_INIT, "%ESetVirtualAddressMap: Entered%N\n"));

    //
    // OK - let's apply all the virtual address values
    //

    AcquireLock (&MemoryLock);

    //
    // Clear all virtual settings
    //

    for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
        Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
        Entry->VirtualStart = 0;
    }

    //
    // Apply virtual settings
    //

    Status = EFI_SUCCESS;
    MaxIndex = MemoryMapSize / DescriptorSize;
    VirtEntry = VirtualMap;

    for (Index = 0; Index < MaxIndex; Index++) {

        //
        // If this map entry doesn't have a virtual address, skip it
        // N.B. VirtualStart of 0 not allowed
        //

        if (VirtEntry->VirtualStart) {
            PhysicalEnd = VirtEntry->PhysicalStart + LShiftU64(VirtEntry->NumberOfPages, EFI_PAGE_SHIFT);

            for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
                Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);

                //
                // If this VirtEntry is for this entry, capture the virtual address
                //

                if (Entry->Start >= VirtEntry->PhysicalStart  &&
                    Entry->End   <= PhysicalEnd) {

                    Entry->VirtualStart = VirtEntry->VirtualStart + Entry->Start - VirtEntry->PhysicalStart;
                    DEBUG ((D_INIT, "Mapping of: %08lx-%08lx  va:%08lx\n", 
                                Entry->Start,
                                Entry->End,
                                Entry->VirtualStart
                                ));
                    break;
                }
            }

            if (Link == &MemoryMap) {
                DEBUG ((D_ERROR, "SetVirtualAddressMap: VirtualMap entry %d not found in memory map\n", Index));
                Status = EFI_NOT_FOUND;
            }
        }

        VirtEntry = NextMemoryDescriptor(VirtEntry, DescriptorSize);
    }


    //
    // Make sure all required ranges obtained a mapping
    //

    for (Link = MemoryMap.Flink; Link != &MemoryMap; Link = Link->Flink) {
        Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
    
        if ((Entry->Attribute & EFI_MEMORY_RUNTIME) && !Entry->VirtualStart) {

            DEBUG ((D_ERROR, "SetVirtualAddressMap: Mapping not supplied for %lx-%lx\n",
                        Entry->Start,
                        Entry->End
                        ));

            Status = EFI_NO_MAPPING;
        }
    }

    ReleaseLock (&MemoryLock);

    //
    // If there's 
    //

    if (!EFI_ERROR(Status)) {

        //
        // Set virtual mode active
        //

        EfiVirtualMode = TRUE;
        MemoryLastConvert = CR(MemoryMap.Flink, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);

        //
        // Notify general components of the memory map is changing
        //

        RtNotifySignalList (EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE);

        //
        // Relocate runtime images
        //

        RtLoaderVirtualAddressFixup();

        //
        // Convert the EFI Runtime Services Table
        //
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->GetTime);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->SetTime);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->GetWakeupTime);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->SetWakeupTime);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->GetVariable);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->GetNextVariableName);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->SetVariable);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->GetNextHighMonotonicCount);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &RT->ResetSystem);
        RT->Hdr.CRC32 = 0;
        RtCalculateCrc32((UINT8 *)&RT->Hdr, RT->Hdr.HeaderSize, &Crc);
        RT->Hdr.CRC32 = Crc;

        //
        // Convert the UGA OS Handoff Table if it is present in the Configuration Table
        //
        for (Index = 0; Index < ST->NumberOfTableEntries; Index++) {
          if (RtCompareGuid(&LocalEfiUgaIoProtocolGuid,&(ST->ConfigurationTable[Index].VendorGuid)) == 0) {
            DriverOsHandoffHeader = ST->ConfigurationTable[Index].VendorTable;
            for (Index1 = 0; Index1 < DriverOsHandoffHeader->NumberOfEntries; Index1++) {
              DriverOsHandoff = (EFI_DRIVER_OS_HANDOFF *)((UINTN)DriverOsHandoffHeader + sizeof (DriverOsHandoffHeader) + Index1 * DriverOsHandoffHeader->SizeOfEntries);
              RtConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &DriverOsHandoff->DevicePath);
              RtConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &DriverOsHandoff->PciRomImage);
            }
            RtConvertPointer (EFI_OPTIONAL_PTR, (VOID **) &(ST->ConfigurationTable[Index].VendorTable));
          }
        }

        //
        // Convert the EFI System Table
        //
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &ST->FirmwareVendor);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &ST->RuntimeServices);
        RtConvertPointer (EFI_OPTIONAL_PTR|EFI_INTERNAL_FNC, (VOID **) &ST->ConfigurationTable);
        ST->Hdr.CRC32 = 0;
        RtCalculateCrc32((UINT8 *)&ST->Hdr, ST->Hdr.HeaderSize, &Crc);
        ST->Hdr.CRC32 = Crc;

        //
        // Notify event system of address change
        //

        RtEventVirtualAddressFixup();

        //
        // Notify the lib to convert
        //

        DEBUG ((D_INIT, "%ESetVirtualAddressMap: EFI done%N (calling platform code)\n"));
        LibFwInstance = FALSE;
        RtLibEnableVirtualMappings();
      
        //
        // Notify the platform code to convert the addresses of the
        // EFI firmware itself.
        //
        // N.B. we leave the MemoryMap list & ConvertAddress to be 
        // in physical mode.  This allows PL->SetVirtualMappings to
        // use ConvertAddress, and we don't need to ever run the
        // memory map structures past this point.
        //
        if (PL->SetVirtualAddressMap)   // may be NULL in 32bit system
            PL->SetVirtualAddressMap (
                    RtConvertPointer,
                    MemoryMapSize,
                    DescriptorSize,
                    DescriptorVersion,
                    VirtualMap
                    );
    }

    return Status;
}

#pragma RUNTIME_CODE(RtConvertPointer)
EFI_STATUS
RUNTIMEFUNCTION
RtConvertPointer (
    IN UINTN        DebugDisposition,    
    IN OUT VOID     **pAddress
    )
{
    UINTN           Address;
    MEMORY_MAP      *Entry;
    LIST_ENTRY      *Link;

    //
    // Make sure pAddress is a valid pointer
    //
    if (pAddress == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Get the address to convert
    //
    Address = (UINTN) *pAddress;

    //
    // If this is a null pointer, return if it's allowed
    //
    if (Address == 0) {
      if (DebugDisposition & EFI_OPTIONAL_PTR) {
        return EFI_SUCCESS;
      }
      return EFI_INVALID_PARAMETER;
    }
        
    //
    // If it's not in the range of the last convert, find the 
    //

    Entry = MemoryLastConvert;
    if (Entry->Start > Address  ||  Entry->End < Address) {

        for (Link=MemoryMap.Flink; Link != &MemoryMap; Link=Link->Flink) {
            Entry = CR(Link, MEMORY_MAP, Link, MEMORY_MAP_SIGNATURE);
            if (Entry->Start <= Address && Entry->End >= Address) {
                break;
            }
        }

        //
        // If it was not found, what should we do?
        //

        if (Link == &MemoryMap) {

#ifdef EFI_NT_EMULATOR
            //
            // If this is the nt emulation environment, skip not found
            // entries that are for internal functions as they don't 
            // appear in the memory map
            //

            if (DebugDisposition & (EFI_INTERNAL_FNC | EFI_INTERNAL_PTR)) {
                return EFI_NOT_FOUND;
            }
#endif

            DEBUG ((D_ERROR, "ConvertAddress: Attempting to convert a non-runtime address %x\n", Address));
            ASSERT (FALSE);
            for (; ;) ;
            return EFI_NOT_FOUND;
        }

        MemoryLastConvert = Entry;
    }


    //
    // Compute new address
    //

    Address = Address - (UINTN) Entry->Start + (UINTN) Entry->VirtualStart;
    *pAddress = (VOID *) Address;
    return EFI_SUCCESS;
}
