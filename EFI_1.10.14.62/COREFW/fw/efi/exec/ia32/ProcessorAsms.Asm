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
;    Processor.c
;
; Abstract:
;
; BugBug:
; - Masm uses "This", "ebx", etc as a directive.
; - H2INC is still not embedded in our build process so I translated the struc manually.
; - Unreferenced variables/arguments (This, NewBsp) were causing compile errors and did not know of "pragma" mechanism in MASM and I did not want to reduce the warning level. Instead, I did a dummy referenced.
;

  .686P
  .MMX
  .MODEL SMALL
  .CODE

EFI_SUCCESS                     equ     0
EFI_WARN_RETURN_FROM_LONG_JUMP  equ     1

_EFI_JUMP_BUFFER                STRUCT 2t
_ebx            DWORD           ?
_esi            DWORD           ?
_edi            DWORD           ?
_ebp            DWORD           ?
_esp            DWORD           ?
_eip            DWORD           ?
_EFI_JUMP_BUFFER                ENDS

EFI_JUMP_BUFFER         TYPEDEF         _EFI_JUMP_BUFFER

;
;Routine Description:
;
;  This routine implements the IA32 variant of the SetJump call.  Its
;  responsibility is to store system state information for a possible
;  subsequent LongJump.
;
;Arguments:
;
;  Pointer to CPU context save buffer.
;
;Returns:
;
;  EFI_SUCCESS
;
SetJump      PROC  C \
  Jump:PTR EFI_JUMP_BUFFER
    
  mov   ecx, Jump
  mov   (EFI_JUMP_BUFFER PTR [ecx])._ebx, ebx
  mov   (EFI_JUMP_BUFFER PTR [ecx])._esi, esi
  mov   (EFI_JUMP_BUFFER PTR [ecx])._edi, edi
  mov   eax, [ebp]
  mov   (EFI_JUMP_BUFFER PTR [ecx])._ebp, eax
  lea   eax, [ebp+4]
  mov   (EFI_JUMP_BUFFER PTR [ecx])._esp, eax
  mov   eax, [ebp+4]
  mov   (EFI_JUMP_BUFFER PTR [ecx])._eip, eax
  mov   eax, EFI_SUCCESS
  
  ret
  
SetJump      ENDP

;
; Routine Description:
; 
;  This routine implements the IA32 variant of the LongJump call.  Its
;  responsibility is restore the system state to the Context Buffer and
;  pass control back.
;
; Arguments:
; 
;  Pointer to CPU context save buffer.
;
; Returns:
;
;  EFI_WARN_RETURN_FROM_LONG_JUMP
;

LongJump     PROC  C \
        Jump:PTR EFI_JUMP_BUFFER

  push  ebx
  push  esi
  push  edi

    ; set return from SetJump to EFI_WARN_RETURN_FROM_LONG_JUMP
  mov   eax, EFI_WARN_RETURN_FROM_LONG_JUMP          
  mov   ecx, Jump
  mov   ebx, (EFI_JUMP_BUFFER PTR [ecx])._ebx
  mov   esi, (EFI_JUMP_BUFFER PTR [ecx])._esi
  mov   edi, (EFI_JUMP_BUFFER PTR [ecx])._edi
  mov   ebp, (EFI_JUMP_BUFFER PTR [ecx])._ebp
  mov   esp, (EFI_JUMP_BUFFER PTR [ecx])._esp
  add   esp, 4                                       ;pop the eip
  jmp   DWORD PTR (EFI_JUMP_BUFFER PTR [ecx])._eip
  mov   eax, EFI_WARN_RETURN_FROM_LONG_JUMP
  
  pop   edi
  pop   esi
  pop   ebx
  ret
  
LongJump     ENDP

SwitchStacks    PROC  C \
  EntryPoint:PTR DWORD, \
  Parameter:DWORD, \
  NewStack:PTR DWORD, \
  NewBsp:PTR DWORD
  
  push  ebx
  mov   eax, NewBsp
  mov   ebx, Parameter
  mov   ecx, EntryPoint
  mov   eax, NewStack
  mov   esp, eax
  push  ebx
  push  0
  jmp   ecx
  
  pop   ebx
  ret
  
SwitchStacks    ENDP

SwitchIplStacks PROC  C \
  EntryPoint:PTR DWORD, \
  Parameter1:DWORD, \
  Parameter2:DWORD, \
  NewStack:PTR DWORD, \
  NewBsp:PTR DWORD
  
  push  ebx
  mov   eax, NewBsp 
  mov   ebx, Parameter1
  mov   edx, Parameter2
  mov   ecx, EntryPoint
  mov   eax, NewStack
  mov   esp, eax

  push  edx
  push  ebx
  call  ecx
  
  pop   ebx
  ret
  
SwitchIplStacks ENDP

  END

