/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Abstract:

    Translates between EFI_BIS and legacy BIS status codes.

--*/


#include <efi.h>
#include <bisBaseCode.h>


//
//  BIS <--> EFI Status Code Mappings. Make any changes here the code
//      should take care of everything.
//

typedef
struct {
    BIS_STATUS  BisCode;
    EFI_STATUS  EfiCode;
} MAP_PAIR;

static MAP_PAIR  mapPairs[] =
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

#define PAIRCOUNT (sizeof(mapPairs) / sizeof(MAP_PAIR))


//significant bits from EFI_STATUS codes.
#define EFI_ERRBITS (~ EFI_MAX_BIT)

// Get significant part of an EFI_STATUS code
static
EFI_STATUS significantEfi(
    IN  EFI_STATUS  EfiCode
    )
{
    EFI_STATUS  toReturn;

    toReturn = EfiCode & EFI_ERRBITS;
    return toReturn;
    
} // significantEfi



//
// mapBisToEfi()  -  map  legacy BIS return code into a EFI_STATUS.  If the BIS
// return code is not in the map, EFI_DEVICE_ERROR is returned.
//
EFI_STATUS
mapBisToEfi(
    IN BIS_STATUS code
    )
{
    EFI_STATUS es;
    int        i;

    // pre-initialize in case the code is not found in the map
    es = EFI_DEVICE_ERROR;
    for (i=0 ; i<PAIRCOUNT; i++) {
        if (code == mapPairs[i].BisCode) {
            es = mapPairs[i].EfiCode;
            break;  // skip rest of loop
        }
    }

    return es;
}



//
// mapEfiToBist()  -  map  EFI  BIS  return  code into legacy BIS code.  If the
// EFI_STATUS code is not found in the map, then the EFI_STATUS value is simply
// truncated as necessary and returned.
//

BIS_STATUS
mapEfiToBis(
    IN EFI_STATUS code
    )
{
    BIS_STATUS bs;
    int        i;
    EFI_STATUS CodePart;

    // pre-initialize in case the code is not found in the map
    bs = (BIS_STATUS) code;
    CodePart = significantEfi(code);
    for (i=0; i<PAIRCOUNT; i++) {
        if (CodePart == significantEfi(mapPairs[i].EfiCode)) {
            bs = mapPairs[i].BisCode;
            break;  // skip rest of loop
        }
    }

    return bs;
}

