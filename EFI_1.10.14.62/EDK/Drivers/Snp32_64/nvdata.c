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
  nvdata.c

Abstract:

Revision history:
  2000-Feb-03 M(f)J   Genesis.
--*/

#include "snp.h"

EFI_STATUS
pxe_nvdata_read (
  IN SNP_DRIVER *snp,
  IN UINTN      RegOffset,
  IN UINTN      NumBytes,
  IN OUT VOID   *BufferPtr
  )
/*++

Routine Description:
 This routine calls Undi to read the desired number of eeprom bytes.

Arguments:
  snp  - pointer to the snp driver structure
  RegOffset  - eeprom register value relative to the base address
  NumBytes  - number of bytes to read
  BufferPtr - pointer where to read into

Returns:

--*/
{
  PXE_DB_NVDATA *db = snp->db;

  snp->cdb.OpCode = PXE_OPCODE_NVDATA;

  snp->cdb.OpFlags = PXE_OPFLAGS_NVDATA_READ;

  snp->cdb.CPBsize = PXE_CPBSIZE_NOT_USED;
  snp->cdb.CPBaddr = PXE_CPBADDR_NOT_USED;

  snp->cdb.DBsize = sizeof (PXE_DB_NVDATA);
  snp->cdb.DBaddr = (UINT64)db;

  snp->cdb.StatCode = PXE_STATCODE_INITIALIZE;
  snp->cdb.StatFlags = PXE_STATFLAGS_INITIALIZE;
  snp->cdb.IFnum = snp->if_num;
  snp->cdb.Control = PXE_CONTROL_LAST_CDB_IN_LIST;

  //
  // Issue UNDI command and check result.
  //

  DEBUG((EFI_D_NET, "\nsnp->undi.nvdata ()  "));

  (*snp->issue_undi32_command) ((UINT64)&snp->cdb);

  switch (snp->cdb.StatCode) {
  case PXE_STATCODE_SUCCESS:
    break;

  case PXE_STATCODE_UNSUPPORTED:
    DEBUG((EFI_D_NET, "\nsnp->undi.nvdata()  %xh:%xh\n",
      snp->cdb.StatFlags, snp->cdb.StatCode));

    return EFI_UNSUPPORTED;

  default:
    DEBUG((EFI_D_NET, "\nsnp->undi.nvdata()  %xh:%xh\n",
      snp->cdb.StatFlags, snp->cdb.StatCode));

    return EFI_DEVICE_ERROR;
  }

  EfiCopyMem (BufferPtr, db->Data.Byte + RegOffset, NumBytes);

  return EFI_SUCCESS;
}

EFI_STATUS
snp_undi32_nvdata (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *this,
  IN BOOLEAN ReadOrWrite,
  IN UINTN RegOffset,
  IN UINTN NumBytes,
  IN OUT VOID *BufferPtr
)
/*++

Routine Description:
 This is an interface call provided by SNP.
 It does the basic checking on the input parameters and retrieves snp structure
 and then calls the read_nvdata() call which does the actual reading

Arguments:
  this  - context pointer
  ReadOrWrite - true for reading and false for writing
  RegOffset  - eeprom register relative to the base
  NumBytes - how many bytes to read
  BufferPtr - address of memory to read into

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

  //
  // Return error if the SNP is not initialized.
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
  // Check for invalid parameter combinations.
  //

  if ( (NumBytes == 0) || (BufferPtr == NULL) ||
       (NumBytes + RegOffset > (MAX_EEPROM_LEN << 2)) ) {
    return EFI_INVALID_PARAMETER;
  }

  // check the implementation flags of undi if we can write the nvdata!

  if (!ReadOrWrite) {
    return EFI_UNSUPPORTED;
  } else {
    return pxe_nvdata_read (snp, RegOffset, NumBytes, BufferPtr);
  }
}

