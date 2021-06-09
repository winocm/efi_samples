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

  pci.c
  
Abstract: 
  EFI shell command "pci"

Revision History

--*/

#include "shelle.h"
#include "pci22.h"
#include "pci_class.h"
#include "pci.h"
#include "Acpi.h"

#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)


EFI_STATUS
PciDump (
  IN EFI_HANDLE                             ImageHandle,
  IN EFI_SYSTEM_TABLE                       *SystemTable
  );

EFI_STATUS
PciFindProtocolInterface (
  IN  EFI_HANDLE                            *HandleBuf,
  IN  UINTN                                 HandleCount,
  IN  UINT16                                Segment,
  IN  UINT16                                Bus,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev
  );

EFI_STATUS
PciGetProtocolAndResource (
  IN  EFI_HANDLE                            Handle,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev,
  OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR     **Descriptors
  );

EFI_STATUS
PciGetNextBusRange (
  IN OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  **Descriptors,
  OUT    UINT16                             *MinBus, 
  OUT    UINT16                             *MaxBus, 
  OUT    BOOLEAN                            *IsEnd
  );
   
EFI_STATUS
PciExplainData (
  IN PCI_CONFIG_SPACE                       *ConfigSpace,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );

EFI_STATUS
PciExplainDeviceData (
  IN PCI_DEVICE_HEADER                      *Device,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );

EFI_STATUS
PciExplainBridgeData (
  IN PCI_BRIDGE_HEADER                      *Bridge,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );

EFI_STATUS 
PciExplainBar (
  IN UINT32                                 *Bar,
  IN UINT16                                 *Command,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev,
  IN OUT UINTN                              *Index
  );

EFI_STATUS
PciExplainCardBusData (
  IN PCI_CARDBUS_HEADER                     *CardBus,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );
  
EFI_STATUS
PciExplainStatus (
  IN UINT16                                 *Status,
  IN BOOLEAN                                MainStatus,
  IN PCI_HEADER_TYPE                        HeaderType
  );
    
EFI_STATUS
PciExplainCommand (
  IN UINT16                                 *Command
  );
      
EFI_STATUS
PciExplainBridgeControl (
  IN UINT16                                 *BridgeControl,
  IN PCI_HEADER_TYPE                        HeaderType
  );

PCI_CONFIG_SPACE                            *mConfigSpace;

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(PciDump)
#endif

EFI_STATUS
PciDump (
  IN EFI_HANDLE                             ImageHandle,
  IN EFI_SYSTEM_TABLE                       *SystemTable
  )
/*++

Routine Description:

  Command entry point. Parses command line arguments and execute it. If 
  needed, calls internal function to perform certain operation.

Arguments:

  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINT16                                    Segment;
  UINT16                                    Bus;
  UINT16                                    Device;
  UINT16                                    Func;
  UINT64                                    Address;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL           *IoDev;
  EFI_STATUS                                Status;
  PCI_COMMON_HEADER                         PciHeader;
  PCI_CONFIG_SPACE                          ConfigSpace;
  UINTN                                     ScreenCount;
  UINTN                                     TempColumn;
  UINTN                                     ScreenSize;
  CHAR16                                    ReturnStr[1];
  BOOLEAN                                   ExplainData;
  UINTN                                     ArgIndex;
  UINTN                                     Index;
  UINTN                                     SizeOfHeader;
  BOOLEAN                                   PrintTitle;
  BOOLEAN                                   PrintPause;
  UINTN                                     HandleBufSize;
  EFI_HANDLE                                *HandleBuf;
  UINTN                                     HandleCount;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR         *Descriptors; 
  UINT16                                    MinBus;
  UINT16                                    MaxBus;
  BOOLEAN                                   IsEnd;
  BOOLEAN                                   SetSegment;
  CHAR16                                    *PromptStr;
  
  IoDev = NULL;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   
    SystemTable,   
    PciDump,
    L"pci",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif
  
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Get all instances of PciRootBridgeIo. Allocate space for 1 EFI_HANDLE and
  // call LocateHandle(), if EFI_BUFFER_TOO_SMALL is returned, allocate enough
  // space for handles and call it again.
  // 
  HandleBufSize = sizeof (EFI_HANDLE);
  HandleBuf = (EFI_HANDLE *)AllocatePool (HandleBufSize);
  if (HandleBuf == NULL) {
    Print (L"pci: Out of resources\n");
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = BS->LocateHandle (
                 ByProtocol, 
                 &gEfiPciRootBridgeIoProtocolGuid,
                 NULL, 
                 &HandleBufSize, 
                 HandleBuf
                 );
   
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HandleBuf = ReallocatePool (HandleBuf, sizeof(EFI_HANDLE), HandleBufSize);
    if (HandleBuf == NULL) {
      Print (L"pci: Out of resources\n");
      return EFI_OUT_OF_RESOURCES;
    }
    
    Status = BS->LocateHandle (
                   ByProtocol, 
                   &gEfiPciRootBridgeIoProtocolGuid,
                   NULL, 
                   &HandleBufSize, 
                   HandleBuf
                   );
  }
 
  if (EFI_ERROR(Status)) {
    Print (L"pci: Locate handle error - %r\n", Status);
    FreePool (HandleBuf);
    return Status;
  }
  HandleCount = HandleBufSize / sizeof(EFI_HANDLE);
  
  //
  // Argument Count == 1(no other argument): enumerate all pci functions
  //
  if (SI->Argc == 1) {
    ST->ConOut->QueryMode (
                  ST->ConOut, 
                  ST->ConOut->Mode->Mode, 
                  &TempColumn, 
                  &ScreenSize
                  );
    
    ScreenCount = 0;
    ScreenSize -= 4;
    if ((ScreenSize & 1) == 1) {
      ScreenSize -= 1;
      PromptStr = L"\nPress ENTER to continue:";
      
    } else {
      PromptStr = L"Press ENTER to continue:";
    }
          
    PrintTitle = TRUE;
    PrintPause = FALSE;
    
    //
    // For each handle, which decides a segment and a bus number range, 
    // enumerate all devices on it.
    //
    for (Index = 0; Index < HandleCount; Index ++) {
      Status = PciGetProtocolAndResource (
                 HandleBuf[Index], 
                 &IoDev, 
                 &Descriptors
                 );
      if (EFI_ERROR(Status)) {
        Print (L"pci: Handle protocol or configuration error - %r\n", Status);
        FreePool (HandleBuf);
        return Status;
      }
      
      //
      // No document say it's impossible for a RootBridgeIo protocol handle
      // to have more than one address space descriptors, so find out every
      // bus range and for each of them do device enumeration.
      //
      while (TRUE) {
        Status = PciGetNextBusRange (&Descriptors, &MinBus, &MaxBus, &IsEnd);
     
        if (EFI_ERROR(Status)) {
          Print (L"pci: Get next bus range error - %r\n", Status);
          FreePool (HandleBuf);
          return Status;
        }
        if (IsEnd) {
          break;
        }
        
        for (Bus = MinBus; Bus <= MaxBus; Bus++) {
          //  
          // For each devices, enumerate all functions it contains
          //
          for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
            //
            // For each function, read its configuration space and print summary
            //
            for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {
              Address = CALC_EFI_PCI_ADDRESS (Bus, Device, Func, 0);
              IoDev->Pci.Read (
                           IoDev, 
                           EfiPciWidthUint16, 
                           Address, 
                           1, 
                           &PciHeader.VendorId
                           );
          
              //
              // If VendorId = 0xffff, there does not exist a device at this 
              // location. For each device, if there is any function on it, 
              // there must be 1 function at Function 0. So if Func = 0, there
              // will be no more functions in the same device, so we can break
              // loop to deal with the next device.
              //  
              if (PciHeader.VendorId == 0xffff && Func == 0) {
                break;
              }
              
              if (PciHeader.VendorId != 0xffff) {
                if (PrintPause) {
                  Input (
                    PromptStr, 
                    ReturnStr, 
                    sizeof(ReturnStr)/sizeof(CHAR16)
                    );
                  Print (L"\n");
                  PrintPause = FALSE;
                }
  
                if (PrintTitle) {
                  Print (L"   Seg  Bus  Dev  Func\n");
                  Print (L"   ---  ---  ---  ----\n");
                  PrintTitle = FALSE;
                }
                
                IoDev->Pci.Read (
                             IoDev, 
                             EfiPciWidthUint32, 
                             Address, 
                             sizeof(PciHeader)/sizeof(UINT32), 
                             &PciHeader
                             );
                             
                Print (
                  L"    %E%02x   %02x   %02x    %02x ==> %N", 
                  IoDev->SegmentNumber, 
                  Bus, 
                  Device, 
                  Func
                  );
                  
                PciPrintClassCode (PciHeader.ClassCode, FALSE);
                Print (
                  L"\n             Vendor 0x%04x Device 0x%04x Prog Interface %x\n",
                  PciHeader.VendorId, 
                  PciHeader.DeviceId, 
                  PciHeader.ClassCode[0]
                  );
                  
                ScreenCount += 2;
                if (ScreenCount >= ScreenSize && ScreenSize != 0) {
                  //
                  // If ScreenSize == 0 we have the console redirected so don't
                  //  block updates
                  //
                  ScreenCount = 0;
                  PrintPause = TRUE;
                  PrintTitle = TRUE;
                }
  
                //
                // If this is not a multi-function device, we can leave the loop 
                // to deal with the next device.
                //
                if (Func == 0 && 
                  ((PciHeader.HeaderType & HEADER_TYPE_MULTI_FUNCTION) == 0x00)) {
                  break;
                }
              }  
            }
          }
        }

        //
        // If Descriptor is NULL, Configuration() returns EFI_UNSUPPRORED, 
        // we assume the bus range is 0~PCI_MAX_BUS. After enumerated all 
        // devices on all bus, we can leave loop.
        //
        if (Descriptors == NULL) {
          break;
        }
      }
    }
    FreePool (HandleBuf);
    return EFI_SUCCESS;
  }
  
  if (SI->Argc == 2) {
    Print (L"pci: Too few arguments\n");
    FreePool (HandleBuf);
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Arg count >= 3, dump binary of specified function, interpret if necessary
  //
  SetSegment  = FALSE;
  ExplainData = FALSE;
  ArgIndex    = 0;
  Segment     = Bus = Device = Func = 0;
  for (Index = 1; Index < SI->Argc; Index ++) {
    if (SI->Argv[Index][0] == '-' && SI->Argv[Index][1] == 'i') {
      ExplainData = TRUE;
      
    } else if (SetSegment) {
      Segment = (UINT16) xtoi(SI->Argv[Index]);
      SetSegment = FALSE;
      
    } else if (SI->Argv[Index][0] == '-' && SI->Argv[Index][1] == 's') {
      SetSegment = TRUE;
        
    } else {
      //  
      // The first Argument(except "-i") is assumed to be Bus number, second 
      // to be Device number, and third to be Func number.
      //
      switch ( ArgIndex ) {
        case 0:
          Bus = (UINT16) xtoi(SI->Argv[Index]);
          break;
          
        case 1:
          Device = (UINT16) xtoi(SI->Argv[Index]);
          break;
          
        case 2:
          Func = (UINT16) xtoi(SI->Argv[Index]);
          break;
          
        default:
          if (SI->Argc > 6) {
            Print (L"pci: Too many arguments\n");
          
          } else {
            Print ( L"pci: Too many arguments except \"-i\", \"-s\"\n" );
          }
          FreePool (HandleBuf);
          return EFI_INVALID_PARAMETER;
      }
      ArgIndex ++;
    }
  }

  if (ArgIndex < 2) {
    Print (L"pci: Too few arguments except \"-i\", \"-s\"\n");
    FreePool (HandleBuf);
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Find the protocol interface who's in charge of current segment, and its 
  // bus range covers the current bus
  //
  Status = PciFindProtocolInterface (
             HandleBuf, 
             HandleCount, 
             Segment, 
             Bus,
             &IoDev
             );
             
  if (EFI_ERROR(Status)) {
    Print (L"pci: Cannot find protocol interface for segment %hd, bus %hd\n",
      Segment, 
      Bus
      );
      
    FreePool (HandleBuf);
    return Status;
  }
  
  Address = CALC_EFI_PCI_ADDRESS (Bus, Device, Func, 0);
  Status = IoDev->Pci.Read (
                        IoDev, 
                        EfiPciWidthUint8, 
                        Address, 
                        sizeof(ConfigSpace), 
                        &ConfigSpace
                        );

  if (EFI_ERROR(Status)) {
    Print (L"pci: Cannot read configuration data - %r\n", Status);
    FreePool (HandleBuf);
    return Status;
  }
  
  mConfigSpace = &ConfigSpace;
  Print (L"%H  PCI Segment %02x Bus %02x Device %02x Func %02x%N [EFI 0x%02x%02x%02x%02x00]\n", 
    Segment, 
    Bus, 
    Device, 
    Func, 
    Segment, 
    Bus, 
    Device, 
    Func
    );

  //
  // Dump standard header of configuration space
  //
  SizeOfHeader = sizeof(ConfigSpace.Common) + sizeof(ConfigSpace.NonCommon);
  
  DumpHex (2, 0, SizeOfHeader, &ConfigSpace);
  Print (L"\n");

  //
  // Dump device dependent Part of configuration space
  //
  DumpHex (
    2, 
    SizeOfHeader, 
    sizeof(ConfigSpace) - SizeOfHeader, 
    ConfigSpace.Data
    );

  //
  // If "-i" appears in command line, interpret data in configuration space
  //
  if (ExplainData) {  
    ST->ConOut->QueryMode (
                  ST->ConOut, 
                  ST->ConOut->Mode->Mode, 
                  &TempColumn, 
                  &ScreenSize
                  );
                  
    if (ScreenSize != 0) {
      Input (L"\nPress ENTER to continue:", ReturnStr, 
        sizeof(ReturnStr)/sizeof(CHAR16));
    }
    Print (L"\n");

    Status = PciExplainData (&ConfigSpace, Address, IoDev);
  }
  
  FreePool (HandleBuf);
  return Status;
}

EFI_STATUS
PciFindProtocolInterface (
  IN  EFI_HANDLE                            *HandleBuf,
  IN  UINTN                                 HandleCount,
  IN  UINT16                                Segment,
  IN  UINT16                                Bus,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev
  )
/*++

Routine Description:

  This function finds out the protocol which is in charge of the given 
  segment, and its bus range covers the current bus number. It lookes 
  each instances of RootBridgeIoProtocol handle, until the one meets the
  criteria is found. 

Arguments:

  HandleBuf       Buffer which holds all PCI_ROOT_BRIDIGE_IO_PROTOCOL handles
  HandleCount     Count of all PCI_ROOT_BRIDIGE_IO_PROTOCOL handles
  Segment         Segment number of device we are dealing with
  Bus             Bus number of device we are dealing with
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINTN                                     Index;
  EFI_STATUS                                Status;
  BOOLEAN                                   FoundInterface;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR         *Descriptors;
  UINT16                                    MinBus;
  UINT16                                    MaxBus;
  BOOLEAN                                   IsEnd;
    
  FoundInterface = FALSE;
  //
  // Go through all handles, until the one meets the criteria is found
  //
  for (Index = 0; Index < HandleCount; Index ++) {
    Status = PciGetProtocolAndResource (HandleBuf[Index], IoDev, &Descriptors);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    //
    // When Descriptors == NULL, the Configuration() is not implemented,
    // so we only check the Segment number
    //
    if (Descriptors == NULL && Segment == (*IoDev)->SegmentNumber) {
        return EFI_SUCCESS;
    }
    
    if ((*IoDev)->SegmentNumber != Segment) {
      continue;
    }
   
    while (TRUE) {
      Status = PciGetNextBusRange (&Descriptors, &MinBus, &MaxBus, &IsEnd);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      if (IsEnd) {
        break;
      }
    
      if (MinBus <= Bus && MaxBus >= Bus) {
        FoundInterface = TRUE;
        break;
      }
    }
  }
  
  if (FoundInterface) {
    return EFI_SUCCESS;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}  

EFI_STATUS
PciGetProtocolAndResource (
  IN  EFI_HANDLE                            Handle,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev,
  OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR     **Descriptors
  )
/*++

Routine Description:

  This function gets the protocol interface from the given handle, and
  obtains its address space descriptors.

Arguments:

  Handle          The PCI_ROOT_BRIDIGE_IO_PROTOCOL handle
  IoDev           Handle used to access configuration space of PCI device
  Descriptors     Points to the address space descriptors

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  EFI_STATUS Status;

  //
  // Get inferface from protocol
  //
  Status = BS->HandleProtocol (
                 Handle, 
                 &gEfiPciRootBridgeIoProtocolGuid, 
                 IoDev
                 );

  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  //
  // Call Configuration() to get address space descriptors
  //
  Status = (*IoDev)->Configuration (*IoDev, Descriptors);
  if (Status == EFI_UNSUPPORTED) {
    *Descriptors = NULL;
    return EFI_SUCCESS;
    
  } else {
    return Status;
  }
}

EFI_STATUS
PciGetNextBusRange (
  IN OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  **Descriptors,
  OUT    UINT16                             *MinBus, 
  OUT    UINT16                             *MaxBus, 
  OUT    BOOLEAN                            *IsEnd
  )        
/*++

Routine Description:

  This function get the next bus range of given address space descriptors.
  It also moves the pointer backward a node, to get prepared to be called
  again.

Arguments:

  Descriptors     points to current position of a serial of address space 
                  descriptors
  MinBus          The lower range of bus number
  MinBus          The upper range of bus number
  IsEnd           Meet end of the serial of descriptors 
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{    
  *IsEnd = FALSE;
  
  //
  // When *Descriptors is NULL, Configuration() is not implemented, so assume
  // range is 0~PCI_MAX_BUS
  //
  if ((*Descriptors) == NULL) {
    *MinBus = 0;
    *MaxBus = PCI_MAX_BUS;
    return EFI_SUCCESS;
  }
  
  //
  // *Descriptors points to one or more address space descriptors, which
  // ends with a end tagged descriptor. Examine each of the descriptors,
  // if a bus typed one is found and its bus range covers bus, this handle
  // is the handle we are looking for.
  //
  if ((*Descriptors)->Desc == ACPI_END_TAG_DESCRIPTOR) {
    *IsEnd = TRUE;
  }
  
  while ((*Descriptors)->Desc != ACPI_END_TAG_DESCRIPTOR) {
    if ((*Descriptors)->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS) {
      *MinBus = (UINT16)(*Descriptors)->AddrRangeMin;
      *MaxBus = (UINT16)(*Descriptors)->AddrRangeMax;
    }

    (*Descriptors) ++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainData (
  IN PCI_CONFIG_SPACE                       *ConfigSpace,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  )
/*++

Routine Description:

  Explain the data in PCI configuration space. The part which is common for
  PCI device and bridge is interpreted in this function. It calls other 
  functions to interpret data unique for device or bridge.

Arguments:

  ConfigSpace     Data in PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  PCI_COMMON_HEADER                         *Common;
  PCI_HEADER_TYPE                           HeaderType;
  EFI_STATUS                                Status;
  UINTN                                     Column;
  UINTN                                     ScreenSize;
  CHAR16                                    ReturnStr[1];
  
  Common = &(ConfigSpace->Common);
  
  //
  // Print Vendor Id and Device Id
  //
  Print (
    L"Vendor ID(0x%x): %E%04x%N                     ", 
    INDEX_OF (&(Common->VendorId)), 
    Common->VendorId
    );

  Print (
    L"Device ID(0x%x): %E%04x%N\n\n",                  
    INDEX_OF (&(Common->DeviceId)), 
    Common->DeviceId
    );

  //
  // Print register Command
  //
  PciExplainCommand (&(Common->Command));

  //
  // Print register Status
  //
  PciExplainStatus (&(Common->Status), TRUE, PciUndefined);

  //
  // Print register Revision ID
  //
  Print (
    L"\nRevision ID(0x%x):     %E%02x%N                 ", 
    INDEX_OF (&(Common->RevisionId)), 
    Common->RevisionId
    );
  
  //
  // Print register BIST
  //        
  Print (L"BIST(0x%02x): ", INDEX_OF (&(Common->BIST)));
  if((Common->BIST & PCI_BIT_7) != 0) {
    Print (L"Capable,Return: %E%02x%N\n", 0x0f & Common->BIST);
    
  } else {
    Print (L" Incapable\n");
  }

  //
  // Print register Cache Line Size
  //
  Print (
    L"Cache Line Size(0x%x): %E%02x%N                 ", 
    INDEX_OF (&(Common->CacheLineSize)), 
    Common->CacheLineSize
    );

  //
  // Print register Latency Timer
  //
  Print (
    L"Latency Timer(0x%x): %E%02x%N\n", 
    INDEX_OF (&(Common->PrimaryLatencyTimer)), 
    Common->PrimaryLatencyTimer
    );

  //
  // Print register Header Type
  //
  Print (
    L"Header Type(0x%02x):    %E%x%N, ", 
    INDEX_OF (&(Common->HeaderType)), 
    Common->HeaderType
    );

  if((Common->HeaderType & PCI_BIT_7) != 0) {
    Print (L"Multi-function, ");
    
  } else {
    Print (L"Single function, ");
  }
  
  HeaderType = (UINT8)(Common->HeaderType & 0x7f);
  switch (HeaderType) {
    case PciDevice:
      Print (L"PCI device\n");
      break;

    case PciP2pBridge:
      Print (L"P2P bridge\n");
      break;

    case PciCardBusBridge:
      Print (L"CardBus bridge\n");
      break;

    default:
      Print (L"Reserved\n");  
      HeaderType = PciUndefined;
  }
  
  // 
  // Print register Class Code
  //
  Print (L"Class: ");
  PciPrintClassCode ((UINT8 *)Common->ClassCode, TRUE);
  Print (L"\n");
  
  ST->ConOut->QueryMode (
                ST->ConOut, 
                ST->ConOut->Mode->Mode, 
                &Column, 
                &ScreenSize
                );
                
  if (ScreenSize != 0 && HeaderType != PciUndefined) {
    Input (L"\nPress ENTER to continue:", ReturnStr, 
      sizeof(ReturnStr)/sizeof(CHAR16));
    Print (L"\n");
  }
  
  //
  // Interpret remaining part of PCI configuration header depending on 
  // HeaderType
  //
  Status = EFI_SUCCESS;
  switch (HeaderType) {
    case PciDevice:
      Status = PciExplainDeviceData (
                 &(ConfigSpace->NonCommon.Device), 
                 Address, 
                 IoDev
                 );
      break;

    case PciP2pBridge:
      Status = PciExplainBridgeData (
                 &(ConfigSpace->NonCommon.Bridge), 
                 Address, 
                 IoDev
                 );
      break;

    case PciCardBusBridge:
      Status = PciExplainCardBusData (
                 &(ConfigSpace->NonCommon.CardBus), 
                 Address, 
                 IoDev
                 );
      break;
  }
  return Status;
}


EFI_STATUS
PciExplainDeviceData (
  IN PCI_DEVICE_HEADER                      *Device,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  )
/*++

Routine Description:

  Explain the device specific part of data in PCI configuration space.

Arguments:

  ConfigSpace     Data in PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINTN                                     Index;
  BOOLEAN                                   BarExist;
  EFI_STATUS                                Status;  
  UINTN                                     BarCount;

  //
  // Print Base Address Registers(Bar). When Bar = 0, this Bar does not 
  // exist. If these no Bar for this function, print "none", otherwise 
  // list detail information about this Bar.
  //
  Print (L"Base Address Registers(0x%x):\n", INDEX_OF (Device->Bar));
  
  BarExist = FALSE;
  BarCount = sizeof(Device->Bar) / sizeof(Device->Bar[0]);
  for (Index = 0; Index<BarCount; Index++) {
    if (Device->Bar[Index] == 0) {
      continue;
    }
    
    if (!BarExist) {
      BarExist = TRUE;
      Print (L"     Start_Address  Type  Space    Prefectchable?     Size             Limit\n");
      Print (L"  --------------------------------------------------------------------------");
    }

    Status = PciExplainBar (
               &(Device->Bar[Index]), 
               &(mConfigSpace->Common.Command), 
               Address, 
               IoDev,
               &Index
               );
    
    if (EFI_ERROR(Status)) {
      break;
    }
  }
    
  if (!BarExist) {
    Print (L"  (None)");
    
  } else {
    Print (L"\n  --------------------------------------------------------------------------");
  }
  
  //      
  // Print register Expansion ROM Base Address
  //
  if ((Device->ROMBar & PCI_BIT_0) == 0) {
    Print (L"\nExpansion ROM Disabled(0x%x)\n\n", INDEX_OF (&(Device->ROMBar)) );
  
  } else {
    Print (
      L"\nExpansion ROM Base Address(0x%x): %E%08x%N\n\n", 
      INDEX_OF (&(Device->ROMBar)), 
      Device->ROMBar
      );
  }
  
  //
  // Print register Cardbus CIS ptr
  //
  Print (
    L"Cardbus CIS ptr(0x%x):   %E%08x%N\n", 
    INDEX_OF (&(Device->CardBusCISPtr)), 
    Device->CardBusCISPtr
    );

  //
  // Print register Sub-vendor ID and subsystem ID
  //
  Print (
    L"Sub VendorID(0x%x):          %E%04x%N      ", 
    INDEX_OF (&(Device->SubVendorId)), 
    Device->SubVendorId
    );

  Print (
    L"Subsystem ID(0x%x):      %E%04x%N\n", 
    INDEX_OF (&(Device->SubSystemId)), 
    Device->SubSystemId
    );

  //
  // Print register Capabilities Ptr
  //
  Print (
    L"Capabilities Ptr(0x%x):        %E%02x%N\n", 
    INDEX_OF (&(Device->CapabilitiesPtr)), 
    Device->CapabilitiesPtr
    );

  //
  // Print register Interrupt Line and interrupt pin
  //
  Print (
    L"Interrupt Line(0x%x):          %E%02x%N      ", 
    INDEX_OF (&(Device->InterruptLine)), 
    Device->InterruptLine
    );

  Print (
    L"Interrupt Pin(0x%x):       %E%02x%N\n",
    INDEX_OF (&(Device->InterruptPin)), 
    Device->InterruptPin
    );

  //
  // Print register Min_Gnt and Max_Lat
  //
  Print (
    L"Min_Gnt(0x%x):                 %E%02x%N      ", 
    INDEX_OF (&(Device->MinGnt)), 
    Device->MinGnt
    );

  Print (
    L"Max_Lat(0x%x):             %E%02x%N\n",
    INDEX_OF (&(Device->MaxLat)), 
    Device->MaxLat
    );

  return EFI_SUCCESS;
} 



EFI_STATUS
PciExplainBridgeData (
  IN  PCI_BRIDGE_HEADER                     *Bridge,
  IN  UINT64                                Address,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       *IoDev
  )
/*++

Routine Description:

  Explain the bridge specific part of data in PCI configuration space.

Arguments:

  Bridge          Bridge specific data region in PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINTN                                     Index;
  BOOLEAN                                   BarExist;
  CHAR16                                    ReturnStr[1];
  UINTN                                     BarCount;
  UINT32                                    IoAddress32;
  INTN                                      TempColumn;
  INTN                                      ScreenSize;
  EFI_STATUS                                Status;
  
  //
  // Print Base Address Registers. When Bar = 0, this Bar does not 
  // exist. If these no Bar for this function, print "none", otherwise 
  // list detail information about this Bar.
  //
  Print (L"Base Address Registers(0x%x):", INDEX_OF (&(Bridge->Bar)));

  BarExist = FALSE;
  BarCount = sizeof(Bridge->Bar) / sizeof(Bridge->Bar[0]);
  
  for (Index = 0; Index<BarCount; Index++) {
    if (Bridge->Bar[Index] == 0) {
      continue;
    }
    
    if (!BarExist ) {
      BarExist = TRUE;
      Print (L"     Start_Address  Type  Space    Prefectchable?     Size             Limit\n");
      Print (L"  --------------------------------------------------------------------------");
    }
    
    Status = PciExplainBar (
               &(Bridge->Bar[Index]), 
               &(mConfigSpace->Common.Command), 
               Address, 
               IoDev,
               &Index
               );
    
    if (EFI_ERROR(Status)) {
      break;
    }
  }
    
  if (!BarExist) {
    Print (L"  (None)");
  }
  
  else {
    Print (L"\n  --------------------------------------------------------------------------");
  }
  
  //    
  //Expansion register ROM Base Address
  //
  if ((Bridge->ROMBar & PCI_BIT_0) == 0) {
    Print (L"\nNo Expansion ROM(0x%x)    ", INDEX_OF (&(Bridge->ROMBar)));
      
  } else {
    Print (L"\nExpansion ROM Base Address(0x%x): %E%08x%N\n", 
      INDEX_OF (&(Bridge->ROMBar)), Bridge->ROMBar);
  }

  //
  // Print Bus Numbers(Primary, Secondary, and Subordinate
  //
  Print (
    L"\n\n(Bus Numbers)  Primary(0x%x)     Secondary(0x%x)   Subordinate(0x%x)\n",
    INDEX_OF (&(Bridge->PrimaryBus)), 
    INDEX_OF (&(Bridge->SecondaryBus)),
    INDEX_OF (&(Bridge->SubordinateBus))
    );
    
  Print (L"               ------------------------------------------------------\n");
        
  Print (L"               %E%02x%N",    Bridge->PrimaryBus);
  Print (L"                %E%02x%N",   Bridge->SecondaryBus);
  Print (L"                %E%02x%N\n", Bridge->SubordinateBus);
  
  // 
  // Print register Secondary Latency Timer
  //
  Print (
    L"\nSecondary Latency Timer(0x%x):       %E%02x%N\n\n", 
    INDEX_OF (&(Bridge->SecondaryLatencyTimer)), 
    Bridge->SecondaryLatencyTimer
    );
 
  //
  // Print register Secondary Status
  //
  PciExplainStatus (&(Bridge->SecondaryStatus), FALSE, PciP2pBridge);

  //
  // Print I/O and memory ranges this bridge forwards. There are 3 resource 
  // types: I/O, memory, and pre-fetchable memory. For each resource type,
  // base and limit address are listed.
  //
  Print (L"\nResource Type                            Base                    Limit\n");
  Print (L"----------------------------------------------------------------------\n");
    
  //
  // IO Base & Limit
  //
  IoAddress32 = (Bridge->IoBaseUpper << 16 | Bridge->IoBase << 8);
  IoAddress32 &= 0xfffff000;
  Print (L"I/O(0x%x)                            %E%08x%N", 
    INDEX_OF (&(Bridge->IoBase)), IoAddress32);

  IoAddress32 = (Bridge->IoLimitUpper << 16 | Bridge->IoLimit << 8);
  IoAddress32 |= 0x00000fff;
  Print (L"                 %E%08x%N\n", IoAddress32);
  
  //
  // Memory Base & Limit
  //
  Print (
    L"Memory(0x%x)                         %E%08x%N", 
    INDEX_OF (&(Bridge->MemoryBase)), 
    (Bridge->MemoryBase << 16) & 0xfff00000
    );
    
  Print (
    L"                 %E%08x%N\n", 
    (Bridge->MemoryLimit << 16) | 0x000fffff
    );

  //
  // Pre-fetch-able Memory Base & Limit
  //
  Print (
    L"Prefetchable Memory(0x%x)    %E%08x%08x%N", 
    INDEX_OF (&(Bridge->PrefetchableMemBase)), 
    Bridge->PrefetchableBaseUpper, 
    Bridge->PrefetchableMemBase << 16
    );
    
  Print (
    L"         %E%08x%08x%N\n", 
    Bridge->PrefetchableLimitUpper, 
    (Bridge->PrefetchableMemLimit << 16) | 0x000fffff
    );
  
  //
  // Print register Capabilities Pointer  
  //
  Print (L"\nCapabilities Ptr(0x%x):   %E%02x%N             \n\n", 
    INDEX_OF (&(Bridge->CapabilitiesPtr)), 
    Bridge->CapabilitiesPtr
    );

  //
  // Page break here
  //
  ST->ConOut->QueryMode (
                ST->ConOut, 
                ST->ConOut->Mode->Mode, 
                &TempColumn, 
                &ScreenSize
                );
                
  if (ScreenSize != 0) {
    Input (
      L"Press Enter to Continue:", 
      ReturnStr, 
      sizeof(ReturnStr)/sizeof(CHAR16)
      );
      
    Print (L"\n\n");
  }
  
  //
  // Print register Bridge Control
  //
  PciExplainBridgeControl (&(Bridge->BridgeControl), PciP2pBridge);
  
  //    
  // Print register Interrupt Line & PIN
  //
  Print (
    L"\nInterrupt Line(0x%x)      %E%02x%N             ", 
    INDEX_OF (&(Bridge->InterruptLine)), 
    Bridge->InterruptLine
    );

  Print (
    L"Interrupt Pin(0x%x)        %E%02x%N\n", 
    INDEX_OF (&(Bridge->InterruptPin)), 
    Bridge->InterruptPin
    );

  return EFI_SUCCESS;
} 

EFI_STATUS
PciExplainBar(
  IN UINT32                                 *Bar,
  IN UINT16                                 *Command,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev,
  IN OUT UINTN                              *Index
  )
/*++

Routine Description:

  Explain the Base Address Register(Bar) in PCI configuration space.

Arguments:

  Bar             Points to the Base Address Register intended to interpret
  Command         Points to the register Command
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINT16                                    OldCommand;
  UINT16                                    NewCommand;
  UINT64                                    Bar64;
  UINT32                                    OldBar32;
  UINT32                                    NewBar32;
  UINT64                                    OldBar64;
  UINT64                                    NewBar64;
  BOOLEAN                                   IsMem;
  BOOLEAN                                   IsBar32;
  UINT64                                    RegAddress;

  IsBar32  = TRUE;
  Bar64    = 0;
  NewBar32 = 0;
  NewBar64 = 0;
  
  //
  // According the bar type, list detail about this bar, for example: 32 or
  // 64 bits; pre-fetchable or not.
  //
  if ((*Bar & PCI_BIT_0) == 0) {
    //
    // This bar is of memory type
    //
    IsMem = TRUE;

    if ((*Bar & PCI_BIT_1) == 0 && (*Bar & PCI_BIT_2) == 0) {
      Print (L"\n          %E%08x%N  ", *Bar & 0xfffffff0);
      Print (L"Mem   ");
      Print (L"32 bits  ");
      
    } else if ((*Bar & PCI_BIT_1) == 0 && (*Bar & PCI_BIT_2) != 0) {
      CopyMem (&Bar64 ,Bar, sizeof (UINT64));
      Print (L"\n  %E%08x", (UINT32)(RShiftU64((Bar64 & 0xfffffffffffffff0), 32)));
      Print (L"%08x%N  ", (UINT32)(Bar64 & 0xfffffffffffffff0));
      Print (L"Mem   ");
      Print (L"64 bits  ");
      IsBar32 = FALSE;
      *Index += 1;
      
    } else {
      //
      //Reserved
      //
      Print (L"\n          %E%08x%N  ", *Bar & 0xfffffff0);
      Print (L"Mem            ");
    }
        
    if ((*Bar & PCI_BIT_3) == 0) {
      Print (L"No     ");
    
    } else {
      Print (L"YES    ");
    }
      
  } else {
    //
    // This bar is of io type
    IsMem = FALSE;
    Print (L"\n              %E%04x%N  ", *Bar & 0xfffffffc);
    Print (L"I/O                               ");
  }

  //            
  // Get BAR length(or the amount of resource this bar demands for). To get
  // Bar length, first we should temporarily disable I/O and memory access
  // of this function(by set bits in the register Command), then write all 
  // "1"s to this bar. The bar value read back is the amount of resource 
  // this bar demands for.  
  //
  
  //      
  // Disable io & mem access
  //
  OldCommand = *Command;
  NewCommand = (UINT16)(OldCommand & 0xfffc);
  RegAddress = Address | INDEX_OF (Command);
  IoDev->Pci.Write (IoDev, EfiPciWidthUint16, RegAddress, 1, &NewCommand);

  RegAddress = Address | INDEX_OF (Bar);

  //
  // Read after write the BAR to get the size
  //
  if (IsBar32) {
    OldBar32 = *Bar;
    NewBar32 = 0xffffffff;
    
    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 1, &NewBar32);
    IoDev->Pci.Read  (IoDev, EfiPciWidthUint32, RegAddress, 1, &NewBar32);
    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 1, &OldBar32);
  
    if (IsMem) {
      NewBar32 = NewBar32 & 0xfffffff0;
      NewBar32 = (~ NewBar32) + 1;

    } else {
      NewBar32 = NewBar32 & 0xfffffffc;
      NewBar32 = (~ NewBar32) + 1;
      NewBar32 = NewBar32 & 0x0000ffff;
    }
  } else {
    CopyMem (&OldBar64 ,Bar, sizeof (UINT64));
    NewBar64 = 0xffffffffffffffff;
    
    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 2, &NewBar64);
    IoDev->Pci.Read  (IoDev, EfiPciWidthUint32, RegAddress, 2, &NewBar64);
    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 2, &OldBar64);

    if (IsMem) {
      NewBar64 = NewBar64 & 0xfffffffffffffff0;
      NewBar64 = (~ NewBar64) + 1;

    } else {
      NewBar64 = NewBar64 & 0xfffffffffffffffc;
      NewBar64 = (~ NewBar64) + 1;
      NewBar64 = NewBar64 & 0x000000000000ffff;
    }
  }
  
  //
  // Enable io & mem access
  //
  RegAddress = Address | INDEX_OF (Command);
  IoDev->Pci.Write (IoDev, EfiPciWidthUint16, RegAddress, 1, &OldCommand);

  if (IsMem) {
    if (IsBar32) {
      Print (L"        %08x  ", NewBar32);
      Print (L"        %08x", NewBar32 + (*Bar & 0xfffffff0) - 1);
    
    } else {
      Print (L"%08x", RShiftU64(NewBar64, 32));
      Print (L"%08x  ", (UINT32)NewBar64);
      Print (L"%08x", RShiftU64((NewBar64 + (Bar64 & 0xfffffffffffffff0) - 1), 32));
      Print (L"%08x", (UINT32)(NewBar64 + (Bar64 & 0xfffffffffffffff0) - 1));
      
    }
  } else {
    Print (L"%04x              ", NewBar32);
    Print (L"%04x", NewBar32 + (*Bar & 0xfffffffc) - 1);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
PciExplainCardBusData (
  IN PCI_CARDBUS_HEADER                     *CardBus,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  )
/*++

Routine Description:

  Explain the cardbus specific part of data in PCI configuration space.

Arguments:

  CardBus         CardBus specific region of PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  BOOLEAN                                   Io32Bit;
  CHAR16                                    ReturnStr[1];
  INTN                                      TempColumn;
  INTN                                      ScreenSize;
  PCI_CARDBUS_DATA                          *CardBusData;
  
  Print (
    L"\nCardBus Socket Registers/ExCA Base Address Register(0x%x): %E%8x%N\n\n", 
    INDEX_OF (&(CardBus->CardBusSocketReg)), 
    CardBus->CardBusSocketReg
    );
  
  //
  // Print Secondary Status
  //
  PciExplainStatus ( &(CardBus->SecondaryStatus), FALSE, PciCardBusBridge);
  
  //
  // Print Bus Numbers(Primary bus number, CardBus bus number, and 
  // Subordinate bus number
  //
  Print (
    L"\n(Bus Numbers)  Pci(0x%x)         CardBus(0x%x)     Subordinate(0x%x)\n",
    INDEX_OF (&(CardBus->PciBusNumber)), 
    INDEX_OF (&(CardBus->CardBusBusNumber)),
    INDEX_OF (&(CardBus->SubordinateBusNumber))
    );
    
  Print (L"               ------------------------------------------------------\n");
        
  Print (L"               %E%02x%N", CardBus->PciBusNumber);
  Print (L"                %E%02x%N", CardBus->CardBusBusNumber);
  Print (L"                %E%02x%N\n", CardBus->SubordinateBusNumber);
  
  // 
  // Print CardBus Latency Timer
  //
  Print (L"\nCardBus Latency Timer(0x%x):         %E%02x%N\n", 
    INDEX_OF (&(CardBus->CardBusLatencyTimer)), 
    CardBus->CardBusLatencyTimer
    );
  
  //  
  // Print Memory/Io ranges this cardbus bridge forwards
  //
  Print (L"\nResource Type              Type              Base                Limit\n");
  Print (L"----------------------------------------------------------------------\n");
  
  Print (
    L"Mem(0x%x)      %s          %E%08x%N             %E%08x%N\n", 
    INDEX_OF (&(CardBus->MemoryBase0)), 
    CardBus->BridgeControl & PCI_BIT_8 ? L"    Prefetchable" : L"Non-Prefetchable",
    CardBus->MemoryBase0  & 0xfffff000, 
    CardBus->MemoryLimit0 | 0x00000fff 
    );
    
  Print (
    L"Mem(0x%x)      %s          %E%08x%N             %E%08x%N\n", 
    INDEX_OF (&(CardBus->MemoryBase1)),
    CardBus->BridgeControl & PCI_BIT_9 ? L"    Prefetchable" : L"Non-Prefetchable",
    CardBus->MemoryBase1  & 0xfffff000, 
    CardBus->MemoryLimit1 | 0x00000fff 
    );
    
  Io32Bit = (BOOLEAN)(CardBus->IoBase0 & PCI_BIT_0);
  Print (
    L"I/O(0x%x)      %s          %E%08x%N             %E%08x%N\n", 
    INDEX_OF (&(CardBus->IoBase0)),
    Io32Bit ? L"          32 bit" : L"          16 bit",
    CardBus->IoBase0  & (Io32Bit ? 0xfffffffc : 0x0000fffc), 
    CardBus->IoLimit0 & (Io32Bit ? 0xffffffff : 0x0000ffff) | 0x00000003
    );

  Io32Bit = (BOOLEAN)(CardBus->IoBase1 & PCI_BIT_0);
  Print (
    L"I/O(0x%x)      %s          %E%08x%N             %E%08x%N\n", 
    INDEX_OF (&(CardBus->IoBase1)),
    Io32Bit ? L"          32 bit" : L"          16 bit",
    CardBus->IoBase1  & (Io32Bit ? 0xfffffffc : 0x0000fffc), 
    CardBus->IoLimit1 & (Io32Bit ? 0xffffffff : 0x0000ffff) | 0x00000003
    );

  //    
  // Print register Interrupt Line & PIN
  //
  Print (
    L"\nInterrupt Line(0x%x):     %E%02x%N             Interrupt Pin(0x%x):       %E%02x%N\n", 
    INDEX_OF (&(CardBus->InterruptLine)), 
    CardBus->InterruptLine,
    INDEX_OF (&(CardBus->InterruptPin)),  
    CardBus->InterruptPin
    );

  //
  // Page break here
  //
  ST->ConOut->QueryMode (
                ST->ConOut, 
                ST->ConOut->Mode->Mode, 
                &TempColumn, 
                &ScreenSize
                );
                
  if (ScreenSize != 0) {
    Input (
      L"Press Enter to Continue:", 
      ReturnStr, 
      sizeof(ReturnStr)/sizeof(CHAR16)
      );
      
    Print (L"\n\n");
  }
  
  //
  // Print register Bridge Control
  //
  PciExplainBridgeControl(&(CardBus->BridgeControl), PciCardBusBridge);
  
  //
  // Print some registers in data region of PCI configuration space for cardbus
  // bridge. Fields include: Sub VendorId, Subsystem ID, and Legacy Mode Base
  // Address.
  //
  CardBusData = (PCI_CARDBUS_DATA *)
    ((UINT8 *)CardBus + sizeof(PCI_CARDBUS_HEADER));

  Print (
    L"\nSub VendorID(0x%x):     %E%04x%N             Subsystem ID(0x%x):      %E%04x%N\n", 
    INDEX_OF (&(CardBusData->SubVendorId)), 
    CardBusData->SubVendorId,
    INDEX_OF (&(CardBusData->SubSystemId)), 
    CardBusData->SubSystemId
    );
  
  Print (L"Optional 16-Bit PC Card legacy Mode Base Address(0x%x): %E%08x%N\n",
    INDEX_OF (&(CardBusData->LegacyBase)), 
    CardBusData->LegacyBase 
    );
  
  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainStatus (
  IN UINT16                                 *Status,
  IN BOOLEAN                                MainStatus,
  IN PCI_HEADER_TYPE                        HeaderType
  )
/*++

Routine Description:

  Explain each meaningful bit of register Status. The definition of Status is
  slightly different depending on the PCI header type.

Arguments:

  Status          Points to the content of register Status
  MainStatus      Indicates if this register is main status(not secondary 
                  status)
  HeaderType      Header type of this PCI device
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  if (MainStatus) {
    Print (L"Status(0x%x): %E%x%N\n", INDEX_OF (Status), *Status );
  
  } else {
    Print (L"Secondary Status(0x%x): %E%x%N\n", INDEX_OF (Status), *Status );
  }
  
  Print (  
    L"  (04)New Capabilities linked list:   %E%d%N",
    (*Status & PCI_BIT_4) != 0
    );
  
  //
  // Bit 5 is meaningless for CardBus Bridge
  //
  if (HeaderType == PciCardBusBridge) {
    Print (
      L"  (05)66MHz Capable:                %EN/A%N\n", 
      (*Status & PCI_BIT_5) != 0
      );
  
  } else {
    Print (
      L"  (05)66MHz Capable:                  %E%d%N\n",
      (*Status & PCI_BIT_5) != 0
      );
  }
    
  Print (
    L"  (07)Fast Back-to-Back Capable:      %E%d%N",
    (*Status & PCI_BIT_7) != 0
    );
  
  Print (  
    L"  (08)Master Data Parity Error:       %E%d%N\n",
    (*Status & PCI_BIT_8) != 0
    );

  //
  // Bit 9 and bit 10 together decides the DEVSEL timing
  //
  Print (  L"  (09)DEVSEL timing:           ");
  if ((*Status & PCI_BIT_9) == 0 && (*Status & PCI_BIT_10) == 0) {
    Print (L"%E    Fast%N");
  
  } else if ((*Status & PCI_BIT_9) != 0 && (*Status & PCI_BIT_10) == 0) {
    Print (L"%E  Medium%N");
  
  } else if ((*Status & PCI_BIT_9) == 0 && (*Status & PCI_BIT_10) != 0) {
    Print (L"%E    Slow%N");
  
  } else {
    Print (L"%EReserved%N");
  }
      
  Print (  
    L"  (11)Signaled Target Abort:          %E%d%N\n",   
    (*Status & PCI_BIT_11) != 0
    );
    
  Print (
    L"  (12)Received Target Abort:          %E%d%N",
    (*Status & PCI_BIT_12) != 0
    );
    
  Print (
    L"  (13)Received Master Abort:          %E%d%N\n",
    (*Status & PCI_BIT_13) != 0
    );
    
  
  if (MainStatus) {
    Print (L"  (14)Signaled System Error:          %E%d%N",
      (*Status & PCI_BIT_14) != 0);

  } else {
    Print (L"  (14)Received System Error:          %E%d%N",
      (*Status & PCI_BIT_14) != 0);
  }
  
  Print (  L"  (15)Detected Parity Error:          %E%d%N\n",
    (*Status & PCI_BIT_15) != 0);
  
  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainCommand (
  IN UINT16                                 *Command
  )
/*++

Routine Description:

  Explain each meaningful bit of register Command. 

Arguments:

  Command         Points to the content of register Command
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  //
  // Print the binary value of register Command
  //
  Print (L"Command(0x%x): %E%04x%N\n", INDEX_OF (Command), *Command);

  //
  // Explain register Command bit by bit
  //
  Print (
    L"  (00)I/O space access enabled:       %E%d%N",
    (*Command & PCI_BIT_0) != 0
    );
    
  Print (
    L"  (01)Memory space access enabled:    %E%d%N\n",
    (*Command & PCI_BIT_1) != 0
    );
    
  Print (
    L"  (02)Behave as bus master:           %E%d%N",
    (*Command & PCI_BIT_2) != 0
    );
    
  Print (
    L"  (03)Monitor special cycle enabled:  %E%d%N\n",
    (*Command & PCI_BIT_3) != 0
    );
    
  Print (
    L"  (04)Mem Write & Invalidate enabled: %E%d%N",
    (*Command & PCI_BIT_4) != 0
    );
    
  Print (
    L"  (05)Palette snooping is enabled:    %E%d%N\n",
    (*Command & PCI_BIT_5) != 0
    );
    
  Print (
    L"  (06)Assert PERR# when parity error: %E%d%N",
    (*Command & PCI_BIT_6) != 0
    );
    
  Print (
    L"  (07)Do address/data stepping:       %E%d%N\n",
    (*Command & PCI_BIT_7) != 0
    );
    
  Print (
    L"  (08)SERR# driver enabled:           %E%d%N",
    (*Command & PCI_BIT_8) != 0
    );
    
  Print (
    L"  (09)Fast back-to-back transact...:  %E%d%N\n\n",
    (*Command & PCI_BIT_9) != 0
    );
  
  return EFI_SUCCESS;
}
  
EFI_STATUS
PciExplainBridgeControl (
  IN UINT16                                 *BridgeControl,
  IN PCI_HEADER_TYPE                        HeaderType
  )
/*++

Routine Description:

  Explain each meaningful bit of register Bridge Control. 

Arguments:

  BridgeControl   Points to the content of register Bridge Control
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  Print (
    L"Bridge Control(0x%x)     %E%04x%N\n", 
    INDEX_OF (BridgeControl), 
    *BridgeControl
    );
    
  Print (  L"  (00)Parity Error Response:          %E%d%N",   
    (*BridgeControl & PCI_BIT_0) != 0);
  Print (  L"  (01)SERR# Enable:                   %E%d%N\n", 
    (*BridgeControl & PCI_BIT_1) != 0);
  Print (  L"  (02)ISA Enable:                     %E%d%N",   
    (*BridgeControl & PCI_BIT_2) != 0);
  Print (  L"  (03)VGA Enable:                     %E%d%N\n", 
    (*BridgeControl & PCI_BIT_3) != 0);
  Print (  L"  (05)Master Abort Mode:              %E%d%N",   
    (*BridgeControl & PCI_BIT_5) != 0);
  
  //
  // Register Bridge Control has some slight differences between P2P bridge 
  // and Cardbus bridge from bit 6 to bit 11.
  //
  if (HeaderType == PciP2pBridge) {
    Print (  L"  (06)Secondary Bus Reset:            %E%d%N\n", 
      (*BridgeControl & PCI_BIT_6) != 0);
    Print (  L"  (07)Fast Back-to-Back Enable:       %E%d%N",   
      (*BridgeControl & PCI_BIT_7) != 0);
    Print (  L"  (08)Primary Discard Timer:       %E%s%N\n", 
      (*BridgeControl & PCI_BIT_8) ? L"2^10" : L"2^15");
    Print (  L"  (09)Secondary Discard Timer:     %E%s%N",   
      (*BridgeControl & PCI_BIT_9) ? L"2^10" : L"2^15");
    Print (  L"  (10)Discard Timer Status:           %E%d%N\n", 
      (*BridgeControl & PCI_BIT_10) != 0);  
    Print (  L"  (11)Discard Timer SERR# Enable:     %E%d%N\n",   
      (*BridgeControl & PCI_BIT_11) != 0);
  
  } else {
    Print (  L"  (06)CardBus Reset:                  %E%d%N\n", 
      (*BridgeControl & PCI_BIT_6) != 0);
    Print (  L"  (07)IREQ/INT Enable:                %E%d%N",
      (*BridgeControl & PCI_BIT_7) != 0);
    Print (  L"  (10)Write Posting Enable:           %E%d%N\n", 
      (*BridgeControl & PCI_BIT_10) != 0);
  }
  return EFI_SUCCESS;
}
