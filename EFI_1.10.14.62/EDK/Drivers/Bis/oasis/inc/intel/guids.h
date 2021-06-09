/*

Copyright (c)  1998 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

*/
/*--------------------------------------*
 * GUIDS.h                              *
 *   guids for Intel Addins to CSSM	    *
 * Generated: 3/17/98            	    *
 *--------------------------------------*/

#ifndef _GUIDS_H
#define _GUIDS_H

#include "cssm.h"

/****************************************************************************/
/*************************   CURRENT GUIDS    *******************************/
/****************************************************************************/
#define intel_preos_cssm_guid          intel_preos_cssm_guid_2_0_0
#ifdef CSSM_BIS
#define intel_preos_csm_guid           intel_preos_csm_bis_guid_2_0_0
#else
#define intel_preos_csm_guid           intel_preos_isr_csm_guid_2_0_0
#endif
#define intel_preos_clm_guid           intel_preos_x509v3_clm_guid_2_0_0
#define intel_preos_nsi_dsm_guid       intel_preos_nsi_dsm_guid_2_0_0
#define intel_preos_tpm_guid           intel_preos_x509v3_tpm_guid_2_0_0
#define intel_preos_port_guid          intel_preos_port_guid_2_0_0
#define intel_preos_vlm_guid           intel_preos_SMv2_vlm_guid_2_0_0

/* ----- Core CSSM ----- */
/* release 2.0 Intel PreOS CSSM: {F38A8021-BDB1-11d1-93C8-00A0C93C3211} */
#define intel_preos_cssm_guid_def 0xf38a8021, 0xbdb1, 0x11d1, { 0x93, 0xc8, 0x0, 0xa0, 0xc9, 0x3c, 0x32, 0x11 }
static CSSM_GUID intel_preos_cssm_guid_2_0_0 = { intel_preos_cssm_guid_def };
#define CSSM_MAJOR (2)
#define CSSM_MINOR (0)

/* ----- CSP ----- */
#define INTEL_CSM_MAJOR_VER	(2)
#define INTEL_CSM_MINOR_VER	(0)

#ifdef CSSM_BIS 
/* release 2.0 Intel BIS PreOS CSP: {F77DDF91-FF0D-11d1-9CB7-00A0C9708162} */
#define intel_preos_csm_bis_guid_def 0xF77ddf91, 0xff0d, 0x11d1, { 0x9c, 0xb7, 0x0, 0xa0, 0xc9, 0x70, 0x81, 0x62 }
static CSSM_GUID intel_preos_csm_bis_guid_2_0_0 =  { intel_preos_csm_bis_guid_def };
#else
/* release 2.0 Intel ISR PreOS CSP: {F38A8022-BDB1-11d1-93C8-00A0C93C3211} */
#define intel_preos_isr_csm_guid_def 0xf38a8022, 0xbdb1, 0x11d1, { 0x93, 0xc8, 0x0, 0xa0, 0xc9, 0x3c, 0x32, 0x11 }
static CSSM_GUID intel_preos_isr_csm_guid_2_0_0 = { intel_preos_isr_csm_guid_def };

/* release 2.0 Intel PSA PreOS CSP: {FFB45C11-FEEF-11d1-9CB7-00A0C9708162} */
#define intel_preos_psa_csm_guid_def 0xffb45c11, 0xfeef, 0x11d1, { 0x9c, 0xb7, 0x0, 0xa0, 0xc9, 0x70, 0x81, 0x62 }
static CSSM_GUID intel_preos_psa_csm_guid_2_0_0 =  { intel_preos_psa_csm_guid_def };
#endif

/* ----- Intel X509v3 CL ----- */
/* release 2.0 Intel PreOS X509v3 CL:  {F38A8023-BDB1-11d1-93C8-00A0C93C3211} */
#define intel_preos_x509v3_clm_guid_def 0xf38a8023, 0xbdb1, 0x11d1, { 0x93, 0xc8, 0x0, 0xa0, 0xc9, 0x3c, 0x32, 0x11 }
static CSSM_GUID intel_preos_x509v3_clm_guid_2_0_0 = {intel_preos_x509v3_clm_guid_def };
#define INTEL_X509V3_CLM_MAJOR_VER	(2)
#define INTEL_X509V3_CLM_MINOR_VER	(0)

#ifndef CSSM_BIS 
/* ----- NSI DL ----- */
/* release 2.0 Intel PreOS NSI DL: {F38A8024-BDB1-11d1-93C8-00A0C93C3211} */
#define intel_preos_nsi_dsm_guid_def 0xf38a8024, 0xbdb1, 0x11d1, { 0x93, 0xc8, 0x0, 0xa0, 0xc9, 0x3c, 0x32, 0x11 }
static CSSM_GUID intel_preos_nsi_dsm_guid_2_0_0 = { intel_preos_nsi_dsm_guid_def };
#define INTEL_NSI_DSM_MAJOR_VER	(2)
#define INTEL_NSI_DSM_MINOR_VER	(0)

/* ----- Intel X509v3 TP ----- */
/* release 2.0 Intel PreOS X509v3 TP:  {C1E38D51-D327-11d1-93D2-00A0C93C3211} */
#define intel_preos_x509v3_tp_guid_def 0xc1e38d51, 0xd327, 0x11d1, { 0x93, 0xd2, 0x0, 0xa0, 0xc9, 0x3c, 0x32, 0x11 }
static CSSM_GUID intel_preos_x509v3_tpm_guid_2_0_0 = {intel_preos_x509v3_tp_guid_def };
#define INTEL_X509V3_TPM_MAJOR_VER	(2)
#define INTEL_X509V3_TPM_MINOR_VER	(0)
#endif

/* ----- Intel SMv2 VL ----- */
/* release 2.0 Intel Signed Manifest v2: {B2CC8C10-F00B-11d1-878B-00A0C91A2629} */
#define intel_preos_SMv2_vl_guid_def 0xb2cc8c10, 0xf00b, 0x11d1, { 0x87, 0x8b, 0x0, 0xa0, 0xc9, 0x1a, 0x26, 0x29 }
static CSSM_GUID intel_preos_SMv2_vlm_guid_2_0_0 = {intel_preos_SMv2_vl_guid_def };
#define INTEL_SMV2_VLM_MAJOR_VER	(2)
#define INTEL_SMV2_VLM_MINOR_VER	(0)

/* ----- Porting Library GUID ----- */
/* release 2.0 PreOS Porting Library GUID: {A32FBD82-FBCC-11CF-8172-00AA00B199DD} */
static  CSSM_GUID intel_preos_port_guid_2_0_0 = 
{ 0xa32fbd82, 0xfbcc, 0x11cf, { 0x81, 0x72, 0x0, 0xaa, 0x0, 0xb1, 0x99, 0xdd } };


#endif
