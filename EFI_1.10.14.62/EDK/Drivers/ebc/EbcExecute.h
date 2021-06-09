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

  EbcExecute.h

Abstract:

  Header file for Virtual Machine support. Contains EBC defines that can
  be of use to a disassembler for the most part. Also provides function 
  prototypes for VM functions.

--*/

#ifndef _EBC_EXECUTE_H_
#define _EBC_EXECUTE_H_

//
// Macros to check and set alignment
//
#define ASSERT_ALIGNED(addr, size)    ASSERT(!((UINT32)(addr) & (size - 1)))
#define IS_ALIGNED(addr, size)        !((UINT32)(addr) & (size - 1))

//
// Define a macro to get the operand. Then we can change it to be either a
// direct read or have it call a function to read memory.
//
#define GETOPERANDS(pVM)      (UINT8)(*(UINT8 *)(pVM->Ip + 1))
#define GETOPCODE(pVM)        (UINT8)(*(UINT8 *)pVM->Ip)

//
// Bit masks for opcode encodings
//
#define OPCODE_M_OPCODE       0x3F    // bits of interest for first level decode
#define OPCODE_M_IMMDATA      0x80
#define OPCODE_M_IMMDATA64    0x40
#define OPCODE_M_64BIT        0x40    // for CMP
#define OPCODE_M_RELADDR      0x10    // for CALL instruction
#define OPCODE_M_CMPI32_DATA  0x80    // for CMPI
#define OPCODE_M_CMPI64       0x40    // for CMPI 32 or 64 bit comparison


#define OPERAND_M_MOVIN_N           0x80
#define OPERAND_M_CMPI_INDEX        0x10

//
// Masks for instructions that encode presence of indexes for operand1 and/or
// operand2.
//
#define OPCODE_M_IMMED_OP1    0x80
#define OPCODE_M_IMMED_OP2    0x40

//
// Bit masks for operand encodings
//
#define OPERAND_M_INDIRECT1   0x08
#define OPERAND_M_INDIRECT2   0x80
#define OPERAND_M_OP1         0x07
#define OPERAND_M_OP2         0x70

//
// Masks for data manipulation instructions
//
#define DATAMANIP_M_64        0x40    // 64-bit width operation
#define DATAMANIP_M_IMMDATA   0x80

//
// For MOV instructions, need a mask for the opcode when immediate
// data applies to R2.
//
#define OPCODE_M_IMMED_OP2    0x40

//
// The MOVI/MOVIn instructions use bit 6 of operands byte to indicate
// if an index is present. Then bits 4 and 5 are used to indicate the width
// of the move.
//
#define MOVI_M_IMMDATA        0x40
#define MOVI_M_DATAWIDTH      0xC0
#define MOVI_DATAWIDTH16      0x40
#define MOVI_DATAWIDTH32      0x80
#define MOVI_DATAWIDTH64      0xC0
#define MOVI_M_MOVEWIDTH      0x30
#define MOVI_MOVEWIDTH8       0x00
#define MOVI_MOVEWIDTH16      0x10
#define MOVI_MOVEWIDTH32      0x20
#define MOVI_MOVEWIDTH64      0x30

//
// Masks for CALL instruction encodings
//
#define OPERAND_M_RELATIVE_ADDR     0x10
#define OPERAND_M_NATIVE_CALL       0x20

//
// Masks for decoding push/pop instructions
//
#define PUSHPOP_M_IMMDATA    0x80     // opcode bit indicating immediate data
#define PUSHPOP_M_64         0x40     // opcode bit indicating 64-bit operation

//
// Mask for operand of JMP instruction
//
#define JMP_M_RELATIVE        0x10
#define JMP_M_CONDITIONAL     0x80
#define JMP_M_CS              0x40

//
// Macros to determine if a given operand is indirect
//
#define OPERAND1_INDIRECT(op)   ((op) & OPERAND_M_INDIRECT1)
#define OPERAND2_INDIRECT(op)   ((op) & OPERAND_M_INDIRECT2)

//
// Macros to extract the operands from second byte of instructions
//
#define OPERAND1_REGNUM(op)           ((op) & OPERAND_M_OP1)
#define OPERAND2_REGNUM(op)           (((op) & OPERAND_M_OP2) >> 4)

#define OPERAND1_CHAR(op)             ('0' + OPERAND1_REGNUM(op))
#define OPERAND2_CHAR(op)             ('0' + OPERAND2_REGNUM(op))

#define OPERAND1_REGDATA(pvm, op)     pvm->R[OPERAND1_REGNUM(op)]
#define OPERAND2_REGDATA(pvm, op)     pvm->R[OPERAND2_REGNUM(op)]

//
// Condition masks usually for byte 1 encodings of code
//
#define CONDITION_M_CONDITIONAL   0x80
#define CONDITION_M_CS            0x40

//
// Bits in the VM->StopFlags field
//
#define STOPFLAG_APP_DONE         0x0001
#define STOPFLAG_BREAKPOINT       0x0002
#define STOPFLAG_INVALID_BREAK    0x0004
#define STOPFLAG_BREAK_ON_CALLEX  0x0008

//
// Masks for working with the VM flags register
//
#define VMFLAGS_CC              0x0001  // condition flag
#define VMFLAGS_STEP            0x0002  // step instruction mode

#define VMFLAGS_ALL_VALID       (VMFLAGS_CC | VMFLAGS_STEP)

//
// Macros for operating on the VM flags register
//
#define VMFLAG_SET(pVM, Flag)         (pVM->Flags |= (Flag))
#define VMFLAG_ISSET(pVM, Flag)       ((pVM->Flags & (Flag)) ? 1 : 0)
#define VMFLAG_CLEAR(pVM, Flag)       (pVM->Flags &= ~(Flag))

//
// Debug macro
//
#define EBCMSG(s)    gST->ConOut->OutputString (gST->ConOut, s)

//
// Define OPCODES
//
#define OPCODE_BREAK        0x00  
#define OPCODE_JMP          0x01
#define OPCODE_JMP8         0x02
#define OPCODE_CALL         0x03
#define OPCODE_RET          0x04
#define OPCODE_CMPEQ        0x05
#define OPCODE_CMPLTE       0x06
#define OPCODE_CMPGTE       0x07
#define OPCODE_CMPULTE      0x08
#define OPCODE_CMPUGTE      0x09
#define OPCODE_NOT          0x0A
#define OPCODE_NEG          0x0B
#define OPCODE_ADD          0x0C
#define OPCODE_SUB          0x0D
#define OPCODE_MUL          0x0E
#define OPCODE_MULU         0x0F
#define OPCODE_DIV          0x10
#define OPCODE_DIVU         0x11
#define OPCODE_MOD          0x12
#define OPCODE_MODU         0x13
#define OPCODE_AND          0x14
#define OPCODE_OR           0x15
#define OPCODE_XOR          0x16
#define OPCODE_SHL          0x17
#define OPCODE_SHR          0x18
#define OPCODE_ASHR         0x19
#define OPCODE_EXTNDB       0x1A
#define OPCODE_EXTNDW       0x1B
#define OPCODE_EXTNDD       0x1C
#define OPCODE_MOVBW        0x1D
#define OPCODE_MOVWW        0x1E
#define OPCODE_MOVDW        0x1F
#define OPCODE_MOVQW        0x20
#define OPCODE_MOVBD        0x21
#define OPCODE_MOVWD        0x22
#define OPCODE_MOVDD        0x23
#define OPCODE_MOVQD        0x24
#define OPCODE_MOVSNW       0x25  // Move signed natural with word index
#define OPCODE_MOVSND       0x26  // Move signed natural with dword index
//#define OPCODE_27         0x27
#define OPCODE_MOVQQ        0x28  // Does this go away?
#define OPCODE_LOADSP       0x29
#define OPCODE_STORESP      0x2A
#define OPCODE_PUSH         0x2B
#define OPCODE_POP          0x2C
#define OPCODE_CMPIEQ       0x2D
#define OPCODE_CMPILTE      0x2E
#define OPCODE_CMPIGTE      0x2F
#define OPCODE_CMPIULTE     0x30
#define OPCODE_CMPIUGTE     0x31
#define OPCODE_MOVNW        0x32
#define OPCODE_MOVND        0x33
//#define OPCODE_34         0x34
#define OPCODE_PUSHN        0x35
#define OPCODE_POPN         0x36
#define OPCODE_MOVI         0x37  
#define OPCODE_MOVIN        0x38
#define OPCODE_MOVREL       0x39

EFI_STATUS 
EbcExecute(
  IN VM_CONTEXT *VmPtr
  );

//
// Math library routines
//
INT64
DivS64x64 (
  IN INT64      Value1,
  IN INT64      Value2,
  OUT INT64     *Remainder,
  OUT UINT32    *Error
  );

UINT64
DivU64x64 (
  IN UINT64   Value1,
  IN UINT64   Value2,
  OUT UINT64  *Remainder,
  OUT UINT32  *Error
  );

INT64
MulS64x64 (
  IN INT64  Value1,
  IN INT64  Value2,
  OUT INT64 *ResultHigh
  );

UINT64 
MulU64x64 (
  IN UINT64   Value1, 
  IN UINT64   Value2, 
  OUT UINT64  *ResultHigh
  );

INT64
ARightShift64 (
  IN INT64  Operand,
  IN INT64  Count
  );
  
UINT64
LeftShiftU64 (
  IN UINT64   Operand,
  IN UINT64   Count
  );

UINT64
RightShiftU64 (
  IN UINT64   Operand,
  IN UINT64   Count
  );

UINT64 
GetVmVersion ();

//
// Define a protocol for an EBC VM test interface.
//
#define EFI_EBC_VM_TEST_PROTOCOL_GUID \
    {0xAAEACCFDL, 0xF27B, 0x4C17, 0xB6, 0x10, 0x75, 0xCA, 0x1F, 0x2D, 0xFB, 0x52} 

//
// Define for forward reference.
//
EFI_INTERFACE_DECL(_EFI_EBC_VM_TEST_PROTOCOL);

typedef 
EFI_STATUS
(* EBC_VM_TEST_EXECUTE) (
  IN struct _EFI_EBC_VM_TEST_PROTOCOL *This,
  IN VM_CONTEXT                       *VmPtr,
  IN OUT UINTN                        *InstructionCount
  );

typedef 
EFI_STATUS
(* EBC_VM_TEST_ASM) (
  IN struct _EFI_EBC_VM_TEST_PROTOCOL *This,
  IN CHAR16                           *AsmText,
  IN OUT INT8                         *Buffer,
  IN OUT UINTN                        *BufferLen
  );

typedef 
EFI_STATUS
(* EBC_VM_TEST_DASM) (
  IN struct _EFI_EBC_VM_TEST_PROTOCOL *This,
  IN OUT CHAR16                       *AsmText,
  IN OUT INT8                         *Buffer,
  IN OUT UINTN                        *Len
  );

//
// Prototype for the actual EBC test protocol interface
//
typedef struct _EFI_EBC_VM_TEST_PROTOCOL {
  EBC_VM_TEST_EXECUTE      Execute;
  EBC_VM_TEST_ASM          Assemble;
  EBC_VM_TEST_DASM         Disassemble;
} EFI_EBC_VM_TEST_PROTOCOL;

EFI_STATUS 
EbcExecuteInstructions (
  IN EFI_EBC_VM_TEST_PROTOCOL *This,
  IN VM_CONTEXT               *VmPtr,
  IN OUT UINTN                *InstructionCount
  );

#endif // ifndef _EBC_EXECUTE_H_

