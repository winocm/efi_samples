#ifndef _EFITCP_H
#define _EFITCP_H

/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    tcp.h

Abstract:

    EFI Transmission Control Protocol



Revision History

--*/

#include EFI_PROTOCOL_DEFINITION(PxeBaseCode)

//
// PXE Base Code protocol
//

#define EFI_TCP_PROTOCOL_GUID \
    { 0x02b3d5f2, 0xac28, 0x11d3, 0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

EFI_INTERFACE_DECL(_EFI_TCP_PROTOCOL);


typedef UINT16 EFI_PXE_BASE_CODE_TCP_PORT;

//
// Port Receive Filter definitions
//
#define EFI_PXE_BASE_CODE_MAX_PORTCNT             8
typedef struct {
    UINT8                       Filters;
    UINT8                       IpCnt;
    UINT16                      reserved;
    EFI_IP_ADDRESS              IpList[EFI_PXE_BASE_CODE_MAX_PORTCNT];
} EFI_TCP_PORT_FILTER;

typedef
EFI_STATUS
(EFIAPI *EFI_TCP_WRITE) (
    IN struct _EFI_PXE_BASE_CODE_PROTOCOL        *This,
    IN UINT16                                    OpFlags,
    IN UINT16                                    *UrgentPointer,
    IN UINT32                                    *SequenceNumber,
    IN UINT32                                    *AckNumber,
    IN UINT16                                    *HlenResCode,
    IN UINT16                                    *Window,
    IN EFI_IP_ADDRESS                            *DestIp,
    IN UINT16                                    *DestPort,
    IN EFI_IP_ADDRESS                            *GatewayIp,  OPTIONAL
    IN EFI_IP_ADDRESS                            *SrcIp,      OPTIONAL
    IN UINT16                                    *SrcPort,    OPTIONAL
    IN UINTN                                     *HeaderSize, OPTIONAL
    IN VOID                                      *HeaderPtr,  OPTIONAL
    IN UINTN                                     *BufferSize,
    IN VOID                                      *BufferPtr
    );

typedef
EFI_STATUS
(EFIAPI *EFI_TCP_READ) (
    IN struct _EFI_PXE_BASE_CODE_PROTOCOL        *This,
    IN UINT16                                    OpFlags,
    IN OUT EFI_IP_ADDRESS                        *DestIp,      OPTIONAL
    IN OUT UINT16                                *DestPort,    OPTIONAL
    IN OUT EFI_IP_ADDRESS                        *SrcIp,       OPTIONAL
    IN OUT UINT16                                *SrcPort,     OPTIONAL
    IN UINTN                                     *HeaderSize,  OPTIONAL
    IN VOID                                      *HeaderPtr,   OPTIONAL
    IN OUT UINTN                                 *BufferSize,
    IN VOID                                      *BufferPtr
    );

typedef
EFI_STATUS
(EFIAPI *EFI_TCP_SET_PORT_FILTER) (
    IN struct _EFI_PXE_BASE_CODE_PROTOCOL    *This,
    IN EFI_TCP_PORT_FILTER                   *NewFilter
    );

//
// TCP Protocol structure
//
typedef struct _EFI_TCP_PROTOCOL {
    EFI_TCP_WRITE             TcpWrite;
    EFI_TCP_READ              TcpRead;
    EFI_TCP_SET_PORT_FILTER   SetPortFilter;
} EFI_TCP_PROTOCOL;

extern EFI_GUID gEfiTcpProtocolGuid;

#endif /* _EFITCP_H */
