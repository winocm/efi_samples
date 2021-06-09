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
    usbutil.h
  
  Abstract:
 
    Helper functions for USB
 
  Revision History
 
  
--*/
 
#ifndef _USB_UTIL_H
#define _USB_UTIL_H

//
// Following APIs are used to query Port Status
//
BOOLEAN
IsPortConnect (
  UINT16  PortStatus
);

BOOLEAN
IsPortEnable (
  UINT16  PortStatus
);

BOOLEAN
IsPortInReset (
  UINT16  PortStatus
);

BOOLEAN
IsPortPowerApplied (
  UINT16  PortStatus
);

BOOLEAN
IsPortLowSpeedDeviceAttached (
  UINT16  PortStatus
);

BOOLEAN
IsPortSuspend (
  UINT16  PortStatus
);

//
// Following APIs are used to query Port Change Status
//
BOOLEAN
IsPortConnectChange (
  UINT16  PortChangeStatus
);

BOOLEAN
IsPortEnableDisableChange (
  UINT16  PortChangeStatus
);

BOOLEAN
IsPortResetChange (
  UINT16  PortChangeStatus
);

BOOLEAN
IsPortSuspendChange (
  UINT16  PortChangeStatus
);

#endif
