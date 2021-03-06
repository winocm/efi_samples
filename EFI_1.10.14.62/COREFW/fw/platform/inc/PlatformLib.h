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

    PlatformLib.h

Abstract:


Revision History

--*/

EFI_STATUS
ConnectDevicePath (
  EFI_DEVICE_PATH  *DevicePathToConnect
  );

EFI_STATUS
ConnectAllConsoles (
  VOID
  );

EFI_STATUS
ConnectAll (
  VOID
  );

VOID
DisconnectAll (
  VOID
  );
