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
  reset.c

Abstract:

Revision history:
  2000-Feb-09 M(f)J   Genesis.
--*/

#include "snp.h"

EFI_STATUS
pxe_reset (
  SNP_DRIVER *snp
  )
/*++

Routine Description:
 This routine calls undi to reset the nic.

Arguments:
 snp - pointer to the snp driver structure

Returns:
 EFI_SUCCESSFUL for a successful completion
 other for failed calls

--*/
{
  snp->cdb.OpCode = PXE_OPCODE_RESET;
  snp->cdb.OpFlags = PXE_OPFLAGS_NOT_USED;
  snp->cdb.CPBsize = PXE_CPBSIZE_NOT_USED;
  snp->cdb.DBsize = PXE_DBSIZE_NOT_USED;
  snp->cdb.CPBaddr = PXE_CPBADDR_NOT_USED;
  snp->cdb.DBaddr = PXE_DBADDR_NOT_USED;
  snp->cdb.StatCode = PXE_STATCODE_INITIALIZE;
  snp->cdb.StatFlags = PXE_STATFLAGS_INITIALIZE;
  snp->cdb.IFnum = snp->if_num;
  snp->cdb.Control = PXE_CONTROL_LAST_CDB_IN_LIST;

  //
  // Issue UNDI command and check result.
  //

  DEBUG((EFI_D_NET, "\nsnp->undi.reset()  "));

  (*snp->issue_undi32_command) ((UINT64)&snp->cdb);

  if (snp->cdb.StatCode != PXE_STATCODE_SUCCESS) {
    DEBUG((EFI_D_WARN, "\nsnp->undi32.reset()  %xh:%xh\n",
      snp->cdb.StatFlags, snp->cdb.StatCode));

    //
    // UNDI could not be reset. Return UNDI error.
    //

    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
snp_undi32_reset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *this,
  IN BOOLEAN              ExtendedVerification
)
/*++

Routine Description:
 This is the SNP interface routine for resetting the NIC
 This routine basically retrieves snp structure, checks the SNP state and
 calls the pxe_reset routine to actually do the reset!

Arguments:
 this - context pointer
 ExtendedVerification - not implemented

Returns:

--*/
{
  SNP_DRIVER *snp;

  //
  // Resolve Warning 4 unreferenced parameter problem
  //

  ExtendedVerification = 0;

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
   
  return (pxe_reset (snp));
}

