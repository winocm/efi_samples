/*-----------------------------------------------------------------------
 *      File:   cserricl.h
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
/*
 * This is the error file for csp token adaptation layer.
 */

#ifndef	_CSERRICL_H
#define	_CSERRICL_H

/****************************************************************************/
/* Private csp error messages												*/
/****************************************************************************/

#define CSSM_CSP_PRIKEY_EXPORT_FAILED			(CSSM_CSP_PRIVATE_ERROR +1L)
#define CSSM_CSP_PRIKEY_IMPORT_FAILED			(CSSM_CSP_PRIVATE_ERROR +2L)
#define CSSM_CSP_PRIKEY_DELETE_FAILED			(CSSM_CSP_PRIVATE_ERROR +3L)
#define CSSM_CSP_PRIKEY_VIEW_FAILED				(CSSM_CSP_PRIVATE_ERROR +3L)
#define CSSM_CSP_PRIKEY_WRAP_FAILED				(CSSM_CSP_PRIVATE_ERROR +4L)
#define CSSM_CSP_PRIKEY_WRAP_NO_METHOD          (CSSM_CSP_PRIVATE_ERROR +5L)
#define CSSM_CSP_PRIKEY_UNWRAP_FAILED			(CSSM_CSP_PRIVATE_ERROR +6L)
#define CSSM_CSP_PRIKEY_UNWRAP_NO_METHOD        (CSSM_CSP_PRIVATE_ERROR +7L)

#define CSSM_CSP_PRIKEYFILE_SET_FAILED			(CSSM_CSP_PRIVATE_ERROR +21L)
#define CSSM_CSP_PRIKEYFILE_GET_FAILED			(CSSM_CSP_PRIVATE_ERROR +22L)

#define CSSM_CSP_PASSWORD_CHANGE_FAILED			(CSSM_CSP_PRIVATE_ERROR +40L)

/****************************************************************************/
#endif
