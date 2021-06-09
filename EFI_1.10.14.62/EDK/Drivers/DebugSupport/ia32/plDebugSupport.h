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

    IA32 specific debug support macros, typedefs and prototypes.

Revision History

--*/

#ifndef _PLDEBUG_SUPPORT_H
#define _PLDEBUG_SUPPORT_H

#include "efi.h"
#include EFI_PROTOCOL_DEFINITION (DebugSupport)

#define NUM_IDT_ENTRIES                 0x78
#define SYSTEM_TIMER_VECTOR             0x68
#define VECTOR_ENTRY_PAGES              1
#define CopyDescriptor(Dest, Src)       EfiCopyMem ((Dest), (Src), sizeof (DESCRIPTOR))
#define ZeroDescriptor(Dest)            CopyDescriptor ((Dest), &NullDesc)
#define ReadIdt(Vector, Dest)           CopyDescriptor ((Dest), &((GetIdtr())[(Vector)]))
#define WriteIdt(Vector, Src)           CopyDescriptor (&((GetIdtr())[(Vector)]), (Src))
#define CompareDescriptor(Desc1, Desc2) EfiCompareMem ((Desc1), (Desc2), sizeof (DESCRIPTOR))
#define EFI_ISA                         IsaIa32
#define FF_FXSR                         (1 << 24)

typedef UINT64 DESCRIPTOR;

typedef struct {
  DESCRIPTOR                OrigDesc;
  VOID                      (*OrigVector)(void);
  DESCRIPTOR                NewDesc;
  VOID                      (*StubEntry) (void);
  VOID                      (*RegisteredCallback) ();
} IDT_ENTRY;

extern EFI_SYSTEM_CONTEXT   SystemContext;
extern UINT8                InterruptEntryStub[];
extern UINT32               StubSize;
extern void                 (*OrigVector) (void);

void
CommonIdtEntry (
  void
  );

BOOLEAN
FxStorSupport (
  void
  );

DESCRIPTOR *
GetIdtr (
  void
  );

void
Vect2Desc (
  DESCRIPTOR * DestDesc,
  void (*Vector) (void)
  );

BOOLEAN
WriteInterruptFlag (
  BOOLEAN NewState
  );

EFI_STATUS
plInitializeDebugSupportDriver (
  void
  );

EFI_STATUS
plUnloadDebugSupportDriver (
  IN EFI_HANDLE                       ImageHandle
  );

//
// DebugSupport protocol member functions
//
EFI_STATUS
GetMaximumProcessorIndex (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  OUT UINTN                           *MaxProcessorIndex
  );
  
EFI_STATUS
RegisterPeriodicCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  IN UINTN                            ProcessorIndex,
  IN EFI_PERIODIC_CALLBACK            PeriodicCallback
  );

EFI_STATUS
RegisterExceptionCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  IN UINTN                            ProcessorIndex,
  IN EFI_EXCEPTION_CALLBACK           NewCallback,
  IN EFI_EXCEPTION_TYPE               ExceptionType
  );

EFI_STATUS
InvalidateInstructionCache (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  IN UINTN                            ProcessorIndex,
  IN VOID                             *Start,
  IN UINT64                           Length
  );

#endif
