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

    UsbAtapi.h
    
Abstract:

    EFI Atapi Protocol definition.

Revision History

--*/

#ifndef _EFI_USB_ATAPI_H
#define _EFI_USB_ATAPI_H

//
// Transfer protocol types
//
#define BOT		0x50
#define CBI0	0x00
#define CBI1	0x01

//
// SubClass Code (defines command set)
//
#define EFI_USB_SUBCLASS_RBC            0x01
#define EFI_USB_SUBCLASS_ATAPI          0x02
#define EFI_USB_SUBCLASS_QIC_157        0x03
#define EFI_USB_SUBCLASS_UFI            0x04
#define EFI_USB_SUBCLASS_SFF_8070i      0x05
#define EFI_USB_SUBCLASS_SCSI           0x06
#define EFI_USB_SUBCLASS_RESERVED_LOW   0x07
#define EFI_USB_SUBCLASS_RESERVED_HIGH  0xff
//
// Global GUID for transfer protocol interface
//
#define EFI_USB_ATAPI_PROTOCOL_GUID \
    { 0x2B2F68DA, 0x0CD2, 0x44cf, 0x8E, 0x8B, 0xBB, 0xA2, 0x0B, 0x1B, 0x5B, 0x75 }

EFI_INTERFACE_DECL(_EFI_USB_ATAPI_PROTOCOL);

typedef
EFI_STATUS
(EFIAPI *EFI_USB_ATAPI_PACKET_CMD) (
	IN 	struct _EFI_USB_ATAPI_PROTOCOL	*This,
	IN  VOID							              *Command,
	IN  UINT8							              CommandSize,
	IN  VOID							              *DataBuffer,
	IN  UINT32							            BufferLength,
	IN  EFI_USB_DATA_DIRECTION	        Direction,
	IN  UINT16							            TimeOutInMilliSeconds
);    

typedef
EFI_STATUS
(EFIAPI *EFI_USB_MASS_STORAGE_RESET) (
	IN 	struct _EFI_USB_ATAPI_PROTOCOL	*This,
	IN  BOOLEAN                         ExtendedVerification
);

//
//  Protocol Interface Structure
//
typedef
struct _EFI_USB_ATAPI_PROTOCOL
{
  EFI_USB_ATAPI_PACKET_CMD        UsbAtapiPacketCmd;
	EFI_USB_MASS_STORAGE_RESET      UsbAtapiReset;
	UINT32                          CommandProtocol;
} EFI_USB_ATAPI_PROTOCOL;

extern EFI_GUID gEfiUsbAtapiProtocolGuid;

#endif
