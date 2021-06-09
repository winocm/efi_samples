/*-----------------------------------------------------------------------
 *      File:   Csp_inc.h
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
 * This file contains the CSP for WfM BIS crypto functions for suppoprted algorithms.
 */

#ifndef	_CSP_INC_H
#define	_CSP_INC_H

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Internal use csp functions                      */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
extern CSSM_RETURN	Csp_DigestData_SHA1(CSSM_CSP_HANDLE CSPHandle, 
                                 const CSSM_DATA_PTR DataBufs_ptr,
                                 uint32 DataBufCount,
                                 CSSM_DATA_PTR Digest_ptr);

extern CSSM_BOOL    Csp_VerifyHash_DSA(
                                 CSSM_CSP_HANDLE CSPHandle, uint32 AlgID,
                                 const CSSM_KEY_PTR PubKey_ptr, 
                                 const CSSM_DATA_PTR DataBufs_ptr,
                                 uint32 DataBufCount,
                                 const CSSM_DATA_PTR Signature_ptr);
#endif /*_CSP_INC_H*/
