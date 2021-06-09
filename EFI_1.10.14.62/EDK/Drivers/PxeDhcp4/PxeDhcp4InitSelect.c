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
  PxeDhcp4InitSelect.c
  
Abstract:

--*/

#include "PxeDhcp4.h"

#define DebugPrint(x)
//#define DebugPrint(x) Aprint(x)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static INTN
offer_verify(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  IN DHCP4_PACKET *tx_pkt,
  IN DHCP4_PACKET *rx_pkt,
  IN UINTN rx_pkt_size)
/*++
  -2 = ignore, stop waiting
  -1 = ignore, keep waiting
  0 = accept, keep waiting
  1 = accept, stop waiting
--*/
{
  EFI_STATUS efi_status;
  DHCP4_PACKET *tmp;
  DHCP4_OP *msg_type_op;
  DHCP4_OP *srvid_op;
  UINT32 magik;

  //
  // Verify parameters.  Touch unused parameters to keep
  // compiler happy.
  //

  ASSERT(Private);
  ASSERT(rx_pkt);

  if (Private == NULL || rx_pkt == NULL) {
    return -2;
  }

  tx_pkt = tx_pkt;
  rx_pkt_size = rx_pkt_size;

  //
  // This may be a BOOTP Reply or DHCP Offer packet.
  // If there is no DHCP magik number, assume that 
  // this is a BOOTP Reply packet.
  //

  magik = htonl(DHCP4_MAGIK_NUMBER);

  while (!EfiCompareMem(&rx_pkt->dhcp4.magik, &magik, 4)) {
    //
    // If there is no DHCP message type option, assume
    // this is a BOOTP reply packet and cache it.
    //

    efi_status = find_opt(rx_pkt, DHCP4_MESSAGE_TYPE, 0, &msg_type_op);

    if (EFI_ERROR(efi_status)) {
      break;
    }

    //
    // If there is a DHCP message type option, it must be a
    // DHCP offer packet
    //

    if (msg_type_op->len != 1) {
      return -1;
    }

    if (msg_type_op->data[0] != DHCP4_MESSAGE_TYPE_OFFER) {
      return -1;
    }

    //
    // There must be a server identifier option.
    //

    efi_status = find_opt(rx_pkt, DHCP4_SERVER_IDENTIFIER,
      0, &srvid_op);

    if (EFI_ERROR(efi_status)) {
      return -1;
    }

    if (srvid_op->len != 4) {
      return -1;
    }

    //
    // Good DHCP offer packet.
    //

    break;
  }

  //
  // Good DHCP (or BOOTP) packet.  Cache it!
  //

  efi_status = gBS->AllocatePool(
    EfiBootServicesData,
    (Private->offers + 1) * sizeof(DHCP4_PACKET),
    (VOID **)&tmp);

  if (EFI_ERROR(efi_status)) {
    return -2;
  }

  ASSERT(tmp);

  if (Private->offers != 0) {
    EfiCopyMem(tmp, Private->offer_list,
      Private->offers * sizeof(DHCP4_PACKET));

    gBS->FreePool(Private->offer_list);
  }

  EfiCopyMem(&tmp[Private->offers++], rx_pkt, sizeof(DHCP4_PACKET));

  Private->offer_list = tmp;

  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static INTN
acknak_verify(
  IN PXE_DHCP4_PRIVATE_DATA *Private,
  IN DHCP4_PACKET *tx_pkt,
  IN DHCP4_PACKET *rx_pkt,
  IN UINTN rx_pkt_size)
/*++
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
  // Verify parameters.  Touch unused parameters to
  // keep compiler happy.
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
  // Packet is good.
  //

  EfiCopyMem(&Private->ServerIp, srvid_op->data, 4);
  EfiCopyMem(&Private->RenewTime, renew_op->data, 4);
  EfiCopyMem(&Private->RebindTime, rebind_op->data, 4);

  return 1;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

EFI_STATUS EFIAPI
PxeDhcp4Init(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN UINTN seconds_timeout,
  OUT UINTN *Offers,
  OUT DHCP4_PACKET **OfferList)
{
  PXE_DHCP4_PRIVATE_DATA *Private;
  DHCP4_PACKET offer;
  EFI_IP_ADDRESS bcast_ip;
  EFI_STATUS efi_status;

  //
  // Verify parameters and protocol state.
  //

  if (This == NULL || seconds_timeout < DHCP4_MIN_SECONDS ||
    seconds_timeout > DHCP4_MAX_SECONDS || Offers == NULL || OfferList == NULL)
  {
    //
    // Return parameters are not initialized when
    // parameters are invalid!
    //

    return EFI_INVALID_PARAMETER;
  }

  *Offers = 0;
  *OfferList = NULL;

  //
  // Check protocol state.
  //

  if (This->Data == NULL) {
    return EFI_NOT_STARTED;
  }

  if (!This->Data->SetupCompleted) {
    return EFI_NOT_READY;
  }

#if 0
  if (!is_good_discover(&This->Data->Discover)) {
    // %%TBD - check discover packet fields
  }
#endif

  //
  // Get pointer to our instance data.
  //

  Private = PXE_DHCP4_PRIVATE_DATA_FROM_THIS(This);

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Private->PxeBc == NULL) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Setup variables...
  //

  Private->offers = 0;
  Private->offer_list = NULL;

  efi_status = gBS->HandleProtocol(
    Private->Handle,
    &gEfiPxeDhcp4CallbackProtocolGuid,
    (VOID *)&Private->callback);

  if (EFI_ERROR(efi_status)) {
    Private->callback = NULL;
  }

  Private->function = EFI_PXE_DHCP4_FUNCTION_INIT;

  //
  // Increment the transaction ID.
  //

  {
    UINT32 xid;

    EfiCopyMem(&xid, &This->Data->Discover.dhcp4.xid, sizeof(UINT32));

    xid = htonl(htonl(xid) + 1);

    EfiCopyMem(&This->Data->Discover.dhcp4.xid, &xid, sizeof(UINT32));
  }

  //
  // Transmit discover and wait for offers...
  //

  EfiSetMem(&bcast_ip, sizeof(EFI_IP_ADDRESS), 0xFF);

  efi_status = tx_rx_udp(Private, &bcast_ip, NULL, NULL, NULL,
    &This->Data->Discover, &offer, &offer_verify, seconds_timeout);

  if (EFI_ERROR(efi_status)) {
    if (Private->offer_list) {
      gBS->FreePool(Private->offer_list);
    }

    Private->offers = 0;
    Private->offer_list = NULL;
    Private->callback = NULL;

    DebugPrint(("%a:%d:%r\n", __FILE__, __LINE__, efi_status));
    return efi_status;
  }

  *Offers = Private->offers;
  *OfferList = Private->offer_list;

  Private->offers = 0;
  Private->offer_list = NULL;
  Private->callback = NULL;

  This->Data->InitCompleted = TRUE;
  This->Data->SelectCompleted = FALSE;
  This->Data->IsBootp = FALSE;
  This->Data->IsAck = FALSE;

  return EFI_SUCCESS;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

EFI_STATUS EFIAPI
PxeDhcp4Select(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN UINTN seconds_timeout,
  IN DHCP4_PACKET *Offer)
{
  PXE_DHCP4_PRIVATE_DATA *Private;
  EFI_STATUS efi_status;
  DHCP4_PACKET request;
  DHCP4_PACKET acknak;
  EFI_IP_ADDRESS bcast_ip;
  EFI_IP_ADDRESS zero_ip;
  EFI_IP_ADDRESS local_ip;
  DHCP4_OP *srvid;
  DHCP4_OP *op;
  UINT32 dhcp4_magik;
  UINT8 buf[16];
  BOOLEAN is_bootp;

  //
  // Verify parameters.
  //

  if (This == NULL || seconds_timeout < DHCP4_MIN_SECONDS ||
    seconds_timeout > DHCP4_MAX_SECONDS || Offer == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check protocol state.
  //

  if (This->Data == NULL) {
    return EFI_NOT_STARTED;
  }

  if (!This->Data->SetupCompleted) {
    return EFI_NOT_READY;
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

#if 0
  if (!is_good_discover(&This->Data->Discover)) {
    // %%TBD - check discover packet fields
  }
#endif

  //
  // Setup useful variables...
  //

  EfiSetMem(&bcast_ip, sizeof(EFI_IP_ADDRESS), 0xFF);

  EfiZeroMem(&zero_ip, sizeof(EFI_IP_ADDRESS));

  EfiZeroMem(&local_ip, sizeof(EFI_IP_ADDRESS));
  local_ip.v4.Addr[0] = 127;
  local_ip.v4.Addr[3] = 1;

  This->Data->SelectCompleted = FALSE;
  This->Data->IsBootp = FALSE;
  This->Data->IsAck = FALSE;

  efi_status = gBS->HandleProtocol(
    Private->Handle,
    &gEfiPxeDhcp4CallbackProtocolGuid,
    (VOID *)&Private->callback);

  if (EFI_ERROR(efi_status)) {
    Private->callback = NULL;
  }

  Private->function = EFI_PXE_DHCP4_FUNCTION_SELECT;

  //
  // Verify offer packet fields.
  //

  if (Offer->dhcp4.op != BOOTP_REPLY) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (Offer->dhcp4.htype != This->Data->Discover.dhcp4.htype) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (Offer->dhcp4.hlen != This->Data->Discover.dhcp4.hlen) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (EfiCompareMem(&Offer->dhcp4.xid, &This->Data->Discover.dhcp4.xid, 4)) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (!EfiCompareMem(&Offer->dhcp4.yiaddr, &bcast_ip, 4)) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (!EfiCompareMem(&Offer->dhcp4.yiaddr, &zero_ip, 4)) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (!EfiCompareMem(&Offer->dhcp4.yiaddr, &local_ip, 4)) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (EfiCompareMem(&Offer->dhcp4.chaddr,
      &This->Data->Discover.dhcp4.chaddr, 16))
  {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  //
  // DHCP option checks
  //

  dhcp4_magik = htonl(DHCP4_MAGIK_NUMBER);
  is_bootp = TRUE;

  if (!EfiCompareMem(&Offer->dhcp4.magik, &dhcp4_magik, 4)) {
    //
    // If present, DHCP message type must be offer.
    //

    efi_status = find_opt(Offer, DHCP4_MESSAGE_TYPE, 0, &op);

    if (!EFI_ERROR(efi_status)) {
      if (op->len != 1 ||
        op->data[0] != DHCP4_MESSAGE_TYPE_OFFER)
      {
        Private->callback = NULL;
        return EFI_INVALID_PARAMETER;
      }

      is_bootp = FALSE;
    }

    //
    // If present, DHCP max message size must be valid.
    //

    efi_status = find_opt(Offer, DHCP4_MAX_MESSAGE_SIZE, 0, &op);

    if (!EFI_ERROR(efi_status)) {
      if (op->len != 2 ||
        ((op->data[0] << 8) | op->data[1]) < DHCP4_DEFAULT_MAX_MESSAGE_SIZE)
      {
        Private->callback = NULL;
        return EFI_INVALID_PARAMETER;
      }
    }

    //
    // If present, DHCP server identifier must be valid.
    //

    efi_status = find_opt(Offer, DHCP4_SERVER_IDENTIFIER, 0, &op);

    if (!EFI_ERROR(efi_status)) {
      if (op->len != 4 ||
        !EfiCompareMem(op->data, &bcast_ip, 4) ||
        !EfiCompareMem(op->data, &zero_ip, 4))
      {
        Private->callback = NULL;
        return EFI_INVALID_PARAMETER;
      }
    }

    //
    // If present, DHCP subnet mask must be valid.
    //

    efi_status = find_opt(Offer,
      DHCP4_SUBNET_MASK, 0, &op);

    if (!EFI_ERROR(efi_status)) {
      if (op->len != 4) {
        Private->callback = NULL;
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  //
  // Early out for BOOTP.
  //

  if ((This->Data->IsBootp = is_bootp) == TRUE) {
    //
    // Copy offer packet to instance data.
    //

    EfiCopyMem(&This->Data->Offer, Offer, sizeof(DHCP4_PACKET));

    //
    // Copy discover to request and offer to acknak.
    //

    EfiCopyMem(&This->Data->Request, &This->Data->Discover,
      sizeof(DHCP4_PACKET));

    EfiCopyMem(&This->Data->AckNak, &This->Data->Offer,
      sizeof(DHCP4_PACKET));

    //
    // Set state flags.
    //

    This->Data->SelectCompleted = TRUE;
    This->Data->IsAck = TRUE;

    Private->callback = NULL;
    return EFI_SUCCESS;
  }

  //
  // Copy discover packet contents to request packet.
  //

  EfiCopyMem(&request, &This->Data->Discover, sizeof(DHCP4_PACKET));

  This->Data->IsAck = FALSE;

  //
  // Change DHCP message type from discover to request.
  //

  efi_status = find_opt(&request, DHCP4_MESSAGE_TYPE, 0, &op);

  if (EFI_ERROR(efi_status) && efi_status != EFI_NOT_FOUND) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (efi_status == EFI_NOT_FOUND) {
    efi_status = find_opt(&request, DHCP4_END, 0, &op);

    if (EFI_ERROR(efi_status)) {
      Private->callback = NULL;
      return EFI_INVALID_PARAMETER;
    }

    op->op = DHCP4_MESSAGE_TYPE;
    op->len = 1;

    op->data[1] = DHCP4_END;
  }

  op->data[0] = DHCP4_MESSAGE_TYPE_REQUEST;

  //
  // Copy server identifier option from offer to request.
  //

  efi_status = find_opt(Offer, DHCP4_SERVER_IDENTIFIER, 0, &srvid);

  if (EFI_ERROR(efi_status)) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  if (srvid->len != 4) {
    Private->callback = NULL;
    return EFI_INVALID_PARAMETER;
  }

  efi_status = add_opt(&request, srvid);

  if (EFI_ERROR(efi_status)) {
    DebugPrint(("%a:%d:%r\n", __FILE__, __LINE__, efi_status));
    Private->callback = NULL;
    return efi_status;
  }

  //
  // Add requested IP address option to request packet.
  //

  op = (DHCP4_OP *)buf;
  op->op = DHCP4_REQUESTED_IP_ADDRESS;
  op->len = 4;
  EfiCopyMem(op->data, &Offer->dhcp4.yiaddr, 4);

  efi_status = add_opt(&request, op);

  if (EFI_ERROR(efi_status)) {
    DebugPrint(("%a:%d:%r\n", __FILE__, __LINE__, efi_status));
    Private->callback = NULL;
    return efi_status;
  }

  //
  // Transimit DHCP request and wait for DHCP ack...
  //

  EfiSetMem(&bcast_ip, sizeof(EFI_IP_ADDRESS), 0xFF);

  efi_status = tx_rx_udp(Private, &bcast_ip, NULL, NULL, NULL,
    &request, &acknak, &acknak_verify, seconds_timeout);

  if (EFI_ERROR(efi_status)) {
    DebugPrint(("%a:%d:%r\n", __FILE__, __LINE__, efi_status));
    Private->callback = NULL;
    return efi_status;
  }

  //
  // Set Data->IsAck and return.
  //

  efi_status = find_opt(&acknak, DHCP4_MESSAGE_TYPE, 0, &op);

  if (EFI_ERROR(efi_status)) {
    Private->callback = NULL;
    return EFI_DEVICE_ERROR;
  }

  if (op->len != 1) {
    Private->callback = NULL;
    return EFI_DEVICE_ERROR;
  }

  switch (op->data[0]) {
  case DHCP4_MESSAGE_TYPE_ACK:
    This->Data->IsAck = TRUE;
    break;

  case DHCP4_MESSAGE_TYPE_NAK:
    This->Data->IsAck = FALSE;
    break;

  default:
    Private->callback = NULL;
    return EFI_DEVICE_ERROR;
  }

  //
  // Copy packets into instance data...
  //

  EfiCopyMem(&This->Data->Offer, Offer, sizeof(DHCP4_PACKET));
  EfiCopyMem(&This->Data->Request, &request, sizeof(DHCP4_PACKET));
  EfiCopyMem(&This->Data->AckNak, &acknak, sizeof(DHCP4_PACKET));

  This->Data->SelectCompleted = TRUE;

  Private->callback = NULL;
  return EFI_SUCCESS;
}

/* eof - PxeDhcp4InitSelect.c */
