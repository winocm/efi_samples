/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    stall.c

Abstract: 

    Efi Stall function and Tick handler


Revision History

--*/
 
#include "efi.h"
#include "efilib.h"
#include "stall.h"
#include "plstall.h"
#include "pldefio.h"


UINT8
RUNTIMEFUNCTION
RtStall_inp (
    IN  UINT16 Address
    );


STATIC
EFI_STATUS
RUNTIMEFUNCTION
RtPlStall(
    IN UINTN Microseconds
    );


#pragma RUNTIME_CODE(RtStall_inp)
UINT8
RUNTIMEFUNCTION
RtStall_inp (
    IN  UINT16 Address
    )
{
	UINT8 Data;

	RtDefIoRead (NULL, IO_UINT8, Address, 1, &Data);
	return Data;
}


#pragma RUNTIME_CODE(RtPlStall)
STATIC
EFI_STATUS
RUNTIMEFUNCTION 
RtPlStall(
    IN UINTN Micro
    )

/*++

Routine Description:

    This routine performs a blocking stall for the specified number of Microseconds.

Arguments:

    Microseconds    - The number of microseconds to stall

Returns:

    None

--*/

{
    UINTN   Count;


		for (Count = (Micro + 29) / 30; Count != 0; Count--) {
		    while ((RtStall_inp(0x61) & 0x10) == 0x10);
		    while ((RtStall_inp(0x61) & 0x10) == 0x00);
		}

    return EFI_SUCCESS;

}


