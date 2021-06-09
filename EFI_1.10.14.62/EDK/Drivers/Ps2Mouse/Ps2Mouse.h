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

  Ps2Mouse.h

Abstract:

  PS/2 Mouse driver header file

Revision History

--*/

#ifndef _PS2MOUSE_H
#define _PS2MOUSE_H

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver consumed protocol prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (IsaIo)


//
// Driver produced protocol prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimplePointer)

// PS/2 mouse sample rate
typedef enum {
  SSR_10,
  SSR_20,
  SSR_40,
  SSR_60,
  SSR_80,
  SSR_100,
  SSR_200,
  MAX_SR
} MOUSE_SR ;

// PS/2 mouse resolution
typedef enum {
  CMR1,
  CMR2,
  CMR4,
  CMR8,
  MAX_CMR
} MOUSE_RE ;

// PS/2 mouse scaling
typedef enum {
  SF1,
  SF2
} MOUSE_SF ;

//
// Driver Private Data
//
#define PS2_MOUSE_DEV_SIGNATURE   EFI_SIGNATURE_32('p','s','2','m')

typedef struct {
  UINTN                           Signature;

  EFI_HANDLE                      Handle;
  EFI_SIMPLE_POINTER_PROTOCOL     SimplePointerProtocol;
  EFI_SIMPLE_POINTER_STATE        State;
  EFI_SIMPLE_POINTER_MODE         Mode;
  BOOLEAN                         StateChanged;

  // PS2 Mouse device specific information
  MOUSE_SR                        SampleRate;
  MOUSE_RE                        Resolution;
  MOUSE_SF                        Scaling;
  UINT8                           DataPackageSize;

  EFI_ISA_IO_PROTOCOL             *IsaIo;

  EFI_EVENT                       TimerEvent;

  EFI_UNICODE_STRING_TABLE        *ControllerNameTable;
} PS2_MOUSE_DEV;

#define PS2_MOUSE_DEV_FROM_THIS(a) \
  CR(a, PS2_MOUSE_DEV, SimplePointerProtocol, PS2_MOUSE_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gPS2MouseDriver;
extern EFI_COMPONENT_NAME_PROTOCOL gPs2MouseComponentName;

//
// Driver entry point
//
EFI_STATUS
PS2MouseDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Other functions 
//

#endif
