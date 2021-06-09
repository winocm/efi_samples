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

    ConfigureTiming.c
    
Abstract: 
    Internal functions for ide controller's timing configuration

Revision History
--*/

#include "idebus.h"
#include "ide.h"

EFI_STATUS
IdeConfigureTiming (
  IN  IDE_BLK_IO_DEV    *IdeDevice
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
SetPioMode(
  IN    IDE_BLK_IO_DEV     *IdeDev
    )
{
  return EFI_SUCCESS;
}