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
# Defining DYNAMIC_PYTHON_MODULES causes many of the lesser used
#  modules to be built as EFI drivers that are loaded on demand
#  rather than being statically linked into the executable.
#

DYNAMIC_PYTHON_MODULES = 1

#
# Set the base output name
#

BASE_NAME = Modules

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\cmds\python\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\cmds\python\$(BASE_NAME)

LIBS = $(LIBS) \
       $(SDK_BUILD_DIR)\lib\libc\libc.lib \
       $(SDK_BUILD_DIR)\lib\libefi\libefi.lib \
       $(SDK_BUILD_DIR)\lib\libm\libm.lib \
       $(SDK_BUILD_DIR)\lib\libdb\libdb.lib \
       $(SDK_BUILD_DIR)\lib\libz\libz.lib \
       $(SDK_BUILD_DIR)\lib\libsocket\libsocket.lib \
       $(BUILD_DIR)\EfiInterfaceLib\EfiInterfaceLib.lib

#
# Rule for how to build a dynamically loaded module
#

{$(BUILD_DIR)}.obj{$(BIN_DIR)}.pym:
    $(LINK) $(L_FLAGS) $(MODULE_LFLAGS)\
            /ENTRY:EfiPythonModuleEntry $** \
            /OUT:$(@R).dll
    $(FWIMAGE) bsdrv $(@R).dll $(@R).pym

#
# Additional compile flags
#

C_FLAGS = /D __STDC__ $(C_FLAGS) 
!IFDEF DYNAMIC_PYTHON_MODULES
C_FLAGS = /D EFI_LOADABLE_MODULE $(C_FLAGS)
!ENDIF

#
# The dynamically loaded modules
#

!IFDEF DYNAMIC_PYTHON_MODULES
DYNAMIC_MODULES = $(BIN_DIR)\arraymodule.pym \
		  $(BIN_DIR)\bsddbmodule.pym \
          $(BIN_DIR)\cmathmodule.pym \
		  $(BIN_DIR)\cpickle.pym \
		  $(BIN_DIR)\cstringio.pym \
		  $(BIN_DIR)\dbmmodule.pym \
		  $(BIN_DIR)\errnomodule.pym \
		  $(BIN_DIR)\mathmodule.pym \
		  $(BIN_DIR)\md5module.pym  \
		  $(BIN_DIR)\newmodule.pym \
		  $(BIN_DIR)\operator.pym \
		  $(BIN_DIR)\parsermodule.pym \
		  $(BIN_DIR)\pcremodule.pym \
		  $(BIN_DIR)\regexmodule.pym \
		  $(BIN_DIR)\rotormodule.pym \
		  $(BIN_DIR)\selectmodule.pym \
		  $(BIN_DIR)\shamodule.pym \
		  $(BIN_DIR)\socketmodule.pym \
		  $(BIN_DIR)\structmodule.pym \
		  $(BIN_DIR)\timemodule.pym \
		  $(BIN_DIR)\timingmodule.pym \
		  $(BIN_DIR)\zlibmodule.pym \
!ENDIF

#
# Default target
#

all : dirs $(OBJECTS) $(DYNAMIC_MODULES)

#
# Dynamic module dependencies
#

!IFDEF DYNAMIC_PYTHON_MODULES

$(BIN_DIR)\arraymodule.pym  : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\bsddbmodule.pym  : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\cmathmodule.pym  : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\cpickle.pym      : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\cstringio.pym    : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\dbmmodule.pym    : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\errnomodule.pym  : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\mathmodule.pym   : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\md5module.pym    : $(BUILD_DIR)\$(*B).obj $(BUILD_DIR)\md5c.obj $(LIBS)
$(BIN_DIR)\newmodule.pym    : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\operator.pym     : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\parsermodule.pym : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\pcremodule.pym   : $(BUILD_DIR)\$(*B).obj $(BUILD_DIR)\pypcre.obj $(LIBS)
$(BIN_DIR)\regexmodule.pym  : $(BUILD_DIR)\$(*B).obj $(BUILD_DIR)\regexpr.obj $(LIBS)
$(BIN_DIR)\rotormodule.pym  : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\selectmodule.pym : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\shamodule.pym    : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\socketmodule.pym : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\structmodule.pym : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\timemodule.pym   : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\timingmodule.pym : $(BUILD_DIR)\$(*B).obj $(LIBS)
$(BIN_DIR)\zlibmodule.pym   : $(BUILD_DIR)\$(*B).obj $(LIBS)

!endif

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

!include $(SDK_INSTALL_DIR)\include\efishell\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\efishell $(INC)

!include ..\include\makefile.hdr
INC = -I ..\include $(INC)

#
# Local include dependencies
#

INC_DEPS = $(INC_DEPS) \
	md5.h \
	regexpr.h \
	timing.h \
	pcre.h \
	pcre-int.h \

#
# Local libraries dependencies
#

$(BUILD_DIR)\EfiInterfaceLib\EfiInterfaceLib.lib : 
	cd EfiInterfaceLib
	nmake -f EfiInterfaceLib.mak all
	cd $(SOURCE_DIR)

#
# Library object files
#

!IFNDEF DYNAMIC_PYTHON_MODULES

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\config.obj \
    $(BUILD_DIR)\getpath.obj \
    $(BUILD_DIR)\main.obj \
    $(BUILD_DIR)\getbuildinfo.obj \
    $(BUILD_DIR)\python.obj \
    $(BUILD_DIR)\arraymodule.obj \
    $(BUILD_DIR)\cmathmodule.obj \
    $(BUILD_DIR)\efimodule.obj \
    $(BUILD_DIR)\errnomodule.obj \
    $(BUILD_DIR)\mathmodule.obj \
    $(BUILD_DIR)\md5module.obj \
    $(BUILD_DIR)\md5c.obj \
    $(BUILD_DIR)\newmodule.obj \
    $(BUILD_DIR)\operator.obj \
    $(BUILD_DIR)\parsermodule.obj \
    $(BUILD_DIR)\pcremodule.obj \
    $(BUILD_DIR)\pypcre.obj \
    $(BUILD_DIR)\regexmodule.obj \
    $(BUILD_DIR)\regexpr.obj \
    $(BUILD_DIR)\rotormodule.obj \
    $(BUILD_DIR)\selectmodule.obj \
    $(BUILD_DIR)\shamodule.obj \
    $(BUILD_DIR)\socketmodule.obj \
    $(BUILD_DIR)\stropmodule.obj \
    $(BUILD_DIR)\structmodule.obj \
    $(BUILD_DIR)\timemodule.obj \
    $(BUILD_DIR)\timingmodule.obj \
    $(BUILD_DIR)\cstringio.obj \
    $(BUILD_DIR)\cpickle.obj \
    $(BUILD_DIR)\bsddbmodule.obj \
    $(BUILD_DIR)\dbmmodule.obj \
    $(BUILD_DIR)\zlibmodule.obj \

!ELSE

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\config.obj \
    $(BUILD_DIR)\getpath.obj \
    $(BUILD_DIR)\main.obj \
    $(BUILD_DIR)\getbuildinfo.obj \
    $(BUILD_DIR)\python.obj \
    $(BUILD_DIR)\efimodule.obj \
    $(BUILD_DIR)\stropmodule.obj \

!ENDIF

#
# Source file dependencies
#

$(BUILD_DIR)\config.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getpath.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\main.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getbuildinfo.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\python.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\arraymodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\cmathmodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\efimodule.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\errnomodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\mathmodule.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\md5module.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\md5c.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\newmodule.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\operator.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\parsermodule.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pcremodule.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pypcre.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\regexmodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\regexpr.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\rotormodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\selectmodule.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\shamodule.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\socketmodule.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\stropmodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\structmodule.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\timemodule.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\timingmodule.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\cstringio.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\cpickle.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\bsddbmodule.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\dbmmodule.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\zlibmodule.obj   : $(*B).c $(INC_DEPS)

#
# Because the target of this makefile is a library, we can't use the
# library targets in master.env
#

$(SDK_BUILD_DIR)\lib\libc\libc.lib : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)\lib\libc
	nmake -f libc.mak all
	cd $(SOURCE_DIR)

$(SDK_BUILD_DIR)\lib\libsocket\libsocket.lib : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)\lib\libsocket
	nmake -f libsocket.mak all
	cd $(SOURCE_DIR)

$(SDK_BUILD_DIR)\lib\libm\libm.lib : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)\lib\libm
	nmake -f libm.mak all
	cd $(SOURCE_DIR)

$(SDK_BUILD_DIR)\lib\libdb\libdb.lib : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)\lib\libdb
	nmake -f libdb.mak all
	cd $(SOURCE_DIR)

$(SDK_BUILD_DIR)\lib\libz\libz.lib : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)\lib\libz
	nmake -f libz.mak all
	cd $(SOURCE_DIR)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
