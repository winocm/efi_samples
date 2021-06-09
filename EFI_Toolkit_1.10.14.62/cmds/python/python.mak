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

BASE_NAME = python

#
# Set entry point
#

!IFDEF OLD_SHELL
IMAGE_ENTRY_POINT = _LIBC_Start_Shellapp_A
!ELSE
IMAGE_ENTRY_POINT = _LIBC_Start_A
!ENDIF

#
# Globals needed by master.mak
#

TARGET_APP = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\cmds\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\cmds\$(BASE_NAME)

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
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

!include include\makefile.hdr
INC = -I include $(INC)

#
# Local libraries
#

LIBS = $(BUILD_DIR)\Parser\Parser.lib \
	   $(BUILD_DIR)\Objects\Objects.lib \
	   $(BUILD_DIR)\Python\Python.lib \
	   $(BUILD_DIR)\Modules\Modules.lib \
	   $(LIBS)

#
# Toolkit Libraries
#

LIBS = $(LIBS) \
       $(SDK_BUILD_DIR)\lib\libc\libc.lib \
       $(SDK_BUILD_DIR)\lib\libm\libm.lib \
       $(SDK_BUILD_DIR)\lib\libsocket\libsocket.lib \
       $(SDK_BUILD_DIR)\lib\libdb\libdb.lib \
       $(SDK_BUILD_DIR)\lib\libz\libz.lib \
       $(SDK_BUILD_DIR)\lib\libefi\libefi.lib \
	   $(SDK_BUILD_DIR)\lib\libtty\libtty.lib \
!IFDEF OLD_SHELL
       $(SDK_BUILD_DIR)\lib\libefishell\libefishell.lib \
!ENDIF

all : dirs $(LIBS) $(OBJECTS)

floppy : 
	copy $(BIN_DIR)\Python.efi a:

#
# Local libraries dependencies
#

$(BUILD_DIR)\Modules\Modules.lib : 
	cd Modules
	nmake -f Modules.mak all
	cd $(SOURCE_DIR)

$(BUILD_DIR)\Objects\Objects.lib :
	cd Objects
	nmake -f Objects.mak all
	cd $(SOURCE_DIR)

$(BUILD_DIR)\Parser\Parser.lib :
	cd Parser
	nmake -f Parser.mak all
	cd $(SOURCE_DIR)

$(BUILD_DIR)\Python\Python.lib :
	cd Python
	nmake -f Python.mak all
	cd $(SOURCE_DIR)

#
#  Program object file 
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\EfiMain.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\EfiMain.obj : $(*B).c $(INC_DEPS)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
