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

    efildr.c
    
Abstract:



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "pe.h"
#include "decompress.h"
#include "EfiLdrHandoff.h"

#define INT15_E820_AddressRangeMemory   1
#define INT15_E820_AddressRangeReserved 2
#define INT15_E820_AddressRangeACPI     3
#define INT15_E820_AddressRangeNVS      4

#define EFILDR_LOAD_ADDRESS        (EFILDR_BASE_SEGMENT << 4)
#define EFILDR_HEADER_ADDRESS      (EFILDR_LOAD_ADDRESS+0x2000)

#define EFI_FIRMWARE_BASE_ADDRESS  0x00200000
#define EFI_MAX_STACK_SIZE         0x00020000

#define EFI_DECOMPRESSED_BUFFER_ADDRESS 0x00600000

#define EFI_MAX_MEMORY_DESCRIPTORS 64

#define LOADED_IMAGE_SIGNATURE     EFI_SIGNATURE_32('l','d','r','i')

typedef struct {
    UINTN                       Signature;
    CHAR16                      *Name;          // Displayable name
    UINTN                       Type;

    BOOLEAN                     Started;        // If entrypoint has been called
    VOID                        *StartImageContext;

    EFI_IMAGE_ENTRY_POINT       EntryPoint;     // The image's entry point
    EFI_LOADED_IMAGE            Info;           // loaded image protocol

    // 
    EFI_PHYSICAL_ADDRESS        ImageBasePage;  // Location in memory
    UINTN                       NoPages;        // Number of pages 
    CHAR8                       *ImageBase;     // As a char pointer
    CHAR8                       *ImageEof;      // End of memory image

    // relocate info
    CHAR8                       *ImageAdjust;   // Bias for reloc calculations
    UINTN                       StackAddress;
    CHAR8                       *FixupData;     //  Original fixup data
} EFILDR_LOADED_IMAGE;

typedef struct {
    UINT32       CheckSum;
    UINT32       Offset;
    UINT32       Length;
    UINT8        FileName[52];
} EFILDR_IMAGE;

typedef struct {          
    UINT32       Signature;     
    UINT32       HeaderCheckSum;
    UINT32       FileLength;
    UINT32       NumberOfImages;
} EFILDR_HEADER;

typedef struct {          
    UINT32       BaseAddress;
    UINT32       Foo;
    UINT32       Length;
    UINT32       Bar;
    UINT32       Type;
} BIOS_MEMORY_MAP_ENTRY;

typedef struct {          
    UINT32                MemoryMapSize;
    BIOS_MEMORY_MAP_ENTRY MemoryMapEntry[1];
} BIOS_MEMORY_MAP;

//
//
//

EFI_STATUS
EFIAPI
Ia32EmbGetInfo (
  IN      EFI_DECOMPRESS_PROTOCOL *This,
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  OUT     UINT32  *DstSize,
  OUT     UINT32  *ScratchSize
  );

EFI_STATUS
EFIAPI
Ia32EmbDecompress (
  IN      EFI_DECOMPRESS_PROTOCOL *This,
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32  DstSize,
  IN OUT  VOID   *Scratch,
  IN      UINT32  ScratchSize
  );

EFI_STATUS
GetPeImageInfo (
    IN VOID                     *FHand,
    OUT UINT32                  *ImageBase,
    OUT UINT32                  *ImageSize
    );

EFI_STATUS
Ia32EmbLoadPeImage (
    IN VOID                     *FHand,
    IN EFILDR_LOADED_IMAGE      *Image,
    IN UINT32                   *NumberOfMemoryMapEntries,
    IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
    );

STATIC
EFI_STATUS
Ia32EmbLoadPeRelocate (
    IN EFILDR_LOADED_IMAGE      *Image,
    IN IMAGE_DATA_DIRECTORY     *RelocDir,
    IN UINTN                     Adjust,
    IN UINT32                   *NumberOfMemoryMapEntries,
    IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
    );

STATIC
VOID
Ia32EmbConvertPeImage (
    IN EFILDR_LOADED_IMAGE      *Image,
    IN UINT32                   DescriptorCount,
    IN UINT32                   DescriptorSize,
    IN CHAR8                    *MemoryMap
    );

STATIC
EFI_STATUS
Ia32EmbImageRead (
    IN VOID                 *FHand,
    IN UINTN                Offset,
    IN OUT UINTN            ReadSize,
    OUT VOID                *Buffer
    );

STATIC
VOID *
Ia32EmbImageAddress (
    IN EFILDR_LOADED_IMAGE     *Image,
    IN UINTN                   Address
    );

INTERNAL
EFI_STATUS
Ia32EmbSetImageType (
    IN OUT EFILDR_LOADED_IMAGE      *Image,
    IN UINTN                        ImageType
    );

INTERNAL
EFI_STATUS
Ia32EmbCheckImageMachineType (
    IN UINTN            MachineType
    );

EFI_STATUS
EfiAddMemoryDescriptor(
    UINTN                 *NoDesc,
    EFI_MEMORY_DESCRIPTOR *Desc,
    EFI_MEMORY_TYPE       Type,
    EFI_PHYSICAL_ADDRESS  BaseAddress,
    UINTN                 NoPages,
    UINT64                Attribute
    );

UINTN
FindSpace(
    UINTN                       NoPages,
    IN UINT32                   *NumberOfMemoryMapEntries,
    IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
    );

UINT64
EFILDRLShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  );

VOID
EFILDRZeroMem (
    IN VOID     *Buffer,
    IN UINTN    Size
    );

VOID
EFILDRCopyMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    );

UINTN
EFILDRstrcmpa (
    IN CHAR8    *s1,
    IN CHAR8    *s2
    );

VOID 
PrintValue(UINT32 Value);

VOID 
PrintString(UINT8 *String);

VOID 
ClearScreen();

//
//
//

STATIC EFILDR_CALLBACK EfiLdrCallBack;
EFILDR_LOADED_IMAGE    EfiCoreImage;
UINT8 *Cursor;

__declspec (naked)  
VOID
EfiLoader (
    UINT32    BiosMemoryMapBaseAddress
    )

{
    BIOS_MEMORY_MAP       *BiosMemoryMap;    
    EFILDR_HEADER         *EFILDRHeader;
    EFILDR_IMAGE          *EFILDRImage;

    UINTN                 i;
    EFI_MEMORY_DESCRIPTOR EfiMemoryDescriptor[EFI_MAX_MEMORY_DESCRIPTORS];
    EFI_STATUS            Status;
    UINT32                NumberOfMemoryMapEntries;
    UINT32                BaseAddress;
    UINT32                Length;
    EFI_MEMORY_TYPE       Type;
    UINT32                DestinationSize;
    UINT32                ScratchSize;
    UINTN                 Attr;
    UINT32                EfiLoaderImageBase;
    UINT32                EfiLoaderImageSize;


    // This function is a 'naked' function, meaning the compiler does not set
    // up the call frame.  Furthermore, we must set up the call frame as if the
    // function was called with a 'near' call, even though it was a 'far' call.
    // The compiler thinks it is a 'NEAR' function and expects only EIP on the
    // stack after the parameters.  Since we made a 'FAR' call to this function,
    // the parameters are 8 bytes into the stack rather than 4.  To fix this,
    // we pop the stack once before setting up the call frame, which leaves the
    // parameters 4 bytes into the stack as expected.  Since we'll never return,
    // there's no worry about what was popped off
    
    // set up call frame since this is a 'naked' function
    __asm pop   eax                 // initial pop to make it look like a 'near' call
    __asm push  ebp
    __asm mov   ebp, esp
    __asm sub   esp, __LOCAL_SIZE

    
    *(UINT8 *)(0x000b8000+10) = 'A';

    ClearScreen();
    PrintString("EFI Loader 0.2\n");   

//    PrintString("&BiosMemoryMapBaseAddress = ");   
//    PrintValue((UINT32)(&BiosMemoryMapBaseAddress));
//    PrintString("  BiosMemoryMapBaseAddress = ");   
//    PrintValue(BiosMemoryMapBaseAddress);
//    PrintString("\n");

    //
    // Add all EfiConventionalMemory descriptors to the table.  If there are partial pages, then
    // round the start address up to the next page, and round the length down to a page boundry.
    //

    BiosMemoryMap = (BIOS_MEMORY_MAP *)(BiosMemoryMapBaseAddress);
    
    NumberOfMemoryMapEntries = 0;

    for(i=0;i<BiosMemoryMap->MemoryMapSize / sizeof(BIOS_MEMORY_MAP_ENTRY);i++) {

        switch(BiosMemoryMap->MemoryMapEntry[i].Type) { 
            case (INT15_E820_AddressRangeMemory):
                Type = EfiConventionalMemory;
                Attr = EFI_MEMORY_WB;
                break;
            case (INT15_E820_AddressRangeReserved):
                Type = EfiReservedMemoryType;
                Attr = EFI_MEMORY_UC;
                break;
            case (INT15_E820_AddressRangeACPI):
                Type = EfiACPIReclaimMemory;
                Attr = EFI_MEMORY_WB;
                break;
            case (INT15_E820_AddressRangeNVS):
                Type = EfiACPIMemoryNVS;
                Attr = EFI_MEMORY_UC;
                break;
            default:
                // We should not get here, according to ACPI 2.0 Spec.
                // BIOS behaviour of the Int15h, E820h
                Type = EfiReservedMemoryType;
                Attr = EFI_MEMORY_UC;
                break;
        }
        if (Type == EfiConventionalMemory) {
            BaseAddress = BiosMemoryMap->MemoryMapEntry[i].BaseAddress;
            Length      = BiosMemoryMap->MemoryMapEntry[i].Length;
            if (BaseAddress & EFI_PAGE_MASK) {
                Length      = Length + (BaseAddress & EFI_PAGE_MASK) - EFI_PAGE_SIZE;
                BaseAddress = ((BaseAddress >> EFI_PAGE_SHIFT) + 1) << EFI_PAGE_SHIFT;
            }
        } else {
            BaseAddress = BiosMemoryMap->MemoryMapEntry[i].BaseAddress;
            Length      = BiosMemoryMap->MemoryMapEntry[i].Length + (BaseAddress & EFI_PAGE_MASK);
            BaseAddress = (BaseAddress >> EFI_PAGE_SHIFT) << EFI_PAGE_SHIFT;
            if (Length & EFI_PAGE_MASK) {
                Length = ((Length >> EFI_PAGE_SHIFT) + 1) << EFI_PAGE_SHIFT;
            }
        }
        EfiAddMemoryDescriptor(&NumberOfMemoryMapEntries,
                               EfiMemoryDescriptor,
                               Type,
                               (EFI_PHYSICAL_ADDRESS)BaseAddress,
                               Length>>EFI_PAGE_SHIFT,
                               Attr);
    }

    //
    // Add a memory descriptor for the Real Mode Interrupt Descriptor Table
    //

    EfiAddMemoryDescriptor(&NumberOfMemoryMapEntries,
                           EfiMemoryDescriptor,
                           EfiBootServicesData,
                           (EFI_PHYSICAL_ADDRESS)0x00000000,
                           1,
                           EFI_MEMORY_WB);

    //
    // Add a memory descriptor for the GDT and IDT used by EFI
    //

    EfiAddMemoryDescriptor(&NumberOfMemoryMapEntries,
                           EfiMemoryDescriptor,
                           EfiBootServicesData,
                           (EFI_PHYSICAL_ADDRESS)EFILDR_LOAD_ADDRESS,
                           (EFILDR_HEADER_ADDRESS - EFILDR_LOAD_ADDRESS + EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT,
                           EFI_MEMORY_WB);

    //
    // Get information on where the image is in memory
    //

    EFILDRHeader = (EFILDR_HEADER *)(EFILDR_HEADER_ADDRESS);
    EFILDRImage  = (EFILDR_IMAGE *)(EFILDR_HEADER_ADDRESS + sizeof(EFILDR_HEADER));

    //
    // Add a memory descriptor for this image so we can do the runtime transition latter
    //

    Status = GetPeImageInfo (
                 (VOID *)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
                 &EfiLoaderImageBase,
                 &EfiLoaderImageSize
                 );
    if (!EFI_ERROR (Status)) {
        EfiAddMemoryDescriptor(&NumberOfMemoryMapEntries,
                               EfiMemoryDescriptor,
                               EfiRuntimeServicesCode,
                               (EFI_PHYSICAL_ADDRESS)EfiLoaderImageBase,
                               (EfiLoaderImageSize + EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT,
                               EFI_MEMORY_WB);
    }

    //
    // Point to the 2nd image
    //
    
    EFILDRImage++;

    //
    // Add a memory descriptor for the remaining portion of the EFILDR file
    //

    EfiAddMemoryDescriptor(&NumberOfMemoryMapEntries,
                           EfiMemoryDescriptor,
                           EfiReservedMemoryType,
                           (EFI_PHYSICAL_ADDRESS)EFILDR_HEADER_ADDRESS,
                           (EFILDRHeader->FileLength + EFI_PAGE_SIZE -1) >> EFI_PAGE_SHIFT,
                           EFI_MEMORY_WB);

    *(UINT8 *)(0x000b8000+12) = 'B';

    //
    // Decompress the image
    //

    Status = Ia32EmbGetInfo(
               NULL, 
               (VOID *)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
               EFILDRImage->Length,
               &DestinationSize, 
               &ScratchSize
               );
    if (EFI_ERROR (Status)) {
      for(;;);
    }

    Status = Ia32EmbDecompress(
               NULL, 
               (VOID *)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
               EFILDRImage->Length,
               (VOID *)EFI_DECOMPRESSED_BUFFER_ADDRESS,
               DestinationSize, 
               (VOID *)((EFI_DECOMPRESSED_BUFFER_ADDRESS + DestinationSize + 0x1000) & 0xfffff000),
               ScratchSize
               );
    if (EFI_ERROR (Status)) {
      for(;;);
    }

    //
    // Load and relocate the EFI PE/COFF Firmware Image 
    //
    Status = Ia32EmbLoadPeImage ((VOID *)(EFI_DECOMPRESSED_BUFFER_ADDRESS), 
                          &EfiCoreImage, 
                          &NumberOfMemoryMapEntries, 
                          EfiMemoryDescriptor);

//    PrintString("Image.NoPages = ");   
//    PrintValue(Image.NoPages);
//    PrintString("\n");

    *(UINT8 *)(0x000b8000+14) = 'C';

    //
    // Display the table of memory descriptors.
    //

//    PrintString("\nEFI Memory Descriptors\n");   

    for(i=0;i<NumberOfMemoryMapEntries;i++) {
        PrintString("Type = ");   
        PrintValue(EfiMemoryDescriptor[i].Type);
        PrintString("  Start = ");   
        PrintValue((UINT32)(EfiMemoryDescriptor[i].PhysicalStart));
        PrintString("  NumberOfPages = ");   
        PrintValue((UINT32)(EfiMemoryDescriptor[i].NumberOfPages));
        PrintString("\n");
    }

    *(UINT8 *)(0x000b8000+16) = 'D';

    //
    // Jump to EFI Firmware
    //

    if (!EFI_ERROR(Status) && EfiCoreImage.EntryPoint!=NULL) {
	   static EFILDRHANDOFF Handoff;

	   Handoff.MemDescCount = NumberOfMemoryMapEntries;
	   Handoff.MemDesc = EfiMemoryDescriptor;
	   Handoff.EfiLdrCallBack = EfiLdrCallBack; 
     Handoff.ImageBase      = (VOID *)(UINTN)EfiCoreImage.ImageBasePage;
     Handoff.ImageSize      = EfiCoreImage.NoPages * EFI_PAGE_SIZE;

        __asm    lea     eax, Handoff
        __asm    mov     ebx, EfiCoreImage.EntryPoint
        __asm    mov     esp, Handoff.ImageBase
        __asm    push    eax
//
// seed the stack with a zero for the return from MainEntry() 
// so a debugger knows where to stop unwinding
//
        __asm    push    0
        __asm    jmp     ebx

/*       EfiCoreImage.EntryPoint (&Handoff); */
    }

    *(UINT8 *)(0x000b8000+18) = 'E';

    //
    // There was a problem loading the image, so HALT the system.
    //

    for(;;);
}

EFI_STATUS
GetPeImageInfo (
    IN VOID                     *FHand,
    OUT UINT32                  *ImageBase,
    OUT UINT32                  *ImageSize
    )
{
    EFI_STATUS                  Status;
    IMAGE_DOS_HEADER            DosHdr;
    IMAGE_NT_HEADERS            PeHdr;

    EFILDRZeroMem (&DosHdr, sizeof(DosHdr));
    EFILDRZeroMem (&PeHdr, sizeof(PeHdr));

    //
    // Read image headers
    //

    Ia32EmbImageRead (FHand, 0, sizeof(IMAGE_DOS_HEADER), &DosHdr);
    if (DosHdr.e_magic != IMAGE_DOS_SIGNATURE) {
        return EFI_UNSUPPORTED;
    }

    Ia32EmbImageRead (FHand, DosHdr.e_lfanew, sizeof(IMAGE_NT_HEADERS), &PeHdr);

    if (PeHdr.Signature != IMAGE_NT_SIGNATURE) {
        return EFI_UNSUPPORTED;
    }
    
    //
    // Verify machine type
    //

    Status = Ia32EmbCheckImageMachineType (PeHdr.FileHeader.Machine);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    *ImageBase = PeHdr.OptionalHeader.ImageBase;
    *ImageSize = PeHdr.OptionalHeader.SizeOfImage;

    return EFI_SUCCESS;
}

EFI_STATUS
Ia32EmbLoadPeImage (
    IN VOID                     *FHand,
    IN EFILDR_LOADED_IMAGE      *Image,
    IN UINT32                   *NumberOfMemoryMapEntries,
    IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
    )
{
    IMAGE_DOS_HEADER            DosHdr;
    IMAGE_NT_HEADERS            PeHdr;
    IMAGE_SECTION_HEADER        *FirstSection;
    IMAGE_SECTION_HEADER        *Section;
    UINTN                       Index;
    EFI_STATUS                  Status;
    CHAR8                       *Base, *End;
    EFI_PHYSICAL_ADDRESS        MaxPhysicalStart;
    UINT64                      MaxNoPages;
    IMAGE_DATA_DIRECTORY        *DirectoryEntry;
    UINTN                       DirCount;
    IMAGE_DEBUG_DIRECTORY_ENTRY TempDebugEntry;
    IMAGE_DEBUG_DIRECTORY_ENTRY *DebugEntry;
    UINTN                       CodeViewSize;
    UINTN                       CodeViewOffset;
    UINTN                       CodeViewFileOffset;

    EFILDRZeroMem (&DosHdr, sizeof(DosHdr));
    EFILDRZeroMem (&PeHdr, sizeof(PeHdr));

    //
    // Read image headers
    //

    Ia32EmbImageRead (FHand, 0, sizeof(IMAGE_DOS_HEADER), &DosHdr);
    if (DosHdr.e_magic != IMAGE_DOS_SIGNATURE) {
//        DEBUG ((D_LOAD, "Ia32EmbLoadPeImage: Dos header signature not found\n"));
*(UINT8 *)(0x000b8000+20) = 'F';
        return EFI_UNSUPPORTED;
    }

    Ia32EmbImageRead (FHand, DosHdr.e_lfanew, sizeof(IMAGE_NT_HEADERS), &PeHdr);

    if (PeHdr.Signature != IMAGE_NT_SIGNATURE) {
//        DEBUG ((D_LOAD, "Ia32EmbLoadPeImage: PE image header signature not found\n"));
*(UINT8 *)(0x000b8000+22) = 'G';
        return EFI_UNSUPPORTED;
    }
    
    //
    // Set the image subsystem type
    //

    Status = Ia32EmbSetImageType (Image, PeHdr.OptionalHeader.Subsystem);
    if (EFI_ERROR(Status)) {
//        DEBUG ((D_LOAD, "Ia32EmbLoadPeImage: Subsystem type not known\n"));
*(UINT8 *)(0x000b8000+24) = 'H';
        return Status;
    }

    //
    // Verify machine type
    //

    Status = Ia32EmbCheckImageMachineType (PeHdr.FileHeader.Machine);
    if (EFI_ERROR(Status)) {
//        DEBUG ((D_LOAD, "Ia32EmbLoadPeImage: Incorrect machine type\n"));
*(UINT8 *)(0x000b8000+26) = 'I';
        return Status;
    }

    //
    // Compute the amount of memory needed to load the image and 
    // allocate it.  This will include all sections plus the codeview debug info.
    // Since the codeview info is actually outside of the image, we calculate
    // its size seperately and add it to the total.
    //
    // Memory starts off as data
    //

    CodeViewSize = 0;
    DirectoryEntry = (IMAGE_DATA_DIRECTORY *)&(PeHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]);
    for (DirCount = 0; 
         (DirCount < DirectoryEntry->Size / sizeof (IMAGE_DEBUG_DIRECTORY_ENTRY)) && CodeViewSize == 0; 
         DirCount++) {
      Status = Ia32EmbImageRead (FHand, 
                        DirectoryEntry->VirtualAddress + DirCount * sizeof (IMAGE_DEBUG_DIRECTORY_ENTRY),
                        sizeof (IMAGE_DEBUG_DIRECTORY_ENTRY),
                        &TempDebugEntry);
      if (!EFI_ERROR (Status)) {
        if (TempDebugEntry.Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
          CodeViewSize = TempDebugEntry.SizeOfData;
          CodeViewFileOffset = TempDebugEntry.FileOffset;
        }
      }
    }
    
    CodeViewOffset = PeHdr.OptionalHeader.SizeOfImage + PeHdr.OptionalHeader.SectionAlignment;
    Image->NoPages = EFI_SIZE_TO_PAGES (CodeViewOffset + CodeViewSize);


    //
    // Compute the amount of memory needed to load the image and 
    // allocate it.  Memory starts off as data
    //

    MaxPhysicalStart = 0;
    for (Index = 0;Index < *NumberOfMemoryMapEntries; Index++) {
        if (EfiMemoryDescriptor[Index].Type == EfiConventionalMemory && 
            EfiMemoryDescriptor[Index].NumberOfPages >= (Image->NoPages + EFI_MAX_STACK_SIZE/EFI_PAGE_SIZE + 1)) {
            if (EfiMemoryDescriptor[Index].PhysicalStart > MaxPhysicalStart) {
                MaxPhysicalStart = EfiMemoryDescriptor[Index].PhysicalStart;
                MaxNoPages       = EfiMemoryDescriptor[Index].NumberOfPages;
            }
        }
    }
    if (MaxPhysicalStart == 0) {
      return EFI_OUT_OF_RESOURCES;
    }

    Image->ImageBasePage = MaxPhysicalStart + (((UINT32)MaxNoPages - (Image->NoPages + 1)) << EFI_PAGE_SHIFT);

    //
    // Add a memory descriptor for the EFI Core Firmware
    //
    EfiAddMemoryDescriptor(
      NumberOfMemoryMapEntries,
      EfiMemoryDescriptor,
      EfiRuntimeServicesCode,
      (EFI_PHYSICAL_ADDRESS)(Image->ImageBasePage),
      Image->NoPages,
      EFI_MEMORY_WB
      );

    //
    // Add a memory descriptor for the EFI Firmware Stack
    //
    EfiAddMemoryDescriptor(
      NumberOfMemoryMapEntries,
      EfiMemoryDescriptor,
      EfiBootServicesData,
      (EFI_PHYSICAL_ADDRESS)(Image->ImageBasePage-EFI_MAX_STACK_SIZE),
      EFI_MAX_STACK_SIZE/EFI_PAGE_SIZE,
      EFI_MEMORY_WB
      );

    if (EFI_ERROR(Status)) {
*(UINT8 *)(0x000b8000+28) = 'J';
        return Status;
    }

//    DEBUG((D_LOAD, "LoadPe: new image base %lx\n", Image->ImageBasePage));
    Image->Info.ImageBase = (VOID *) Image->ImageBasePage;
    Image->Info.ImageSize = (Image->NoPages << EFI_PAGE_SHIFT) - 1;
    Image->ImageBase = (CHAR8 *) Image->ImageBasePage;
    Image->ImageEof  = Image->ImageBase + Image->Info.ImageSize;
    Image->ImageAdjust = Image->ImageBase;

    //
    // Copy the Image header to the base location
    //
    Status = Ia32EmbImageRead (
                FHand, 
                0, 
                PeHdr.OptionalHeader.SizeOfHeaders, 
                Image->ImageBase
                );

    if (EFI_ERROR(Status)) {
*(UINT8 *)(0x000b8000+30) = 'K';
        return Status;
    }

    //
    // Load each directory of the image into memory... 
    //  Save the address of the Debug directory for later
    //
    DebugEntry = NULL;
    for (Index = 0; Index < PeHdr.OptionalHeader.NumberOfRvaAndSizes; Index++) {
      if (PeHdr.OptionalHeader.DataDirectory[Index].VirtualAddress != 0 &&
          PeHdr.OptionalHeader.DataDirectory[Index].Size != 0 ) {
        Status = Ia32EmbImageRead (
                    FHand,
                    PeHdr.OptionalHeader.DataDirectory[Index].VirtualAddress,
                    PeHdr.OptionalHeader.DataDirectory[Index].Size,
                    Image->ImageBase + PeHdr.OptionalHeader.DataDirectory[Index].VirtualAddress
                    );
        if (EFI_ERROR(Status)) {
          return Status;
        }
        if (Index == IMAGE_DIRECTORY_ENTRY_DEBUG) {
          DebugEntry = (IMAGE_DEBUG_DIRECTORY_ENTRY *) (Image->ImageBase + PeHdr.OptionalHeader.DataDirectory[Index].VirtualAddress);
        }
      }
    }

    //
    // Load each section of the image
    //

    // BUGBUG: change this to use the in memory copy

    FirstSection = (IMAGE_SECTION_HEADER *) (
                        Image->ImageBase +
                        DosHdr.e_lfanew + 
                        sizeof(PeHdr) + 
                        PeHdr.FileHeader.SizeOfOptionalHeader - 
                        sizeof (IMAGE_OPTIONAL_HEADER)
                        );

    Section = FirstSection;
    for (Index=0; Index < PeHdr.FileHeader.NumberOfSections; Index += 1) {

        //
        // Compute sections address
        //

        Base = Ia32EmbImageAddress(Image, Section->VirtualAddress);
        End = Ia32EmbImageAddress(Image, Section->VirtualAddress + Section->Misc.VirtualSize);
        
        if (EFI_ERROR(Status) || !Base  ||  !End) {
//            DEBUG((D_LOAD|D_ERROR, "LoadPe: Section %d was not loaded\n", Index));
*(UINT8 *)(0x000b8000+32) = 'L';
            return EFI_LOAD_ERROR;
        }

//        DEBUG((D_LOAD, "LoadPe: Section %d, loaded at %x\n", Index, Base));

        //
        // Read the section
        //
 
        if (Section->SizeOfRawData) {
            Status = Ia32EmbImageRead (FHand, Section->PointerToRawData, Section->SizeOfRawData, Base);
            if (EFI_ERROR(Status)) {
*(UINT8 *)(0x000b8000+34) = 'M';
                return Status;
            }
        }

        //
        // If raw size is less then virt size, zero fill the remaining
        //

        if (Section->SizeOfRawData < Section->Misc.VirtualSize) {
            EFILDRZeroMem (
                Base + Section->SizeOfRawData, 
                Section->Misc.VirtualSize - Section->SizeOfRawData
                );
        }

        //
        // Next Section
        //

        Section += 1;
    }

    //
    // Copy in CodeView information if it exists
    //
    if (CodeViewSize != 0) {
      Status = Ia32EmbImageRead (FHand, CodeViewFileOffset, CodeViewSize, Image->ImageBase + CodeViewOffset);
      DebugEntry->RVA = (UINT32) (CodeViewOffset);
    }

    //
    // Apply relocations only if needed
    //
    if((UINTN)(Image->ImageBase) != (UINTN) (PeHdr.OptionalHeader.ImageBase)) {
        Status = Ia32EmbLoadPeRelocate (
                Image,
                &PeHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC],
                (UINTN) Image->ImageBase - PeHdr.OptionalHeader.ImageBase,
                NumberOfMemoryMapEntries,
                EfiMemoryDescriptor
                );

        if (EFI_ERROR(Status)) {
*(UINT8 *)(0x000b8000+36) = 'N';
            return Status;
        }
    }

    //
    // Use exported EFI specific interface if present, else use the image's entry point
    //
    Image->EntryPoint = (EFI_IMAGE_ENTRY_POINT) 
                            (Ia32EmbImageAddress(
                                Image, 
                                PeHdr.OptionalHeader.AddressOfEntryPoint
                                ));

    return Status;
}

#define ALIGN_POINTER(p,s)  ((VOID *) (p + ((s - ((UINTN)p)) & (s-1))))

STATIC
EFI_STATUS
Ia32EmbLoadPeRelocate (
    IN EFILDR_LOADED_IMAGE      *Image,
    IN IMAGE_DATA_DIRECTORY     *RelocDir,
    IN UINTN                     Adjust,
    IN UINT32                   *NumberOfMemoryMapEntries,
    IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
    )
{
    IMAGE_BASE_RELOCATION       *RelocBase, *RelocBaseEnd;
    UINT16                      *Reloc, *RelocEnd;
    CHAR8                       *Fixup, *FixupBase;
    UINT16                      *F16;
    UINT32                      *F32;
    CHAR8                       *FixupData;
    UINTN                       NoFixupPages;

    //
    // Find the relocation block
    //

    RelocBase = Ia32EmbImageAddress(Image, RelocDir->VirtualAddress);
    RelocBaseEnd = Ia32EmbImageAddress(Image, RelocDir->VirtualAddress + RelocDir->Size);
    if (!RelocBase || !RelocBaseEnd) {
*(UINT8 *)(0x000b8000+22) = 'O';
        return EFI_LOAD_ERROR;
    }

    NoFixupPages = (RelocDir->Size / sizeof(UINT16) * sizeof(UINTN) + EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT;
    Image->FixupData = (CHAR8*) FindSpace (NoFixupPages, NumberOfMemoryMapEntries, EfiMemoryDescriptor);
    if (!Image->FixupData) {
        return EFI_OUT_OF_RESOURCES;
    }
    EfiAddMemoryDescriptor(NumberOfMemoryMapEntries,
           EfiMemoryDescriptor,
           EfiBootServicesData,
           (UINTN)Image->FixupData,
           NoFixupPages,
           EFI_MEMORY_WB);


    //
    // Run the whole relocation block
    //

    FixupData = Image->FixupData;
    while (RelocBase < RelocBaseEnd) {
           
        Reloc = (UINT16 *) ((CHAR8 *) RelocBase + sizeof(IMAGE_BASE_RELOCATION));
        RelocEnd = (UINT16 *) ((CHAR8 *) RelocBase + RelocBase->SizeOfBlock);
        FixupBase = Ia32EmbImageAddress (Image, RelocBase->VirtualAddress);
        if ((CHAR8 *) RelocEnd < Image->ImageBase || (CHAR8 *) RelocEnd > Image->ImageEof) {
*(UINT8 *)(0x000b8000+22) = 'P';
            return EFI_LOAD_ERROR;
        }

        //
        // Run this relocation record
        //

        while (Reloc < RelocEnd) {

            Fixup = FixupBase + (*Reloc & 0xFFF);
            switch ((*Reloc) >> 12) {

            case IMAGE_REL_BASED_ABSOLUTE:
                break;

            case IMAGE_REL_BASED_HIGH:
                F16 = (UINT16 *) Fixup;
                *F16 = (*F16 << 16) + (UINT16) Adjust;
                *(UINT16 *) FixupData = *F16;
                FixupData = FixupData + sizeof(UINT16);
                break;

            case IMAGE_REL_BASED_LOW:
                F16 = (UINT16 *) Fixup;
                *F16 = *F16 + (UINT16) Adjust;
                *(UINT16 *) FixupData = *F16;
                FixupData = FixupData + sizeof(UINT16);
                break;

            case IMAGE_REL_BASED_HIGHLOW:
                F32 = (UINT32 *) Fixup;
                *F32 = *F32 + (UINT32) Adjust;
                FixupData = ALIGN_POINTER(FixupData, sizeof(UINT32));
                *(UINT32 *) FixupData = *F32;
                FixupData = FixupData + sizeof(UINT32);
                break;

            case IMAGE_REL_BASED_HIGHADJ:
                BREAKPOINT();                 // BUGBUG: not done
                break;

            default:
//                DEBUG((D_LOAD|D_ERROR, "PeRelocate: unknown fixed type\n"));
*(UINT8 *)(0x000b8000+22) = 'Q';
                return EFI_LOAD_ERROR;
            }

            // Next reloc record
            Reloc += 1;
        }

        // next reloc block
        RelocBase = (IMAGE_BASE_RELOCATION *) RelocEnd;
    }

    return EFI_SUCCESS;
}


STATIC
VOID
Ia32EmbConvertPeImage (
    IN EFILDR_LOADED_IMAGE      *Image,
    IN UINT32                   DescriptorCount,
    IN UINT32                   DescriptorSize,
    IN CHAR8                    *MemoryMap
    )
/*  Reapply fixups to move an image to a new address */
{
    IMAGE_DOS_HEADER            *DosHdr;
    IMAGE_NT_HEADERS            *PeHdr;
    IMAGE_DATA_DIRECTORY        *RelocDir;
    IMAGE_BASE_RELOCATION       *RelocBase, *RelocBaseEnd;
    UINT16                      *Reloc, *RelocEnd;
    CHAR8                       *Fixup, *FixupBase;
    UINT16                      *F16;
    UINT32                      *F32;
    CHAR8                       *FixupData;
    UINTN                       Adjust;
    EFI_MEMORY_DESCRIPTOR       *Descriptor;

    for (Adjust = 0; Adjust < DescriptorCount; ++Adjust) {
        Descriptor = (VOID*)(MemoryMap + Adjust * DescriptorSize);

        if (Descriptor->PhysicalStart < Image->ImageBasePage + (Image->NoPages << EFI_PAGE_SHIFT)
           && Descriptor->PhysicalStart + EFILDRLShiftU64(Descriptor->NumberOfPages, EFI_PAGE_SHIFT) > Image->ImageBasePage
		 && (Descriptor->Attribute & EFI_MEMORY_RUNTIME))
            break;
    }
    if (Adjust >= DescriptorCount) {
        BREAKPOINT();
        return;
    }

    Adjust = (UINTN) (Descriptor->VirtualStart - Descriptor->PhysicalStart);

    /* 
     *  Find the image's relocate dir info
     */

    DosHdr = (IMAGE_DOS_HEADER *) Image->ImageBase;
    PeHdr = (IMAGE_NT_HEADERS *) (((CHAR8 *) DosHdr) + DosHdr->e_lfanew);
    RelocDir = &PeHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    RelocBase = Ia32EmbImageAddress(Image, RelocDir->VirtualAddress);
    RelocBaseEnd = Ia32EmbImageAddress(Image, RelocDir->VirtualAddress + RelocDir->Size);

    /* 
     *  Run the whole relocation block
     */

    FixupData = Image->FixupData;
    while (RelocBase < RelocBaseEnd) {
           
        Reloc = (UINT16 *) ((CHAR8 *) RelocBase + sizeof(IMAGE_BASE_RELOCATION));
        RelocEnd = (UINT16 *) ((CHAR8 *) RelocBase + RelocBase->SizeOfBlock);
        FixupBase = Ia32EmbImageAddress (Image, RelocBase->VirtualAddress);

        /* 
         *  Run this relocation record
         */

        while (Reloc < RelocEnd) {

            Fixup = FixupBase + (*Reloc & 0xFFF);
            switch ((*Reloc) >> 12) {

            case IMAGE_REL_BASED_ABSOLUTE:
                break;

            case IMAGE_REL_BASED_HIGH:
                F16 = (UINT16 *) Fixup;
                if (*(UINT16 *) FixupData == *F16) {
                    *F16 = (*F16 << 16) + (UINT16) Adjust;
                }
                FixupData = FixupData + sizeof(UINT16);
                break;

            case IMAGE_REL_BASED_LOW:
                F16 = (UINT16 *) Fixup;
                if (*(UINT16 *) FixupData == *F16) {
                    *F16 = *F16 + (UINT16) Adjust;
                }
                FixupData = FixupData + sizeof(UINT16);
                break;

            case IMAGE_REL_BASED_HIGHLOW:
                F32 = (UINT32 *) Fixup;
                FixupData = ALIGN_POINTER(FixupData, sizeof(UINT32));
                if (*(UINT32 *) FixupData == *F32) {
                    *F32 = *F32 + (UINT32) Adjust;
                }
                FixupData = FixupData + sizeof(UINT32);
                break;

            case IMAGE_REL_BASED_HIGHADJ:
                BREAKPOINT();                 /*  BUGBUG: not done */
                break;

            default:
                BREAKPOINT();                 /*  BUGBUG: not done */
                return;
            }

            /*  Next reloc record */
            Reloc += 1;
        }

        /*  next reloc block */
        RelocBase = (IMAGE_BASE_RELOCATION *) RelocEnd;
    }
}


STATIC
EFI_STATUS
Ia32EmbImageRead (
    IN VOID                 *FHand,
    IN UINTN                Offset,
    IN OUT UINTN            ReadSize,
    OUT VOID                *Buffer
    )
// Load some data from the image
{
    EFILDRCopyMem(Buffer,(VOID *)((UINT32)FHand + Offset),ReadSize);

    return EFI_SUCCESS;
}


STATIC
VOID *
Ia32EmbImageAddress (
    IN EFILDR_LOADED_IMAGE     *Image,
    IN UINTN                   Address
    )
// Convert an image address to the loaded address
{
    CHAR8        *p;

    p = Image->ImageAdjust + Address;

    if (p < Image->ImageBase || p > Image->ImageEof) {
//        DEBUG((D_LOAD|D_ERROR, "Ia32EmbImageAddress: pointer is outside of image\n"));
        p = NULL;
    }

//    DEBUG((D_LOAD, "Ia32EmbImageAddress: ImageBase %x, ImageEof %x, Address %x, p %x\n", 
//                                Image->ImageBase, Image->ImageEof,
//                                Address, p));
    return p;
}


INTERNAL
EFI_STATUS
Ia32EmbSetImageType (
    IN OUT EFILDR_LOADED_IMAGE      *Image,
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
Ia32EmbCheckImageMachineType (
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

#if EFI_FCODE
    if (MachineType == EFI_IMAGE_MACHINE_FCODE) {
        Status = EFI_SUCCESS;
    }
#endif

    return Status;
}

EFI_STATUS
EfiAddMemoryDescriptor(
    UINTN                 *NoDesc,
    EFI_MEMORY_DESCRIPTOR *Desc,
    EFI_MEMORY_TYPE       Type,
    EFI_PHYSICAL_ADDRESS  BaseAddress,
    UINTN                 NoPages,
    UINT64                Attribute
    )

{
    UINTN i;
    UINTN Temp;
    UINTN Index;

    if (NoPages == 0) {
        return EFI_SUCCESS;
    }

    //
    // See if the new memory descriptor needs to be carved out of an existing memory descriptor
    //

    Index = *NoDesc;
    for(i=0;i<Index;i++) {

        if (Desc[i].Type == EfiConventionalMemory) {

            Temp = ((UINT32)(BaseAddress - Desc[i].PhysicalStart) / EFI_PAGE_SIZE) + NoPages;

            if (Desc[i].PhysicalStart < BaseAddress && Desc[i].NumberOfPages >= Temp) {
                if (Desc[i].NumberOfPages > Temp) {
                    Desc[*NoDesc].Type          = EfiConventionalMemory;
                    Desc[*NoDesc].PhysicalStart = BaseAddress + (NoPages * EFI_PAGE_SIZE);
                    Desc[*NoDesc].NumberOfPages = Desc[i].NumberOfPages - Temp;
                    Desc[*NoDesc].VirtualStart  = 0;
                    Desc[*NoDesc].Attribute     = Desc[i].Attribute;
                    *NoDesc = *NoDesc + 1;
                }
                Desc[i].NumberOfPages = Temp - NoPages;
            }

            if (Desc[i].PhysicalStart == BaseAddress && Desc[i].NumberOfPages==NoPages) {
                Desc[i].Type      = Type;
                Desc[i].Attribute = Attribute;
                return EFI_SUCCESS;
            }

            if (Desc[i].PhysicalStart == BaseAddress && Desc[i].NumberOfPages>NoPages) {
                Desc[i].NumberOfPages -= NoPages;
                Desc[i].PhysicalStart += NoPages * EFI_PAGE_SIZE;
            }
        }
    }

    //
    // Add the new memory descriptor
    //

    Desc[*NoDesc].Type          = Type;
    Desc[*NoDesc].PhysicalStart = BaseAddress;
    Desc[*NoDesc].NumberOfPages = NoPages;
    Desc[*NoDesc].VirtualStart  = 0;
    Desc[*NoDesc].Attribute     = Attribute;
    *NoDesc = *NoDesc + 1;

    return(EFI_SUCCESS);
}

UINTN
FindSpace(
    UINTN                       NoPages,
    IN UINT32                   *NumberOfMemoryMapEntries,
    IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
    )
{
    EFI_PHYSICAL_ADDRESS        MaxPhysicalStart;
    UINT64                      MaxNoPages;
    UINTN                       Index;

    MaxPhysicalStart = 0;
    for (Index = 0;Index < *NumberOfMemoryMapEntries; Index++) {
        if (EfiMemoryDescriptor[Index].Type == EfiConventionalMemory && 
            EfiMemoryDescriptor[Index].NumberOfPages >= NoPages) {
            if (EfiMemoryDescriptor[Index].PhysicalStart > MaxPhysicalStart) {
		      if (EfiMemoryDescriptor[Index].PhysicalStart + EFILDRLShiftU64(EfiMemoryDescriptor[Index].NumberOfPages, EFI_PAGE_SHIFT) <= 0x100000000) {
                    MaxPhysicalStart = EfiMemoryDescriptor[Index].PhysicalStart;
                    MaxNoPages       = EfiMemoryDescriptor[Index].NumberOfPages;
		      }
		      else if (EfiMemoryDescriptor[Index].PhysicalStart + (NoPages << EFI_PAGE_SHIFT) <= 0x100000000) {
                    MaxPhysicalStart = 0x100000000 - (NoPages << EFI_PAGE_SHIFT);
                    MaxNoPages       = NoPages;
		      }
            }
        }
    }
    if (!MaxPhysicalStart)
        return 0;
    return (UINTN)(MaxPhysicalStart + EFILDRLShiftU64(MaxNoPages - NoPages, EFI_PAGE_SHIFT));
}


UINT64
EFILDRLShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  )
// Left shift 64bit by 32bit and get a 64bit result
{
  UINT64      Result;

  _asm {
    mov     eax, dword ptr Operand[0]
    mov     edx, dword ptr Operand[4]
    mov     ecx, Count
    and     ecx, 63

    shld    edx, eax, cl
    shl     eax, cl

    cmp     ecx, 32
    jc      short ls10

    mov     edx, eax
    xor     eax, eax

ls10:
    mov     dword ptr Result[0], eax
    mov     dword ptr Result[4], edx
  }

  return Result;
}

VOID
EFILDRZeroMem (
    IN VOID     *Buffer,
    IN UINTN    Size
    )
{
    UINT8       *pt;

    pt = Buffer;
    while (Size--) {
        *(pt++) = 0;
    }
}

VOID
EFILDRCopyMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    )
{
    CHAR8    *d, *s;

    d = Dest;
    s = Src;
    while (len--) {
        *(d++) = *(s++);
    }
}

UINTN
EFILDRstrcmpa (
    IN CHAR8    *s1,
    IN CHAR8    *s2
    )
// compare strings
{
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return *s1 - *s2;
}

VOID ClearScreen()

{
    UINT32 i;

    Cursor =  (UINT8 *)(0x000b8000 + 160);
    for(i=0;i<80*49;i++) {
        *Cursor = ' ';
        Cursor += 2;
    }
    Cursor =  (UINT8 *)(0x000b8000 + 160);
}


VOID PrintValue(UINT32 Value)

{
    UINT32 i;
    UINT8  ch;

    for(i=0;i<8;i++) {
        ch = (UINT8)((Value >> ((7-i)*4)) & 0x0f) + '0';
        if (ch>'9') {
            ch = ch - '0' -10 + 'A';
        }
        *Cursor = ch;
        Cursor += 2;
    }
}

VOID PrintString(UINT8 *String)

{
    UINT32 i;

    for(i=0;String[i]!=0;i++) {
        if (String[i] == '\n') {
            Cursor = (UINT8 *)(0xb8000 + (((((UINT32)Cursor-0xb8000) + 160) / 160) * 160));
        } else {
            *Cursor = String[i];
            Cursor += 2;
        }
    }
}

//
// EFI Decompression Algorithm code
//

VOID
Ia32EmbFillBuf (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        NumOfBits
  )
/*++

Routine Description:

  Shift mBitBuf NumOfBits left. Read in NumOfBits of bits from source.

Arguments:

  Sd        - The global scratch data
  NumOfBit  - The number of bits to shift and read.

Returns: (VOID)

--*/
{
  Sd->mBitBuf = (UINT16)(Sd->mBitBuf << NumOfBits);

  while (NumOfBits > Sd->mBitCount) {

    Sd->mBitBuf |= (UINT16)(Sd->mSubBitBuf << 
      (NumOfBits = (UINT16)(NumOfBits - Sd->mBitCount)));

    if (Sd->mCompSize > 0) {

      //
      // Get 1 byte into SubBitBuf
      //
      Sd->mCompSize --;
      Sd->mSubBitBuf = 0;
      Sd->mSubBitBuf = Sd->mSrcBase[Sd->mInBuf ++];
      Sd->mBitCount = 8;

    } else {

      Sd->mSubBitBuf = 0;
      Sd->mBitCount = 8;

    }
  }

  Sd->mBitCount = (UINT16)(Sd->mBitCount - NumOfBits);  
  Sd->mBitBuf |= Sd->mSubBitBuf >> Sd->mBitCount;
}


UINT16
Ia32EmbGetBits(
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16    NumOfBits
  )
/*++

Routine Description:

  Get NumOfBits of bits out from mBitBuf. Fill mBitBuf with subsequent 
  NumOfBits of bits from source. Returns NumOfBits of bits that are 
  popped out.

Arguments:

  Sd            - The global scratch data.
  NumOfBits     - The number of bits to pop and read.

Returns:

  The bits that are popped out.

--*/
{
  UINT16  OutBits;

  OutBits = (UINT16)(Sd->mBitBuf >> (BITBUFSIZ - NumOfBits));

  Ia32EmbFillBuf (Sd, NumOfBits);

  return  OutBits;
}


UINT16
Ia32EmbMakeTable (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16      NumOfChar,
  IN  UINT8       *BitLen,
  IN  UINT16      TableBits,
  OUT UINT16       *Table
  )
/*++

Routine Description:

  Creates Huffman Code mapping table according to code length array.

Arguments:

  Sd        - The global scratch data
  NumOfChar - Number of symbols in the symbol set
  BitLen    - Code length array
  TableBits - The width of the mapping table
  Table     - The table
  
Returns:
  
  0         - OK.
  BAD_TABLE - The table is corrupted.

--*/
{
  UINT16  Count[17];
  UINT16  Weight[17];
  UINT16  Start[18];
  UINT16   *p;
  UINT16  k;
  UINT16  i;
  UINT16  Len;
  UINT16  Char;
  UINT16  JuBits;
  UINT16  Avail;
  UINT16  NextCode;
  UINT16  Mask;


  for (i = 1; i <= 16; i ++) {
    Count[i] = 0;
  }

  for (i = 0; i < NumOfChar; i++) {
    Count[BitLen[i]]++;
  }

  Start[1] = 0;

  for (i = 1; i <= 16; i ++) {
    Start[i + 1] = (UINT16)(Start[i] + (Count[i] << (16 - i)));
  }

  if (Start[17] != 0) {/*(1U << 16)*/
    return (UINT16)BAD_TABLE;
  }

  JuBits = (UINT16)(16 - TableBits);

  for (i = 1; i <= TableBits; i ++) {
    Start[i] >>= JuBits;
    Weight[i] = (UINT16)(1U << (TableBits - i));
  }

  while (i <= 16) {
    Weight[i++] = (UINT16)(1U << (16 - i));
  }

  i = (UINT16)(Start[TableBits + 1] >> JuBits);

  if (i != 0) {
    k = (UINT16)(1U << TableBits);
    while (i != k) {
      Table[i++] = 0;
    }
  }

  Avail = NumOfChar;
  Mask = (UINT16)(1U << (15 - TableBits));

  for (Char = 0; Char < NumOfChar; Char++) {

    Len = BitLen[Char];
    if (Len == 0) {
      continue;
    }

    NextCode = (UINT16)(Start[Len] + Weight[Len]);

    if (Len <= TableBits) {

      for (i = Start[Len]; i < NextCode; i ++) {
        Table[i] = Char;
      }

    } else {

      k = Start[Len];
      p = &Table[k >> JuBits];
      i = (UINT16)(Len - TableBits);

      while (i != 0) {
        if (*p == 0) {
          Sd->mRight[Avail] = Sd->mLeft[Avail] = 0;
          *p = Avail ++;
        }

        if (k & Mask) {
          p = &Sd->mRight[*p];
        } else {
          p = &Sd->mLeft[*p];
        }

        k <<= 1;
        i --;
      }

      *p = Char;

    }

    Start[Len] = NextCode;
  }
  
  //
  // Succeeds
  //
  return 0;
}


UINT16
Ia32EmbDecodeP (
  IN  SCRATCH_DATA  *Sd
  )
/*++

Routine description:

  Decodes a position value.

Arguments:

  Sd      - the global scratch data

Returns:

  The position value decoded.

--*/
{
  UINT16  Val;
  UINT16  Mask;

  Val = Sd->mPTTable[Sd->mBitBuf >> (BITBUFSIZ - 8)];

  if (Val >= NP) {
    Mask = 1U << (BITBUFSIZ - 1 - 8);

    do {

      if (Sd->mBitBuf & Mask) {
        Val = Sd->mRight[Val];
      } else {
        Val = Sd->mLeft[Val];
      }

      Mask >>= 1;
    } while (Val >= NP);
  }
  
  //
  // Advance what we have read
  //
  Ia32EmbFillBuf (Sd, Sd->mPTLen[Val]);

  if (Val) {
    Val = (UINT16)((1U << (Val - 1)) + Ia32EmbGetBits (Sd, (UINT16)(Val - 1)));
  }
  
  return Val;
}


UINT16
Ia32EmbReadPTLen (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16  nn,
  IN  UINT16  nbit,
  IN  UINT16  Special
  )
/*++

Routine Descriptiion:

  Reads code lengths for the Extra Set or the Position Set

Arguments:

  Sd        - The global scratch data
  nn        - Number of symbols
  nbit      - Number of bits needed to represent nn
  Special   - The special symbol that needs to be taken care of 

Returns:

  0         - OK.
  BAD_TABLE - Table is corrupted.

--*/
{
  UINT16    n;
  UINT16    c;
  UINT16    i;
  UINT16    Mask;

  n = Ia32EmbGetBits (Sd, nbit);

  if (n == 0) {
    c = Ia32EmbGetBits (Sd, nbit);

    for ( i = 0; i < 256; i ++) {
      Sd->mPTTable[i] = c;
    }

    for ( i = 0; i < nn; i++) {
      Sd->mPTLen[i] = 0;
    }

    return 0;
  }

  i = 0;

  while (i < n) {

    c = (UINT16)(Sd->mBitBuf >> (BITBUFSIZ - 3));

    if (c == 7) {
      Mask = 1U << (BITBUFSIZ - 1 - 3);
      while (Mask & Sd->mBitBuf) {
        Mask >>= 1;
        c += 1;
      }
    }

    Ia32EmbFillBuf (Sd, (UINT16)((c < 7) ? 3 : c - 3));

    Sd->mPTLen [i++] = (UINT8)c;

    if (i == Special) {
      c = Ia32EmbGetBits (Sd, 2);
      while ((INT16)(--c) >= 0) {
        Sd->mPTLen[i++] = 0;
      }
    }
  }

  while (i < nn) {
    Sd->mPTLen [i++] = 0;
  }

  return ( Ia32EmbMakeTable (Sd, nn, Sd->mPTLen, 8, Sd->mPTTable) );
}


VOID
Ia32EmbReadCLen (
  SCRATCH_DATA  *Sd
  )
/*++

Routine Description:

  Reads code lengths for Char&Len Set.

Arguments:

  Sd    - the global scratch data

Returns: (VOID)

--*/
{
  UINT16    n;
  UINT16    c;
  UINT16    i;
  UINT16    Mask;

  n = Ia32EmbGetBits(Sd, CBIT);

  if (n == 0) {
    c = Ia32EmbGetBits(Sd, CBIT);

    for (i = 0; i < NC; i ++) {
      Sd->mCLen[i] = 0;
    }

    for (i = 0; i < 4096; i ++) {
      Sd->mCTable[i] = c;
    }

    return;
  }

  i = 0;
  while (i < n) {

    c = Sd->mPTTable[Sd->mBitBuf >> (BITBUFSIZ - 8)];
    if (c >= NT) {
      Mask = 1U << (BITBUFSIZ - 1 - 8);

      do {

        if (Mask & Sd->mBitBuf) {
          c = Sd->mRight [c];
        } else {
          c = Sd->mLeft [c];
        }

        Mask >>= 1;

      }while (c >= NT);
    }

    //
    // Advance what we have read
    //
    Ia32EmbFillBuf (Sd, Sd->mPTLen[c]);

    if (c <= 2) {

      if (c == 0) {
        c = 1;
      } else if (c == 1) {
        c = (UINT16)(Ia32EmbGetBits (Sd, 4) + 3);
      } else if (c == 2) {
        c = (UINT16)(Ia32EmbGetBits (Sd, CBIT) + 20);
      }

      while ((INT16)(--c) >= 0) {
        Sd->mCLen[i++] = 0;
      }

    } else {

      Sd->mCLen[i++] = (UINT8)(c - 2);

    }
  }

  while (i < NC) {
    Sd->mCLen[i++] = 0;
  }

  Ia32EmbMakeTable (Sd, NC, Sd->mCLen, 12, Sd->mCTable);

  return;
}


UINT16
Ia32EmbDecodeC (
  SCRATCH_DATA  *Sd
  )
/*++

Routine Description:

  Decode a character/length value.

Arguments:

  Sd    - The global scratch data.

Returns:

  The value decoded.

--*/
{
  UINT16      j;
  UINT16      Mask;

  if (Sd->mBlockSize == 0) {

    //
    // Starting a new block
    //

    Sd->mBlockSize = Ia32EmbGetBits(Sd, 16);
    Sd->mBadTableFlag = Ia32EmbReadPTLen (Sd, NT, TBIT, 3);
    if (Sd->mBadTableFlag != 0) {
      return 0;
    }

    Ia32EmbReadCLen (Sd);

    Sd->mBadTableFlag = Ia32EmbReadPTLen (Sd, NP, PBIT, (UINT16)(-1));
    if (Sd->mBadTableFlag != 0) {
      return 0;
    }
  }

  Sd->mBlockSize --;
  j = Sd->mCTable[Sd->mBitBuf >> (BITBUFSIZ - 12)];

  if (j >= NC) {
    Mask = 1U << (BITBUFSIZ - 1 - 12);

    do {
      if (Sd->mBitBuf & Mask) {
        j = Sd->mRight[j];
      } else {
        j = Sd->mLeft[j];
      }

      Mask >>= 1;
    } while (j >= NC);
  }

  //
  // Advance what we have read
  //
  Ia32EmbFillBuf(Sd, Sd->mCLen[j]);

  return j;
}


VOID
Ia32EmbDecode (
  SCRATCH_DATA  *Sd,
  UINT16        NumOfBytes
  )
 /*++

Routine Description:

  Decode NumOfBytes and put the resulting data at starting point of mBuffer.
  The buffer is circular.

Arguments:

  Sd            - The global scratch data
  NumOfBytes    - Number of bytes to decode

Returns: (VOID)

 --*/
{
  UINT16      di;
  UINT16      r;
  UINT16      c;
  
  r = 0;
  di = 0;

  Sd->mBytesRemain --;
  while ((INT16)(Sd->mBytesRemain) >= 0) {
    Sd->mBuffer[di++] = Sd->mBuffer[Sd->mDataIdx++];
    
    if (Sd->mDataIdx >= WNDSIZ) {
      Sd->mDataIdx -= WNDSIZ;
    }

    r ++;
    if (r >= NumOfBytes) {
      return;
    }
    Sd->mBytesRemain --;
  }

  for (;;) {
    c = Ia32EmbDecodeC (Sd);
    if (Sd->mBadTableFlag != 0) {
      return;
    }

    if (c < 256) {

      //
      // Process an Original character
      //

      Sd->mBuffer[di++] = (UINT8)c;
      r ++;
      if (di >= WNDSIZ) {
        return;
      }

    } else {

      //
      // Process a Pointer
      //

      c = (UINT16)(c - (UINT8_MAX + 1 - THRESHOLD));
      Sd->mBytesRemain = c;

      Sd->mDataIdx = (r - Ia32EmbDecodeP(Sd) - 1) & (WNDSIZ - 1); //Make circular

      di = r;
      
      Sd->mBytesRemain --;
      while ((INT16)(Sd->mBytesRemain) >= 0) {
        Sd->mBuffer[di++] = Sd->mBuffer[Sd->mDataIdx++];
        if (Sd->mDataIdx >= WNDSIZ) {
          Sd->mDataIdx -= WNDSIZ;
        }

        r ++;
        if (di >= WNDSIZ) {
          return;
        }
        Sd->mBytesRemain --;
      }
    }
  }

  return;
}

EFI_STATUS
EFIAPI
Ia32EmbGetInfo (
  IN      EFI_DECOMPRESS_PROTOCOL  *This,
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  OUT     UINT32  *DstSize,
  OUT     UINT32  *ScratchSize
  )
/*++

Routine Description:

  The implementation of EFI_DECOMPRESS_PROTOCOL.GetInfo().

Arguments:

  This        - The protocol instance pointer
  Source      - The source buffer containing the compressed data.
  SrcSize     - The size of source buffer
  DstSize     - The size of destination buffer.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - The size of destination buffer and the size of scratch buffer are successull retrieved.
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
{
  UINT8 *Src;

  *ScratchSize = sizeof (SCRATCH_DATA);

  Src = Source;
  if (SrcSize < 8) {
    return EFI_INVALID_PARAMETER;
  }
  
  *DstSize = Src[4] + (Src[5] << 8) + (Src[6] << 16) + (Src[7] << 24);
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Ia32EmbDecompress (
  IN      EFI_DECOMPRESS_PROTOCOL  *This,
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32  DstSize,
  IN OUT  VOID   *Scratch,
  IN      UINT32  ScratchSize
  )
/*++

Routine Description:

  The implementation of EFI_DECOMPRESS_PROTOCOL.Decompress().

Arguments:

  This        - The protocol instance pointer
  Source      - The source buffer containing the compressed data.
  SrcSize     - The size of source buffer
  Destination - The destination buffer to store the decompressed data
  DstSize     - The size of destination buffer.
  Scratch     - The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - Decompression is successfull
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
{
  UINT32        Index;
  UINT16        Count;
  UINT32        CompSize;
  UINT32        OrigSize;
  UINT8         *Dst1;
  EFI_STATUS    Status;
  SCRATCH_DATA  *Sd;
  UINT8         *Src;
  UINT8         *Dst;
  
  Status = EFI_SUCCESS;
  Src  = Source;
  Dst  = Destination;
  Dst1 = Dst;
  
  if (ScratchSize < sizeof (SCRATCH_DATA)) {
      return  EFI_INVALID_PARAMETER;
  }
  
  Sd = (SCRATCH_DATA *)Scratch;
  
  if (SrcSize < 8) {
    return EFI_INVALID_PARAMETER;
  }
  
  CompSize = Src[0] + (Src[1] << 8) + (Src[2] << 16) + (Src[3] << 24);
  OrigSize = Src[4] + (Src[5] << 8) + (Src[6] << 16) + (Src[7] << 24);
  
  if (SrcSize < CompSize + 8) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (DstSize != OrigSize) {
    return EFI_INVALID_PARAMETER;
  }
  
  Src = Src + 8;

  for (Index = 0; Index < sizeof(SCRATCH_DATA); Index++) {
    ((UINT8*)Sd)[Index] = 0;
  }  

  Sd->mBytesRemain = (UINT16)(-1);
  Sd->mSrcBase = Src;
  Sd->mDstBase = Dst;
  Sd->mCompSize = CompSize;
  Sd->mOrigSize = OrigSize;

  //
  // Fill the first two bytes
  //
  Ia32EmbFillBuf(Sd, BITBUFSIZ);

  while (Sd->mOrigSize > 0) {

    Count = (UINT16) (WNDSIZ < Sd->mOrigSize? WNDSIZ: Sd->mOrigSize);
    Ia32EmbDecode (Sd, Count);

    if (Sd->mBadTableFlag != 0) {
      //
      // Something wrong with the source
      //
      return EFI_INVALID_PARAMETER;      
    }

    for (Index = 0; Index < Count; Index ++) {
      if (Dst1 < Dst + DstSize) {
        *Dst1++ = Sd->mBuffer[Index];
      } else {
        return EFI_INVALID_PARAMETER;
      }
    }

    Sd->mOrigSize -= Count;
  }

  if (Sd->mBadTableFlag != 0) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    Status = EFI_SUCCESS;
  }  
      
  return  Status;
}

VOID
EfiLdrCallBack (
  UINTN id, 
  UINTN p1, 
  UINTN p2, 
  UINTN p3
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
    switch(id) {
    case EFILDR_CB_VA:
        Ia32EmbConvertPeImage (
             &EfiCoreImage,
             p1,
             p2,
             (VOID*)p3);
    }
}
