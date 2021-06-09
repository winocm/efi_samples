/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
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

    Main entry point on EFI 64-bit environment. This environment layers 
    on top of the SAL_A

Revision History

--*/

#include "Sal64.h"
#include "plshell.h"
#include "SalHandoff.h"
#include "intload.h"
#include "Drivers.h"
#include "PlDefio.h"
#include "PlatformLib.h"

#define EFI_STACK_SIZE_IN_PAGES           (0x20000 / EFI_PAGE_SIZE)   // 128 KB
#define EFI_BSP_SIZE_IN_PAGES             (0x4000 / EFI_PAGE_SIZE)    // 16 KB
#define EFI_INTERRUPT_STACK_SIZE_IN_PAGES (0x20000 / EFI_PAGE_SIZE)   // 128 KB
#define EFI_INTERRUPT_BSP_SIZE_IN_PAGES   (0x4000 / EFI_PAGE_SIZE)    // 16 KB

typedef struct {
    UINT64 EfiStackAddr;
    UINT64 EfiBspAddr;
    UINT64 EfiIntStackAddr;
    UINT64 EfiIntBspAddr;
} EFI_STACK_INFO;

EFI_STACK_INFO EfiStackInfo;

EFI_PCI_OPTION_ROM_TABLE PciOptionRomTable;

EFI_STATUS
BuildEfiCoreImageHandle (
  IN  VOID                 *EntryPoint,
  IN  VOID                 *BaseAddress,
  IN  UINT64               Size,
  OUT EFI_HANDLE           *CoreImageHandle
  );

STATIC
VOID
PlInitializeTables (
    IN  UINTN   FunctionId
    );

STATIC
VOID
PlInstallMemoryMap (
    IN  UINTN                   FunctionId,
    IN  UINTN                   NoDesc,
    IN  EFI_MEMORY_DESCRIPTOR   *Desc
    );

STATIC
VOID
PlInstallBaseDevices (
    IN  UINTN   FunctionId,
    IN  UINTN   NvramBanks,
    IN  UINTN   NvramSize
    );

//
//
//

STATIC
VOID
PlExitBootServices (
    IN EFI_EVENT        Event,
    IN VOID             *Context
    );

VOID    
TimerCallBack (
    IN UINTN ElapsedTime
    );

VOID
PlAddBootOptionDefaults (
    IN  UINT16  BootOptionNumber
    );

//
//
//

rArg
MainEntry (
    IN UINTN                    FunctionId,
    IN SALEFIHANDOFF            *SalEfiHandoff
    )
/*++

  Routine Description:

    This is the entry point after this code has been loaded into memory. 

Arguments:
    FunctionId -    0x00 - SAL Call's just return right away
                    0x01 - SAL to EFI handoff BIOS INT 13 and INT 16 drivers loaded
                    0x11 - SAL to EFI handoff Cache Disabled same as 0x01
                    0x02 - SAL to EFI handoff Native Keyboard & IDE Drivers
    BugBug: These need to be in a common include file with the SAL. This is
            just implementation detail and not architecture.
    
    SalEfiHandoff - Info passed to EFI from Sal When it's launched.

Returns:

    Calls into EFI Firmware

--*/
{
    rArg        RetParam = {0, 0, 0, 0};
    EFI_STATUS  Status;
    EFI_EVENT   Event;
    EFI_HANDLE  EfiCoreImageHandle;


POST_CODE(0xaf);
    /* 
     *  for now stub out call with FunctionId == 0
     */
    if(FunctionId == 0) {
        //
        // This call back allows the SAL to call EFI incrementally.
        // Function ID zero is called but not used, so we just return.
        // This could be used to call EFI right after memory init, but
        // before the legacy BIOS was initialized for example...
        //

        //
        // Add the platform support to the EFI FW tables.
        //
        PlInitializeTables(FunctionId);

        //
        // Initialize the EFI FW memory map
        //
POST_CODE(0x01);
        PlInstallMemoryMap (FunctionId, SalEfiHandoff->MemDescCount, SalEfiHandoff->MemDesc);
POST_CODE(0x02);
        FW->MemoryMapInstalled();                           // Second
POST_CODE(0x03);

        Status = BS->AllocatePages (AllocateAnyPages, 
                                    EfiBootServicesData,
                                    EFI_STACK_SIZE_IN_PAGES,
                                    &EfiStackInfo.EfiStackAddr);
        EfiStackInfo.EfiStackAddr += (EFI_STACK_SIZE_IN_PAGES * EFI_PAGE_SIZE - 0x10);

        Status = BS->AllocatePages (AllocateAnyPages, 
                                    EfiBootServicesData,
                                    EFI_BSP_SIZE_IN_PAGES,
                                    &EfiStackInfo.EfiBspAddr);

        Status = BS->AllocatePages (AllocateAnyPages, 
                                    EfiBootServicesData,
                                    EFI_INTERRUPT_STACK_SIZE_IN_PAGES,
                                    &EfiStackInfo.EfiIntStackAddr);
        EfiStackInfo.EfiIntStackAddr += (EFI_INTERRUPT_STACK_SIZE_IN_PAGES * EFI_PAGE_SIZE - 0x10);

        Status = BS->AllocatePages (AllocateAnyPages, 
                                    EfiBootServicesData,
                                    EFI_INTERRUPT_BSP_SIZE_IN_PAGES,
                                    &EfiStackInfo.EfiIntBspAddr);

POST_CODE(0x04);
        RetParam.p1 = (UINT64)(&(EfiStackInfo.EfiStackAddr));

        return (RetParam);
    }
    // 
    //  Initialize the configuration tables in the EFI System Table
    //
    //  The ACPI Table added here is the ACPI 1.0 Table.  If an ACPI 2.0 table is present in the system,
    //  an additional table would have to be installed.  If the SAL passes an ACPI 2.0 table instead of
    //  an ACPI 1.0 table, then the GUID would have to be changed to the ACPI 2.0 GUID.
    //
POST_CODE(0x04);
    BS->InstallConfigurationTable(&SalSystemTableGuid, SalEfiHandoff->SalSystemTable);
    BS->InstallConfigurationTable(&AcpiTableGuid,      SalEfiHandoff->AcpiTable);
    BS->InstallConfigurationTable(&Acpi20TableGuid,    FindAcpiRsdPtr());
    BS->InstallConfigurationTable(&MpsTableGuid,       SalEfiHandoff->MpsTable);
    BS->InstallConfigurationTable(&SMBIOSTableGuid,    FindSMBIOSPtr());
    if (SalEfiHandoff->PciOptionRomCount > 0) {
      PciOptionRomTable.PciOptionRomCount       = SalEfiHandoff->PciOptionRomCount;
      PciOptionRomTable.PciOptionRomDescriptors = SalEfiHandoff->PciOptionRomDescriptors;
      BS->InstallConfigurationTable (&gEfiPciOptionRomTableGuid, &PciOptionRomTable);
    }

POST_CODE(0x05);
    PlInitSalPalProc ((UINT64)SalEfiHandoff->SALCallBack, 0x12345678abcdef0);

    // 
    //  Initialize timer interrupts
    //
POST_CODE(0x06);
    PlTimer(ID_SALCB_TIMER_START, TIMER_PERIOD, (PLABEL *)TimerCallBack);

    //
    // Once the memory map has been installed, basic EFI services
    // are now functional.   Although no devices or variable
    // store support is on line.
    //
POST_CODE(0x07);
    InitializeLib (NULL, ST);

    //
    // Build an image handle for the current executing image
    // 
    BuildEfiCoreImageHandle (
      MainEntry,
      SalEfiHandoff->EfiCoreBaseAddress,
      SalEfiHandoff->EfiCoreLength,
      &EfiCoreImageHandle
      );

POST_CODE(0x08);
    PlInitWatchdogTimer();

    //
    // Initialize support for calling BIOS functions
    //
POST_CODE(0x09);
    InitializeBiosIntCaller ();

    //
    // Install base devices.  This would at least include a 
    // global device_IO device, all NV ram store device(s), and 
    // the timer tick.  It may optionally include other device_io
    // devices.
    //
POST_CODE(0x0a);
    PlInstallBaseDevices(FunctionId, 
                         SalEfiHandoff->NVRAMBanks, 
                         SalEfiHandoff->NVRAMSize);                   
POST_CODE(0x0b);
    FW->NvStoreInstalled();                             // Third

    //
    // Say we only support English
    //
POST_CODE(0x0e);
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

POST_CODE(0x0f);
    InitializeLib (NULL, ST);

    //
    // Install and connect all built in EFI 1.1 Drivers.
    //
POST_CODE(0x10);
    LOAD_INTERNAL_BS_DRIVER (L"BiosDisk",               InstallBiosBlkIoDrivers);

    LOAD_INTERNAL_BS_DRIVER (L"Ebc",                    InitializeEbcDriver);
    LOAD_INTERNAL_BS_DRIVER (L"Decompress",             DecompressDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"PcatPciRootBridge",      InitializePcatPciRootBridge);
    LOAD_INTERNAL_BS_DRIVER (L"PciBus",                 PciBusEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"PcatIsaAcpi",            PcatIsaAcpiDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"IsaBus",                 IsaBusControllerDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"IsaSerial",              SerialControllerDriverEntryPoint);

#ifdef SOFT_SDV
    LOAD_INTERNAL_BS_DRIVER (L"BiosVga",                BiosVgaDriverEntryPoint);
#else
    LOAD_INTERNAL_BS_DRIVER (L"BiosVgaMiniPort",        BiosVgaMiniPortDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"VgaClass",               VgaClassDriverEntryPoint);
#endif
//  LOAD_INTERNAL_BS_DRIVER (L"CirrusLogic5430",        CirrusLogic5430UgaDrawDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"GraphicsConsole",        InitializeGraphicsConsole);
    LOAD_INTERNAL_BS_DRIVER (L"Terminal",               InitializeTerminal);
    LOAD_INTERNAL_BS_DRIVER (L"ConPlatform",            ConPlatformDriverEntry);
    LOAD_INTERNAL_BS_DRIVER (L"ConSplitter",            ConSplitterDriverEntry);
    LOAD_INTERNAL_BS_DRIVER (L"BiosKeyboard",           BiosKeyboardDriverEntryPoint);

POST_CODE(0x11);
    ConnectAllConsoles ();

    //
    // Once consoles are installed, message may be printed
    // to the consoles
    //
POST_CODE(0x12);
    EFIFirmwareBanner();
    PlPrintLogonBanner();
    if ((FunctionId & SAL_EFI_CACHE_DISABLED) == SAL_EFI_CACHE_DISABLED) {
        Print (L"%ECache Disabled%N. ");
    } else {
        Print (L"%ECache Enabled%N. ");
    }
    Print (L"This image %HMainEntry%N is at address %016x\n",  *( UINT64 *)MainEntry);

POST_CODE(0x13);
    LOAD_INTERNAL_BS_DRIVER (L"DiskIo",                 DiskIoDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Partition",              PartitionEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Fat",                    FatEntryPoint);
    LOAD_INTERNAL_RT_DRIVER (L"Undi",                   InitializeUNDIDriver );
    LOAD_INTERNAL_BS_DRIVER (L"BiosSnp16",              BiosSnp16DriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Snp3264",                InitializeSnpNiiDriver);
    LOAD_INTERNAL_BS_DRIVER (L"PxeBc",                  InitializeBCDriver);
    LOAD_INTERNAL_BS_DRIVER (L"PxeDhcp4",               PxeDhcp4DriverEntryPoint);
//  LOAD_INTERNAL_BS_DRIVER (L"BIS",                    EFIBIS_BaseCodeModuleInit);
    LOAD_INTERNAL_BS_DRIVER (L"SerialMouse",            SerialMouseDriverEntryPoint);

    //
    // Create an event to be signalled when ExitBootServices occurs
    //
POST_CODE(0x14);
    Status = BS->CreateEvent(
                    EVT_SIGNAL_EXIT_BOOT_SERVICES, 
                    TPL_NOTIFY,
                    PlExitBootServices,
                    NULL,
                    &Event
                    );

    ASSERT (!EFI_ERROR(Status));

#ifdef EFI_BOOTSHELL    
    PlInitializeInternalLoad();
#endif

    PlInitializeLegacyBoot();

    // 
    // loop thru boot manager and boot maintenance until a boot
    // option is selected
    //
POST_CODE(0x15);
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

    RetParam.p0 = 0;
    return (RetParam);
}

#ifdef EFI_BOOTSHELL    
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
    IN  UINTN   FunctionId
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

    PlTable.EmulateLoad          = NULL;
    PlTable.IdleLoop             = PlIdleLoop;
    PlTable.SetInterruptState    = PlSetInterruptState;
    PlTable.FlushCache           = RtPlCacheFlush;
    PlTable.SetVirtualAddressMap = RtPlSetVirtualAddressMap;
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
    // Update / provide any entries in the boot services table
    // The core EFI firmware does not provide the following
    // entries in the boot services table:
    //          Stal
    //

    BS->Stall            = RtPlStall;
    BS->SetWatchdogTimer = PlSetWatchdogTimer;

    //
    // Update / provide any entries in the runtime service table.
    // The core EFI firmware does not provide the following
    // entries in the runtime services table:

    RT->ResetSystem     = RtPlResetSystem;
    RT->GetTime         = RtPlGetTime;
    RT->SetTime         = RtPlSetTime;
    RT->GetWakeupTime   = RtPlGetWakeupTime;
    RT->SetWakeupTime   = RtPlSetWakeupTime;
   
    ST->NumberOfTableEntries = 0;
    ST->ConfigurationTable   = NULL;

    //
    // Done with table initializations
    //
}

STATIC
VOID
PlInstallMemoryMap (
    IN  UINTN                   FunctionId,
    IN  UINTN                   NoDesc,
    IN  EFI_MEMORY_DESCRIPTOR   *Desc
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

    NoDesc      - Number of desc in Desc

    Desc        - The passed in memory map

Returns:

    Memory map descriptors added

--*/
{
    UINTN   Index;
    UINTN   Index1;
    UINTN   BestIndex;
    UINT64  End;
    UINT64  BestSize;

    for (Index=0; Index < NoDesc; Index++) {
        if (Desc[Index].Type == EfiACPIReclaimMemory || 
            Desc[Index].Type == EfiACPIMemoryNVS     || 
            (Desc[Index].Attribute & EFI_MEMORY_RUNTIME) ) {

            if (Desc[Index].PhysicalStart & 0x1fff) {
                for (Index1=0; Index1 < NoDesc; Index1++) {
                    if (Desc[Index1].Type == EfiConventionalMemory && Desc[Index1].NumberOfPages > 0) {
                        End = Desc[Index1].PhysicalStart + LShiftU64 (Desc[Index1].NumberOfPages, EFI_PAGE_SHIFT);
                        if (End == Desc[Index].PhysicalStart) {
                            Desc[Index1].NumberOfPages--;
                            Desc[Index].PhysicalStart -= EFI_PAGE_SIZE;
                            Desc[Index].NumberOfPages++;
                        }
                    }
                }
            }

            if (Desc[Index].NumberOfPages & 1) {
                End = Desc[Index].PhysicalStart + LShiftU64 (Desc[Index].NumberOfPages, EFI_PAGE_SHIFT);
                for (Index1=0; Index1 < NoDesc; Index1++) {
                    if (Desc[Index1].Type == EfiConventionalMemory && Desc[Index1].NumberOfPages > 0) {
                        if (End == Desc[Index1].PhysicalStart) {
                            Desc[Index].NumberOfPages++;
                            Desc[Index1].NumberOfPages--;
                            Desc[Index1].PhysicalStart += EFI_PAGE_SIZE;
                        }
                    }
                }
            }
        }
    }

    //
    // Find the largest available free memory descriptor
    // and add it first
    //
    BestIndex = NoDesc;
    BestSize = 0;
    for (Index=0; Index < NoDesc; Index += 1) {
        if (Desc[Index].Type == EfiConventionalMemory  &&
            Desc[Index].NumberOfPages > BestSize) {
            
            BestIndex = Index;
            BestSize = Desc[Index].NumberOfPages;
        }
    }

    ASSERT (BestIndex < NoDesc);
    FW->AddMemoryDescriptor (
            Desc[BestIndex].Type,
            Desc[BestIndex].PhysicalStart,
            Desc[BestIndex].NumberOfPages,
            Desc[BestIndex].Attribute
            );

    //
    // Now add all remaining information to the memory map
    //

    for (Index=0; Index < NoDesc; Index++) {
        if (Index != BestIndex && Desc[Index].NumberOfPages > 0) {
            FW->AddMemoryDescriptor (
                Desc[Index].Type, 
                Desc[Index].PhysicalStart,   
                Desc[Index].NumberOfPages,
                Desc[Index].Attribute
                );
        }
    }
}

STATIC
VOID
PlInstallBaseDevices (                      
    IN  UINTN   FunctionId,
    IN  UINTN   NvramBanks,
    IN  UINTN   NvramSize
    )
/*++

Routine Description:

    Add handles to the base devices here. In particular
    the console devices, internal nvram device(s), and
    the timer tick handler.

    Variable store support is not enabled at this time. 
    (The firware needs the nvram & system volume devices
    to enable such support)

Arguments:

    None

Returns:

    Base device handles added

--*/
{
    EFI_DEVICE_PATH                 *DevicePath;
    EFI_HANDLE                      Handle;
    EFI_STATUS                      Status;
    UINT64                          IoPortBase;

    //
    //
    // First add a device(s) to handle device_io request
    // (this is done since the next devices may need to access
    // their IO space)
    //

    if (LibGetSalIoPortMapping (&IoPortBase) != EFI_SUCCESS) {
        //
        // BugBug: If you can't find it in the SAL System Table take a guess.
        //          If this guess is wrong you will plant in the ASSERT in the stall 
        //          routine. It will look like the timer is not ticking since IOs are
        //          not working.
        //
        IoPortBase = 0x0000ffffc000000;
    }  

    //
    // EFI is defined to be in physical mode. Thus we must flip bit 63 to
    //  make the IO cycles be uncacheable.
    //
    IoPortBase |= 0x8000000000000000;

    PlInstallDefaultIoDevice (
                        BiosRootDevicePath, 
                        0x8000000000000000, // Force non cachable access
                        IoPortBase
                        );

    
    
    //
    // Set up the GlobalIoFncs global to point to global memory/io/pci space
    //
    DevicePath = BiosRootDevicePath;
    Status = BS->LocateDevicePath (&DeviceIoProtocol, &DevicePath, &Handle);
    if (!EFI_ERROR(Status)) {
        Status = BS->HandleProtocol (Handle, &DeviceIoProtocol, (VOID *)&GlobalIoFncs);
    }
    ASSERT (!EFI_ERROR(Status));

    //
    // Initialize to PCI Root Bus List
    //

    InitializePciRootBusList (&PciRootBusList);    


    //
    // Add SAL callback NVRAM store.
    //

    PlInitNvVarStoreFlash ( 
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        NvramSize, NvramBanks
        );
    
    //
    // Install the Unicode string device.  This device supports
    // case insensitive comparisons of Unicode strings.
    //

    PlInitializeUnicodeStringDevice();
}

STATIC
VOID
PlIdleLoop (
    IN BOOLEAN  Polling
    )
{
}

STATIC
VOID
PlExitBootServices (
    IN EFI_EVENT        Event,
    IN VOID             *Context
    )
/*++

Routine Description:

    Signal handlers for ExitBootServices event

Arguments:

    Event type
    Context fo the event

Returns:
   

--*/
{
    //
    // Clear non-runtime pointer
    //

    PlTable.EmulateLoad = NULL;
    PlTable.IdleLoop = NULL;
    PlTable.SetInterruptState = NULL;

    // EFI f/w takes care of boot service's table

    //
    // Cancel Timer
    //

    PlTimer(ID_SALCB_TIMER_CANCEL, 0, NULL);
}

VOID    
TimerCallBack (
    IN UINTN ElapsedTime
    )
/*++

Routine Description:

    Timer callback . Called by SAL whenever the timer period expires


Arguments:

    ElapsedTime in msec

Returns:

    None

--*/
{
    EFI_TPL     OldTpl; 

    OldTpl = BS->RaiseTPL(TPL_HIGH_LEVEL);

    //
    // Inform the firmware of an "timer interrupt".  The time expired
    // since the last call is 10,000 times the number of ms.  (or 100ns units)
    //

    FW->TickHandler(ElapsedTime*10000);

    BS->RestoreTPL(OldTpl); 

    return;
}
