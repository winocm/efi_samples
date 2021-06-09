      TITLE   EfiCoreZeroMem.asm: Optimized memory-zero routine

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
;   EfiCoreZeroMem.asm
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

EfiCoreZeroMem  PROTO  C Buffer:PTR DWORD, Count:DWORD

;------------------------------------------------------------------------------
;  Procedure:  EfiCoreZeroMem
;
;   VOID
;   EfiCoreZeroMem (
;     IN VOID   *Buffer,
;     IN UINTN  Length
;     )
;
;  Input:  VOID   *Buffer - Pointer to buffer to clear
;          UINTN  Length  - Number of bytes to clear
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
EfiCoreZeroMem  PROC C Buffer:PTR DWORD, Count:DWORD
	LOCAL	MmxSave:QWORD

  ; Pick up misaligned start bytes (get pointer 4-byte aligned)
_StartByteZero:
  mov   eax, Buffer		
  and   al, 3                       ; check lower 2 bits of address
  test  al, al
  je    _ZeroBlocks           	    ; already aligned?
  cmp   Count, 0     	
  je    _ZeroMemDone                

  ; Clear the byte memory location
  mov   ecx, Buffer		
  mov   BYTE PTR [ecx], 0           
  inc	ecx
  mov   Buffer, ecx    

  ; Decrement our count
  mov   eax, Count
  dec	eax
  mov   Count, eax   
  jmp   _StartByteZero        ; back to top of loop

_ZeroBlocks:
  ; Compute how many 64-byte blocks we can clear 
  mov   ecx, Count
  mov   eax, ecx                    ; copy to eax too
  shr   ecx, 6                      ; convert to 64-byte count
  shl   ecx, 6                      ; convert back to bytes
  sub   eax, ecx                    ; subtract from the original count
  mov   Count, eax  
  shr   ecx, 6                      ; and this is how many 64-byte blocks

  ; If no 64-byte blocks, then skip 
  cmp    ecx, 0
  je    _ZeroRemaining

  ; Save mm0
  movq	MmxSave, mm0

  ; Save edi, then put the buffer pointer into it.
  push  edi
  mov   edi, Buffer

  pxor  mm0, mm0  		    ; Clear mm0 (xor mm0, mm0)
ALIGN  16
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
  movq	mm0, MmxSave

  ; Write back our advanced buffer pointer, then restore edi
  mov     Buffer, edi
  pop     edi  

_ZeroRemaining:
  ; Zero out as many DWORDS as possible
  mov   ecx, Buffer
  mov   edx, Count
_ZeroRemainingLoop:
  cmp   edx, 4
  jb    _ZeroRemainingBytes
  mov   DWORD PTR [ecx], 0            ; write 0 to memory 
  add   ecx, 4                        ; advance Buffer
  sub   edx, 4                        ; decrement count
  jmp   _ZeroRemainingLoop

  ; Pick up remaining as bytes
_ZeroRemainingBytes:
  cmp   edx, 0
  je    _ZeroMemDone
  mov   BYTE PTR [ecx], 0             ; write a zero
  inc	ecx
  dec	edx
  jmp   _ZeroRemainingBytes

_ZeroMemDone:
  ret   
  
EfiCoreZeroMem  ENDP
  END
