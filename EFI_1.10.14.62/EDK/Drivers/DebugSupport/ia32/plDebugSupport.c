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

    plDebugSupport.c

Abstract:

    IA32 specific debug support functions

Revision History

--*/

//
// Master EFI header file
//
#include "efi.h"

//
// Common library header files
//
#include "EfiDriverLib.h"

//
// Produced protocols
//
#include EFI_PROTOCOL_DEFINITION (DebugSupport)

//
// private header files
//
#include "plDebugSupport.h"

// This the global main table to keep track of the interrupts
//IDT_ENTRY                   IdtEntryTable[NUM_IDT_ENTRIES];
IDT_ENTRY                   *IdtEntryTable = NULL;
DESCRIPTOR                  NullDesc = 0;


#ifndef EFI_NT_EMULATOR
static EFI_STATUS
CreateEntryStub (
  IN EFI_EXCEPTION_TYPE     ExceptionType,
  OUT void **               Stub
  )
/*++

Routine Description: Allocate pool for a new IDT entry stub.  Copy the generic
    stub into the new buffer and fixup the vector number and jump target address.

Arguments:
    ExceptionType - This is the exception type that the new stub will be created
                    for.
    Stub - On successful exit, *Stub contains the newly allocated entry stub.
Returns:
  Typically EFI_SUCCESS
  other possibilities are passed through from AllocatePool

--*/
{
  EFI_STATUS Status;
  UINT8 * StubCopy;

  //
  // First, allocate a new buffer and copy the stub code into it
  //
  Status = gBS->AllocatePool (EfiBootServicesData, StubSize, Stub);
  if (Status == EFI_SUCCESS) {
    StubCopy = *Stub;
    gBS->CopyMem (StubCopy, InterruptEntryStub, StubSize);

    //
    // Next fixup the stub code for this vector
    //

    // The stub code looks like this:
    //
    //    00000000  89 25 00000004 R  mov     AppEsp, esp             ; save stack top
    //    00000006  BC 00008014 R     mov     esp, offset DbgStkBot   ; switch to debugger stack
    //    0000000B  6A 00             push    0                       ; push vector number - will be modified before installed
    //    0000000D  E9                db      0e9h                    ; jump rel32
    //    0000000E  00000000          dd      0                       ; fixed up to relative address of CommonIdtEntry
    //

    //
    // poke in the exception type so the second push pushes the exception type
    //
    StubCopy[0x0c] = (UINT8) ExceptionType;

    //
    // fixup the jump target to point to the common entry
    //
    *(UINT32* ) &StubCopy[0x0e] =  (UINT32) CommonIdtEntry - (UINT32) &StubCopy[StubSize];
  }
  return Status;
}


static EFI_STATUS
HookEntry (
  IN EFI_EXCEPTION_TYPE           ExceptionType,
  IN VOID                         (*NewCallback) ()
  )
/*++

Routine Description:
  Creates a nes entry stub.  Then saves the current IDT entry and replaces it
  with an interrupt gate for the new entry point.  The IdtEntryTable is updated
  with the new registered function.

  This code executes in boot services context.  The stub entry executes in interrupt
  context.

Arguments:
  ExceptionType - specifies which vector to hook.
  NewCallback - a pointer to the new function to be registered.

Returns:
  EFI_SUCCESS
  Other possibilities are passed through by CreateEntryStub

--*/
{
  BOOLEAN OldIntFlagState;
  EFI_STATUS Status;

  Status = CreateEntryStub (ExceptionType, (void **) &IdtEntryTable[ExceptionType].StubEntry);
  if (Status == EFI_SUCCESS) {
    OldIntFlagState = WriteInterruptFlag(0);
    ReadIdt (ExceptionType, &(IdtEntryTable[ExceptionType].OrigDesc));

    ((UINT16 *) &IdtEntryTable[ExceptionType].OrigVector)[0] = ((UINT16 *) &IdtEntryTable[ExceptionType].OrigDesc)[0];
    ((UINT16 *) &IdtEntryTable[ExceptionType].OrigVector)[1] = ((UINT16 *) &IdtEntryTable[ExceptionType].OrigDesc)[3];

    Vect2Desc (&IdtEntryTable[ExceptionType].NewDesc, IdtEntryTable[ExceptionType].StubEntry);
    IdtEntryTable[ExceptionType].RegisteredCallback = NewCallback;
    WriteIdt (ExceptionType, &(IdtEntryTable[ExceptionType].NewDesc));
    WriteInterruptFlag(OldIntFlagState);
  }
  return Status;
}


static EFI_STATUS
UnhookEntry (
  IN EFI_EXCEPTION_TYPE           ExceptionType
  )
/*++

Routine Description:
  Undoes HookEntry. This code executes in boot services context.

Arguments:
  ExceptionType - specifies which entry to unhook

Returns:
  EFI_SUCCESS
  Other values are passed through from FreePool

--*/
{
  BOOLEAN OldIntFlagState;
  EFI_STATUS Status;

  OldIntFlagState = WriteInterruptFlag(0);
  WriteIdt (ExceptionType, &(IdtEntryTable[ExceptionType].OrigDesc));
  Status = gBS->FreePool ((VOID *)(UINTN)IdtEntryTable[ExceptionType].StubEntry);
  EfiZeroMem (&IdtEntryTable[ExceptionType], sizeof (IDT_ENTRY));
  WriteInterruptFlag(OldIntFlagState);

  return (Status);
}
#endif  


EFI_STATUS
ManageIdtEntryTable (
  VOID (*NewCallback)(),
  EFI_EXCEPTION_TYPE ExceptionType
  )
/*++

Routine Description:
  This is the main worker function that manages the state of the interrupt
  handlers.  It both installs and uninstalls interrupt handlers based on the
  value of NewCallback.  If NewCallback is NULL, then uninstall is indicated.
  If NewCallback is non-NULL, then install is indicated.

Arguments:
  NewCallback - If non-NULL, NewCallback specifies the new handler to register.
                If NULL, specifies that the previously registered handler should
                    be uninstalled.
  ExceptionType - Indicates which entry to manage

Returns:
  EFI_SUCCESS
  EFI_INVALID_PARAMETER - requested uninstalling a handler from a vector that has
    no handler registered for it... or requested install to a vector that
    already has a handler registered.
  Other possible return values are passed through from UnHookEntry and HookEntry.

--*/
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

#ifndef EFI_NT_EMULATOR
  if (CompareDescriptor (&IdtEntryTable[ExceptionType].NewDesc, &NullDesc)) {  // we've already installed to this vector
    if (NewCallback != NULL) {   // if the input handler is non-null, error
      Status = EFI_INVALID_PARAMETER;
    } else {
      Status = UnhookEntry (ExceptionType);
    }
  } else {                      // no user handler installed on this vector
    if (NewCallback == NULL) {   // if the input handler is null, error
      Status = EFI_INVALID_PARAMETER;
    } else {
      Status = HookEntry (ExceptionType, NewCallback);
    }
  }
#endif  
  return Status;
}


EFI_STATUS
GetMaximumProcessorIndex (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  OUT UINTN                           *MaxProcessorIndex
  )
/*++

Routine Description: This is a DebugSupport protocol member function.

Arguments:

Returns: Always returns EFI_SUCCESS with *MaxProcessorIndex set to 0

--*/
{
  *MaxProcessorIndex = 0;
  return (EFI_SUCCESS);
}

EFI_STATUS
RegisterPeriodicCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL *This,
  IN UINTN                      ProcessorIndex,
  IN EFI_PERIODIC_CALLBACK      PeriodicCallback
  )
/*++

Routine Description: This is a DebugSupport protocol member function.

Arguments:

Returns:

--*/
{
  return ManageIdtEntryTable (PeriodicCallback, SYSTEM_TIMER_VECTOR);
}


EFI_STATUS
RegisterExceptionCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL *This,
  IN UINTN                      ProcessorIndex,
  IN EFI_EXCEPTION_CALLBACK     NewCallback,
  IN EFI_EXCEPTION_TYPE         ExceptionType
  )
/*++

Routine Description:
  This is a DebugSupport protocol member function.

  This code executes in boot services context.

Arguments:

Returns:

  None

--*/
{
  return ManageIdtEntryTable (NewCallback, ExceptionType);
}


EFI_STATUS
InvalidateInstructionCache (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  IN UINTN                            ProcessorIndex,
  IN VOID                             *Start,
  IN UINT64                           Length
  )
/*++

Routine Description:
  This is a DebugSupport protocol member function.
  For IA32, this is a no-op since the instruction and data caches are coherent.

Arguments:

Returns:

  None

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
plInitializeDebugSupportDriver (void)
/*++

Routine Description:
  Initializes driver's handler registration database.

  This code executes in boot services context.

Arguments:
  None

Returns:
  EFI_SUCCESS
  EFI_UNSUPPORTED - if IA32 processor does not support FXSTOR/FXRSTOR instructions,
                    the context save will fail, so these processor's are not supported.

--*/
{
  if (FxStorSupport() == FALSE) {
    return EFI_UNSUPPORTED;
  }
  else {
    IdtEntryTable = EfiLibAllocateZeroPool (sizeof (IDT_ENTRY) * NUM_IDT_ENTRIES);
    if (IdtEntryTable != NULL) {
      return EFI_SUCCESS;
    } else {
      return EFI_OUT_OF_RESOURCES;
    }
  }
}


EFI_STATUS
plUnloadDebugSupportDriver (
  IN EFI_HANDLE ImageHandle
  )
/*++

Routine Description:
  This is the callback that is written to the LoadedImage protocol instance
  on the image handle. It uninstalls all registered handlers and frees all entry
  stub memory.

  This code executes in boot services context.

Arguments:
  ImageHandle - The image handle of the unload handler

Returns:

  None

--*/
{
  EFI_EXCEPTION_TYPE      ExceptionType;

  for (ExceptionType = 0; ExceptionType < NUM_IDT_ENTRIES; ExceptionType++) {
    ManageIdtEntryTable (NULL, ExceptionType);
  }
  gBS->FreePool (IdtEntryTable);
  return EFI_SUCCESS;
}


void
InterruptDistrubutionHub (
  EFI_EXCEPTION_TYPE      ExceptionType,
  EFI_SYSTEM_CONTEXT_IA32 *ContextRecord
  )
/*++

Routine Description: Common piece of code that invokes the registered handlers.

  This code executes in exception context so no efi calls are allowed.

Arguments:

Returns:

  None

--*/
{
  if (IdtEntryTable[ExceptionType].RegisteredCallback != NULL) {
    if (ExceptionType != SYSTEM_TIMER_VECTOR) {
      IdtEntryTable[ExceptionType].RegisteredCallback (ExceptionType, ContextRecord);
    } else {
      OrigVector = IdtEntryTable[ExceptionType].OrigVector;
      IdtEntryTable[ExceptionType].RegisteredCallback (ContextRecord);
    }
  }
}




