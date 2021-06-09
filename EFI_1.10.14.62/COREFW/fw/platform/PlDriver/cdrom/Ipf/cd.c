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
#include "SalEfi.h" 
#include "SalProc.h"
#include "plcd.h"


VOID
PlGetCDRomInfo (
    OUT UINTN           *Count,
    OUT CDROM_INFO      **Info
    )
/*++

Routine Description:

    This routine gets number of CD's through a SAL callback

Arguments:

    Pointer to number of CD's supported variable
    CDrom info pointer

Returns:
 
    None

NOTE: Caller will need to call FreePool to delete memory allocated
      if non-zero count returned

--*/
{
    rArg    Results = {-3,0,0,0};
    UINTN   BufferSize;


    *Count = 0;
    BufferSize = sizeof(CDROM_INFO)*4;

    while (Results.p0 == -3) {
        //
        // Allocate pool          
        //
        *Info  = (CDROM_INFO *) AllocateZeroPool(BufferSize);        
        ASSERT(*Info);

        //
        // call SAL callback to get info
        //
        SalCallBack(
            ID_SALCB_GETMEDIAINFO, ID_SALCB_GETMEDIAINFO_CDROM, 
            BufferSize, (UINTN)*Info, 0, 0, 0, 0,
            &Results
            );
        
        if(Results.p0 == -3) {
            //
            // check return and delete buffer and try again
            //
            FreePool(*Info);
            BufferSize = Results.p1;
        }
    }


    if(Results.p0 == -1) {
        //
        // temporary fix until support available in sal
        //
        DEBUG ((D_ERROR, "Sal CallBack ID_SALCB_GETMEDIAINFO returned error Hard code 1 CD-ROM"));
        *Count = 1;
        (*Info)->DeviceNumber = 0xff;
        return;
    }
    
    if(!Results.p0) { 
        //
        // update count if success
        //
        *Count = Results.p1;
    } else {
        *Count = 0;
        FreePool (*Info);
    }
}

