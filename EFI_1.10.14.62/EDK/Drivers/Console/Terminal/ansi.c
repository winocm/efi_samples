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

    ansi.c
    
Abstract: 
    

Revision History
--*/

#include "Terminal.h"


VOID
AnsiRawDataToUnicode (
  IN  TERMINAL_DEV    *TerminalDevice
  )
{
  UINT8               RawData;
  
  //
  // pop the raw data out from the raw fifo,
  // and translate it into unicode, then push 
  // the unicode into unicode fifo, until the raw fifo is empty.
  //
  while (!IsRawFiFoEmpty (TerminalDevice)) {
    
    RawFiFoRemoveOneKey (TerminalDevice,&RawData);
    
    UnicodeFiFoInsertOneKey (TerminalDevice,(UINT16)RawData);   
  }
}   

EFI_STATUS
AnsiTestString (
  IN  TERMINAL_DEV    *TerminalDevice,
  IN  CHAR16          *WString
  )
{
  CHAR8         GraphicChar;
  
  //
  // support three kind of character:
  // valid ascii, valid efi control char, valid text graphics.
  //
  for (;*WString != CHAR_NULL; WString++) {
    
    if ( !(TerminalIsValidAscii (*WString) || 
           TerminalIsValidEfiCntlChar (*WString) || 
           TerminalIsValidTextGraphics (*WString, &GraphicChar, NULL) )) {
            
            return EFI_UNSUPPORTED;
    }          
  } 
  
  return EFI_SUCCESS;
}   
