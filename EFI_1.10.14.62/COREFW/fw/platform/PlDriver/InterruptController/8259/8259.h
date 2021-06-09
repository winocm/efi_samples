#ifndef _8259_H
#define _8259_H
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

    8259.h

Abstract: 

    8259 primitives

Revision History

--*/

typedef UINT16 INT_CTRL_MASK;
#define DEFAULT_EFI_MASK (INT_CTRL_MASK)0xffff


INT_CTRL_MASK
BOOTSERVICE
PlGetIntrCtrlMask(
    VOID
    );
    
VOID
BOOTSERVICE
PlSetIntrCtrlMask(
    INT_CTRL_MASK
    );
    
#endif
