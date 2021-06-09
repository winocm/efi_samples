/* SCCSID: inc/cssmdefs.h, dss_cdsa_fwk, fwk_rel1 1.11 9/2/97 11:35:04 */
/*
 * (C) COPYRIGHT International Business Machines Corp. 1996, 1997
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _CSSMDEFS_H
#define _CSSMDEFS_H

#include "trc_util.h"

#if 0

/* Do not edit the next line, and keep integrity defines after it */
/*---INTEGRITY SECTION---*/
#if 0
#define ISL_INCLUDED                       
/* Bilaterial Authentication (req. for CSP Filtering) */
#define ISL_SELF_CHECK                     
/* Self Check */
#define ISL_CSP_FILTERING 

#define OBFUSCATION_ENABLED                                
   /* CSP Manifest Filtering (must include ISL_INCLUDED) */

#define CSSM_CHECK_KR_POLICY_MODULE_INTEGRITY
#endif

/*---TRACE SECTION---*/
#if 1
	#define DO_TRACE
	#if 0
		#define BINARY_TRACE
	#endif
#endif

#endif
#endif
