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

    crc.c

Abstract:

    CRC32 functions



Revision History

--*/

#include "lib.h"


VOID
SetCrc (
    IN OUT EFI_TABLE_HEADER *Hdr
    )
/*++

Routine Description:

    Updates the CRC32 value in the table header

Arguments:

    Hdr     - The table to update

Returns:

    None

--*/
{
    SetCrcAltSize (Hdr->HeaderSize, Hdr);
}

VOID
SetCrcAltSize (
    IN UINTN                 Size,
    IN OUT EFI_TABLE_HEADER *Hdr
    )
/*++

Routine Description:

    Updates the CRC32 value in the table header

Arguments:

    Hdr     - The table to update

Returns:

    None

--*/
{
  UINT32 Crc;

  Hdr->CRC32 = 0;
  BS->CalculateCrc32((UINT8 *)Hdr, Size, &Crc);
  Hdr->CRC32 = Crc;
}


BOOLEAN
CheckCrc (
    IN UINTN                 MaxSize,
    IN OUT EFI_TABLE_HEADER *Hdr
    )
/*++

Routine Description:

    Checks the CRC32 value in the table header

Arguments:

    Hdr     - The table to check

Returns:

    TRUE if the CRC is OK in the table

--*/
{
    return CheckCrcAltSize (MaxSize, Hdr->HeaderSize, Hdr);
}




BOOLEAN
CheckCrcAltSize (
    IN UINTN                 MaxSize,
    IN UINTN                 Size,
    IN OUT EFI_TABLE_HEADER *Hdr
    )
/*++

Routine Description:

    Checks the CRC32 value in the table header

Arguments:

    Hdr     - The table to check

Returns:

    TRUE if the CRC is OK in the table

--*/
{
    UINT32      Crc;
    UINT32      OrgCrc;
    BOOLEAN     f;

    if (Size == 0) {
        //
        // If header size is 0 CRC will pass so return FALSE here
        //
        return FALSE;
    }
    if (MaxSize && Size > MaxSize) {
        DEBUG((D_ERROR, "CheckCrc32: Size > MaxSize\n"));
        return FALSE;
    }

    // clear old crc from header
    OrgCrc = Hdr->CRC32;
    Hdr->CRC32 = 0;
    BS->CalculateCrc32((UINT8 *)Hdr, Size, &Crc);

    // set restults
    Hdr->CRC32 = OrgCrc;

    // return status
    f = OrgCrc == (UINT32) Crc;
    if (!f) {
        DEBUG((D_ERROR, "CheckCrc32: Crc check failed\n"));
    }

    return f;
}
