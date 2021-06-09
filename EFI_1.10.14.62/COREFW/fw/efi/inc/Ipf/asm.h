// 
// 
// Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
// This software and associated documentation (if any) is furnished
// under a license and may only be used or copied in accordance
// with the terms of the license. Except as permitted by such
// license, no part of this software or documentation may be
// reproduced, stored in a retrieval system, or transmitted in any
// form or by any means without the express written consent of
// Intel Corporation.
// 
// 
//Module Name:
// 
//   asm.h
// 
//Abstract:
// 
//   This module contains generic macros for an assembly writer.
// 
// 
//Revision History
// 
 
#ifndef _ASM_H
#define _ASM_H

#define TRUE        1
#define FALSE       0
#define PROCEDURE_ENTRY(name)   .##text;            \
                .##type name, @function;    \
                .##proc name;           \
name::

#define PROCEDURE_EXIT(name)    .##endp name

#endif  // _ASM_H 
