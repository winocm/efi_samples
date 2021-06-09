#ifndef _PLSAL_H
#define _PLSAL_H
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

    PlSal.h

Abstract: 

    Support for PAL and SAL calls

Revision History

--*/


VOID
PlInitSalIVA (
    );

UINT64
EFIAPI
RtPlGetRtSalIVA (
    );

VOID
PlInitSalPalProc (
    IN  UINT64  SALCallBackEntry,
    IN  UINT64  SALCallBackGP
    );

//
// Global Plabels for SalProc(SAL OS Interface) and SalCallBack(Private implementation interfaces) 
//
extern PLABEL      RtGlobalSalProcEntry;

extern PLABEL      RtGlobalSALCallBack;

extern UINT64      RtSalIVA;

#endif