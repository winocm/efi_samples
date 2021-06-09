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

EFI_LIBS =                                                                                                              \
  output\corefw\fw\efi\efi.lib                                      \
  Output\Corefw\Fw\Platform\Lib\Lib.lib                             \
  output\corefw\fw\platform\BuildTip\Sal64\Sal64.lib                \
  output\corefw\fw\platform\drivers\BiosInt\Disk\Disk.lib           \
  output\corefw\fw\platform\drivers\BiosInt\BiosVga\BiosVga.lib     \
  output\corefw\fw\platform\drivers\BiosInt\BiosKeyBoard\BiosKeyBoard.lib   \
  output\corefw\fw\platform\drivers\BiosInt\BiosVgaMiniPort\BiosVgaMiniPort.lib     \
  output\corefw\fw\platform\drivers\BiosInt\BiosSnp16\BiosSnp16.lib \
  output\corefw\fw\platform\BootMgr\Default\Default.lib             \
  output\corefw\fw\platform\BootMgr\Maint\Maint.lib                 \
  output\corefw\fw\platform\PlDriver\Defio\Defio.lib                \
  output\corefw\fw\platform\PlDriver\BiosInt\BiosInt.lib            \
  output\corefw\fw\platform\PlDriver\Cache\Cache.lib                \
  output\corefw\fw\platform\PlDriver\CdRom\CdRom.lib                \
  output\corefw\fw\platform\PlDriver\GoVirtual\GoVirtual.lib        \
  output\corefw\fw\platform\PlDriver\LegacyBoot\LegacyBoot.lib      \
  output\corefw\fw\platform\PlDriver\NVRAM\NVRAM.lib                \
  output\corefw\fw\platform\PlDriver\Reset\Reset.lib                \
  output\corefw\fw\platform\PlDriver\Stall\8253\8253.lib            \
  output\corefw\fw\platform\PlDriver\Time\Dallas1287\Dallas1287.lib \
  output\corefw\fw\platform\PlDriver\Timer\Timer.lib                \
  output\corefw\fw\platform\PlDriver\Tpl\Tpl.lib                    \
  output\corefw\fw\platform\PlDriver\Sal\Sal.lib                    \
  output\corefw\fw\platform\PlDriver\Watchdog\Watchdog.lib          \
  output\corefw\fw\platform\PlDriver\Unicode\English\English.lib    \
  output\lib\lib.lib                                                \
  output\Edk\Lib\EfiCommonLib\EfiCommonLib.lib                      \
  output\Edk\Lib\EfiDriverLib\EfiDriverLib.lib                      \
  output\Edk\Lib\Print\Print.lib                                    \
  output\Edk\Protocol\Protocol.lib                                  \
  output\Edk\Guid\Guid.lib                                          \
  output\Edk\drivers\PcatPciRootBridge\PcatPciRootBridge.lib        \
  output\Edk\drivers\PciBus\PciBus.lib                              \
  output\Edk\drivers\Decompress\Decompress.lib                      \
  output\Edk\drivers\Ebc\Ebc.lib                                    \
  output\Edk\drivers\DiskIo\DiskIo.lib                              \
  output\Edk\drivers\Partition\Partition.lib                        \
  output\Edk\drivers\FileSystem\Fat\Fat.lib                         \
  output\Edk\drivers\Console\ConPlatform\ConPlatform.lib            \
  output\Edk\drivers\Console\ConSplitter\ConSplitter.lib            \
  Output\Edk\Drivers\Console\GraphicsConsole\GraphicsConsole.lib    \
  output\Edk\drivers\Console\Terminal\Terminal.lib                  \
  output\Edk\drivers\Undi\Undi.lib                                  \
  output\Edk\drivers\VgaClass\VgaClass.lib                          \
  Output\Edk\Drivers\SerialMouse\SerialMouse.lib                    \
  output\Edk\drivers\Snp32_64\Snp32_64.lib                          \
  output\Edk\drivers\PxeBc\PxeBc.lib                                \
  output\Edk\drivers\PxeDhcp4\PxeDhcp4.lib                          \
  Output\Edk\Drivers\PcatIsaAcpiBios\PcatIsaAcpiBios.lib            \
  Output\Edk\Drivers\IsaBus\IsaBus.lib                              \
  Output\Edk\Drivers\IsaSerial\IsaSerial.lib                        \
#  Output\Edk\Drivers\Bis\BaseCode\BaseCode.lib                      \
#  Output\Edk\Drivers\Bis\Oasis\Src\Addins\Intel\Cssmcl\Cssmcl.lib   \
#  Output\Edk\Drivers\Bis\Oasis\Src\Addins\Intel\Cssmcsp\Cssmcsp.lib \
#  Output\Edk\Drivers\Bis\Oasis\Src\Addins\Intel\Cssmvl\Cssmvl.lib   \
#  Output\Edk\Drivers\Bis\Oasis\Src\Fwk\Cssm\Cssm.lib                \
#  Output\Edk\Drivers\Bis\Oasis\Src\Fwk\Port\Port.lib                \
#  Output\Edk\Drivers\Bis\Oasis\Src\Fwk\Util\Ber_der\R1_1\R1_1.lib   \
#  Output\Edk\Drivers\Bis\Oasis\Src\Icl\Icl.lib                      \
#  Output\Edk\Drivers\Bis\Oasis\Src\Integrity\Integrity.lib          \

#
# We sometimes link the shell's tools into this build, so include
# libs for all the tools and the shell
#
# (Note this is for debugging, and a real build should remove
# this as to be sure it's not mistakenly including any parts of
# the shell)
#

!IF "$(EFI_BOOTSHELL)" == "YES"
EFI_LIBS = $(EFI_LIBS)                                                \
  output\shell\newshell\newshell.lib                                  \
  output\shell\shellenv\shellenv.lib                                  \
  output\shell\lib\lib.lib                                            \
  output\shell\rm\rm.lib                                              \
  output\shell\mkdir\mkdir.lib                                        \
  output\shell\ls\ls.lib                                              \
  output\shell\mode\mode.lib                                          \
  output\shell\cp\cp.lib                                              \
  output\shell\memmap\memmap.lib                                      \
  output\shell\type\type.lib                                          \
  output\shell\load\load.lib                                          \
  output\shell\loadbmp\loadbmp.lib                                    \
  output\shell\dmpstore\dmpstore.lib                                  \
  output\shell\ver\ver.lib                                            \
  output\shell\attrib\attrib.lib                                      \
  output\shell\mv\mv.lib                                              \
  output\shell\date\date.lib                                          \
  output\shell\time\time.lib                                          \
  output\shell\stall\stall.lib                                        \
  output\shell\reset\reset.lib                                        \
  output\shell\debug\debug.lib                                        \
  output\shell\iomod\iomod.lib                                        \
  output\shell\mem\mem.lib                                            \
  output\shell\pci\pci.lib                                            \
  output\shell\err\err.lib                                            \
  output\shell\vol\vol.lib                                            \
  output\shell\comp\comp.lib                                          \
  output\shell\cls\cls.lib                                            \
  output\shell\bcfg\bcfg.lib                                          \
  output\shell\edit\edit.lib                                          \
  output\shell\hexedit\hexedit.lib                                    \
  output\shell\eficompress\eficompress.lib                            \
  output\shell\efidecompress\efidecompress.lib                        \
  output\shell\loadpcirom\loadpcirom.lib                              \
  output\corefw\fw\platform\PlDriver\InternalShell\InternalShell.lib  \
!ENDIF

#
#
#
!IFNDEF EFI_TARGETNAME
TARGET_BASENAME=Sal64
!ELSE
TARGET_BASENAME=$(EFI_TARGETNAME)
!ENDIF

OUTPUTS=bin\$(TARGET_BASENAME).exe

all: banner genmake output\null.obj helpdata makemaker $(OUTPUTS)

$(OUTPUTS): $(EFI_LIBS)
  $(LINK) $(L_FLAGS) $(NT_LIBS) $(EFI_LIBS) /entry:MainEntry /out:$@ /pdb:$*.pdb
# pe2sym $*.pdb $*.sym

#
# Everything to build
#

banner:
  -type $(EFI_SOURCE)\corefw\fw\platform\BuildTip\Sal64\banner.c > $(EFI_SOURCE)\corefw\fw\platform\BuildTip\Sal64\banner.xxx
  -del $(EFI_SOURCE)\corefw\fw\platform\BuildTip\Sal64\banner.c
  -rename $(EFI_SOURCE)\corefw\fw\platform\BuildTip\Sal64\banner.xxx banner.c

genmake:
  "$(BUILD_TOOL)\genmake"

output\null.obj: $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c
  $(CC) -c $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c /Fooutput\null.obj

$(EFI_SOURCE)\Shell\HelpData\HelpData.c: $(EFI_SOURCE)\Shell\HelpData\HelpData.src
  $(BUILD_TOOL)\GenHelp $(EFI_SOURCE)\Shell\HelpData\HelpData.src $(EFI_SOURCE)\Shell\HelpData\HelpData.c

helpdata: $(EFI_SOURCE)\Shell\HelpData\HelpData.c

makemaker::
  $(MAKE) -f output\lib\makefile                                              all
  $(MAKE) -f output\Edk\Lib\EfiCommonLib\makefile                             all
  $(MAKE) -f output\Edk\Lib\EfiDriverLib\makefile                             all
  $(MAKE) -f output\Edk\Lib\Print\makefile                                    all
  $(MAKE) -f output\Edk\Protocol\makefile                                     all
  $(MAKE) -f output\Edk\Guid\makefile                                         all
  $(MAKE) -f Output\Corefw\Fw\Platform\Lib\makefile                           all

  $(MAKE) -f output\Edk\drivers\DebugPort\makefile                            all
  $(MAKE) -f output\Edk\drivers\DebugSupport\makefile                         all
  $(MAKE) -f output\Edk\drivers\Decompress\makefile                           all
  $(MAKE) -f output\Edk\drivers\Ebc\makefile                                  all
  $(MAKE) -f output\Edk\Drivers\PcatPciRootBridge\makefile                    all
  $(MAKE) -f output\Edk\Drivers\PciBus\makefile                               all
  $(MAKE) -f output\Edk\Drivers\DiskIo\makefile                               all
  $(MAKE) -f output\Edk\Drivers\Partition\makefile                            all
  $(MAKE) -f output\Edk\Drivers\FileSystem\Fat\makefile                       all
  $(MAKE) -f output\Edk\drivers\Console\ConPlatform\makefile                  all
  $(MAKE) -f output\Edk\drivers\Console\ConSplitter\makefile                  all
  $(MAKE) -f Output\Edk\Drivers\Console\GraphicsConsole\makefile              all
  $(MAKE) -f output\Edk\drivers\Console\Terminal\makefile                     all
  $(MAKE) -f output\Edk\drivers\Undi\makefile                                 all
  $(MAKE) -f output\Edk\drivers\VgaClass\makefile                             all
  $(MAKE) -f Output\Edk\Drivers\SerialMouse\makefile                          all
  $(MAKE) -f output\Edk\drivers\Snp32_64\makefile                             all
  $(MAKE) -f output\Edk\drivers\PxeBc\makefile                                all
  $(MAKE) -f output\Edk\drivers\PxeDhcp4\makefile                             all

  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcl\makefile    all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcsp\makefile   all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmvl\makefile    all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\cssm\makefile               all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\port\makefile               all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\util\ber_der\r1_1\makefile  all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\icl\makefile                    all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\integrity\makefile              all

  $(MAKE) -f Output\Edk\Drivers\Bis\BaseCode\makefile                         all

  $(MAKE) -f output\corefw\fw\efi\makefile                                    all
  $(MAKE) -f output\corefw\fw\platform\BuildTip\Sal64\makefile                all
  $(MAKE) -f output\corefw\fw\platform\drivers\BiosInt\Disk\makefile          all
  $(MAKE) -f output\corefw\fw\platform\drivers\BiosInt\BiosKeyboard\makefile  all
  $(MAKE) -f output\corefw\fw\platform\drivers\BiosInt\BiosVga\makefile       all
  $(MAKE) -f output\corefw\fw\platform\drivers\BiosInt\BiosVgaMiniPort\makefile  all
  $(MAKE) -f output\corefw\fw\platform\drivers\BiosInt\BiosSnp16\makefile     all
  $(MAKE) -f output\corefw\fw\platform\BootMgr\Default\makefile               all
  $(MAKE) -f output\corefw\fw\platform\BootMgr\Maint\makefile                 all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Defio\makefile                all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\BiosInt\makefile              all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Cache\makefile                all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\CdRom\makefile                all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\GoVirtual\makefile            all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\LegacyBoot\makefile           all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\NVRAM\makefile                all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Reset\makefile                all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Stall\8253\makefile           all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Time\Dallas1287\makefile      all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Timer\makefile                all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Tpl\makefile                  all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Sal\makefile                  all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Watchdog\makefile             all
  $(MAKE) -f output\corefw\fw\platform\PlDriver\Unicode\English\makefile      all

  $(MAKE) -f Output\Edk\Drivers\PcatIsaAcpiBios\Makefile                      all
  $(MAKE) -f Output\Edk\Drivers\IsaBus\Makefile                               all
  $(MAKE) -f Output\Edk\Drivers\IsaSerial\Makefile                            all

!IF "$(EFI_BOOTSHELL)" == "YES"
  $(MAKE) -f output\corefw\fw\platform\PlDriver\InternalShell\makefile        all
  $(MAKE) -f output\shell\lib\makefile                                        all
  $(MAKE) -f output\shell\shellenv\makefile                                   all
  $(MAKE) -f output\shell\newshell\makefile                                   all
  $(MAKE) -f output\shell\ls\makefile                                         all
  $(MAKE) -f output\shell\mode\makefile                                       all
  $(MAKE) -f output\shell\mkdir\makefile                                      all
  $(MAKE) -f output\shell\rm\makefile                                         all
  $(MAKE) -f output\shell\cp\makefile                                         all
  $(MAKE) -f output\shell\comp\makefile                                       all
  $(MAKE) -f output\shell\type\makefile                                       all
  $(MAKE) -f output\shell\memmap\makefile                                     all
  $(MAKE) -f output\shell\dmpstore\makefile                                   all
  $(MAKE) -f output\shell\load\makefile                                       all
  $(MAKE) -f output\shell\loadbmp\makefile                                    all
  $(MAKE) -f output\shell\getmtc\makefile                                     all
  $(MAKE) -f output\shell\debug\makefile                                      all
  $(MAKE) -f output\shell\pci\makefile                                        all
  $(MAKE) -f output\shell\mem\makefile                                        all
  $(MAKE) -f output\shell\iomod\makefile                                      all
  $(MAKE) -f output\shell\ver\makefile                                        all
  $(MAKE) -f output\shell\date\makefile                                       all
  $(MAKE) -f output\shell\time\makefile                                       all
  $(MAKE) -f output\shell\reset\makefile                                      all
  $(MAKE) -f output\shell\stall\makefile                                      all
  $(MAKE) -f output\shell\attrib\makefile                                     all
  $(MAKE) -f output\shell\setsize\makefile                                    all
  $(MAKE) -f output\shell\touch\makefile                                      all
  $(MAKE) -f output\shell\mv\makefile                                         all
  $(MAKE) -f output\shell\err\makefile                                        all
  $(MAKE) -f output\shell\vol\makefile                                        all
  $(MAKE) -f output\shell\bcfg\makefile                                       all
  $(MAKE) -f output\shell\cls\makefile                                        all
  $(MAKE) -f output\shell\edit\makefile                                       all
  $(MAKE) -f output\shell\hexedit\makefile                                    all
  $(MAKE) -f output\shell\EfiCompress\makefile                                all
  $(MAKE) -f output\shell\EfiDecompress\makefile                              all
  $(MAKE) -f output\shell\LoadPciRom\makefile                                 all
!ENDIF

!IF "$(EFI_BOOTSHELL)" == "NO"
  $(MAKE) -f output\shell\makefile                                            all
!ENDIF

bsc:
  - dir /s /b *.sbr > temp.rm
  $(BSCMAKE) /n /o bin/$(TARGET_BASENAME).BSC "@temp.rm"
  -del temp.rm

clean:
  - rd  /s /q output
  - rd  /s /q bin
  - del VC?0.*
  - del $(TARGET_BASENAME).*

testlib:
  - lib /out:bin\alllib.lib $(EFI_LIBS)

!IF EXIST (Restricted.mak)
!INCLUDE Restricted.mak
!ENDIF
