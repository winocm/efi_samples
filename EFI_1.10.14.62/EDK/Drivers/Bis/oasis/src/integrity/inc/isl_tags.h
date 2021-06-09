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
  
  isl_tags.h

Abstract:


Revision History

--*/
#ifndef _ISL_TAGS_H
#define _ISL_TAGS_H

#ifndef OASIS
#include "muttags.h"
#else
#define NAMEPROC(w,x,y,z)
#define NAMELOOP(w,x,y,z)
#endif

#define ISL_DSCM_TAG            0xFF
#define ISL_EISL_TAG            0x10

#define ISL_ARCHIVE_TAG         0x00
#define ISL_CERCHAIN_TAG        0x02
#define ISL_CERT_TAG            0x03
#define ISL_GETDLL_TAG          0x04
#define ISL_ITERATOR_TAG        0x05
#define ISL_LOCATOR_TAG         0x06
#define ISL_MPARSE_TAG          0x07
#define ISL_MANIFEST_TAG        0x08
#define ISL_MEMORY_TAG          0x09
#define ISL_MODULE_TAG          0x0a
#define ISL_PK_TAG              0x0b
#define ISL_PKCS_TAG            0x0c
#define ISL_PKCS7SIG_TAG        0x0d
#define ISL_REFLIST_TAG         0x0e
#define ISL_REGISTRY_TAG        0x0f
#define ISL_SECLINK_TAG         0x10
#define ISL_SELFCHECK_TAG       0x11
#define ISL_SIGROOT_TAG         0x12
#define ISL_UTIL_TAG            0x13
#define ISL_X509CERT_TAG        0x14
#define ISL_BER_BN_TAG			0x15

#define CSSM_EISL_TAG			0x20
#define CSSM_CALLOUTS_TAG		0x00
#endif
