/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    init.c

Abstract:

    Main entry point on EFI emulation application



Revision History

--*/


#include "ntemul.h"
#include "plshell.h"
#include "intload.h"
#include "PlWatchDog.h"
#include "Drivers.h"
#include "PlDefio.h"
#include "PlatformLib.h"

//
// prototypes
//
EFI_STATUS
BuildEfiCoreImageHandle (
  IN  VOID                 *EntryPoint,
  IN  VOID                 *BaseAddress,
  IN  UINT64               Size,
  OUT EFI_HANDLE           *CoreImageHandle
  );

EFI_STATUS
PlKillNtConsoles (
    UINTN   ResetCode
    );


STATIC
EFI_STATUS
NtPlResetSystem (
    IN EFI_RESET_TYPE   ResetType,
    IN EFI_STATUS       ResetStatus,
    IN UINTN            DataSize,
    IN CHAR16           *ResetData OPTIONAL
    );

STATIC
VOID
PlInitializeTables (
    VOID
    );

STATIC
VOID
PlInstallMemoryMap (
    VOID
    );

STATIC
VOID
PlInstallBaseDevices (
    VOID
    );

STATIC
VOID
PlIdleLoop (
    IN BOOLEAN  Polling
    );

STATIC
BOOLEAN
PlSetInterruptState (
    IN BOOLEAN  Enable
    );

STATIC
VOID
PlSetVirtualAddressMap (
    IN EFI_CONVERT_POINTER      ConvertPointer,
    IN UINTN                    MemoryMapSize,
    IN UINTN                    DescriptorSize,
    IN UINT32                   DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR    *VirtualMap
    );

VOID
PlInstallSignalHandlers (
    VOID
    );

STATIC
EFI_STATUS
PlStall(
    IN UINTN Microseconds
    );

STATIC
VOID
PlEnableTimerInterrupt(
    VOID
    );

//
//
//

INTN
main (
    INTN        argc,
    CHAR8       **argv,
    CHAR8       **envp
    )
{
    EFI_STATUS      Status;
	EFI_HANDLE      EfiCoreImageHandle;

    //
    // Set the process to the highest priority in the idle class
    //
    SetPriorityClass (GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    //
    // Add the platform support to the EFI FW tables.
    //
    PlInitializeTables();                               // First

    //
    // Initialize the EFI FW memory map
    //
    PlInstallMemoryMap();     
    FW->MemoryMapInstalled();                           // Second

    //
    // Once the memory map has been installed, basic EFI services
    // are now functional.   Although no devices or variable
    // store support is on line.
    //
    InitializeLib (NULL, ST);

    //
    // Build an image handle for the current executing image
    // 
    BuildEfiCoreImageHandle (
      main,
      0,
      0,
      &EfiCoreImageHandle
      );

    PlInitializeWinNtThunkTable ();

    PlInitWatchdogTimer();

    //
    // Install base devices.  This would at least include a 
    // global device_IO device, all NV ram store device(s), and 
    // the timer tick.  It may optionally include other device_io
    // devices.
    //
    PlInstallBaseDevices ();
    FW->NvStoreInstalled ();
    PlInstallSignalHandlers ();
    PlEnableTimerInterrupt ();
    
    //
    // Say we only support English
    //
    Status = RT->SetVariable (  
                    VarLanguageCodes, &EfiGlobalVariable,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    strlena (LanguageCodeEnglish), LanguageCodeEnglish 
                    );
    ASSERT (!EFI_ERROR(Status));

    //
    // Set supported language to English.
    //
    Status = RT->SetVariable (  
                    VarLanguage, &EfiGlobalVariable,
                    EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    strlena (LanguageCodeEnglish), LanguageCodeEnglish 
                    );
    //ASSERT (!EFI_ERROR(Status));
    
    //
    // Set EFIDebug based on NVRAM variable;
    //
    EFIDebugVariable ();

    //
    // Install and connect all built in EFI 1.1 Drivers.
    //
    LOAD_INTERNAL_BS_DRIVER (L"Ebc",                    InitializeEbcDriver);
    LOAD_INTERNAL_BS_DRIVER (L"Decompress",             DecompressDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"WinNtBusDriver",         InitializeWinNtBusDriver);
    LOAD_INTERNAL_BS_DRIVER (L"WinNtSerialIo",          InitializeWinNtSerialIo);

    LOAD_INTERNAL_BS_DRIVER (L"WinNtPciRootBridge",     InitializeWinNtPciRootBridge);
    LOAD_INTERNAL_BS_DRIVER (L"PciBus",                 PciBusEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"WinNtConsole",           InitializeWinNtConsole);
    LOAD_INTERNAL_BS_DRIVER (L"WinNtUga",               InitializeWinNtUga);
    LOAD_INTERNAL_BS_DRIVER (L"GraphicsConsole",        InitializeGraphicsConsole);
    LOAD_INTERNAL_BS_DRIVER (L"Terminal",               InitializeTerminal);
    LOAD_INTERNAL_BS_DRIVER (L"ConPlatform",            ConPlatformDriverEntry);
    LOAD_INTERNAL_BS_DRIVER (L"ConSplitter",            ConSplitterDriverEntry);

    ConnectAllConsoles ();

    //
    // Once consoles are installed, messages may be printed to the consoles
    //
    EFIFirmwareBanner();
    PlPrintLogonBanner();
    Print (L"This image %HMainEntry%N is at address %08x\n",  (UINTN)main);

    //
    // Install remaining EFI 1.1 Drivers
    //
    LOAD_INTERNAL_BS_DRIVER (L"WinNtSimpleFileSystem",  InitializeWinNtSimpleFileSystem);
    LOAD_INTERNAL_BS_DRIVER (L"WinNtBlockIo",           InitializeWinNtBlockIo);
    LOAD_INTERNAL_BS_DRIVER (L"DiskIo",                 DiskIoDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Partition",              PartitionEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Fat",                    FatEntryPoint);

    //
    // Install all built in EFI 1.1 Drivers that require EFI Variable Services
    //
    LOAD_INTERNAL_BS_DRIVER (L"BIS",                    EFIBIS_BaseCodeModuleInit);
    LOAD_INTERNAL_BS_DRIVER (L"SerialMouse",            SerialMouseDriverEntryPoint);

#if EFI_BOOTSHELL
    PlInitializeInternalLoad();
#endif

    // 
    // loop thru boot manager and boot maintenance until a boot
    // option is selected
    //
    while (TRUE) {
        //
        // The platform code is ready to boot the machine. Pass control
        // to the boot manager
        // 
        LOAD_INTERNAL_DRIVER(
            FW,
            IMAGE_SUBSYSTEM_EFI_APPLICATION,
            L"bootmgr",
            InitializeBootManager
            );

        //
        // If we return from above, means that no boot choices were found
        // or boot maintenance chosen. hence invoke boot maintenance menu
        // 
        LOAD_INTERNAL_DRIVER(
            FW,
            IMAGE_SUBSYSTEM_EFI_APPLICATION,
            L"bmaint",
            InitializeBootMaintenance
            );
    }

    return 0;
}

#if EFI_BOOTSHELL
//
// Global Data
//
STATIC BOOLEAN  ShellToolsLoaded = FALSE;


VOID
PlStartShell (
    VOID
    )
{
    if (ShellToolsLoaded == FALSE) {
        PlLoadShellTools();
        PlLoadShellDebugTools();
        PlLoadShellAdditionalTools();
        ShellToolsLoaded = TRUE;
    }
    PlLoadShell();
}
#endif

STATIC
VOID
PlInitializeTables (
    VOID
    )
/*++

Routine Description:

    Initialization function called to obtain and update any 
    global table information or entry points that are in:

        SystemTable
        SystemTable->BootServices
        SystemTAble->RuntimeServices

Arguments:

    None

Returns:

    Tables set and updated

--*/
{

    //
    // Initialize the platform table
    //

    PlTable.EmulateLoad          = PlEmulateLoad;
    PlTable.IdleLoop             = PlIdleLoop;
    PlTable.SetInterruptState    = PlSetInterruptState;
    PlTable.SetVirtualAddressMap = NULL;
    PlTable.SI_HandoffState      = NULL;
    PlTable.EI_ReturnState       = NULL;

    //
    // Call the EFI FW's entry point and get back the initial
    // Firwmare and SystemTable
    //

    EFIEntryPoint (&PlTable, &FW, &ST);  
    BS = ST->BootServices;
    RT = ST->RuntimeServices;

    //
    // Update / provide system table information.
    // The core EFI firmware does not provide the following 
    // system table information and should be filled in here:
    //          MpsTable
    //          AcpiTable
    //          SMBIOSTable
    //          OEMVendorGuid
    //          OEMSystemTable
    //

    ST->NumberOfTableEntries = 0;
    ST->ConfigurationTable   = NULL;

    //
    // Update / provide any entries in the boot services table
    // The core EFI firmware does not provide the following
    // entries in the boot services table:
    //          Stall
    //

    BS->Stall = PlStall;
    BS->SetWatchdogTimer = PlSetWatchdogTimer;

    //
    // Update / provide any entries in the runtime service table.
    // The core EFI firmware does not provide the following
    // entries in the runtime services table:
    //          GetTime
    //          SetTime
    //          GetWakeupTime
    //          SetWakeupTime
    //

    RT->GetTime       = PlGetTime;
    RT->SetTime       = PlSetTime;
    RT->GetWakeupTime = PlGetWakeupTime;
    RT->SetWakeupTime = PlSetWakeupTime;

    RT->ResetSystem   = NtPlResetSystem;

    //
    // Done with table initializations
    //
}

STATIC
VOID
PlInstallMemoryMap (
    VOID
    )
/*++

Routine Description:

    Called to initialize the memory map descriptors for
    the system's memory map.   

    N.B. The FW requires that the first call be of type
    "ConventialMemory". 

    N.B. The FW requires that page 0 in the map is not 
    not ConventialMemory.  (Note the emulator can work
    around this by reporting page 0 as BootServicesData
    memory if needed.  The page is then not used at 
    Boot Services time, but it available in the memory
    map for later use)

Arguments:

    None

Returns:

    Memory map descriptors added

--*/
{
    HANDLE      File, Map;
    VOID        *View;
    UINT32      MapSize;

    //
    // Map in a large chunk of memory, then provide it to the
    // FW as our memory map
    //

    MapSize = 0x1400000;         // 20mb of space
    File = CreateFile(L"corefile", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_ALWAYS, 0, NULL);
    Map  = CreateFileMapping(File, NULL, PAGE_READWRITE, 0, MapSize, L"ntefi_core");
    View = MapViewOfFileEx (Map, FILE_MAP_ALL_ACCESS, 0, 0, MapSize, NULL);

    //
    // Add some memory descriptors
    //

    FW->AddMemoryDescriptor (EfiConventionalMemory, (UINTN) View, MapSize / 4096, EFI_MEMORY_WB);
}

STATIC
VOID
PlInstallBaseDevices (
    VOID
    )
/*++

Routine Description:

    Add handles to the base devices here. In particular
    the console devices, internal nvram device(s), and
    the timer tick handler.

Arguments:

    None

Returns:

    Base device handles added

--*/
{
    EFI_DEVICE_PATH                 *DevicePath;
    EFI_HANDLE                      Handle;
    EFI_STATUS                      Status;

    //
    //
    // First add a device(s) to handle device_io request
    // (this is done since the next devices may need to access
    // their IO space)
    //

    // On a PC there's only global IO, and the firwmare is
    // nice enough to provide us with a driver that does just
    // that...  add it

    PlInstallDefaultIoDevice (
                        BiosRootDevicePath, 
                        0x0000000000000000, // Force non cachable access
                        0x0000000000000000  
                        );

    //
    // Set up the GlobalIoFncs global to point to global memory/io/pci space
    //
    DevicePath = BiosRootDevicePath;
    Status = BS->LocateDevicePath (&DeviceIoProtocol, &DevicePath, &Handle);
    if (!EFI_ERROR(Status)) {
        Status = BS->HandleProtocol (Handle, &DeviceIoProtocol, &GlobalIoFncs);
    }
    ASSERT (!EFI_ERROR(Status));

    //
    // Initialize to PCI Root Bus List
    //

    InitializePciRootBusList (&PciRootBusList);    
 
    //
    // BUGBUG: add the timer tick handler here
    //
        
    //
    // Add the internal NvRam device
    //

    PlInitNvVarStoreEmul ( 
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        0x4000, 2
        );

    //
    // Install the Unicode string device.  This device supports
    // case insensitive comparisons of Unicode strings.
    //

    PlInitializeUnicodeStringDevice();
}

STATIC
VOID
PlExitBootServices (
    IN EFI_EVENT        Event,
    IN VOID             *Context
    )
{
    //
    // Clear non-runtime pointer
    //

    PlTable.EmulateLoad = NULL;
    PlTable.IdleLoop = NULL;
    PlTable.SetInterruptState = NULL;

    // EFI f/w takes care of boot service's table
}

STATIC
VOID
PlSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    )
{
    EFI_CONVERT_POINTER     ConvertPointer;
  
    //
    // Different components are updating themselves to work in the 
    // new memory mappings.  The only function that is safe to call
    // during this notification is ConvertPointer.   
    //

    ConvertPointer = RT->ConvertPointer;

    //
    // Fix any runtime pointers
    //


    //
    // Notify Lib of change
    //

    RtLibEnableVirtualMappings ();
}


VOID
PlInstallSignalHandlers (
    VOID
    )
{
    EFI_STATUS          Status;
    EFI_EVENT           Event;

    //
    // Create an event to be signalled when ExitBootServices occurs
    //

    Status = BS->CreateEvent(
                    EVT_SIGNAL_EXIT_BOOT_SERVICES, 
                    TPL_NOTIFY,
                    PlExitBootServices,
                    NULL,
                    &Event
                    );

    ASSERT (!EFI_ERROR(Status));

    //
    // Create an event to be signalled when SetVirtualAddressMap occurs
    // N.B. We must pass the "runtime" table pointer to use during the
    // notification
    //

    Status = BS->CreateEvent(
                    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                    TPL_NOTIFY,
                    PlSetVirtualMapping,
                    NULL,
                    &Event
                    );

    ASSERT (!EFI_ERROR(Status));
}


VOID
PlIdleLoop (
    IN BOOLEAN  Polling
    )
{
  Sleep(5);
}

BOOLEAN
PlSetInterruptState (
    IN BOOLEAN      Enable
    )
{
    BOOLEAN         PreviousState;

    PreviousState = NtInterruptState;
    NtInterruptState = Enable;
    return PreviousState;
}

STATIC
VOID
PlSetVirtualAddressMap (
    IN EFI_CONVERT_POINTER      ConvertPointer,
    IN UINTN                    MemoryMapSize,
    IN UINTN                    DescriptorSize,
    IN UINT32                   DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR    *VirtualMap
    )
{
    // fyi.. can't use DEBUG print in here because everyhting but
    // the base firmware has been fixedup for virtual mode
}


STATIC
EFI_STATUS
PlStall(
    IN UINTN Microseconds
    )
{
    UINTN ms;

    ms = Microseconds / 1000 + (Microseconds % 1000 ? 1 : 0);
    Sleep(ms);
    return EFI_SUCCESS;
}

#define TIMER_TICK_DURATION 10
STATIC HANDLE  hMainThread;

DWORD WINAPI
TimerThread(
    LPVOID lpParameter   // thread data
    )
{
    EFI_TPL     OriginalTPL;
    DWORD       CurrentTick, Delta;

    //
    //  Set our thread priority higher than the "main" thread.
    //

    if (!SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
        printf("SetThreadPriority(HIGH) returned %d\n", GetLastError());

    while (1) {

        //
        //  Wait the appropriate interval
        //

        Sleep( TIMER_TICK_DURATION );

        //
        //  Suspend the main thread until we are done
        //

        (void)SuspendThread(hMainThread);

        while (NtInterruptState == FALSE) {

            //
            //  Resume the main thread
            //

            (void)ResumeThread(hMainThread);

            //
            //  Wait for interrupts to be enabled.
            //

            while (NtInterruptState == FALSE)
                Sleep(0);

            //
            //  Suspend the main thread until we are done
            //

            (void)SuspendThread(hMainThread);
        }

        // 
        //  Get the current system tick
        //

        CurrentTick = GetTickCount ();
        Delta = CurrentTick - NtLastTick;
        NtLastTick = CurrentTick;

        // 
        //  If delay was more then 1 second, pitch it (probably debugging)
        //

        if (Delta < 1000) {

            OriginalTPL = BS->RaiseTPL (TPL_HIGH_LEVEL );

            // 
            //  Inform the firmware of an "timer interrupt".  The time
            //  expired since the last call is 10,000 times the number
            //  of ms.  (or 100ns units)
            //

            FW->TickHandler (Delta * 10000);

            BS->RestoreTPL (OriginalTPL);
        }

        //
        //  Resume the main thread
        //

        (void)ResumeThread(hMainThread);
    }
}

STATIC VOID
PlEnableTimerInterrupt(
    VOID
    )
{
    HANDLE  hTimerThread;
    DWORD   dwThreadId;

    // 
    //  Initialize timer interrupts
    //  (N.B. in the NT emulation environment we only need to get the
    //  starting tick location)
    //

    NtLastTick = GetTickCount();

    //
    //  Get our handle so the timer tick thread can suspend us.
    //

    if (!DuplicateHandle(
                GetCurrentProcess(),  // handle to source process
                GetCurrentThread(),   // handle to duplicate
                GetCurrentProcess(),  // handle to target process
                &hMainThread,         // duplicate handle
                0,                    // requested access
                FALSE,                // handle inheritance option
                DUPLICATE_SAME_ACCESS // optional actions
                )) {
        printf("DuplicateHandle returned error %d\n", GetLastError());
        return;
    }

    //
    //  Create a thread to simulate timer interrupts
    //

    hTimerThread = CreateThread(
                        NULL,
                        0,
                        TimerThread,
                        NULL,
                        0,
                        &dwThreadId );
    if (hTimerThread == NULL) {
        printf("Error %d creating TimerThread\n", GetLastError());
    }
}

STATIC
EFI_STATUS
NtPlResetSystem (
    IN EFI_RESET_TYPE   ResetType,
    IN EFI_STATUS       ResetStatus,
    IN UINTN            DataSize,
    IN CHAR16           *ResetData OPTIONAL
    )
{
    ExitProcess((UINT)ResetType);  
    return EFI_SUCCESS;
}
