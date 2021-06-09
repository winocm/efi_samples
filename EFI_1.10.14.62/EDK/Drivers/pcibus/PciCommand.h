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

  PciCmd.c
  
Abstract:

  PCI Bus Driver

Revision History

--*/


#ifndef _EFI_PCI_COMMAND_H
#define _EFI_PCI_COMMAND_H

EFI_STATUS 
PciReadCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT16       *Command
);

  
EFI_STATUS 
PciSetCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciEnableCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciDisableCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciDisableBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);


EFI_STATUS 
PciEnableBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciReadBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT16       *Command
);

#endif

