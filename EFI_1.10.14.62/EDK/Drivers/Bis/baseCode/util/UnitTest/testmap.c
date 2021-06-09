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

    testmap.c

Abstract:

    Unit-test for codeMapping.c

--*/



#include <efi.h>
#include <efierr.h>
#include <bisBaseCode.h>
#include <stdio.h>


typedef
struct {
    BIS_STATUS  BisCode;
    EFI_STATUS  EfiCode;
} MAP_PAIR;

#define BIS_BOGUS_CODE          (222)
#define EFI_BOGUS_CODE          (333)
#define EFI_NOT_FOUND_EXTENDED  (EFI_NOT_FOUND | 0xF000)
#define EFI_BOGUS_BUT_MATCHING  (EFI_NOT_FOUND | 0x80)

static MAP_PAIR  TwoWayCases[] =
{
    {BIS_OK,                           EFI_SUCCESS },
    {BIS_MEMALLOC_FAILED,              EFI_OUT_OF_RESOURCES },
    {BIS_BAD_APPHANDLE,                EFI_NO_MAPPING},
    {BIS_NOT_IMPLEMENTED,              EFI_UNSUPPORTED },
    {BIS_BAD_PARM,                     EFI_INVALID_PARAMETER},
    {BIS_BOA_CERT_NOTFOUND,            EFI_NOT_FOUND},
    {BIS_SECURITY_FAILURE,             EFI_SECURITY_VIOLATION},
    {BIS_INIT_FAILURE,                 EFI_DEVICE_ERROR },
    {BIS_INCOMPAT_VER,                 EFI_INCOMPATIBLE_VERSION },
};
#define NUM_TWO_WAY     (sizeof(TwoWayCases) / sizeof(MAP_PAIR))

static MAP_PAIR  BisToEfiCases[] =
{
    {BIS_BOGUS_CODE,                   EFI_DEVICE_ERROR},
};
#define NUM_BIS_TO_EFI  (sizeof(BisToEfiCases) / sizeof(MAP_PAIR))

static MAP_PAIR  EfiToBisCases[] =
{
    {(BIS_STATUS) EFI_BOGUS_CODE,          EFI_BOGUS_CODE},
    {(BIS_STATUS) EFI_NOT_FOUND_EXTENDED,  EFI_NOT_FOUND_EXTENDED},
    {(BIS_STATUS) EFI_BOGUS_BUT_MATCHING,  EFI_BOGUS_BUT_MATCHING},
};
#define NUM_EFI_TO_BIS  (sizeof(EfiToBisCases) / sizeof(MAP_PAIR))


// __FILE__ doesn't make CHAR16 stuff
static CHAR16  filename[] = L"testmap.c";


void main(
    int      argc,
    char **  argv
    )
{
    // unreferenced
    argc;
    argv;

    // Try the two-way cases
    {
        int        i;
        BIS_STATUS bs;
        EFI_STATUS es;

        for (i=0; i<NUM_TWO_WAY; i++) {
            es = mapBisToEfi(
                TwoWayCases[i].BisCode,  // code
                __LINE__,                // callingLine
                filename);               // module
            if (es != TwoWayCases[i].EfiCode) {
                printf(
                    "mapBisToEfi expected %d, got %d\n",
                    TwoWayCases[i].EfiCode,
                    es);
            }
            bs = mapEfiToBis(
                TwoWayCases[i].EfiCode);  // code
            if (bs != TwoWayCases[i].BisCode) {
                printf(
                    "mapEfiToBis expected %d, got %d\n",
                    TwoWayCases[i].BisCode,
                    bs);
            }
        }
    }

    // Try the one-way bis to efi cases
    {
        int        i;
        EFI_STATUS es;

        for (i=0; i<NUM_BIS_TO_EFI; i++) {
            es = mapBisToEfi(
                BisToEfiCases[i].BisCode,  // code
                __LINE__,                // callingLine
                filename);               // module
            if (es != BisToEfiCases[i].EfiCode) {
                printf(
                    "mapBisToEfi expected %d, got %d\n",
                    BisToEfiCases[i].EfiCode,
                    es);
            }
        }
    }

    // Try the one-way efi to bis cases
    {
        int        i;
        BIS_STATUS bs;

        for (i=0; i<NUM_EFI_TO_BIS; i++) {
            bs = mapEfiToBis(
                EfiToBisCases[i].EfiCode);  // code
            if (bs != EfiToBisCases[i].BisCode) {
                printf(
                    "mapEfiToBis expected %d, got %d\n",
                    EfiToBisCases[i].BisCode,
                    bs);
            }
        }
    }
    
} // main
