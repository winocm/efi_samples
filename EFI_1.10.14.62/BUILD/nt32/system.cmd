REM #########################################################################
REM #
REM # This following section documents the envirnoment variables for the Win NT 
REM # build.  These variables are used to define the (virtual) hardware 
REM # configuration of the NT environment
REM #
REM # A ! can be used to seperate multiple instances in a variable. Each 
REM # instance represents a seperate hardware device. 
REM #
REM # EFI_WIN_NT_PHYSICAL_DISKS - maps to drives on your system
REM # EFI_WIN_NT_VIRTUAL_DISKS  - maps to a device emulated by a file
REM # EFI_WIN_NT_FILE_SYSTEM    - mouts a directory as a file system
REM # EFI_WIN_NT_CONSOLE        - make a logical comand line window (only one!)
REM # EFI_WIN_NT_UGA            - Builds UGA Windows of Width and Height
REM # EFI_WIN_NT_SERIAL_PORT    - maps physical serial ports
REM #
REM #  <F>ixed       - Fixed disk like a hard drive.
REM #  <R>emovable   - Removable media like a floppy or CD-ROM.
REM #  Read <O>nly   - Write protected device.
REM #  Read <W>rite  - Read write device.
REM #  <block count> - Decimal number of blocks a device supports.
REM #  <block size>  - Decimal number of bytes per block.
REM #
REM #  NT envirnonment variable contents. '<' and '>' are not part of the variable, 
REM #  they are just used to make this help more readable. There should be no 
REM #  spaces between the ';'. Extra spaces will break the variable. A '!' is  
REM #  used to seperate multiple devices in a variable.
REM #
REM #  EFI_WIN_NT_VIRTUAL_DISKS = 
REM #    <F | R><O | W>;<block count>;<block size>[!...]
REM #
REM #  EFI_WIN_NT_PHYSICAL_DISKS =
REM #    <drive letter>:<F | R><O | W>;<block count>;<block size>[!...]
REM #
REM #  Virtual Disks: These devices use a file to emulate a hard disk or removable
REM #                 media device. 
REM #                 
REM #    Thus a 20 MB emulated hard drive would look like:
REM #    EFI_WIN_NT_VIRTUAL_DISKS=FW;40960;512
REM #
REM #    A 1.44MB emulated floppy with a block size of 1024 would look like:
REM #    EFI_WIN_NT_VIRTUAL_DISKS=RW;1440;1024
REM #
REM #  Physical Disks: These devices use NT to open a real device in your system
REM #
REM #    Thus a 120 MB floppy would look like:
REM #    EFI_WIN_NT_PHYSICAL_DISKS=B:RW;245760;512
REM #
REM #    Thus a standard CD-ROM floppy would look like:
REM #    EFI_WIN_NT_PHYSICAL_DISKS=Z:RO;307200;2048
REM #
REM #  EFI_WIN_NT_FILE_SYSTEM = 
REM #    <directory path>[!...]
REM #
REM #    Mounting the two directories C:\FOO and C:\BAR would look like:
REM #    EFI_WIN_NT_FILE_SYSTEM=c:\foo!c:\bar
REM #
REM #  EFI_WIN_NT_CONSOLE = 
REM #    <window title>
REM #
REM #    Declaring a text console window with the title "My EFI Console" woild look like:
REM #    EFI_WIN_NT_CONSOLE=My EFI Console
REM #
REM #  EFI_WIN_NT_UGA = 
REM #    <width> <height>[!...]
REM #
REM #    Declaring a two UGA windows with resolutions of 800x600 and 1024x768 would look like:
REM #    Example : EFI_WIN_NT_UGA=800 600!1024 768
REM #
REM #  EFI_WIN_NT_SERIAL_PORT = 
REM #    <port name>[!...]
REM #
REM #    Declaring two serial ports on COM1 and COM2 would look like:
REM #    Example : EFI_WIN_NT_SERIAL_PORT=COM1!COM2
REM #
REM #########################################################################

REM #########################################################################
REM # Settings for virtual devices in the NT Emulation Environment
REM #########################################################################

set EFI_WIN_NT_PHYSICAL_DISKS=a:RW;2880;512
set EFI_WIN_NT_VIRTUAL_DISKS=RW;2880;512
set EFI_WIN_NT_FILE_SYSTEM=.
set EFI_WIN_NT_CONSOLE="EFI Console Window"
set EFI_WIN_NT_UGA="800 600"
set EFI_WIN_NT_SERIAL_PORT=COM1!COM2

REM #########################################################################
REM # Echo settings to the screen
REM #########################################################################

echo.
echo ************************************************************************
echo * Supported Build Commands                                             *
echo ************************************************************************
echo *     nmake                 - Incremental compile and link             *
echo *     nmake clean           - Remove all OBJ, LIB, EFI, and EXE files  *
echo *     nmake run             - Execute EFI                              *
echo *     nmake runloop         - Execute EFI with simulated resets        *
echo *     nmake bsc             - Create Browse Information File           *
echo ************************************************************************
echo EFI_WIN_NT_PHYSICAL_DISKS=%EFI_WIN_NT_PHYSICAL_DISKS%
echo EFI_WIN_NT_VIRTUAL_DISKS=%EFI_WIN_NT_VIRTUAL_DISKS%
echo EFI_WIN_NT_FILE_SYSTEM=%EFI_WIN_NT_FILE_SYSTEM%
echo EFI_WIN_NT_CONSOLE=%EFI_WIN_NT_CONSOLE%
echo EFI_WIN_NT_UGA=%EFI_WIN_NT_UGA%
echo EFI_WIN_NT_SERIAL_PORT=%EFI_WIN_NT_SERIAL_PORT%
