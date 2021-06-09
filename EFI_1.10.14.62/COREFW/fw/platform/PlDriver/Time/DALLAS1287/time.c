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

    time.c

Abstract: 

    Efi Runtime Time functions


Revision History

--*/
 
#include "efi.h"
#include "efilib.h"
#include "pltime.h"
#include "plstall.h"
#include "rtc.h"
#include "PlDefio.h"

//
// Declare constants
//

#define EFI_RTC_NAME    L"RTC"

#define EFI_RTC_GUID \
    { 0xe4329f89, 0x546b, 0x11d4, 0x9a, 0x39, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

//
// Declare global variables
//

#pragma BEGIN_RUNTIME_DATA()
static EFI_GUID EfiRtcGuid = EFI_RTC_GUID;
static UINT8 SavedAddressRegister = 0;
static FLOCK RtcLock = { TPL_HIGH_LEVEL, 0, 0 };
static UINT16 SavedTimeZone = EFI_UNSPECIFIED_TIMEZONE;
static UINT8 SavedDaylight = 0;

//
// Declare runtime worker functions
//

EFI_STATUS
RUNTIMEFUNCTION
RtRtcTimeFieldsValid (
    IN EFI_TIME *Time
    );

EFI_STATUS
RUNTIMEFUNCTION
RtRtcWaitToUpdate (
    UINTN Timeout
    );

VOID
RUNTIMEFUNCTION
RtRTC_outp (
    IN  UINT16 Address,
    IN  UINT8 Data
    );

UINT8
RUNTIMEFUNCTION
RtRTC_inp (
    IN  UINT16 Address
    );

VOID
RUNTIMEFUNCTION
RtRtcSaveContext (
    VOID
    );

VOID
RUNTIMEFUNCTION
RtRtcRestoreContext (
    VOID
    );

VOID
RUNTIMEFUNCTION
RtConvertRtcTimeToEfiTime (
    IN EFI_TIME       *Time,
    IN RTC_REGISTER_B RegisterB
    );

VOID
RUNTIMEFUNCTION
RtConvertEfiTimeToRtcTime (
    IN EFI_TIME       *Time,
    IN RTC_REGISTER_B RegisterB,
    IN UINT8          *Century
    );

EFI_STATUS
RUNTIMEFUNCTION
RtRtcTestCenturyRegister (
    VOID
    );

    
//
// Declare runtime services
//

#pragma RUNTIME_CODE(RtPlSetTime)
STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlSetTime (
    IN EFI_TIME     *Time
    )
{
    EFI_STATUS            Status;
    EFI_TIME              RtcTime;
    RTC_REGISTER_B        RegisterB;
    UINT8                 Century;

    //
    // Make sure that the time fields are valid
    //

    Status = RtRtcTimeFieldsValid (Time);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    RtCopyMem (&RtcTime, Time, sizeof(EFI_TIME));

    //
    // Save the RTC's Context
    //

    RtRtcSaveContext();

    //
    // Wait for up to 0.1 seconds for the RTC to be updated
    //

    Status = RtRtcWaitToUpdate (100000);
    if (EFI_ERROR(Status)) {
        RtRtcRestoreContext();
        return Status;
    }

    //
    // Read Register B, and inhibit updates of the RTC
    //

    RegisterB.Data = RtRTC_inp (RTC_ADDRESS_REGISTER_B);
    RegisterB.Bits.SET = 1;
    RtRTC_outp (RTC_ADDRESS_REGISTER_B, RegisterB.Data);

    RtConvertEfiTimeToRtcTime (&RtcTime, RegisterB, &Century);

    RtRTC_outp (RTC_ADDRESS_SECONDS,          RtcTime.Second);
    RtRTC_outp (RTC_ADDRESS_MINUTES,          RtcTime.Minute);
    RtRTC_outp (RTC_ADDRESS_HOURS,            RtcTime.Hour);
    RtRTC_outp (RTC_ADDRESS_DAY_OF_THE_MONTH, RtcTime.Day);
    RtRTC_outp (RTC_ADDRESS_MONTH,            RtcTime.Month);
    RtRTC_outp (RTC_ADDRESS_YEAR,             (UINT8)RtcTime.Year);
    if (RtRtcTestCenturyRegister() == EFI_SUCCESS) {
        Century = (Century & 0x7f) | (RtRTC_inp (RTC_ADDRESS_CENTURY) & 0x80);
    }
    RtRTC_outp (RTC_ADDRESS_CENTURY, Century);

    //
    // Allow updates of the RTC registers
    //

    RegisterB.Bits.SET = 0;
    RtRTC_outp(RTC_ADDRESS_REGISTER_B,RegisterB.Data);

    //
    // Restore the RTC's Context
    //

    RtRtcRestoreContext();

    //
    // Set the variable that containts the TimeZone and Daylight fields
    //

    SavedTimeZone = Time->TimeZone;
    SavedDaylight = Time->Daylight;
#ifdef EFI_NT_EMULATOR
    Status = RT->SetVariable (
                EFI_RTC_NAME,
                &EfiRtcGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(INT16) + sizeof(UINT8),
                &Time->TimeZone
                );
#endif
#ifndef EFI32
    Status = RT->SetVariable (
                EFI_RTC_NAME,
                &EfiRtcGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(INT16) + sizeof(UINT8),
                &Time->TimeZone
                );
#endif

    return Status;
}

#pragma RUNTIME_CODE(RtPlGetTime)
STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlGetTime (
    IN EFI_TIME                     *Time,
    IN EFI_TIME_CAPABILITIES        *Capabilities OPTIONAL
    )
{
    EFI_STATUS            Status;
    RTC_REGISTER_B        RegisterB;
    UINT8                 Century;
    UINTN                 BufferSize;

    if (Time == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Save the RTC's Context
    //

    RtRtcSaveContext();

    //
    // Wait for up to 0.1 seconds for the RTC to be updated
    //

    Status = RtRtcWaitToUpdate(100000);
    if (EFI_ERROR(Status)) {
        RtRtcRestoreContext();
        return Status;
    }

    //
    // Read Register B
    //

    RegisterB.Data = RtRTC_inp(RTC_ADDRESS_REGISTER_B);

    //
    // Get the Time/Date/Daylight Savings values.
    //

    Time->Second = RtRTC_inp(RTC_ADDRESS_SECONDS);  
    Time->Minute = RtRTC_inp(RTC_ADDRESS_MINUTES);
    Time->Hour   = RtRTC_inp(RTC_ADDRESS_HOURS);
    Time->Day    = RtRTC_inp(RTC_ADDRESS_DAY_OF_THE_MONTH);
    Time->Month  = RtRTC_inp(RTC_ADDRESS_MONTH);
    Time->Year   = RtRTC_inp(RTC_ADDRESS_YEAR);

    RtConvertRtcTimeToEfiTime(Time,RegisterB);

    if (RtRtcTestCenturyRegister() == EFI_SUCCESS) {
        Century = RtBCDtoDecimal ((UINT8)(RtRTC_inp(RTC_ADDRESS_CENTURY) & 0x7f));
    } else {
        Century = RtBCDtoDecimal (RtRTC_inp(RTC_ADDRESS_CENTURY));
    }
    Time->Year = Century * 100 + Time->Year;

    //
    // Restore the RTC's Context
    //

    RtRtcRestoreContext();

    //
    // Get the variable that containts the TimeZone and Daylight fields
    //

    Time->TimeZone = SavedTimeZone;
    Time->Daylight = SavedDaylight;

    BufferSize = sizeof(INT16) + sizeof(UINT8);
#ifdef EFI_NT_EMULATOR
    Status = RT->GetVariable (
                EFI_RTC_NAME,
                &EfiRtcGuid,
                NULL,
                &BufferSize,
                &Time->TimeZone
                );
#endif
#ifndef EFI32
    Status = RT->GetVariable (
                EFI_RTC_NAME,
                &EfiRtcGuid,
                NULL,
                &BufferSize,
                &Time->TimeZone
                );
#endif

    //
    // Make sure all field values are in correct range
    //

    Status = RtRtcTimeFieldsValid(Time);
    if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }

    //
    //  Fill in Capabilities if it was passed in
    //

    if (Capabilities) {
        RtZeroMem (Capabilities, sizeof(EFI_TIME_CAPABILITIES));
        Capabilities->Resolution = 1;            // 1 hertz
        Capabilities->Accuracy   = 50000000;     // 50 ppm
        Capabilities->SetsToZero = FALSE;
    }
    
    return EFI_SUCCESS;
}

#pragma RUNTIME_CODE(RtPlSetWakeupTime)
STATIC
EFI_STATUS
RUNTIMESERVICE
RtPlSetWakeupTime (
    IN BOOLEAN                      Enable,
    IN EFI_TIME                     *Time    OPTIONAL
    )
{
    //
    // The implementation of this function is platform specific.
    //
    return EFI_UNSUPPORTED;
}

#pragma RUNTIME_CODE(RtPlGetWakeupTime)
EFI_STATUS
RUNTIMESERVICE
RtPlGetWakeupTime (
    OUT BOOLEAN                     *Enabled,
    OUT BOOLEAN                     *Pending,
    OUT EFI_TIME                    *Time
    )
{
    //
    // The implementation of this function is platform specific.
    //
    return EFI_UNSUPPORTED;
}

//
// Low level I/O routines
//

#pragma RUNTIME_CODE(RtRTC_inp)
UINT8
RUNTIMEFUNCTION
RtRTC_inp (
    IN  UINT16 Address
    )
{
    UINT8 Data;

    RtDefIoWrite (NULL, IO_UINT8, PCAT_RTC_ADDRESS_REGISTER, 1, &Address);
    RtDefIoRead (NULL, IO_UINT8, PCAT_RTC_DATA_REGISTER, 1, &Data);
    return Data;
}

#pragma RUNTIME_CODE(RtRTC_outp)
VOID
RUNTIMEFUNCTION
RtRTC_outp (
    IN  UINT16 Address,
    IN  UINT8 Data
    )
{
    RtDefIoWrite (NULL, IO_UINT8, PCAT_RTC_ADDRESS_REGISTER, 1, &Address);
    RtDefIoWrite (NULL, IO_UINT8, PCAT_RTC_DATA_REGISTER, 1, &Data);
}

#pragma RUNTIME_CODE(RtRtcSaveContext)
VOID
RUNTIMEFUNCTION
RtRtcSaveContext (
    VOID
    )
{
    //
    // Acquire RTC Lock to make access to RTC atomic
    //

    RtAcquireLock(&RtcLock);

    //
    // Save the RTC's Address Register
    //

    RtDefIoRead (NULL, IO_UINT8, PCAT_RTC_ADDRESS_REGISTER, 1, &SavedAddressRegister);
}

#pragma RUNTIME_CODE(RtRtcRestoreContext)
VOID
RUNTIMEFUNCTION
RtRtcRestoreContext (
    VOID
    )
{
    //
    // Restore the RTC's Address Register
    //

    RtDefIoWrite (NULL, IO_UINT8, PCAT_RTC_ADDRESS_REGISTER, 1, &SavedAddressRegister);

    //
    // Release RTC Lock.
    //

    RtReleaseLock(&RtcLock);
}

//
// Get/Set Time/Date Functions
//

#pragma RUNTIME_CODE(RtRtcTimeFieldsValid)
EFI_STATUS
RUNTIMEFUNCTION
RtRtcTimeFieldsValid (
    IN EFI_TIME *Time
    )

{
    EFI_STATUS Status;

    Status = EFI_SUCCESS;
    if (Time->Year < 1998 || Time->Year > 2099) {
        Status = EFI_INVALID_PARAMETER;
    }

	if (Time->Month < 1 || Time->Month > 12) {
		Status = EFI_INVALID_PARAMETER;
	}

	if (Time->Day < 1) {
		Status = EFI_INVALID_PARAMETER;
	}
	else {
		if (Time->Month == 2) {
			if ((Time->Year % 4 == 0 && Time->Year % 100 != 0) || Time->Year % 400 == 0) {
				// Leap year
				if (Time->Day > 29) {
					Status = EFI_INVALID_PARAMETER;
				}
			}
			else if (Time->Day > 28) {
				Status = EFI_INVALID_PARAMETER;
			}
		}
		// 0x15AA is a bit mask, with the nth bit set if its corresponding month has 31 days.
		else if (Time->Day > (((1 << Time->Month) & 0x15AA) ? 31 : 30)) {
			Status = EFI_INVALID_PARAMETER;
		}
	}

    if (Time->Hour > 23) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (Time->Minute > 59) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (Time->Second > 59) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (Time->Nanosecond > 999999999) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (!(Time->TimeZone == EFI_UNSPECIFIED_TIMEZONE ||
          (Time->TimeZone >= -1440 && Time->TimeZone <= 1440))) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (Time->Daylight & (~(EFI_TIME_ADJUST_DAYLIGHT | EFI_TIME_IN_DAYLIGHT))) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (EFI_ERROR(Status)) {
        DEBUG((D_ERROR,"RTC : Invalid time/date field parameter\n"));
    }

    return Status;
}

#pragma RUNTIME_CODE(RtRtcWaitToUpdate)
EFI_STATUS
RUNTIMEFUNCTION
RtRtcWaitToUpdate (
    UINTN Timeout
    )

{
    RTC_REGISTER_A RegisterA;
    RTC_REGISTER_D RegisterD;

    //
    // See if the RTC is functioning correctly
    //

    RegisterD.Data = RtRTC_inp(RTC_ADDRESS_REGISTER_D);
    
    if (RegisterD.Bits.VRT == 0) {
        DEBUG((D_ERROR,"RTC : RTC/NVRAM not functional\n"));
        return EFI_DEVICE_ERROR;
    }

    //
    // Wait for up to 0.1 seconds for the RTC to be ready.
    //

    Timeout = (Timeout / 10) + 1;
    RegisterA.Data = RtRTC_inp(RTC_ADDRESS_REGISTER_A);
    while(RegisterA.Bits.UIP == 1 && Timeout > 0) {
        //
        // Calling the Rt version of Stall that we must be linked with.
        //  BS->Stall(10) does not exist at runtime
        //
        RtPlStall (10);
        RegisterA.Data = RtRTC_inp(RTC_ADDRESS_REGISTER_A);
        Timeout--;
    }

    if (Timeout == 0) {
        DEBUG((D_ERROR,"RTC : Timeout waiting to update\n"));
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}

#pragma RUNTIME_CODE(RtRtcTestCenturyRegister)
EFI_STATUS
RUNTIMEFUNCTION
RtRtcTestCenturyRegister (
    VOID
    )

{
    UINT8 Century;
    UINT8 Temp;

    Century = RtRTC_inp (RTC_ADDRESS_CENTURY);
    RtRTC_outp (RTC_ADDRESS_CENTURY, 0x00);
    Temp = RtRTC_inp(RTC_ADDRESS_CENTURY) & 0x7f;
    RtRTC_outp (RTC_ADDRESS_CENTURY, Century);
    if (Temp == 0x19 || Temp == 0x20) {
        return EFI_SUCCESS;
    }
    return EFI_DEVICE_ERROR;
}

#pragma RUNTIME_CODE(RtConvertEfiTimeToRtcTime)
VOID
RUNTIMEFUNCTION
RtConvertEfiTimeToRtcTime (
    IN EFI_TIME       *Time,
    IN RTC_REGISTER_B RegisterB,
    IN UINT8          *Century
    )

{
    BOOLEAN        PM;

    //
    // Adjust hour field if RTC in in 12 hour mode
    //

    if (RegisterB.Bits.MIL == 0) {
        PM = (Time->Hour >= 12);
        if (Time->Hour >= 13) {
            Time->Hour = (Time->Hour - 12);
        } else if (Time->Hour == 0) {
            Time->Hour = 12;
        }
    }

    //
    // Set the Time/Date/Daylight Savings values.
    //

    *Century = RtDecimaltoBCD((UINT8)(Time->Year / 100));
    
    Time->Year = Time->Year % 100;

    if (RegisterB.Bits.DM == 0) {
        Time->Year   = RtDecimaltoBCD((UINT8)Time->Year);
        Time->Month  = RtDecimaltoBCD(Time->Month);
        Time->Day    = RtDecimaltoBCD(Time->Day);
        Time->Hour   = RtDecimaltoBCD(Time->Hour);
        Time->Minute = RtDecimaltoBCD(Time->Minute);
        Time->Second = RtDecimaltoBCD(Time->Second);
    }

    //
    // If we are in 12 hour mode and PM is set, then set bit 7 of the Hour field.
    //

    if (RegisterB.Bits.MIL == 0 && PM) {
        Time->Hour = Time->Hour | 0x80;
    }
}

#pragma RUNTIME_CODE(RtConvertRtcTimeToEfiTime)
VOID
RUNTIMEFUNCTION
RtConvertRtcTimeToEfiTime (
    IN EFI_TIME       *Time,
    IN RTC_REGISTER_B RegisterB
    )

{
    BOOLEAN        PM;

    PM           = ((Time->Hour & 0x80) != 0);

    Time->Hour   = Time->Hour & 0x7f;

    if (RegisterB.Bits.DM == 0) {
        Time->Year   = RtBCDtoDecimal((UINT8)Time->Year);
        Time->Month  = RtBCDtoDecimal(Time->Month);
        Time->Day    = RtBCDtoDecimal(Time->Day);
        Time->Hour   = RtBCDtoDecimal(Time->Hour);
        Time->Minute = RtBCDtoDecimal(Time->Minute);
        Time->Second = RtBCDtoDecimal(Time->Second);
    }

    //
    // If time is in 12 hour format, convert it to 24 hour format
    //

    if (RegisterB.Bits.MIL == 0) {
        if (PM && Time->Hour < 12) {
            Time->Hour = Time->Hour + 12;
        }
        if (!PM && Time->Hour == 12) {
            Time->Hour = 0;
        }
    }
    
    Time->Nanosecond = 0;
    Time->TimeZone   = EFI_UNSPECIFIED_TIMEZONE;
    Time->Daylight   = 0;
}
