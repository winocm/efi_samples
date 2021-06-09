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

  Event.c

Abstract:

  Support for Event lib fucntions.

--*/

#include "Efi.h"
#include "EfiDriverLib.h"


EFI_EVENT
EfiLibCreateProtocolNotifyEvent (
  IN EFI_GUID             *ProtocolGuid,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                *Registration
  )
/*++

Routine Description:

  Create a protocol notification event and return it.

Arguments:

  ProtocolGuid    - Protocol to register notification event on.

  NotifTpl        - Maximum TPL to single the NotifyFunction.

  NotifyFunciton  - EFI notification routine.

  NotifyContext   - Context passed into Event when it is created.

  Registration    - Registration key returned from RegisterProtocolNotify().

Returns: 

  The EFI_EVENT that has been registered to be signaled when a ProtocolGuid 
  is added to the system.

--*/
{
  EFI_STATUS              Status;
  EFI_EVENT               Event;

  //
  // Create the event
  //

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  NotifyContext,
                  &Event
                  );
  ASSERT (!EFI_ERROR(Status));

  //
  // Register for protocol notifactions on this event
  //

  Status = gBS->RegisterProtocolNotify (
                  ProtocolGuid, 
                  Event, 
                  Registration
                  );

  ASSERT (!EFI_ERROR(Status));

  //
  // Kick the event so we will perform an initial pass of
  // current installed drivers
  //

  gBS->SignalEvent (Event);
  return Event;
}
