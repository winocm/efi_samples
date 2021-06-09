#ifndef _PXE_H
#define _PXE_H

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

    pxe.h

Abstract:

    PXE 2.1 Common Structures



Revision History

--*/

//
// Packet definitions
//

typedef struct {
    UINT8                           BootpOpcode;
    UINT8                           BootpHwType;
    UINT8                           BootpHwAddrLen;
    UINT8                           BootpGateHops;
    UINT32                          BootpIdent;
    UINT16                          BootpSeconds;
    UINT16                          BootpFlags;
    UINT8                           BootpCiAddr[4];
    UINT8                           BootpYiAddr[4];
    UINT8                           BootpSiAddr[4];
    UINT8                           BootpGiAddr[4];
    UINT8                           BootpHwAddr[16];
    UINT8                           BootpSrvName[64];
    UINT8                           BootpBootFile[128];
    UINT32                          DhcpMagik;
    UINT8                           DhcpOptions[56];
} EFI_PXE_BASE_CODE_DHCPV4_PACKET;

// TBD in EFI v1.1
//typedef struct {
//    UINT8                           reserved;
//} EFI_PXE_BASE_CODE_DHCPV6_PACKET;

typedef union {
    UINT8                               Raw[1472];
    EFI_PXE_BASE_CODE_DHCPV4_PACKET     Dhcpv4;
//    EFI_PXE_BASE_CODE_DHCPV6_PACKET     Dhcpv6;
} EFI_PXE_BASE_CODE_PACKET;

#endif /* _PXE_H */
