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
// getDO.C
//
// Contains GetDataObjectByName( ) function.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bis_priv.h>

//------------------------------------------------
//  GetDataObjectByName
//      - for the given SignedManifest 'smInfo'
//      - Get the InfoMap if it hasnt yet been done.
//      - search the Map for 'objName'
//      - if name found and other attrib types are correct,
//          return the memory address and length in
//          the CSSM_DATA struct that was passed in by address.
//      - if requested a base64 decode will be done. In this case
//        requestedObj->Data must eventually be MEM_free'd otherwise
//        no explicit free is used (data is in manifest).
//
// returns: 
//      BIS_OK and fills in 'requestedObj' on success.
//      CSSM_ERROR_REPORTED and sets cssmInfo.lastErr on CSSM errors.
//      CSSM_LMAPENT_NOTFOUND if 'objName' is not in map.
//
BIS_STATUS
GetDataObjectByName(
     APP_CSSMINFO_PTR           cssmInfo
    ,BIS_SM_PTR                 smInfo       //SM to obtain signature from.
    ,CSSM_STRING                objName
    ,CSSM_DATA_PTR              requestedObj //[OUT]
    ,BIS_BOOLEAN                decodeFromBase64
    )
{
    BIS_STATUS                  brc= BIS_OK;
    CSSM_VL_DO_INFO_PTR         doInfoPtr; 
    CSSM_VL_ATTRIBUTE_PTR       thisEntry;
         
    UINT32                      i, n, sizeOfObjName;
	ISL_DATA                    encodedData;
	ISL_DATA                    decodedData;
    
        
    //
    requestedObj->Length= 0;
    requestedObj->Data=   NULL;


    //If it has not been done for this 'smInfo' struct,
    // get the data object location map.
    doInfoPtr= smInfo->doInfoPtr;
    if (doInfoPtr == NULL)
    {
        doInfoPtr=
        CSSM_VL_GetDoInfoByName( 
            cssmInfo->hVL
            , smInfo->hVerifiableObj
            , UPDATE_PARMS_SECTION_NAME);

        if (doInfoPtr == NULL)
        {
            return saveCssmErr( cssmInfo );
        }

        smInfo->doInfoPtr= doInfoPtr;

    }


    //Prepare to Search the array of attributes for 'objName'.
    n=      doInfoPtr->BundleWideAttributes.NumberOfAttributes;
    brc=    BIS_SECURITY_FAILURE;
    thisEntry= doInfoPtr->BundleWideAttributes.Attributes;
    sizeOfObjName= BIS_strlen(objName);

    //Search the array.
    for ( i=0; i<n; ++i)
    {

        if (thisEntry->Info.AttributeNameFormat == CSSM_DB_ATTRIBUTE_NAME_AS_BLOB
        &&  thisEntry->Info.AttributeFormat     == CSSM_DB_ATTRIBUTE_FORMAT_BLOB
        &&  BIS_strncmp( thisEntry->Info.Label.Name.Data
            , objName, sizeOfObjName) == 0 )
        {

                //We've got it
                requestedObj->Length= thisEntry->Value.Length;
                requestedObj->Data=   thisEntry->Value.Data;
                brc= BIS_OK;
                break;
          
        }

        else    //Get next attribute pointer.
        {
            ++thisEntry;
        }

    }

    
    //Decode from base 64 binary encoding
    // GWG: added 3rd test in if clause for zero length certs
    if ( decodeFromBase64 && (brc == BIS_OK) && requestedObj->Length > 0)
    {
        encodedData.Data=   requestedObj->Data;
        encodedData.Length= requestedObj->Length;

        
        decodedData.Length=  
        requestedObj->Length= IslUtil_Base64DecodeSize( &encodedData);

        decodedData.Data=
        requestedObj->Data=   MEM_malloc( cssmInfo->appInfo
                              , decodedData.Length );

        if (decodedData.Data==BIS_NULL)
        {
            brc= BIS_MEMALLOC_FAILED;
        }

        else 
        {
            IslUtil_Base64Decode( &encodedData, &decodedData );
            brc= BIS_OK;
        }

    }

    return brc;
}

//eof
