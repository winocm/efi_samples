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

SOURCE_DIR=$(SDK_INSTALL_DIR)\protocols

!include $(SDK_INSTALL_DIR)\build\$(SDK_BUILD_ENV)\sdk.env

all :
	cd $(SOURCE_DIR)\tcpipv4
	nmake -f tcpipv4.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\dhclient
	nmake -f dhclient.mak all
	cd $(SOURCE_DIR)
	
	cd $(SOURCE_DIR)\locale
	nmake -f locale.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\pppd
	nmake -f pppd.mak all
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ramdisk
	nmake -f ramdisk.mak all
	cd $(SOURCE_DIR)

!IF "$(PROCESSOR)" == "Ia64"

	cd $(SOURCE_DIR)\mp
	nmake -f mp.mak all
	cd $(SOURCE_DIR)

!ENDIF

clean :
	cd $(SOURCE_DIR)\tcpipv4
	nmake -f tcpipv4.mak clean 
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\dhclient
	nmake -f dhclient.mak clean
	cd $(SOURCE_DIR)
	
	cd $(SOURCE_DIR)\locale
	nmake -f locale.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\pppd
	nmake -f pppd.mak clean
	cd $(SOURCE_DIR)

	cd $(SOURCE_DIR)\ramdisk
	nmake -f ramdisk.mak clean
	cd $(SOURCE_DIR)

!IF "$(PROCESSOR)" == "Ia64"

	cd $(SOURCE_DIR)\mp
	nmake -f mp.mak clean 
	cd $(SOURCE_DIR)

!ENDIF
