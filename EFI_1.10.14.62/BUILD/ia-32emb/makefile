#
# Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
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

EFI_LIBS =                                                                        \
  Output\Corefw\Fw\Efi\Efi.lib                                                    \
  Output\Corefw\Fw\Platform\Lib\Lib.lib                                           \
  Output\Corefw\Fw\Platform\BootMgr\Default\Default.lib                           \
  Output\Corefw\Fw\Platform\BootMgr\Maint\Maint.lib                               \
  Output\Corefw\Fw\Platform\BuildTip\Ia-32emb\Ia-32emb.lib                        \
  Output\Corefw\Fw\Platform\PlDriver\Cache\Cache.lib                              \
  Output\Corefw\Fw\Platform\PlDriver\Cpu_interrupt\Cpu_interrupt.lib              \
  Output\Corefw\Fw\Platform\PlDriver\Cpu_timer\Cpu_timer.lib                      \
  Output\Corefw\Fw\Platform\PlDriver\Defio\Defio.lib                              \
  Output\Corefw\Fw\Platform\PlDriver\EfiLdr\EfiLdr.lib                            \
  Output\Corefw\Fw\Platform\PlDriver\GoVirtual\GoVirtual.lib                      \
  Output\Corefw\Fw\Platform\PlDriver\InterruptController\8259\8259.lib            \
  Output\Corefw\Fw\Platform\PlDriver\InterruptController\InterruptController.lib  \
  Output\Corefw\Fw\Platform\PlDriver\NVRAM\NVRAM.lib                              \
  Output\Corefw\Fw\Platform\PlDriver\Reset\Reset.lib                              \
  Output\Corefw\Fw\Platform\PlDriver\Stall\8253\8253.lib                          \
  Output\Corefw\Fw\Platform\PlDriver\Time\Dallas1287\Dallas1287.lib               \
  Output\Corefw\Fw\Platform\PlDriver\Tpl\Tpl.lib                                  \
  Output\Corefw\Fw\Platform\PlDriver\Unicode\English\English.lib                  \
  Output\Corefw\Fw\Platform\PlDriver\Watchdog\Watchdog.lib                        \
  Output\Edk\Drivers\Bis\BaseCode\BaseCode.lib                                    \
  Output\Edk\Drivers\Bis\Oasis\Src\Addins\Intel\Cssmcl\Cssmcl.lib                 \
  Output\Edk\Drivers\Bis\Oasis\Src\Addins\Intel\Cssmcsp\Cssmcsp.lib               \
  Output\Edk\Drivers\Bis\Oasis\Src\Addins\Intel\Cssmvl\Cssmvl.lib                 \
  Output\Edk\Drivers\Bis\Oasis\Src\Fwk\Cssm\Cssm.lib                              \
  Output\Edk\Drivers\Bis\Oasis\Src\Fwk\Port\Port.lib                              \
  Output\Edk\Drivers\Bis\Oasis\Src\Fwk\Util\Ber_der\R1_1\R1_1.lib                 \
  Output\Edk\Drivers\Bis\Oasis\Src\Icl\Icl.lib                                    \
  Output\Edk\Drivers\Bis\Oasis\Src\Integrity\Integrity.lib                        \
  Output\Edk\Drivers\CirrusLogic5430\CirrusLogic5430.lib                          \
  Output\Edk\Drivers\Console\ConPlatform\ConPlatform.lib                          \
  Output\Edk\Drivers\Console\ConSplitter\ConSplitter.lib                          \
  Output\Edk\Drivers\Console\GraphicsConsole\GraphicsConsole.lib                  \
  Output\Edk\Drivers\Console\Terminal\Terminal.lib                                \
  Output\Edk\Drivers\Decompress\Decompress.lib                                    \
  Output\Edk\Drivers\DiskIo\DiskIo.lib                                            \
  Output\Edk\Drivers\Ebc\Ebc.lib                                                  \
  Output\Edk\Drivers\FileSystem\Fat\Fat.lib                                       \
  Output\Edk\Drivers\Ide\Ide.lib                                                  \
  Output\Edk\Drivers\Partition\Partition.lib                                      \
  Output\Edk\Drivers\PcatPciRootBridge\PcatPciRootBridge.lib                      \
  Output\Edk\Drivers\PciBus\PciBus.lib                                            \
  Output\Edk\Drivers\PciVgaMiniPort\PciVgaMiniPort.lib                            \
  Output\Edk\Drivers\PxeBc\PxeBc.lib                                              \
  Output\Edk\Drivers\PxeDhcp4\PxeDhcp4.lib                                        \
  Output\Edk\Drivers\SerialMouse\SerialMouse.lib                                  \
  Output\Edk\Drivers\Snp32_64\Snp32_64.lib                                        \
  Output\Edk\Drivers\VgaClass\VgaClass.lib                                        \
  Output\Edk\Drivers\PcatIsaAcpi\PcatIsaAcpi.lib                                  \
  Output\Edk\Drivers\IsaBus\IsaBus.lib                                            \
  Output\Edk\Drivers\IsaSerial\IsaSerial.lib                                      \
  Output\Edk\Drivers\IsaFloppy\IsaFloppy.lib                                      \
  Output\Edk\Drivers\Ps2Keyboard\Ps2Keyboard.lib                                  \
  Output\Edk\Drivers\Ps2Mouse\Ps2Mouse.lib                                        \
  Output\Edk\Guid\Guid.lib                                                        \
  Output\Edk\Lib\EfiCommonLib\EfiCommonLib.lib                                    \
  Output\Edk\Lib\EfiDriverLib\EfiDriverLib.lib                                    \
  Output\Edk\Lib\Print\Print.lib                                                  \
  Output\Edk\Protocol\Protocol.lib                                                \
  Output\Lib\Lib.lib                                                              \
  Output\Edk\Drivers\Usb\UsbBus\UsbBus.lib                                        \
  Output\Edk\Drivers\Usb\Uhci\Uhci.lib                                            \
  Output\Edk\Drivers\Usb\UsbKb\UsbKb.lib                                          \
  Output\Edk\Drivers\Usb\UsbMouse\UsbMouse.lib                                    \
  Output\Edk\Drivers\Usb\UsbMassStorage\UsbMassStorage.lib                        \
  Output\Edk\Drivers\Usb\UsbBot\UsbBot.lib                                        \
  Output\Edk\Drivers\Usb\UsbCbi\Cbi1\Cbi1.lib                                     \
  Output\Edk\Drivers\Usb\UsbCbi\Cbi0\Cbi0.lib                                     \

#
# We sometimes link the shell's tools into this build, so include
# libs for all the tools and the shell.
#
# (Note this is for debugging, and a real build should remove
# this as to be sure it's not mistakenly including any parts of
# the shell)
#

!IF "$(EFI_BOOTSHELL)" == "YES"
EFI_LIBS = $(EFI_LIBS)  \
  Output\Corefw\Fw\Platform\PlDriver\InternalShell\InternalShell.lib  \
  Output\Shell\Attrib\Attrib.lib                                      \
  Output\Shell\Bcfg\Bcfg.lib                                          \
  Output\Shell\Cls\Cls.lib                                            \
  Output\Shell\Comp\Comp.lib                                          \
  Output\Shell\Cp\Cp.lib                                              \
  Output\Shell\Date\Date.lib                                          \
  Output\Shell\Debug\Debug.lib                                        \
  Output\Shell\Dmpstore\Dmpstore.lib                                  \
  Output\Shell\Edit\Edit.lib                                          \
  Output\Shell\Eficompress\Eficompress.lib                            \
  Output\Shell\Efidecompress\Efidecompress.lib                        \
  Output\Shell\Err\Err.lib                                            \
  Output\Shell\Hexedit\Hexedit.lib                                    \
  Output\Shell\Iomod\Iomod.lib                                        \
  Output\Shell\Lib\Lib.lib                                            \
  Output\Shell\Load\Load.lib                                          \
  Output\Shell\LoadBmp\LoadBmp.lib                                    \
  Output\Shell\Loadpcirom\Loadpcirom.lib                              \
  Output\Shell\Ls\Ls.lib                                              \
  Output\Shell\Mem\Mem.lib                                            \
  Output\Shell\Memmap\Memmap.lib                                      \
  Output\Shell\Mkdir\Mkdir.lib                                        \
  Output\Shell\Mode\Mode.lib                                          \
  Output\Shell\Mv\Mv.lib                                              \
  Output\Shell\Newshell\Newshell.lib                                  \
  Output\Shell\Pci\Pci.lib                                            \
  Output\Shell\Reset\Reset.lib                                        \
  Output\Shell\Rm\Rm.lib                                              \
  Output\Shell\Shellenv\Shellenv.lib                                  \
  Output\Shell\Stall\Stall.lib                                        \
  Output\Shell\Time\Time.lib                                          \
  Output\Shell\Type\Type.lib                                          \
  Output\Shell\Ver\Ver.lib                                            \
  Output\Shell\Vol\Vol.lib                                            \
  Output\Shell\GetMTC\GetMTC.lib                                      \
  Output\Shell\SetSize\SetSize.lib                                    \
  Output\Shell\Touch\Touch.lib                                        \
!ENDIF

#
#
#

TARGET_BASENAME=IA-32Emb
OUTPUTS=Bin\$(TARGET_BASENAME).exe

all: vccheck banner genmake output\null.obj helpdata makemaker $(OUTPUTS)

$(OUTPUTS): $(EFI_LIBS)
  $(LINK) $(L_FLAGS) $(NT_LIBS) $(EFI_LIBS) /entry:MainEntry /out:$@ /pdb:$*.pdb
#  $(LIB) $(EFI_LIBS) /out:a.lib


#
# Everything to build
#

#
# Check the version of C2.dll
#
vccheck:
  "$(BUILD_TOOL)\VcCheck"

banner:
  -type $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\IA-32Emb\Banner.c > $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\IA-32Emb\Banner.xxx
  -del $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\IA-32Emb\Banner.c
  -rename $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\IA-32Emb\Banner.xxx banner.c

genmake:
  "$(BUILD_TOOL)\genmake"

output\null.obj: $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c
  $(CC) -c $(EFI_SOURCE)\Corefw\Fw\Efi\Null\Null.c /Fooutput\null.obj

$(EFI_SOURCE)\Shell\HelpData\HelpData.c: $(EFI_SOURCE)\Shell\HelpData\HelpData.src
  $(BUILD_TOOL)\GenHelp $(EFI_SOURCE)\Shell\HelpData\HelpData.src $(EFI_SOURCE)\Shell\HelpData\HelpData.c

helpdata: $(EFI_SOURCE)\Shell\HelpData\HelpData.c

makemaker::
  $(MAKE) -f Output\Lib\Makefile                                                    all
  $(MAKE) -f Output\Edk\Lib\EfiCommonLib\Makefile                                   all
  $(MAKE) -f Output\Edk\Lib\EfiDriverLib\Makefile                                   all
  $(MAKE) -f Output\Edk\Lib\Print\Makefile                                          all
  $(MAKE) -f Output\Edk\Protocol\Makefile                                           all
  $(MAKE) -f Output\Edk\Guid\Makefile                                               all
  $(MAKE) -f Output\Corefw\Fw\Platform\Lib\makefile                                 all

  $(MAKE) -f Output\Corefw\Fw\Efi\Makefile                                          all
  $(MAKE) -f Output\Corefw\Fw\Platform\BootMgr\Default\Makefile                     all
  $(MAKE) -f Output\Corefw\Fw\Platform\BootMgr\Maint\Makefile                       all
  $(MAKE) -f Output\Corefw\Fw\Platform\BuildTip\Ia-32emb\Makefile                   all
  $(MAKE) -f Output\Edk\Drivers\PxeBc\Makefile                                      all
  $(MAKE) -f Output\Edk\Drivers\PxeDhcp4\Makefile                                   all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Cache\Makefile                      all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Cpu_interrupt\Makefile              all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Cpu_timer\Makefile                  all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Defio\Makefile                      all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\EfiLdr\Makefile                     all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\GoVirtual\Makefile                  all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\InterruptController\8259\Makefile   all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\InterruptController\Makefile        all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\NVRAM\Makefile                      all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Reset\Makefile                      all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Stall\8253\Makefile                 all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Time\Dallas1287\Makefile            all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Tpl\Makefile                        all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\Unicode\English\Makefile            all
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\WatchDog\Makefile                   all

  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcl\makefile          all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmcsp\makefile         all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\addins\intel\cssmvl\makefile          all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\cssm\makefile                     all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\port\makefile                     all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\fwk\util\ber_der\r1_1\makefile        all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\icl\makefile                          all
  $(MAKE) -f Output\Edk\Drivers\Bis\oasis\src\integrity\makefile                    all

  $(MAKE) -f Output\Edk\Drivers\Bis\BaseCode\makefile                               all

  $(MAKE) -f Output\Edk\Drivers\CirrusLogic5430\makefile                            all

  $(MAKE) -f Output\Edk\Drivers\Console\ConPlatform\Makefile                        all
  $(MAKE) -f Output\Edk\Drivers\Console\ConSplitter\Makefile                        all
  $(MAKE) -f Output\Edk\Drivers\Console\GraphicsConsole\makefile                    all
  $(MAKE) -f Output\Edk\Drivers\Console\Terminal\Makefile                           all

  $(MAKE) -f Output\Edk\Drivers\Decompress\Makefile                                 all
  $(MAKE) -f Output\Edk\Drivers\DiskIo\Makefile                                     all
  $(MAKE) -f Output\Edk\Drivers\Ebc\Makefile                                        all
  $(MAKE) -f Output\Edk\Drivers\FileSystem\Fat\Makefile                             all
  $(MAKE) -f Output\Edk\Drivers\Ide\Makefile                                        all
  $(MAKE) -f Output\Edk\Drivers\Partition\Makefile                                  all
  $(MAKE) -f Output\Edk\Drivers\PcatPciRootBridge\Makefile                          all
  $(MAKE) -f Output\Edk\Drivers\PciBus\Makefile                                     all
  $(MAKE) -f Output\Edk\Drivers\PciVgaMiniPort\Makefile                             all
  $(MAKE) -f Output\Edk\Drivers\SerialMouse\Makefile                                all
  $(MAKE) -f Output\Edk\Drivers\Snp32_64\Makefile                                   all
  $(MAKE) -f Output\Edk\Drivers\Undi\Makefile                                       all
  $(MAKE) -f Output\Edk\Drivers\VgaClass\Makefile                                   all

  $(MAKE) -f Output\Edk\Drivers\Usb\UsbBus\Makefile                                 all
  $(MAKE) -f Output\Edk\Drivers\Usb\Uhci\Makefile                                   all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbKb\Makefile                                  all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbMouse\Makefile                               all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbMassStorage\Makefile                         all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbBot\Makefile                                 all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbCbi\Cbi1\Makefile                            all
  $(MAKE) -f Output\Edk\Drivers\Usb\UsbCbi\Cbi0\Makefile                            all

  $(MAKE) -f Output\Edk\Drivers\PcatIsaAcpi\Makefile                                all
  $(MAKE) -f Output\Edk\Drivers\IsaBus\Makefile                                     all
  $(MAKE) -f Output\Edk\Drivers\IsaSerial\Makefile                                  all
  $(MAKE) -f Output\Edk\Drivers\IsaFloppy\Makefile                                  all
  $(MAKE) -f Output\Edk\Drivers\Ps2Keyboard\Makefile                                all
  $(MAKE) -f Output\Edk\Drivers\Ps2Mouse\Makefile                                   all

!IF "$(EFI_BOOTSHELL)" == "YES"
  $(MAKE) -f Output\Corefw\Fw\Platform\PlDriver\InternalShell\Makefile              all
  $(MAKE) -f Output\Shell\Lib\Makefile                                              all
  $(MAKE) -f Output\Shell\Attrib\Makefile                                           all
  $(MAKE) -f Output\Shell\Bcfg\Makefile                                             all
  $(MAKE) -f Output\Shell\Cls\Makefile                                              all
  $(MAKE) -f Output\Shell\Comp\Makefile                                             all
  $(MAKE) -f Output\Shell\Cp\Makefile                                               all
  $(MAKE) -f Output\Shell\Date\Makefile                                             all
  $(MAKE) -f Output\Shell\Debug\Makefile                                            all
  $(MAKE) -f Output\Shell\Dmpstore\Makefile                                         all
  $(MAKE) -f Output\Shell\Edit\Makefile                                             all
  $(MAKE) -f Output\Shell\EfiCompress\Makefile                                      all
  $(MAKE) -f Output\Shell\EfiDecompress\Makefile                                    all
  $(MAKE) -f Output\Shell\Err\Makefile                                              all
  $(MAKE) -f Output\Shell\Getmtc\Makefile                                           all
  $(MAKE) -f Output\Shell\Hexedit\Makefile                                          all
  $(MAKE) -f Output\Shell\Iomod\Makefile                                            all
  $(MAKE) -f Output\Shell\LoadPciRom\Makefile                                       all
  $(MAKE) -f Output\Shell\Load\Makefile                                             all
  $(MAKE) -f Output\Shell\LoadBmp\Makefile                                          all
  $(MAKE) -f Output\Shell\Ls\Makefile                                               all
  $(MAKE) -f Output\Shell\Mem\Makefile                                              all
  $(MAKE) -f Output\Shell\Memmap\Makefile                                           all
  $(MAKE) -f Output\Shell\Mkdir\Makefile                                            all
  $(MAKE) -f Output\Shell\Mode\Makefile                                             all
  $(MAKE) -f Output\Shell\Mv\Makefile                                               all
  $(MAKE) -f Output\Shell\Newshell\Makefile                                         all
  $(MAKE) -f Output\Shell\Pci\Makefile                                              all
  $(MAKE) -f Output\Shell\Reset\Makefile                                            all
  $(MAKE) -f Output\Shell\Rm\Makefile                                               all
  $(MAKE) -f Output\Shell\Setsize\Makefile                                          all
  $(MAKE) -f Output\Shell\Shellenv\Makefile                                         all
  $(MAKE) -f Output\Shell\Stall\Makefile                                            all
  $(MAKE) -f Output\Shell\Time\Makefile                                             all
  $(MAKE) -f Output\Shell\Touch\Makefile                                            all
  $(MAKE) -f Output\Shell\Type\Makefile                                             all
  $(MAKE) -f Output\Shell\Ver\Makefile                                              all
  $(MAKE) -f Output\Shell\Vol\Makefile                                              all

  $(MAKE) -f Output\Shell\dumpbs\makefile                                           all
!ENDIF

!IF "$(EFI_BOOTSHELL)" == "NO"
  $(MAKE) -f Output\Shell\Makefile                                                  all
!ENDIF

bsc:
  - dir /s /b *.sbr > temp.rm
  $(BSCMAKE) /n /o Bin\$(TARGET_BASENAME).bsc "@temp.rm"
  - del temp.rm

clean:
  - rd  /s /q output
  - rd  /s /q bin
  - del VC?0.*
  - del $(TARGET_BASENAME).*

#
# Transfer EFILDR to the floppy disk
#
floppy: Bin\Efildr Bin\Bootsect.com
  - copy Bin\Efildr a:\Efildr

#
# Transfer EFILDR to the fat16 partition
#
fat16: Bin\Efildr16 Bin\Bs16.com

#
# Transfer EFILDR to the fat32 partition
#
fat32: Bin\Efildr20 Bin\Bs32.com

#
# Link the uncompressed component that contains the PE/COFF loader and the decompressor
# Afte EFILDR is loaded from disk, this is the component that control is passed.
# It is linked a 4MB with no relocations, and 16 byte alignment is used to reduce its size
#
Bin\Efildr.dll: Output\Corefw\Fw\Platform\BuildTip\Ia-32emb\Efildr.obj 
  link /nologo /MACHINE:X86 /SUBSYSTEM:CONSOLE /NODEFAULTLIB /INCREMENTAL:NO /MAP /FIXED /BASE:0x00010000 /OPT:REF /ALIGN:32 /DRIVER /ENTRY:EfiLoader Output\Corefw\Fw\Platform\BuildTip\Ia-32emb\Efildr.obj /OUT:Bin\Efildr.dll

#
# Change the subsystem type of EFILDR.DLL to EFI_APPLICATION
#
Bin\Efildr.efi: Bin\Efildr.dll
  $(FWIMAGE) app Bin\Efildr.dll Bin\Efildr.efi

#
# CHange the subsystem type of IA-32EMB.EXE to EFI_APPLICATION
#
Bin\IA-32Emb.efi: Bin\IA-32Emb.exe
  $(FWIMAGE) app Bin\IA-32Emb.exe Bin\IA-32Emb.efi

#
# COmpress IA-32EMB.EFI to IA-32EMB.Z
#
Bin\IA-32EMB.Z : Bin\IA-32EMB.efi
  $(EFICOMPRESS) Bin\IA-32EMB.efi Bin\IA-32EMB.Z

#
# Compress UNDI.EFI to UNDI.Z
#
Bin\Undi.Z : Bin\Undi.efi
  $(EFICOMPRESS) Bin\Undi.efi Bin\Undi.Z

#
# Combine EFILDR.EFI+IA-32EMB.Z+UNDI.Z to create EFILDR32
#    EFILDR.EFI is an uncompressed fixed link image that has the PE/COFF loader and decompressor
#    IA-32EMB.Z contains the compressed EFI Core and boot service drivers
#    UNDI.Z     is the one runtime service driver
#
Bin\Efildr32: Bin\Efildr.efi Bin\IA-32Emb.Z Bin\Undi.Z
  $(BUILD_TOOL)\Efildrimage Bin\Efildr32 Bin\Efildr.efi Bin\IA-32Emb.Z Bin\Undi.Z

Bin\BootSect.obj: $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\BootSect.asm
  $(MASM) /c /omf /FoBin\BootSect.obj $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\BootSect.asm 

Bin\BootSect.com: Bin\BootSect.obj
  $(MASMLINK) /tiny Bin\BootSect.obj,Bin\Bootsect.com,Bin\Bootsect.map,,,

Bin\Start.obj: $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Start.asm
  $(MASM) /c /omf /FoBin\Start.obj $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Start.asm 

Bin\Start.com: Bin\Start.obj
  $(MASMLINK) /tiny Bin\Start.obj,Bin\Start.com,Bin\Start.map,,,

Bin\Efi32.obj: $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Efi32.asm
  $(MASM) /c /omf /FoBin\Efi32.obj $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Efi32.asm 

Bin\Efi32.com: Bin\Efi32.obj
  $(MASMLINK) /tiny Bin\Efi32.obj,Bin\Efi32.com,Bin\Efi32.map,,,

Bin\Efi32.com2: Bin\Efi32.com
  $(BUILD_TOOL)\Splitfile Bin\Efi32.com 135168

Bin\Efildr: Bin\Start.com Bin\Efi32.com2 Bin\Efildr32
  copy /b Bin\Start.com+Bin\Efi32.com2+Bin\Efildr32 Bin\Efildr

Bin\BS16.obj: $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\BS16.asm
  $(MASM) /c /omf /FoBin\BS16.obj $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\BS16.asm 

Bin\BS16.com: Bin\BS16.obj
  $(MASMLINK) /tiny Bin\BS16.obj,Bin\Bs16.com,Bin\Bs16.map,,,

Bin\Start16.obj: $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Start16.asm
  $(MASM) /c /omf /FoBin\Start16.obj $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Start16.asm 

Bin\Start16.com: Bin\Start16.obj
  $(MASMLINK) /tiny Bin\Start16.obj,Bin\Start16.com,Bin\Start16.map,,,

Bin\Efildr16: Bin\Start16.com Bin\Efi32.com2 Bin\Efildr32
  copy /b Bin\Start16.com+Bin\Efi32.com2+Bin\Efildr32 Bin\Efildr16

Bin\BS32.obj: $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\BS32.asm
  $(MASM) /c /omf /FoBin\BS32.obj $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\BS32.asm 

Bin\BS32.com: Bin\BS32.obj
  $(MASMLINK) /tiny Bin\BS32.obj,Bin\Bs32.com,Bin\Bs32.map,,,

Bin\Start32.obj: $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Start32.asm
  $(MASM) /c /omf /FoBin\Start32.obj $(EFI_SOURCE)\Corefw\Fw\Platform\BuildTip\BootSector\Start32.asm 

Bin\Start32.com: Bin\Start32.obj
  $(MASMLINK) /tiny Bin\Start32.obj,Bin\Start32.com,Bin\Start32.map,,,

Bin\Efildr20: Bin\Start32.com Bin\Efi32.com2 Bin\Efildr32
  copy /b Bin\Start32.com+Bin\Efi32.com2+Bin\Efildr32 Bin\Efildr20

createfloppy:
  -cd $(BUILD_TOOL)
  -Dskimage -w Efiboot.img
  -cd $(MAKEDIR)

floppytools:
!IF "$(EFI_BOOTSHELL)" == "YES"
  - @echo All EFI Shell tools are built into boot floppy image with EFI_BOOTSHELL=YES
!ELSE
  - @mkdir a:\Efi\Boot
  - @copy /y Bin\Shell.efi a:\Efi\Boot\BootIa32.efi
  - @echo The EFI Shell is now the default removable media boot option on the floppy
!ENDIF

testlib:
  - lib /out:bin\alllib.lib $(EFI_LIBS)

!INCLUDE ..\makefile.common

!IF EXIST (Restricted.mak)
!INCLUDE Restricted.mak
!ENDIF
