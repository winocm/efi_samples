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

  pe.c
  
Abstract:

  Routines for managing PE32 images.

--*/

#include "loader.h"
#include "pe.h"

#include EFI_PROTOCOL_DEFINITION(Ebc)

#define MAX_PAGES_FOR_CODEVIEW  4

STATIC
EFI_STATUS
RUNTIMEFUNCTION
LoadPeRelocate (
  IN LOADED_IMAGE             *Image,
  IMAGE_DATA_DIRECTORY        *RelocDir,
  IN UINT64                   Adjust,
  IN BOOLEAN                  RelocsStripped
  );

VOID
RUNTIMEFUNCTION INTERNAL
ConvertPeImage (
    IN LOADED_IMAGE             *Image
    );

STATIC
VOID *
GetPeProcAddress (
    IN LOADED_IMAGE             *Image,
    IMAGE_NT_HEADERS            *PeHdr,
    IN CHAR8                    *Name
    );

STATIC
EFI_STATUS
ImageRead (
    IN SIMPLE_READ_FILE     FHand,
    IN UINTN                Offset,
    IN OUT UINTN            ReadSize,
    OUT VOID                *Buffer
    );

STATIC
RUNTIMEFUNCTION
VOID *
ImageAddress (
    IN LOADED_IMAGE     *Image,
    IN UINTN             Address
    );

STATIC
EFI_STATUS
FlushICache (
  IN EFI_PHYSICAL_ADDRESS     Start,
  IN UINT64                   Length
  );

#ifdef EFI_NT_EMULATOR

EFI_STATUS
WinNtLoadAsDll (
  IN  CHAR8  *PdbFileName,
  IN  VOID   **ImageEntryPoint
  );

#endif
  
//
// Declare runtime functions
//

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(ConvertPeImage)
#pragma RUNTIME_CODE(ImageAddress)
#endif

//
//
//

EFI_STATUS
LoadPeImage (
    IN SIMPLE_READ_FILE         FHand,
    IN LOADED_IMAGE             *Image
    )
{
  IMAGE_DOS_HEADER            DosHdr;
  IMAGE_NT_HEADERS            PeHdr;
  IMAGE_SECTION_HEADER        *FirstSection;
  IMAGE_SECTION_HEADER        *Section;
  UINTN                       Index, NumDebugEntries;
  IMAGE_DATA_DIRECTORY        *DirectoryEntry;
  IMAGE_DEBUG_DIRECTORY_ENTRY *DebugDirectory;
  EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY  *CodeViewEntry;
  EFI_STATUS                  Status;
  CHAR8                       *Base, *End, *MaxEnd, *CodeViewEntryEnd;
  IMAGE_DATA_DIRECTORY        *RelocDir;
  IMAGE_BASE_RELOCATION       *RelocBase, *RelocBaseEnd;
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  EFI_EBC_PROTOCOL            *EbcProtocol;
  BOOLEAN                     RelocsStripped;
#ifdef EFI_NT_EMULATOR
  VOID                        *DllEntryPoint;
#endif
    
  ZeroMem (&DosHdr, sizeof(DosHdr));
  ZeroMem (&PeHdr, sizeof(PeHdr));

  //
  // Read image headers
  //

  ImageRead (FHand, 0, sizeof(IMAGE_DOS_HEADER), &DosHdr);
  if (DosHdr.e_magic != IMAGE_DOS_SIGNATURE) {
      DEBUG ((D_LOAD, "LoadPeImage: Dos header signature not found\n"));
      return EFI_UNSUPPORTED;
  }

  ImageRead (FHand, DosHdr.e_lfanew, sizeof(IMAGE_NT_HEADERS), &PeHdr);

  //
  // Make sure it has a normal PE32 image header
  //

  if (PeHdr.Signature != IMAGE_NT_SIGNATURE) {
      DEBUG ((D_LOAD, "LoadPeImage: PE image header signature not found\n"));
      return EFI_UNSUPPORTED;
  }
  
  //
  // Set the image subsystem type
  //

  Status = SetImageType (Image, PeHdr.OptionalHeader.Subsystem);
  if (EFI_ERROR(Status)) {
      DEBUG ((D_LOAD, "LoadPeImage: Subsystem type not known\n"));
      return Status;
  }

  //
  // Verify machine type
  //

  Status = CheckImageMachineType (PeHdr.FileHeader.Machine);
  if (EFI_ERROR(Status)) {
      DEBUG ((D_LOAD, "LoadPeImage: Incorrect machine type\n"));
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

  Image->NumberOfPages = EFI_SIZE_TO_PAGES (PeHdr.OptionalHeader.SizeOfImage + PeHdr.OptionalHeader.SectionAlignment) + MAX_PAGES_FOR_CODEVIEW;
  (UINTN) (Image->ImageBasePage) = (UINTN)(PeHdr.OptionalHeader.ImageBase);
  Image->Info.ImageSize = ((Image->NumberOfPages) << EFI_PAGE_SHIFT);
  Image->ImageBase = (CHAR8 *) Image->ImageBasePage;
  Image->ImageEof  = Image->ImageBase + Image->Info.ImageSize;
  Image->ImageAdjust = Image->ImageBase;

  //
  // Check to see if relocations are available in this image.
  //
  RelocBase = NULL;
  RelocBaseEnd = NULL;
  if (IMAGE_DIRECTORY_ENTRY_BASERELOC < PeHdr.OptionalHeader.NumberOfRvaAndSizes) {
    RelocDir = &(PeHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]);
    RelocBase = (VOID *)RelocDir->VirtualAddress;
    RelocBaseEnd = (VOID *)(RelocDir->VirtualAddress + RelocDir->Size);
  }
  //
  // Load the image based on whether or not relocation have been stripped
  // from it.
  //
  RelocsStripped = 
    (PeHdr.FileHeader.Characteristics &  IMAGE_FILE_RELOCS_STRIPPED) ? TRUE : FALSE;
  if (RelocsStripped) {
    //
    // Relocations have been stripped, so attempt to load the image at 
    // its linked address. 
    //
    Status = AllocatePages (
                AllocateAddress,
                Image->Info.ImageCodeType, 
                Image->NumberOfPages,
                &Image->ImageBasePage
                );
    Image->Info.ImageBase = (VOID *) Image->ImageBasePage;
    Image->ImageBase = (CHAR8 *) Image->ImageBasePage;
  } else {
    //
    // Relocations have not been stripped so attempt to load the image 
    // near top of memory.
    //
    BaseAddress = MAX_ADDRESS;
    DEBUG ((D_LOAD, "LoadPeImage: Attempting to load image at NON linker assigned address\n"));


    Status = AllocatePages (
                AllocateMaxAddress,
                Image->Info.ImageCodeType, 
                Image->NumberOfPages,
                &BaseAddress
                );

    Image->ImageBasePage = (BaseAddress + PeHdr.OptionalHeader.SectionAlignment - 1) & 
                          ~((UINTN)PeHdr.OptionalHeader.SectionAlignment - 1);
    Image->Info.ImageBase = (VOID *) Image->ImageBasePage;
    Image->ImageBase = (CHAR8 *) Image->ImageBasePage;
    Image->ImageBasePage = BaseAddress;
  }

  if (EFI_ERROR(Status)) {
    Image->ImageBasePage = 0;
    return Status;
  }

  DEBUG((D_LOAD, "LoadPe: new image base %lx\n", Image->ImageBasePage));
  Image->ImageEof  = (CHAR8 *)(Image->ImageBasePage + Image->Info.ImageSize);
  Image->ImageAdjust = Image->ImageBase;

  //
  // Copy the machine type from the context to the image private data. This
  // is needed during image unload to know if we should call an EBC protocol
  // to unload the image.
  //
  Image->Machine = PeHdr.FileHeader.Machine;

  //
  // Copy the Image header to the base location
  //

  Status = ImageRead (
              FHand, 
              0, 
              PeHdr.OptionalHeader.SizeOfHeaders, 
              Image->ImageBase
              );

  if (EFI_ERROR(Status)) {
    return Status;
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
  for (Index=0, MaxEnd = NULL; Index < PeHdr.FileHeader.NumberOfSections; Index += 1) {

      //
      // Compute sections address
      //

      Base = ImageAddress(Image, Section->VirtualAddress);
      End = ImageAddress(Image, Section->VirtualAddress + Section->Misc.VirtualSize - 1);
      if (End > MaxEnd) {
          MaxEnd = End;
      }

      if (EFI_ERROR(Status) || !Base  ||  !End) {
          DEBUG((D_LOAD|D_ERROR, "LoadPe: Section %d was not loaded\n", Index));
          return EFI_LOAD_ERROR;
      }

      DEBUG((D_LOAD, "LoadPe: Section %d, loaded at %x\n", Index, Base));

      //
      // Read the section
      //

      if (Section->SizeOfRawData) {
          Status = ImageRead (FHand, Section->PointerToRawData, Section->SizeOfRawData, Base);
          if (EFI_ERROR(Status)) {
              return Status;
          }
      }

      //
      // If raw size is less then virt size, zero fill the remaining
      //

      if (Section->SizeOfRawData < Section->Misc.VirtualSize) {
          ZeroMem (
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
  // Apply relocations. Call the relocate function even if there are no
  // relocations because that function performs additional checks to guarantee
  // that it is ok to not have them.
  //
  if (IMAGE_DIRECTORY_ENTRY_BASERELOC < PeHdr.OptionalHeader.NumberOfRvaAndSizes) {
    Status = LoadPeRelocate (
            Image,
            &PeHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC],
            (UINT64) Image->ImageBase - PeHdr.OptionalHeader.ImageBase,
            RelocsStripped
            );
  } else {
    Status = LoadPeRelocate (
            Image,
            NULL,
            (UINT64) Image->ImageBase - PeHdr.OptionalHeader.ImageBase,
            RelocsStripped
            );
  }
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // The codeview information can be located nearly anywhere in the file.
  // It is found by looking at the members of the debug directory for a codeview 
  // record. If there is a codeview record that contains an RVA that is not 0, 
  // then the codeview record is contained within a section and has been loaded
  // at the RVA.  If the RVA is zero and the FileOffset is not zero, then the 
  // codeview record is outside of all the sections.  We pre-allocated an extra
  // 4 pages for this contingency (a 16K PDB path seems ridiculous).  We'll read
  // the codeview record from the file image into this extra space.  Whichever
  // is the case, we'll free any unused pages at the end of the image allocatoin.
  //
  CodeViewEntry = NULL;
  if (IMAGE_DIRECTORY_ENTRY_DEBUG < PeHdr.OptionalHeader.NumberOfRvaAndSizes) {
    DirectoryEntry = (VOID *)&(PeHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]);
    DebugDirectory = (VOID *) (Image->ImageBase + DirectoryEntry->VirtualAddress);
    NumDebugEntries = DirectoryEntry->Size / sizeof (IMAGE_DEBUG_DIRECTORY_ENTRY);
    for (Index = 0; Index < NumDebugEntries; Index++) {
      if (DebugDirectory[Index].Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
        if (DebugDirectory[Index].RVA == 0) {
          if (DebugDirectory[Index].FileOffset != 0 && MAX_PAGES_FOR_CODEVIEW >= EFI_SIZE_TO_PAGES (DebugDirectory[Index].SizeOfData)) {
            //
            // The PDB file path is outside the normal image, so copy it into
            // the extra space we allocated beyond the end of the image
            //
            CodeViewEntry = (EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY *)(Image->ImageBase + PeHdr.OptionalHeader.SizeOfImage);
            CodeViewEntryEnd = Image->ImageBase + PeHdr.OptionalHeader.SizeOfImage + DebugDirectory[Index].SizeOfData;
            Status = ImageRead (FHand, 
                                DebugDirectory[Index].FileOffset, 
                                DebugDirectory[Index].SizeOfData, 
                                CodeViewEntry);
            if (EFI_ERROR (Status)) {
              return (Status);
            }
            DebugDirectory[Index].RVA = (UINT32)((UINTN) CodeViewEntry - (UINTN) Image->ImageBase);
          }
        } else {
          CodeViewEntry = (VOID *) ((UINTN) Image->ImageBase + DebugDirectory[Index].RVA);
          CodeViewEntryEnd = Image->ImageBase + PeHdr.OptionalHeader.SizeOfImage;
        }
        break; 
      }
    }
  }
  
  // 
  // We allocated extra space for the image in case we needed to copy in
  // the PDB path.  We need to free any pages that were not used for this
  // purpose and adjust the Image data fields to reflect the new size...
  //
  // BUGBUG : However, freeing the extra pages fragments the memory map with
  // a small chunk of free memory next to every image.  For now, don't free the
  // extra space to keep the memory map from fragmenting.
  //
//  if (CodeViewEntry) {
//    Image->NumberOfPages = EFI_SIZE_TO_PAGES((UINTN)(CodeViewEntryEnd - Image->ImageBasePage)) ;
//    Image->Info.ImageSize = ((Image->NumberOfPages) << EFI_PAGE_SHIFT);
//    BS->FreePages (
//          (EFI_PHYSICAL_ADDRESS)Image->ImageBasePage + Image->Info.ImageSize,
//          EFI_SIZE_TO_PAGES((UINTN)Image->ImageEof - ((UINTN)Image->ImageBasePage + (UINTN)Image->Info.ImageSize))
//          );
//    Image->ImageEof  = (CHAR8 *)(Image->ImageBasePage + Image->Info.ImageSize);
//  }
      
  //
  // Use exported EFI specific interface if present, else use the image's entry point
  //

  DEBUG((D_LOAD|D_WARN, "LoadPe: using PE image entry point\n"));
  Image->EntryPoint = (EFI_IMAGE_ENTRY_POINT) 
                          (ImageAddress(
                              Image, 
                              PeHdr.OptionalHeader.AddressOfEntryPoint
                              ));
  DEBUG((D_LOAD, "LoadPe: Image Entry Point %016x [%016x]\n", Image->EntryPoint, *(UINT64 *)Image->EntryPoint));

  if (PL->FlushCache) {
    //
    // On some machines I-caches do not maintain coherency so you need to 
    //  flush the data that we loaded out of any D-cache.
    //
    PL->FlushCache (Image->ImageBase, MaxEnd);
    DEBUG((D_LOAD, "LoadPe: Cache Flush Done\n"));
  }

  EbcProtocol = NULL;
  if (Image->Machine == EFI_IMAGE_MACHINE_EBC) {
    //
    // Locate the EBC interpreter protocol
    //
    Status = BS->LocateProtocol (&gEfiEbcProtocolGuid, NULL, &EbcProtocol);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    //
    // Register a callback for flushing the instruction cache so that created
    // thunks can be flushed.
    //
    Status = EbcProtocol->RegisterICacheFlush (EbcProtocol, FlushICache);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  
    //
    // Create a thunk for the image's entry point. This will be the new
    // entry point for the image.
    //
    Status = EbcProtocol->CreateThunk ( EbcProtocol,
                                        Image->Handle, 
                                        (VOID *)(UINTN)Image->EntryPoint, 
                                        (VOID **)&Image->EntryPoint);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  if (CodeViewEntry != NULL) {
#ifdef EFI_NT_EMULATOR
    //
    // Load NT DLL if we know the name
    //
    
    if (Image->Machine != EFI_IMAGE_MACHINE_EBC) {
      DllEntryPoint = NULL;
      if(CodeViewEntry->Signature == CODEVIEW_SIGNATURE_RSDS){
        Status = WinNtLoadAsDll ((CHAR8 *)( (EFI_IMAGE_DEBUG_CODEVIEW_RSDS_ENTRY*)CodeViewEntry + 1), &DllEntryPoint);
      }else if(CodeViewEntry->Signature == CODEVIEW_SIGNATURE_NB10){
        Status = WinNtLoadAsDll ((CHAR8 *)(CodeViewEntry + 1), &DllEntryPoint);
      }
      if (!EFI_ERROR(Status) && DllEntryPoint != NULL) {
        Image->EntryPoint = (EFI_IMAGE_ENTRY_POINT)(UINTN)DllEntryPoint;
      }
    }
#endif
  }
  return EFI_SUCCESS;
}

#define ALIGN_POINTER(p,s)  ((VOID *) (p + ((s - ((UINTN)p)) & (s-1))))

STATIC
EFI_STATUS
RUNTIMEFUNCTION
LoadPeRelocate (
  IN LOADED_IMAGE             *Image,
  IN IMAGE_DATA_DIRECTORY     *RelocDir,
  IN UINT64                   Adjust,
  IN BOOLEAN                  RelocsStripped
  )
{
  IMAGE_BASE_RELOCATION       *RelocBase, *RelocBaseEnd;
  UINT16                      *Reloc, *RelocEnd;
  CHAR8                       *Fixup, *FixupBase;
  UINT16                      *F16;
  UINT32                      *F32;
  CHAR8                       *FixupData;
  UINTN                       BufferSize;
  EFI_STATUS                  Status;

  //
  // Find the relocation block
  //

  RelocBase = (VOID *)RelocDir->VirtualAddress;
  RelocBaseEnd = (VOID *)(RelocDir->VirtualAddress + RelocDir->Size);
  
  // 
  // If no relocations, then make sure they weren't stripped.
  //
  if (!RelocDir || !RelocBase || !RelocBaseEnd) {

    if (RelocsStripped) {
      //
      // If the image is a runtime driver, then never load it with relocations
      // stripped. Otherwise, if the link address is the same as the load 
      // address, then it's ok.
      //
      return (Adjust == 0 && Image->Type != IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER) ? EFI_SUCCESS : EFI_LOAD_ERROR;
    }
    //
    // PIC code 
    //
    return EFI_SUCCESS;
  }
  RelocBase = ImageAddress (Image, RelocDir->VirtualAddress);
  RelocBaseEnd = ImageAddress (Image, RelocDir->VirtualAddress + RelocDir->Size);

  //
  // Allocate a buffer large enough for the maximum fixup data size
  //

  Image->FixupData = AllocatePool (RelocDir->Size / sizeof(UINT16) * sizeof(UINTN));
  if (!Image->FixupData) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Run the relocation information and apply the fixups
  //

  Status = EFI_LOAD_ERROR;
  FixupData = Image->FixupData;
  while (RelocBase < RelocBaseEnd) {
         
    Reloc = (UINT16 *) ((CHAR8 *) RelocBase + sizeof(IMAGE_BASE_RELOCATION));
    RelocEnd = (UINT16 *) ((CHAR8 *) RelocBase + RelocBase->SizeOfBlock);
    FixupBase = ImageAddress (Image, RelocBase->VirtualAddress);
    if ((CHAR8 *) RelocEnd < Image->ImageBase || (CHAR8 *) RelocEnd > Image->ImageEof) {
      goto Done;
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
        Status = LoadPeRelocate_Ex (Reloc,Fixup,&FixupData,Adjust);
        if (EFI_ERROR (Status)) {
            goto Done;
        }
      }

      // Next reloc record
      Reloc += 1;
    }

    // next reloc block
    RelocBase = (IMAGE_BASE_RELOCATION *) RelocEnd;
  }

  Status = EFI_SUCCESS;

Done:

  //
  // If the image was loaded re-size the fixup data buffer (into
  // the proper type for the image)
  //

  if (!EFI_ERROR(Status)) {
    BufferSize = (UINTN) (FixupData - Image->FixupData);
    Status = BSAllocatePool(Image->Info.ImageCodeType, BufferSize, &FixupData);
    if (!EFI_ERROR(Status)) {
      CopyMem (FixupData, Image->FixupData, BufferSize);
      FreePool (Image->FixupData);
      Image->FixupData = FixupData;
    }
  }

  return Status;
}


VOID
RUNTIMEFUNCTION INTERNAL
ConvertPeImage (
  IN LOADED_IMAGE             *Image
  )
// Reapply fixups to move an image to a new address
{
  CHAR8                       *OldBase, *NewBase;
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
  EFI_STATUS                  Status;

  OldBase = Image->ImageBase;
  NewBase = Image->ImageBase;
  RT->ConvertPointer (0, &NewBase);

  DEBUG ((D_INIT, "ConvertPeImage: Changing image base from %x to %x\n", OldBase, NewBase));
  Adjust = (UINTN) NewBase - (UINTN) OldBase;

  //
  // Find the image's relocate dir info
  //

  DosHdr = (IMAGE_DOS_HEADER *) Image->ImageBase;
  ASSERT (DosHdr->e_magic == IMAGE_DOS_SIGNATURE);
  PeHdr = (IMAGE_NT_HEADERS *) (((CHAR8 *) DosHdr) + DosHdr->e_lfanew);
  ASSERT (PeHdr->Signature == IMAGE_NT_SIGNATURE);

  if (IMAGE_DIRECTORY_ENTRY_BASERELOC < PeHdr->OptionalHeader.NumberOfRvaAndSizes) {
    RelocDir = &PeHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    RelocBase = ImageAddress(Image, RelocDir->VirtualAddress);
    RelocBaseEnd = ImageAddress(Image, RelocDir->VirtualAddress + RelocDir->Size);
    ASSERT (RelocBase && RelocBaseEnd);
  } else {
    //
    // No relocation directory entry
    //
    RelocBase = NULL;
    RelocBaseEnd = NULL;
  }
  //
  // Run the whole relocation block
  //

  FixupData = Image->FixupData;
  while (RelocBase < RelocBaseEnd) {
         
    Reloc = (UINT16 *) ((CHAR8 *) RelocBase + sizeof(IMAGE_BASE_RELOCATION));
    RelocEnd = (UINT16 *) ((CHAR8 *) RelocBase + RelocBase->SizeOfBlock);
    ASSERT ((CHAR8 *) RelocEnd >= Image->ImageBase && (CHAR8 *) RelocEnd <= Image->ImageEof);
    FixupBase = ImageAddress (Image, RelocBase->VirtualAddress);

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
        BREAKPOINT();                 // BUGBUG: not done
        break;

      default:
        Status = ConvertPeImage_Ex (Reloc,Fixup,&FixupData,Adjust);
        if (EFI_ERROR (Status)) {
            return;
        }
      }

        // Next reloc record
        Reloc += 1;
    }

    // next reloc block
    RelocBase = (IMAGE_BASE_RELOCATION *) RelocEnd;
  }

  if (PL->FlushCache) {
    //
    // On some machines I-caches do not maintain coherency so you need to 
    //  flush the data that we loaded out of any D-cache.
    //
    PL->FlushCache (Image->ImageBase, Image->ImageEof);
  }
}

// BUGBUG -- function no longer used?

STATIC
VOID *
GetPeProcAddress (
  IN LOADED_IMAGE         *Image,
  IN IMAGE_NT_HEADERS     *PeHdr,
  IN CHAR8                *FunctionName
  )
{
  IMAGE_DATA_DIRECTORY        *DataDir;    
  IMAGE_EXPORT_DIRECTORY      *ExportDir, *ExportDirEnd;
  CHAR8                       *Name;
  UINT32                      *Fncs, *Names;
  UINTN                        Index;

  //
  // If we have a directory entry for the exports, search it
  //
  if (IMAGE_DIRECTORY_ENTRY_EXPORT < PeHdr->OptionalHeader.NumberOfRvaAndSizes) {
  
    DataDir = &PeHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    //
    // Locate the export directory table
    //

    ExportDir = ImageAddress (Image, DataDir->VirtualAddress);
    ExportDirEnd = ImageAddress (Image, DataDir->VirtualAddress + DataDir->Size);
    if (!ExportDir || !ExportDirEnd || ExportDir == ExportDirEnd) {
        return NULL;
    }

    //
    // Locate the export function table
    //

    Fncs = ImageAddress (Image, (UINTN) ExportDir->AddressOfFunctions);
    Names = ImageAddress (Image, (UINTN) ExportDir->AddressOfNames);
    if (!Fncs || !Names) {
      return NULL;
    }

    //
    // Scan the export directory looking for a match
    //

    for (Index=0; Index < ExportDir->NumberOfNames; Index++) {
      Name = ImageAddress(Image, Names[Index]);
      if (Name &&  strcmpa (Name, FunctionName) == 0) {
        break;
      }
    }
  
    //
    // If found, return the address of the function
    //

    if (Index < ExportDir->NumberOfNames && Index < ExportDir->NumberOfFunctions) {
      return ImageAddress (Image, Fncs[Index]);
    }
  }
  // 
  // Not found
  //

  return NULL;
}



STATIC
EFI_STATUS
ImageRead (
  IN SIMPLE_READ_FILE     FHand,
  IN UINTN                Offset,
  IN OUT UINTN            ReadSize,
  OUT VOID                *Buffer
  )
// Load some data from the image
{
    UINTN                   OrigReadSize;
    EFI_STATUS              Status;

    //
    // Read the data
    //

    OrigReadSize = ReadSize;
    Status = ReadSimpleReadFile (FHand, Offset, &ReadSize, Buffer);

    //
    // If we didn't get it all, then there is an error
    //

    if (OrigReadSize != ReadSize) {
        Status = EFI_LOAD_ERROR;
    }

    //
    // If there's an error, zero the buffer
    //

    if (EFI_ERROR(Status)) {
        ZeroMem (Buffer, OrigReadSize);
    }
    return Status;
}


STATIC
VOID *
RUNTIMEFUNCTION
ImageAddress (
  IN LOADED_IMAGE     *Image,
  IN UINTN            Address
  )
// Convert an image address to the loaded address
{
  CHAR8        *p;

  p = Image->ImageAdjust + Address;

  if (p < Image->ImageBase || p > Image->ImageEof) {
    DEBUG((D_LOAD|D_ERROR, "ImageAddress: pointer is outside of image\n"));
    p = NULL;
  }

//    DEBUG((D_LOAD, "ImageAddress: ImageBase %x, ImageEof %x, Address %x, p %x\n", 
//                                Image->ImageBase, Image->ImageEof,
//                                Address, p));
  return p;
}

//
// This callback function is used by the EBC interpreter driver to flush the 
// processor instruction cache after creating thunks. We're simply hiding
// the "this" pointer that must be passed into the real flush function.
//
STATIC
EFI_STATUS
FlushICache (
  IN EFI_PHYSICAL_ADDRESS     Start,
  IN UINT64                   Length
  )
{
  if (PL->FlushCache) {
    //
    // On some machines I-caches do not maintain coherency so you need to 
    //  flush the data that we loaded out of any D-cache.
    //
    PL->FlushCache ((VOID *)Start, (CHAR8 *)(Start+Length));
    DEBUG((D_LOAD, "LoadPe: Cache Flush Done\n"));
  }
  return EFI_SUCCESS;
}

   