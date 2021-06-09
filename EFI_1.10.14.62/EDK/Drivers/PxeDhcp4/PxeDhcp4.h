#ifndef _PXEDHCP4_H
#define _PXEDHCP4_H

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
  PxeDhcp4.h

Abstract:
  Common header for PxeDhcp4 protocol driver

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//
// Driver Consumed Protocol Prototypes
//

#include EFI_PROTOCOL_DEFINITION(PxeBaseCode)
#include EFI_PROTOCOL_DEFINITION(SimpleNetwork)
#include EFI_PROTOCOL_DEFINITION(PxeDhcp4Callback)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//
// Driver Produced Protocol Prototypes
//

#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(ComponentName)
#include EFI_PROTOCOL_DEFINITION(PxeDhcp4)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//
// PxeDhcp4 protocol instance data
//
typedef struct {

  //
  // Signature field used to locate beginning of containment record.
  //
  UINTN Signature;

#define PXE_DHCP4_PRIVATE_DATA_SIGNATURE \
  EFI_SIGNATURE_32('p','x','D','4')

  //
  // Device handle the protocol is bound to.
  //
  EFI_HANDLE Handle;

  //
  // Public PxeDhcp4 protocol interface.
  //
  EFI_PXE_DHCP4_PROTOCOL PxeDhcp4;

  //
  // Consumed PxeBc, Snp and PxeDhcp4Callback protocol interfaces.
  //
  EFI_PXE_BASE_CODE_PROTOCOL *PxeBc;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  EFI_PXE_DHCP4_CALLBACK_PROTOCOL *callback;

  //
  // PxeDhcp4 called function for PxeDhcp4Callback.
  //
  EFI_PXE_DHCP4_FUNCTION function;

  //
  // Timeout event and flag for PxeDhcp4Callback.
  //
  EFI_EVENT TimeoutEvent;
  BOOLEAN timeout_occurred;

  //
  // Periodic event and flag for PxeDhcp4Callback.
  //
  EFI_EVENT periodic_event;
  BOOLEAN periodic_occurred;

  //
  // DHCP server IP address.
  //
  UINT32 ServerIp;

  //
  // DHCP renewal and rebinding times, in seconds.
  //
  UINT32 RenewTime;
  UINT32 RebindTime;

  //
  // Number of offers received & allocated offer list.
  //
  UINTN offers;
  DHCP4_PACKET *offer_list;

  //
  // 
  //
  BOOLEAN StopPxeBc;

} PXE_DHCP4_PRIVATE_DATA;

#define PXE_DHCP4_PRIVATE_DATA_FROM_THIS(a) \
  CR(a, PXE_DHCP4_PRIVATE_DATA, PxeDhcp4, PXE_DHCP4_PRIVATE_DATA_SIGNATURE)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//
// Protocol function prototypes.
//

extern EFI_STATUS EFIAPI PxeDhcp4Run(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN OPTIONAL UINTN OpLen,
  IN OPTIONAL VOID *OpList);

extern EFI_STATUS EFIAPI PxeDhcp4Setup(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN EFI_PXE_DHCP4_DATA *Data);

extern EFI_STATUS EFIAPI PxeDhcp4Init(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN UINTN seconds_timeout,
  OUT UINTN *offer_list_entries,
  OUT DHCP4_PACKET **offer_list);

extern EFI_STATUS EFIAPI PxeDhcp4Select(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN UINTN seconds_timeout,
  IN DHCP4_PACKET *offer_list);

extern EFI_STATUS EFIAPI PxeDhcp4Renew(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  UINTN seconds_timeout);

extern EFI_STATUS EFIAPI PxeDhcp4Rebind(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  UINTN seconds_timeout);

extern EFI_STATUS EFIAPI PxeDhcp4Release(
  IN EFI_PXE_DHCP4_PROTOCOL *This);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//
// Support function prototypes.
//

extern UINT16 htons(UINTN n);

extern UINT32 htonl(UINTN n);

extern VOID timeout_notify(
  IN EFI_EVENT Event,
  IN VOID *Context);

extern VOID periodic_notify(
  IN EFI_EVENT Event,
  IN VOID *Context);

extern EFI_STATUS find_opt(
  IN DHCP4_PACKET *Packet,
  IN UINT8 OpCode,
  IN UINTN Skip,
  OUT DHCP4_OP **OpPtr);

extern EFI_STATUS add_opt(
  IN DHCP4_PACKET *Packet,
  IN DHCP4_OP *OpPtr);

extern EFI_STATUS start_udp(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  IN OPTIONAL EFI_IP_ADDRESS *station_ip,
  IN OPTIONAL EFI_IP_ADDRESS *subnet_mask);

extern VOID stop_udp(
  IN PXE_DHCP4_PRIVATE_DATA *Private);

extern EFI_STATUS start_receive_events(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  IN UINTN seconds_timeout);

extern VOID stop_receive_events(
  IN PXE_DHCP4_PRIVATE_DATA *Private);

extern EFI_STATUS tx_udp(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  IN EFI_IP_ADDRESS *dest_ip,
  IN OPTIONAL EFI_IP_ADDRESS *gateway_ip,
  IN EFI_IP_ADDRESS *src_ip,
  IN VOID *buffer,
  IN UINTN BufferSize);

extern EFI_STATUS rx_udp(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  OUT VOID *buffer,
  OUT UINTN *BufferSize,
  IN OUT EFI_IP_ADDRESS *dest_ip,
  IN OUT EFI_IP_ADDRESS *src_ip,
  IN UINT16 op_flags);

extern EFI_STATUS tx_rx_udp(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  IN OUT EFI_IP_ADDRESS *ServerIp,
  IN OPTIONAL EFI_IP_ADDRESS *gateway_ip,
  IN OPTIONAL EFI_IP_ADDRESS *client_ip,
  IN OPTIONAL EFI_IP_ADDRESS *subnet_mask,
  IN DHCP4_PACKET *tx_pkt,
  OUT DHCP4_PACKET *rx_pkt,
  IN UINTN (*rx_vfy)(
    IN PXE_DHCP4_PRIVATE_DATA *Private,
    IN DHCP4_PACKET *tx_pkt,
    IN DHCP4_PACKET *rx_pkt,
    IN UINTN rx_pkt_size),
  IN UINTN seconds_timeout);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//
// Global variable definitions.
//

extern EFI_DRIVER_BINDING_PROTOCOL gPxeDhcp4DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPxeDhcp4ComponentName;

#endif /* _PXEDHCP4_H */
/* EOF - PxeDhcp4.h */
