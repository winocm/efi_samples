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
//  iva.s
//
// Abstract:
//
//  This provides read/write access to the cr.IVA
//
// Revision History:
//
//--

.file  "Iva.s"

//.text
.section rtcode, "ax", "progbits"

// EXPORTS
.global ReadIVA
.type ReadIVA, @function

// UINT64
// ReadIVA (
//   )
//
// Routine Description:
//
//   Read the IVA and return in R8
//
// Arguments:
//
//   None
//
// Returns:
//
//   IVA in R8
//
//
        
  .type    RtPlGetRtSalIVA, @function
  .global  RtPlGetRtSalIVA

  .type    RtSetVirtualAddressMapInternal, @function
  .global  RtSetVirtualAddressMapInternal

  .proc  RtSetVirtualAddressMap
RtSetVirtualAddressMap::
  alloc  loc0 = ar.pfs, 8, 3, 8, 0 ;;  // 4 in, 3 local, 4 out, 0 rot
  mov    loc1 = b0 ;;

  mov   loc2 = cr.iva ;;                        // Save OS IVA locally

  br.call.sptk      b0 = RtPlGetRtSalIVA ;;     // Load the old SAL IVA
  mov   cr.iva = r8 ;;
  
  mov   out0 = in0 ;;                           // Call through to the real proc
  mov   out1 = in1 ;;                           // Using 8 outs guarantees we get
  mov   out2 = in2 ;;                           // all params passed through
  mov   out3 = in3 ;;
  mov   out4 = in4 ;;
  mov   out5 = in5 ;;
  mov   out6 = in6 ;;
  mov   out7 = in7 ;;
  br.call.sptk      b0 = RtSetVirtualAddressMapInternal ;;
  
  mov   cr.iva = loc2 ;;                        // Put the IVA back

  mov   b0 = loc1 ;;    
  mov   ar.pfs = loc0 ;;
  br.ret.sptk b0 ;;
  .endp  RtSetVirtualAddressMap


// EXPORTS
.global ReadIVA
.type ReadIVA, @function

// UINT64
// ReadIVA (
//   )
//
// Routine Description:
//
//   Read the IVA and return in R8
//
// Arguments:
//
//   None
//
// Returns:
//
//   IVA in R8
//
//
ReadIVA::
  mov r8 = cr.iva ;;
  br.ret.sptk b0 ;;



//.global WriteIVA
//.type WriteIVA, @function
//
//// VOID
//// WriteIVA (
////   IN UINT64  IVA
////   )
////
//// Routine Description:
////
////   Writes to the IVA
////
//// Arguments:
////
////   IVA -- New value to write to IVA
////
//// Returns:
////
////   None
////
////
//WriteIVA::
//  alloc  r13 = ar.pfs, 1, 0, 0, 0  // 3 in, 0 local, 0 out, 0 rot
//  mov    cr.iva = r32 ;;
//  mov    ar.pfs = r13 ;;
//  br.ret.sptk b0 ;;
