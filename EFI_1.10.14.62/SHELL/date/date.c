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

  date.c

Abstract: 

  shell command "date"

Revision History

--*/

#include "shell.h"

//
//
//

EFI_STATUS
InitializeDate (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
//
//

static 
BOOLEAN
GetNumber (
  IN OUT  INTN  *ArgNum, 
  IN OUT  INTN  *Position, 
  IN OUT  INTN  *Number,
  IN BOOLEAN    GetYear 
  );

static
BOOLEAN
ValidDay (
  IN  EFI_TIME  time
  );

static
BOOLEAN
IsLeapYear (
  IN EFI_TIME   time
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeDate)
#endif

INTN DayOfMonth[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//
//
//

EFI_STATUS
InitializeDate (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Display or set date
  
Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error
  
Notes:  
  date [mm/dd/yyyy]

--*/
{
  EFI_STATUS  Status;
  EFI_TIME    Time;
  UINTN       Index;
  UINTN       Offset;
  UINTN       Data;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeDate,
    L"date",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);
  
  if (SI->Argc == 1) {
    Status = RT->GetTime(&Time,NULL);

    if (!EFI_ERROR(Status)) {
      Print(L"%02d/%02d/%04d\n",Time.Month,Time.Day,Time.Year);
      return EFI_SUCCESS;
    }else {
      Print(L"date: Clock not functional\n");
      return Status;      
    }
  }

  //
  // Get current time
  //
  Status = RT->GetTime(&Time,NULL);
  if (EFI_ERROR(Status)) {
    Print(L"date: Clock not functional\n");
    return Status;      
  }

  //
  // Init start number of argument and 
  // offset position in argument string
  //
  Index = 1;          
  Offset = 0;        

  //
  // Get month 
  //
  if ( !GetNumber(&Index, &Offset, &Data, FALSE) ) {
    Print(L"date: Invalid month\n");
    return EFI_INVALID_PARAMETER;
  }
  if (Data < 1 || Data > 12) {
    Print(L"date: Invalid month. Month range: 1 - 12\n");
    return EFI_INVALID_PARAMETER;
  } 
  Time.Month = (UINT8)Data;

  //
  // Get day 
  //
  if ( !GetNumber(&Index, &Offset, &Data, FALSE) ) {
    Print(L"date: Invalid day\n");    
    return EFI_INVALID_PARAMETER;
  }
  if (Data < 1 || Data > 31) {
    Print(L"date: Invalid day\n");
    return EFI_INVALID_PARAMETER;
  }  
  Time.Day = (UINT8)Data;

  //
  // Get year. 
  //
  if ( !GetNumber(&Index, &Offset, &Data, TRUE) ) {
    Print(L"date: Invalid Year. Year range : 1998 - 2099\n");
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Minimal year number supported is 1998
  //
  if (Data<100) {
    Data = Data + 1900;
    if (Data < 1998) {
      Data = Data + 100;
    }
  }
  if (Data < 1998 || Data > 2099) {
    Print(L"date: Invalid Year. Year range: 1998 - 2099\n");
    return EFI_INVALID_PARAMETER;
  }
  Time.Year = (UINT16)Data;
  
  if ( !ValidDay(Time) ) {
    Print(L"date: Invalid day\n");
    return EFI_INVALID_PARAMETER;
  }
  
  Status = RT->SetTime(&Time);
  if (EFI_ERROR(Status)) {
    Print(L"date: Clock not functional\n");
    return Status;
  }
  return EFI_SUCCESS;
}  
 
//
// Get number from arguments
//

static
BOOLEAN
GetNumber(
  IN OUT  INTN    *ArgNum, 
  IN OUT  INTN    *Position, 
  IN OUT  INTN    *Number, 
  IN BOOLEAN      GetYear 
  )
{
  CHAR16    Char;
  INTN      Data;
  BOOLEAN   Stop;
  BOOLEAN   EndNum;
  BOOLEAN   FindNumber;

  Data       = 0;
  Stop       = FALSE;
  EndNum     = FALSE;
  FindNumber = FALSE;

  while ( *ArgNum < (INTN)SI->Argc && !Stop ) {
    for ( ; *Position < (INTN)StrLen(SI->Argv[*ArgNum]) && !Stop; (*Position)++ ) {
      Char = SI->Argv[*ArgNum][*Position];
      if (Char >= '0' && Char <= '9') {
        //
        // For numeric characters
        //
        if ( EndNum ) {
          return FALSE;
        }
        Data = Data * 10 + (Char - '0');
        //
        // to avoid data over-flow
        //
        if (Data > 10000) {
          return FALSE;
        }
        FindNumber = TRUE;
      } else if (Char == '/') {
        //
        // For separator characters in data format
        //
        if ( !FindNumber || GetYear ) {
          return FALSE;
        }
        Stop = TRUE;
      } else {
        //
        // Any other characters regards as invalid
        //
        return FALSE;
      }
    }
    
    //
    // Adjust scan Offset in next argument
    //
    if ( *Position == (INTN)StrLen(SI->Argv[*ArgNum]) ) {
      *Position = 0;
      (*ArgNum)++;
      EndNum = TRUE;
    }
  }  

  *Number = Data;
  return FindNumber;    
}

static
BOOLEAN
ValidDay (
  IN  EFI_TIME  time
  )
{
  if ( time.Day > DayOfMonth[time.Month - 1] ) {
    return FALSE;
  }
  //
  // Pay attention to month==2
  //  
  if ( time.Month == 2 && ((IsLeapYear(time) && time.Day > 29) || (!IsLeapYear(time) && time.Day > 28)) ) {
    return FALSE;
  }
  
  return TRUE;
}

static
BOOLEAN
IsLeapYear (
  IN EFI_TIME   time
  )
{
  //
  // 
  // 
  if ( time.Year % 4 == 0 ) {
    if ( time.Year % 100 == 0 ) {
      if ( time.Year % 400 == 0 ) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  } else {
    return FALSE;  
  }
}
  
