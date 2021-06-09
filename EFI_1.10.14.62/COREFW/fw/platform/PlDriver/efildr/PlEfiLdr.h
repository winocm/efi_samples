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

    PlEfiLdr.h

Abstract: 

    Support for transition to runtime

Revision History

--*/

#ifndef _PLEFILDR_H
#define _PLEFILDR_H

#include "EfiLdrHandoff.h"

VOID
PlInitEfiLdrCallBack (
    IN EFILDR_CALLBACK*EfiLdrCallBackEntry
    );

VOID
RtPlEfiLdrCallBack (
    UINTN FuncId,
    UINTN parm1,
    UINTN parm2,
    UINTN parm3);

#endif
