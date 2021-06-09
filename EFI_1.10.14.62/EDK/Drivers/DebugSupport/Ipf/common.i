//++
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
// Module Name:
//
//  Common.i
//
// Abstract:
//
//  This is set of useful macros
//
// Revision History:
//
//--

#define NESTED_SETUP(i,l,o,r)               \
         alloc loc1=ar##.##pfs,i,l,o,r ;    \
         mov loc0=b0 ;;


#define NESTED_RETURN                       \
         mov b0=loc0 ;                      \
         mov ar##.##pfs=loc1 ;;             \
         br##.##ret##.##dpnt  b0 ;;

#define MASK(bp,value)  (value << bp)

