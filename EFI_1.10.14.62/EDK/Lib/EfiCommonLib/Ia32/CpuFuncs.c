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

    CpuFuncs.c
    
Abstract:

    IA-32 Processor-specific routines

Revision History

--*/
#include "Efi.h"
#include "CpuFuncs.h"

VOID
EfiWriteMsr(
  IN  UINT32      Input,
  IN  UINT64      Value
  )
/*++

Routine Description:

  This will write the value in EDX:EAX to MSR specified by ECX

Arguments:

Returns:

--*/
{
  _asm {
    mov ecx, Input
    mov eax, dword ptr Value[0]
    mov edx, dword ptr Value[4]
    wrmsr
  }
  return;
}

UINT64
EfiReadMsr (
  IN  UINT32  Input
  )
/*++

Routine Description:

  This will load MSR specified by ECX into EDX:EAX

Arguments:

Returns:

--*/
{

  UINT64  Value;

  _asm {
    mov ecx, Input;
    rdmsr
    mov dword ptr Value[0], eax
    mov dword ptr Value[4], edx
  }

  return  Value;
}

VOID
EfiCpuid(
  IN   UINT32               RegEax,
  OUT  EFI_CPUID_REGISTER   *Reg
  )
/*++

Routine Description:

  This will read the CPUID specified by RegEax.

Arguments:

Returns:

--*/
{
  _asm {
    mov eax, RegEax
    cpuid
    push  edi
    mov edi, dword ptr Reg
    mov dword ptr [edi].RegEax, eax
    mov dword ptr [edi].RegEbx, ebx
    mov dword ptr [edi].RegEcx, ecx
    mov dword ptr [edi].RegEdx, edx
    pop edi
  }

  return;

}

UINT64
EfiReadTsc ()
{
  UINT64    Value;

  _asm {
    rdtsc
    mov dword ptr Value[0], eax
    mov dword ptr Value[4], edx
  }

  return Value;
}





