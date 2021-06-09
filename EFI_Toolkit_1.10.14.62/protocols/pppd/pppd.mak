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

BASE_NAME = pppd

#
# Set entry point
#

IMAGE_ENTRY_POINT = EfiHook

#
# Globals needed by master.mak
#

TARGET_BS_DRIVER = $(BASE_NAME)
SOURCE_DIR       = $(SDK_INSTALL_DIR)\protocols\$(BASE_NAME)
BUILD_DIR        = $(SDK_BUILD_DIR)\protocols\$(BASE_NAME)

#
# Additional compiler flags
#

LIBMD_SUPPORT = NO

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

#
# Libraries
#

LIBS = $(LIBS) \
       $(SDK_BUILD_DIR)\lib\libc\libc.lib \
       $(SDK_BUILD_DIR)\lib\libsocket\libsocket.lib \
       $(SDK_BUILD_DIR)\lib\libtty\libtty.lib \
!IF "$(LIBMD_SUPPORT)" == "YES"
       $(SDK_BUILD_DIR)\lib\libmd\libmd.lib \
       $(SDK_BUILD_DIR)\lib\libcrypt\libcrypt.lib \
!ENDIF

#
# Additional compile flags
#

C_FLAGS = /D __STDC__ /D __FreeBSD__ /D NO_DRAND48 $(C_FLAGS)

#
# Adjust flags for libmd support
#
!IF "$(LIBMD_SUPPORT)" == "YES"
C_FLAGS = /D LIBMD_SUPPORT $(C_FLAGS)
!ENDIF

#
# Main targets
#

all : dirs $(LIBS) $(OBJECTS)

#
# Program object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\main.obj \
    $(BUILD_DIR)\sys-bsd.obj \
    $(BUILD_DIR)\fsm.obj \
    $(BUILD_DIR)\ipcp.obj \
    $(BUILD_DIR)\lcp.obj \
    $(BUILD_DIR)\magic.obj \
    $(BUILD_DIR)\cbcp.obj \
    $(BUILD_DIR)\ccp.obj \
    $(BUILD_DIR)\upap.obj \
    $(BUILD_DIR)\options.obj \
    $(BUILD_DIR)\auth.obj \
    $(BUILD_DIR)\stubs.obj \
    $(BUILD_DIR)\EfiHook.obj \
!IF "$(LIBMD_SUPPORT)" == "YES"
    $(BUILD_DIR)\chap.obj \
!ENDIF


#
# Source file dependencies
#

$(BUILD_DIR)\main.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\sys-bsd.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\fsm.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ipcp.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\lcp.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\magic.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\auth.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\cbcp.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ccp.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\chap.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\upap.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\options.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\stubs.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\EfiHook.obj : $(*B).c $(INC_DEPS)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
