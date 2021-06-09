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
#    Master makefile
#
# Revision History
#

!include master.env

#
# Everything to build
#

all: vccheck genmake output\null.obj library makemaker

#
# Check the version of C2.dll
#
vccheck:
  "$(BUILD_TOOL)\VcCheck"

genmake:
  "$(BUILD_TOOL)\genmake"

output\null.obj: $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c
  $(CC) -c $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c /Fooutput\null.obj

library:
  $(MAKE) -f output\Edk\Lib\EfiCommonLib\makefile                                   all
  $(MAKE) -f output\Edk\Lib\EfiDriverLib\makefile                                   all
  $(MAKE) -f output\Edk\Lib\Print\makefile                                          all
  $(MAKE) -f output\Edk\Protocol\makefile                                           all
  $(MAKE) -f output\Edk\Guid\makefile                                               all

makemaker::
  $(MAKE) -f output\Edk\drivers\AtapiPassThru\makefile                              all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcl\makefile          all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcsp\makefile         all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmvl\makefile          all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\cssm\makefile                     all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\port\makefile                     all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\util\ber_der\r1_1\makefile        all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\icl\makefile                          all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\integrity\makefile                    all
  $(MAKE) -f Output\Edk\Drivers\Bis\BaseCode\makefile                               all
  $(MAKE) -f output\Edk\drivers\Console\ConPlatform\makefile                        all
  $(MAKE) -f output\Edk\drivers\Console\ConSplitter\makefile                        all
  $(MAKE) -f output\Edk\drivers\Console\GraphicsConsole\makefile                    all
  $(MAKE) -f output\Edk\drivers\Console\Terminal\makefile                           all
  $(MAKE) -f Output\Edk\Drivers\CirrusLogic5430\makefile                            all
  $(MAKE) -f output\Edk\drivers\DebugPort\makefile                                  all
  $(MAKE) -f output\Edk\drivers\DebugSupport\makefile                               all
  $(MAKE) -f output\Edk\drivers\Decompress\makefile                                 all
  $(MAKE) -f output\Edk\Drivers\DiskIo\makefile                                     all
  $(MAKE) -f output\Edk\drivers\Ebc\makefile                                        all
  $(MAKE) -f output\Edk\Drivers\FileSystem\Fat\makefile                             all
  $(MAKE) -f output\Edk\Drivers\Ide\makefile                                        all
  $(MAKE) -f Output\Edk\Drivers\IsaBus\Makefile                                     all
  $(MAKE) -f Output\Edk\Drivers\IsaSerial\Makefile                                  all
  $(MAKE) -f Output\Edk\Drivers\IsaFloppy\Makefile                                  all
  $(MAKE) -f output\Edk\Drivers\Partition\makefile                                  all
  $(MAKE) -f Output\Edk\Drivers\PcatIsaAcpi\Makefile                                all
  $(MAKE) -f Output\Edk\Drivers\PcatIsaAcpiBios\Makefile                            all
  $(MAKE) -f output\Edk\Drivers\PcatPciRootBridge\makefile                          all
  $(MAKE) -f output\Edk\Drivers\PciBus\makefile                                     all
  $(MAKE) -f Output\Edk\Drivers\PciVgaMiniPort\Makefile                             all
  $(MAKE) -f Output\Edk\Drivers\Ps2Keyboard\Makefile                                all
  $(MAKE) -f Output\Edk\Drivers\Ps2Mouse\Makefile                                   all
  $(MAKE) -f output\Edk\Drivers\PxeBc\makefile                                      all
  $(MAKE) -f output\Edk\Drivers\PxeDhcp4\makefile                                   all
  $(MAKE) -f output\Edk\drivers\ScsiBus\makefile                                    all
  $(MAKE) -f output\Edk\drivers\ScsiDisk\makefile                                   all
  $(MAKE) -f output\Edk\drivers\SerialMouse\makefile                                all
  $(MAKE) -f output\Edk\drivers\Snp32_64\makefile                                   all
  $(MAKE) -f output\Edk\Drivers\Undi\makefile                                       all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbBus\Makefile                                 all
  $(MAKE) -f Output\Edk\Drivers\Usb\Uhci\Makefile                                   all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbKb\Makefile                                  all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbMouse\Makefile                               all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbMassStorage\Makefile                         all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbBot\Makefile                                 all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbCbi\Cbi1\Makefile                            all
  $(MAKE) -f Output\Edk\Drivers\VgaClass\Makefile                                   all
      
clean:
  - rd  /s /q output
  - rd  /s /q bin
  - del VC?0.*
  
testlib:
  - lib /out:bin\alllib.lib $(EFI_LIBS)

!INCLUDE ..\makefile.common

!IF EXIST (Restricted.mak)
!INCLUDE Restricted.mak
!ENDIF
