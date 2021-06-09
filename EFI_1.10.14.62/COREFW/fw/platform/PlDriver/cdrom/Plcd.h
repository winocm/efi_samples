#ifndef _CD_H
#define _CD_H
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

    cd.h

Abstract:

    Call back to system to find the CD-ROMs in the system

Revision History

--*/

#include "emulenv.h"

//
// Following structure used for getting CDROM info thru platform code
//

typedef struct {
    UINTN  DeviceNumber;
    UINTN  Attributes;
} CDROM_INFO;

VOID
PlGetCDRomInfo (
    OUT UINTN           *Count,
    OUT CDROM_INFO      **Info
    );

#endif