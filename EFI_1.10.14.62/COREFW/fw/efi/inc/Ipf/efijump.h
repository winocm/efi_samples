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


Abstract:




Revision History

--*/

typedef struct {
    CHAR8       Buffer[1024];   // NOTE:Set/LongJump needs to have this buffer start
                                // at 16 byte boundary. Either fix the structure
                                // which call this buffer or fix inside SetJump/LongJump
                                // Choosing 1K buffer storage for now
} EFI_JUMP_BUFFER;


BOOLEAN
SetJump (
    IN EFI_JUMP_BUFFER  *Jump
    );

VOID
LongJump (
    IN EFI_JUMP_BUFFER  *Jump
    );
