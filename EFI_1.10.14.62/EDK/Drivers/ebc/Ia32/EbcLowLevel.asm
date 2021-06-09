  page    ,132
  title   VM ASSEMBLY LANGUAGE ROUTINES
;****************************************************************************
;*                                                                          *
;*  Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved       *
;*  This software and associated documentation (if any) is furnished        *
;*  under a license and may only be used or copied in accordance            *
;*  with the terms of the license. Except as permitted by such              *
;*  license, no part of this software or documentation may be               *
;*  reproduced, stored in a retrieval system, or transmitted in any         *
;*  form or by any means without the express written consent of             *
;*  Intel Corporation.                                                      *
;*                                                                          *
;****************************************************************************
;****************************************************************************
;                                   REV 1.0
;****************************************************************************
;
; Rev  Date      Description
; ---  --------  ------------------------------------------------------------
; 1.0  03/14/01  Initial creation of file.
;
;****************************************************************************
                             
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
; This code provides low level routines that support the Virtual Machine
; for option ROMs. 
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

;---------------------------------------------------------------------------
; Equate files needed.
;---------------------------------------------------------------------------

.XLIST

.LIST

;---------------------------------------------------------------------------
; Assembler options
;---------------------------------------------------------------------------

.686p
.model  flat        
.code        
;---------------------------------------------------------------------------
;;GenericPostSegment      SEGMENT USE16
;---------------------------------------------------------------------------

;****************************************************************************
; EbcLLCALLEX
;
; This function is called to execute an EBC CALLEX instruction. 
; This instruction requires that we thunk out to external native
; code. For IA32, we simply switch stacks and jump to the 
; specified function. On return, we restore the stack pointer
; to its original location.
;
; Destroys no working registers.
;****************************************************************************
; VOID EbcLLCALLEX(UINTN FuncAddr, UINTN NewStackPointer, VOID *FramePtr)
_EbcLLCALLEX        PROC    NEAR    PUBLIC
			push 	ebp
      mov		ebp, esp							; standard function prolog
      
			; Get function address in a register
			; mov ecx, FuncAddr => mov ecx, dword ptr [FuncAddr]
      mov		ecx, dword ptr [esp]+8

			; Set stack pointer to new value
			; mov eax, NewStackPointer => mov eax, dword ptr [NewSp]
      mov		eax, dword ptr [esp] + 0Ch
      mov		esp, eax      

			; Now call the external routine
      call	ecx
      
      ; ebp is preserved by the callee. In this function it
      ; equals the original esp, so set them equal
      mov		esp, ebp

			; Standard function epilog
      mov			esp, ebp
      pop			ebp
      ret
_EbcLLCALLEX		ENDP


; UINTN EbcLLGetEbcEntryPoint(VOID);
; Routine Description:
; 	The VM thunk code stuffs an EBC entry point into a processor
;   register. Since we can't use inline assembly to get it from
;   the interpreter C code, stuff it into the return value 
;   register and return.
;
; Arguments:
;   	None.
;
; Returns:
;     The contents of the register in which the entry point is passed.
;
_EbcLLGetEbcEntryPoint        PROC    NEAR    PUBLIC
		ret
_EbcLLGetEbcEntryPoint		ENDP

;/*++
;
;Routine Description:
;  
;  Return the caller's value of the stack pointer.
;
;Arguments:
;
;  None.
;
;Returns:
;
;  The current value of the stack pointer for the caller. We
;  adjust it by 4 here because when they called us, the return address
;  is put on the stack, thereby lowering it by 4 bytes.
;
;--*/

; UINTN EbcLLGetStackPointer()            
_EbcLLGetStackPointer        PROC    NEAR    PUBLIC
		mov		eax, esp      ; get current stack pointer
    add   eax, 4        ; stack adjusted by this much when we were called
    ret
_EbcLLGetStackPointer		ENDP

; UINT64 EbcLLGetReturnValue(VOID);
; Routine Description:
; 	When EBC calls native, on return the VM has to stuff the return
;   value into a VM register. It's assumed here that the value is still
;		in the register, so simply return and the caller should get the
;   return result properly.
;
; Arguments:
;   	None.
;
; Returns:
;     The unmodified value returned by the native code.
;
_EbcLLGetReturnValue   PROC    NEAR    PUBLIC
		ret
_EbcLLGetReturnValue		ENDP

END
