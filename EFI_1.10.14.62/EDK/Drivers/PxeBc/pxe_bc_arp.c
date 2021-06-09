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
  pxe_bc_arp.c

Abstract:

--*/

#include "bc.h"

//
// Definitions for ARP
// Per RFC 826
//
STATIC ARP_HEADER ArpHeader;

#pragma pack(1)
STATIC struct {
  UINT8       MediaHeader[14];
  ARP_HEADER  ArpHeader;
  UINT8       ArpData[64];
} ArpReplyPacket;
#pragma pack()

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

VOID
InitArpHeader(VOID)
/*++
Routine description:
  Initialize ARP packet header.

Parameters:
  none

Returns:
  none

--*/
{
  ArpHeader.HwType     = HTONS(ETHERNET_ADD_SPC);
  ArpHeader.ProtType   = HTONS(ETHER_TYPE_IP);
  ArpHeader.HwAddLen   = ENET_HWADDLEN;
  ArpHeader.ProtAddLen = IPV4_PROTADDLEN;
  ArpHeader.OpCode     = HTONS(ARP_REQUEST);

  EfiCopyMem(&ArpReplyPacket.ArpHeader, &ArpHeader, sizeof(ARP_HEADER));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

VOID
HandleArpReceive(
  IN PXE_BASECODE_DEVICE  *Private,
  IN ARP_PACKET           *ArpPacketPtr,
  IN VOID                 *MediaHeader)
/*++
Routine description:
  Process ARP packet.

Parameters:
  Private := Pointer to PxeBc interface
  ArpPacketPtr := Pointer to ARP packet
  MediaHeader := Pointer to media header.
Returns:
--*/
{
  EFI_PXE_BASE_CODE_MODE  *PxeBcMode;
  EFI_SIMPLE_NETWORK_MODE *SnpMode;
  EFI_MAC_ADDRESS         TmpMacAddr;
  UINTN                   Index;
  UINT8                   *SrcHwAddr;
  UINT8                   *SrcPrAddr;
  UINT8                   *DstHwAddr;
  UINT8                   *DstPrAddr;
  UINT8                   *TmpPtr;

  //
  //
  //
  PxeBcMode = Private->EfiBc.Mode;
  SnpMode = Private->SimpleNetwork->Mode;

  //
  // For now only ethernet addresses are supported.
  // This will need to be updated when other media
  // layers are supported by PxeBc, Snp and UNDI.
  //
  if (ArpPacketPtr->ArpHeader.HwType != HTONS(ETHERNET_ADD_SPC)) {
    return;
  }

  //
  // For now only IP protocol addresses are supported.
  // This will need to be updated when other protocol
  // types are supported by PxeBc, Snp and UNDI.
  //
  if (ArpPacketPtr->ArpHeader.ProtType != HTONS(ETHER_TYPE_IP)) {
    return;
  }

  //
  // For now only SNP hardware address sizes are supported.
  //
  if (ArpPacketPtr->ArpHeader.HwAddLen != SnpMode->HwAddressSize) {
    return;
  }

  //
  // For now only PxeBc protocol address sizes are supported.
  //
  if (ArpPacketPtr->ArpHeader.ProtAddLen != Private->IpLength) {
    return;
  }

  //
  // Ignore out of range opcodes
  //
  switch (ArpPacketPtr->ArpHeader.OpCode) {
  case HTONS(ARP_REPLY):
  case HTONS(ARP_REQUEST):
    break;

  default:
    return;
  }

  //
  // update entry in our ARP cache if we have it
  //
  SrcHwAddr = (UINT8 *)&ArpPacketPtr->SrcHardwareAddr;
  SrcPrAddr = SrcHwAddr + SnpMode->HwAddressSize;

  for (Index = 0; Index < PxeBcMode->ArpCacheEntries; ++Index) {
    if (EfiCompareMem(&PxeBcMode->ArpCache[Index].IpAddr,
      SrcPrAddr, Private->IpLength))
    {
      continue;
    }

    EfiCopyMem(&PxeBcMode->ArpCache[Index].MacAddr,
      SrcHwAddr, SnpMode->HwAddressSize);

    break;
  }

  //
  // Done if ARP packet was not for us.
  //
  DstHwAddr = SrcPrAddr + Private->IpLength;
  DstPrAddr = DstHwAddr + SnpMode->HwAddressSize;

  if (EfiCompareMem(DstPrAddr, &PxeBcMode->StationIp, Private->IpLength)) {
    return;   // not for us
  }

  //
  // for us - if we did not update entry, add it
  //
  if (Index == PxeBcMode->ArpCacheEntries) {
    //
    // if we have a full table, get rid of oldest
    //
    if (Index == PXE_ARP_CACHE_SIZE) {
      Index = Private->OldestArpEntry;

      if (++Private->OldestArpEntry == PXE_ARP_CACHE_SIZE) {
        Private->OldestArpEntry = 0;
      }
    } else {
      ++PxeBcMode->ArpCacheEntries;
    }

    EfiCopyMem(&PxeBcMode->ArpCache[Index].MacAddr,
      SrcHwAddr,
      SnpMode->HwAddressSize);

    EfiCopyMem(&PxeBcMode->ArpCache[Index].IpAddr,
      SrcPrAddr,
      Private->IpLength);
  }

  //
  // if this is not a request or we don't yet have an IP, finished
  //
  if (ArpPacketPtr->ArpHeader.OpCode != HTONS(ARP_REQUEST) ||
    !Private->GoodStationIp)
  {
    return;
  }

  //
  // Assemble ARP reply.
  //

  //
  // Create media header.  [ dest mac | src mac | prot ]
  //
  EfiCopyMem(
    &ArpReplyPacket.MediaHeader[0],
    SrcHwAddr,
    SnpMode->HwAddressSize);

  EfiCopyMem(
    &ArpReplyPacket.MediaHeader[SnpMode->HwAddressSize],
    &SnpMode->CurrentAddress,
    SnpMode->HwAddressSize);

  EfiCopyMem(
    &ArpReplyPacket.MediaHeader[2 * SnpMode->HwAddressSize],
    &((UINT8 *)MediaHeader)[2 * SnpMode->HwAddressSize],
    sizeof(UINT16));

  //
  // ARP reply header is almost filled in,
  // just insert the correct opcode.
  //
  ArpReplyPacket.ArpHeader.OpCode = HTONS(ARP_REPLY);

  //
  // Now fill in ARP data.  [ src mac | src prot | dest mac | dest prot ]
  //
  TmpPtr = ArpReplyPacket.ArpData;
  EfiCopyMem(TmpPtr, &SnpMode->CurrentAddress, SnpMode->HwAddressSize);

  TmpPtr += SnpMode->HwAddressSize;
  EfiCopyMem(TmpPtr, &PxeBcMode->StationIp, Private->IpLength);

  TmpPtr += Private->IpLength;
  EfiCopyMem(TmpPtr, SrcHwAddr, SnpMode->HwAddressSize);

  TmpPtr += SnpMode->HwAddressSize;
  EfiCopyMem(TmpPtr, SrcPrAddr, Private->IpLength);

  //
  // Now send out the ARP reply.
  //
  EfiCopyMem(&TmpMacAddr, SrcHwAddr, sizeof(EFI_MAC_ADDRESS));

  SendPacket(
    Private,
    &ArpReplyPacket.MediaHeader,
    &ArpReplyPacket.ArpHeader,
    sizeof(ARP_HEADER) + 2 * (Private->IpLength + SnpMode->HwAddressSize),
    &TmpMacAddr,
    PXE_PROTOCOL_ETHERNET_ARP,
    EFI_PXE_BASE_CODE_FUNCTION_ARP);

  //
  // Give time (100 microseconds) for ARP reply to get onto wire.
  //
  gBS->Stall(1000);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

BOOLEAN
GetHwAddr(
  IN PXE_BASECODE_DEVICE  *Private,
  IN EFI_IP_ADDRESS       *ProtocolAddrPtr,
  OUT EFI_MAC_ADDRESS     *HardwareAddrPtr)
/*++
Routine description:
  Locate IP address in ARP cache and return MAC address.

Parameters:
  Private := Pointer to PxeBc interface
  ProtocolAddrPtr := Pointer to IP address
  HardwareAddrPtr := Pointer to MAC address storage

Returns:
  TRUE := If IP address was found and MAC address was stored
  FALSE := If IP address was not found
--*/
{
  EFI_PXE_BASE_CODE_MODE  *PxeBcMode;
  UINTN                   HardwareAddrLength;
  UINTN                   i;

  PxeBcMode = Private->EfiBc.Mode;
  HardwareAddrLength = Private->SimpleNetwork->Mode->HwAddressSize;

  for (i = 0; i < PxeBcMode->ArpCacheEntries; ++i) {
    if (!EfiCompareMem(ProtocolAddrPtr, &PxeBcMode->ArpCache[i].IpAddr,
      Private->IpLength))
    {
      EfiCopyMem(HardwareAddrPtr, &PxeBcMode->ArpCache[i].MacAddr,
        HardwareAddrLength);

      return TRUE;
    }
  }

  return FALSE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

STATIC EFI_STATUS
SendRequest(
  IN PXE_BASECODE_DEVICE  *Private,
  IN EFI_IP_ADDRESS       *ProtocolAddrPtr,
  IN EFI_MAC_ADDRESS      *HardwareAddrPtr)
/*++
Routine description:
  Transmit ARP request packet

Parameters:
  Private := Pointer to PxeBc interface
  ProtocolAddrPtr := Pointer IP address to find
  HardwareAddrPtr := Pointer to MAC address to find

Returns:
  EFI_SUCCESS := ARP request sent
  other := ARP request could not be sent
--*/
{
  EFI_PXE_BASE_CODE_MODE  *PxeBcMode;
  EFI_SIMPLE_NETWORK_MODE *SnpMode;
  ARP_PACKET              *ArpPacket;
  EFI_STATUS              Status;
  UINTN                   HardwareAddrLength;
  UINT8                   *SrcProtocolAddrPtr;
  UINT8                   *DestHardwareAddrptr;
  UINT8                   *DestProtocolAddrPtr;

  //
  //
  //
  PxeBcMode = Private->EfiBc.Mode;
  SnpMode = Private->SimpleNetwork->Mode;
  HardwareAddrLength = SnpMode->HwAddressSize;

  //
  // Allocate ARP buffer
  //
  if (Private->ArpBuffer == NULL) {
    Status = gBS->AllocatePool(
      EfiBootServicesData,
      SnpMode->MediaHeaderSize + sizeof(ARP_PACKET),
      &Private->ArpBuffer);

    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  ArpPacket = (VOID *)(Private->ArpBuffer + SnpMode->MediaHeaderSize);

  // for now, only handle one kind of hw and pr address
  ArpPacket->ArpHeader = ArpHeader;
  ArpPacket->ArpHeader.HwAddLen = (UINT8)HardwareAddrLength;
  ArpPacket->ArpHeader.ProtAddLen = (UINT8)Private->IpLength;

  // rest more generic
  SrcProtocolAddrPtr = (UINT8 *)(&ArpPacket->SrcHardwareAddr) +
    HardwareAddrLength;
  DestHardwareAddrptr = SrcProtocolAddrPtr + Private->IpLength;
  DestProtocolAddrPtr = DestHardwareAddrptr + HardwareAddrLength;

  EfiCopyMem(DestProtocolAddrPtr, ProtocolAddrPtr, Private->IpLength);
  EfiCopyMem(DestHardwareAddrptr, HardwareAddrPtr, HardwareAddrLength);
  EfiCopyMem(SrcProtocolAddrPtr, &PxeBcMode->StationIp, Private->IpLength);
  EfiCopyMem(&ArpPacket->SrcHardwareAddr, &SnpMode->CurrentAddress,
    HardwareAddrLength);

  return SendPacket(Private, Private->ArpBuffer, ArpPacket,
    sizeof(ARP_HEADER) + ((Private->IpLength + HardwareAddrLength) << 1), 
    &SnpMode->BroadcastAddress,
    PXE_PROTOCOL_ETHERNET_ARP, EFI_PXE_BASE_CODE_FUNCTION_ARP);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//
// check for address - if not there, send ARP request, wait and check again
// not how it would be done in a full system
//

#define ARP_REQUEST_TIMEOUT_MS 500  // try for half a second

////////////////////////////////////////////////////////////
//
//  BC Arp Routine
//

EFI_STATUS EFIAPI
BcArp(
  IN EFI_PXE_BASE_CODE_PROTOCOL *This,
  IN EFI_IP_ADDRESS             *ProtocolAddrPtr,
  OUT EFI_MAC_ADDRESS           *HardwareAddrPtr OPTIONAL
)
/*++
Routine description:
  PxeBc ARP API.

Parameters:
  This := Pointer to PxeBc interface
  ProtocolAddrPtr := Pointer to IP address to find
  HardwareAddrPtr := Pointer to MAC address found.

Returns:
--*/
{
  EFI_MAC_ADDRESS     Mac;
  EFI_STATUS          StatCode;
  PXE_BASECODE_DEVICE *Private;

  //
  // Lock the instance data and make sure started
  //

  StatCode = EFI_SUCCESS;

  if (This == NULL) {
    DEBUG((EFI_D_ERROR, "BC *This pointer == NULL"));
    return EFI_INVALID_PARAMETER;
  }

  Private = CR(This, PXE_BASECODE_DEVICE, EfiBc, PXE_BASECODE_DEVICE_SIGNATURE);

  if (Private == NULL) {
    DEBUG((EFI_D_ERROR, "PXE_BASECODE_DEVICE poiner == NULL"));
    return EFI_INVALID_PARAMETER;
  }

  EfiAcquireLock(&Private->Lock);

  if (This->Mode == NULL || !This->Mode->Started) {
    DEBUG((EFI_D_ERROR, "BC was not started."));
    EfiReleaseLock(&Private->Lock);
    return EFI_NOT_STARTED;
  }

  DEBUG((EFI_D_INFO, "\nBcArp()"));

  //
  // Issue BC command
  //

  if (ProtocolAddrPtr == NULL) {
    DEBUG((EFI_D_INFO, "\nBcArp()  Exit #1  %Xh (%r)",
      EFI_INVALID_PARAMETER, EFI_INVALID_PARAMETER));

    EfiReleaseLock(&Private->Lock);
    return EFI_INVALID_PARAMETER;
  }

  if (HardwareAddrPtr == NULL) {
    HardwareAddrPtr = &Mac;
  }

  EfiZeroMem(HardwareAddrPtr, Private->SimpleNetwork->Mode->HwAddressSize);

  if (GetHwAddr(Private, ProtocolAddrPtr, HardwareAddrPtr)) {
    DEBUG((EFI_D_INFO, "\nBcArp()  Exit #2  %Xh (%r)",
      EFI_SUCCESS, EFI_SUCCESS));

    EfiReleaseLock(&Private->Lock);
    return EFI_SUCCESS;
  }

  StatCode = DoArp(Private, ProtocolAddrPtr, HardwareAddrPtr);

  DEBUG((EFI_D_INFO, "\nBcArp()  Exit #3  %Xh (%r)", StatCode, StatCode));

  EfiReleaseLock(&Private->Lock);
  return StatCode;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

EFI_STATUS 
DoArp(
  IN PXE_BASECODE_DEVICE  *Private,
  IN EFI_IP_ADDRESS       *ProtocolAddrPtr,
  OUT EFI_MAC_ADDRESS     *HardwareAddrPtr
)
/*++
Routine description:
  Internal ARP implementation.

Parameters:
  Private := Pointer to PxeBc interface
  ProtocolAddrPtr := Pointer to IP address to find
  HardwareAddrPtr := Pointer to MAC address found

Returns:
  EFI_SUCCESS := MAC address found
  other := MAC address could not be found
--*/
{
  EFI_STATUS  StatCode;
  EFI_EVENT   TimeoutEvent;
  UINTN       HeaderSize;
  UINTN       BufferSize;
  UINT16      Protocol;

  DEBUG((EFI_D_INFO, "\nDoArp()"));

  //
  //
  //
  StatCode = SendRequest(Private, ProtocolAddrPtr, HardwareAddrPtr);

  if (EFI_ERROR(StatCode)) {
    DEBUG((EFI_D_INFO, "\nDoArp()  Exit #1  %Xh (%r)", StatCode, StatCode));
    return StatCode;
  }

  //
  //
  //
  StatCode = gBS->CreateEvent(
    EFI_EVENT_TIMER,
    EFI_TPL_CALLBACK,
    NULL,
    NULL,
    &TimeoutEvent);

  if (EFI_ERROR(StatCode)) {
    return StatCode;
  }

  StatCode = gBS->SetTimer(
    TimeoutEvent,
    TimerRelative,
    ARP_REQUEST_TIMEOUT_MS * 10000);

  if (EFI_ERROR(StatCode)) {
    gBS->CloseEvent(TimeoutEvent);
    return StatCode;
  }

  //
  //
  //
  for (;;) {
    StatCode = WaitForReceive(Private, EFI_PXE_BASE_CODE_FUNCTION_ARP,
      TimeoutEvent, &HeaderSize, &BufferSize, &Protocol);

    if (EFI_ERROR(StatCode)) {
      break;
    }

    if (Protocol != PXE_PROTOCOL_ETHERNET_ARP) {
      continue;
    }

    HandleArpReceive(Private,
      (ARP_PACKET *)(Private->ReceiveBufferPtr+HeaderSize),
      Private->ReceiveBufferPtr);

    if (GetHwAddr(Private, ProtocolAddrPtr, HardwareAddrPtr)) {
      break;
    }
  }

  DEBUG((EFI_D_INFO, "\nDoArp()  Exit #2  %Xh, (%r)",
    StatCode, StatCode));

  gBS->CloseEvent(TimeoutEvent);

  return StatCode;
}

/* eof - pxe_bc_arp.c */
