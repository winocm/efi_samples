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

#include "loader.h"


//
// These globals are runtime globals
//

#pragma BEGIN_RUNTIME_DATA()

//
// ImageListLock - Lock to gaurd image lists.
//

INTERNAL FLOCK       ImageListLock = {0};

//
// BootImageList - List of driver images loaded into boot services memory.
//

INTERNAL LIST_ENTRY  BootImageList = {0};

//
// RuntimeImageList - List of images loaded into runtime memory.
//

INTERNAL LIST_ENTRY  RuntimeImageList = {0};
