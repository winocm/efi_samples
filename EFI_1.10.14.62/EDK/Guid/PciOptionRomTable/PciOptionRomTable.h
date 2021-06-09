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

  PciOptionRomTable.h
    
Abstract:

  GUID and data structure used to describe the list of PCI Option ROMs present in a system.

--*/

#ifndef _PCI_OPTION_ROM_TABLE_GUID_H_

#define EFI_PCI_OPTION_ROM_TABLE_GUID \
  { 0x7462660f, 0x1cbd, 0x48da, 0xad, 0x11, 0x91, 0x71, 0x79, 0x13, 0x83, 0x1c }

extern EFI_GUID gEfiPciOptionRomTableGuid;

typedef struct {
	EFI_PHYSICAL_ADDRESS   RomAddress; 
	EFI_MEMORY_TYPE        MemoryType;
	UINT32                 RomLength; 
	UINT32                 Seg; 
	UINT8                  Bus; 
	UINT8                  Dev; 
	UINT8                  Func; 
	BOOLEAN                ExecutedLegacyBiosImage; 
  BOOLEAN                DontLoadEfiRom;
} EFI_PCI_OPTION_ROM_DESCRIPTOR;

typedef struct {
  UINT64			                   PciOptionRomCount;
  EFI_PCI_OPTION_ROM_DESCRIPTOR	 *PciOptionRomDescriptors;
} EFI_PCI_OPTION_ROM_TABLE;

#endif
