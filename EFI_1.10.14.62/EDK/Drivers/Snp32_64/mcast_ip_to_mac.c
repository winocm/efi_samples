/*++
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module name:
  mcast_ip_to_mac.c

Abstract:

Revision history:
  2000-Feb-17 M(f)J   Genesis.
--*/

#include "snp.h"

EFI_STATUS
pxe_ip2mac (
  IN SNP_DRIVER *snp,
  IN BOOLEAN IPv6,
  IN EFI_IP_ADDRESS *IP,
  IN OUT EFI_MAC_ADDRESS *MAC
  )
/*++

Routine Description:
 this routine calls undi to convert an multicast IP address to a MAC address

Arguments:
  snp  - pointer to snp driver structure
  IPv6  - flag to indicate if this is an ipv6 address
  IP    - multicast IP address
  MAC   - pointer to hold the return MAC address

Returns:

--*/
{
  PXE_CPB_MCAST_IP_TO_MAC *cpb = snp->cpb;
  PXE_DB_MCAST_IP_TO_MAC *db = snp->db;

  snp->cdb.OpCode = PXE_OPCODE_MCAST_IP_TO_MAC;
  snp->cdb.OpFlags = (UINT16)(IPv6 ?
                              PXE_OPFLAGS_MCAST_IPV6_TO_MAC :
                              PXE_OPFLAGS_MCAST_IPV4_TO_MAC);
  snp->cdb.CPBsize = sizeof (PXE_CPB_MCAST_IP_TO_MAC);
  snp->cdb.DBsize = sizeof (PXE_DB_MCAST_IP_TO_MAC);

  snp->cdb.CPBaddr = (UINT64)cpb;
  snp->cdb.DBaddr = (UINT64)db;

  snp->cdb.StatCode = PXE_STATCODE_INITIALIZE;
  snp->cdb.StatFlags = PXE_STATFLAGS_INITIALIZE;
  snp->cdb.IFnum = snp->if_num;
  snp->cdb.Control = PXE_CONTROL_LAST_CDB_IN_LIST;

  EfiCopyMem (&cpb->IP, IP, sizeof (PXE_IP_ADDR));

  //
  // Issue UNDI command and check result.
  //

  DEBUG((EFI_D_NET, "\nsnp->undi.mcast_ip_to_mac()  "));

  (*snp->issue_undi32_command) ((UINT64)&snp->cdb);

  switch (snp->cdb.StatCode) {
    case PXE_STATCODE_SUCCESS:
        break;

    case PXE_STATCODE_UNSUPPORTED:
        DEBUG((EFI_D_NET, "\nsnp->undi.mcast_ip_to_mac()  %xh:%xh\n",
              snp->cdb.StatFlags, snp->cdb.StatCode));
        return EFI_UNSUPPORTED;

    default:
       //
       // UNDI command failed.  Return EFI_DEVICE_ERROR
       // to caller.
       //

       DEBUG((EFI_D_NET, "\nsnp->undi.mcast_ip_to_mac()  %xh:%xh\n",
               snp->cdb.StatFlags, snp->cdb.StatCode));

       return EFI_DEVICE_ERROR;
  }

  EfiCopyMem (MAC, &db->MAC, sizeof(PXE_MAC_ADDR));
  return EFI_SUCCESS;
}

EFI_STATUS
snp_undi32_mcast_ip_to_mac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *this,
  IN BOOLEAN IPv6,
  IN EFI_IP_ADDRESS *IP,
  OUT EFI_MAC_ADDRESS *MAC
  )
/*++

Routine Description:
 This is the SNP interface routine for converting a multicast IP address to
 a MAC address.
 This routine basically retrieves snp structure, checks the SNP state and
 calls the pxe_ip2mac routine to actually do the conversion

Arguments:
  this  - context pointer
  IPv6  - flag to indicate if this is an ipv6 address
  IP    - multicast IP address
  MAC   - pointer to hold the return MAC address

Returns:

--*/
{
  SNP_DRIVER *snp;

  //
  // Get pointer to SNP driver instance for *this.
  //

  if (this == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  snp = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(this);

  if (snp == NULL) {
    return EFI_DEVICE_ERROR;
  }

  switch (snp->mode.State) {
    case EfiSimpleNetworkInitialized:
        break;

    case EfiSimpleNetworkStopped:
        return EFI_NOT_STARTED;

    case EfiSimpleNetworkStarted:
        return EFI_DEVICE_ERROR;

    default:
        return EFI_DEVICE_ERROR;
  }

  if (IP == NULL || MAC == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return (pxe_ip2mac (snp, IPv6, IP, MAC));
}

