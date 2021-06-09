#
# Copyright (c) 2000
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

BASE_NAME = dhclient

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
# Additional compile flags
#

C_FLAGS = $(C_FLAGS) -D __FreeBSD__ -D __STDC__

#
# Uncomment the next line if the DHCP client should support
# maintaining lease files.  This allows the client to request
# the same IP address it used the last time it ran.
#
C_FLAGS = $(C_FLAGS) -D LEASE_FILE_SUPPORT

#
# This next line is for debug.  It will print to the screen when
# leases are renewed or changed.
#
#C_FLAGS = $(C_FLAGS) -D BACKGROUND_DISPLAY

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

INC = -I . \
      -I .\includes $(INC)

#
# Libraries
#

LIBS = $(LIBS) \
       $(SDK_BUILD_DIR)\lib\libc\libc.lib \
       $(SDK_BUILD_DIR)\lib\libsocket\libsocket.lib \

all : dirs $(LIBS) $(OBJECTS)

#
# Local include dependencies
#

INC_DEPS = $(INC_DEPS) \
    includes/cdefs.h \
    includes/dhcp.h \
    includes/dhctoken.h \
    includes/hash.h \
    includes/inet.h \
    includes/osdep.h \
    includes/site.h \
    includes/sysconf.h \
    includes/tree.h \
    includes/cf/freebsd.h \

#
# Program object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\EfiHook.obj \
    $(BUILD_DIR)\dhclient.obj \
    $(BUILD_DIR)\clparse.obj \
	$(BUILD_DIR)\alloc.obj \
	$(BUILD_DIR)\conflex.obj \
	$(BUILD_DIR)\convert.obj \
	$(BUILD_DIR)\dispatch.obj \
	$(BUILD_DIR)\errwarn.obj \
	$(BUILD_DIR)\hash.obj \
	$(BUILD_DIR)\inet.obj \
	$(BUILD_DIR)\memory.obj \
	$(BUILD_DIR)\options.obj \
	$(BUILD_DIR)\packet.obj \
	$(BUILD_DIR)\parse.obj \
	$(BUILD_DIR)\print.obj \
	$(BUILD_DIR)\socket.obj \
	$(BUILD_DIR)\tables.obj \
	$(BUILD_DIR)\tree.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\EfiHook.obj  : $(*B).c        $(INC_DEPS)
$(BUILD_DIR)\dhclient.obj : client\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\clparse.obj  : client\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\alloc.obj    : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\conflex.obj  : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\convert.obj  : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\dispatch.obj : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\errwarn.obj  : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\hash.obj     : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet.obj     : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\memory.obj   : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\options.obj  : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\packet.obj   : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\parse.obj    : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\print.obj    : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\socket.obj   : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\tables.obj   : common\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\tree.obj     : common\$(*B).c $(INC_DEPS)

#
# Because dhclient compiles out of sub-directories, we need some
# of our own inference rules.  $(CC_LINE) is defined in master.mak
#

{client}.c{$(BUILD_DIR)}.obj: ; $(CC_LINE)
{common}.c{$(BUILD_DIR)}.obj: ; $(CC_LINE)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
