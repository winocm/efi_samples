#ifndef _PXE_DHCP4CALLBACK_H
#define _PXE_DHCP4CALLBACK_H

/*++

Copyright (c) 1999 - 2002 Intel Corporation.  All rights reserved.

This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  PxeDhcp4Callback.h

Abstract:
  EFI PXE DHCP4 Callback protocol definition.

--*/

#include "..\PxeDhcp4\PxeDhcp4.h"

//
// GUID definition
//

#define EFI_PXE_DHCP4_CALLBACK_PROTOCOL_GUID \
{ 0xc1544c01, 0x92a4, 0x4198, 0x8a, 0x84, 0x77, 0x85, 0x83, 0xc2, 0x36, 0x21 }


//
// Revision number
//

#define EFI_PXE_DHCP4_CALLBACK_INTERFACE_REVISION   0x00010000

//
// Interface definition
//

EFI_INTERFACE_DECL(_EFI_PXE_DHCP4_CALLBACK_PROTOCOL);

typedef enum {
  EFI_PXE_DHCP4_FUNCTION_FIRST,
  EFI_PXE_DHCP4_FUNCTION_INIT,
  EFI_PXE_DHCP4_FUNCTION_SELECT,
  EFI_PXE_DHCP4_FUNCTION_RENEW,
  EFI_PXE_DHCP4_FUNCTION_REBIND,
  EFI_PXE_DHCP4_FUNCTION_LAST
} EFI_PXE_DHCP4_FUNCTION;

typedef enum {
  EFI_PXE_DHCP4_CALLBACK_STATUS_FIRST,
  EFI_PXE_DHCP4_CALLBACK_STATUS_ABORT,
  EFI_PXE_DHCP4_CALLBACK_STATUS_IGNORE_ABORT,
  EFI_PXE_DHCP4_CALLBACK_STATUS_KEEP_ABORT,
  EFI_PXE_DHCP4_CALLBACK_STATUS_CONTINUE,
  EFI_PXE_DHCP4_CALLBACK_STATUS_IGNORE_CONTINUE,
  EFI_PXE_DHCP4_CALLBACK_STATUS_KEEP_CONTINUE,
  EFI_PXE_DHCP4_CALLBACK_STATUS_LAST
} EFI_PXE_DHCP4_CALLBACK_STATUS;

typedef
EFI_PXE_DHCP4_CALLBACK_STATUS
(EFIAPI *EFI_PXE_DHCP4_CALLBACK) (
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN EFI_PXE_DHCP4_FUNCTION Function,
  IN UINT32                 PacketLen,
  IN DHCP4_PACKET           *Packet OPTIONAL
  );

typedef struct _EFI_PXE_DHCP4_CALLBACK_PROTOCOL {
  UINT64                      Revision;
  EFI_PXE_DHCP4_CALLBACK      Callback;
} EFI_PXE_DHCP4_CALLBACK_PROTOCOL;

//
// GUID declaration
//

extern EFI_GUID gEfiPxeDhcp4CallbackProtocolGuid;

#endif /* _PXE_DHCP4CALLBACK_H */
/* EOF - PxeDhcp4Callback.h */
