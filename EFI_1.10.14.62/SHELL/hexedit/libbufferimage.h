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
    libBufferImage.h

  Abstract:
    Defines BufferImage - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file

--*/


#ifndef _LIB_BUFFER_IMAGE_H_
#define _LIB_BUFFER_IMAGE_H_

#include "heditortype.h"


  EFI_STATUS  HBufferImageInit (VOID);
  EFI_STATUS  HBufferImageCleanup (VOID);
  EFI_STATUS  HBufferImageRefresh (VOID);
  EFI_STATUS  HBufferImageHide (VOID);
  EFI_STATUS  HBufferImageHandleInput (EFI_INPUT_KEY*);
  EFI_STATUS  HBufferImageBackup (VOID);

  EFI_STATUS HBufferImageRead ( 
    IN CHAR16* , 
    IN CHAR16*,
    IN UINTN   ,
    IN UINTN   ,
    IN UINTN   ,
    IN UINTN   ,
    IN HEFI_EDITOR_ACTIVE_BUFFER_TYPE ,
    IN BOOLEAN 
);

 EFI_STATUS HBufferImageSave ( 
    IN CHAR16* , 
    IN CHAR16*,    
    IN UINTN   ,
    IN UINTN   ,
    IN UINTN   ,
    IN UINTN   ,
    IN HEFI_EDITOR_ACTIVE_BUFFER_TYPE     
);

 INTN HBufferImageCharToHex ( 
  IN CHAR16 
);


  EFI_STATUS  HBufferImageRestoreMousePosition (VOID);
  EFI_STATUS  HBufferImageRestorePosition (VOID);

VOID
HBufferImageMovePosition ( 
  IN UINTN ,
  IN UINTN ,
  IN BOOLEAN 
);


EFI_STATUS HBufferImageHandleInput ( EFI_INPUT_KEY *);

HEFI_EDITOR_LINE*  HBufferImageCreateLine  ( VOID);


EFI_STATUS HBufferImageDoCharInput (CHAR16 );
EFI_STATUS HBufferImageAddChar ( CHAR16 );

BOOLEAN HInCurrentScreen (UINTN  ); 
BOOLEAN HAboveCurrentScreen (UINTN );
BOOLEAN HUnderCurrentScreen (UINTN );

EFI_STATUS HBufferImageScrollRight ();
EFI_STATUS HBufferImageScrollLeft ();
EFI_STATUS HBufferImageScrollDown ();
EFI_STATUS HBufferImageScrollUp ();
EFI_STATUS HBufferImagePageUp ();
EFI_STATUS HBufferImagePageDown ();
EFI_STATUS HBufferImageHome ();
EFI_STATUS HBufferImageEnd ();

EFI_STATUS HBufferImageDoBackspace ();
EFI_STATUS HBufferImageDoDelete ();

EFI_STATUS HBufferImageCutLine ( HEFI_EDITOR_LINE ** );
EFI_STATUS HBufferImagePasteLine ( );

EFI_STATUS HBufferImageGetFileInfo ( EFI_FILE_HANDLE ,
                                     CHAR16 *, 
                                     EFI_FILE_INFO ** 
                                   );

EFI_STATUS HBufferImageSearch ( CHAR16 * , UINTN);
EFI_STATUS HBufferImageReplace ( CHAR16 * , UINTN);

EFI_STATUS   HBufferImageFree (  VOID ) ;

EFI_STATUS HBufferImageDeleteCharacterFromBuffer ( IN  UINTN, 
                                                   IN UINTN, 
                                                   UINT8 * 
                                                 );

EFI_STATUS HBufferImageAddCharacterToBuffer ( IN  UINTN, 
                                              IN UINTN, 
                                              UINT8 * 
                                            );



EFI_STATUS
HBufferImageBufferToList ( IN VOID *,  IN UINTN );


EFI_STATUS
HBufferImageListToBuffer ( IN VOID *,  IN UINTN );


VOID
HBufferImageAdjustMousePosition ( INT32 , INT32  );


BOOLEAN
HBufferImageIsAtHighBits ( UINTN, UINTN * ) ;

 
EFI_STATUS
HBufferImageCutLine ( 
  HEFI_EDITOR_LINE **
);

UINTN  HBufferImageGetTotalSize ( );

BOOLEAN HBufferImageIsInSelectedArea (   UINTN ,   UINTN );

#endif
