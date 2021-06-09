#ifndef _TIME_H
#define _TIME_H
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

    time.h

Abstract: 

    Efi runtime time functions


Revision History

--*/

STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlGetTime (
    IN EFI_TIME                     *Time,
    IN EFI_TIME_CAPABILITIES        *Capabilities OPTIONAL
    );

STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlSetWakeupTime (
    IN BOOLEAN                      Enable,
    IN EFI_TIME                     *Time
    );

EFI_STATUS
RUNTIMESERVICE
RtPlGetWakeupTime (
    OUT BOOLEAN                     *Enabled,
    OUT BOOLEAN                     *Pending,
    OUT EFI_TIME                    *Time
    );

STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlSetTime (
    IN EFI_TIME     *Time
    );

#endif