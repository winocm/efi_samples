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
  
  sei.h

Abstract:


Revision History

--*/
#ifndef _SEI_INCLUDE_
#define _SEI_INCLUDE_
//#include "cssmport.h"

#define IN
#define OUT

#ifdef EFI64
typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef          __int16 sint16;
typedef unsigned __int32 uint32;
typedef          __int32 sint32;
#else

/* Basic Types */
#ifndef uint8
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned int uint32;
typedef int sint32;
#endif
typedef double uint64;
#endif
/* hyper on in wtypes.h
#define uint64 unsigned int[2] 
#define sint64 int[2] */

typedef enum {
    SEI_PASS = 0,
    SEI_FAIL = 1,
    SEI_NOTAVAILABLE = 2,
    SEI_NOTSUPPORTED = 3,
    SEI_ACCESSDENIED = 4,
    SEI_BUSY = 5,
    SEI_SEQUENCEERROR = 6,
    SEI_INTERNALERROR = 7
} SEI_RETURN;

SEI_RETURN SeiGetRandNum (IN uint8 Size,
                          IN uint32 Seed,
                          OUT uint64 *Buffer);

SEI_RETURN SeiSignWithPlatformKey (IN uint32 SignAlgorithmID,
                                   IN const uint8 *ImagePtr,
                                   IN uint32 ImageLength,
                                   OUT uint8 *SignaturePtr,
                                   OUT uint32 *SignatureLength);

SEI_RETURN SeiVerifyWithPlatformKey (IN uint32 SignAlgorithmID,
                                     IN const uint8 *ImagePtr,
                                     IN uint32 ImageLength,
                                     IN const uint8 *SignaturePtr,
                                     IN uint32 SignatureLength);

#endif
