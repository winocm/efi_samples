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
  PxeDhcp4Setup.c
  
Abstract:

--*/

#include "PxeDhcp4.h"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

EFI_STATUS EFIAPI
PxeDhcp4Setup(
  IN EFI_PXE_DHCP4_PROTOCOL *This,
  IN EFI_PXE_DHCP4_DATA *Data)
{
  PXE_DHCP4_PRIVATE_DATA *Private;
  EFI_STATUS efi_status;
  DHCP4_HEADER *pkt;
  UINT8 *oplen;
  UINT8 *cp;

  //
  // Return error if parameters are invalid.
  //
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = PXE_DHCP4_PRIVATE_DATA_FROM_THIS(This);

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (This->Data != NULL) {
    return EFI_ALREADY_STARTED;
  }

  if (Private->PxeBc == NULL) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Check contents of provided Data structure.
  //
  if (Data != NULL) {
    //
    // Do protocol state checks first.
    //
    if (Data->SelectCompleted) {
      if (!Data->InitCompleted || !Data->SetupCompleted) {
        return EFI_INVALID_PARAMETER;
      }

      if (Data->IsBootp && !Data->IsAck) {
        return EFI_INVALID_PARAMETER;
      }
    } else if (Data->InitCompleted) {
      if (!Data->SetupCompleted || Data->IsBootp || Data->IsAck) {
        return EFI_INVALID_PARAMETER;
      }
    } else if (Data->SetupCompleted) {
      if (Data->IsBootp || Data->IsAck) {
        return EFI_INVALID_PARAMETER;
      }
    }

    //
    // Do packet content checks.
    //
    if (Data->SetupCompleted) {
      // %%TBD - check discover packet
    }

    if (Data->SelectCompleted) {
      if (Data->IsBootp) {
        // %%TBD - check offer packet

        if (EfiCompareMem(&Data->Discover,
          &Data->Request, sizeof(DHCP4_PACKET)))
        {
          return EFI_INVALID_PARAMETER;
        }

        if (EfiCompareMem(&Data->Offer,
          &Data->AckNak, sizeof(DHCP4_PACKET)))
        {
          return EFI_INVALID_PARAMETER;
        }
      } else {
        // %%TBD - check offer, request & acknak packets
      }
    }
  }

  //
  // Allocate data structure.  Return error
  // if there is not enough available memory.
  //
  efi_status = gBS->AllocatePool(EfiBootServicesData,
    sizeof(EFI_PXE_DHCP4_DATA), &This->Data);

  if (EFI_ERROR(efi_status)) {
    This->Data = NULL;
    return efi_status;
  }

  if (This->Data == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Start PxeBc because we want to use its UdpWrite, UdpRead and
  // SetFilter calls.
  //
  efi_status = Private->PxeBc->Start(Private->PxeBc, FALSE);

  if (EFI_ERROR(efi_status)) {
    if (efi_status != EFI_ALREADY_STARTED) {
      gBS->FreePool(This->Data);
      Private->PxeBc->Stop(Private->PxeBc);
      return efi_status;
    }

    Private->StopPxeBc = FALSE;
  } else {
    Private->StopPxeBc = TRUE;
  }

  //
  // Use new data.
  //
  if (Data != NULL) {
    EfiCopyMem(This->Data, Data, sizeof(EFI_PXE_DHCP4_DATA));
    return EFI_SUCCESS;
  }

  //
  // Initialize new public data structure.
  //
  EfiZeroMem(This->Data, sizeof(EFI_PXE_DHCP4_DATA));

  //
  // Fill in default DHCP discover packet.
  // Check for MAC addresses of strange lengths, just in case.
  //
  pkt = &This->Data->Discover.dhcp4;

  pkt->op = BOOTP_REQUEST;

  pkt->htype = Private->Snp->Mode->IfType;

  if (Private->Snp->Mode->HwAddressSize > 16) {
    pkt->hlen = 16;
  } else {
    pkt->hlen = (UINT8)Private->Snp->Mode->HwAddressSize;
  }

  pkt->hops = 0;  /* Set to zero per RFC 2131. */

  if (pkt->hlen < sizeof pkt->xid) {
    if (pkt->hlen != 0) {
      EfiCopyMem(&pkt->xid,
        &Private->Snp->Mode->CurrentAddress,
        pkt->hlen);
    }
  } else {
    EfiCopyMem(&pkt->xid,
      &Private->Snp->Mode->CurrentAddress.Addr[pkt->hlen - sizeof pkt->xid],
      sizeof pkt->xid);
  }

  // %%TBD - xid should be randomized

  pkt->secs = htons(DHCP4_INITIAL_SECONDS);

  pkt->flags = htons(DHCP4_BROADCAST_FLAG);

  if (pkt->hlen != 0) {
    EfiCopyMem(pkt->chaddr, &Private->Snp->Mode->CurrentAddress, pkt->hlen);
  }

  pkt->magik = htonl(DHCP4_MAGIK_NUMBER);

  cp = pkt->options;

  *cp++ = DHCP4_MESSAGE_TYPE;
  *cp++ = 1;
  *cp++ = DHCP4_MESSAGE_TYPE_DISCOVER;

  *cp++ = DHCP4_MAX_MESSAGE_SIZE;
  *cp++ = 2;
  *cp++ = (UINT8)((DHCP4_DEFAULT_MAX_MESSAGE_SIZE >> 8) & 0xFF);
  *cp++ = (UINT8)(DHCP4_DEFAULT_MAX_MESSAGE_SIZE & 0xFF);

  *cp++ = DHCP4_PARAMETER_REQUEST_LIST;
  oplen = cp;
  *cp++ = 0;
  *cp++ = DHCP4_SUBNET_MASK;
  *cp++ = DHCP4_TIME_OFFSET;
  *cp++ = DHCP4_ROUTER_LIST;
  *cp++ = DHCP4_TIME_SERVERS;
  *cp++ = DHCP4_NAME_SERVERS;
  *cp++ = DHCP4_DNS_SERVERS;
  *cp++ = DHCP4_HOST_NAME;
  *cp++ = DHCP4_BOOT_FILE_SIZE;
  *cp++ = DHCP4_MESSAGE_TYPE;
  *cp++ = DHCP4_DOMAIN_NAME;
  *cp++ = DHCP4_ROOT_PATH;
  *cp++ = DHCP4_EXTENSION_PATH;
  *cp++ = DHCP4_MAX_DATAGRAM_SIZE;
  *cp++ = DHCP4_DEFAULT_TTL;
  *cp++ = DHCP4_BROADCAST_ADDRESS;
  *cp++ = DHCP4_NIS_DOMAIN_NAME;
  *cp++ = DHCP4_NIS_SERVERS;
  *cp++ = DHCP4_NTP_SERVERS;
  *cp++ = DHCP4_VENDOR_SPECIFIC;
  *cp++ = DHCP4_REQUESTED_IP_ADDRESS;
  *cp++ = DHCP4_LEASE_TIME;
  *cp++ = DHCP4_SERVER_IDENTIFIER;
  *cp++ = DHCP4_RENEWAL_TIME;
  *cp++ = DHCP4_REBINDING_TIME;
  *cp++ = DHCP4_CLASS_IDENTIFIER;
  *cp++ = DHCP4_TFTP_SERVER_NAME;
  *cp++ = DHCP4_BOOTFILE;
  *cp++ = 128; 
  *cp++ = 129; 
  *cp++ = 130; 
  *cp++ = 131; 
  *cp++ = 132;
  *cp++ = 133;
  *cp++ = 134; 
  *cp++ = 135;
  *oplen = (UINT8)((cp - oplen) - 1);

  *cp++ = DHCP4_END;

  This->Data->SetupCompleted = TRUE;

  return EFI_SUCCESS;
}

/* eof - PxeDhcp4Setup.c */
