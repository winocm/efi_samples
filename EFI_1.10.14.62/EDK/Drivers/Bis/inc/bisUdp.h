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

    bisUdp.h

Abstract:


Revision History

--*/
//
// User Datagram Protocol Used by BisRemote
//
#ifndef BISUDP_H
#define BISUDP_H

//remove this line if we get incorporated into efi.h
#include <efi.h>

#include <efibis.h>
#include <efibistypes.h>
#include <bpctypes.h>





	//-----------------------------------//
	//  BISUDP_PROTOCOL
	//-----------------------------------//

#define BISUDP_PROTOCOL      \
{ 0x665f8790, 0x67dd, 0x11d4, 0x98, 0x16, 0x0, 0xa0, 0xc9, 0x1f, 0xad, 0xcf };

INTERFACE_DECL(BISUDP_INTERFACE);
typedef struct _BISUDP_INTERFACE BISUDP_INTERFACE;




typedef
BPC_HANDLE
	(EFIAPI *UDP_CONNECT)(
	  BPC_CONNECT_PARMS *cParms,
	  UINT32 *errcode );


typedef
UINT32
 	(EFIAPI *UDP_DISCONNECT)(
	  BPC_HANDLE hConn);

typedef
BPC_STATUS
	(EFIAPI *UDP_RECVBUFFER)(
      BPC_HANDLE    targetSystem,
      void         *buffer,
      UINT16        bufferLength,
      UINT16       *readCount);


typedef
BPC_STATUS
	(EFIAPI *UDP_SENDBUFFER)(
      BPC_HANDLE  targetSystem,
      void        *buffer,
      UINT16      bufferLength );


typedef
BPC_STATUS
	(EFIAPI *UDP_IOCTL)(
      BPC_HANDLE     targetSystem,
      UINT16         controlCode,      //[in] transport specific function code.
      void           *buffer,          //[in/out]
      UINT16         *bufferLength     //[in/out]
    );

typedef
UINT16                          //status;
	(EFIAPI *UDP_SETREADFILTER)(
      BPC_HANDLE        targetSystem,
      EFI_IPv4_ADDRESS  src_ip,   // IN - only return pkts from this addrs.
      UINT16            s_port    // IN - only return pkts from this socket.
    );


typedef struct 	_BISUDP_INTERFACE
{
	//methods
	UDP_CONNECT		  Connect;
	UDP_DISCONNECT	  Disconnect;
	UDP_RECVBUFFER	  RecvBuffer;
	UDP_SENDBUFFER	  SendBuffer;
	UDP_IOCTL		  IoCtl;
	UDP_SETREADFILTER SetReadFilter;

	//member vars
	VOID			 *InstanceData;
	EFI_HANDLE		 DeviceHandle;

}
BISUDP_INTERFACE;


#endif
