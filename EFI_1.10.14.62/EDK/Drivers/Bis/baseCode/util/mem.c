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

#include <bisBaseCode.h>

//************************************************************************************************//
//
//  Mem.c -
//      MEM_alloc and MEM_free are used to allocate and track
//          memory on behalf of an application instance.
//
//      The BIS_xxx function are low level memory functions
//      analogous to the unix style malloc and free calls.
//      built on heapman.c
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//




#pragma pack(1)




	//---------------------------------------------------------------//
	// MEM_malloc - allocate memory and record allocation in appinfo //
	//---------------------------------------------------------------//

void *MEM_malloc( BIS_APPINFO *appInfo,  UINT32 size )
{
    void *memBlk;

    memBlk= x_malloc( size );
    if ( memBlk != BIS_NULL)
    {
        COL_AddElement( appInfo->memoryAllocations, (UINTN)memBlk );
        //TBD enter error trap if above call fails.
    }

    return memBlk;
}



	//--------------------------------------------------------------//
	// MEM_free - free memory and update allocation info in appinfo //
	//--------------------------------------------------------------//

BIS_BOOLEAN
  MEM_free( BIS_APPINFO *appInfo, void *memBlock )
{
    BIS_BOOLEAN rc= BIS_FALSE;    //assume failure

    //If not found and remove from collection, it is bogus.
    if ( COL_RemoveElement( appInfo->memoryAllocations, (UINTN)memBlock ) )
    {
        gBS->FreePool( memBlock);
        rc= BIS_TRUE;
    }

    return rc;

}



	//-----------------------------------------------------------------//
	// MEM_allocBisData - allocate a BIS_DATA for specified size.      //
    //      The caller may specified zero if they will supply the
    //      memory. If non zero, the function allocates it and fills
    //      in the fields in the BIS_DATA struct.
    //
	//-----------------------------------------------------------------//

BIS_DATA_PTR
MEM_allocBisData( BIS_APPINFO *appInfo,  UINT32 size )
{
    void            *memBlk;
    BIS_DATA_PTR    bdPtr;

    //Alloc the BIS_DATA struct and the requested mem as one block.
    memBlk= MEM_malloc( appInfo,  size + sizeof(BIS_DATA) );
    bdPtr=  (BIS_DATA_PTR)memBlk;
    if (bdPtr)
    {

        //Init the Length field, and the Data pointer
        bdPtr->length= size;
        if (size==0)  {
            bdPtr->data= BIS_NULL; }
        else
        {
            bdPtr->data= (char*)memBlk + sizeof(BIS_DATA);
        }
    }

    return bdPtr;
}


	//-----------------------------------------------//
	// _malloc - low level malloc implementation.    //
	//-----------------------------------------------//

void  * x_malloc( UINT32 size )
{
	VOID                *Buffer= NULL;

  gBS->AllocatePool (
                  EfiBootServicesData, size, &Buffer
                  );
  EfiZeroMem (Buffer, size);
	
  return Buffer;
}


	//-------------//
	// _calloc     //
	//-------------//


void  * x_calloc( UINT32 num, UINT32 size )
{
	VOID                *Buffer= NULL;

  Buffer = x_malloc (size * num);
	return Buffer;
}


//History:
// Branched from smbios-bis file:
//   Archive: /SMA/Src/bis/Mem/mem.c
//   Revision: 14
//   Date: 10/07/98 1:58p


//eof
