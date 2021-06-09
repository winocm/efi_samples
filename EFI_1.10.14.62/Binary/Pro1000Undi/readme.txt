Intel PRO/1000 EFI Device Driver


XYY in the filenames below indicates the version number of the driver.

eXYYb2.efi: This is the EBC (EFI Byte Code) executable, which is capable of
  running on both IA32, and IA64 processor based systems.

eXYYe2.efi: This version of the EFI driver is intended solely for IA32
  processor based systems.  The e207e2.efi driver is identical to what OEM
  customers will include in their EFI32 bios.

eXYYi2.efi: This version of the EFI driver is intended solely for IA64
  processor based systems.  The e207i2.efi driver is identical to what OEM
  customers will include in their EFI64 bios.

Prerequisites
 Before you can use any of the EFI drivers you must have a system with an EFI based
 BIOS, or an EFI boot floppy.  For more information, please see the Intel EFI web page.

 Once you have an EFI enabled system you need to make sure the EFI Simple Network
 Protocol (referred to as SNP in the rest of this document) driver is loaded. This is
 done by typing the EFI "drivers" command.  "SNP3264.efi" will be listed if the SNP
 driver is loaded.  The job of the SNP driver is to manage connections to all of the
 hardware specific network drivers to the OS.

Loading the EFI Network Driver
 Once you have an EFI enabled system with the SNP driver loaded you are ready to load
 the EFI Network driver.  The EFI "load" command is used to load the network driver:
 load E206B2.efi

 After the load command completes a message will appear indicating whether the driver
 load was successful.  If the load was successful, execute the "drivers" command to see
 whether the network driver loaded.   The screen below shows a sample output from the
 EFI "drivers" command.  Driver handle A1 in the "DRV" column shows that the E206B2.efi
 driver loaded successfully.

            T   D
D           Y C I
R           P F A
V  VERSION  E G G #D #C DRIVER NAME                         IMAGE NAME
== ======== = = = == == =================================== ===================
18 00000010 B - -  5 38 PCI Bus Driver                       PciBus
19 00000010 D - -  1  - PC-AT ISA Device Enumeration Driver  PcatIsaAcpi
1A 00000010 B - -  1  3 ISA Bus Driver                       IsaBus
1B 00000010 B - -  2  2 ISA Serial Driver                    IsaSerial
1C 00000000 ? - -  -  - BIOS[INT10] VGA Mini Port Driver     BiosVgaMiniPort
1D 00000010 ? - -  -  - VGA Class Driver                     VgaClass
1E 00000010 D - -  1  - UGA Console Driver                   GraphicsConsole
1F 00000010 D - -  2  - Serial Terminal Driver               Terminal
20 00000010 D - -  1  - Platform Console Management Driver   ConPlatform
25 00000010 B - -  1  1 Console Splitter Driver              ConSplitter
2C 00000010 D - -  2  - Usb Uhci Driver                      UsbUhci
2D 00000010 B - -  2  3 USB Bus Driver                       UsbBus
2E 00000010 ? - -  -  - Usb Bot Mass Storage Driver          UsbBot
2F 00000010 ? - -  -  - <UNKNOWN>                            UsbCbi1
30 00000010 D - -  1  - Usb Keyboard Driver                  UsbKeyboard
31 00000114 D - -  1  - ATI Rage XL UGA Driver               PciRom Seg=00000000
60 00000010 D - -  2  - Generic Disk I/O Driver              DiskIo
61 00000010 ? - -  -  - Partition Driver(MBR/GPT/El Torito)  Partition
62 00000010 D - -  1  - FAT File System Driver               Fat
64 00000030 B - -  1  1 Intel(R) PRO/1000 v1.20 EFI-64       GigUndi
65 00000010 D - -  1  - Simple Network Protocol Driver       Snp3264
66 00000010 D - -  1  - PXE Base Code Driver                 PxeBc
67 00000010 D - -  1  - PXE DHCPv4 Driver                    PxeDhcp4
68 00000010 D - -  1  - Usb Mouse Driver                     UsbMouse
78 01020300 D X X  2  - LSI Logic Ultra320 SCSI Driver       VenHw(6D9FEEB1-E585
A1 00002100 ? - -  -  - Intel(R) PRO/1000 v2.10 EFI-64       \/E210B2.efi

 The #D and #C fields indicate whether the driver successfully bound to a device and
 created a child handle respectively.  In the screen above these field are blank
 indicating the E206B2.efi network driver has not bound to a device.  In the above
 screen the driver did not bind because a previous version of the network driver was
 already bound to the device (handle 64 above).  Remedying this problem is described in
 the next section.

Connecting the EFI Network Driver
 Once the driver is loaded it needs to bind to the network adapter.  If an earlier
 version of the driver was already loaded then the EFI "reconnect" command must be used
 to reconnect all drivers in the system so that the network adapter will bind to the
 newer version of the driver loaded in the system.  The "reconnect" command is used:
 reconnect -r

 The "-r" parameter indicates that a recursive reconnect be performed. When the
 reconnect command completes it will report whether the reconnection was successful.
 The EFI "drivers" command should be used to verify whether the network adapter
 successfully rebound to the new driver.

Unloading the EFI Network Driver
 To unload a network driver from memory the EFI "unload" command is used.  The syntax
 for using the unload command is as follows: "unload [driver handle]", where driver
 handle is the number assigned to the driver in the far left column of the "drivers"
 output screen. 

Intel EFI Network Driver naming convention
 The Intel EFI drivers all follow a naming convention allowing the driver version,
 driver type, and the hardware supported by the driver to be determined by looking at
 the filename.  Below is the decoder for the driver filenames:

Exyytn.efi

where:   x   is the major version number of the EFI driver in decimal
         yy  is the minor version number of the EFI driver in decimal

         t  is the type of driver
           B - EFI EBC UNDI driver only
           E - EFI32 specific UNDI only
           I - EFI64 specific UNDI only

         n is the general type of adapter the driver is for
           1 - Fast Ethernet
           2 - Gigabit
           4 - 10 Gigabit
           6 - TCP Offload Engine (TOE)

 Examples: 
   E206B2.efi - Version 2.06 of EBC EFI driver for PRO/1000
   E103I4.efi - Version 1.03 of EBC EFI driver for PRO/10GBE
   E001E6.efi - Version 0.01 of EFI32 driver for TOE

