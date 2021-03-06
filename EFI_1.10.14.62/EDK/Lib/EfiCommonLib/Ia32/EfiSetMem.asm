      TITLE   EfiSetMem.asm: Optimized setmemory routine

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
;   EfiSetMem.asm
; 
; Abstract:
; 
;   This is the code that supports IA32-optimized SetMem service
;
;------------------------------------------------------------------------------

; PROC:PRIVATE
  .686P
  .MMX
  .MODEL SMALL
  .CODE

EfiCommonLibSetMem  PROTO  C Buffer:PTR DWORD, Count:DWORD, Value:BYTE

;------------------------------------------------------------------------------
;  Procedure:  EfiCommonLibSetMem
;
;   VOID
;   EfiCommonLibSetMem (
;     IN VOID   *Buffer,
;     IN UINTN  Count,
;     IN UINT8  Value
;     )
;
;  Input:  VOID   *Buffer - Pointer to buffer to write
;          UINTN  Count   - Number of bytes to write
;          UINT8  Value   - Value to write
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
EfiCommonLibSetMem  PROC C Buffer:PTR DWORD, Count:DWORD, Value:BYTE
  LOCAL QWordValue:QWORD
  LOCAL MmxSave:QWORD

  
  mov edx, Count
  test edx, edx
  je _SetMemDone

  push edi
  push ebx
  
  mov eax, Buffer
  mov bl, Value
  mov edi, eax
  mov bh, bl
  
  cmp edx, 256
  jb _SetRemindingByte
  
  and al, 07h
  test al, al
  je _SetBlock
  
  mov eax, edi
  shr eax, 3
  inc eax
  shl eax, 3
  sub eax, edi
  cmp eax, edx
  jnb _SetRemindingByte
  
  sub edx, eax
  mov ecx, eax

  mov al, bl
  rep stosb

_SetBlock:
  mov eax, edx
  shr eax, 6
  test eax, eax
  je _SetRemindingByte

  shl eax, 6
  sub edx, eax
  shr eax, 6

  mov WORD PTR QWordValue[0], bx
  mov WORD PTR QWordValue[2], bx
  mov WORD PTR QWordValue[4], bx
  mov WORD PTR QWordValue[6], bx
 
  
  movq  MmxSave, mm0
  movq  mm0, QWordValue

@@:
  movq  QWORD PTR ds:[edi], mm0
  movq  QWORD PTR ds:[edi+8], mm0
  movq  QWORD PTR ds:[edi+16], mm0
  movq  QWORD PTR ds:[edi+24], mm0
  movq  QWORD PTR ds:[edi+32], mm0
  movq  QWORD PTR ds:[edi+40], mm0
  movq  QWORD PTR ds:[edi+48], mm0
  movq  QWORD PTR ds:[edi+56], mm0
  add edi, 64
  dec eax
  jnz @B
  
; Restore mm0
  movq  mm0, MmxSave
  emms                                 ; Exit MMX Instruction
  
_SetRemindingByte:
  mov ecx, edx

  mov eax, ebx
  shl eax, 16
  mov ax, bx
  shr ecx, 2
  rep stosd
  
  mov ecx, edx
  and ecx, 3
  rep stosb
  
  pop ebx
  pop edi

_SetMemDone:
  ret 0

EfiCommonLibSetMem  ENDP
  END
  