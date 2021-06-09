#ifndef _STALL_H
#define _STALL_H
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

    stall.h

Abstract: 

    Efi stall and tick services for legacy hardware


Revision History

--*/



VOID 
RUNTIMEFUNCTION
RtStallLoop(
    UINTN Limit
    );


#endif