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
    usbutil.c

  Abstract:

    Helper functions for USB

  Revision History

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#include "usb.h"
#include "usbutil.h"

//
// Following APIs are used to query Port Status
//
BOOLEAN
IsPortConnect (
  UINT16  PortStatus
)
/*++

  Routine Description:
    Tell if there is a device connected to that port according to
    the Port Status.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 0 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_CONNECTION) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortEnable (
  UINT16  PortStatus
)
/*++

  Routine Description:
    Tell if Port is enabled.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 1 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_ENABLE) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortInReset (
  UINT16  PortStatus
)
/*++

  Routine Description:
    Tell if the port is being reset.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 4 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_RESET) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortPowerApplied (
  UINT16  PortStatus
)
/*++

  Routine Description:
    Tell if there is power applied to that port.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 8 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_POWER) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortLowSpeedDeviceAttached (
  UINT16  PortStatus
)
/*++

  Routine Description:
    Tell if the connected device is a low device.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 9 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_LOW_SPEED) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortSuspend (
  UINT16  PortStatus
)
/*++

  Routine Description:
    Tell if the port is suspend.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 2 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_SUSPEND) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}


//
// Following APIs are used to query Port Change Status
//
BOOLEAN
IsPortConnectChange (
  UINT16  PortChangeStatus
)
/*++

  Routine Description:
    Tell if there is a Connect Change status in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 0 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_CONNECTION) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortEnableDisableChange (
  UINT16  PortChangeStatus
)
/*++

  Routine Description:
    Tell if there is a Enable/Disable change in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 1 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_ENABLE) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortResetChange (
  UINT16  PortChangeStatus
)
/*++

  Routine Description:
    Tell if there is a Port Reset Change status in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 4 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_RESET) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}


BOOLEAN
IsPortSuspendChange (
  UINT16  PortChangeStatus
)
/*++

  Routine Description:
    Tell if there is a Suspend Change Status in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
{
  //
  // return the bit 2 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_SUSPEND) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}
