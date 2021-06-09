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
  start.c

Abstract:

Revision history:
  2000-Feb-07 M(f)J   Genesis.
--*/

#include "snp.h"

EFI_STATUS
pxe_start (
  SNP_DRIVER *snp
  )
/*++

Routine Description:
 this routine calls undi to start the interface and changes the snp state!

Arguments:
  snp  - pointer to snp driver structure

Returns:

--*/
{
  PXE_CPB_START *cpb = snp->cpb;
  PXE_CPB_START_31 *cpb_31 = snp->cpb;

  //
  // Initialize UNDI Start CDB for H/W UNDI
  //

  snp->cdb.OpCode = PXE_OPCODE_START;
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
  // Make changes to H/W UNDI Start CDB if this is
  // a S/W UNDI.
  //

  if (snp->is_swundi) {
    if (snp->IsOldUndi) {
       snp->cdb.CPBsize = sizeof (PXE_CPB_START);
       snp->cdb.CPBaddr = (UINT64)cpb;

       cpb->Delay = (UINT64)&snp_undi32_callback_delay_30;
       cpb->Block = (UINT64)&snp_undi32_callback_block_30;

       //
       // Virtual == Physical.  This can be set to zero.
       //
       cpb->Virt2Phys = (UINT64)&snp_undi32_callback_v2p_30;
       cpb->Mem_IO = (UINT64)&snp_undi32_callback_memio_30;
    } else {
       snp->cdb.CPBsize = sizeof (PXE_CPB_START_31);
       snp->cdb.CPBaddr = (UINT64)cpb_31;

       cpb_31->Delay = (UINT64)&snp_undi32_callback_delay;
       cpb_31->Block = (UINT64)&snp_undi32_callback_block;

       //
       // Virtual == Physical.  This can be set to zero.
       //
       cpb_31->Virt2Phys = (UINT64)0;
       cpb_31->Mem_IO = (UINT64)&snp_undi32_callback_memio;

       cpb_31->Map_Mem = (UINT64)&snp_undi32_callback_map;
       cpb_31->UnMap_Mem = (UINT64)&snp_undi32_callback_unmap;
       cpb_31->Sync_Mem = (UINT64)&snp_undi32_callback_sync;

       cpb_31->Unique_ID = (UINT64)snp;
    }
  }

  //
  // Issue UNDI command and check result.
  //

  DEBUG((EFI_D_NET, "\nsnp->undi.start()  "));

  (*snp->issue_undi32_command) ((UINT64)&snp->cdb);

  if (snp->cdb.StatCode != PXE_STATCODE_SUCCESS) {
    //
    // UNDI could not be started. Return UNDI error.
    //

    DEBUG((EFI_D_ERROR, "\nsnp->undi.start()  %xh:%xh\n",
      snp->cdb.StatCode, snp->cdb.StatFlags));

    return EFI_DEVICE_ERROR;
  }

  //
  // Set simple network state to Started and return success.
  //

  snp->mode.State = EfiSimpleNetworkStarted;

  return EFI_SUCCESS;
}


EFI_STATUS
snp_undi32_start (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *this
  )
/*++

Routine Description:
 This is the SNP interface routine for starting the interface
 This routine basically retrieves snp structure, checks the SNP state and
 calls the pxe_start routine to actually do start undi interface

Arguments:
  this  - context pointer

Returns:

--*/
{
  SNP_DRIVER *snp;
  EFI_STATUS status;
  int i;

  if (this == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  snp = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(this);

  if (snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  switch (snp->mode.State) {
    case EfiSimpleNetworkStopped:
        break;

    case EfiSimpleNetworkStarted:
    case EfiSimpleNetworkInitialized:
        return EFI_ALREADY_STARTED;

    default:
        return EFI_DEVICE_ERROR;
  }

  if ((status = pxe_start (snp)) != EFI_SUCCESS) {
    return status;
  }

  // clear the map_list in SNP structure
  for (i = 0; i < MAX_MAP_LENGTH; i++) {
    snp->map_list[i].virt = 0;
    snp->map_list[i].map_cookie = 0;
  }

  snp->mode.MCastFilterCount = 0;

  return status;
}

