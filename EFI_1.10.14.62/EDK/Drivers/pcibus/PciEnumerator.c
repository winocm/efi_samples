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

  PciEnumerator.c
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#include "Pcibus.h"

EFI_STATUS
PciEnumerator (
  IN EFI_HANDLE                    Controller
  )
/*++

Routine Description:

  This routine is used to enumerate entire pci bus system 
  in a given platform

Arguments:

Returns:

  None

--*/
{
  //
  // This PCI bus driver depends on the legacy BIOS
  // to do the resource allocation
  //
  gFullEnumeration = FALSE;

  return PciEnumeratorLight (Controller) ;
  
}




