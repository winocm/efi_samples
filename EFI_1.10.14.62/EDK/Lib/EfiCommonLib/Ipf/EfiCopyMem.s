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
//  EfiCopyMem.s
//
// Abstract:
//
//  This is a hand-optimized memory copy routine.
//
// Revision History:
//
//--

.file  "EfiCopyMem.s"

// EXPORTS
.global EfiCopyMem
.type EfiCopyMem, @function

.text

// VOID
// EfiCopyMem (
//   IN VOID   *Destination,
//   IN VOID   *Source,
//   IN UINTN  Length
//   )
//
// Routine Description:
//
//   Copy Length bytes from Source to Destination.
//
// Arguments:
//
//   Destination - Target of copy
//
//   Source      - Place to copy from
//
//   Length      - Number of bytes to copy
//
// Returns:
//
//   None
//
//
// Alternate entry point for bcopy. Swap source and target arguments.
EfiCopyMem:
  alloc   r13 = ar.pfs, 3, 0, 0, 0  // 3 in, 0 local, 0 out, 0 rot
  mov r14 = r32
  mov r32 = r33
  ;;
  mov r33 = r14
  ;;

// void bcopy(vm_offset_t src, vm_offset_t tgt, vm_size_t size)
//
// Copy "size" bytes from address "src" to address "tgt"
//
bcopy:
  alloc   r13 = ar.pfs, 3, 0, 0, 0  // 3 in, 0 local, 0 out, 0 rot
  src = in0     // NOTE: reused for rotating data3
  tgt = in1     // NOTE: reused for rotating data2
  size = in2      // NOTE: reused for rotating data1

  src_top = r16
  temp = r17
  rest_size = r18
  count = r19
  tgt8 = r20
  src8 = r21
  data = r22
  data8 = r23
  saved_lc = r31
  saved_pr = r2
        mov     saved_pr=pr;;

  cmp.ltu p6 = src, tgt
  add src_top = src, size
  or  temp = src, tgt
  and rest_size = 0x7, size
  shr.u count = size, 4
  mov r8 = r0
  mov saved_lc = ar.lc
  ;;
  (p6) cmp.gtu p6 = src_top, tgt
  cmp.gtu p7 = 8, size
  (p6) br.cond.dpnt .overlap
  (p7) br.cond.dpnt .less_than_8_bytes
  and temp = 0x7, temp
  add count = -1, count 
  mov ar.ec = 2
  ;; 
  cmp.ne  p8 = temp, r0
  tbit.nz p9 = size, 3
  (p8) br.cond.dpnt .not_8_byte_aligned
  brp.sptk.imp .aligned_loop, .aligned_loop_bb
  ;;

//  
//  High Performance case:  * source and target are both 8 byte aligned
//                          * size >= 8
//  Main Loop moves 16 bytes / clock
//
  cmp.eq  p6 = -1, count
  mov ar.lc = count
  (p6)  br.cond.spnt .after_aligned_loop
  ;;
  add tgt8 = 8, tgt
  add src8 = 8, src
  mov pr.rot = 0x10000 
  ;;
.aligned_loop:
  (p17) st8 [tgt] = data, 16
  (p16) ld8 data = [src], 16 
.aligned_loop_bb:
  (p17) st8 [tgt8] = data8, 16
  (p16) ld8 data8 = [src8], 16
    br.ctop.sptk .aligned_loop
  ;;
.after_aligned_loop:
  (p9)  ld8 data = [src], 8
  ;;
  (p9)  st8 [tgt] = data, 8

//
//  Byte move: * no alignment restrictions on source or target
//             * size < 8
//  Main Loop moves 1 byte / clock
//
.less_than_8_bytes:
  cmp.eq  p6 = rest_size, r0
  add count = -2, rest_size
  (p6)  br.cond.sptk .return
  brp.sptk.imp .rest_loop, .rest_loop
  ;;
  ld1 data = [src], 1
  cmp.eq p7 = -1, count
  (p7)  br.cond.spnt .after_rest_loop
  mov ar.lc = count
  ;;
.rest_loop:
  st1 [tgt] = data, 1
  ld1 data = [src], 1
  br.cloop.sptk .rest_loop
  ;;
.after_rest_loop:
  st1 [tgt] = data, 1
  br.cond.sptk  .return

// 
//  General case: * no alignment restrictions on copy or target
//                * size >= 8
//  Main Loop moves 8 bytes / clock
//

  data3 = r32
  data2 = r33
  data1 = r34
  last_loaded = r36

  all_1 = r13
  start_tgt = r14
  aligned_src = r15
  aligned_tgt = r16
  src_offset = r17
  sdata1 = r18
  sdata2 = r19
  rest_size = r20
  rest_offset = r21
  rshift = r22
  lshift = r23
  count = r24
  start_data = r25
  start_mask = r26
  tgt_data = r27
  tgt_mask = r28
  end_data = r29
  tgt_offset = r30

.not_8_byte_aligned:
  alloc   r13 = ar.pfs, 3, 5, 0, 8  // Make r32-r39 rotating 
  and src_offset = 0x7, src
  and tgt_offset = 0x7, tgt
  and aligned_src = -8, src
  and aligned_tgt = -8, tgt
  add rest_size = -8, size
  ;;
  cmp.leu p6, p7 = tgt_offset, src_offset
  sub rshift = src_offset, tgt_offset
  add rest_size = tgt_offset, rest_size 
  ;;
  (p7)  add rshift = 8, rshift
  ld8 data1 = [aligned_src], 8
  add all_1 = -1, r0
  mov pr.rot = 0x10000    // Start with Stage 1
  shr.u count = rest_size, 3
  brp.sptk.imp .unaligned_loop, .unaligned_loop_bb
  ;;
  (p6)  ld8 data2 = [aligned_src], 8
  (p7)  mov data2 = data1
  shl rshift = rshift, 3
  mov start_tgt = aligned_tgt
  add aligned_tgt = 8, aligned_tgt
  cmp.eq  p8 = count, r0
  ;;
  sub lshift = 64, rshift
  (p6)  shr.u sdata1 = data1, rshift 
  shl tgt_offset = tgt_offset, 3
  and rest_size = 0x7, rest_size
  add count = -1, count
  mov ar.ec = 4     // 4 Pipeline Stages
  ;;
  (p6)  shl sdata2 = data2, lshift 
  (p7)  shl sdata1 = data1, lshift 
  cmp.ne  p9 = rest_size, r0
  shl rest_size = rest_size, 3
  shl start_mask = all_1, tgt_offset
  ;;
  (p6)  or  start_data = sdata1, sdata2
  (p7)  mov start_data = sdata1
  mov ar.lc = count
  cmp.ltu p10 = lshift, rest_size
  (p8)  mov last_loaded = data2 
  (p8)  br.cond.spnt .after_unaligned_loop
  ;;

  //  Pipelined Loop
  //  Stage 1: Load new source word
  //  Stage 2: Shift new and previous source word
  //  Stage 3: Combine new and previous source word
  //  Stage 4: Write combined word to target
.unaligned_loop:
  (p19) st8 [aligned_tgt] = tgt_data, 8 // Stage 4
  (p18) or  tgt_data = sdata1, sdata2 // Stage 3
  (p17) shr.u sdata1 = data1, rshift    // Stage 2
.unaligned_loop_bb:
  (p16) ld8 data3 = [aligned_src], 8  // Stage 1
  (p17) shl sdata2 = data2, lshift    // Stage 2
  br.ctop.sptk  .unaligned_loop;;
.after_unaligned_loop:

  //  Partial write to last target word
  (p10) ld8 data1 = [aligned_src]
    ld8 tgt_data = [start_tgt]
  (p9)  shr.u sdata2 = last_loaded, rshift
  ;;
    and start_data = start_data, start_mask
    andcm tgt_data = tgt_data, start_mask
  (p10) shl sdata1 = data1, lshift  
  ;;
  (p10) or  sdata2 = sdata1, sdata2
  (p9)  ld8 end_data = [aligned_tgt]
  shl tgt_mask = all_1, rest_size
  ;;
  (p9)  andcm sdata2 = sdata2, tgt_mask
  (p9)  and end_data = end_data, tgt_mask
  ;;
  (p9)  or  end_data = end_data, sdata2
    or  start_data = start_data, tgt_data
  ;;
    st8 [start_tgt] = start_data
  (p9)  st8 [aligned_tgt] = end_data
  br.cond.sptk  .return

// 
//  Special case:
//  The target region overlaps with the source region 
//  in a way that would cause us to destroy source data
//  before copying it.
//  To prevent this, we start at the end of the source region 
//  and copy backwards.
//
//  Since this is not a very frequent case, no effort has been
//  made to optimize for unaligned copies.
//
  
  src_top = r16
  temp = r17
  rest_size = r18
  count = r19
  tgt8 = r20
  src8 = r21
  data = r22
  data8 = r23

.overlap:
  add src = src, size
  add   tgt = tgt, size
  mov rest_size = size
  cmp.gtu p7 = 8, size
  (p7) br.cond.dpnt .copy_bytewise
  brp.sptk.imp .dwordwise_loop, .dwordwise_loop_bb
  ;;
  or  temp = src, tgt
  tbit.nz p9, p10 = size, 3
  cmp.eq  p6 = count, r0
  add tgt8 = -16, tgt
  add src8 = -16, src
  ;;
  and temp = 0x7, temp
  add count = -1, count 
  mov ar.ec = 2
  mov pr.rot = 0x10000 
  ;; 
  cmp.ne  p8 = temp, r0
  mov ar.lc = count
  (p8) br.cond.dpnt .copy_bytewise

//  
//  High Performance case:  * source and target are both 8 byte aligned
//                          * size >= 8
//  Main Loop moves 16 bytes / clock
//
  add tgt = -8, tgt
  add src = -8, src 
  (p6)  br.cond.spnt .after_dwordwise_loop
  ;;
.dwordwise_loop:
  (p17) st8 [tgt] = data, -16
  (p16) ld8 data = [src], -16 
.dwordwise_loop_bb:
  (p17) st8 [tgt8] = data8, -16
  (p16) ld8 data8 = [src8], -16
  br.ctop.sptk .dwordwise_loop
  ;;
.after_dwordwise_loop:
  (p9)  ld8 data = [src]
  (p10) add src = 8, src
  (p10) add tgt = 8, tgt
  and     rest_size = 0x7, size
  ;;
  (p9)  st8 [tgt] = data

//
//  Byte move: * no alignment restrictions on source or target
//  Main Loop moves 1 byte / clock
//
.copy_bytewise:
  add count = -2, rest_size
  cmp.eq  p6 = rest_size, r0
  (p6)  br.cond.sptk .return
  add src = -1, src
  brp.sptk.imp .bytewise_loop, .bytewise_loop
  ;;
  add tgt = -1, tgt
  cmp.eq  p7 = -1, count
  ld1 data = [src], -1
  mov ar.lc = count
  (p7)  br.cond.spnt .after_bytewise_loop
  ;;
.bytewise_loop:
  st1 [tgt] = data, -1
  ld1 data = [src], -1
  br.cloop.sptk .bytewise_loop
  ;;
.after_bytewise_loop:
  st1 [tgt] = data, -1
.return:
  mov ar.lc = saved_lc
  mov     pr = saved_pr
  br.ret.sptk b0
