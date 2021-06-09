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
    libMemImage.c

  Abstract:
    Functions to deal with Mem buffer
--*/


#ifndef _LIB_MEM_IMAGE
#define _LIB_MEM_IMAGE

#include "heditor.h"

extern EFI_HANDLE HImageHandleBackup;

extern HEFI_EDITOR_BUFFER_IMAGE  HBufferImage;

extern  BOOLEAN HBufferImageNeedRefresh ;
extern  BOOLEAN HBufferImageOnlyLineNeedRefresh ;
extern  BOOLEAN HBufferImageMouseNeedRefresh ;

extern HEFI_EDITOR_GLOBAL_EDITOR HMainEditor;


HEFI_EDITOR_MEM_IMAGE HMemImage;
HEFI_EDITOR_MEM_IMAGE HMemImageBackupVar;

//
// for basic initialization of HDiskImage
//
HEFI_EDITOR_MEM_IMAGE  HMemImageConst = {
  NULL,
  0,
  0
};




EFI_STATUS
HMemImageInit (
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
  EFI_STATUS                        Status;
  UINTN                             PciRootBridgeIoHandleCount;
  EFI_HANDLE                        *PciRootBridgeIoBuffer;

  //
  // basically initialize the HMemImage
  //
  CopyMem (&HMemImage, &HMemImageConst, sizeof(HMemImage));

  PciRootBridgeIoBuffer = NULL;
  Status = BS->LocateHandleBuffer (
                    ByProtocol,           
                    &gEfiPciRootBridgeIoProtocolGuid, 
                    NULL,
                    &PciRootBridgeIoHandleCount,  
                    (void ***)&PciRootBridgeIoBuffer
                    );
  if (!EFI_ERROR(Status) && PciRootBridgeIoHandleCount > 0) {
    Status = BS->HandleProtocol (
                  PciRootBridgeIoBuffer[0], 
                  &gEfiPciRootBridgeIoProtocolGuid, 
                  &HMemImage.IoFncs
                  );
    BS->FreePool(PciRootBridgeIoBuffer);
    
    if ( EFI_ERROR ( Status ) ) {
      return EFI_LOAD_ERROR;
    }
  } else {
      return EFI_LOAD_ERROR;
  }


  return EFI_SUCCESS; 
}



EFI_STATUS 
HMemImageBackup (
  VOID
) 
/*++

Routine Description: 

  Backup function for HDiskImage
  Only a few fields need to be backup. 
  This is for making the Disk buffer refresh 
  as few as possible.

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  HMemImageBackupVar.Offset = HMemImage.Offset;                          
  HMemImageBackupVar.Size = HMemImage.Size;

  return EFI_SUCCESS;
}




EFI_STATUS
HMemImageCleanup   (
  VOID
)
/*++

Routine Description: 

  Cleanup function for HDiskImage

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  return EFI_SUCCESS;  
}








EFI_STATUS
HMemImageSetMemOffsetSize (
  IN UINTN Offset,
  IN UINTN Size 
)
/*++

Routine Description: 

  Set FileName field in HFileImage

Arguments:  

  Str -- File name to set

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  
  HMemImage.Offset = Offset;
  HMemImage.Size = Size;
  
  return EFI_SUCCESS;
}




EFI_STATUS
HMemImageRead ( 
  IN UINTN Offset,
  IN UINTN Size, 
  BOOLEAN Recover
)
/*++

Routine Description: 

  Read a disk from disk into HBufferImage

Arguments:  

  FileName -- filename to read
  Recover -- if is for recover, no information print

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR
  EFI_OUT_OF_RESOURCES
  
--*/
{

  EFI_STATUS        Status;
  void              *Buffer;
  CHAR16            *Str;
  HEFI_EDITOR_LINE  *Line;

  HEFI_EDITOR_ACTIVE_BUFFER_TYPE BufferTypeBackup;


  BufferTypeBackup = HBufferImage.BufferType ;
  HBufferImage.BufferType = MEM_BUFFER;
  

  Buffer = AllocatePool ( Size );
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = HMemImage.IoFncs->Mem.Read (
                        HMemImage.IoFncs,
                        EfiPciWidthUint8,
                        Offset,
                        Size,
                        Buffer
                        );

  if (EFI_ERROR(Status)) {
    FreePool(Buffer);
    return EFI_LOAD_ERROR;
  }


  HBufferImageFree ();
   
  Status = HBufferImageBufferToList ( Buffer, Size );
  FreePool(Buffer);
 
  if ( EFI_ERROR (Status ) ) {
    return Status;
  }

    
  Status = HMemImageSetMemOffsetSize ( Offset, Size );
   
  HBufferImage.DisplayPosition.Row = TEXT_START_ROW;
  HBufferImage.DisplayPosition.Column = HEX_POSITION;
  
  HBufferImage.MousePosition.Row = TEXT_START_ROW;
  HBufferImage.MousePosition.Column = HEX_POSITION;
  
  HBufferImage.LowVisibleRow = 1;
  HBufferImage.HighBits = TRUE;
  

  HBufferImage.BufferPosition.Row = 1;
  HBufferImage.BufferPosition.Column = 1;
  
  if ( !Recover ) {
    Str = PoolPrint(L"%d Lines Read",HBufferImage.NumLines);
    if ( Str == NULL ) {
      return EFI_OUT_OF_RESOURCES;
    }
  
    HMainStatusBarSetStatusString( Str);
    HEditorFreePool ( Str );

    HMainEditor.SelectStart = 0;
    HMainEditor.SelectEnd = 0;

  }
  
  // 
  // has line
  //
  if ( HBufferImage.Lines != NULL  ) {
    HBufferImage.CurrentLine = 
    CR(HBufferImage.ListHead->Flink,HEFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
  } else { 
    // 
    // create a dummy line
    //
    Line = HBufferImageCreateLine ();
    if ( Line == NULL ) {
      return EFI_OUT_OF_RESOURCES ;
    }
    
    HBufferImage.CurrentLine = Line;
  }
    
  HBufferImage.Modified = FALSE;
  HBufferImageNeedRefresh = TRUE;
  HBufferImageOnlyLineNeedRefresh = FALSE;
  HBufferImageMouseNeedRefresh = TRUE;

  return EFI_SUCCESS;



}












EFI_STATUS
HMemImageSave ( 
  IN UINTN Offset,
  IN UINTN Size
)
/*++

Routine Description: 

  Save lines in HBufferImage to disk

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR
  EFI_OUT_OF_RESOURCES

--*/
{

  
  EFI_STATUS      Status;
  VOID            *Buffer;
    
  HEFI_EDITOR_ACTIVE_BUFFER_TYPE BufferTypeBackup;
  
  //
  // not modified, so directly return
  //
  if ( HBufferImage.Modified == FALSE ) {
    return EFI_SUCCESS;
  }
  

  BufferTypeBackup = HBufferImage.BufferType ;
  HBufferImage.BufferType = MEM_BUFFER;
    
  Buffer = AllocatePool ( Size );

  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = HBufferImageListToBuffer ( Buffer, Size );
  if ( EFI_ERROR ( Status )) {
    FreePool(Buffer);
    return Status;
  }

  //
  // write back to memory
  //
  Status = HMemImage.IoFncs->Mem.Write (
                        HMemImage.IoFncs,
                        EfiPciWidthUint8,
                        Offset,
                        Size,
                        Buffer
                        );

  FreePool(Buffer);

  if (EFI_ERROR(Status)) {
    return EFI_LOAD_ERROR;
  }

  //
  // now not modified
  //
  HBufferImage.Modified = FALSE;

  return  EFI_SUCCESS;
}







#endif 
