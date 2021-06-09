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
 callback.c

Abstract:
 This file contains two sets of callback routines for undi3.0 and undi3.1.
 the callback routines for Undi3.1 have an extra parameter UniqueId which
 stores the interface context for the NIC that snp is trying to talk..

Revision history:
  2002-June-3 Varalakshmi
  2000-Feb-16 M(f)J   Genesis.
--*/

#include "snp.h"

// Global variables

// these 2 global variables are used only for 3.0 undi. we could not place
// them in the snp structure because we will not know which snp structure
// in the callback context!
//
STATIC BOOLEAN mInitializeLock = TRUE;
STATIC EFI_LOCK mLock;

// End Global variables
extern EFI_PCI_IO_PROTOCOL *mPciIoFncs;


VOID
snp_undi32_callback_v2p_30 (
  IN UINT64     CpuAddr,
  IN OUT UINT64 DeviceAddrPtr
  )
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 UNDI call this routine with a virtual or CPU address that SNP provided
 to convert it to a physical or device address. Since EFI uses the identical
 mapping, this routine returns the physical address same as the virtual address
 for most of the addresses. an address above 4GB cannot generally be used as a
 device address, it needs to be mapped to a lower physical address. This routine
 does not call the map routine itself, but it assumes that the mapping was done
 at the time of providing the address to UNDI. This routine just looks up the
 address in a map table (which is the v2p structure chain)

Arguments:
 CpuAddr - virtual address of a buffer
 DeviceAddrPtr - pointer to the physical address

Returns:
 void - The DeviceAddrPtr will contain 0 in case of any error

--*/
{
  struct s_v2p *v2p;
  //
  // Do nothing if virtual address is zero or physical pointer is NULL.
  // No need to map if the virtual address is within 4GB limit since
  // EFI uses identical mapping
  //
  if ((CpuAddr == 0) || (DeviceAddrPtr == 0)) {
    DEBUG((EFI_D_ERROR, "\nv2p: Null virtual address or physical pointer.\n"));
    return;
  }

  if (CpuAddr < FOUR_GIGABYTES) {
    *(UINT64 *)(UINTN)DeviceAddrPtr = CpuAddr;
    return;
  }
  //
  // SNP creates a vaddr tp paddr mapping at the time of calling undi with any
  // big address, this callback routine just looks up in the v2p list and
  // returns the physical address for any given virtual address.
  //
  if (find_v2p (&v2p, (VOID *)(UINTN)CpuAddr) != EFI_SUCCESS) {
    *(UINT64 *)(UINTN)DeviceAddrPtr = CpuAddr;
  } else {
    *(UINT64 *)(UINTN)DeviceAddrPtr = v2p->paddr;
  }
}

VOID
snp_undi32_callback_block_30 (
  IN UINT32 Enable
  )
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 UNDI call this routine when it wants to have exclusive access to a critical
 section of the code/data

Arguments:
 Enable - non-zero indicates acquire
          zero indicates release

Returns:
 void
--*/
{
  // tcpip was calling snp at tpl_notify and if we acquire a lock that was
  // created at a lower level (TPL_CALLBACK) it gives an assert!
  //
  if (mInitializeLock) {
    EfiInitializeLock (&mLock, EFI_TPL_NOTIFY);
    mInitializeLock = FALSE;
  }

  if (Enable != 0)
    EfiAcquireLock (&mLock);
  else
    EfiReleaseLock (&mLock);
}

VOID
snp_undi32_callback_delay_30 (
  IN UINT64 MicroSeconds)
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 UNDI call this routine with the number of micro seconds when it wants to
 pause.

Arguments:
 MicroSeconds - number of micro seconds to pause, ususlly multiple of 10

Returns:
 void
--*/
{
  if (MicroSeconds != 0)
    gBS->Stall ((UINTN)MicroSeconds);
}

VOID
snp_undi32_callback_memio_30 (
  IN UINT8 ReadOrWrite,
  IN UINT8 NumBytes,
  IN UINT64 Address,
  IN OUT UINT64 BufferAddr
  )
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 This is the IO routine for UNDI. This is not currently being used by UNDI3.0
 because Undi3.0 uses io/mem offsets relative to the beginning of the device
 io/mem address and so it needs to use the PCI_IO_FUNCTION that abstracts the
 start of the device's io/mem addresses. Since SNP cannot retrive the context
 of the undi3.0 interface it cannot use the PCI_IO_FUNCTION that specific for
 that NIC and uses one global IO functions structure, this does not work.
 This however works fine for EFI1.0 Undis because they use absolute addresses
 for io/mem access.

Arguments:
  ReadOrWrite - indicates read or write, IO or Memory
  NumBytes    - number of bytes to read or write
  Address     - IO or memory address to read from or write to
  BufferAddr  - memory location to read into or that contains the bytes
                to write

Returns:

--*/
{
  EFI_PCI_IO_PROTOCOL_WIDTH Width;

  switch (NumBytes) {
    case 2:
        Width = 1;
        break;
    case 4:
        Width = 2;
        break;
    case 8:
        Width = 3;
        break;
    default:
        Width = 0;
  }
  switch (ReadOrWrite) {
    case PXE_IO_READ:
        mPciIoFncs->Io.Read (
                           mPciIoFncs,
                           Width,
                           1,       // BAR 1, IO base address
                           Address,
                           1,       // count
                           (VOID *)(UINTN)BufferAddr
                           );
        break;
    case PXE_IO_WRITE:
        mPciIoFncs->Io.Write (
                           mPciIoFncs,
                           Width,
                           1,       // BAR 1, IO base address
                           Address,
                           1,       // count
                           (VOID *)(UINTN)BufferAddr
                           );
        break;
    case PXE_MEM_READ:
        mPciIoFncs->Mem.Read (
                            mPciIoFncs,
                            Width,
                            0,       // BAR 0, Memory base address
                            Address,
                            1,       // count
                            (VOID *)(UINTN)BufferAddr
                            );
        break;
    case PXE_MEM_WRITE:
        mPciIoFncs->Mem.Write (
                            mPciIoFncs,
                            Width,
                            0,       // BAR 0, Memory base address
                            Address,
                            1,       // count
                            (VOID *)(UINTN)BufferAddr
                            );
        break;
  }
  return;
}


// New callbacks for 3.1:
// there won't be a virtual2physical callback for UNDI 3.1 because undi3.1 uses
// the MemMap call to map the required address by itself!
//

VOID
snp_undi32_callback_block (
  IN UINT64 UniqueId,
  IN UINT32 Enable)
/*++

Routine Description:
 This is a callback routine supplied to UNDI3.1 at undi_start time.
 UNDI call this routine when it wants to have exclusive access to a critical
 section of the code/data

Arguments:
 UniqueId - This was supplied to UNDI at Undi_Start, SNP uses this to store
            Undi interface context (Undi does not read or write this variable)
 Enable   - non-zero indicates acquire
            zero indicates release

Returns:
 void

--*/
{
  SNP_DRIVER *snp = (SNP_DRIVER *)(UINTN)UniqueId;

  // tcpip was calling snp at tpl_notify and when we acquire a lock that was
  // created at a lower level (TPL_CALLBACK) it gives an assert!

  if (Enable != 0)
    EfiAcquireLock (&snp->lock);
  else
    EfiReleaseLock (&snp->lock);
}

VOID
snp_undi32_callback_delay (
  IN UINT64 UniqueId,
  IN UINT64 MicroSeconds
  )
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 UNDI call this routine with the number of micro seconds when it wants to
 pause.

Arguments:
 MicroSeconds - number of micro seconds to pause, ususlly multiple of 10

Returns:
 void
--*/
{
  if (MicroSeconds != 0)
    gBS->Stall ((UINTN)MicroSeconds);
}

/*
 *  IO routine for UNDI start CPB.
 */

VOID
snp_undi32_callback_memio (
  UINT64 UniqueId,
  UINT8 ReadOrWrite,
  UINT8 NumBytes,
  UINT64 Address,
  UINT64 BufferAddr
  )
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 This is the IO routine for UNDI3.1.

Arguments:
  ReadOrWrite - indicates read or write, IO or Memory
  NumBytes    - number of bytes to read or write
  Address     - IO or memory address to read from or write to
  BufferAddr  - memory location to read into or that contains the bytes
                to write

Returns:

--*/
{
  static  UINT8               FirstCall = TRUE;
  static  UINT8               MemBar;
  static  UINT8               IoBar;

  SNP_DRIVER                  *snp = (SNP_DRIVER *)(UINTN)UniqueId;
  EFI_PCI_IO_PROTOCOL_WIDTH   Width = 0;
  UINT32                      Buffer[6];

  //
  // This code assumes that the first two BARs are the device memory
  // and I/O access BARs. We try to find out which one is at offset 0x10 (which
  // one comes first).
  //
  if (FirstCall) {
    //
    // Read BAR-0
    //
    mPciIoFncs->Pci.Read(
      mPciIoFncs,
      EfiPciIoWidthUint32,
      0x10,
      6,
      Buffer);

    //
    // Is BAR-0 memory or I/O? PCI SPec 2.2 says, Bit 0 determines if this
    // is a memory or I/O BAR.
    //
    if (Buffer[0] & 1) {
      IoBar = 0;
      MemBar = 1;
    } else {
      MemBar = 0;

      //
      // Is it 32 bit or 64 bit memory bar? PCI spec 2.2 (p.222) says that bits
      // 1 and 2 determine if this is a 64 bit or 32 bit memory bar.
      // There are no 64 bit I/O bars
      //
      switch (Buffer[0] & 0x6) {
      case 0x4: // 64 bit
        IoBar = 2;
        break;

      case 0x0: // 32 bit
        IoBar = 1;
        break;

      default:
        DEBUG((EFI_D_ERROR, "Snp3264/Callback.c, %d\n", __LINE__));
        return;
      }
    }
    //
    //
    //
    FirstCall = FALSE;
  }

  //
  //
  //
  switch (NumBytes) {
  case 2:
    Width = 1;
    break;

  case 4:
    Width = 2;
    break;

  case 8:
    Width = 3;
    break;
  }

  //
  //
  //
  switch (ReadOrWrite) {
    case PXE_IO_READ:
        snp->IoFncs->Io.Read (
                          snp->IoFncs,
                          Width,
                          IoBar,       // IO base address
                          Address,
                          1,       // count
                          (VOID *)(UINTN)BufferAddr
                          );
        break;
    case PXE_IO_WRITE:
        snp->IoFncs->Io.Write (
                          snp->IoFncs,
                          Width,
                          IoBar,       // IO base address
                          Address,
                          1,       // count
                          (VOID *)(UINTN)BufferAddr
                          );
        break;
    case PXE_MEM_READ:
        snp->IoFncs->Mem.Read (
                           snp->IoFncs,
                           Width,
                           MemBar,       // Memory base address
                           Address,
                           1,       // count
                           (VOID *)(UINTN)BufferAddr
                           );
        break;
    case PXE_MEM_WRITE:
        snp->IoFncs->Mem.Write (
                           snp->IoFncs,
                           Width,
                           MemBar,       // Memory base address
                           Address,
                           1,       // count
                           (VOID *)(UINTN)BufferAddr
                           );
        break;
  }
  return;
}


VOID
snp_undi32_callback_map (
  IN UINT64 UniqueId,
  IN UINT64 CpuAddr,
  IN UINT32 NumBytes,
  IN UINT32 Direction,
  IN OUT UINT64 DeviceAddrPtr
  )
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 UNDI call this routine when it has to map a CPU address to a device
 address.

Arguments:
 UniqueId - This was supplied to UNDI at Undi_Start, SNP uses this to store
            Undi interface context (Undi does not read or write this variable)
 CpuAddr  - Virtual address to be mapped!
 NumBytes - size of memory to be mapped
 Direction- direction of data flow for this memory's usage:
            cpu->device, device->cpu or both ways
 DeviceAddrPtr - pointer to return the mapped device address

Returns:

--*/
{
  EFI_PHYSICAL_ADDRESS          *DevAddrPtr;
  EFI_PCI_IO_PROTOCOL_OPERATION DirectionFlag;
  UINTN                         BuffSize = (UINTN) NumBytes;
  SNP_DRIVER                    *snp = (SNP_DRIVER *)(UINTN)UniqueId;
  int                           i;

  DevAddrPtr = (EFI_PHYSICAL_ADDRESS *)(UINTN)DeviceAddrPtr;

  if (CpuAddr == 0) {
    *DevAddrPtr = 0;
    return;
  }

  switch (Direction) {
    case TO_AND_FROM_DEVICE:
        DirectionFlag = EfiPciIoOperationBusMasterCommonBuffer;
        break;
    case FROM_DEVICE:
        DirectionFlag = EfiPciIoOperationBusMasterWrite;
        break;
    case TO_DEVICE:
        DirectionFlag = EfiPciIoOperationBusMasterRead;
        break;
    default:
        *DevAddrPtr = 0;
        return ;  //any non zero indicates error!
  }
  //
  // find an unused map_list entry
  //
  for (i = 0; i < MAX_MAP_LENGTH; i++) {
    if (snp->map_list[i].virt == 0)
      break;
  }
  if (i >= MAX_MAP_LENGTH) {
#if SNP_DEBUG
    Print (L"SNP maplist is FULL\n");
#endif
    *DevAddrPtr = 0;
    return;
  }
  snp->map_list[i].virt = (EFI_PHYSICAL_ADDRESS)CpuAddr;

  if (snp->IoFncs->Map (snp->IoFncs,
                        DirectionFlag,
                        (VOID *)(UINTN)CpuAddr,
                        &BuffSize,
                        DevAddrPtr,
                        &(snp->map_list[i].map_cookie)) != EFI_SUCCESS) {
    *DevAddrPtr = 0;
    snp->map_list[i].virt = 0;
  }
   return;
}

VOID
snp_undi32_callback_unmap (
  IN UINT64 UniqueId,
  IN UINT64 CpuAddr,
  IN UINT32 NumBytes,
  IN UINT32 Direction,
  IN UINT64 DeviceAddr
  )
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 UNDI call this routine when it wants to unmap an address that was previously
 mapped using map callback

Arguments:
 UniqueId - This was supplied to UNDI at Undi_Start, SNP uses this to store
            Undi interface context (Undi does not read or write this variable)
 CpuAddr  - Virtual address that was mapped!
 NumBytes - size of memory mapped
 Direction- direction of data flow for this memory's usage:
            cpu->device, device->cpu or both ways
 DeviceAddr - the mapped device address

Returns:

--*/
{
  SNP_DRIVER  *snp = (SNP_DRIVER *)(UINTN)UniqueId;
  UINT16      i;

  for (i = 0; i < MAX_MAP_LENGTH; i++) {
    if (snp->map_list[i].virt == CpuAddr)
      break;
  }
  if (i >= MAX_MAP_LENGTH) {
#if SNP_DEBUG
    Print (L"SNP could not find a mapping, failed to unmap.\n");
#endif
    return;
  }

  snp->IoFncs->Unmap (snp->IoFncs, snp->map_list[i].map_cookie);
  snp->map_list[i].virt = 0;
  snp->map_list[i].map_cookie = NULL;
  return;
}


VOID
snp_undi32_callback_sync (UINT64 UniqueId, UINT64 CpuAddr, UINT32 NumBytes,
      UINT32 Direction, UINT64 DeviceAddr)
/*++

Routine Description:
 This is a callback routine supplied to UNDI at undi_start time.
 UNDI call this routine when it wants synchronize the virtual buffer contents
 with the mapped buffer contents. The virtual and mapped buffers need not
 correspond to the same physical memory (especially if the virtual address is
 > 4GB). Depending on the direction for which the buffer is mapped, undi will
 need to synchronize their contents whenever it writes to/reads from the buffer
 using either the cpu address or the device address.

 EFI does not provide a sync call, since virt=physical, we sould just do
 the synchronization ourself here!

Arguments:
 UniqueId - This was supplied to UNDI at Undi_Start, SNP uses this to store
            Undi interface context (Undi does not read or write this variable)
 CpuAddr  - Virtual address that was mapped!
 NumBytes - size of memory mapped
 Direction- direction of data flow for this memory's usage:
            cpu->device, device->cpu or both ways
 DeviceAddr - the mapped device address

Returns:

--*/
{
  if ((CpuAddr == 0) || (DeviceAddr == 0) || (NumBytes == 0))
    return;

  switch (Direction) {
    case FROM_DEVICE:
        EfiCopyMem ((UINT8 *)(UINTN)CpuAddr, (UINT8 *)(UINTN)DeviceAddr, NumBytes);
         break;
    case TO_DEVICE:
        EfiCopyMem ((UINT8 *)(UINTN)DeviceAddr, (UINT8 *)(UINTN)CpuAddr, NumBytes);
        break;
  }
  return;
}
