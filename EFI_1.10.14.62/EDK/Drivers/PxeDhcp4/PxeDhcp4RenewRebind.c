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
  PxeDhcp4RenewRebind.c
  
Abstract:

--*/

#include "PxeDhcp4.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static INTN
acknak_verify(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  IN DHCP4_PACKET *tx_pkt,
  IN DHCP4_PACKET *rx_pkt,
  IN UINTN rx_pkt_size)
/*++
Routine Description:

Parameters:

Returns:
  -2 = ignore, stop waiting
  -1 = ignore, keep waiting
  0 = accept, keep waiting
  1 = accept, stop waiting
--*/
{
  EFI_STATUS efi_status;
  DHCP4_OP *msg_type_op;
  DHCP4_OP *srvid_op;
  DHCP4_OP *renew_op;
  DHCP4_OP *rebind_op;
  UINT32 magik;

  //
  // Verify parameters.  Unused parameters are also touched
  // to make the compiler happy.
  //

  ASSERT(Private);
  ASSERT(rx_pkt);

  if (Private == NULL || rx_pkt == NULL) {
    return -2;
  }

  tx_pkt = tx_pkt;
  rx_pkt_size = rx_pkt_size;

  //
  // This must be a DHCP Ack message.
  //

  magik = htonl(DHCP4_MAGIK_NUMBER);

  if (EfiCompareMem(&rx_pkt->dhcp4.magik, &magik, 4)) {
    return -1;
  }

  efi_status = find_opt(rx_pkt, DHCP4_MESSAGE_TYPE, 0, &msg_type_op);

  if (EFI_ERROR(efi_status)) {
    return -1;
  }

  if (msg_type_op->len != 1) {
    return -1;
  }

  if (msg_type_op->data[0] != DHCP4_MESSAGE_TYPE_ACK) {
    return -1;
  }

  //
  // There must be a server identifier.
  //

  efi_status = find_opt(rx_pkt, DHCP4_SERVER_IDENTIFIER, 0, &srvid_op);

  if (EFI_ERROR(efi_status)) {
    return -1;
  }

  if (srvid_op->len != 4) {
    return -1;
  }

  //
  // There must be a renewal time.
  //

  efi_status = find_opt(rx_pkt, DHCP4_RENEWAL_TIME, 0, &renew_op);

  if (EFI_ERROR(efi_status)) {
    return -1;
  }

  if (renew_op->len != 4) {
    return -1;
  }

  //
  // There must be a rebinding time.
  //

  efi_status = find_opt(rx_pkt, DHCP4_REBINDING_TIME, 0, &rebind_op);

  if (EFI_ERROR(efi_status)) {
    return -1;
  }

  if (rebind_op->len != 4) {
    return -1;
  }

  //
  // Packet is good enough.
  //

  EfiCopyMem(&Private->ServerIp, srvid_op->data, 4);
  EfiCopyMem(&Private->RenewTime, renew_op->data, 4);
  EfiCopyMem(&Private->RebindTime, rebind_op->data, 4);

  return 1;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static EFI_STATUS EFIAPI
renew_rebind(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN UINTN seconds_timeout,
  IN BOOLEAN renew)
{
  PXE_DHCP4_PRIVATE_DATA *Private;
  EFI_IP_ADDRESS ServerIp;
  EFI_IP_ADDRESS client_ip;
  EFI_IP_ADDRESS subnet_mask;
  EFI_IP_ADDRESS gateway_ip;
  DHCP4_PACKET Request;
  DHCP4_PACKET AckNak;
  DHCP4_OP *op;
  EFI_STATUS efi_status;

  //
  // Check for invalid parameters.
  //

  if (This == NULL || seconds_timeout < DHCP4_MIN_SECONDS ||
    seconds_timeout > DHCP4_MAX_SECONDS)
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check for proper protocol state.
  //

  if (This->Data == NULL) {
    return EFI_NOT_STARTED;
  }

  if (!This->Data->SelectCompleted) {
    return EFI_NOT_READY;
  }

  if (This->Data->IsBootp) {
    return EFI_SUCCESS;
  }

  if (!This->Data->IsAck) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get pointer to instance data.
  //

  Private = PXE_DHCP4_PRIVATE_DATA_FROM_THIS(This);

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Private->PxeBc == NULL) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Copy Discover packet to temporary request packet
  // to be used for Renew/Rebind operation.
  //

  EfiCopyMem(&Request, &This->Data->Discover, sizeof(DHCP4_PACKET));

  EfiCopyMem(&Request.dhcp4.ciaddr, &This->Data->AckNak.dhcp4.yiaddr, 4);

  Request.dhcp4.flags = 0;  /* Reply does not need to be broadcast. */

  //
  // Change message type from discover to request.
  //

  efi_status = find_opt(&Request, DHCP4_MESSAGE_TYPE, 0, &op);

  if (EFI_ERROR(efi_status)) {
    return EFI_INVALID_PARAMETER;
  }

  if (op->len != 1) {
    return EFI_INVALID_PARAMETER;
  }

  op->data[0] = DHCP4_MESSAGE_TYPE_REQUEST;

  //
  // Need a subnet mask.
  //

  efi_status = find_opt(&This->Data->AckNak,
    DHCP4_SUBNET_MASK, 0, &op);

  if (EFI_ERROR(efi_status)) {
    return EFI_INVALID_PARAMETER;
  }

  if (op->len != 4) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem(&subnet_mask, sizeof(EFI_IP_ADDRESS));
  EfiCopyMem(&subnet_mask, op->data, 4);

  //
  // Need a server IP address (renew) or a broadcast
  // IP address (rebind).
  //

  EfiZeroMem(&gateway_ip, sizeof(EFI_IP_ADDRESS));

  if (renew) {
    efi_status = find_opt(&This->Data->AckNak,
      DHCP4_SERVER_IDENTIFIER, 0, &op);

    if (EFI_ERROR(efi_status)) {
      return EFI_INVALID_PARAMETER;
    }

    if (op->len != 4) {
      return EFI_INVALID_PARAMETER;
    }

    EfiZeroMem(&ServerIp, sizeof(EFI_IP_ADDRESS));
    EfiCopyMem(&ServerIp, op->data, 4);

    //
    //
    //

    if (EfiCompareMem(&This->Data->AckNak.dhcp4.giaddr, &gateway_ip, 4)) {
      EfiCopyMem(&gateway_ip, &This->Data->AckNak.dhcp4.giaddr, 4);
    }
  } else {
    EfiSetMem(&ServerIp, sizeof(EFI_IP_ADDRESS), 0xFF);
  }

  //
  // Need a client IP address.
  //

  EfiZeroMem(&client_ip, sizeof(EFI_IP_ADDRESS));
  EfiCopyMem(&client_ip, &Request.dhcp4.ciaddr, 4);

  //
  //
  //

  efi_status = gBS->HandleProtocol(
    Private->Handle,
    &gEfiPxeDhcp4CallbackProtocolGuid,
    (VOID *)&Private->callback);

  if (EFI_ERROR(efi_status)) {
    Private->callback = NULL;
  }

  Private->function = renew ?
    EFI_PXE_DHCP4_FUNCTION_RENEW :
    EFI_PXE_DHCP4_FUNCTION_REBIND;

  //
  // Transimit DHCP request and wait for DHCP ack...
  //

  efi_status = tx_rx_udp(Private, &ServerIp, &gateway_ip,
    &client_ip, &subnet_mask, &Request, &AckNak,
    &acknak_verify, seconds_timeout);

  if (EFI_ERROR(efi_status)) {
    Private->callback = NULL;
    return efi_status;
  }

  //
  // Copy server identifier, renewal time and rebinding time 
  // from temporary ack/nak packet into cached ack/nak packet.
  //

  efi_status = find_opt(&This->Data->AckNak,
    DHCP4_SERVER_IDENTIFIER, 0, &op);

  if (!EFI_ERROR(efi_status)) {
    if (op->len == 4) {
      EfiCopyMem(op->data, &Private->ServerIp, 4);
    }
  }

  efi_status = find_opt(&This->Data->AckNak, DHCP4_RENEWAL_TIME, 0, &op);

  if (!EFI_ERROR(efi_status)) {
    if (op->len == 4) {
      EfiCopyMem(op->data, &Private->RenewTime, 4);
    }
  }

  efi_status = find_opt(&This->Data->AckNak, DHCP4_REBINDING_TIME, 0, &op);

  if (!EFI_ERROR(efi_status)) {
    if (op->len == 4) {
      EfiCopyMem(op->data, &Private->RebindTime, 4);
    }
  }

  Private->callback = NULL;
  return efi_status;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

EFI_STATUS EFIAPI
PxeDhcp4Renew(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN UINTN seconds_timeout)
{
  return renew_rebind(This, seconds_timeout, TRUE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

EFI_STATUS EFIAPI
PxeDhcp4Rebind(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN UINTN seconds_timeout)
{
  return renew_rebind(This, seconds_timeout, FALSE);
}

/* eof - PxeDhcp4RenewRebind.c */
