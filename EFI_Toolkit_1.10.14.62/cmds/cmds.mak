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

SOURCE_DIR = $(SDK_INSTALL_DIR)\cmds

!include $(SDK_INSTALL_DIR)\build\$(SDK_BUILD_ENV)\sdk.env

all :
	cd $(SOURCE_DIR)\ed
	nmake -f ed.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\edit
	nmake -f edit.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\hexdump
	nmake -f hexdump.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\hostname
	nmake -f hostname.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ifconfig
	nmake -f ifconfig.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ping
	nmake -f ping.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\route
	nmake -f route.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\python
	nmake -f python.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ftp
	nmake -f ftp.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\mkramdisk
	nmake -f mkramdisk.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\which
	nmake -f which.mak all
	cd $(SOURCE_DIR)
	
  cd $(SOURCE_DIR)\loadarg
  nmake -f loadarg.mak all
  cd $(SOURCE_DIR)

  cd $(SOURCE_DIR)\nunload
  nmake -f nunload.mak all
  cd $(SOURCE_DIR)

!IF "$(PROCESSOR)" == "Ia64"

	cd $(SOURCE_DIR)\mptest
	nmake -f mptest.mak all
	cd $(SOURCE_DIR)

!ENDIF

clean :
	cd $(SOURCE_DIR)\ed
	nmake -f ed.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\hexdump
	nmake -f hexdump.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\hostname
	nmake -f hostname.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ifconfig
	nmake -f ifconfig.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ping
	nmake -f ping.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\route
	nmake -f route.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\python
	nmake -f python.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ftp
	nmake -f ftp.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\mkramdisk
	nmake -f mkramdisk.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\which
	nmake -f which.mak clean
	cd $(SOURCE_DIR)
	
  cd $(SOURCE_DIR)\loadarg
  nmake -f loadarg.mak clean
  cd $(SOURCE_DIR)

  cd $(SOURCE_DIR)\nunload
  nmake -f nunload.mak clean
  cd $(SOURCE_DIR)

!IF "$(PROCESSOR)" == "Ia64"

	cd $(SOURCE_DIR)\mptest
	nmake -f mptest.mak clean 
	cd $(SOURCE_DIR)

!ENDIF
