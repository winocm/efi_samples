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

    data.c

Abstract:

    Global data in the Bios Platform code



Revision History

--*/


#include "BootBlock.h"


//
// Runtime Globals
//

#pragma RUNTIME_DATA()
//
// Global Plabels for SalProc(SAL OS Interface) and SalCallBack(Private implementation interfaces) 
//
PLABEL      RtGlobalSalProcEntry = { 0x0, 0x0 };

PLABEL      RtGlobalSALCallBack = { 0x0, 0x0 };
