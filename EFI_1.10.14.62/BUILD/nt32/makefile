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
# Libs to link together a FW image
#

EFI_LIBS =                                                            \
  Output\Corefw\Fw\Efi\efi.lib                                        \
  Output\Corefw\Fw\Platform\Lib\Lib.lib                               \
  Output\Corefw\Fw\Platform\Buildtip\nt32\nt32.lib                    \
  Output\Corefw\Fw\Platform\PlDriver\EfiLdr\EfiLdr.lib                \
  Output\Corefw\Fw\Platform\PlDriver\GoVirtual\GoVirtual.lib          \
  Output\Corefw\Fw\Platform\PlDriver\NullDefio\NullDefio.lib          \
  Output\Corefw\Fw\Platform\PlDriver\Unicode\English\English.lib      \
  Output\Corefw\Fw\Platform\PlDriver\Watchdog\Watchdog.lib            \
  Output\Edk\Guid\Guid.lib                                            \
  Output\Edk\Protocol\Protocol.lib                                    \
  Output\Lib\lib.lib                                                  \

!IF "$(EFI_BOOTSHELL)" == "YES"
EFI_LIBS = $(EFI_LIBS)  \
  Output\Corefw\Fw\Platform\PlDriver\InternalShell\InternalShell.lib  \
!ENDIF

#
# Libs to link together the NT console app
#

NT_LIBS = kernel32.lib msvcrt.lib gdi32.lib user32.lib

#
#
#

OUTPUTS=bin\nt32.exe 

all: vccheck banner genmake output\null.obj helpdata makemaker $(OUTPUTS)

bin\nt32.exe: $(EFI_LIBS)
  $(LINK) /LIBPATH:"$(EFI_LIBPATH)" /LIBPATH:"$(EFI_PSDKPATH)\Lib" $(L_FLAGS_EXE) /OUT:bin\nt32.exe /PDB:bin\nt32.pdb $(NT_LIBS) $(EFI_LIBS)

#
# Everything to build
#

#
# Check the version of C2.dll
#
vccheck:
  "$(BUILD_TOOL)\VcCheck"

banner:
  -type $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\NT32\banner.c > $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\NT32\banner.xxx
  -del $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\NT32\banner.c
  -rename $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\NT32\banner.xxx banner.c

genmake:
  "$(BUILD_TOOL)\genmake"

output\null.obj: $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c
  $(CC) -c $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c /Fooutput\null.obj

$(EFI_SOURCE)\Shell\HelpData\HelpData.c: $(EFI_SOURCE)\Shell\HelpData\HelpData.src
  $(BUILD_TOOL)\GenHelp $(EFI_SOURCE)\Shell\HelpData\HelpData.src $(EFI_SOURCE)\Shell\HelpData\HelpData.c

helpdata: $(EFI_SOURCE)\Shell\HelpData\HelpData.c

makemaker::
  $(MAKE) -f Output\Edk\Guid\makefile                                        all
  $(MAKE) -f Output\Edk\Lib\EfiDriverLib\makefile                            all
  $(MAKE) -f Output\Edk\Lib\EfiCommonLib\makefile                            all
  $(MAKE) -f Output\Edk\Lib\Print\makefile                                   all
  $(MAKE) -f Output\Edk\Protocol\makefile                                    all
  $(MAKE) -f Output\Lib\makefile                                             all
  $(MAKE) -f Output\Corefw\Fw\Platform\Lib\makefile                          all

  $(MAKE) -f Output\Corefw\Fw\Efi\makefile                                   all
  $(MAKE) -f Output\Corefw\Fw\Platform\BootMgr\Default\makefile              all
  $(MAKE) -f Output\Corefw\Fw\Platform\BootMgr\Maint\makefile                all
  $(MAKE) -f Output\Corefw\Fw\Platform\Buildtip\nt32\makefile                all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\EfiLdr\makefile              all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\GoVirtual\makefile           all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\NullDefio\makefile           all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Unicode\English\makefile     all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Watchdog\makefile            all

  $(MAKE) -f Output\Edk\Drivers\AtapiPassThru\makefile                       all

  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcl\makefile   all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcsp\makefile  all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmvl\makefile   all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\cssm\makefile              all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\port\makefile              all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\util\ber_der\r1_1\makefile all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\icl\makefile                   all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\integrity\makefile             all

  $(MAKE) -f Output\Edk\Drivers\Bis\BaseCode\makefile                        all

  $(MAKE) -f Output\Edk\Drivers\Console\ConPlatform\makefile                 all
  $(MAKE) -f Output\Edk\Drivers\Console\ConSplitter\makefile                 all
  $(MAKE) -f Output\Edk\Drivers\Console\GraphicsConsole\makefile             all
  $(MAKE) -f Output\Edk\Drivers\Console\Terminal\makefile                    all
  $(MAKE) -f output\Edk\drivers\DebugPort\makefile                           all
  $(MAKE) -f output\Edk\drivers\DebugSupport\makefile                        all
  $(MAKE) -f Output\Edk\Drivers\Decompress\makefile                          all
  $(MAKE) -f Output\Edk\Drivers\DiskIo\makefile                              all
  $(MAKE) -f Output\Edk\Drivers\Ebc\makefile                                 all
  $(MAKE) -f Output\Edk\Drivers\FileSystem\Fat\makefile                      all
  $(MAKE) -f Output\Edk\Drivers\Partition\makefile                           all
  $(MAKE) -f Output\Edk\Drivers\PciBus\makefile                              all
  $(MAKE) -f Output\Edk\Drivers\PxeBc\makefile                               all
  $(MAKE) -f Output\Edk\Drivers\SerialMouse\makefile                         all
  $(MAKE) -f Output\Edk\Drivers\Snp32_64\makefile                            all
  $(MAKE) -f Output\Edk\Drivers\Undi\makefile                                all
  $(MAKE) -f Output\Edk\Drivers\WinNtThunk\WinNtPciRootBridge\makefile       all
  $(MAKE) -f Output\Edk\Drivers\WinNtThunk\WinNtBusDriver\makefile           all
  $(MAKE) -f Output\Edk\Drivers\WinNtThunk\BlockIo\makefile                  all
  $(MAKE) -f Output\Edk\Drivers\WinNtThunk\Console\makefile                  all
  $(MAKE) -f Output\Edk\Drivers\WinNtThunk\SerialIo\makefile                 all
  $(MAKE) -f Output\Edk\Drivers\WinNtThunk\SimpleFileSystem\makefile         all
  $(MAKE) -f Output\Edk\Drivers\WinNtThunk\Uga\makefile                      all

!IF "$(EFI_BOOTSHELL)" == "YES"
  $(MAKE) -f Output\Shell\Lib\makefile                                       all

  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\InternalShell\makefile       all
  $(MAKE) -f Output\Shell\EfiCompress\makefile                               all
  $(MAKE) -f Output\Shell\EfiDecompress\makefile                             all
  $(MAKE) -f Output\Shell\LoadPciRom\makefile                                all
  $(MAKE) -f Output\Shell\attrib\makefile                                    all
  $(MAKE) -f Output\Shell\bcfg\makefile                                      all
  $(MAKE) -f Output\Shell\cls\makefile                                       all
  $(MAKE) -f Output\Shell\comp\makefile                                      all
  $(MAKE) -f Output\Shell\cp\makefile                                        all
  $(MAKE) -f Output\Shell\date\makefile                                      all
  $(MAKE) -f Output\Shell\debug\makefile                                     all
  $(MAKE) -f Output\Shell\dmpstore\makefile                                  all
  $(MAKE) -f Output\Shell\edit\makefile                                      all
  $(MAKE) -f Output\Shell\err\makefile                                       all
  $(MAKE) -f Output\Shell\getmtc\makefile                                    all
  $(MAKE) -f Output\Shell\hexedit\makefile                                   all
  $(MAKE) -f Output\Shell\iomod\makefile                                     all
  $(MAKE) -f Output\Shell\load\makefile                                      all
  $(MAKE) -f Output\Shell\loadbmp\makefile                                   all
  $(MAKE) -f Output\Shell\ls\makefile                                        all
  $(MAKE) -f Output\Shell\mem\makefile                                       all
  $(MAKE) -f Output\Shell\memmap\makefile                                    all
  $(MAKE) -f Output\Shell\mkdir\makefile                                     all
  $(MAKE) -f Output\Shell\mode\makefile                                      all
  $(MAKE) -f Output\Shell\mv\makefile                                        all
  $(MAKE) -f Output\Shell\newshell\makefile                                  all
  $(MAKE) -f Output\Shell\pci\makefile                                       all
  $(MAKE) -f Output\Shell\reset\makefile                                     all
  $(MAKE) -f Output\Shell\rm\makefile                                        all
  $(MAKE) -f Output\Shell\setsize\makefile                                   all
  $(MAKE) -f Output\Shell\shellenv\makefile                                  all
  $(MAKE) -f Output\Shell\stall\makefile                                     all
  $(MAKE) -f Output\Shell\time\makefile                                      all
  $(MAKE) -f Output\Shell\touch\makefile                                     all
  $(MAKE) -f Output\Shell\type\makefile                                      all
  $(MAKE) -f Output\Shell\ver\makefile                                       all
  $(MAKE) -f Output\Shell\vol\makefile                                       all
  
  $(MAKE) -f Output\Shell\dumpbs\makefile                                    all
!ENDIF

!IF "$(EFI_BOOTSHELL)" == "NO"
  $(MAKE) -f Output\Shell\makefile                                           all
!ENDIF

bsc:
  - dir /s /b *.sbr > temp.rm
  $(BSCMAKE) /n /o bin\nt32.bsc "@temp.rm"

clean:
  - rd  /s /q output
  - rd  /s /q bin
  - del VC?0.*
  - del $(TARGET_BASENAME).*

run:
  - cd bin
  - nt32.exe

runloop:
  - runloop

testlib:
  - lib /out:bin\alllib.lib $(EFI_LIBS)

!INCLUDE ..\makefile.common

!IF EXIST (Restricted.mak)
!INCLUDE Restricted.mak
!ENDIF
