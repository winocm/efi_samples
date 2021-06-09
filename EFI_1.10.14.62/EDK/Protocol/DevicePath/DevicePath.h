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

  DevicePath.h

Abstract:

  The device path protocol as defined in EFI 1.0.

  The device path represents a programatic path to a device. It's the view
  from a software point of view. It also must persist from boot to boot, so 
  it can not contain things like PCI bus numbers that change from boot to boot.
  

--*/

#ifndef _DEVICE_PATH_H_
#define _DEVICE_PATH_H_

//
// Device Path protocol
//
#define EFI_DEVICE_PATH_PROTOCOL_GUID  \
  { 0x9576e91, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

typedef struct {
  UINT8  Type;
  UINT8  SubType;
  UINT8  Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

#define EFI_END_ENTIRE_DEVICE_PATH          0xff
#define EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE  0xff
#define EFI_END_INSTANCE_DEVICE_PATH        0x01
#define EFI_END_DEVICE_PATH_LENGTH          (sizeof(EFI_DEVICE_PATH_PROTOCOL))

#define EfiDevicePathNodeLength(a)  (((a)->Length[0]) | ((a)->Length[1] << 8))
#define EfiNextDevicePathNode(a)    ((EFI_DEVICE_PATH_PROTOCOL *) ( ((UINT8 *) (a)) + \
                                      EfiDevicePathNodeLength(a)))

//
// BugBug: Support the hack of unpacked device paths
//
#define EfiDevicePathType(a)          ( ((a)->Type) & 0x7f )
#define EfiIsDevicePathEndType(a)     (EfiDevicePathType(a) == 0x7f)


#define EfiIsDevicePathEndSubType(a)          ((a)->SubType == EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE) 
#define EfiIsDevicePathEndInstanceSubType(a)  ((a)->SubType == EFI_END_INSTANCE_DEVICE_PATH) 

#define EfiIsDevicePathEnd(a)         ( EfiIsDevicePathEndType(a) && \
                                        EfiIsDevicePathEndSubType(a) )
#define EfiIsDevicePathEndInstance(a) ( EfiIsDevicePathEndType(a) && \
                                        EfiIsDevicePathEndInstanceSubType(a) )

extern EFI_GUID gEfiDevicePathProtocolGuid;

#endif
