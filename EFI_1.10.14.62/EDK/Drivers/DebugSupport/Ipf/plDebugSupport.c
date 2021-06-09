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

    PlDebugSupport.c

Abstract:

    IPF specific debug support functions

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

typedef struct {
  UINT64          low;
  UINT64          high;
} BUNDLE;

//
// number of bundles to swap in ivt
//
#define NUM_BUNDLES_IN_STUB 5
#define NUM_IVT_ENTRIES     64

typedef struct {
  BUNDLE                    OrigBundles[NUM_BUNDLES_IN_STUB];
  VOID                      (*RegisteredCallback) ();
} IVT_ENTRY;

static EFI_STATUS
ManageIvtEntryTable (
  IN  EFI_EXCEPTION_TYPE    ExceptionType,
  IN  BUNDLE                NewBundles[4],
  IN  VOID                  (*NewCallback) ()
  );

static VOID
HookEntry (
  IN  EFI_EXCEPTION_TYPE    ExceptionType,
  IN  BUNDLE                NewBundles[4],
  IN  VOID                  (*NewCallback) ()
  );

static VOID
UnhookEntry (
  IN  EFI_EXCEPTION_TYPE    ExceptionType
  );

static VOID
ChainExternalInterrupt (
  IN  VOID                  (*NewCallback) ()
  );

static VOID
UnchainExternalInterrupt (
  VOID
  );

static VOID
GetHandlerEntryPoint (
  UINTN                     HandlerIndex,
  VOID                      **EntryPoint
  );


IVT_ENTRY                   IvtEntryTable[NUM_IVT_ENTRIES];

//
// IPF context record is overallocated by 512 bytes to guarantee a 512 byte alignment exists
// within the buffer and still have a large enough buffer to hold a whole IPF context record.
//
UINT8                       IpfContextBuf[sizeof(EFI_SYSTEM_CONTEXT_IPF) + 512];

//
// The PatchSaveBuffer is used to store the original bundles from the IVT where it is patched
// with the common handler.
//
UINT8                       PatchSaveBuffer[0x400];
UINTN                       ExternalInterruptCount;

EFI_STATUS
plInitializeDebugSupportDriver (
  VOID
  )
/*++

Routine Description:
  IPF specific DebugSupport driver initialization.  Must be public because it's
  referenced from DebugSupport.c

Arguments:

Returns:

  EFI_SUCCESS

--*/
{
  gBS->SetMem (IvtEntryTable, sizeof(IvtEntryTable), 0);
  ExternalInterruptCount = 0;
  return EFI_SUCCESS;
}


EFI_STATUS
plUnloadDebugSupportDriver (
  IN EFI_HANDLE       ImageHandle
  )
/*++

Routine Description:
  Unload handler that is called during UnloadImage() - deallocates pool memory
  used by the driver.  Must be public because it's referenced from DebugSuport.c

Arguments:
  IN EFI_HANDLE       ImageHandle

Returns:

  EFI_STATUS - anything other than EFI_SUCCESS indicates the callback was not registered.

--*/
{
  EFI_EXCEPTION_TYPE  ExceptionType;
  
  for (ExceptionType = 0; ExceptionType < NUM_IVT_ENTRIES; ExceptionType++) {
    ManageIvtEntryTable (ExceptionType, NULL, NULL);
  }
  return EFI_SUCCESS;
}


VOID
CommonHandler (
  IN EFI_EXCEPTION_TYPE ExceptionType,
  IN EFI_SYSTEM_CONTEXT Context
  )
/*++

Routine Description:
  C routine that is called for all registered exceptions.  This is the main
  exception dispatcher.  Must be public because it's referenced from AsmFuncs.s.

Arguments:
  IN EFI_EXCEPTION_TYPE ExceptionType,
  IN EFI_SYSTEM_CONTEXT Context

Returns:

  Nothing
  
--*/
{
  static BOOLEAN InHandler = FALSE;

  DEBUG_CODE (
    if (InHandler != FALSE) {
      EfiDebugPrint (EFI_D_GENERIC, "ERROR: Re-entered debugger!\n"
                                    "       ExceptionType == %X\n"
                                    "       Context       == %X\n"
                                    "       Context.SystemContextIpf->CrIip  == %X\n"
                                    "       Context.SystemContextIpf->CrIpsr == %X\n"
                                    "       InHandler     == %X\n",
                                    ExceptionType, 
                                    Context, 
                                    Context.SystemContextIpf->CrIip,
                                    Context.SystemContextIpf->CrIpsr,
                                    InHandler);
    }
  )
  ASSERT (InHandler == FALSE);
  InHandler = TRUE;
  if (IvtEntryTable[ExceptionType].RegisteredCallback != NULL) {
    if (ExceptionType != EXCEPT_IPF_EXTERNAL_INTERRUPT) {
      IvtEntryTable[ExceptionType].RegisteredCallback(ExceptionType, Context.SystemContextIpf);
    } else {
      IvtEntryTable[ExceptionType].RegisteredCallback(Context.SystemContextIpf);
    }
  } else {
    ASSERT (0);
  }
  InHandler = FALSE;
}


static VOID
GetHandlerEntryPoint (
  UINTN   HandlerIndex, 
  VOID    ** EntryPoint
  )
/*++

Routine Description:
  Given an int number, return the physical address of the entry point in the IFT
  
Arguments:
  UINTN   HandlerIndex, 
  VOID    ** EntryPoint

Returns:

  Nothing
  
--*/
{
  UINT8 * TempPtr;

  //
  // get base address of IVT
  //
  TempPtr = GetIva();

  if (HandlerIndex < 20) {
    //
    // first 20 provide 64 bundles per vector
    //
    TempPtr += 0x400 * HandlerIndex;
  } else {
    //
    // the rest provide 16 bundles per vector
    //
    TempPtr += 0x5000 + 0x100 * (HandlerIndex - 20);
  }

  * EntryPoint = (VOID *) TempPtr;
}


static EFI_STATUS
ManageIvtEntryTable (
  IN  EFI_EXCEPTION_TYPE           ExceptionType,
  IN  BUNDLE                       NewBundles[NUM_BUNDLES_IN_STUB],
  IN  VOID                         (*NewCallback) ()
  )
/*++

Routine Description:
  This is the worker function that installs and removes all handlers
  
Arguments:
  IN  EFI_EXCEPTION_TYPE           ExceptionType,
  IN  BUNDLE                       NewBundles[NUM_BUNDLES_IN_STUB],
  IN  VOID                         (*NewCallback) ()

Returns:

  EFI_STATUS - any return other than EFI_SUCCESS indicates the request was not
  satisfied.
  
--*/
{
  BUNDLE      * B0Ptr;
  UINT64        InterruptFlags;
  EFI_TPL       OldTpl;

  //
  // Get address of bundle 0
  //
  GetHandlerEntryPoint (ExceptionType, (VOID**)&B0Ptr);

  if (IvtEntryTable[ExceptionType].RegisteredCallback != NULL) {
    //
    // we've already installed to this vector
    //
    if (NewCallback != NULL) {
      //
      // if the input handler is non-null, error
      //
      return EFI_ALREADY_STARTED;
    } else {
      //
      // else remove the previously installed handler
      //
      OldTpl = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL);
      InterruptFlags = ProgramInterruptFlags(DISABLE_INTERRUPTS);
      if (ExceptionType == EXCEPT_IPF_EXTERNAL_INTERRUPT) {
        UnchainExternalInterrupt();
      } else {
        UnhookEntry(ExceptionType);
      }
      ProgramInterruptFlags(InterruptFlags);
      gBS->RestoreTPL (OldTpl);
      //
      // re-init IvtEntryTable
      //
      gBS->SetMem(&IvtEntryTable[ExceptionType], sizeof(IVT_ENTRY), 0);
    }
  } else {
    //
    // no user handler installed on this vector
    //
    if (NewCallback != NULL) {
      OldTpl = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL);
      InterruptFlags = ProgramInterruptFlags(DISABLE_INTERRUPTS);
      if (ExceptionType == EXCEPT_IPF_EXTERNAL_INTERRUPT) {
        ChainExternalInterrupt(NewCallback);
      } else {
        HookEntry(ExceptionType, NewBundles, NewCallback);
      }
      ProgramInterruptFlags(InterruptFlags);
      gBS->RestoreTPL (OldTpl);
    }
  }
  return EFI_SUCCESS;
}


static VOID
HookEntry (
  IN  EFI_EXCEPTION_TYPE  ExceptionType,
  IN  BUNDLE              NewBundles[4],
  IN  VOID                (*NewCallback) ()
  )
/*++

Routine Description:
  Saves original IVT contents and inserts a few new bundles which are fixed up
  to store the ExceptionType and then call the common handler.
  
Arguments:
  IN  EFI_EXCEPTION_TYPE  ExceptionType,
  IN  BUNDLE              NewBundles[4],
  IN  VOID                (*NewCallback) ()

Returns:

  Nothing
    
--*/
{
  BUNDLE * FixupBundle;
  BUNDLE * B0Ptr;

  //
  // Get address of bundle 0
  //
  GetHandlerEntryPoint (ExceptionType, (VOID**)&B0Ptr);

  //
  // copy original bundles from IVT to IvtEntryTable so we can restore them later
  //
  gBS->CopyMem(IvtEntryTable[ExceptionType].OrigBundles, B0Ptr,
          sizeof(BUNDLE) * NUM_BUNDLES_IN_STUB);
  //
  // insert new B0
  //
  gBS->CopyMem(B0Ptr, NewBundles, sizeof(BUNDLE) * NUM_BUNDLES_IN_STUB);

  //
  // fixup IVT entry so it stores its index and whether or not to chain...
  //
  FixupBundle = B0Ptr + 2;
  FixupBundle->high |= ExceptionType << 36;

  InstructionCacheFlush (B0Ptr, 5);
  IvtEntryTable[ExceptionType].RegisteredCallback = NewCallback;
}


static VOID
UnhookEntry (
  IN  EFI_EXCEPTION_TYPE  ExceptionType
  )
/*++

Routine Description:
  Restores original IVT contents when unregistering a callback function
  
Arguments:
  IN  EFI_EXCEPTION_TYPE  ExceptionType,

Returns:

  Nothing
    
--*/
{
  BUNDLE * B0Ptr;

  //
  // Get address of bundle 0
  //
  GetHandlerEntryPoint (ExceptionType, (VOID**)&B0Ptr);
  //
  // restore original bundles in IVT
  //
  gBS->CopyMem(B0Ptr, IvtEntryTable[ExceptionType].OrigBundles,
          sizeof(BUNDLE) * NUM_BUNDLES_IN_STUB);
  InstructionCacheFlush (B0Ptr, 5);
}


static VOID
ChainExternalInterrupt (
  IN  VOID  (*NewCallback) ()
  )
/*++

Routine Description:
  Sets up cache flush and calls assembly function to chain external interrupt.
  Records new callback in IvtEntryTable.
  
Arguments:
  IN  VOID  (*NewCallback) ()

Returns:

  Nothing
    
--*/
{
  VOID * Start;

  Start = (VOID *) ((UINT8 *) GetIva() + 0x400 * EXCEPT_IPF_EXTERNAL_INTERRUPT + 0x400);
  IvtEntryTable[EXCEPT_IPF_EXTERNAL_INTERRUPT].RegisteredCallback = NewCallback;
  ChainHandler();
  InstructionCacheFlush (Start, 0x400);
}


static VOID
UnchainExternalInterrupt (
  VOID
  )
/*++

Routine Description:
  Sets up cache flush and calls assembly function to restore external interrupt.
  Removes registered callback from IvtEntryTable.
  
Arguments:
  Nothing
  
Returns:

  Nothing
    
--*/
{
  VOID * Start;

  Start = (VOID *) ((UINT8 *) GetIva() + 0x400 * EXCEPT_IPF_EXTERNAL_INTERRUPT + 0x400);
  UnchainHandler();
  InstructionCacheFlush (Start, 0x400);
  IvtEntryTable[EXCEPT_IPF_EXTERNAL_INTERRUPT].RegisteredCallback = NULL;
}



//
// The rest of the functions in this file are all member functions for the
// DebugSupport protocol
//

EFI_STATUS
GetMaximumProcessorIndex (
  IN EFI_DEBUG_SUPPORT_PROTOCOL    *This,
  OUT UINTN                        *MaxProcessorIndex
  )
/*++

Routine Description: This is a DebugSupport protocol member function.  Hard
  coded to support only 1 processor for now.

Arguments:

Returns: Always returns EFI_SUCCESS with *MaxProcessorIndex set to 0

--*/
{
  *MaxProcessorIndex = 0;
  return (EFI_SUCCESS);
}


EFI_STATUS
RegisterPeriodicCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL     *This,
  IN UINTN                          ProcessorIndex,
  IN EFI_PERIODIC_CALLBACK          NewPeriodicCallback
  )
/*++

Routine Description:
  DebugSupport protocol member function

Arguments:
  IN EFI_DEBUG_SUPPORT_PROTOCOL   *This,
  IN UINTN                        ProcessorIndex,
  IN EFI_PERIODIC_CALLBACK        NewPeriodicCallback

Returns:

  EFI_STATUS - anything other than EFI_SUCCESS indicates the callback was not registered.

--*/
{
  return ManageIvtEntryTable(EXCEPT_IPF_EXTERNAL_INTERRUPT, NULL, NewPeriodicCallback);
}


EFI_STATUS
RegisterExceptionCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL    *This,
  IN UINTN                         ProcessorIndex,
  IN EFI_EXCEPTION_CALLBACK        NewCallback,
  IN EFI_EXCEPTION_TYPE            ExceptionType
  )
/*++

Routine Description:
  DebugSupport protocol member function

Arguments:
  IN EFI_DEBUG_SUPPORT_PROTOCOL   *This,
  IN EFI_EXCEPTION_CALLBACK       NewCallback,
  IN EFI_EXCEPTION_TYPE           ExceptionType

Returns:

  EFI_STATUS - anything other than EFI_SUCCESS indicates the callback was not registered.

--*/
{
  return ManageIvtEntryTable (ExceptionType,
    (BUNDLE *) ((EFI_PLABEL *)HookStub)->EntryPoint, NewCallback);
}


EFI_STATUS
InvalidateInstructionCache (
  IN EFI_DEBUG_SUPPORT_PROTOCOL    *This,
  IN UINTN                         ProcessorIndex,
  IN VOID                          *Start,
  IN UINTN                         Length
  )
/*++

Routine Description:
  DebugSupport protocol member function.  Calls assembly routine to flush cache.

Arguments:

Returns:
  EFI_SUCCESS

--*/
{
  InstructionCacheFlush (Start, Length);
  return (EFI_SUCCESS);
}


