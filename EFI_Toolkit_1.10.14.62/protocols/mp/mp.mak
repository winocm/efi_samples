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
# Set the base output name and type for this makefile
#

BASE_NAME = mp 

#
# Set entry point
#

IMAGE_ENTRY_POINT = MpProtocolEntry

#
# Globals needed by master.mak
#

TARGET_BS_DRIVER = $(BASE_NAME)
SOURCE_DIR       = $(SDK_INSTALL_DIR)\protocols\$(BASE_NAME)
BUILD_DIR        = $(SDK_BUILD_DIR)\protocols\$(BASE_NAME)

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

!include makefile.hdr
INC = -I . $(INC)

#
# Libraries
#

LIBS = $(LIBS) \
       $(SDK_BUILD_DIR)\lib\libefi\libefi.lib \
       $(SDK_BUILD_DIR)\lib\libc\libc.lib \
       $(SDK_BUILD_DIR)\lib\libefishell\libefishell.lib

#
# Main targets
#

all : dirs $(LIBS) $(OBJECTS)

#
# Program object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\acpi.obj \
    $(BUILD_DIR)\api_deinitpro.obj \
    $(BUILD_DIR)\api_infoproc.obj \
    $(BUILD_DIR)\api_initpro.obj \
    $(BUILD_DIR)\api_numproc.obj \
	$(BUILD_DIR)\api_startproc.obj \
	$(BUILD_DIR)\api_startprocadd.obj \
	$(BUILD_DIR)\api_stopproc.obj \
	$(BUILD_DIR)\comm.obj \
    $(BUILD_DIR)\entry.obj \
    $(BUILD_DIR)\interface.obj \
    $(BUILD_DIR)\proclist.obj \
	$(BUILD_DIR)\stopallproc.obj \
    $(BUILD_DIR)\util.obj \
    $(BUILD_DIR)\wakeup.obj \
    $(BUILD_DIR)\bootrendez.obj \
    $(BUILD_DIR)\chain.obj \
    $(BUILD_DIR)\misc.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\acpi.obj         		: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\api_deinitpro.obj	 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\api_infoproc.obj 		: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\api_initpro.obj 		: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\api_numproc.obj 		: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\api_startproc.obj	 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\api_startprocadd.obj 	: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\api_stopproc.obj 		: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\comm.obj 				: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\entry.obj 				: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\interface.obj 			: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\proclist.obj 			: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\stopallproc.obj 		: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\util.obj 				: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\wakeup.obj 			: $(*B).c $(INC_DEPS)
$(BUILD_DIR)\bootrendez.obj 		: $(*B).s $(INC_DEPS)
$(BUILD_DIR)\chain.obj 				: $(*B).s $(INC_DEPS)
$(BUILD_DIR)\misc.obj 				: $(*B).s $(INC_DEPS)

#
# Handoff to Master.Mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
