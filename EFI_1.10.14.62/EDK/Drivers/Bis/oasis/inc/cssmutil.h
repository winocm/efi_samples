/*-----------------------------------------------------------------------
 *      File:   CSSMUTIL.H
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  -----------------------------------------------------------------------
 */

/*  * INTEL CONFIDENTIAL 
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

#ifndef _CSSMUTIL_H
#define _CSSMUTIL_H    

#include "cssm.h"
#pragma warning (disable:4201 4514 4214 4115)
#include <time.h>
#pragma warning (default:4201 4214 4115)

/* Gary's BER/DER routines to be included from
 * tools/signvrfy/berder.h and tools/signvrfy/berder.c
 * eventually 
 */


#define cssm_min(int1, int2)        ((int1 < int2) ?    int1 : int2)
#define cssm_max(int1, int2)        ((int1 > int2) ?    int1 : int2)

#define CSSM_MAX_TIME_LEN   30

/* Macro for allocating a CSSM_DATA_PTR */
#define cssm_MallocCSSMData(fx, len) \
    ( (fx == NULL) \
       ? ((CSSM_DATA_PTR) NULL) : ((CSSM_DATA_PTR) fx (sizeof (uint32) + len)) )

#define cssm_UpcallMallocCSSMData(fx, handle, len) \
    ( (fx == NULL) \
       ? ((CSSM_DATA_PTR) NULL) : ((CSSM_DATA_PTR) fx (handle, sizeof (uint32) + len)) )

/* Time converstion utilities */
char*  cssm_TimeToUtc( const sint32 lTime, char* UTCBuffer );
char*  cssm_TimeToGeneralizedTime( const sint32 lTime, char* GTBuffer );
sint32 cssm_UtcToTime (const char *szTime );
sint32 cssm_GeneralizedTimeToTime( const char *szTime );
sint32 cssm_GetSecondsSince1970();
char*  cssm_GeneralizedTimeToUtc( const char *szGTime, char *szUTCTime );

/* Time type checking utilities */
CSSM_BOOL cssm_IsBadUtc (const char* szTime, sint32 length);
CSSM_BOOL cssm_IsBadGeneralizedTime (const char* szGTime, sint32 length);

#if defined (WIN32)
char * cssm_FiletimeToUtc( const FILETIME FileTime, 
                           const CSSM_BOOL IsLocal, char *szUtcTime);
CSSM_RETURN cssm_UtcToFiletime( const char *pUTCTime, FILETIME *pTime); 
char * cssm_FiletimeToMMDDYY( const FILETIME FileTime, const CSSM_BOOL IsLocal, 
                              char *szUtcTime);
CSSM_RETURN cssm_MMDDYYToFiletime( const char *szMMDDYYTime, FILETIME *pFiletime); 
#endif

#endif
