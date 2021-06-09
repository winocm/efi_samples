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

BASE_NAME = libm

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\lib\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\lib\$(BASE_NAME)

#
# Additional compiler flags
#

C_FLAGS = /D __STDC__ $(C_FLAGS)

#
# Include paths
#

!include $(SOURCE_DIR)\.\makefile.hdr

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

!include $(SDK_INSTALL_DIR)\lib\libm\common_source\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\lib\libm\common_source $(INC)

#
# Default targets
#

all : sub_dirs $(OBJECTS)

sub_dirs: $(BUILD_DIR)\efi \
          $(BUILD_DIR)\common \
          $(BUILD_DIR)\common_source \
          $(BUILD_DIR)\ieee \
          $(BUILD_DIR)\gen \

#
# Sub-directory targets
#

$(BUILD_DIR)\efi           : ; - md $(BUILD_DIR)\efi
$(BUILD_DIR)\common        : ; - md $(BUILD_DIR)\common
$(BUILD_DIR)\common_source : ; - md $(BUILD_DIR)\common_source
$(BUILD_DIR)\ieee          : ; - md $(BUILD_DIR)\ieee
$(BUILD_DIR)\gen           : ; - md $(BUILD_DIR)\gen

#
#  Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\efi\init.obj \
\
    $(BUILD_DIR)\common\atan2.obj \
    $(BUILD_DIR)\common\sincos.obj \
    $(BUILD_DIR)\common\tan.obj \
\
    $(BUILD_DIR)\common_source\acosh.obj \
    $(BUILD_DIR)\common_source\asincos.obj \
    $(BUILD_DIR)\common_source\asinh.obj \
    $(BUILD_DIR)\common_source\atan.obj \
    $(BUILD_DIR)\common_source\atanh.obj \
    $(BUILD_DIR)\common_source\cosh.obj \
    $(BUILD_DIR)\common_source\erf.obj \
    $(BUILD_DIR)\common_source\exp.obj \
    $(BUILD_DIR)\common_source\exp__e.obj \
    $(BUILD_DIR)\common_source\expm1.obj \
    $(BUILD_DIR)\common_source\floor.obj \
    $(BUILD_DIR)\common_source\fmod.obj \
    $(BUILD_DIR)\common_source\gamma.obj \
    $(BUILD_DIR)\common_source\j0.obj \
    $(BUILD_DIR)\common_source\j1.obj \
    $(BUILD_DIR)\common_source\jn.obj \
    $(BUILD_DIR)\common_source\lgamma.obj \
    $(BUILD_DIR)\common_source\log.obj \
    $(BUILD_DIR)\common_source\log__l.obj \
    $(BUILD_DIR)\common_source\log10.obj \
    $(BUILD_DIR)\common_source\log1p.obj \
    $(BUILD_DIR)\common_source\pow.obj \
    $(BUILD_DIR)\common_source\sinh.obj \
    $(BUILD_DIR)\common_source\tanh.obj \
\
    $(BUILD_DIR)\ieee\cabs.obj \
    $(BUILD_DIR)\ieee\cbrt.obj \
    $(BUILD_DIR)\ieee\support.obj \
\
    $(BUILD_DIR)\gen\fabs.obj \
    $(BUILD_DIR)\gen\frexp.obj \
    $(BUILD_DIR)\gen\ldexp.obj \
    $(BUILD_DIR)\gen\modf.obj \
    $(BUILD_DIR)\gen\abs.obj \
    $(BUILD_DIR)\gen\atof.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\efi\init.obj              : efi\$(*B).c           $(INC_DEPS)

$(BUILD_DIR)\common\atan2.obj          : common\$(*B).c        $(INC_DEPS)
$(BUILD_DIR)\common\sincos.obj         : common\$(*B).c        $(INC_DEPS)
$(BUILD_DIR)\common\tan.obj            : common\$(*B).c        $(INC_DEPS)

$(BUILD_DIR)\common_source\acosh.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\asincos.obj : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\asinh.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\atan.obj    : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\atanh.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\cosh.obj    : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\erf.obj     : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\exp.obj     : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\exp__e.obj  : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\expm1.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\floor.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\fmod.obj    : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\gamma.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\j0.obj      : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\j1.obj      : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\jn.obj      : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\lgamma.obj  : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\log.obj     : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\log__l.obj  : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\log10.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\log1p.obj   : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\pow.obj     : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\sinh.obj    : common_source\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\common_source\tanh.obj    : common_source\$(*B).c $(INC_DEPS)

$(BUILD_DIR)\ieee\cabs.obj             : ieee\$(*B).c          $(INC_DEPS)
$(BUILD_DIR)\ieee\cbrt.obj             : ieee\$(*B).c          $(INC_DEPS)
$(BUILD_DIR)\ieee\support.obj          : ieee\$(*B).c          $(INC_DEPS)

$(BUILD_DIR)\gen\fabs.obj              : gen\$(*B).c           $(INC_DEPS)
$(BUILD_DIR)\gen\frexp.obj             : gen\$(*B).c           $(INC_DEPS)
$(BUILD_DIR)\gen\ldexp.obj             : gen\$(*B).c           $(INC_DEPS)
$(BUILD_DIR)\gen\modf.obj              : gen\$(*B).c           $(INC_DEPS)
$(BUILD_DIR)\gen\abs.obj               : gen\$(*B).c           $(INC_DEPS)
$(BUILD_DIR)\gen\atof.obj              : gen\$(*B).c           $(INC_DEPS)

#
# Because libm compiles out of sub-directories, we need some of our own
# inference rules.  $(CC_LINE) is defined in master.mak
#

{efi}.c{$(BUILD_DIR)\efi}.obj:                     ; $(CC_LINE)
{common}.c{$(BUILD_DIR)\common}.obj:               ; $(CC_LINE)
{common_source}.c{$(BUILD_DIR)\common_source}.obj: ; $(CC_LINE)
{ieee}.c{$(BUILD_DIR)\ieee}.obj:                   ; $(CC_LINE)
{gen}.c{$(BUILD_DIR)\gen}.obj:                     ; $(CC_LINE)

#
# Handoff to Master.Mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
