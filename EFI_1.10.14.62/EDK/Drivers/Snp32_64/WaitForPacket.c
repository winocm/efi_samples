/*++
Copyright (c) 1999 - 2002 Intel Corporation.  All rights reserved.

This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module name:
  WaitForPacket.c

Abstract:
  Event handler to check for available packet.

--*/

#include "snp.h"

VOID EFIAPI
SnpWaitForPacketNotify(
  EFI_EVENT Event,
  VOID      *SnpPtr)
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  PXE_DB_GET_STATUS PxeDbGetStatus;

  //
  // Do nothing if either parameter is a NULL pointer.
  //
  if (Event == NULL || SnpPtr == NULL) {
    return;
  }

  //
  // Do nothing if the SNP interface is not initialized.
  //
  switch (((SNP_DRIVER *)SnpPtr)->mode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
  case EfiSimpleNetworkStarted:
  default:
    return;
  }

  //
  // Fill in CDB for UNDI GetStatus().
  //
  ((SNP_DRIVER *)SnpPtr)->cdb.OpCode = PXE_OPCODE_GET_STATUS;
  ((SNP_DRIVER *)SnpPtr)->cdb.OpFlags = 0;
  ((SNP_DRIVER *)SnpPtr)->cdb.CPBsize = PXE_CPBSIZE_NOT_USED;
  ((SNP_DRIVER *)SnpPtr)->cdb.CPBaddr = PXE_CPBADDR_NOT_USED;
  ((SNP_DRIVER *)SnpPtr)->cdb.DBsize = sizeof(UINT32) * 2;
  ((SNP_DRIVER *)SnpPtr)->cdb.DBaddr = (UINT64)(((SNP_DRIVER *)SnpPtr)->db);
  ((SNP_DRIVER *)SnpPtr)->cdb.StatCode = PXE_STATCODE_INITIALIZE;
  ((SNP_DRIVER *)SnpPtr)->cdb.StatFlags = PXE_STATFLAGS_INITIALIZE;
  ((SNP_DRIVER *)SnpPtr)->cdb.IFnum = ((SNP_DRIVER *)SnpPtr)->if_num;
  ((SNP_DRIVER *)SnpPtr)->cdb.Control = PXE_CONTROL_LAST_CDB_IN_LIST;

  //
  // Clear contents of DB buffer.
  //
  EfiZeroMem(((SNP_DRIVER *)SnpPtr)->db, sizeof(UINT32) * 2);

  //
  // Issue UNDI command and check result.
  //
  (*((SNP_DRIVER *)SnpPtr)->issue_undi32_command)((UINT64)&((SNP_DRIVER *)SnpPtr)->cdb);

  if (((SNP_DRIVER *)SnpPtr)->cdb.StatCode != EFI_SUCCESS) {
    return;
  }

  //
  // We might have a packet.  Check the receive length and signal
  // the event if the length is not zero.
  //
  EfiCopyMem(
    &PxeDbGetStatus, 
    ((SNP_DRIVER *)SnpPtr)->db,
    sizeof(UINT32) * 2);

  if (PxeDbGetStatus.RxFrameLen != 0) {
    gBS->SignalEvent(Event);
  }
}

/* eof - WaitForPacket.c */
