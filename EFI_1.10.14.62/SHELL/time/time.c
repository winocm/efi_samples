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

  EFI shell command "time"

Revision History

--*/

#include "shell.h"

EFI_STATUS
InitializeTime (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

static
BOOLEAN
GetNumber(
  IN OUT  INTN    *ArgNum, 
  IN OUT  INTN    *Offset, 
  IN OUT  INTN    *number,
  IN    BOOLEAN   GetSecond
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeTime)
#endif

EFI_STATUS
InitializeTime (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*+++

  time [hh:mm:ss] 

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
    ImageHandle,   SystemTable,   InitializeTime,
    L"time",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);
  
  if (SI->Argc == 1) {
    Status = RT->GetTime (&Time, NULL);

    if (!EFI_ERROR(Status)) {
      Print(L"%02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second);
      return EFI_SUCCESS;
    } else {
      Print(L"time: Clock not functional\n");
      return Status;
    }
  }

  //
  // Get current time
  //
  Status = RT->GetTime (&Time, NULL);
  if (EFI_ERROR(Status)) {
    Print(L"time: Clock not functional\n");
    return Status;      
  }

  Index = 1;          // number of argument
  Offset = 0;        // Offset in argument string
  
  //
  // Get hour
  //
  if ( !GetNumber(&Index, &Offset, &Data, FALSE) ) {
    Print(L"time: Invalid hour\n");
    return EFI_INVALID_PARAMETER;
  }
  if (Data < 0 || Data > 23 ) {
    Print(L"time: Invalid hour. Hour range: 0 - 23\n");
    return EFI_INVALID_PARAMETER;
  } 
  Time.Hour = (UINT8)Data;

  //
  // Get minute
  //
  if ( !GetNumber(&Index, &Offset, &Data, FALSE) ) {
    Print(L"time: Invalid minutes\n");
    return EFI_INVALID_PARAMETER;
  }
  if (Data < 0 || Data > 59) {
    Print(L"time: Invalid minute. Minute range: 0 - 59\n");
    return EFI_INVALID_PARAMETER;
  }
  Time.Minute = (UINT8)Data;
  
  //
  // Get second
  //
  if ( !GetNumber(&Index, &Offset, &Data, TRUE) ) {
    Print(L"time: Invalid seconds\n");
    return EFI_INVALID_PARAMETER;
  }
  if (Data < 0 || Data > 59  ) {
    Print(L"time: Invalid seconds. Second range: 0 - 59\n");
    return EFI_INVALID_PARAMETER;
  }
  Time.Second = (UINT8)Data;
 
  Status = RT->SetTime(&Time);
  if (EFI_ERROR(Status)) {
    Print(L"time: Clock not functional\n");
    return Status;
  }
  return EFI_SUCCESS;
}  
 
static
BOOLEAN
GetNumber(
  IN OUT  INTN    *ArgNum, 
  IN OUT  INTN    *Offset, 
  IN OUT  INTN    *Number, 
  IN    BOOLEAN   GetSecond
  )
{ 
  CHAR16    ch;
  INTN      Data;
  BOOLEAN   Stop;
  BOOLEAN   EndNum;
  BOOLEAN   FindNumber;
  //
  //
  //
  Data       = 0;
  Stop       = FALSE;
  EndNum     = FALSE;
  FindNumber = FALSE;
    
  while ( *ArgNum < (INTN)SI->Argc && !Stop ) {
    for ( ; *Offset < (INTN)StrLen(SI->Argv[*ArgNum]) && !Stop; (*Offset)++ ) {
      ch = SI->Argv[*ArgNum][*Offset];
      if ( ch >= '0' && ch <= '9'  ) {
        if ( EndNum ) {
          return FALSE;
        }
        Data = Data * 10 + (ch - '0');
        FindNumber = TRUE;
      } else if (ch == ':') {
        if ( !FindNumber ) {
          return FALSE;
        }
        if ( GetSecond ) {
          return FALSE;          
        }
        Stop = TRUE;
      } else {
        return FALSE;
      }
    }
    //
    //
    //
    if ( *Offset == (INTN)StrLen(SI->Argv[*ArgNum]) ) {
      *Offset = 0;
      (*ArgNum)++;
      EndNum = TRUE;
    }
  }  
  *Number = Data;
  return (BOOLEAN)(FindNumber | GetSecond);
}
