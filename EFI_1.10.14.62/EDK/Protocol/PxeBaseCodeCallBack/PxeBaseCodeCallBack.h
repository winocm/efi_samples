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
  PxeBaseCodeCallBack.h

Abstract:
  EFI PXE Base Code CallBack Protocol

--*/

#ifndef _PXE_BASE_CODE_CALLBACK_H_
#define _PXE_BASE_CODE_CALLBACK_H_


#include "Pxe.h"

//
// Call Back Definitions
//

#define EFI_PXE_BASE_CODE_CALLBACK_PROTOCOL_GUID \
  { 0x245dca21, 0xfb7b, 0x11d3, 0x8f, 0x01, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

//
// Revision Number
//

#define EFI_PXE_BASE_CODE_CALLBACK_INTERFACE_REVISION   0x00010000

//
// Protocol definition
//

EFI_INTERFACE_DECL(_EFI_PXE_BASE_CODE_CALLBACK_PROTOCOL);

typedef enum {
  EFI_PXE_BASE_CODE_FUNCTION_FIRST,
  EFI_PXE_BASE_CODE_FUNCTION_DHCP,
  EFI_PXE_BASE_CODE_FUNCTION_DISCOVER,
  EFI_PXE_BASE_CODE_FUNCTION_MTFTP,
  EFI_PXE_BASE_CODE_FUNCTION_UDP_WRITE,
  EFI_PXE_BASE_CODE_FUNCTION_UDP_READ,
  EFI_PXE_BASE_CODE_FUNCTION_ARP,
  EFI_PXE_BASE_CODE_FUNCTION_IGMP,
  EFI_PXE_BASE_CODE_FUNCTION_TCP_WRITE,
  EFI_PXE_BASE_CODE_FUNCTION_TCP_READ,
  EFI_PXE_BASE_CODE_PXE_FUNCTION_LAST
} EFI_PXE_BASE_CODE_FUNCTION;

typedef enum {
  EFI_PXE_BASE_CODE_CALLBACK_STATUS_FIRST,
  EFI_PXE_BASE_CODE_CALLBACK_STATUS_CONTINUE,
  EFI_PXE_BASE_CODE_CALLBACK_STATUS_ABORT,
  EFI_PXE_BASE_CODE_CALLBACK_STATUS_LAST
} EFI_PXE_BASE_CODE_CALLBACK_STATUS;

typedef
EFI_PXE_BASE_CODE_CALLBACK_STATUS
(EFIAPI *EFI_PXE_CALLBACK) (
  IN struct _EFI_PXE_BASE_CODE_CALLBACK_PROTOCOL   *This,
  IN EFI_PXE_BASE_CODE_FUNCTION                    Function,
  IN BOOLEAN                                       Received,
  IN UINT32                                        PacketLen,
  IN EFI_PXE_BASE_CODE_PACKET                      *Packet     OPTIONAL
  );


typedef struct _EFI_PXE_BASE_CODE_CALLBACK_PROTOCOL {
  UINT64                      Revision;
  EFI_PXE_CALLBACK            Callback;
} EFI_PXE_BASE_CODE_CALLBACK_PROTOCOL;

extern EFI_GUID gEfiPxeBaseCodeCallbackProtocolGuid;

#endif /* _EFIPXEBC_H */
/* EOF - PxeBaseCodeCallBack.h */
