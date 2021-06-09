      TITLE   EfiZeroMem.asm: Optimized memory-zero routine

;------------------------------------------------------------------------------
;
; Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
; This software and associated documentation (if any) is furnished
; under a license and may only be used or copied in accordance
; with the terms of the license. Except as permitted by such
; license, no part of this software or documentation may be
; reproduced, stored in a retrieval system, or transmitted in any
; form or by any means without the express written consent of
; Intel Corporation.
;
;
; Module Name:
;
;   EfiZeroMem.asm
; 
; Abstract:
; 
;   This is the code that supports IA32-optimized ZeroMem service
;
;------------------------------------------------------------------------------

; PROC:PRIVATE
  .686P
  .MMX
  .MODEL SMALL
  .CODE

EfiCommonLibZeroMem  PROTO  C Buffer:PTR DWORD, Count:DWORD

;------------------------------------------------------------------------------
;  Procedure:  EfiCommonLibZeroMem
;
;   VOID
;   EfiCommonLibZeroMem (
;     IN VOID   *Buffer,
;     IN UINTN  Count
;     )
;
;  Input:  VOID   *Buffer - Pointer to buffer to clear
;          UINTN  Count  - Number of bytes to clear
;
;  Output: None.
;
;  Saves:
;
;  Modifies:
;
;  Description:  This function is an optimized zero-memory function.
;
;  Notes:  This function tries to zero memory 8 bytes at a time. As a result, 
;          it first picks up any misaligned bytes, then words, before getting 
;          in the main loop that does the 8-byte clears.
;
;------------------------------------------------------------------------------
EfiCommonLibZeroMem  PROC C Buffer:PTR DWORD, Count:DWORD
    LOCAL  MmxSave:QWORD
    
  ; Save edi, then put the buffer pointer into it.
  push  edi
  mov   ecx, Count
  mov   edi, Buffer

  ; Pick up misaligned start bytes (get pointer 4-byte aligned)
_StartByteZero:
  mov   eax, edi    
  and   al, 3                       ; check lower 2 bits of address
  test  al, al
  je    _ZeroBlocks                 ; already aligned?
  cmp   ecx, 0
  je    _ZeroMemDone

  ; Clear the byte memory location
  mov   BYTE PTR [edi], 0           
  inc    edi

  ; Decrement our count
  dec    ecx
  jmp   _StartByteZero        ; back to top of loop

_ZeroBlocks:

  ; Compute how many 64-byte blocks we can clear 
  mov   edx, ecx
  shr   ecx, 6                      ; convert to 64-byte count
  shl   ecx, 6                      ; convert back to bytes
  sub   edx, ecx                    ; subtract from the original count
  shr   ecx, 6                      ; and this is how many 64-byte blocks

  ; If no 64-byte blocks, then skip 
  cmp    ecx, 0
  je    _ZeroRemaining

  ; Save mm0
  movq  MmxSave, mm0

  pxor  mm0, mm0          ; Clear mm0

@@:
  movq  QWORD PTR ds:[edi], mm0
  movq  QWORD PTR ds:[edi+8], mm0
  movq  QWORD PTR ds:[edi+16], mm0
  movq  QWORD PTR ds:[edi+24], mm0
  movq  QWORD PTR ds:[edi+32], mm0
  movq  QWORD PTR ds:[edi+40], mm0
  movq  QWORD PTR ds:[edi+48], mm0
  movq  QWORD PTR ds:[edi+56], mm0
   
  add    edi, 64
  dec    ecx
  jnz    @B
  
; Restore mm0
  movq  mm0, MmxSave
  emms                                 ; Exit MMX Instruction

_ZeroRemaining:
  ; Zero out as many DWORDS as possible
  mov   ecx, edx
  shr   ecx, 2
  xor   eax, eax

  rep stosd

  ; Zero out remaining as bytes
  mov   ecx, edx
  and   ecx, 03

  rep   stosb
 
_ZeroMemDone:
  pop edi

  ret   
  
EfiCommonLibZeroMem  ENDP
  END
