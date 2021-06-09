///////////////////////////////////////////////////////////////////////////////
//
// This software is provided "as is" with no warranties, express or
// implied, including but not limited to any implied warranty of
// merchantability, fitness for a particular purpose, or freedom from
// infringement.
//
// Intel Corporation may have patents or pending patent applications,
// trademarks, copyrights, or other intellectual property rights that
// relate to this software.  The furnishing of this document does not
// provide any license, express or implied, by estoppel or otherwise,
// to any such patents, trademarks, copyrights, or other intellectual
// property rights.
//
// This software is furnished under license and may only be used or
// copied in accordance with the terms of the license. Except as
// permitted by such license, no part of this software may be reproduced,
// stored in a retrieval system, or transmitted in any form or by any
// means without the express written consent of Intel Corporation.
//
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
// NVM.C
//
// Description: High-Level Non-volatile Memory Functions.
//
//      The functions implemented here are used by BIS
//      to manage it's platform dependent persistant storage.
//
//      The api is intentionally general in order that it may
//      be adapted to different platform dependednt storage mechanisms.
//
//      The implementation, on the other hand, is written to take advantage
//      of the fact the "NVM_BIS_AREA_ID" is the only NVM area that exists and
//      that the NVI.H functions passed to BIS_main are available.
//
//
//************************************************************************************************//

#include <bisBaseCode.h>





    //----------------------------------------------------------------//
    // NVM_Init - validate and save the persistent storage function   //
    //      pointer structure. Allocate a buffer to hold a copy of the//
    //      flash data, read the data into it's buffer;

BIS_STATUS
NVM_Init(  )
{
    EFI_STATUS           efiStatus=     EFI_SUCCESS;
	BISBC_INSTANCEDATA  *BISID= getInstanceData( BISBC );

	BISID->flashDataChanged=  BIS_FALSE;    //The copy was changed.
	BISID->doValidityCheck=   BIS_TRUE;     //Indicates whether the one time

    //Call PersistentStorage_GetLength to find memory size requirement.
    BISID->Persist->GetLength(BISID->Persist, &BISID->flashDataLength, NULL);

    //Call MEM_allocBisData to create buffer to hold
    // copy of persistent storage.
    BISID->flashDataChanged=  BIS_FALSE;
    BISID->flashData= x_malloc( BISID->flashDataLength );
    if (BISID->flashData != BIS_NULL)
    {
        //Call PersistentStorage_Read to get data into memory.
        efiStatus= BISID->Persist->Read(
			  BISID->Persist
            , BISID->flashData
			, NULL);

        //Free the memory if error.
        if (efiStatus != BIS_OK)
        {
            gBS->FreePool( BISID->flashData);
            BISID->Persist=   BIS_NULL;
            BISID->flashData=        BIS_NULL;
        }

    }
    else    //Failed to allocate buffer. zero function table pointer
    {       //to signal problem to other functions.
		BISID->Persist= BIS_NULL;
    }


    return mapEfiToBis(efiStatus);
}

    //--------------------------------------------------------------//
    //  NVM_Open - open a NonVolatileMemory area by ID.             //
    //                                                              //
    //  On Success returns:                                         //
    //      the length of the area in [OUT] parm 'areaLength'.      //
    //      and                                                     //
    //      a handle used for subsequent operations on the area.    //
    //                                                              //
    //  On Error returns:                                           //
    //      the 0 in the [OUT] parm 'areaLength'.                   //
    //      and                                                     //
    //      BIS_NULL.                                               //
    //                                                              //
    //--------------------------------------------------------------//

NVM_AREA_HANDLE
NVM_Open( BIS_APPINFO *appInfo, NVM_AREA_ID *memAreaToOpen, UINT32 *areaLength)
{
    BIS_DATA_PTR    bisData;
    UINT32          computedDigest;
    UINT32          storedDigest = 0;
    EFI_STATUS      rv=EFI_SUCCESS;
    NVM_AREA_HANDLE newHandle;
	BISBC_INSTANCEDATA  *BISID= getInstanceData( BISBC );


    //Return NULL if Persist ptr is null
    *areaLength= 0;
    if (BISID->Persist==BIS_NULL) return BIS_NULL;

    //Check handle for validity, return error if not valid.
    if ( !IS_APPINFO_VALID(appInfo) ) return BIS_NULL;

    //If NVM_AREA_ID not equal to NVM_BIS_AREA_ID, return NULL.
    if ( checkForBisGUID( memAreaToOpen ) == BIS_FALSE) return BIS_NULL;


    //Call MEM_allocBisData to create buffer pointing to flashdata.
    bisData= MEM_allocBisData( appInfo,  0 );   //alloc a BIS_DATA w/o buffer.
    if (bisData == BIS_NULL) return BIS_NULL;
    bisData->data=   BISID->flashData;
    bisData->length= *areaLength= BISID->flashDataLength;


    //Check the cached NVM for validity. This is done here instead of
    //in NVM_Init because we need an appInfo context to use to
    //call digestNVM.
    if ( BISID->doValidityCheck )
    {
        BISID->doValidityCheck= BIS_FALSE;         //Only done once.

        //Recalc digest of NVM contents.
        rv= digestNVM( appInfo, &computedDigest );

        if (rv==EFI_SUCCESS)
        {
            //Read the stored digest out of the NVM.
            //This stored value was computed on last NVM_Close.
            newHandle= (NVM_AREA_HANDLE)bisData;
            rv= NVM_Read(
                newHandle,                  //nvm handle
                (UINT8*)&storedDigest,      //buffer to read into
                GET_NVM_FIELD_SIZE( truncatedDigest ),    //nvm field length
                GET_NVM_FIELD_OFFSET( truncatedDigest ) //nvm field offset
                );
        }


        //Check that the computed digest is equal to the stored value.
        if (rv==EFI_SUCCESS)
        {
            if (computedDigest != storedDigest)
            {
                rv= EFI_DEVICE_ERROR;
            }
        }

        //If digests don't match, or any error was encountered
        //trying to compute them, clear the flash cache so that
        //it appears the the checkFlag is not set and that
        //there is no BOAC installed.
        if (rv!=EFI_SUCCESS)
        {
            EfiSetMem(BISID->flashData, BISID->flashDataLength, 0);
        }
    }



    //return address of memory as handle and length in *areaLength.
    return bisData;
}





    //--------------------------------------------------//
    // NVM_READ -                                       //
    //                                                  //
    //--------------------------------------------------//

EFI_STATUS
NVM_Read( NVM_AREA_HANDLE areaHandle, UINT8 *data
, UINT32 dataLength, UINT32 offset )
{
    EFI_STATUS   rv;
    UINT8        *buffer;
    BIS_DATA_PTR bisData;


    //Check offset and length for validity
    rv= NVM_CheckParms( areaHandle, data, dataLength, offset );
    if (  rv != EFI_SUCCESS)
    {
        return rv;
    }

    //Cast the areaHandle to a BIS_DATA_PTR and extract buffer ptr.
    bisData= (BIS_DATA_PTR)areaHandle;
    buffer=  (UINT8*)bisData->data;

    //Copy to data from 'p' to user's buffer 'data' at specified offset
    //and length.
    EfiCopyMem( data, buffer+offset, dataLength );
    return EFI_SUCCESS;
}



    //--------------------------------------------------//
    // NVM_WRITE                                        //
    //                                                  //
    //--------------------------------------------------//
EFI_STATUS
NVM_Write( NVM_AREA_HANDLE areaHandle
    , UINT8 *data
    , UINT32 dataLength
    , UINT32 offset )
{
    EFI_STATUS   rv;
    UINT8        *buffer;
    BIS_DATA_PTR bisData;
    UINT8        *tmpData;
    UINT8        dummyData;
	BISBC_INSTANCEDATA  *BISID= getInstanceData( BISBC );


    // Make sure tmpData points to something safe
    // GWG & PD:
    if (dataLength == 0)
    {
        tmpData = &dummyData;
    }
    else {
        tmpData = data;
    }

    //Check offset and length for validity
    rv= NVM_CheckParms( areaHandle, tmpData, dataLength, offset );
    if (  rv != EFI_SUCCESS)
    {
        return rv;
    }

    //Cast the areaHandle to a BIS_DATA_PTR and extract buffer ptr.
    bisData= (BIS_DATA_PTR)areaHandle;
    buffer=  (UINT8*)bisData->data;

    //Copy to data from 'p' to user's buffer 'data' at specified offset
    //and length.
    EfiCopyMem( buffer+offset, tmpData, dataLength );
    BISID->flashDataChanged=  BIS_TRUE;
    return EFI_SUCCESS;
}



    //-------------------------------------------------------------//
    // NVM_Close - write the buffer associated with 'areaHandle'   //
    //   back to underlying storage, delete handle.                //
    //-------------------------------------------------------------//

EFI_STATUS
NVM_Close( BIS_APPINFO *appInfo, NVM_AREA_HANDLE areaHandle )
{
  BIS_DATA_PTR bisData;
  UINT8        *areaBuffer;
  EFI_STATUS   rv=            EFI_SUCCESS;
  EFI_STATUS   efiStatus=     EFI_SUCCESS;
  UINT32       computedDigest;
	BISBC_INSTANCEDATA  *BISID= getInstanceData( BISBC );

  bisData=    (BIS_DATA_PTR)areaHandle;

    //Check handle for validity, return error if not valid.
    if ( BISID->Persist==BIS_NULL)  return BIS_INIT_FAILURE;
    if ( !IS_APPINFO_VALID(appInfo) ) return BIS_BAD_APPHANDLE;
    if ( areaHandle==BIS_NULL )       return BIS_BAD_PARM;

    //If buffer copy of flash data has changed, write it out.
    if (BISID->flashDataChanged == BIS_TRUE )
    {
        //Cast handle to BIS_DATA_PTR. Extract buffer pointer.
        //Get actual area length.
        bisData=    (BIS_DATA_PTR)areaHandle;
        areaBuffer= bisData->data;

        //Recalc digest of NVM contents.
        rv= digestNVM( appInfo, &computedDigest );
        if ( rv==BIS_OK )
        {
            //put the computed digest into nvm
            rv= NVM_Write(
                areaHandle,                   //nvm handle
                (UINT8*)&computedDigest,      //buffer to read into
                GET_NVM_FIELD_SIZE( truncatedDigest ),    //nvm field length
                GET_NVM_FIELD_OFFSET( truncatedDigest ) //nvm field offset
                );

        }


        //Dont update flash,  if digest calculation or write failed.
        //Error should be propagated up to BIS_Shutdown.
        if (rv==EFI_SUCCESS)
        {
            //Call PersistentStorage_Write to put buffer to persistent storage.
            efiStatus= BISID->Persist->Write(
					                                    BISID->Persist,
                	                           (UINT8*)areaBuffer,
					                                    NULL);

            rv = efiStatus;

            BISID->flashDataChanged= BIS_FALSE;
        }
    }

    //Free the memory 'handle'. Only the appInfo BIS_DATA structure
    //is being freed. The data pointer that it constains was allocated
    //by NVM_Init.
    MEM_free( appInfo, bisData);

    return rv;

}


    //---------------------------------------------------------//
    //  checkForBisGUID - compare parms to NVM_BIS_AREA_ID.    //
    //---------------------------------------------------------//

static  EFI_GUID bisGUID= NVM_BIS_AREA_ID;


BIS_BOOLEAN
    checkForBisGUID( NVM_AREA_ID *callerGUID )
{

    BIS_BOOLEAN rv= BIS_FALSE;

    if ( EfiCompareGuid( (EFI_GUID*)callerGUID, &bisGUID )) {
		rv= BIS_TRUE;
	}

    return rv;

}





    //--------------------------------------------------//
    //  NVM_CheckParms - common read / write parm check //
    //                                                  //
    //--------------------------------------------------//

EFI_STATUS
NVM_CheckParms(  
  NVM_AREA_HANDLE areaHandle,
  UINT8           *data,
  UINT32          dataLength,
  UINT32          offset 
  )
{
    BIS_DATA_PTR bisData;
    UINT32   areaLength;

    //Check handle and data pointer for validity, return error if not valid.
    if ( areaHandle == BIS_NULL )     return EFI_DEVICE_ERROR;
    if ( data       == BIS_NULL )     return EFI_INVALID_PARAMETER;

    //Check area and offset.
    bisData= (BIS_DATA_PTR)areaHandle;
    areaLength= bisData->length;

    //Check offset and length combinations.
    if ( offset >= areaLength) return EFI_DEVICE_ERROR;
    areaLength= areaLength - offset;
    if ( dataLength > areaLength)  return EFI_DEVICE_ERROR;

    return EFI_SUCCESS;
}





//--------------------------------------------------------------------------------//
//  digestNVM - compute a SHA1 digest of the
//
//  PARMS:
//    BIS_APPINFO_PTR  appInfo     - appInfo structure.
//    UINT32          *digestOut   - [OUT] pointer to trucated digest
//
//  returns: BIS_OK on success.//
//----------------------------------------------------------------------------------//

EFI_STATUS
digestNVM(
    BIS_APPINFO_PTR  appInfo,
    UINT32          *truncDigestOut
    )
{
    BISBC_INSTANCEDATA  *BISID= getInstanceData( BISBC );
    EFI_STATUS    		rv=     EFI_SUCCESS;
    BIS_DATA_PTR  		fullDigest;
    UINT32        		offset;
    CSSM_DATA     		cssmData[1];


    //The truncatedDigest  field must be the first thing
    //in the NVM area and we enforce that constraint.
    offset= GET_NVM_FIELD_OFFSET( truncatedDigest );
    if ( offset != 0 )
    {
        rv= EFI_DEVICE_ERROR;
    }


    //Setup and create the digest.
    if ( rv==EFI_SUCCESS )
    {
        //Prep to digest everything after the digestField in the NVM cache.

        cssmData[0].Data=
        	BISID->flashData + GET_NVM_FIELD_SIZE( truncatedDigest );
        cssmData[0].Length=
        	BISID->flashDataLength - GET_NVM_FIELD_SIZE( truncatedDigest );


        //Prep to digest everything after the digestField in the NVM cache.
        //Digest the NVM
        rv= sha1Digest(
                appInfo,        //app control block
                cssmData,       //vector to NVM data
                1,              //one element in vector.
                &fullDigest);   //output, complete sha1 digest.

    	//Extract the 1st 4 bytes from the digest
    	if (rv==EFI_SUCCESS)
    	{
    	    //extract 1st 32bits of digest, to user's outparm.
    	    *truncDigestOut=(*(UINT32*)fullDigest->data);

    	    //Free the memory allocated by sha1Digest.
    	    MEM_free( appInfo, fullDigest );
	   	}

    }


    return rv;

}


//History:
//Branched from smbios bis file...
//	Archive: /SMA/Src/bis/util/nvm.c
//	Revision: 18
//  Date: 2/02/99 6:16p


//eof
