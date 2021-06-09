      TITLE   EfiCoreCopyMem.asm: Optimized memory-copy routine

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
;   EfiCoreCopyMem.asm
; 
; Abstract:
; 
;   This is the code that supports IA32-optimized CopyMem service
;
;------------------------------------------------------------------------------

; PROC:PRIVATE
  .686P
  .MMX
  .MODEL SMALL
  .CODE

EfiCoreCopyMem  PROTO  C Destination:PTR DWORD, Source:PTR DWORD, Count:DWORD

;------------------------------------------------------------------------------
;  VOID
;  EfiCoreCopyMem (
;    IN VOID   *Destination,
;    IN VOID   *Source,
;    IN UINTN  Count
;    )
;------------------------------------------------------------------------------

EfiCoreCopyMem  PROC  C Destination:PTR DWORD, Source:PTR DWORD, Count:DWORD
	LOCAL	MmxSave:QWORD

  mov   ecx, Count

  ; First off, make sure we have no overlap. That is to say,
  ;   if (Source == Destination)           => do nothing
  ;   if (Source + Count <= Destination)   => regular copy
  ;   if (Destination + Count <= Source)   => regular copy
  ;   otherwise, do a reverse copy
  mov   eax, Source
  add   eax, ecx                      ; Source + Count
  cmp   eax, Destination
  jle   _StartByteCopy

  mov   eax, Destination
  add   eax, ecx                      ; Dest + Count
  cmp   eax, Source
  jle   _StartByteCopy

  mov   eax, Source
  cmp   eax, Destination
  je    _CopyMemDone         
  jl    _CopyOverlapped               ; too bad -- overlaps

  ; Pick up misaligned start bytes to get destination pointer 4-byte aligned
_StartByteCopy:
  cmp   DWORD PTR ecx, 0
  je    _CopyMemDone                ; Count == 0, all done
  mov   edx, Destination
  and   dl, 3                       ; check lower 2 bits of address
  test  dl, dl			    
  je    SHORT _CopyBlocks           ; already aligned?

  ; Copy a byte
  mov   edx, Source
  mov   al, BYTE PTR [edx]          ; get byte from Source
  inc   edx
  mov   Source, edx   
  mov   edx, Destination
  mov   BYTE PTR [edx], al          ; write byte to Destination
  inc   edx
  mov   Destination, edx    
  dec	ecx
  jmp   _StartByteCopy        	     ; back to top of loop

_CopyBlocks:
  ; Compute how many 64-byte blocks we can clear 
  mov   eax, ecx                    ; get Count in eax
  shr   eax, 6                      ; convert to 64-byte count
  shl   eax, 6                      ; convert back to bytes
  sub   ecx, eax                    ; subtract from the original count
  shr   eax, 6                      ; and this is how many 64-byte blocks

  ; If no 64-byte blocks, then skip 
  cmp   eax, 0
  je    _CopyRemainingDWords

  ; Save mm0
  movq	MmxSave, mm0

  ; Put source and destination pointers in esi/edi
  push  edi
  push  esi

  mov   edi, Destination
  mov   esi, Source

ALIGN  16
copymmx:
  movq  mm0, QWORD PTR ds:[esi]
  movq  QWORD PTR ds:[edi], mm0
  movq  mm0, QWORD PTR ds:[esi+8]
  movq  QWORD PTR ds:[edi+8], mm0
  movq  mm0, QWORD PTR ds:[esi+16]
  movq  QWORD PTR ds:[edi+16], mm0
  movq  mm0, QWORD PTR ds:[esi+24]
  movq  QWORD PTR ds:[edi+24], mm0
  movq  mm0, QWORD PTR ds:[esi+32]
  movq  QWORD PTR ds:[edi+32], mm0
  movq  mm0, QWORD PTR ds:[esi+40]
  movq  QWORD PTR ds:[edi+40], mm0
  movq  mm0, QWORD PTR ds:[esi+48]
  movq  QWORD PTR ds:[edi+48], mm0
  movq  mm0, QWORD PTR ds:[esi+56]
  movq  QWORD PTR ds:[edi+56], mm0
  add   edi, 64
  add   esi, 64
  dec  eax
  jnz  copymmx
  
; Restore mm0
  movq	mm0, MmxSave

_CopyRemaining:
  ; Copy advanced source and destination pointers back to memory
  mov   Destination, edi      ; write back advanced destination pointer
  mov   Source, esi     ; write back advanced source pointer
  pop   esi
  pop   edi

  ; Copy as many DWORDS as possible
_CopyRemainingDWords:
  cmp   ecx, 4
  jb    _CopyRemainingBytes

  mov   edx, Source
  mov   eax, DWORD PTR [edx]        ; get data from Source
  add   edx, 4                      ; advance Source pointer
  mov   Source, edx   ; write Source pointer back
  mov   edx, Destination
  mov   DWORD PTR [edx], eax        ; write byte to Destination
  add   edx, 4                      ; advance Destination pointer
  mov   Destination, edx    
  sub   ecx, 4                      ; decrement Count
  jmp   _CopyRemainingDWords  	    ; back to top

_CopyRemainingBytes:
  cmp   ecx, 0
  je    _CopyMemDone
  mov   edx, Source
  mov   al, BYTE PTR [edx]          ; get byte from Source
  inc	edx
  mov   Source, edx   
  mov   edx, Destination
  mov   BYTE PTR [edx], al          ; write byte to Destination
  add   edx, 1                      ; advance Destination pointer
  mov   Destination, edx    
  dec	ecx
  jmp   SHORT _CopyRemainingBytes   ; back to top of loop

  ;
  ; We do this block if the source and destination buffers overlap. To
  ; handle it, copy starting at the end of the source buffer and work
  ; your way back. Since this is the atypical case, this code has not
  ; been optimized, and thus simply copies bytes.
  ;
_CopyOverlapped:
   
  ; Move the source and destination pointers to the end of the range
  mov   eax, Source
  add   eax, ecx                      ; Source + Count
  dec	eax
  mov   Source, eax
  mov   eax, Destination
  add   eax, ecx                      ; Dest + Count
  dec	eax
  mov   Destination, eax      

_CopyOverlappedLoop:
  cmp   ecx, 0
  je    _CopyMemDone
  mov   edx, Source
  mov   al, BYTE PTR [edx]          ; get byte from Source
  dec	edx
  mov   Source, edx   
  mov   edx, Destination
  mov   BYTE PTR [edx], al          ; write byte to Destination
  dec   edx
  mov   Destination, edx    
  dec	ecx
  jmp   _CopyOverlappedLoop   	    ; back to top of loop

_CopyMemDone:
  ret
  
EfiCoreCopyMem  ENDP
  END
