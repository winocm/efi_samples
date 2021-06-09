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

#include "imem.h"


//
// These globals are runtime globals
//

#pragma BEGIN_RUNTIME_DATA()

//
// MemoryLock - synchronizes access to the memory map and pool lists
//

FLOCK           MemoryLock = {0};

//
// MemoryMap - the curernt memory map
//

LIST_ENTRY      MemoryMap = {0};

//
// MemoryLastConvert - the last memory descriptor used for a conversion request
//

MEMORY_MAP      *MemoryLastConvert = NULL;
