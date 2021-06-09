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
  initialize.c

Abstract:

Revision history:
  2000-Feb-09 M(f)J   Genesis.
--*/

#include "snp.h"

VOID EFIAPI
SnpWaitForPacketNotify(
  IN EFI_EVENT  Event,
  IN VOID       *SnpPtr);

EFI_STATUS
pxe_init (
  SNP_DRIVER *snp,
  UINT16 CableDetectFlag
  )
/*++

Routine Description:
 this routine calls undi to initialize the interface.

Arguments:
 snp  - pointer to snp driver structure
 CableDetectFlag - Do/don't detect the cable (depending on what undi supports)

Returns:

--*/
{
  PXE_CPB_INITIALIZE *cpb = snp->cpb;
  VOID *addr;
  EFI_STATUS Status;

  if (snp->tx_rx_bufsize != 0) {
    Status = snp->IoFncs->AllocateBuffer(
                             snp->IoFncs,
                             AllocateAnyPages,
                             EfiBootServicesData,
                             SNP_MEM_PAGES(snp->tx_rx_bufsize),
                             &addr,
                             0
                             );

    if (Status != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "\nsnp->pxe_init()  AllocateBuffer  %xh (%r)\n",
        Status, Status));

      return Status;
    }

    ASSERT(addr);

    snp->tx_rx_buffer = addr;
  }

  cpb->MemoryAddr = (UINT64)snp->tx_rx_buffer;

  cpb->MemoryLength = snp->tx_rx_bufsize;

  // let UNDI decide/detect these values
  //
  cpb->LinkSpeed = 0;
  cpb->TxBufCnt = 0;
  cpb->TxBufSize = 0;
  cpb->RxBufCnt = 0;
  cpb->RxBufSize = 0;

  cpb->Duplex = PXE_DUPLEX_DEFAULT;

  cpb->LoopBack = LOOPBACK_NORMAL;

  snp->cdb.OpCode = PXE_OPCODE_INITIALIZE;
  snp->cdb.OpFlags = CableDetectFlag;

  snp->cdb.CPBsize = sizeof(PXE_CPB_INITIALIZE);
  snp->cdb.DBsize = sizeof(PXE_DB_INITIALIZE);

  snp->cdb.CPBaddr = (UINT64)snp->cpb;
  snp->cdb.DBaddr = (UINT64)snp->db;

  snp->cdb.StatCode = PXE_STATCODE_INITIALIZE;
  snp->cdb.StatFlags = PXE_STATFLAGS_INITIALIZE;
  snp->cdb.IFnum = snp->if_num;
  snp->cdb.Control = PXE_CONTROL_LAST_CDB_IN_LIST;

  DEBUG((EFI_D_NET, "\nsnp->undi.initialize()  "));

  (*snp->issue_undi32_command) ((UINT64)&snp->cdb);

  if (snp->cdb.StatCode == PXE_STATCODE_SUCCESS) {
    snp->mode.State = EfiSimpleNetworkInitialized;

    Status = EFI_SUCCESS;
  } else {
    DEBUG((EFI_D_WARN, "\nsnp->undi.initialize()  %xh:%xh\n",
      snp->cdb.StatFlags, snp->cdb.StatCode));

    if (snp->tx_rx_buffer != NULL) {
      snp->IoFncs->FreeBuffer(
                     snp->IoFncs,
                     SNP_MEM_PAGES(snp->tx_rx_bufsize),
                     (VOID *)snp->tx_rx_buffer
                     );
    }

    snp->tx_rx_buffer = NULL;

    Status = EFI_DEVICE_ERROR;
  }

  return Status;
}

EFI_STATUS
snp_undi32_initialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *this,
  IN UINTN extra_rx_buffer_size OPTIONAL,
  IN UINTN extra_tx_buffer_size OPTIONAL
)
/*++

Routine Description:
 This is the SNP interface routine for initializing the interface
 This routine basically retrieves snp structure, checks the SNP state and
 calls the pxe_initialize routine to actually do the undi initialization

Arguments:
 this  - context pointer
 extra_rx_buffer_size - optional parameter, indicates extra space for rx_buffers
 extra_tx_buffer_size - optional parameter, indicates extra space for tx_buffers

Returns:

--*/
{
  EFI_STATUS EfiStatus;
  SNP_DRIVER *snp;

  //
  //
  //
  if (this == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  snp = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(this);

  if (snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  //
  //
  switch (snp->mode.State) {
  case EfiSimpleNetworkStarted:
      break;

  case EfiSimpleNetworkStopped:
      return EFI_NOT_STARTED;

  case EfiSimpleNetworkInitialized:
      return EFI_DEVICE_ERROR;

  default:
      return EFI_DEVICE_ERROR;
  }

  //
  //
  //
  EfiStatus = gBS->CreateEvent(
    EFI_EVENT_NOTIFY_WAIT,
    EFI_TPL_NOTIFY,
    &SnpWaitForPacketNotify,
    snp,
    &snp->snp.WaitForPacket);

  if (EFI_ERROR(EfiStatus)) {
    snp->snp.WaitForPacket = NULL;
    return EFI_DEVICE_ERROR;
  }

  //
  // Compute tx/rx buffer sizes based on UNDI init info and parameters.
  //
  snp->tx_rx_bufsize = (UINT32)(snp->init_info.MemoryRequired +
    extra_rx_buffer_size + extra_tx_buffer_size);

  if (snp->mode.MediaPresentSupported) {
    if (pxe_init(snp, PXE_OPFLAGS_INITIALIZE_DETECT_CABLE) == EFI_SUCCESS) {
      snp->mode.MediaPresent = TRUE;
      return EFI_SUCCESS;
    }
  }

  snp->mode.MediaPresent = FALSE;

  EfiStatus = pxe_init(snp, PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE);

  if (EFI_ERROR(EfiStatus)) {
    gBS->CloseEvent(snp->snp.WaitForPacket);
  }

  return EfiStatus;
}
