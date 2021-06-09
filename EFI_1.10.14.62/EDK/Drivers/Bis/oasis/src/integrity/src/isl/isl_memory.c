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
  
  isl_memory.c

Abstract:


Revision History

--*/
#include "isl_internal.h"
#include "islutil.h"

/*-----------------------------------------------------------------------------
 * Name: SizeofObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static void id(
	ISL_SERVICE_CLASS *AlgClass,		/* return service type code */
	ISL_CONST_DATA_PTR AlgID,			/* return archive-specific encoding */
	ISL_CONST_DATA_PTR ServiceName)		/* return human-readable description */
{
	/* used only for testing */
	*AlgClass = ISL_ServiceGetData;
	AlgID->Data = (uint8 *)"memory";
	AlgID->Length = cssm_strlen((char *)AlgID->Data);
	ServiceName->Data = (uint8 *)"Memory";
	ServiceName->Length = cssm_strlen((char *)ServiceName->Data);
}
/*-----------------------------------------------------------------------------
 * Name: SizeofObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_SIZE size()
{
	return sizeof(ISL_CONST_DATA);
}

/*-----------------------------------------------------------------------------
 * Name: SizeofObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS init(
	ISL_GET_DATA_SERVICE_CONTEXT *memory,
	ISL_CONST_DATA parameters)
{
	ISL_CONST_DATA_PTR out = (ISL_CONST_DATA_PTR) memory;

    if (out == NULL ||
        parameters.Data == NULL ||
        parameters.Length != sizeof(ISL_CONST_DATA)) return ISL_FAIL;

    *out = *(ISL_CONST_DATA_PTR)parameters.Data;
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: SizeofObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_ERROR_CODE update(
	ISL_GET_DATA_SERVICE_CONTEXT *context,
	ISL_CONST_DATA *data)
{
	ISL_DATA_PTR it = (ISL_DATA_PTR) context;
	data->Data = (const void *) it->Data;
	data->Length = it->Length;
	it->Length = 0;			/* next call gets "end of stream" */
	return ISL_NO_ERROR;
}

/*-----------------------------------------------------------------------------
 * Name: recycle
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
#pragma warning (disable: 4100)
static ISL_ERROR_CODE recycle(
	ISL_GET_DATA_SERVICE_CONTEXT *context)
{	
	return ISL_NO_ERROR;
}
#pragma warning (default: 4100)

/*-----------------------------------------------------------------------------
 * Name: initwithclass
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
#pragma warning (disable: 4100)
static ISL_STATUS initwithclass(
	ISL_GET_DATA_SERVICE_CONTEXT *Memory,
	ISL_CONST_DATA Parameters,					/* archive-specific-encoded parameters */
	ISL_CLASS_PTR Class,						/* class structure */
	ISL_MANIFEST_SECTION_PTR Section)			/* manifest section, if any */
{
	return init(Memory, Parameters);
}
#pragma warning (default: 4100)

/*-----------------------------------------------------------------------------
 * Name: SizeofObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_GET_DATA_METHODS getMemoryMethods = { 
	{id, 0},
	size, 
	init, 
	update, 
	recycle,
	initwithclass};
