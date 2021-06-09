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
#  Set the base output name
#

BASE_NAME = libdb

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\lib\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\lib\$(BASE_NAME)

#
# Additional compiler flags
#

C_FLAGS = /D __STDC__ /D __DBINTERFACE_PRIVATE /D MMAP_NOT_AVAILABLE $(C_FLAGS)

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd  $(INC)

!include makefile.hdr
INC = -I btree \
      -I hash \
      -I recno $(INC)

#
# Default target
#

all : sub_dirs $(OBJECTS)

sub_dirs : $(BUILD_DIR)\btree \
           $(BUILD_DIR)\db \
           $(BUILD_DIR)\hash \
           $(BUILD_DIR)\mpool \
           $(BUILD_DIR)\recno \
	
#
# Sub-directory targets
#

$(BUILD_DIR)\btree : ; - md $(BUILD_DIR)\btree
$(BUILD_DIR)\hash  : ; - md $(BUILD_DIR)\hash
$(BUILD_DIR)\mpool : ; - md $(BUILD_DIR)\mpool
$(BUILD_DIR)\recno : ; - md $(BUILD_DIR)\recno
$(BUILD_DIR)\db    : ; - md $(BUILD_DIR)\db

#
#  Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\btree\bt_close.obj \
    $(BUILD_DIR)\btree\bt_conv.obj \
    $(BUILD_DIR)\btree\bt_debug.obj \
    $(BUILD_DIR)\btree\bt_delete.obj \
    $(BUILD_DIR)\btree\bt_get.obj \
    $(BUILD_DIR)\btree\bt_open.obj \
    $(BUILD_DIR)\btree\bt_overflow.obj \
    $(BUILD_DIR)\btree\bt_page.obj \
    $(BUILD_DIR)\btree\bt_put.obj \
    $(BUILD_DIR)\btree\bt_search.obj \
    $(BUILD_DIR)\btree\bt_seq.obj \
    $(BUILD_DIR)\btree\bt_split.obj \
    $(BUILD_DIR)\btree\bt_utils.obj \
\
    $(BUILD_DIR)\db\db.obj \
\
    $(BUILD_DIR)\hash\hash.obj \
    $(BUILD_DIR)\hash\hash_bigkey.obj \
    $(BUILD_DIR)\hash\hash_buf.obj \
    $(BUILD_DIR)\hash\hash_func.obj \
    $(BUILD_DIR)\hash\hash_log2.obj \
    $(BUILD_DIR)\hash\hash_page.obj \
    $(BUILD_DIR)\hash\hsearch.obj \
    $(BUILD_DIR)\hash\ndbm.obj \
\
    $(BUILD_DIR)\mpool\mpool.obj \
\
    $(BUILD_DIR)\recno\rec_close.obj \
    $(BUILD_DIR)\recno\rec_delete.obj \
    $(BUILD_DIR)\recno\rec_get.obj \
    $(BUILD_DIR)\recno\rec_open.obj \
    $(BUILD_DIR)\recno\rec_put.obj \
    $(BUILD_DIR)\recno\rec_search.obj \
    $(BUILD_DIR)\recno\rec_seq.obj \
    $(BUILD_DIR)\recno\rec_utils.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\btree\bt_close.obj    : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_conv.obj     : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_debug.obj    : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_debug.obj    : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_delete.obj   : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_get.obj      : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_open.obj     : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_overflow.obj : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_page.obj     : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_put.obj      : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_search.obj   : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_seq.obj      : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_split.obj    : btree\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\btree\bt_utils.obj    : btree\$(*B).c $(INC_DEPS)

$(BUILD_DIR)\db\db.obj             : db\$(*B).c    $(INC_DEPS)

$(BUILD_DIR)\hash\hash.obj         : hash\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\hash\hash_bigkey.obj  : hash\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\hash\hash_buf.obj     : hash\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\hash\hash_func.obj    : hash\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\hash\hash_log2.obj    : hash\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\hash\hash_page.obj    : hash\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\hash\hsearch.obj      : hash\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\hash\ndbm.obj         : hash\$(*B).c  $(INC_DEPS)

$(BUILD_DIR)\mpool\mpool.obj       : mpool\$(*B).c $(INC_DEPS)

$(BUILD_DIR)\recno\rec_close.obj   : recno\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\recno\rec_delete.obj  : recno\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\recno\rec_get.obj     : recno\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\recno\rec_open.obj    : recno\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\recno\rec_put.obj     : recno\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\recno\rec_search.obj  : recno\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\recno\rec_seq.obj     : recno\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\recno\rec_utils.obj   : recno\$(*B).c $(INC_DEPS)

#
# Because libdb compiles out of sub-directories, we need some of our own
# inference rules.  $(CC_LINE) is defined in master.mak
#

{btree}.c{$(BUILD_DIR)\btree}.obj: ; $(CC_LINE)
{db}.c{$(BUILD_DIR)\db}.obj:       ; $(CC_LINE)
{hash}.c{$(BUILD_DIR)\hash}.obj:   ; $(CC_LINE)
{mpool}.c{$(BUILD_DIR)\mpool}.obj: ; $(CC_LINE)
{recno}.c{$(BUILD_DIR)\recno}.obj: ; $(CC_LINE)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak

