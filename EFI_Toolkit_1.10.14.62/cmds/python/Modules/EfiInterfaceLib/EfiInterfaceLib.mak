#
# Copyright (c) 1999, 2000
# Intel Corporation.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. All advertising materials mentioning features or use of this software must
#    display the following acknowledgement:
# 
#    This product includes software developed by Intel Corporation and its
#    contributors.
# 
# 4. Neither the name of Intel Corporation or its contributors may be used to
#    endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

#
# Include sdk.env environment
#

!include $(SDK_INSTALL_DIR)\build\$(SDK_BUILD_ENV)\sdk.env

#
# Set the base output name
#

BASE_NAME = EfiInterfaceLib

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\cmds\python\Modules\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\cmds\python\Modules\$(BASE_NAME)

#
# Additional compile flags
#

C_FLAGS = /D __STDC__ /D EFI_LOADABLE_MODULE $(C_FLAGS)

#
# Default target
#

all : dirs $(OBJECTS)
    
#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

!include ..\..\include\makefile.hdr
INC = -I ..\..\include $(INC)

#
# Local include dependencies
#

INC_DEPS = $(INC_DEPS) \

#
# Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\efi_init.obj \
    $(BUILD_DIR)\efi_interface.obj \
    $(BUILD_DIR)\modsupport.obj \
    $(BUILD_DIR)\pyerrors.obj \
    $(BUILD_DIR)\object.obj \
    $(BUILD_DIR)\objimpl.obj \
    $(BUILD_DIR)\dictobject.obj \
    $(BUILD_DIR)\methodobject.obj \
    $(BUILD_DIR)\intobject.obj \
    $(BUILD_DIR)\listobject.obj \
    $(BUILD_DIR)\moduleobject.obj \
    $(BUILD_DIR)\complexobject.obj \
    $(BUILD_DIR)\floatobject.obj \
    $(BUILD_DIR)\stringobject.obj \
    $(BUILD_DIR)\longobject.obj \
    $(BUILD_DIR)\abstract.obj \
    $(BUILD_DIR)\tupleobject.obj \
    $(BUILD_DIR)\classobject.obj \
    $(BUILD_DIR)\funcobject.obj \
    $(BUILD_DIR)\fileobject.obj \
    $(BUILD_DIR)\sysmodule.obj \
    $(BUILD_DIR)\ceval.obj \
    $(BUILD_DIR)\import.obj \
    $(BUILD_DIR)\pythonrun.obj \
    $(BUILD_DIR)\pydebug.obj \
    $(BUILD_DIR)\cobject.obj \
    $(BUILD_DIR)\compile.obj \
    $(BUILD_DIR)\node.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\efi_init.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\efi_interface.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\modsupport.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pyerrors.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\object.obj  	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\objimpl.obj  	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\dictobject.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\methodobject.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\intobject.obj  	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\listobject.obj   	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\moduleobject.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\complexobject.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\floatobject.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\stringobject.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\longobject.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\abstract.obj   	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tupleobject.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\classobject.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\funcobject.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\fileobject.obj  	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\sysmodule.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ceval.obj    	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\import.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pythonrun.obj  	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pydebug.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\cobject.obj   	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\compile.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\node.obj    	: $(*B).c $(INC_DEPS)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
