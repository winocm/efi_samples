/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  EbcSupport.c

Abstract:

  This module contains EBC support routines that are customized based on
  the target processor.

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

//
// To support the EFI debug support protocol
//
#include EFI_PROTOCOL_DEFINITION(Ebc)
#include EFI_PROTOCOL_DEFINITION(DebugSupport)

#include "EbcInt.h"
#include "EbcExecute.h"

#define VM_STACK_SIZE               (1024 * 32)

#define EBC_THUNK_SIZE              128

//
// For code execution, thunks must be aligned on 16-byte boundary
//
#define EBC_THUNK_ALIGNMENT         16

//
// Per the IA-64 Software Conventions and Runtime Architecture Guide, 
// section 3.3.4, IPF stack must always be 16-byte aligned.
//
#define IPF_STACK_ALIGNMENT         16

//
// Opcodes for IPF instructions. We'll need to hand-create thunk code (stuffing
// bits) to insert a jump to the interpreter.
//
#define OPCODE_NOP                (UINT64)0x00008000000
#define OPCODE_BR_COND_SPTK_FEW   (UINT64)0x00100000000
#define OPCODE_MOV_BX_RX          (UINT64)0x00E00100000

//
// Opcode for MOVL instruction
//
#define MOVL_OPCODE     0x06

VOID 
EbcAsmLLCALLEX (
  IN UINTN    CallAddr, 
  IN UINTN    EbcSp
  );

static
EFI_STATUS
WriteBundle (
  IN    VOID    *MemPtr,
  IN    UINT8   Template,
  IN    UINT64  Slot0,
  IN    UINT64  Slot1,
  IN    UINT64  Slot2
  );

static 
VOID 
PushU64 (
  VM_CONTEXT *VmPtr, 
  UINT64 Arg
  )
{
  //
  // Advance the VM stack down, and then copy the argument to the stack.
  // Hope it's aligned.
  //
  VmPtr->R[0] -= sizeof(UINT64);
  *(UINT64 *)VmPtr->R[0] = Arg;
}
UINT64 
EbcInterpret (
  UINT64      Arg1,
  ...
  )
{
  //
  // Create a new VM context on the stack
  //
  VM_CONTEXT      VmContext;
  UINTN           Addr;
  VA_LIST         List;
  UINT64          Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8;
  UINTN           Arg9Addr;
  //
  // Get the EBC entry point from the processor register. Make sure you don't
  // call any functions before this or you could mess up the register the
  // entry point is passed in.
  //
  Addr = EbcLLGetEbcEntryPoint ();
  //
  // Need the args off the stack.
  //
  VA_START (List, Arg1);
  Arg2 = VA_ARG(List, UINT64);
  Arg3 = VA_ARG(List, UINT64);
  Arg4 = VA_ARG(List, UINT64);
  Arg5 = VA_ARG(List, UINT64);
  Arg6 = VA_ARG(List, UINT64);
  Arg7 = VA_ARG(List, UINT64);
  Arg8 = VA_ARG(List, UINT64);
  Arg9Addr = (UINTN)List;
  //
  // Now clear out our context
  //
  EfiZeroMem ((VOID *)&VmContext, sizeof (VM_CONTEXT));  
  //
  // Set the VM instruction pointer to the correct location in memory.
  //
  VmContext.Ip = (VMIP)Addr;
  //
  // Initialize the stack pointer for the EBC. Get the current system stack
  // pointer and adjust it down by the max needed for the interpreter.
  //
  Addr = (UINTN)Arg9Addr;
  //
  // NOTE: Eventually we should have the interpreter allocate memory
  //       for stack space which it will use during its execution. This
  //       would likely improve performance because the interpreter would
  //       no longer be required to test each memory access and adjust 
  //       those reading from the stack gap.
  //
  // For IPF, the stack looks like (assuming 10 args passed)
  //   arg10
  //   arg9       (Bottom of high stack)
  //   [ stack gap for interpreter execution ]
  //   [ magic value for detection of stack corruption ]
  //   arg8       (Top of low stack)
  //   arg7....
  //   arg1
  //   [ 64-bit return address ] 
  //   [ ebc stack ]
  // If the EBC accesses memory in the stack gap, then we assume that it's
  // actually trying to access args9 and greater. Therefore we need to
  // adjust memory accesses in this region to point above the stack gap.
  //
  VmContext.HighStackBottom = (UINTN)Addr;
  //
  // Now adjust the EBC stack pointer down to leave a gap for interpreter 
  // execution. Then stuff a magic value there.
  //
  VmContext.R[0] = (UINT64)Addr;
  VmContext.R[0] -= VM_STACK_SIZE;
  PushU64 (&VmContext, (UINT64)VM_STACK_KEY_VALUE);
  VmContext.StackMagicPtr = (UINTN *)VmContext.R[0];
  VmContext.LowStackTop = (UINTN)VmContext.R[0];
  //
  // Push the EBC arguments on the stack. Does not matter that they may not
  // all be valid. 
  //
  PushU64 (&VmContext, Arg8);
  PushU64 (&VmContext, Arg7);
  PushU64 (&VmContext, Arg6);
  PushU64 (&VmContext, Arg5);
  PushU64 (&VmContext, Arg4);
  PushU64 (&VmContext, Arg3);
  PushU64 (&VmContext, Arg2);
  PushU64 (&VmContext, Arg1);
  //
  // Push a bogus return address on the EBC stack because the
  // interpreter expects one there. For stack alignment purposes on IPF,
  // EBC return addresses are always 16 bytes. Push a bogus value as well.
  //
  PushU64 (&VmContext, 0);
  PushU64 (&VmContext, 0xDEADBEEFDEADBEEF);
  VmContext.StackRetAddr = (UINT64)VmContext.R[0];
  //
  // Begin executing the EBC code
  //
  EbcExecute (&VmContext);
  //
  // Return the value in R[7] unless there was an error
  //
  return (UINT64)VmContext.R[7];
}

UINT64
ExecuteEbcImageEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  IPF implementation.

  Begin executing an EBC image. The address of the entry point is passed
  in via a processor register, so we'll need to make a call to get the
  value.
  
Arguments:

  ImageHandle   - image handle for the EBC application we're executing
  SystemTable   - standard system table passed into an driver's entry point

Returns:

  The value returned by the EBC application we're going to run.

--*/
{
  //
  // Create a new VM context on the stack
  //
  VM_CONTEXT      VmContext;
  UINTN           Addr;

  //
  // Get the EBC entry point from the processor register. Make sure you don't
  // call any functions before this or you could mess up the register the
  // entry point is passed in.
  //
  Addr = EbcLLGetEbcEntryPoint ();

  //
  // Now clear out our context
  //
  EfiZeroMem ((VOID *)&VmContext, sizeof (VM_CONTEXT));  

  //
  // Save the image handle so we can track the thunks created for this image
  //
  VmContext.ImageHandle = ImageHandle;
  VmContext.SystemTable = SystemTable;
    
  //
  // Set the VM instruction pointer to the correct location in memory.
  //
  VmContext.Ip = (VMIP)Addr;

  //
  // Get the stack pointer. This is the bottom of the upper stack.
  //
  Addr = EbcLLGetStackPointer ();
  VmContext.HighStackBottom = (UINTN)Addr;
  VmContext.R[0] = (INT64)Addr;

  //
  // Allocate stack space for the interpreter. Then put a magic value
  // at the bottom so we can detect stack corruption.
  //
  VmContext.R[0] -= VM_STACK_SIZE;
  PushU64 (&VmContext, (UINT64)VM_STACK_KEY_VALUE);
  VmContext.StackMagicPtr = (UINTN *)(UINTN)VmContext.R[0];

  //
  // When we thunk to external native code, we copy the last 8 qwords from
  // the EBC stack into the processor registers, and adjust the stack pointer
  // up. If the caller is not passing 8 parameters, then we've moved the
  // stack pointer up into the stack gap. If this happens, then the caller
  // can mess up the stack gap contents (in particular our magic value). 
  // Therefore, leave another gap below the magic value. Pick 10 qwords down,
  // just as a starting point.
  //
  VmContext.R[0] -= 10 * sizeof (UINT64);

  //
  // Align the stack pointer such that after pushing the system table, 
  // image handle, and return address on the stack, it's aligned on a 16-byte
  // boundary as required for IPF.
  //
  VmContext.R[0] &= (INT64)~0x0f;
  VmContext.LowStackTop = (UINTN)VmContext.R[0];
  //
  // Simply copy the image handle and system table onto the EBC stack.
  // Greatly simplifies things by not having to spill the args
  //
  PushU64 (&VmContext, (UINT64)SystemTable);
  PushU64 (&VmContext, (UINT64)ImageHandle);

  //
  // Interpreter assumes 64-bit return address is pushed on the stack.
  // IPF does not do this so pad the stack accordingly. Also, a 
  // "return address" is 16 bytes as required for IPF stack alignments.
  //
  PushU64 (&VmContext, (UINT64)0);
  PushU64 (&VmContext, (UINT64)0x1234567887654321);
  VmContext.StackRetAddr = (UINT64)VmContext.R[0];

  //
  // Begin executing the EBC code
  //
  EbcExecute (&VmContext);

  //
  // Return the value in R[7] unless there was an error
  //
  return (UINT64)VmContext.R[7];
}
EFI_STATUS 
EbcCreateThunks  (
  IN EFI_HANDLE   ImageHandle,
  IN VOID         *EbcEntryPoint,
  OUT VOID        **Thunk,
  IN  UINT32      Flags
  )
/*++

Routine Description:

  Create thunks for an EBC image entry point, or an EBC protocol service.
  
Arguments:

  ImageHandle     - Image handle for the EBC image. If not null, then we're
                    creating a thunk for an image entry point.
  EbcEntryPoint   - Address of the EBC code that the thunk is to call
  Thunk           - Returned thunk we create here
  Flags           - Flags indicating options for creating the thunk
  
Returns:

  Standard EFI status.
  
--*/
{
  UINT8         *Ptr, *ThunkBase;
  UINT64        Addr;
  UINT64        Code[3];    // Code in a bundle
  UINT64        RegNum;     // register number for MOVL
  UINT64        I;          // bits of MOVL immediate data
  UINT64        Ic;         // bits of MOVL immediate data
  UINT64        Imm5c;      // bits of MOVL immediate data
  UINT64        Imm9d;      // bits of MOVL immediate data
  UINT64        Imm7b;      // bits of MOVL immediate data
  UINT64        Br;         // branch register for loading and jumping
  UINT64        *Data64Ptr;
  UINT32        ThunkSize, Size;
  EFI_STATUS    Status;
        
  //
  // Check alignment of pointer to EBC code, which must always be aligned
  // on a 2-byte boundary.
  //
  if ((UINT32)(UINTN)EbcEntryPoint & 0x01) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Allocate memory for the thunk. Make the (most likely incorrect) assumption
  // that the returned buffer is not aligned, so round up to the next
  // alignment size.
  //
  Size = EBC_THUNK_SIZE + EBC_THUNK_ALIGNMENT - 1;
  ThunkSize = Size;
  Status = gBS->AllocatePool (EfiBootServicesData,
                              Size,
                              (VOID *)&Ptr);
  if (Status != EFI_SUCCESS) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Save the start address of the buffer.
  //
  ThunkBase = Ptr;

  //
  // Make sure it's aligned for code execution. If not, then
  // round up.
  //
  if ((UINT32)(UINTN)Ptr & (EBC_THUNK_ALIGNMENT - 1)) {
    Ptr = (UINT8 *)(((UINTN)Ptr + (EBC_THUNK_ALIGNMENT - 1)) & 
                    ~(UINT64)(EBC_THUNK_ALIGNMENT-1));
  }
  //
  // Return the pointer to the thunk to the caller to user as the
  // image entry point.
  //
  *Thunk = (VOID *)Ptr;

  // Clear out the thunk entry
  //EfiZeroMem(Ptr, Size); 

  //
  // For IPF, when you do a call via a function pointer, the function pointer
  // actually points to a function descriptor which consists of a 64-bit 
  // address of the function, followed by a 64-bit gp for the function being
  // called. See the the Software Conventions and Runtime Architecture Guide
  // for details. 
  // So first off in our thunk, create a descriptor for our actual thunk code.
  // This means we need to create a pointer to the thunk code (which follows 
  // the descriptor we're going to create), followed by the gp of the Vm
  // interpret function we're going to eventually execute.
  //
  Data64Ptr = (UINT64 *)Ptr;

  //
  // Write the function's entry point (which is our thunk code that follows
  // this descriptor we're creating).
  //
  *Data64Ptr = (UINT64)(Data64Ptr + 2);
  //
  // Get the gp from the descriptor for EbcInterpret and stuff it in our thunk
  // descriptor.
  //
  *(Data64Ptr + 1) = *(UINT64 *)((UINT64 *)(UINTN)EbcInterpret + 1);
  //
  // Advance our thunk data pointer past the descriptor. Since the 
  // descriptor consists of 16 bytes, the pointer is still aligned for
  // IPF code execution (on 16-byte boundary).
  //
  Ptr += sizeof (UINT64) * 2;

  //*************************** FIRST BUNDLE ********************************
  //
  // Write code bundle for: movl r8 = EBC_ENTRY_POINT so we pass
  // the ebc entry point in to the interpreter function via a processor
  // register.
  // Note -- we could easily change this to pass in a pointer to a structure
  // that contained, among other things, the EBC image's entry point. But
  // for now pass it directly.
  //
  Addr = (UINT64)EbcEntryPoint;

  //
  // Now generate the code bytes. First is nop.m 0x0
  //
  Code[0] = OPCODE_NOP;

  //
  // Next is simply Addr[62:22] (41 bits) of the address
  //
  Code[1] = RightShiftU64 (Addr, 22) & 0x1ffffffffff;

  //
  // Extract bits from the address for insertion into the instruction
  // i = Addr[63:63]
  //
  I = RightShiftU64 (Addr, 63) & 0x01;
  //
  // ic = Addr[21:21]
  //
  Ic = RightShiftU64 (Addr, 21) & 0x01;
  //
  // imm5c = Addr[20:16] for 5 bits
  //
  Imm5c = RightShiftU64 (Addr, 16) & 0x1F;
  //
  // imm9d = Addr[15:7] for 9 bits
  //
  Imm9d = RightShiftU64 (Addr, 7) & 0x1FF;
  //
  // imm7b = Addr[6:0] for 7 bits
  //
  Imm7b = Addr & 0x7F;

  //
  // Put the EBC entry point in r8, which is the location of the return value
  // for functions.
  //
  RegNum = 8;

  //
  // Next is jumbled data, including opcode and rest of address
  //
  Code[2] = LeftShiftU64 (Imm7b, 13)
          | LeftShiftU64 (0x00, 20)   // vc
          | LeftShiftU64 (Ic, 21)
          | LeftShiftU64 (Imm5c, 22)
          | LeftShiftU64 (Imm9d, 27)
          | LeftShiftU64 (I, 36)
          | LeftShiftU64 ((UINT64)MOVL_OPCODE, 37)
          | LeftShiftU64 ((RegNum & 0x7F), 6);

  WriteBundle ((VOID *)Ptr, 0x05, Code[0], Code[1], Code[2]);

  //*************************** NEXT BUNDLE *********************************
  //
  // Write code bundle for: 
  //   movl rx = offset_of(EbcInterpret|ExecuteEbcImageEntryPoint)
  //
  // Advance pointer to next bundle, then compute the offset from this bundle
  // to the address of the entry point of the interpreter.
  //
  Ptr += 16; 
  if (Flags & FLAG_THUNK_ENTRY_POINT) {
    Addr = (UINT64)ExecuteEbcImageEntryPoint;
  } else {
    Addr = (UINT64) EbcInterpret;
  }
  //
  // Indirection on Itanium-based systems
  //
  Addr = *(UINT64 *)Addr;

  //
  // Now write the code to load the offset into a register
  //
  Code[0] = OPCODE_NOP;

  //
  // Next is simply Addr[62:22] (41 bits) of the address
  //
  Code[1] = RightShiftU64 (Addr, 22) & 0x1ffffffffff;

  //
  // Extract bits from the address for insertion into the instruction
  // i = Addr[63:63]
  //
  I = RightShiftU64 (Addr, 63) & 0x01;
  //
  // ic = Addr[21:21]
  //
  Ic = RightShiftU64 (Addr, 21) & 0x01;
  //
  // imm5c = Addr[20:16] for 5 bits
  //
  Imm5c = RightShiftU64 (Addr, 16) & 0x1F;
  //
  // imm9d = Addr[15:7] for 9 bits
  //
  Imm9d = RightShiftU64 (Addr, 7) & 0x1FF;
  //
  // imm7b = Addr[6:0] for 7 bits
  //
  Imm7b = Addr & 0x7F;

  //
  // Put it in r31, a scratch register
  //
  RegNum = 31;

  //
  // Next is jumbled data, including opcode and rest of address
  //
  Code[2] =   LeftShiftU64(Imm7b, 13)
          | LeftShiftU64 (0x00, 20)   // vc
          | LeftShiftU64 (Ic, 21)
          | LeftShiftU64 (Imm5c, 22)
          | LeftShiftU64 (Imm9d, 27)
          | LeftShiftU64 (I, 36)
          | LeftShiftU64 ((UINT64)MOVL_OPCODE, 37)
          | LeftShiftU64 ((RegNum & 0x7F), 6);

  WriteBundle ((VOID *)Ptr, 0x05, Code[0], Code[1], Code[2]);

  //*************************** NEXT BUNDLE *********************************
  //
  // Load branch register with EbcInterpret() function offset from the bundle
  // address: mov b6 = RegNum
  //
  // See volume 3 page 4-29 of the Arch. Software Developer's Manual.
  //
  // Advance pointer to next bundle
  //
  Ptr += 16; 
  Code[0] = OPCODE_NOP;
  Code[1] = OPCODE_NOP;
  Code[2] = OPCODE_MOV_BX_RX;
  
  //
  // Pick a branch register to use. Then fill in the bits for the branch
  // register and user register (same user register as previous bundle).
  //
  Br = 6;
  Code[2] |= LeftShiftU64(Br, 6);
  Code[2] |= LeftShiftU64(RegNum, 13);
  WriteBundle ((VOID *)Ptr, 0x0d, Code[0], Code[1], Code[2]);

  //*************************** NEXT BUNDLE *********************************
  //
  // Now do the branch:  (p0) br.cond.sptk.few b6
  //
  // Advance pointer to next bundle.
  // Fill in the bits for the branch register (same reg as previous bundle)
  //
  Ptr += 16; 
  Code[0] = OPCODE_NOP;
  Code[1] = OPCODE_NOP;
  Code[2] = OPCODE_BR_COND_SPTK_FEW;
  Code[2] |= LeftShiftU64 (Br, 13);
  WriteBundle ((VOID *)Ptr, 0x1d, Code[0], Code[1], Code[2]);

  //
  // Add the thunk to our list of allocated thunks so we can do some cleanup
  // when the image is unloaded. Do this last since the Add function flushes
  // the instruction cache for us.
  //
  EbcAddImageThunk (ImageHandle, (VOID *)ThunkBase, ThunkSize);

  //
  // Done
  //
  return EFI_SUCCESS;
}
static 
EFI_STATUS
WriteBundle (
  IN    VOID    *MemPtr,
  IN    UINT8   Template,
  IN    UINT64  Slot0,
  IN    UINT64  Slot1,
  IN    UINT64  Slot2
  )
/*++

Routine Description:

  Given raw bytes of Itanium based code, format them into a bundle and
  write them out.
  
Arguments:

  MemPtr    - pointer to memory location to write the bundles to
  Template   - 5-bit template
  Slot0-2   - instruction slot data for the bundle

Returns:

  Standard EFI status
  
--*/
{
  UINT8   *BPtr;
  UINT32  i;
  UINT64  Low64;
  UINT64  High64;

  //
  // Verify pointer is aligned
  //
  if ((UINT64)MemPtr & 0xF) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Verify no more than 5 bits in template
  //
  if (Template & ~0x1F) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Verify max of 41 bits used in code
  //
  if ((Slot0 | Slot1 | Slot2) & ~0x1ffffffffff) {
    return EFI_INVALID_PARAMETER;
  }

  Low64 = LeftShiftU64 (Slot1, 46) | LeftShiftU64 (Slot0, 5) | Template;
  High64 = RightShiftU64 (Slot1, 18) | LeftShiftU64 (Slot2, 23);

  //
  // Now write it all out
  //
  BPtr = (UINT8 *)MemPtr;
  for (i = 0; i < 8; i++) {
    *BPtr = (UINT8)Low64;
    Low64 = RightShiftU64 (Low64, 8);
    BPtr++;
  }
  for (i = 0; i < 8; i++) {
    *BPtr = (UINT8)High64;
    High64 = RightShiftU64 (High64, 8);
    BPtr++;
  }
  return EFI_SUCCESS;
}
VOID 
EbcLLCALLEX (
  IN UINTN    CallAddr, 
  IN UINTN    EbcSp,
  IN VOID     *FramePtr
  )
/*++

Routine Description:
  Implements the EBC CALLEX instruction to call an external function, which
  may be native or EBC code.

  We'll copy the entire EBC stack frame down below itself in memory and use
  that copy for passing parameters. 

Arguments:
  CallAddr    - address (function pointer) of function to call
  EbcSp       - current EBC stack pointer
  FramePtr    - current EBC frame pointer.

Returns:
  NA

--*/
{
  UINTN   FrameSize;
  VOID    *Destination;
  VOID    *Source;
  // 
  // The stack for an EBC function looks like this:
  //     FramePtr  (8)
  //     RetAddr   (8)
  //     Locals    (n)
  //     Stack for passing args (m)
  //
  // Pad the frame size with 64 bytes because the low-level code we call
  // will move the stack pointer up assuming worst-case 8 args in registers.
  //
  FrameSize = (UINTN)FramePtr - (UINTN)EbcSp + 64;
  Source = (VOID *)EbcSp;
  Destination = (VOID *)((UINT8 *)EbcSp - FrameSize - IPF_STACK_ALIGNMENT);
  Destination = (VOID *)((UINTN)((UINTN)Destination + IPF_STACK_ALIGNMENT - 1) & ~((UINTN)IPF_STACK_ALIGNMENT - 1));
  gBS->CopyMem (Destination, Source, FrameSize);
  EbcAsmLLCALLEX ((UINTN)CallAddr, (UINTN)Destination);    
}
