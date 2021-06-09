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

 LoadedImage.c

Abstract:

  EFI 1.0 Loaded Image Protocol definition.

  Every EFI driver and application is passed an image handle when it is loaded.
  This image handle will contain a Loaded Image Protocol.

--*/

#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION(LoadedImage)

EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiLoadedImageProtocolGuid, "LoadedImage Protocol", "EFI 1.0 Loaded Image Protocol");
