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
  PxeDhcp4Run.c
  
Abstract:
  Simplified entry point for starting basic PxeDhcp4 client operation.

--*/

#include "PxeDhcp4.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

EFI_STATUS EFIAPI
PxeDhcp4Run(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN OPTIONAL UINTN OpLen,
  IN OPTIONAL VOID *OpList)
{
  PXE_DHCP4_PRIVATE_DATA *Private;
  DHCP4_PACKET *offer_list;
  EFI_STATUS efi_status;
  EFI_IP_ADDRESS zero_ip;
  UINTN offers;
  UINTN timeout;
  UINTN n;
  UINT16 seconds;

  //
  // Validate parameters.
  //

  if (This == NULL ||
    (OpLen != 0 && OpList == NULL) ||
    (OpLen == 0 && OpList != NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  for (n = 0; n < OpLen; ) {
    switch (((UINT8 *)OpList)[n]) {
    case DHCP4_PAD:
      ++n;
      continue;

    case DHCP4_END:
      ++n;
      break;

    default:
      n += 2 + ((UINT8 *)OpList)[n + 1];
      continue;
    }

    break;
  }

  if (n != OpLen) {
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
  // Initialize DHCP discover packet.
  //

  efi_status = PxeDhcp4Setup(This, NULL);

  if (EFI_ERROR(efi_status)) {
    return efi_status;
  }

  for (n = 0; n < OpLen; ) {
    switch (((UINT8 *)OpList)[n]) {
    case DHCP4_PAD:
      ++n;
      continue;

    case DHCP4_END:
      ++n;
      break;

    default:
      efi_status = add_opt(&This->Data->Discover,
        (DHCP4_OP *)&(((UINT8 *)OpList)[n]));

      if (EFI_ERROR(efi_status)) {
        return efi_status;
      }

      n += 2 + ((UINT8 *)OpList)[n + 1];
      continue;
    }

    break;
  }
 
  //
  // Basic DHCP D.O.R.A.
  // 1, 2, 4, 8, 16 & 32 second timeouts.
  // Callback routine can be used to break out earlier.
  //

  EfiZeroMem(&zero_ip, sizeof(EFI_IP_ADDRESS));

  for (timeout = 1; ; ) {
    //
    // Broadcast DHCP discover and wait for DHCP offers.
    //

    efi_status = PxeDhcp4Init(This, timeout, &offers, &offer_list);

    switch (efi_status) {
    case EFI_NO_RESPONSE:
    case EFI_TIMEOUT:
    case EFI_SUCCESS:
      break;

    case EFI_ABORTED:
    default:
      return efi_status;
    }

    //
    // Try to select from each DHCP or BOOTP offer.
    //

    for (n = 0; n < offers; ++n) {
      //
      // Ignore proxyDHCP offers.
      //

      if (!EfiCompareMem(&offer_list[n].dhcp4.yiaddr, &zero_ip, 4))
      {
        continue;
      }

      //
      // Issue DHCP Request and wait for DHCP Ack/Nak.
      //

      efi_status = PxeDhcp4Select(This, timeout,
        &offer_list[n]);

      if (EFI_ERROR(efi_status)) {
        continue;
      }

      //
      // Exit when we have got our DHCP Ack.
      //

      if (This->Data->IsAck) {
        return EFI_SUCCESS;
      }
    }

    //
    // No DHCP Acks.  Release DHCP Offer list storage.
    //

    if (offer_list != NULL) {
      gBS->FreePool(offer_list);
      offer_list = NULL;
    }

    //
    // Try again until we have used up >= DHCP4_MAX_SECONDS.
    //

    if ((timeout <<= 1) > DHCP4_MAX_SECONDS) {
      if (!EFI_ERROR(efi_status)) {
        efi_status = EFI_TIMEOUT;
      }

      return efi_status;
    }

    //
    // Next timeout value.
    //

    EfiCopyMem(&seconds, &This->Data->Discover.dhcp4.secs, 2);

    seconds = htons(htons(seconds) + timeout);

    EfiCopyMem(&This->Data->Discover.dhcp4.secs, &seconds, 2);
  }
}

/* eof - PxeDhcp4Run.c */
