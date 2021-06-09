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
  pxe_bc_udp.c

Abstract:

--*/
#include "bc.h"


////////////////////////////////////////////////////////////////////////
//
//  Udp Write Routine - called by base code - e.g. TFTP - already locked
//

EFI_STATUS
UdpWrite(
  IN PXE_BASECODE_DEVICE            *Private,
  IN UINT16                         OpFlags,
  IN EFI_IP_ADDRESS                 *DestIpPtr,
  IN EFI_PXE_BASE_CODE_UDP_PORT     *DestPortPtr,
  IN EFI_IP_ADDRESS                 *GatewayIpPtr,  OPTIONAL
  IN EFI_IP_ADDRESS                 *SrcIpPtr,      OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_UDP_PORT *SrcPortPtr,    OPTIONAL
  IN UINTN                          *HeaderSizePtr, OPTIONAL
  IN VOID                           *HeaderPtr,     OPTIONAL
  IN UINTN                          *BufferSizeptr,
  IN VOID                           *BufferPtr)
/*++
Routine description:
  UDP write packet.

Parameters:
  Private := Pointer to PxeBc interface
  OpFlags := 
  DestIpPtr := 
  DestPortPtr := 
  GatewayIpPtr := 
  SrcIpPtr := 
  SrcPortPtr := 
  HeaderSizePtr := 
  HeaderPtr := 
  BufferSizeptr := 
  BufferPtr := 

Returns:
  EFI_SUCCESS := 
  EFI_INVALID_PARAMETER := 
  other := 
--*/
{
  UINTN                       TotalLength;
  UINTN                       HeaderSize;
  EFI_PXE_BASE_CODE_UDP_PORT  DefaultSrcPort;

  //
  //
  //
  HeaderSize = (HeaderSizePtr != NULL) ? *HeaderSizePtr : 0;
  DefaultSrcPort = 0;

  // check parameters
  if (BufferSizeptr == NULL || BufferPtr == NULL || DestIpPtr == NULL ||
    DestPortPtr == NULL || (HeaderSize != 0 && HeaderPtr == NULL) ||
    (OpFlags & ~(EFI_PXE_BASE_CODE_UDP_OPFLAGS_MAY_FRAGMENT |
      EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT)))
  {
    DEBUG((EFI_D_WARN, "\nUdpWrite()  Exit #1  %xh (%r)",
      EFI_INVALID_PARAMETER, EFI_INVALID_PARAMETER));

    return EFI_INVALID_PARAMETER;
  }

  TotalLength = *BufferSizeptr + HeaderSize + sizeof(UDPV4_HEADER);

  if (TotalLength > 0x0000ffff) {
    DEBUG((EFI_D_WARN, "\nUdpWrite()  Exit #2  %xh (%r)",
      EFI_BAD_BUFFER_SIZE, EFI_BAD_BUFFER_SIZE));

    return EFI_BAD_BUFFER_SIZE;
  }

  if (SrcIpPtr == NULL) {
    SrcIpPtr = &Private->EfiBc.Mode->StationIp;
  }

  if (SrcPortPtr == NULL) {
    SrcPortPtr = &DefaultSrcPort;
    OpFlags |= EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT;
  }

  if (OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT) {
    *SrcPortPtr = Private->RandomPort;

    if (++Private->RandomPort == 0) {
      Private->RandomPort = PXE_RND_PORT_LOW;
    }
  }

#define IpTxBuffer ((IPV4_BUFFER *)Private->TransmitBufferPtr)
  // build pseudo header and udp header in transmit buffer
#define Udpv4Base  ((UDPV4_HEADERS *)(IpTxBuffer->u.Data - sizeof(UDPV4_PSEUDO_HEADER)))

  Udpv4Base->Udpv4PseudoHeader.SrcAddr.L = SrcIpPtr->Addr[0];
  Udpv4Base->Udpv4PseudoHeader.DestAddr.L = DestIpPtr->Addr[0];
  Udpv4Base->Udpv4PseudoHeader.Zero = 0;
  Udpv4Base->Udpv4PseudoHeader.Protocol = PROT_UDP;
  Udpv4Base->Udpv4PseudoHeader.TotalLength = HTONS(TotalLength);
  Udpv4Base->Udpv4Header.SrcPort = HTONS(*SrcPortPtr);
  Udpv4Base->Udpv4Header.DestPort = HTONS(*DestPortPtr);
  Udpv4Base->Udpv4Header.TotalLength = Udpv4Base->Udpv4PseudoHeader.TotalLength;
  Udpv4Base->Udpv4Header.Checksum = 0;

  if (HeaderSize != 0) {
    EfiCopyMem(IpTxBuffer->u.Udp.Data, HeaderPtr, HeaderSize);
  }

  HeaderSize += sizeof(UDPV4_HEADER);

  Udpv4Base->Udpv4Header.Checksum = IpChecksum2((UINT16 *)Udpv4Base,
    HeaderSize+sizeof(UDPV4_PSEUDO_HEADER), 
    (UINT16 *)BufferPtr, (UINT16)*BufferSizeptr);

  if (Udpv4Base->Udpv4Header.Checksum == 0) {
    Udpv4Base->Udpv4Header.Checksum = 0xffff;   // transmit zero checksum as ones complement
  }

  return Ip4Send(Private, OpFlags, PROT_UDP,
    Udpv4Base->Udpv4PseudoHeader.SrcAddr.L,
    Udpv4Base->Udpv4PseudoHeader.DestAddr.L, 
    (GatewayIpPtr) ? GatewayIpPtr->Addr[0] : 0,
    HeaderSize, BufferPtr, *BufferSizeptr);
}

////////////////////////////////////////////////////////////
//
//  BC Udp Write Routine
//

EFI_STATUS
BcUdpWrite(
  IN EFI_PXE_BASE_CODE_PROTOCOL     *This,
  IN UINT16                         OpFlags,
  IN EFI_IP_ADDRESS                 *DestIpPtr,
  IN EFI_PXE_BASE_CODE_UDP_PORT     *DestPortPtr,
  IN EFI_IP_ADDRESS                 *GatewayIpPtr,  OPTIONAL
  IN EFI_IP_ADDRESS                 *SrcIpPtr,      OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_UDP_PORT *SrcPortPtr,    OPTIONAL
  IN UINTN                          *HeaderSizePtr, OPTIONAL
  IN VOID                           *HeaderPtr,     OPTIONAL
  IN UINTN                          *BufferSizeptr,
  IN VOID                           *BufferPtr
)
/*++
Routine description:
  UDP write API entry point.

Parameters:
  This := Pointer to PxeBc interface.
  OpFlags := 
  DestIpPtr := 
  DestPortPtr := 
  GatewayIpPtr := 
  SrcIpPtr := 
  SrcPortPtr := 
  HeaderSizePtr := 
  HeaderPtr := 
  BufferSizeptr := 
  BufferPtr := 

Returns:
  EFI_SUCCESS := 
  other := 
--*/
{
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

  Private->Function = EFI_PXE_BASE_CODE_FUNCTION_UDP_WRITE;

  //
  // Issue BC command
  //
  StatCode = UdpWrite(Private, OpFlags, DestIpPtr, DestPortPtr, GatewayIpPtr,
    SrcIpPtr, SrcPortPtr, HeaderSizePtr, HeaderPtr, BufferSizeptr,
    BufferPtr);

  //
  // Unlock the instance data
  //
  EfiReleaseLock(&Private->Lock);
  return StatCode;
}

///////////////////////////////////////////////////////////////////////
//
//  Udp Read Routine - called by base code - e.g. TFTP - already locked
//

EFI_STATUS
UdpRead(
  IN PXE_BASECODE_DEVICE            *Private,
  IN UINT16                         OpFlags,
  IN OUT EFI_IP_ADDRESS             *DestIpPtr,     OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_UDP_PORT *DestPortPtr,   OPTIONAL
  IN OUT EFI_IP_ADDRESS             *SrcIpPtr,      OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_UDP_PORT *SrcPortPtr,    OPTIONAL
  IN UINTN                          *HeaderSizePtr, OPTIONAL
  IN VOID                           *HeaderPtr,   OPTIONAL
  IN OUT UINTN                      *BufferSizeptr,
  IN VOID                           *BufferPtr,
  EFI_EVENT                         TimeoutEvent)
/*++
Routine description:
  UDP read packet.

Parameters:
  Private := Pointer to PxeBc interface
  OpFlags := 
  DestIpPtr := 
  DestPortPtr := 
  SrcIpPtr := 
  SrcPortPtr := 
  HeaderSizePtr := 
  HeaderPtr := 
  BufferSizeptr := 
  BufferPtr := 
  TimeoutEvent :=

Returns:
  EFI_SUCCESS := 
  EFI_INVALID_PARAMETER := 
  other := 
--*/
{
  EFI_STATUS      StatCode;
  EFI_IP_ADDRESS  TmpSrcIp;
  EFI_IP_ADDRESS  TmpDestIp;
  UINTN           BufferSize;
  UINTN           HeaderSize;

  // combination structure of pseudo header/udp header
#pragma pack(1)
  struct {
    UDPV4_PSEUDO_HEADER Udpv4PseudoHeader;
    UDPV4_HEADER        Udpv4Header;
    UINT8               ProtHdr[64];
  } Hdrs;
#pragma pack()

  HeaderSize = (HeaderSizePtr != NULL) ? *HeaderSizePtr : 0;

  // read [with filtering]
  // check parameters
  if (BufferSizeptr == NULL || BufferPtr == NULL ||
    (HeaderSize != 0 && HeaderPtr == NULL)
    || (OpFlags & ~UDP_FILTER_MASK)
    // if filtering on a particular IP/Port, need it
    || (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_IP) && SrcIpPtr == NULL)
    || (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT) && SrcPortPtr == NULL)
    || (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_DEST_PORT) && DestPortPtr == NULL))
  {
    DEBUG((EFI_D_INFO, "\nUdpRead()  Exit #1  Invalid Parameter"));
    return EFI_INVALID_PARAMETER;
  }

  BufferSize = *BufferSizeptr;  // in case we loop

  // we need source and dest IPs for pseudo header
  if (SrcIpPtr == NULL) {
    SrcIpPtr = &TmpSrcIp;
  }

  if (DestIpPtr == NULL) {
    DestIpPtr = &TmpDestIp;
    TmpDestIp = Private->EfiBc.Mode->StationIp;
  }

#if SUPPORT_IPV6
  if (Private->EfiBc.Mode->UsingIpv6) {
    // %%TBD
  }
#endif

  for (;;) {
    *BufferSizeptr = BufferSize;

    StatCode = IpReceive(Private, OpFlags, SrcIpPtr, DestIpPtr, PROT_UDP,
      &Hdrs.Udpv4Header, HeaderSize + sizeof Hdrs.Udpv4Header,
      BufferPtr, BufferSizeptr, TimeoutEvent);

    if (StatCode == EFI_SUCCESS || StatCode == EFI_BUFFER_TOO_SMALL) {
      UINT16 SPort;
      UINT16 DPort;

      SPort = NTOHS(Hdrs.Udpv4Header.SrcPort);
      DPort = NTOHS(Hdrs.Udpv4Header.DestPort);

      // do filtering
      if (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT) &&
        *SrcPortPtr != SPort)
      {
        continue;
      }

      if (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_DEST_PORT) &&
        *DestPortPtr != DPort)
      {
        continue;
      }

      // check checksum
      if (StatCode == EFI_SUCCESS && Hdrs.Udpv4Header.Checksum) {
        Hdrs.Udpv4PseudoHeader.SrcAddr.L = SrcIpPtr->Addr[0];
        Hdrs.Udpv4PseudoHeader.DestAddr.L = DestIpPtr->Addr[0];
        Hdrs.Udpv4PseudoHeader.Zero = 0;
        Hdrs.Udpv4PseudoHeader.Protocol = PROT_UDP;
        Hdrs.Udpv4PseudoHeader.TotalLength = Hdrs.Udpv4Header.TotalLength;

        if (Hdrs.Udpv4Header.Checksum == 0xffff) {
          Hdrs.Udpv4Header.Checksum = 0;
        }

        if (IpChecksum2((UINT16 *)&Hdrs.Udpv4PseudoHeader,
          HeaderSize + sizeof(Hdrs.Udpv4PseudoHeader) +
          sizeof(Hdrs.Udpv4Header),
          (UINT16 *)BufferPtr, *BufferSizeptr))
        {
          DEBUG((EFI_D_INFO, "\nUdpRead()  Hdrs.Udpv4PseudoHeader == %Xh",
            Hdrs.Udpv4PseudoHeader));
          DEBUG((EFI_D_INFO, "\nUdpRead()  Header size == %d",
            HeaderSize + sizeof(Hdrs.Udpv4PseudoHeader)));
          DEBUG((EFI_D_INFO, "\nUdpRead()  BufferPtr == %Xh",
            BufferPtr));
          DEBUG((EFI_D_INFO, "\nUdpRead()  Buffer size == %d",
            *BufferSizeptr));
          DEBUG((EFI_D_INFO, "\nUdpRead()  Exit #2  Device Error"));
          return EFI_DEVICE_ERROR;
        }
      }

      // all passed 
      if (SrcPortPtr != NULL) {
        *SrcPortPtr = SPort;
      }

      if (DestPortPtr != NULL) {
        *DestPortPtr = DPort;
      }

      if (HeaderSize != 0) {
        EfiCopyMem(HeaderPtr, Hdrs.ProtHdr, HeaderSize);
      }
    }

    switch (StatCode) {
    case EFI_SUCCESS:
    case EFI_TIMEOUT:
      break;

    default:
      DEBUG((EFI_D_INFO, "\nUdpRead()  Exit #3  %Xh %r",
        StatCode, StatCode));
    }

    return StatCode;
  }
}

////////////////////////////////////////////////////////////
//
//  BC Udp Read Routine
//

EFI_STATUS
BcUdpRead(
  IN EFI_PXE_BASE_CODE_PROTOCOL     *This,
  IN UINT16                         OpFlags,
  IN OUT EFI_IP_ADDRESS             *DestIp,      OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_UDP_PORT *DestPort,    OPTIONAL
  IN OUT EFI_IP_ADDRESS             *SrcIp,       OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_UDP_PORT *SrcPort,     OPTIONAL
  IN UINTN                          *HeaderSize,  OPTIONAL
  IN VOID                           *HeaderPtr,   OPTIONAL
  IN OUT UINTN                      *BufferSize,
  IN VOID                           *BufferPtr
  )
/*++
Routine description:
  UDP read API entry point.

Parameters:
  This := Pointer to PxeBc interface.
  OpFlags := 
  DestIpPtr := 
  DestPortPtr := 
  SrcIpPtr := 
  SrcPortPtr := 
  HeaderSizePtr := 
  HeaderPtr := 
  BufferSizeptr := 
  BufferPtr := 

Returns:
  EFI_SUCCESS := 
  other := 
--*/
{
  PXE_BASECODE_DEVICE *Private;
  EFI_STATUS          StatCode;
  EFI_EVENT           TimeoutEvent;

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

  Private->Function = EFI_PXE_BASE_CODE_FUNCTION_UDP_READ;

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
    EfiReleaseLock(&Private->Lock);
    return StatCode;
  }

  StatCode = gBS->SetTimer(
    TimeoutEvent,
    TimerRelative,
    10000);

  if (EFI_ERROR(StatCode)) {
    gBS->CloseEvent(TimeoutEvent);
    EfiReleaseLock(&Private->Lock);
    return StatCode;
  }

  //
  // Issue BC command
  //
  StatCode = UdpRead(Private, OpFlags, DestIp, DestPort, SrcIp, SrcPort,
    HeaderSize, HeaderPtr, BufferSize, BufferPtr, TimeoutEvent);

  gBS->CloseEvent(TimeoutEvent);

  //
  // Unlock the instance data and return
  //
  EfiReleaseLock(&Private->Lock);
  return StatCode;
}

/* eof - pxe_bc_udp.c */
