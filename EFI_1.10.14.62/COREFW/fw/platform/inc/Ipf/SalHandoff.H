#ifndef _SAL_HANDOFF_H
#define _SAL_HANNOFF_H
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

    SalHandoff.h

Abstract: 

    Handoff from SAL to EFI.


Revision History

--*/

#include EFI_GUID_DEFINITION(PciOptionRomTable)

#define SAL_EFI_NULL_FUNCTIONID             0x00
#define SAL_EFI_CALLBACK_FUNCTIONID         0x01
#define SAL_EFI_NATIVE_DRIVER_FUNCTIONID    0x02
#define SAL_EFI_CACHE_DISABLED              0x10

typedef struct _SALEFIHANDOFF {
    UINT64                         MemDescCount;
    EFI_MEMORY_DESCRIPTOR          *MemDesc;
    VOID                           *SALPROC_Entry;
    VOID                           *SalSystemTable;
    VOID                           *MpsTable;
    VOID                           *AcpiTable;
    VOID                           *SALCallBack;
    UINT64                         EFIGPValue;
    UINT64                         NVRAMBanks; /*  All banks should be of same size */
    UINT64                         NVRAMSize;
    UINT64                         PciOptionRomCount;
    EFI_PCI_OPTION_ROM_DESCRIPTOR  *PciOptionRomDescriptors;
    VOID                           *EfiCoreBaseAddress;
    UINT64                         EfiCoreLength;
} SALEFIHANDOFF;

#endif