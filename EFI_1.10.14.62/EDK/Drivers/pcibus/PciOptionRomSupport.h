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

  PciOptionRomSupport.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/


#ifndef _EFI_PCI_OP_ROM_SUPPORT_H
#define _EFI_PCI_OP_ROM_SUPPORT_H

EFI_STATUS
GetOpRomInfo(
  IN PCI_IO_DEVICE    *PciIoDevice
);

EFI_STATUS
LoadOpRomImage(
  IN PCI_IO_DEVICE   *PciDevice,
  IN UINT64          ReservedMemoryBase
);

EFI_STATUS
ProcessOpRomImage (
  PCI_IO_DEVICE   *PciDevice
);



#endif