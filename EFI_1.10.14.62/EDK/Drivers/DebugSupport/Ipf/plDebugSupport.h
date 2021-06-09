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

    plDebugSupport.h

Abstract:

    IPF specific debugsupport types, macros, and definitions

Revision History

--*/

#ifndef _PLDEBUG_SUPPORT_H
#define _PLDEBUG_SUPPORT_H


#include "Efi.h"
#include EFI_PROTOCOL_DEFINITION (DebugSupport)

//
// processor specific functions that must be public
//

EFI_STATUS
plInitializeDebugSupportDriver (
  VOID
  );

EFI_STATUS
plUnloadDebugSupportDriver (
  IN EFI_HANDLE                   ImageHandle
  );

//
// Assembly worker functions and data referenced from plDebugSupport.c
//

VOID *
GetIva(void);

VOID
HookStub (
  VOID
  );

VOID
ChainHandler (
  VOID
  );

VOID
UnchainHandler (
  VOID
  );

#define DISABLE_INTERRUPTS        0UL
#define ENABLE_INTERRUPTS         0x6000UL
UINT64
ProgramInterruptFlags(
  IN UINT64                       NewInterruptState
  );

VOID
InstructionCacheFlush (
  IN VOID    *StartAddress,
  IN UINTN   SizeInBytes
  );


//
// The remaining definitions comprise the protocol members.
//

#define EFI_ISA                   IsaIpf

EFI_STATUS
GetMaximumProcessorIndex (
  IN EFI_DEBUG_SUPPORT_PROTOCOL   *This,
  OUT UINTN                       *MaxProcessorIndex
  );

EFI_STATUS
RegisterPeriodicCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL   *This,
  IN UINTN                        ProcessorIndex,
  IN EFI_PERIODIC_CALLBACK        PeriodicCallback
  );

EFI_STATUS
RegisterExceptionCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL   *This,
  IN UINTN                        ProcessorIndex,
  IN EFI_EXCEPTION_CALLBACK       NewHandler,
  IN EFI_EXCEPTION_TYPE           ExceptionType
  );

EFI_STATUS
InvalidateInstructionCache (
  IN EFI_DEBUG_SUPPORT_PROTOCOL   *This,
  IN UINTN                        ProcessorIndex,
  IN VOID                         *Start,
  IN UINTN                        Length
  );

#endif
