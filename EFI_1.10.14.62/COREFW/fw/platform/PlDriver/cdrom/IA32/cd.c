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

    cd.c

Abstract: 

    Call back to platform to get the number of CD-ROMs present.
    This stuff is really messed up on PC's, so that's why we have to ask the BIOS



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "plcd.h"


VOID
PlGetCDRomInfo (
    OUT UINTN           *Count,
    OUT CDROM_INFO      **Info
    )
/*++

Routine Description:

    This routine gets number of CD's, Need to ask the BIOS

Arguments:

    Pointer to number of CD's supported variable
    CDrom info pointer

Returns:
 
    None

NOTE: Caller will need to call FreePool to delete memory allocated
      if non-zero count returned

--*/
{
    UINTN   BufferSize;
    CDROM_INFO  *CdInfo;

    *Count = 0;

    BufferSize = sizeof(CDROM_INFO)*4;
    *Info  = (CDROM_INFO *) AllocateZeroPool(BufferSize);     
    CdInfo = *Info;
    ASSERT(CdInfo);


    //
    // BugBug: The CD thing is bad on a PC. This is just a hack for debug.
    //  Take a guess on where a CD-ROM may be. This is not PC compatible. This
    //  guesses based on stuff I saw on some set of platforms
    //
    *Count = 2;
    CdInfo->DeviceNumber = 0xff;
    CdInfo++;
    CdInfo->DeviceNumber = 0xef;
}

