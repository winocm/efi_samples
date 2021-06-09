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

BASE_NAME = Python

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\cmds\python\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\cmds\python\$(BASE_NAME)

#
# Additional compile flags
#

C_FLAGS = /D __STDC__ $(C_FLAGS)

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd  $(INC)

!include ..\include\makefile.hdr
INC = -I ..\include $(INC)

#
# Default target
#

all : dirs $(OBJECTS)

#
#  Local include dependencies
#

INC_DEPS = $(INC_DEPS) \
	importdl.h \

#
#  Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\bltinmodule.obj \
    $(BUILD_DIR)\ceval.obj \
    $(BUILD_DIR)\compile.obj \
    $(BUILD_DIR)\errors.obj \
    $(BUILD_DIR)\frozen.obj \
    $(BUILD_DIR)\frozenmain.obj \
    $(BUILD_DIR)\getargs.obj \
    $(BUILD_DIR)\getcompiler.obj \
    $(BUILD_DIR)\getcopyright.obj \
    $(BUILD_DIR)\getmtime.obj \
    $(BUILD_DIR)\getplatform.obj \
    $(BUILD_DIR)\getversion.obj \
    $(BUILD_DIR)\graminit.obj \
    $(BUILD_DIR)\import.obj \
    $(BUILD_DIR)\importdl.obj \
    $(BUILD_DIR)\marshal.obj \
    $(BUILD_DIR)\modsupport.obj \
    $(BUILD_DIR)\mystrtoul.obj \
    $(BUILD_DIR)\pyfpe.obj \
    $(BUILD_DIR)\pystate.obj \
    $(BUILD_DIR)\pythonrun.obj \
    $(BUILD_DIR)\structmember.obj \
    $(BUILD_DIR)\sysmodule.obj \
    $(BUILD_DIR)\traceback.obj \
    $(BUILD_DIR)\sigcheck.obj \
    $(BUILD_DIR)\EfiInterface.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\bltinmodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ceval.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\compile.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\errors.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\frozen.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\frozenmain.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getargs.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getcompiler.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getcopyright.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getmtime.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getplatform.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getversion.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\graminit.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\import.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\importdl.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\marshal.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\modsupport.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\mystrtoul.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pyfpe.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pystate.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pythonrun.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\structmember.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\sysmodule.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\traceback.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\sigcheck.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\EfiInterface.obj : $(*B).c $(INC_DEPS)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
