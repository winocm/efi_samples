//++
// Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
// This software and associated documentation (if any) is furnished
// under a license and may only be used or copied in accordance
// with the terms of the license. Except as permitted by such
// license, no part of this software or documentation may be
// reproduced, stored in a retrieval system, or transmitted in any
// form or by any means without the express written consent of
// Intel Corporation.

//  All Rights Reserved   
//
//  INTEL CORPORATION PROPRIETARY INFORMATION
//
//  This software is supplied under the terms of a license
//  agreement or nondisclosure agreement with Intel Corpo-
//  ration and may not be copied or disclosed except in
//  accordance with the terms of that agreement.
//
//
// Module Name:
//
//  EfiZeroMem.s
//
// Abstract:
//
//
//  This is a hand-optimized zero-memory routine.
//  It will always produce the optimal storage size sequence,
//  according to address alignment and number of bytes to fill.
//  e.g. zeroing 12 bytes from an address that is aligned on a
//  mod 5 boundary will produce the following storage sizes:
//  1 byte -> 2 bytes -> 8 bytes -> 1 byte
//
//  The estimated number of clock cycles for this routine is
//  15              for sizes from 0..15
//  (size/16) + 14  for sizes 16 and above
//  Add to this the penalty for 1 mispredicted branch
//  and any penalties caused by the memory interface for cacheline
//  fills and spills when zeroing large amounts of memory. 
// 
//
// Revision History:
//
//--

.file  "EfiZeroMem.s"

// EXPORTS
.global EfiZeroMem
.text
//
// VOID
// EfiZeroMem (
//   IN VOID   *Buffer,
//   IN UINTN  Length
//   )
//
//
//Routine Description:
//
//  Set Buffer to 0 for Length bytes.
//
//Arguments:
//
//  Buffer  - Memory to set.
//
//  Length  - Number of bytes to set
//
//
//Returns:
//
//  None
//
//
.proc EfiZeroMem
EfiZeroMem:
  alloc   r13 = ar.pfs, 2, 0, 0, 0  // 2 in, 0 local, 0 out, 0 rot
  addr = in0 
  size = in1

  align = r16
  size1 = r17
  size2 = r18
  size3 = r19
  size4 = r20
  mask = r21
  mask2 = r22
  addr8 = r23
  pr_align = r24
  pr_rest = r25
  saved_lc = r26

  // (1) Calculate bitfield with alignment requirements:
  // bit 0: Needs alignment on a 2 byte boundary
  // bit 1: Needs alignment on a 4 byte boundary
  // bit 2: Needs alignment on a 8 byte boundary
  // bit 3: Needs alignment on a 16 byte boundary
  //
  // This bitfield is generated as 0x10 - (addr & 0xf)
  //
  // (2) Calculate alignment qualifiers to allow only
  //     alignment operations that don't exceed the
  //     size of the zero region.
  //     We do this by generating a mask that has all
  //     bits below the position of the most significant
  //     "1" bit in "size" set.
  //     An and operation with this mask will disable
  //     all alignment operations exceeding "size"
  //
  // (3) Merge the result of (1) and (2) together
  //     and copy it into p6..p9       
  //
  // (4) Prepare address and count values for the 
  //     bzero main loop
  //
  // (5) Setup p12..p15 for the epilog
  
  and align = 0xf, addr   // (1)
  shr size1 = size, 1     // (2)
  shr size2 = size, 2     // (2)
  shr size3 = size, 3     // (2)
  shr size4 = size, 4     // (2)
  ;;
  sub align = 0x10, align     // (1)
  or  mask = size1, size2   // (2)
  or  mask2 = size3, size4    // (2)
  cmp.leu p6, p7 = 16, size   // (2)
  ;;  
  (p6) add mask = 15, r0      // (2)
  (p7) or mask = mask, mask2    // (2)
  ;;
  and align = align, mask   // (3)
  add addr8 = 15, addr    // (4)
  brp.sptk.imp .loop, .loop
  ;;
  dep.z pr_align = align, 6, 4    // (3)
  sub size = size, align  // size remaining after alignment
  and addr8 = -16, addr8    // (4)
  ;;
  mov pr = pr_align, 0x3c0    // (3) Set p6 .. p9
  dep.z pr_rest = size, 12, 4   // (5)
  shr.u size = size, 4      // (4)
  ;;

  // Store zeros for aligning the target address.
  // p6..p9 contain the predicates for storing 1, 2, 4, 8 bytes

  (p6) st1 [addr] = r0, 1
  cmp.ne  p10, p11 = size, r0   // (4)
  ;;
  (p7) st2 [addr] = r0, 2
  (p10) adds size = -1, size    // (5)
  add addr8 = 8, addr8    // (4)
  ;;
  (p8) st4 [addr] = r0, 4
  mov saved_lc = ar.lc
  mov ar.lc = size      // (5)
  ;;
  (p9) st8 [addr] = r0, 8
  mov pr = pr_rest, 0xf000    // (5) Set p12..p15
  ;;

  // Main Loop
  // Writes 16 zero bytes per cycle.
  // If we have less than 16 bytes left before entering the loop, 
  // p10 is FALSE and the main loop is skipped.
.loop:
  (p10) st8 [addr] = r0, 16
  (p10) st8 [addr8] = r0, 16
  br.cloop.sptk .loop
  ;;

  //  Now store whatever number of zero's is left.
  //  The 4 lsbs of the remaining size have already
  //  been loaded into p12..p15, so here we go:

  (p15) st8 [addr] = r0, 8
  ;;
  (p14) st4 [addr] = r0, 4
  ;;
  (p13) st2 [addr] = r0, 2
  ;;
  (p12) st1 [addr] = r0, 1
  mov ar.lc = saved_lc
  br.ret.sptk b0
.endp EfiZeroMem

