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
  
  isl_parse.h

Abstract:


Revision History

--*/
#include "cssmtype.h"
#include "isltype.h"

#ifndef ISL_PARSE_H
#define ISL_PARSE_H

typedef CSSM_DB_ATTRIBUTE_DATA	ISL_ATTRIBUTE_INFO;
typedef CSSM_DB_ATTRIBUTE_DATA_PTR ISL_ATTRIBUTE_INFO_PTR;

typedef struct isl_attribute_info_group {
	uint32 NumberOfAttributes;
	ISL_ATTRIBUTE_INFO_PTR Attributes;
} ISL_ATTRIBUTE_INFO_GROUP, *ISL_ATTRIBUTE_INFO_GROUP_PTR;

typedef struct isl_section_info {
	ISL_CONST_DATA Image;
	ISL_ATTRIBUTE_INFO_GROUP Attributes;
} ISL_SECTION_INFO, *ISL_SECTION_INFO_PTR;

typedef struct isl_section_info_group{
	uint32 NumberOfSections;
	ISL_SECTION_INFO_PTR Sections;
} ISL_SECTION_INFO_GROUP, *ISL_SECTION_INFO_GROUP_PTR;

ISL_STATUS
isl_JarFileDecode(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	ISL_CONST_DATA Input,
	ISL_SECTION_INFO_GROUP_PTR SectionInfoGroupPtr);
#endif
