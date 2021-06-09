//++
// Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
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
//   EbcLowLevel.s
//
// Abstract:
//
//   Contains low level routines for the Virtual Machine implementation
//   on an Itanium-based platform.
//
//
//--

.file  "EbcLowLevel.s"

#define PROCEDURE_ENTRY(name)   .##text;            \
                                .##type name, @function;    \
                                .##proc name;           \
name::

#define PROCEDURE_EXIT(name)    .##endp name

// Note: use of NESTED_SETUP requires number of locals (l) >= 3

#define NESTED_SETUP(i,l,o,r) \
         alloc loc1=ar##.##pfs,i,l,o,r ;\
         mov loc0=b0

#define NESTED_RETURN \
         mov b0=loc0 ;\
         mov ar##.##pfs=loc1 ;;\
         br##.##ret##.##dpnt  b0;;


//-----------------------------------------------------------------------------
//++
// EbcAsmLLCALLEX
//
//	Implements the low level EBC CALLEX instruction. Sets up the
//  stack pointer, does the spill of function arguments, and
//  calls the native function. On return it restores the original
//  stack pointer and returns to the caller.
//
// Arguments : 
//
// On Entry :
//    in0 = Address of native code to call
//	  in1 = New stack pointer
//
// Return Value: 
// 
// As per static calling conventions. 
// 
//--
//---------------------------------------------------------------------------
;// void EbcAsmLLCALLEX (UINTN FunctionAddr, UINTN EbcStackPointer)
PROCEDURE_ENTRY(EbcAsmLLCALLEX)
	NESTED_SETUP (2,6,8,0)

  // NESTED_SETUP uses loc0 and loc1 for context save

  //
  // Save a copy of the EBC VM stack pointer
  //
  mov r8 = in1;;

  //
  // Copy stack arguments from EBC stack into registers. 
  // Assume worst case and copy 8.
  //
  ld8   out0 = [r8], 8;;
  ld8   out1 = [r8], 8;;
  ld8   out2 = [r8], 8;;
  ld8   out3 = [r8], 8;;
  ld8   out4 = [r8], 8;;
  ld8   out5 = [r8], 8;;
  ld8   out6 = [r8], 8;;
  ld8   out7 = [r8], 8;;

  //
  // Save the original stack pointer
  //
  mov   loc2 = r12;

  //
  // Save the gp
  //
  or    loc3 = r1, r0

  //
  // Set the new aligned stack pointer. Reserve space for the required 
  // 16-bytes of scratch area as well.
  //
  add  r12 = 48, in1

  //
  // Now call the function. Load up the function address from the descriptor
  // pointed to by in0. Then get the gp from the descriptor at the following
  // address in the descriptor.
  //
  ld8   r31 = [in0], 8;;
  ld8   r30 = [in0];;
  mov   b1 = r31
  mov   r1 = r30
  (p0) br.call.dptk.many b0 = b1;;

  //
  // Restore the original stack pointer and gp
  //
  mov   r12 = loc2
  or    r1 = loc3, r0

  //
  // Now return
	//
	NESTED_RETURN

PROCEDURE_EXIT(EbcAsmLLCALLEX)

//
// UINTN EbcLLGetEbcEntryPoint(VOID)
//
// Description:
//    Simply return, so that the caller retrieves the return register
//    contents (R8). That's where the thunk-to-ebc code stuffed the
//    EBC entry point.
//
PROCEDURE_ENTRY(EbcLLGetEbcEntryPoint)
		br.ret.sptk	b0 ;;
PROCEDURE_EXIT(EbcLLGetEbcEntryPoint)

//
// INT64 EbcLLGetReturnValue(VOID)
//
// Description:
//		This function is called to get the value returned by native code
// 		to EBC. It simply returns because the return value should still
//		be in the register, so the caller just gets the unmodified value.
//
PROCEDURE_ENTRY(EbcLLGetReturnValue)
		br.ret.sptk	b0 ;;
PROCEDURE_EXIT(EbcLLGetReturnValue)

//
// UINTN EbcLLGetStackPointer(VOID)
//
PROCEDURE_ENTRY(EbcLLGetStackPointer)
		mov		r8 = r12 ;;
    br.ret.sptk	b0 ;;
    br.sptk.few b6 
PROCEDURE_EXIT(EbcLLGetStackPointer)

