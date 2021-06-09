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

  DrawLogo.c
  
Abstract:

  This routine is used by the graphics console to display splash screen.

  Today we draw the splash logo in all four corners of the screen.

Revision History


--*/

#include "GraphicsConsole.h"



VOID
GraphicsConsoleDrawLogo (
  IN  EFI_UGA_DRAW_PROTOCOL  *UgaDraw
  )
{
  EFI_STATUS              Status;
  EFI_UGA_SPLASH_PROTOCOL *UgaSplash;
  UINT32                  SizeOfX, SizeOfY, ColorDepth, RefreshRate;

  Status = gBS->LocateProtocol (&gEfiUgaSplashProtocolGuid, NULL, (VOID **)&UgaSplash);
  if (EFI_ERROR (Status)) {
    return;
  }

 
  UgaDraw->GetMode (UgaDraw, &SizeOfX, &SizeOfY, &ColorDepth, &RefreshRate);
  
  //
  // Draw Logo in upper left corner
  //
  UgaDraw->Blt (
            UgaDraw,
            UgaSplash->Image,                 EfiUgaBltBufferToVideo,           
            0,                                0,
            SizeOfX - UgaSplash->PixelWidth,  0,
            UgaSplash->PixelWidth,            UgaSplash->PixelHeight,
            UgaSplash->PixelWidth * sizeof (EFI_UGA_PIXEL)
            );


}
