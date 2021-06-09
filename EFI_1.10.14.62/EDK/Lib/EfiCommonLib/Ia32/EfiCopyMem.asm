      TITLE   EfiCopyMem.asm: Optimized memory-copy routine

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
;   EfiCopyMem.asm
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

EfiCommonLibCopyMem  PROTO  C Destination:PTR DWORD, Source:PTR DWORD, Count:DWORD

;------------------------------------------------------------------------------
;  VOID
;  EfiCommonLibCopyMem (
;    IN VOID   *Destination,
;    IN VOID   *Source,
;    IN UINTN  Count
;    )
;------------------------------------------------------------------------------

EfiCommonLibCopyMem  PROC  C Destination:PTR DWORD, Source:PTR DWORD, Count:DWORD
  LOCAL  MmxSave:QWORD
  
  ; Put source and destination pointers in esi/edi
  push  esi
  push  edi
  mov   ecx, Count
  mov   esi, Source
  mov   edi, Destination

  ; First off, make sure we have no overlap. That is to say,
  ;   if (Source == Destination)           => do nothing
  ;   if (Source + Count <= Destination)   => regular copy
  ;   if (Destination + Count <= Source)   => regular copy
  ;   otherwise, do a reverse copy
  mov   eax, esi
  add   eax, ecx                      ; Source + Count
  cmp   eax, edi
  jle   _StartByteCopy

  mov   eax, edi
  add   eax, ecx                      ; Dest + Count
  cmp   eax, esi
  jle   _StartByteCopy

  cmp   esi, edi
  je    _CopyMemDone         
  jl    _CopyOverlapped               ; too bad -- overlaps

  ; Pick up misaligned start bytes to get destination pointer 4-byte aligned
_StartByteCopy:
  cmp   ecx, 0
  je    _CopyMemDone                ; Count == 0, all done
  mov   edx, edi
  and   dl, 3                       ; check lower 2 bits of address
  test  dl, dl          
  je    SHORT _CopyBlocks           ; already aligned?

  ; Copy a byte
  mov   al, BYTE PTR [esi]          ; get byte from Source
  mov   BYTE PTR [edi], al          ; write byte to Destination
  dec    ecx
  inc   edi
  inc   esi
  jmp   _StartByteCopy               ; back to top of loop

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
  movq  MmxSave, mm0

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
  dec   eax
  jnz   copymmx
  
; Restore mm0
  movq  mm0, MmxSave
  emms                                 ; Exit MMX Instruction

  ; Copy as many DWORDS as possible
_CopyRemainingDWords:
  cmp   ecx, 4
  jb    _CopyRemainingBytes

  mov   eax, DWORD PTR [esi]        ; get data from Source
  mov   DWORD PTR [edi], eax        ; write byte to Destination
  sub   ecx, 4                      ; decrement Count
  add   esi, 4                      ; advance Source pointer
  add   edi, 4                      ; advance Destination pointer
  jmp   _CopyRemainingDWords        ; back to top

_CopyRemainingBytes:
  cmp   ecx, 0
  je    _CopyMemDone
  mov   al, BYTE PTR [esi]          ; get byte from Source
  mov   BYTE PTR [edi], al          ; write byte to Destination
  dec    ecx
  inc    esi
  inc   edi                      ; advance Destination pointer
  jmp   SHORT _CopyRemainingBytes   ; back to top of loop

  ;
  ; We do this block if the source and destination buffers overlap. To
  ; handle it, copy starting at the end of the source buffer and work
  ; your way back. Since this is the atypical case, this code has not
  ; been optimized, and thus simply copies bytes.
  ;
_CopyOverlapped:
   
  ; Move the source and destination pointers to the end of the range
  add   esi, ecx                      ; Source + Count
  dec    esi
  add   edi, ecx                      ; Dest + Count
  dec    edi

_CopyOverlappedLoop:
  cmp   ecx, 0
  je    _CopyMemDone
  mov   al, BYTE PTR [esi]          ; get byte from Source
  mov   BYTE PTR [edi], al          ; write byte to Destination
  dec    ecx
  dec    esi
  dec   edi
  jmp   _CopyOverlappedLoop         ; back to top of loop

_CopyMemDone:
  pop   edi
  pop   esi

  ret
  
EfiCommonLibCopyMem  ENDP
  END
