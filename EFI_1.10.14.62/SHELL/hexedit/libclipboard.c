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
    libclipboard.c

  Abstract:
    Functions to deal with Clip Board
--*/


#ifndef _LIB_CLIP_BOARD
#define _LIB_CLIP_BOARD

#include "heditor.h"


HEFI_EDITOR_CLIPBOARD HClipBoard;

//
// for basic initialization of HClipBoard
//
HEFI_EDITOR_CLIPBOARD  HClipBoardConst = {
  NULL,
  0
};




EFI_STATUS
HClipBoardInit (
  VOID
)
/*++

Routine Description: 

  Initialization function for HDiskImage

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR

--*/
{ 

  //
  // basiclly initialize the HDiskImage
  //
  CopyMem (&HClipBoard, &HClipBoardConst, sizeof(HClipBoard));

  return EFI_SUCCESS; 
}





EFI_STATUS
HClipBoardCleanup (
  VOID
)
/*++

Routine Description: 

  Initialization function for HDiskImage

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR

--*/
{ 

  HEditorFreePool ( HClipBoard.Buffer );
  
  return EFI_SUCCESS;
}




EFI_STATUS
HClipBoardSet (
  IN UINT8 *Buffer,
  IN UINTN Size
)
{
  //
  // free the old clipboard buffer
  // and set new clipboard buffer
  //
  HEditorFreePool ( HClipBoard.Buffer );
  HClipBoard.Buffer = Buffer;
  
  HClipBoard.Size = Size;
  
  return EFI_SUCCESS;
}




UINTN
HClipBoardGet (
  OUT UINT8  **Buffer
)
{
  //
  // return the clipboard buffer
  //  
  *Buffer = HClipBoard.Buffer ;
  
  return HClipBoard.Size;
}


#endif 
