#
# Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
# This software and associated documentation (if any) is furnished
# under a license and may only be used or copied in accordance
# with the terms of the license. Except as permitted by such
# license, no part of this software or documentation may be
# reproduced, stored in a retrieval system, or transmitted in any
# form or by any means without the express written consent of
# Intel Corporation.
#
# Module Name:
#
#    Makefile
#    
# Abstract:
#
#    Master makefile for building EBC drivers.
#
################################################################################

!include master.env

#
# Everything to build
#

all: genmake output\null.obj library drivers

genmake :
  "$(BUILD_TOOL)\genmake"

output\null.obj : $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c
  $(CC) $(C_FLAGS) $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c /Fooutput\null.obj

library :
  $(MAKE) -f output\Edk\Lib\EfiCommonLib\makefile             all
  $(MAKE) -f output\Edk\Lib\EfiDriverLib\makefile             all
  $(MAKE) -f output\Edk\Lib\Print\makefile                    all
  $(MAKE) -f output\Edk\Protocol\makefile                     all
  $(MAKE) -f output\Edk\Guid\makefile                         all

drivers ::
  $(MAKE) -f Output\Edk\Drivers\CirrusLogic5430\makefile      all
  $(MAKE) -f output\Edk\Drivers\Ide\makefile                  all
#  $(MAKE) -f output\Edk\Drivers\Undi\makefile                 all

#
# NOTE: To build the Undi driver you must edit its make.inf file and change it
# from TARGET_RT_DRIVER to a TARGET_BS_DRIVER.
#

clean:
  - rd  /s /q output
  - rd  /s /q bin
  
testlib:
  - lib /out:bin\alllib.lib $(EFI_LIBS)

!IF EXIST (Restricted.mak)
!INCLUDE Restricted.mak
!ENDIF
