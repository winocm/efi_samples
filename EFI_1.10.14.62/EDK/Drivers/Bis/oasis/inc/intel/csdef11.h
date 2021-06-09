/*-----------------------------------------------------------------------
 *      File:   csdef11.h
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

#ifndef	_CSDEF11_H
#define	_CSDEF11_H

/* Special getinfo flags */
#define GENCAL_INFO_PRESENCE_CHECK	0x80000000

/* Passthrough identifiers */
/* GENCAL_PT_STORE_DATA
 * Stores a GENCAL_DATA_OBJECT on the device. Data is written to the device
 * without translation. The name of the object on the device is also written
 * without translation. If the object already exists, an attempt will be
 * made to overwrite the existing value.
 */
#define GENCAL_PT_STORE_DATA	1
/* GENCAL_PT_FETCH_DATA
 * Fetches an object from the device with the specified name. If two object
 * with the same name are on the device, the first instance is returned. The
 * application is required to allocate memory for the returned data object.
 */
#define GENCAL_PT_FETCH_DATA	2

#define GENCAL_PT_DELETE_DATA	3

/* Data object structure used by the STORE_DATA and FETCH_DATA passthroughs */
typedef struct _gencal_data_object {
	CSSM_DATA Name;
	CSSM_DATA Object;
} GENCAL_DATA_OBJECT, *GENCAL_DATA_OBJECT_PTR;

#endif
