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
  
    Ebc.h
  
Abstract:

  Describes the protocol interface to the EBC interpreter.
  
--*/  
#ifndef _EBC_DEBUG_HELPER_H_
#define _EBC_DEBUG_HELPER_H_


//
// Definitions for the EBC debug support helper protocol
//


#define EFI_EBC_DEBUG_HELPER_PROTOCOL_GUID \
{ 0x4dc98014, 0xbfb0, 0x41e2, 0x87, 0xae, 0x67, 0xb7, 0x34, 0xff, 0xc9, 0x74 }

//
// Define for forward reference.
//
EFI_INTERFACE_DECL(_EFI_EBC_DEBUG_HELPER_PROTOCOL);

/*++

Routine Description:
  
  Called by a debugger to determine if a given native processor address
  is in a thunk created by the interpreter.

Arguments:

  This      - pointer to protocol
  Addr      - physical address, typically the IP of the native processor

Returns:

  TRUE - if Addr is in a thunk created by the interpreter
  FALSE otherwise

--*/
typedef 
BOOLEAN
(EFIAPI *EFI_EBC_DEBUG_HELPER_IN_THUNK) (
  IN struct _EFI_EBC_DEBUG_HELPER_PROTOCOL    *This,
  IN EFI_PHYSICAL_ADDRESS                     Addr
  );

typedef 
EFI_STATUS
(EFIAPI *EFI_EBC_DEBUG_HELPER_BREAK_REQUEST) (
  IN struct _EFI_EBC_DEBUG_HELPER_PROTOCOL    *This
  );

typedef 
EFI_STATUS
(EFIAPI *EFI_EBC_DEBUG_HELPER_BREAK_REQUEST_CANCEL) (
  IN struct _EFI_EBC_DEBUG_HELPER_PROTOCOL    *This
  );

//
// Prototype for the actual EBC debug helper protocol interface
//
typedef struct _EFI_EBC_DEBUG_HELPER_PROTOCOL {
  EFI_EBC_DEBUG_HELPER_IN_THUNK              InThunk;
  EFI_EBC_DEBUG_HELPER_BREAK_REQUEST         BreakRequest;
  EFI_EBC_DEBUG_HELPER_BREAK_REQUEST_CANCEL  BreakRequestCancel;
} EFI_EBC_DEBUG_HELPER_PROTOCOL;

#endif	
