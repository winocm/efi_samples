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
    libFileImage.c

  Abstract:
    Functions to deal with file buffer
--*/


#ifndef _LIB_FILE_IMAGE
#define _LIB_FILE_IMAGE

#include "heditor.h"

extern EFI_HANDLE                HImageHandleBackup;
extern HEFI_EDITOR_BUFFER_IMAGE  HBufferImage;

extern BOOLEAN HBufferImageNeedRefresh ;
extern BOOLEAN HBufferImageOnlyLineNeedRefresh ;
extern BOOLEAN HBufferImageMouseNeedRefresh ;

extern HEFI_EDITOR_GLOBAL_EDITOR HMainEditor;

HEFI_EDITOR_FILE_IMAGE HFileImage;
HEFI_EDITOR_FILE_IMAGE HFileImageBackupVar;

//
// for basic initialization of HFileImage
//
HEFI_EDITOR_BUFFER_IMAGE  HFileImageConst = {
  NULL,
  0,
  FALSE
};




EFI_STATUS
HFileImageInit (
  VOID
)
/*++

Routine Description: 

  Initialization function for HFileImage

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR

--*/
{ 

  //
  // basically initialize the HFileImage
  //
  CopyMem (&HFileImage, &HFileImageConst, sizeof(HFileImage));

  CopyMem (
    &HFileImageBackupVar, 
    &HFileImageConst, 
    sizeof(HFileImageBackupVar)
    );

  
  return EFI_SUCCESS; 
}



EFI_STATUS 
HFileImageBackup (
  VOID
) 
/*++

Routine Description: 

  Backup function for HFileImage
  Only a few fields need to be backup. 
  This is for making the file buffer refresh 
  as few as possible.

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  HEditorFreePool ( HFileImageBackupVar.FileName );
  HFileImageBackupVar.FileName = PoolPrint ( L"%s", HFileImage.FileName );
  if ( HFileImageBackupVar.FileName == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}




EFI_STATUS
HFileImageCleanup   (
  VOID
)
/*++

Routine Description: 

  Cleanup function for HFileImage

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  
  HEditorFreePool ( HFileImage.FileName );
  HEditorFreePool ( HFileImageBackupVar.FileName );

  return EFI_SUCCESS;  
}








EFI_STATUS
HFileImageSetFileName (
  IN CHAR16 *Str 
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
  UINTN Size;
  UINTN Index;

  //
  // free the old file name
  //
  HEditorFreePool ( HFileImage.FileName );
  
  Size = StrLen ( Str);

  HFileImage.FileName = AllocatePool ( 2 * (Size + 1 ));
  if ( HFileImage.FileName == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }

  for ( Index = 0 ; Index < Size; Index++) {
    HFileImage.FileName[Index] = Str[Index];
  }
  
  HFileImage.FileName[Size] = L'\0';
  
  return EFI_SUCCESS;
}





EFI_STATUS
HFileImageGetFileInfo ( 
  IN  EFI_FILE_HANDLE Handle,
  IN  CHAR16          *FileName,
  OUT EFI_FILE_INFO   **InfoOut
) 
/*++

Routine Description: 

  Get this file's information

Arguments:  

  Handle -- in NT32 mode Directory handle, in other mode File Handle
  EFI_FILE_INFO ** -- parameter to pass file information out

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES
  EFI_LOAD_ERROR

--*/
{

  VOID       *Info;
  UINTN      Size;
  EFI_STATUS Status;

  Size = SIZE_OF_EFI_FILE_INFO + 1024;
  Info = AllocatePool (Size);
  if (!Info) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // get file information
  //
  Status = Handle -> GetInfo ( Handle,&GenericFileInfo, &Size, Info );
  if ( EFI_ERROR ( Status ) ) {
    return EFI_LOAD_ERROR ;
  }

  *InfoOut = ( EFI_FILE_INFO * ) Info;

  return EFI_SUCCESS;


}








EFI_STATUS
HFileImageRead ( 
  IN CHAR16  *FileName,
  IN BOOLEAN Recover
)
/*++

Routine Description: 

  Read a file from disk into HBufferImage

Arguments:  

  FileName -- filename to read
  Recover -- if is for recover, no information print

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR
  EFI_OUT_OF_RESOURCES
  
--*/
{
  HEFI_EDITOR_LINE     *Line;
  UINT8                *Buffer;
  CHAR16               *UnicodeBuffer;        
  UINTN                FileSize;
  EFI_FILE_HANDLE      Handle;
  BOOLEAN              CreateFile;
  EFI_STATUS           Status;
  EFI_LIST_ENTRY       DirList;
  SHELL_FILE_ARG       *Arg;
  EFI_FILE_INFO        *Info;
  CHAR16               *FileNameTmp;
  

  HEFI_EDITOR_ACTIVE_BUFFER_TYPE    BufferTypeBackup;

  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Vol;
  EFI_FILE_HANDLE                   RootFs;
  EFI_FILE_SYSTEM_INFO              *VolumeInfo;
  UINTN                             Size;
  CHAR16                            *CurDir;

  CHAR16                            *FSMappingPtr;
  CHAR16                            FSMapping[10];
  UINTN                             i, j;
  
  //
  // variable initialization
  //
  Line = NULL;
  FileSize = 0;
  Handle = 0;
  CreateFile = FALSE;

  BufferTypeBackup = HBufferImage.BufferType ;
  HBufferImage.BufferType = FILE_BUFFER;


  //
  // in this function, when you return error ( except EFI_OUT_OF_RESOURCES )
  // you should set status string
  // since this function maybe called before the editorhandleinput loop
  // so any error will cause editor return
  // so if you want to print the error status
  // you should set the status string
  //

  FileNameTmp = PoolPrint(L"%s", FileName );
  if ( FileNameTmp == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // GET CURRENT DIR HANDLE
  //
  InitializeListHead (&DirList);

  //
  // after that filename changed to path
  //
  Status = ShellFileMetaArg(FileNameTmp, &DirList);
  FreePool ( FileNameTmp );
  
  if ( EFI_ERROR ( Status ) ) {
    HMainStatusBarSetStatusString (L"Disk Error");
    return EFI_LOAD_ERROR ;
  }



  if ( DirList.Flink == &DirList ) {
    HMainStatusBarSetStatusString (L"Disk Error");
    return EFI_LOAD_ERROR ;
  }

  Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // since when Status is returned correct, not everything is correct, 
  // so  I have to add these check (-:(((((
  //
  
  if ( Arg == NULL ) {
    HMainStatusBarSetStatusString (L"Disk Error");
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR ;
  }

  if ( Arg->Parent == NULL ) {
    HMainStatusBarSetStatusString (L"Disk Error");
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR ;
  }

  
  Info = Arg->Info;
     
  
  //
  // check whether this file exists
  //
  if ( !Arg->Status ) {
    if ( !Arg->Handle ) {
          Status = Arg->Parent->Open (
          Arg->Parent,
          &Arg->Handle,
          Arg->FileName,
          EFI_FILE_MODE_READ ,
          0
          );
    }

    Handle = Arg->Handle;


    Status = HFileImageGetFileInfo ( Handle, Arg->FileName, &Info );
    if ( EFI_ERROR ( Status ) ) {
      HMainStatusBarSetStatusString (L"Disk Error");
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }


    if ( Info->Attribute & EFI_FILE_DIRECTORY ) {
      HMainStatusBarSetStatusString (L"Directory Can Not Be Edited");
      ShellFreeFileList (&DirList);
	  //-----------2003-08-08----------------
	  FreePool(Info);
	  //-------------------------------------
      return EFI_LOAD_ERROR;
    }
      
    //
    // check if read only
    //
    if ( Info->Attribute & EFI_FILE_READ_ONLY ) {
      HFileImage.ReadOnly = TRUE;
    } else {
      HFileImage.ReadOnly = FALSE;
    }

    //
    // get file size
    //
    FileSize = ( UINTN ) Info->FileSize;
    HFileImage.Size = FileSize;
	//--------------2003-08-08---------------
	FreePool(Info);
	//---------------------------------------


  } else {
    //
    // file not exists, check whether this file can be created to avoid illegal file name error
    //
    Status = Arg->Parent->Open(Arg->Parent,
                               &Arg->Handle,Arg->FileName,
                               FILE_CREATE, 
                               0
                               );
    if (EFI_ERROR(Status)) {
      ShellFreeFileList (&DirList);
      HMainStatusBarSetStatusString (L"Illegal File Name");
      return EFI_LOAD_ERROR;
    } else {
      Status = Arg->Handle->Delete (Arg->Handle);
      if (Status == EFI_WARN_DELETE_FAILURE) {
        Status = EFI_ACCESS_DENIED;
      }      
      Arg->Handle = NULL;
      if (EFI_ERROR(Status)) {
        ShellFreeFileList (&DirList);
        HMainStatusBarSetStatusString (L"Delete File Failed");
        return EFI_LOAD_ERROR;
      }
    }  
      //
      // file not exists, so set CreateFile to TRUE
      //
      CreateFile = TRUE;
      HFileImage.ReadOnly = FALSE;
      HFileImage.Size = 0;

      //
      // all the check ends
      // so now begin to set file name, free lines
      //
      if ( BufferTypeBackup != FILE_BUFFER || 
        ( StriCmp ( FileName, HFileImage.FileName ) != 0 )) {
        HFileImageSetFileName  ( FileName );
      }
  
      //
      // free the old lines
      //
      HBufferImageFree ();

  }

  //
  // file exists
  //
  if ( CreateFile == FALSE ) {
    
    //
    // allocate buffer to read file
    //
    Buffer = (UINT8*) AllocatePool(FileSize);
    if ( Buffer == NULL ) {
      ShellFreeFileList (&DirList);
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // read file into Buffer
    //
    Status = Handle->Read(Handle,&FileSize,Buffer);
    if ( EFI_ERROR ( Status )) {
      HMainStatusBarSetStatusString(L"Read File Failed");
      HEditorFreePool ( Buffer );
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }
    
    
    //
    // nothing in this file
    //
    if (FileSize == 0) {
      HEditorFreePool (Buffer);
      
      goto Done;
    }

 

    // 
    // all the check ends
    // so now begin to set file name, free lines
    //
    if ( BufferTypeBackup != FILE_BUFFER ||  
       ( StrCmp ( FileName, HFileImage.FileName ) != 0 )) {
      HFileImageSetFileName  ( FileName );
    }
  
    //
    // free the old lines
    //
    HBufferImageFree ();

    Status = HBufferImageBufferToList ( Buffer, FileSize );
    HEditorFreePool (Buffer);
    if ( EFI_ERROR (Status ) ) {
      return Status;
    }

  }   //end of if CreateFile

Done:

  
  HBufferImage.DisplayPosition.Row = TEXT_START_ROW;
  HBufferImage.DisplayPosition.Column = HEX_POSITION;

  HBufferImage.MousePosition.Row = TEXT_START_ROW;
  HBufferImage.MousePosition.Column = HEX_POSITION;
  
  HBufferImage.LowVisibleRow = 1;
  HBufferImage.HighBits = TRUE;
  

  HBufferImage.BufferPosition.Row = 1;
  HBufferImage.BufferPosition.Column = 1;

  if ( !Recover ) {
    UnicodeBuffer = PoolPrint(L"%d Lines Read",HBufferImage.NumLines);
    if ( UnicodeBuffer == NULL ) {
      ShellFreeFileList (&DirList);
      return EFI_OUT_OF_RESOURCES;
    }
  
    HMainStatusBarSetStatusString(UnicodeBuffer);
    HEditorFreePool ( UnicodeBuffer );
    
    HMainEditor.SelectStart = 0;
    HMainEditor.SelectEnd = 0;


    //
    // get vol's attribute, to find whether the vol is read-only
    // check whether we have fs?: in filename
    i = 0;
    FSMappingPtr = NULL;
    while ( FileName[i] != 0 ) {
      if ( FileName[i] == L':' ) {
        FSMappingPtr = &FileName[i];
        break;
      }

      i++;
    }

    if ( FSMappingPtr == NULL ) {
      CurDir = ShellCurDir(NULL);
    } else {
      i = 0;
      j = 0;
      while ( FileName[i] != 0 ) {
        if ( FileName[i] == L':' ) {
            break;
        }

          FSMapping[j++] = FileName[i];

        i++;
      }

      FSMapping[j] = 0;
      CurDir = ShellCurDir(FSMapping);
    }


    if (CurDir) {
      for (i=0; i < StrLen(CurDir) && CurDir[i] != ':'; i++) {;}
      CurDir[i] = 0;
      DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (CurDir);
      //-------------2003-08-08---------------------------
      FreePool(CurDir);
      //--------------------------------------------------
    } else {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if (DevicePath == NULL) {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }
    
    Status = LibDevicePathToInterface (
                &gEfiSimpleFileSystemProtocolGuid, 
                DevicePath, 
                (VOID **)&Vol
             );
    if (EFI_ERROR(Status)) {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    Status = Vol->OpenVolume (Vol, &RootFs);
    if (EFI_ERROR(Status)) {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    //
    // Get volume information of file system 
    //
    Size = SIZE_OF_EFI_FILE_SYSTEM_INFO + 100;
    VolumeInfo = (EFI_FILE_SYSTEM_INFO *)AllocatePool(Size);
    Status = RootFs->GetInfo(RootFs,&gEfiFileSystemInfoGuid,&Size,VolumeInfo);
    if (EFI_ERROR(Status)) {
      RootFs -> Close ( RootFs );
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if (VolumeInfo->ReadOnly) {
      HMainStatusBarSetStatusString(L"WARNING: Volume Read Only");
    }
    
    RootFs -> Close ( RootFs );    
    //-------------2003-08-08---------------------------
    FreePool(VolumeInfo);
    //--------------------------------------------------
    
  }
  
  // 
  // has line
  //
  if ( HBufferImage.Lines != 0  ) {
    HBufferImage.CurrentLine = 
    CR(HBufferImage.ListHead->Flink,HEFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
  } else { 
    // 
    // create a dummy line
    //
    Line = HBufferImageCreateLine ();
    if ( Line == NULL ) {
      ShellFreeFileList (&DirList);
      return EFI_OUT_OF_RESOURCES ;
    }
    
    HBufferImage.CurrentLine = Line;
  }
    
  HBufferImage.Modified = FALSE;
  HBufferImageNeedRefresh = TRUE;
  HBufferImageOnlyLineNeedRefresh = FALSE;
  HBufferImageMouseNeedRefresh = TRUE;

  ShellFreeFileList (&DirList);

  return EFI_SUCCESS;
}












EFI_STATUS
HFileImageSave ( 
  IN CHAR16 *FileName
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

  EFI_LIST_ENTRY      *Link;
  HEFI_EDITOR_LINE    *Line;
  CHAR16              *Str;
  EFI_STATUS          Status;
  UINTN               NumLines;
  EFI_FILE_HANDLE     FileHandle;
  EFI_LIST_ENTRY      DirList;
  SHELL_FILE_ARG      *Arg;
  EFI_FILE_INFO       *Info;
  CHAR16              *FileNameTmp;  
  UINT64              Attribute;
  UINTN               TotalSize;
  UINT8               *Buffer;
  UINT8               *Ptr;

  HEFI_EDITOR_ACTIVE_BUFFER_TYPE BufferTypeBackup;
  

  BufferTypeBackup = HBufferImage.BufferType ;
  HBufferImage.BufferType = FILE_BUFFER;

  
  FileNameTmp = PoolPrint(L"%s", FileName );
  if ( FileNameTmp == NULL ) {
    return EFI_OUT_OF_RESOURCES;
  }

  // 
  // get file handle
  //

  //
  // GET CURRENT DIR HANDLE
  //
  InitializeListHead (&DirList);
  Status = ShellFileMetaArg(FileNameTmp, &DirList);
  FreePool ( FileNameTmp );
  
  if ( EFI_ERROR ( Status ) ) {
    return EFI_LOAD_ERROR ;
  }


  if ( DirList.Flink == &DirList ) {
    return EFI_LOAD_ERROR ;
  }

  Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
  
  //
  // since when Status is returned correct, not everything is correct, 
  // so  I have to add these check (-:(((((
  //
  if ( Arg == NULL ) {
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR ;
  }

  if ( Arg->Parent == NULL ) {
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR ;
  }
  
  

  // 
  //if is the old file
  //

  if ( StrCmp ( FileName, HFileImage.FileName ) == 0 ) {


    //
    // check whether file exists on disk
    //
    if ( !Arg->Status ) {
    
      //
      // current file exists on disk
      // so if not modified, then not save
      //
      if ( HBufferImage.Modified == FALSE ) {
        ShellFreeFileList (&DirList);
        return EFI_SUCCESS;
      }
  
      //
      // if file is read-only, set error
      //
      if ( HFileImage.ReadOnly == TRUE ) {
        HMainStatusBarSetStatusString ( L"Read Only File Can Not Be Saved");
        ShellFreeFileList (&DirList);
        return EFI_SUCCESS;
      }
    } // end of !Arg->Status
  }

  
  Info = Arg -> Info;
    
  Attribute = 0;

  //
  // open current file
  //
  if ( !Arg->Status ) {
    if ( ! Arg->Handle ) {
      Status = Arg->Parent->Open (
          Arg->Parent,
          &Arg->Handle,
          Arg->FileName,
          EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
          0
          );
    }

    FileHandle = Arg->Handle;

    Status = HFileImageGetFileInfo ( FileHandle, Arg->FileName, &Info );
    if ( EFI_ERROR ( Status ) ) {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }
    
    if ( Info->Attribute & EFI_FILE_DIRECTORY ) {
      HMainStatusBarSetStatusString (L"Directory Can Not Be Saved");
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if ( Info->Attribute & EFI_FILE_READ_ONLY ) {
      Attribute = Info->Attribute - EFI_FILE_READ_ONLY;
    } else  {
      Attribute = Info ->Attribute;
    }

    // 
    //if file exits, so delete it
    //
    Status = FileHandle->Delete (FileHandle);
    if (Status == EFI_WARN_DELETE_FAILURE) {
      Status = EFI_ACCESS_DENIED;
    }    
    Arg->Handle = NULL;
    if (EFI_ERROR(Status)) {
      ShellFreeFileList (&DirList);

      // 
      // because in nt32 mode, when across file systems, 
      // it will generate exception
      // so comment it

      //          FileHandle -> Close ( FileHandle );

      HMainStatusBarSetStatusString (L"Write File Failed");
      return EFI_LOAD_ERROR;
    }
  }

  // 
  //create this file
  //
  Status = Arg->Parent->Open(
             Arg->Parent,
             &Arg->Handle,
             Arg->FileName,
             FILE_CREATE, 
             Attribute
            );
  if (EFI_ERROR(Status)) {
    ShellFreeFileList (&DirList);

    HMainStatusBarSetStatusString (L"Create File Failed");

    return EFI_LOAD_ERROR;
  }

  FileHandle = Arg->Handle;
  
  // 
  //write all the lines back to disk
  //
  NumLines = 0;
  TotalSize = 0;
  for (Link = HBufferImage.ListHead->Flink;
    Link != HBufferImage.ListHead; Link = Link->Flink) {
    Line = CR(Link,HEFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);

    if ( Line -> Size != 0 ) {
      TotalSize += Line -> Size;
    } // end of if Line -> Size != 0

    NumLines++;
  } //end of for Link

  Buffer = AllocatePool ( TotalSize );
  if ( Buffer == NULL ) {
    FileHandle->Delete (FileHandle);
    Arg->Handle = NULL;
    ShellFreeFileList (&DirList);

    return EFI_OUT_OF_RESOURCES;
  }
  
  Ptr = Buffer;
  for (Link = HBufferImage.ListHead->Flink;
       Link != HBufferImage.ListHead; Link = Link->Flink) {
    Line = CR(Link,HEFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);

    if ( Line -> Size != 0 ) {
      CopyMem ( Ptr, Line -> Buffer, Line -> Size );
      Ptr += Line -> Size;
    } // end of if Line -> Size != 0
  } //end of for Link

  Status = FileHandle->Write(FileHandle,&TotalSize,Buffer);
  FreePool ( Buffer );
  if (EFI_ERROR(Status)) {
    FileHandle->Delete (FileHandle);
    Arg->Handle = NULL;
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR;
  }


  HBufferImage.Modified = FALSE;

  //
  // set status string
  //
  Str = PoolPrint(L"%d Lines Wrote",NumLines);
  HMainStatusBarSetStatusString(Str);
  HEditorFreePool ( Str );
   
  //
  // now everything is ready , you can set the new file name to filebuffer
  //
  if ( BufferTypeBackup != FILE_BUFFER || 
       StriCmp ( FileName, HFileImage.FileName ) != 0 ) {
    //
    // not the same
    //
    HFileImageSetFileName ( FileName );
    if ( HFileImage.FileName == NULL ) {
      FileHandle -> Delete ( FileHandle );
      Arg->Handle = NULL;
      ShellFreeFileList (&DirList);

      return EFI_OUT_OF_RESOURCES;
    }
  }
      
  HFileImage.ReadOnly = FALSE;


  ShellFreeFileList (&DirList);
  
  return  EFI_SUCCESS;
}







#endif 
