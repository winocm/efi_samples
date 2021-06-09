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
  shutdown.c

Abstract:

Revision history:
  2000-Feb-14 M(f)J   Genesis.
--*/

#include "snp.h"

EFI_STATUS pxe_shutdown (
  IN SNP_DRIVER *snp
  )
/*++

Routine Description:
 this routine calls undi to shut down the interface.

Arguments:
  snp  - pointer to snp driver structure

Returns:

--*/
{
  snp->cdb.OpCode = PXE_OPCODE_SHUTDOWN;
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

  DEBUG((EFI_D_NET, "\nsnp->undi.shutdown()  "));

  (*snp->issue_undi32_command)((UINT64)&snp->cdb);

  if (snp->cdb.StatCode != PXE_STATCODE_SUCCESS) {
    //
    // UNDI could not be shutdown. Return UNDI error.
    //
    DEBUG((EFI_D_WARN, "\nsnp->undi.shutdown()  %xh:%xh\n", snp->cdb.StatFlags, snp->cdb.StatCode));

    return EFI_DEVICE_ERROR;
  }

  //
  // Free allocated memory.
  //

  if (snp->tx_rx_buffer != NULL) {
    snp->IoFncs->FreeBuffer(
                    snp->IoFncs,
                    SNP_MEM_PAGES(snp->tx_rx_bufsize),
                    (VOID *)snp->tx_rx_buffer
                    );
  }

  snp->tx_rx_buffer = NULL;
  snp->tx_rx_bufsize = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
snp_undi32_shutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *this
  )
/*++

Routine Description:
 This is the SNP interface routine for shutting down the interface
 This routine basically retrieves snp structure, checks the SNP state and
 calls the pxe_shutdown routine to actually do the undi shutdown

Arguments:
  this  - context pointer

Returns:

--*/
{
  SNP_DRIVER *snp;
  EFI_STATUS status;

  //
  //
  //
  if (this == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  snp = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(this);

  if (snp == NULL) {
    return EFI_DEVICE_ERROR;
  }

  //
  //
  //
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

  //
  //
  //
  status = pxe_shutdown(snp);

  snp->mode.State = EfiSimpleNetworkStarted;
  snp->mode.ReceiveFilterSetting = 0;

  gBS->CloseEvent(snp->snp.WaitForPacket);

  return status;
}

