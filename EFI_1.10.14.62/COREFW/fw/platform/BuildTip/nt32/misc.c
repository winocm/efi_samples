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

    misc.c

Abstract:

    Miscellaneous support routines for EFI emulation.


--*/


#include "ntemul.h"

#define EFI_RTC_NAME             L"RTC"
#define EFI_ALARM_NAME           L"ALARM"
#define EFI_ALARM_ENABLE_NAME    L"ALARMEN"

//
// BugBug - get this from the platform time code.  Need to find a common declaration point.
//
#define EFI_RTC_GUID \
    { 0xe4329f89, 0x546b, 0x11d4, 0x9a, 0x39, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }
EFI_GUID EfiRtcGuid = EFI_RTC_GUID;

VOID
PlSystemTimeToEfiTime (
    IN SYSTEMTIME       *SystemTime,
    OUT EFI_TIME        *Time
    )
{
    TIME_ZONE_INFORMATION   TimeZone;
    GetTimeZoneInformation (&TimeZone);

    Time->Year   = (UINT16) SystemTime->wYear;
    Time->Month  = (UINT8)  SystemTime->wMonth;
    Time->Day    = (UINT8)  SystemTime->wDay;
    Time->Hour   = (UINT8)  SystemTime->wHour;
    Time->Minute = (UINT8)  SystemTime->wMinute;
    Time->Second = (UINT8)  SystemTime->wSecond;
    Time->Nanosecond = (UINT32) SystemTime->wMilliseconds * 1000000;
    Time->TimeZone = (INT16) TimeZone.Bias;

    if (TimeZone.StandardDate.wMonth) {
        Time->Daylight = EFI_TIME_ADJUST_DAYLIGHT;
    }    
}

VOID
PlEfiTimeToSystemTime (
    IN EFI_TIME        *Time,
    OUT SYSTEMTIME     *SystemTime
    )
{
    SystemTime->wYear         = Time->Year;
    SystemTime->wMonth        = Time->Month;
    SystemTime->wDay          = Time->Day;
    SystemTime->wHour         = Time->Hour;
    SystemTime->wMinute       = Time->Minute;
    SystemTime->wSecond       = Time->Second;
    SystemTime->wMilliseconds = Time->Nanosecond / 1000000;
}

EFI_STATUS
PlGetTime (
    IN EFI_TIME                 *Time,
    IN EFI_TIME_CAPABILITIES    *Cap OPTIONAL
    )
// Provide the EFI get time function
{
    SYSTEMTIME              SystemTime;
    UINTN                   BufferSize;
    EFI_STATUS              Status;

    if (Time == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    ZeroMem (Time, sizeof(EFI_TIME));

    GetSystemTime (&SystemTime);
    PlSystemTimeToEfiTime (&SystemTime, Time);

    Time->Nanosecond = 0;

    BufferSize = sizeof(INT16) + sizeof(UINT8);
    Status = RT->GetVariable (
                EFI_RTC_NAME,
                &EfiRtcGuid,
                NULL,
                &BufferSize,
                &Time->TimeZone
                );

    if (Cap) {
        ZeroMem (Cap, sizeof(EFI_TIME_CAPABILITIES));
        Cap->Resolution = 1;            // 1 hertz
        Cap->Accuracy   = 50000000;     // 50 ppm
        Cap->SetsToZero = FALSE;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
PlTimeFieldsValid (
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


EFI_STATUS
PlSetTime (
    IN EFI_TIME                 *Time
    )
// Provide the EFI set time function
{
    SYSTEMTIME    SystemTime;
    EFI_STATUS    Status;

    Status = PlTimeFieldsValid (Time);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    PlEfiTimeToSystemTime (Time, &SystemTime);
    SetSystemTime (&SystemTime);

    Status = RT->SetVariable (
                EFI_RTC_NAME,
                &EfiRtcGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(INT16) + sizeof(UINT8),
                &Time->TimeZone
                );

    return EFI_SUCCESS;
}


EFI_STATUS
PlSetWakeupTime (
    IN BOOLEAN                      Enable,
    IN EFI_TIME                     *Time
    )
{
    EFI_STATUS Status;
    
    Status = PlTimeFieldsValid (Time);
    if (EFI_ERROR(Status)) {
        return Status;
    }


    Status = RT->SetVariable (
                EFI_ALARM_NAME,
                &EfiRtcGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(EFI_TIME),
                Time
                );

    Status = RT->SetVariable (
                EFI_ALARM_ENABLE_NAME,
                &EfiRtcGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(BOOLEAN),
                &Enable
                );

    return Status;
}

EFI_STATUS
PlGetWakeupTime (
    OUT BOOLEAN                     *Enabled,
    OUT BOOLEAN                     *Pending,
    OUT EFI_TIME                    *Time
    )
{
    EFI_STATUS            Status;
    UINTN                 BufferSize;
    EFI_TIME              CurrentTime;
    UINT32                Current;
    UINT32                Wakeup;

    if (Enabled == NULL || Pending == NULL || Time == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    ZeroMem(Time,sizeof(Time));
    BufferSize = sizeof (EFI_TIME);
    Status = RT->GetVariable (
                EFI_ALARM_NAME,
                &EfiRtcGuid,
                NULL,
                &BufferSize,
                Time
                );

    *Enabled = FALSE;
    BufferSize = sizeof (BOOLEAN);
    Status = RT->GetVariable (
                EFI_ALARM_ENABLE_NAME,
                &EfiRtcGuid,
                NULL,
                &BufferSize,
                Enabled
                );

    *Pending = FALSE;

    if (!*Enabled) {
        return EFI_SUCCESS;
    }

    Status = PlGetTime(&CurrentTime,NULL);

    Current = (CurrentTime.Year << 16) | (CurrentTime.Month << 8) | (CurrentTime.Day);
    Wakeup  = (Time->Year << 16) | (Time->Month << 8) | (Time->Day);

    if (Current < Wakeup) {
        return EFI_SUCCESS;
    }
    if (Current > Wakeup) {
        *Pending = TRUE;
        return EFI_SUCCESS;
    }

    Current = (CurrentTime.Hour << 16) | (CurrentTime.Minute << 8) | (CurrentTime.Second);
    Wakeup  = (Time->Hour << 16) | (Time->Minute << 8) | (Time->Second);

    if (Current < Wakeup) {
        return EFI_SUCCESS;
    }

    *Pending = TRUE;
    return EFI_SUCCESS;
}

#define MAX_FILE_NAME_LENGTH 280

EFI_STATUS
WinNtLoadAsDll (
  IN  CHAR8  *PdbFileName,
  IN  VOID   **ImageEntryPoint
  )
/*++

Routine Description:

  Loads the .DLL file is present when a PE/COFF file is loaded.  This provides source level
  debugging for drivers that have cooresponding .DLL files on the local system.

Arguments:

  PdbFileName     - The name of the .PDB file.  This was found from the PE/COFF
                    file's debug directory entry.

  ImageEntryPoint - A pointer to the DLL entry point of the .DLL file was loaded.

Returns:

  EFI_SUCCESS     - The .DLL file was loaded, and the DLL entry point is returned in ImageEntryPoint

  EFI_NOT_FOUND   - The .DLL file could not be found

  EFI_UNSUPPORTED - The .DLL file was loaded, but the entry point to the .DLL file could not
                    determined.

--*/
{
  CHAR16      DllFileName[MAX_FILE_NAME_LENGTH];
  HMODULE     Library;
  UINTN       Index;
  

  *ImageEntryPoint = NULL;

  //
  // Convert filename from ASCII to Unicode
  //
  for (Index = 0;Index < MAX_FILE_NAME_LENGTH && PdbFileName[Index] != 0; Index++) {
    DllFileName[Index] = PdbFileName[Index];
  }
  DllFileName[Index] = 0;

  //
  // Check that we have a valid filename
  //
  if (Index < 5 ||
      Index >= MAX_FILE_NAME_LENGTH ||
      DllFileName[Index - 4] != '.') {
    return EFI_NOT_FOUND;
  }

  //
  // Replace .PDB with .DLL on the filename
  //
  DllFileName[Index - 3] = 'D';
  DllFileName[Index - 2] = 'L';
  DllFileName[Index - 1] = 'L';

  //
  // Load the .DLL file into the user process's address space
  //
  Library = GetModuleHandle(DllFileName);
  if(Library != NULL){
      FreeLibrary(Library);
  }
  Library = LoadLibraryEx (
                      DllFileName,
                      NULL,
                      DONT_RESOLVE_DLL_REFERENCES
                      );

  if (Library == NULL) {
    return EFI_NOT_FOUND;
  }
  //
  // InitializeDriver is the entry point we put in all our EFI DLL's. The
  // DONT_RESOLVE_DLL_REFERENCES argument to LoadLIbraryEx() supresses the normal
  // DLL entry point of DllMain, and prevents other modules that are referenced
  // in side the DllFileName from being loaded.
  //
  *ImageEntryPoint = (VOID *)(UINTN)GetProcAddress (
                                              Library,
                                              "InitializeDriver"
                                              );
  
  if(*ImageEntryPoint == NULL) {
    return EFI_UNSUPPORTED;
  }
  return EFI_SUCCESS;
}

