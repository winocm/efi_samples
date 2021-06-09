/*-----------------------------------------------------------------------
 *      File: config.c  
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------
 */
/* 
    * INTEL CONFIDENTIAL 
    * This file, software, or program is supplied under the terms of a 
    * license agreement or nondisclosure agreement with Intel Corporation 
    * and may not be copied or disclosed except in accordance with the 
    * terms of that agreement. This file, software, or program contains 
    * copyrighted material and/or trade secret information of Intel 
    * Corporation, and must be treated as such. Intel reserves all rights 
    * in this material, except as the license agreement or nondisclosure 
    * agreement specifically indicate. 
    */ 
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software is subject to the U.S. Export Administration Regulations 
 * and other U.S. law, and may not be exported or re-exported to certain 
 * countries (currently Afghanistan (Taliban-controlled areas), Cuba, Iran, 
 * Iraq, Libya, North Korea, Serbia (except Kosovo), Sudan and Syria) or to 
 * persons or entities prohibited from receiving U.S. exports (including Denied 
 * Parties, Specially Designated Nationals, and entities on the Bureau of 
 * Export Administration Entity List or involved with missile technology or 
 * nuclear, chemical or biological weapons).
 */ 
/*
**	Implementation of ISL_ArchiveConfiguration class for Javasoft Jar format 
**  archive
*/
#include "isl_internal.h"

/* 
**	ISL_ArchiveConfiguration class public methods (configuration)
*/
/* class methods */
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
static ISL_SIZE SizeofObject() 			/* returns sizeof object */
{
	return sizeof(struct isl_config);
}
	/* object methods */
/*-----------------------------------------------------------------------------
 * Name: InitializeNew
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS InitializeNew(				/* ISL_ARCHIVE_CONTEXT constructor */
		ISL_CONFIG_PTR Context,			/* pointer to configuration context memory */
		ISL_MEMORY_METHODS *memoryMethods)		/* malloc, free methods */
{
	if (Context == NULL ||
		memoryMethods == NULL ||
		memoryMethods->malloc_func == NULL ||
		memoryMethods->free_func == NULL)
	{
		return ISL_FAIL;
	}

	cssm_memset(Context, 0, sizeof(struct isl_config));
//	Context->Methods = &ArchiveConfigMethods;
	Context->AllocRef = memoryMethods->AllocRef;
	Context->MemoryMethods = memoryMethods;
	Context->Algorithms = 0;
	return ISL_OK;
}
/*-----------------------------------------------------------------------------
 * Name: AddAlgorithm
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS AddAlgorithm(
		ISL_CONFIG_PTR Context,				/* configuration context */
		const ISL_SERVICE_CLASS_METHODS *Methods)	/* vector of methods */
{
	struct isl_algorithm_list_item *item;

	ISL_SERVICE_CLASS AlgClass;		/* service type performed by algorithm */
	ISL_CONST_DATA AlgID;			/* archive-specific-encoded algorithm ID */
	ISL_CONST_DATA HumanName;		/* human-readable algorithm name */

	if (Context == NULL ||
		Methods == NULL ||
		Methods->id == NULL) 
	{
		return ISL_FAIL;
	}

	item = Context->MemoryMethods->malloc_func(
		sizeof(struct isl_algorithm_list_item),
		Context->AllocRef); 
	if (item == NULL) {
		return ISL_FAIL;
	}
	Methods->id(&AlgClass, &AlgID, &HumanName);
	item->ServiceType = AlgClass;
	item->ArchiveName = AlgID;
	item->HumanName = HumanName;
	item->Methods = (ISL_SERVICE_CLASS_METHODS *)Methods;
	item->Next = Context->Algorithms;
	Context->Algorithms = item;
	return ISL_OK;
}
/*-----------------------------------------------------------------------------
 * Class: ArchiveConfiguration
 * Name:  FindAlgorithm
 *
 * Description:
 * Attempts to locate a service methods vector named by the algorithm ID
 *
 * Parameters: 
 * Context (input)   : this pointer
 * AlgID (input)	 : name of the service 
 *
 * Return value:
 * Return a pointer to the service methods vector
 * 
 * Error Codes:
 * CSSM_VL_
 * CSSM_VL_INVALID_NAME
 * CSSM_VL_MEMORY_ERROR
 * CSSM_VL_INVALID_DATA_POINTER
 *---------------------------------------------------------------------------*/
static ISL_SERVICE_CLASS_METHODS *FindAlgorithm(
		ISL_CONFIG_PTR Context,
		ISL_CONST_DATA AlgID)
{
	struct isl_algorithm_list_item *item;

	if (Context == NULL ||
		AlgID.Data == NULL) return NULL;

	for (item = Context->Algorithms; item; item = item->Next) {
		if (IS_EQUAL(item->HumanName, AlgID) || 
			IS_EQUAL(item->ArchiveName, AlgID))
        {
			return item->Methods;
		}
	}
	return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: Recycle
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS Recycle(				/* destructor for ISL_Context */
		ISL_CONFIG_PTR Context) 	/* configuration context */
{
	struct isl_algorithm_list_item *item, *next;

	if (Context == NULL) return ISL_FAIL;

	for (item = Context->Algorithms; item; item = next) {
		next = item->Next;
		Context->MemoryMethods->free_func(item, Context->AllocRef);
	}
	Context->Algorithms = 0;
	return ISL_OK;
}

/*
**	export only the vector of methods
*/
struct isl_config_methods ArchiveConfigMethods = {
	{0, 0},
	SizeofObject, 
	InitializeNew, 
	AddAlgorithm, 
	FindAlgorithm, 
	Recycle,
};
