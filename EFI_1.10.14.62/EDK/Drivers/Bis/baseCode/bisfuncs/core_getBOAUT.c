/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

*/


//************************************************************************************************//
// core_get_BOA_uptok.c
//
// Description: 
// 
//
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//



#include <BIS_PRIV.H>





UINT32 
core_GetBootObjectAuthorizationUpdateToken( 
     BIS_APPINFO_PTR  appInfo
    ,APP_CSSMINFO_PTR cssmInfo
    ,BIS_GBOAUT_PARMS *parmBlock 
    ,UINT32            updateCount
    ,CSSM_GUID_PTR     platformGuid)
{
    #define DIGEST_COMPONENTS 3

    EFI_STATUS     rc;
    CSSM_DATA      DataBufs[DIGEST_COMPONENTS];
    CSSM_GUID      bootObjAuthSetGuid= 
                     BOOT_OBJECT_AUTHORIZATION_PARMSET_GUIDVALUE;


 
    //Load the datastructure with components to digest.
    DataBufs[0].Data=    (UINT8*)platformGuid;
    DataBufs[0].Length=  sizeof(CSSM_GUID);

    DataBufs[1].Data=    (UINT8*)&updateCount;
    DataBufs[1].Length=  sizeof(updateCount);

    DataBufs[2].Data=    (UINT8*)&bootObjAuthSetGuid;
    DataBufs[2].Length=  sizeof(CSSM_GUID);

    
    //Create digest, that is the update token.
    rc=  sha1Digest( 
         appInfo                    //appHandle
        ,DataBufs                   //no data buffs to digest
        ,DIGEST_COMPONENTS          //nbrDataBufs
        ,&parmBlock->updateToken    //digestOut
        );


    //BUGBUG MARMAR - oh no!  returning a pure BIS_STATUS in the parameter block....ugh!
    return (parmBlock->returnValue= (BIS_STATUS)rc);

}


//eof
