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

  pci_class.h

Abstract:

  Supporting routines for EFI shell command "pci"

Revision History

--*/

#ifndef  _PCI_CLASS_H_
#define  _PCI_CLASS_H_


#include "efi.h"  // for UINT32 etc.

#define PCI_CLASS_STRING_LIMIT    54
//
// Printable strings for Pci class code
//
typedef struct {
  CHAR16*    BaseClass;     // Pointer to the PCI base class string
  CHAR16*    SubClass;      // Pointer to the PCI sub class string
  CHAR16*    PIFClass;      // Pointer to the PCI programming interface string
} PCI_CLASS_STRINGS;

//
// a structure holding a single entry, which also points to its lower level
// class
//
typedef struct PCI_CLASS_ENTRY_TAG {
  UINT8                        Code;             // Class, subclass or I/F code
  CHAR16                       *DescText;        // Description string
  struct PCI_CLASS_ENTRY_TAG   *LowerLevelClass; // Subclass or I/F if any
} PCI_CLASS_ENTRY;

VOID
PciGetClassStrings (
  IN      UINT32               ClassCode,
  IN OUT  PCI_CLASS_STRINGS    *ClassStrings
  );

VOID
PciPrintClassCode (
  IN      UINT8                *ClassCodePtr,
  IN      BOOLEAN              IncludePIF
  );
  
#endif // _PCI_CLASS_H_


