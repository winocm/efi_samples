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
    Editor.h

  Abstract:
    Main include file for text editor

--*/


#ifndef _EDITOR_H_
#define _EDITOR_H_


#include "editortype.h"


#include "libeditor.h"
#include "libfilebuffer.h"
#include "libtitlebar.h"
#include "libstatusbar.h"
#include "libinputbar.h"
#include "libmenubar.h"
#include "libmisc.h"

extern  EFI_EDITOR_GLOBAL_EDITOR    MainEditor;
extern  BOOLEAN                     EditorFirst;
extern  BOOLEAN                     EditorExit;

#endif  // _EDITOR_H
