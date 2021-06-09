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

  EfiRom.h
  
Abstract:

  Header file for the EFI ROM generation utility.

Revision History

--*/
#ifndef _EFI_ROM_H_
#define _EFI_ROM_H_


typedef unsigned char     UINT8;
typedef char              INT8;
typedef unsigned short    UINT16;
typedef unsigned int      UINT32;

#define ROM_SIGNATURE     0xAA55
#define PCIDS_SIGNATURE   "PCIR"

#pragma pack(push)
#pragma pack(1)

#define CODE_TYPE_EFI     0x03  // for code-type field in pci data structure

//
// PCI data structure
//
typedef struct 
{
  UINT8           PciDsSig[4];
  UINT16          VendId;
  UINT16          DevId;
  UINT16          VpdOff;
  UINT16          Size;
  UINT8           Rev;
  UINT8           ClassCode[3];
  UINT16          ImageLen;
  UINT16          RevLvl;
  UINT8           CodeType;
  UINT8           Indicator;
  UINT16          Rsvd;
} PCI_DATA_STRUCTURE;

//
// Architecture unique data format in our option ROM header.
//
typedef struct
{
  UINT16          Size;
  UINT32          HeaderSig;
  UINT16          SubSystem;
  UINT16          MachineType;
  UINT8           Reserved[10];
  UINT16          EfiOffset;
} ARCH_DATA;

//
// Standard PCI expansion ROM header as defined in the PCI Local bus 
// specification Rev. 2.2.
//
typedef struct 
{
  UINT16          RomSig;
  ARCH_DATA       ArchData;
  UINT16          PciDsOff;
  UINT8           Padding[2];    // to align on 4-byte offset
} OPTION_ROM_HEADER;


#pragma pack(pop)

//
// Define structures of a PE32 image so we can get the machine type from the image.
//

// From the EFI 1.0 tree file pe.h
typedef struct {      // DOS .EXE header
    UINT16   e_magic;                     // Magic number
    UINT16   e_cblp;                      // Bytes on last page of file
    UINT16   e_cp;                        // Pages in file
    UINT16   e_crlc;                      // Relocations
    UINT16   e_cparhdr;                   // Size of header in paragraphs
    UINT16   e_minalloc;                  // Minimum extra paragraphs needed
    UINT16   e_maxalloc;                  // Maximum extra paragraphs needed
    UINT16   e_ss;                        // Initial (relative) SS value
    UINT16   e_sp;                        // Initial SP value
    UINT16   e_csum;                      // Checksum
    UINT16   e_ip;                        // Initial IP value
    UINT16   e_cs;                        // Initial (relative) CS value
    UINT16   e_lfarlc;                    // File address of relocation table
    UINT16   e_ovno;                      // Overlay number
    UINT16   e_res[4];                    // Reserved words
    UINT16   e_oemid;                     // OEM identifier (for e_oeminfo)
    UINT16   e_oeminfo;                   // OEM information; e_oemid specific
    UINT16   e_res2[10];                  // Reserved words
    UINT32   e_lfanew;                    // File address of new exe header
} COFF_MSDOS_STUB;

/******************************************************************************
    COFF SIGNATURE SECTION TYPE DEFINITIONS
*******************************************************************************/
// The MS-DOS stub is followed by a 4-byte signature. It must be
// "PE\0\0" to identify the image as a PE image.
typedef struct {
  UINT8       Signature[4];
} COFF_SIGNATURE;

/******************************************************************************
    COFF FILE HEADER SECTION TYPE DEFINITIONS
*******************************************************************************/
typedef struct {
	UINT16			Machine;
	UINT16			NumberOfSections;
	UINT32			TimeDateStamp;
	UINT32			PointerToSymbolTable;
	UINT32			NumberOfSymbols;
	UINT16			SizeOfOptionalHeader;
	UINT16			Characteristics;
} COFF_FILE_HEADER;

/******************************************************************************
    COFF OPTIONAL HEADER TYPE DEFINITIONS
*******************************************************************************/
typedef struct {
  UINT32      VirtualAddress;
  UINT32      Size;
} IMAGE_DATA_DIRECTORY;

// The COFF file header is followed by the optional header (for executable images)
typedef struct {
  // Standard fields
	UINT16			Magic;
	UINT8				MajorLinkerVersion;
	UINT8				MinorLinkerVersion;
	UINT32			SizeOfCode;
	UINT32			SizeOfInitializedData;
	UINT32			SizeOfUninitializedData;
	UINT32			AddressOfEntryPoint;
	UINT32			BaseOfCode;
  UINT32      BaseOfData;

  // NT additional fields
  UINT32      ImageBase;
  UINT32      SectionAlignment;
  UINT32      FileAlignment;
  UINT16      MajorOperatingSystemVersion;
  UINT16      MinorOperatingSystemVersion;
  UINT16      MajorImageVersion;
  UINT16      MinorImageVersion;
  UINT16      MajorSubsystemVersion;
  UINT16      MinorSubsystemVersion;
  UINT32      Reserved;
  UINT32      SizeOfImage;
  UINT32      SizeOfHeaders;
  UINT32      CheckSum;
  UINT16      Subsystem;
  UINT16      DLLCharacteristics;
  UINT32      SizeOfStackReserve;
  UINT32      SizeOfStackCommit;
  UINT32      SizeOfHeapReserve;
  UINT32      SizeOfHeapCommit;
  UINT32      LoaderFlags;
  UINT32      NumberOfRvaAndSizes;

  // Data directories
  IMAGE_DATA_DIRECTORY DataDirectory[16];  
} COFF_OPTIONAL_HEADER;


#endif // #ifndef _EFI_ROM_H_
