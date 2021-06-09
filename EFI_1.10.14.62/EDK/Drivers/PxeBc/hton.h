/*

Copyright (c) 1999 - 2002 Intel Corporation.  All rights reserved.

This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module name:
  hton.h

Abstract:
  Byte swapping macros.

*/

#ifndef _HTON_H_
#define _HTON_H_

//
// Only Intel order functions are defined at this time.
//

#define HTONS(v) (UINT16)((((v)<<8)&0xff00)+(((v)>>8)&0x00ff))

#define HTONL(v) (UINT32)((((v)<<24)&0xff000000)+(((v)<<8)&0x00ff0000)+(((v)>>8)&0x0000ff00)+(((v)>>24)&0x000000ff))

#define HTONLL(v) swap64(v)

#define U8PTR(na) ((UINT8 *)&(na))

#define NTOHS(ns) ((UINT16)(((*U8PTR(ns))<<8)+*(U8PTR(ns)+1)))

#define NTOHL(ns) ((UINT32)(((*U8PTR(ns))<<24)+((*(U8PTR(ns)+1))<<16)+((*(U8PTR(ns)+2))<<8)+*(U8PTR(ns)+3)))

#endif /* _HTON_H_ */

/* EOF - hton.h */
