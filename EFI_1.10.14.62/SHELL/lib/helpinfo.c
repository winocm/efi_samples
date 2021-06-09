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

  HelpInfo.c

Abstract:

  Help Infomation of Shell Command access module.

Notes:

 The transaction functions depend on the raw data format.
 Data Organization Format in gHelpData[]:

 Cmd1_Name'\0' Cmd1_Format'\0' Cmd1_Line_Help'\0'
 Cmd1_Verbose_Help ...'\0'
 Cmd2_Name'\0' Cmd2_Format'\0' Cmd2_Line_Help'\0'
 Cmd2_Verbose_Help ...'\0'
 ...
 CmdN_Name'\0' CmdN_Format'\0' CmdN_Line_Help'\0'
 CmdN Verbose Help ...'\0'
 '\0'

Revision History

--*/

#include "shelllib.h"


CHAR16* 
GetCmdFormat(
  IN  CHAR16* CmdName
  )
/*++
Routine Description:
  Get Help Format
  
Arguments:
  CmdName     - cmd's name

Returns:
  Pointer to Help Format string.

--*/
{
  CHAR16  *Ptr;

  Ptr = gHelpData;
  while (StriCmp (Ptr, CmdName) != 0) {
    if ( *Ptr == 0) {
      return NULL;
    }
    while ( *Ptr++ != 0);
  }
  //
  // Found Cmd
  // Skip Cmd Name line,
  // Then Ptr points to Cmd Format line
  //
  while ( *Ptr++ != 0);
  
  return Ptr;
}

CHAR16* 
GetLineHelp(
  IN  CHAR16* CmdName
  )
/*++
Routine Description:
  Get Line Help information 

Arguments:
  CmdName     - cmd's line help

Returns:
  Pointer to line help information string

--*/
{
  CHAR16  *Ptr;

  Ptr = gHelpData;
  while (StriCmp (Ptr, CmdName) != 0) {
    if ( *Ptr == 0) {
      return NULL;
    }
    while ( *Ptr++ != 0);
  }
  //
  // Found Cmd
  // Skip Cmd Name line and Cmd Format line,
  // Then Ptr points to Cmd Format
  //
  while ( *Ptr++ != 0);
  while ( *Ptr++ != 0);

  return Ptr;
}

CHAR16* 
GetVerboseHelp(
  IN  CHAR16* CmdName
  )
/*++
Routine Description:
  Get Page Help information 

Arguments:
  CmdName     - cmd's page help

Returns:
  Pointer to page help information string.

--*/
{
  CHAR16  *Ptr;

  Ptr = gHelpData;
  while (StriCmp (Ptr, CmdName) != 0) {
    if ( *Ptr == 0) {
      return NULL;
    }
    while ( *Ptr++ != 0);
  }
  //
  // Found Cmd
  // Skip Cmd Name line, Cmd Format line and Cmd Line help line,
  // Then Ptr points to Cmd Format.
  //
  while ( *Ptr++ != 0);
  while ( *Ptr++ != 0);
  while ( *Ptr++ != 0);

  return Ptr;
}

VOID
PrintVerboseHelp(
  IN  CHAR16* VerboseHelp
  )
{
  //
  // Default is to enable page break
  //
  EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  Print (L"%s", VerboseHelp);
}

VOID
PrintHelpInfo(
  IN  CHAR16  *CmdName
  )
{
  CHAR16      *HelpInfo;

  //
  // If having verbose help, print verbose help
  // If no verbose help but having line help, print line help
  // ELSE print error message 
  //

  HelpInfo = NULL;
  HelpInfo = GetVerboseHelp (CmdName);
  if(HelpInfo != NULL) {
    PrintVerboseHelp (HelpInfo);
  } else if (GetLineHelp (CmdName) != NULL) {
    Print (GetLineHelp (CmdName));
    Print (L"\n");
  } else {
    Print (L"%s: No help Information\n", CmdName);
  }
}
