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

#include "ivar.h"


//
// These globals are runtime globals
//

#pragma BEGIN_RUNTIME_DATA()

//
// VariableStoreLock - Protects access to all variable stores
//

FLOCK   VariableStoreLock = { 0 };

//
// VariableStore - Variable store by type
//

INTERNAL VARIABLE_STORE VariableStore[MAX_VARIABLE_STORAGE_TYPE] = { NULL };


//
// VariableStoreType - Converts variable attributes to variable store
//

INTERNAL VARIABLE_STORE *VariableStoreType[VAR_ATTRIBUTE_TYPE+1] = { NULL };

