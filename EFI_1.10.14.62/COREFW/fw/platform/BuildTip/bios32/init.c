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

    Main entry point on EFI 32-bit environment.

Revision History

--*/

#include "bios32.h"
#include "plshell.h"
#include "PlIntCtrl.h"
#include "CpuInterrupt.h"
#include "CpuTimer.h"
#include "intload.h"
#include "Drivers.h"
#include "PlDefio.h"
#include "PlatformLib.h"
#include "PlEfiLdr.h"
 
#include EFI_PROTOCOL_DEFINITION (Decompress)

//
//
//
#define EFILDR_HEADER_ADDRESS      0x00022000

typedef struct {
    UINT32       CheckSum;
    UINT32       Offset;
    UINT32       Length;
    UINT8        FileName[52];
} EFILDR_IMAGE;

typedef struct {          
    UINT32       Signature;     
    UINT32       HeaderCheckSum;
    UINT32       FileLength;
    UINT32       NumberOfImages;
} EFILDR_HEADER;

//
// Function Prototypes
//
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
    );

STATIC
VOID
PlInstallMemoryMap (
    IN UINTN                    NoDesc,
    IN EFI_MEMORY_DESCRIPTOR    *Desc
    );

STATIC
VOID 
PlInstallBaseDevices (                      
    );

STATIC
VOID
InitTickHandler(
    VOID
    );

STATIC
VOID
PlExitBootServices (
    IN EFI_EVENT        Event,
    IN VOID             *Context
    );

VOID
MainEntry (
    IN EFILDRHANDOFF  *Handoff
    )

/*++

  Routine Description:

    This is the entry point after this code has been loaded into memory. 

Arguments:


Returns:

    Calls into EFI Firmware

--*/
{
    EFI_STATUS                Status;
    EFI_EVENT                 Event;
    EFI_DECOMPRESS_PROTOCOL   *Decompress;
    EFILDR_HEADER             *EFILDRHeader;
    EFILDR_IMAGE              *EFILDRImage;
    UINTN                     Index;
    CHAR16                    DriverFileName[280];
    EFI_DEVICE_PATH           *FilePath;
    EFI_HANDLE                ImageHandle;
    VOID                      *Destination;
    VOID                      *Scratch;
    UINTN                     DestinationSize;
    UINTN                     ScratchSize;
    EFI_HANDLE                EfiCoreImageHandle;

    PlInitEfiLdrCallBack (Handoff->EfiLdrCallBack);

    //
    // Add the platform support to the EFI FW tables.
    //
POST_CODE(0xaf);
    PlInitializeTables();

    //
    // Initialize the EFI FW memory map
    //
POST_CODE(0x01);
    PlInstallMemoryMap (Handoff->MemDescCount,Handoff->MemDesc);

    //
    // Set up interrupt vector mappings in interrupt controller for EFI
    //
POST_CODE(0x02);
    PlSetupInterruptController(INT_CTRLR_EFIMODE);
POST_CODE(0x03);
    InitTickHandler();

POST_CODE(0x04);
    FW->MemoryMapInstalled();                           // Second

    //
    // Once the memory map has been installed, basic EFI services
    // are now functional.   Although no devices or variable
    // store support is on line.
    //
POST_CODE(0x05);
    InitializeLib (NULL, ST);

    //
    // Build an image handle for the current executing image
    // 
POST_CODE(0x06);
    BuildEfiCoreImageHandle (
      MainEntry,
      Handoff->ImageBase,
      Handoff->ImageSize,
      &EfiCoreImageHandle
      );

POST_CODE(0x07);
    PlInitWatchdogTimer();

    //
    // Initialize support for calling BIOS functions
    //
POST_CODE(0x08);
    InitializeBiosIntCaller ();

    //
    // Install base devices.  This would at least include a 
    // global device_IO device, all NV ram store device(s), and 
    // the timer tick.  It may optionally include other device_io
    // devices.
    //
POST_CODE(0x09);
    PlInstallBaseDevices();                   
POST_CODE(0x0a);
    FW->NvStoreInstalled();                             // Third

    //
    // Install and connect all built in EFI 1.1 Drivers.
    //
POST_CODE(0x0d);
    LOAD_INTERNAL_BS_DRIVER (L"BiosDisk",               InstallBiosBlkIoDrivers);

    LOAD_INTERNAL_BS_DRIVER (L"Ebc",                    InitializeEbcDriver);
    LOAD_INTERNAL_BS_DRIVER (L"Decompress",             DecompressDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"PcatPciRootBridge",      InitializePcatPciRootBridge);
    LOAD_INTERNAL_BS_DRIVER (L"PciBus",                 PciBusEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"PcatIsaAcpi",            PcatIsaAcpiDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"IsaBus",                 IsaBusControllerDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"IsaSerial",              SerialControllerDriverEntryPoint);

POST_CODE(0x0e);
    LOAD_INTERNAL_BS_DRIVER (L"BiosVgaMiniPort",        BiosVgaMiniPortDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"VgaClass",               VgaClassDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"CirrusLogic5430",        CirrusLogic5430UgaDrawDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"GraphicsConsole",        InitializeGraphicsConsole);
    LOAD_INTERNAL_BS_DRIVER (L"ConPlatform",            ConPlatformDriverEntry);
    LOAD_INTERNAL_BS_DRIVER (L"ConSplitter",            ConSplitterDriverEntry);

POST_CODE(0x0f);
    ConnectAllConsoles ();

    //
    // Once consoles are installed, messages may be printed to the consoles
    //
POST_CODE(0x10);
    EFIFirmwareBanner();
    PlPrintLogonBanner();
    Print (L"This image %HMainEntry%N is at address %08x\n",  (UINTN)MainEntry);

    //
    // Install remaining EFI 1.1 Drivers
    //
POST_CODE(0x11);
    LOAD_INTERNAL_BS_DRIVER (L"DiskIo",                 DiskIoDriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Partition",              PartitionEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Fat",                    FatEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"BiosKeyboard",           BiosKeyboardDriverEntryPoint);

    //
    // Load all images that follow the EFI Core image in EFILDR
    // 
POST_CODE(0x12);
    Status = BS->LocateProtocol(&gEfiDecompressProtocolGuid, NULL, &Decompress);

    if (!EFI_ERROR (Status)) {
      EFILDRHeader = (EFILDR_HEADER *)(EFILDR_HEADER_ADDRESS);
      EFILDRImage  = (EFILDR_IMAGE *)(EFILDR_HEADER_ADDRESS + sizeof(EFILDR_HEADER));
      EFILDRImage += 2;
      for (Index = 2; Index < EFILDRHeader->NumberOfImages; Index++, EFILDRImage++) {
        SPrint(DriverFileName, sizeof(DriverFileName), L"%a", EFILDRImage->FileName);
        FilePath = FileDevicePath (NULL, DriverFileName);

        //
        // Decompress the image
        //

        Status = Decompress->GetInfo(
                   Decompress, 
                   (VOID *)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
                   EFILDRImage->Length,
                   &DestinationSize, 
                   &ScratchSize
                   );
        if (!EFI_ERROR (Status)) {
          Destination = AllocatePool(DestinationSize);
          Scratch = AllocatePool(ScratchSize);
          if (Destination != NULL && Scratch != NULL) {

            Status = Decompress->Decompress(
                       Decompress, 
                       (VOID *)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
                       EFILDRImage->Length,
                       Destination,
                       DestinationSize, 
                       Scratch,
                       ScratchSize
                       );
            if (!EFI_ERROR (Status)) {
  
              Status = BS->LoadImage(
                             TRUE,
                             EfiCoreImageHandle,
                             FilePath,
                             Destination,
                             DestinationSize,
                             &ImageHandle
                             );
              if (!EFI_ERROR (Status)) {
                Status = BS->StartImage (ImageHandle, 0, NULL);
              }
            }
          }
          if (Destination != NULL) {
            FreePool (Destination);
          }
          if (Scratch != NULL) {
            FreePool (Scratch);
          }
        }
      }
    }

    BS->FreePages((EFI_PHYSICAL_ADDRESS)EFILDR_HEADER_ADDRESS, (EFILDRHeader->FileLength + EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT);

    //
    // Connect all EFI 1.1 drivers to get disks connected so access to the
    // BootStr file is possible.  Also redisplay the banner just in case
    // additional consoles showed up.
    //
POST_CODE(0x13);
    ConnectAll();
    EFIFirmwareBanner();
    PlPrintLogonBanner();
    Print (L"This image %HMainEntry%N is at address %08x\n",  (UINTN)MainEntry);

    //
    //
    //
POST_CODE(0x14);
    PlInitNvVarStoreFile ( 
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        0x4000, 2
        );
    FW->NvStoreInstalled();                             // Third

    //
    // Say we only support English
    //
POST_CODE(0x15);
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

POST_CODE(0x16);
    InitializeLib (NULL, ST);

    //
    // Install all built in EFI 1.1 Drivers that require EFI Variable Services
    //
POST_CODE(0x17);
    LOAD_INTERNAL_BS_DRIVER (L"Terminal",               InitializeTerminal);
    LOAD_INTERNAL_BS_DRIVER (L"BiosSnp16",              BiosSnp16DriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"Snp3264",                InitializeSnpNiiDriver);
    LOAD_INTERNAL_BS_DRIVER (L"PxeBc",                  InitializeBCDriver);
    LOAD_INTERNAL_BS_DRIVER (L"BIS",                    EFIBIS_BaseCodeModuleInit);
    LOAD_INTERNAL_BS_DRIVER (L"PxeDhcp4",               PxeDhcp4DriverEntryPoint);
    LOAD_INTERNAL_BS_DRIVER (L"SerialMouse",            SerialMouseDriverEntryPoint);

    //
    // Connect all EFI 1.1 drivers to get banner out to all consoles.
    //
POST_CODE(0x18);
    ConnectAll();
    EFIFirmwareBanner();
    PlPrintLogonBanner();
    Print (L"This image %HMainEntry%N is at address %08x\n",  (UINTN)MainEntry);

    //
    // Create an event to be signalled when ExitBootServices occurs
    //
POST_CODE(0x19);
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

POST_CODE(0x1a);
    PlInitializeLegacyBoot();

POST_CODE(0x1b);
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

    BS->Stall           = RtPlStall;
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
    IN UINTN                    NoDesc,
    IN EFI_MEMORY_DESCRIPTOR    *Desc
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
    UINTN               Index, BestIndex;
    UINT64              BestSize;

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

    for (Index=0; Index < NoDesc; Index += 1) {
        if (Index != BestIndex) {
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
  EFI_DEVICE_PATH  *DevicePath;
  EFI_HANDLE       Handle;
  EFI_STATUS       Status;

  BS->InstallConfigurationTable(&AcpiTableGuid,   FindAcpiRsdPtr());
  BS->InstallConfigurationTable(&MpsTableGuid,    FindMPSPtr());
  BS->InstallConfigurationTable(&SMBIOSTableGuid, FindSMBIOSPtr());

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
    // Cancel Timer
    //
    PlDisableTimerInterrupt();

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
InitTickHandler(
    VOID
    )
{
    PlCalibrateCpuFreq();
    InstallInterruptHandler(PlGetVectorFromIrq (0), SystemTimerHandler);
    PlEnableTimerInterrupt();
}
