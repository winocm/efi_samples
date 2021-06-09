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
    HEditor.h

  Abstract:
    Main include file for hex editor

--*/


#ifndef _HEDITOR_H_
#define _HEDITOR_H_


#include "heditortype.h"


#include "libeditor.h"

#include "libbufferimage.h"
#include "libfileimage.h"
#include "libdiskimage.h"
#include "libmemimage.h"

#include "libtitlebar.h"
#include "libstatusbar.h"
#include "libinputbar.h"
#include "libmenubar.h"

#include "libmisc.h"

#include "libclipboard.h"

extern  HEFI_EDITOR_GLOBAL_EDITOR    HMainEditor;
extern  BOOLEAN                      HEditorFirst;
extern  BOOLEAN                      HEditorExit;

#endif  // _HEDITOR_H
