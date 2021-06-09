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

    Usb.c

  Abstract:

    Parse usb device configurations.

  Revision History

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(UsbIo)

#include "usb.h"
#include EFI_PROTOCOL_DEFINITION(UsbHostController)
#include "usbbus.h"
#include "usblib.h"

//
// Here are some internal helper functions
//
static EFI_STATUS
GetExpectedDescriptor(
  IN  UINT8*      Buffer,
  IN  UINTN       Length,
  IN  UINT8       DescType,
  IN  UINT8       DescLength,
  OUT UINTN       *ParsedBytes
  );

static EFI_STATUS
ParseThisEndpoint (
  IN  ENDPOINT_DESC_LIST_ENTRY    *EndpointEntry,
  IN  UINT8                       *Buffer,
  IN  UINTN                       BufferLength,
  OUT UINTN                       *ParsedBytes
  );

static EFI_STATUS
ParseThisInterface(
  IN  INTERFACE_DESC_LIST_ENTRY   *InterfaceEntry,
  IN  UINT8                       *Buffer,
  IN  UINTN                       *BufferLen,
  OUT UINTN                       *ParsedBytes
  );

static EFI_STATUS
ParseThisConfig(
  IN  CONFIG_DESC_LIST_ENTRY  *ConfigDescEntry,
  IN  UINT8                   *Buffer,
  IN  UINTN                   Length
  );

//
// Implementations
//
BOOLEAN
IsHub (
  IN USB_IO_CONTROLLER_DEVICE *Dev
  )
/*++
  
  Routine Description:
    Tell if a usb controller is a hub controller.
    
  Parameters:
    Dev     -     UsbIoController device structure.
    
  Return Value:
    TRUE/FALSE
    
--*/
{
  EFI_USB_INTERFACE_DESCRIPTOR    Interface;
  EFI_USB_IO_PROTOCOL             *UsbIo;
  EFI_USB_ENDPOINT_DESCRIPTOR     EndpointDescriptor;
  UINT8                           i;
  
  if (Dev == NULL) {
    return FALSE;
  }

  UsbIo = &Dev->UsbIo;

  UsbIo->UsbGetInterfaceDescriptor(
           UsbIo,
           &Interface
         );

  //
  // Check classcode
  //
  if(Interface.InterfaceClass != 0x09) {
    return FALSE;
  }

  //
  // Check sub-class code
  //
  if(Interface.InterfaceSubClass != 0) {
    return FALSE;
  }

  //
  // Check protocol
  //
  if(Interface.InterfaceProtocol != 0x0) {
    return FALSE;
  }

  for (i = 0; i < Interface.NumEndpoints; i++) {
    UsbIo->UsbGetEndpointDescriptor(
             UsbIo,
             i,
             &EndpointDescriptor
         );
         
    if ((EndpointDescriptor.EndpointAddress & 0x80) == 0) {
      continue;
    }
    
    if (EndpointDescriptor.Attributes != 0x03) {
      continue;
    }
    
    Dev->HubEndpointAddress = EndpointDescriptor.EndpointAddress;
    return TRUE;
  }
  
  return FALSE;
}

EFI_STATUS
UsbGetStringtable (
  IN  USB_IO_DEVICE *Dev
  )
/*++
  
  Routine Description:
    Get the string table stored in a usb device.
    
  Parameters:
    Dev     -     UsbIoController device structure.
    
  Return Value:
    EFI_SUCCESS
    EFI_UNSUPPORTED
    EFI_OUT_OF_RESOURCES
    
--*/
{
  EFI_STATUS                  Result;
  UINT32                      Status;
  EFI_USB_SUPPORTED_LANGUAGES *LanguageTable;
  UINT8                       *Buffer, *ptr;
  UINTN                       i, LangTableSize;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  UINT16                      TempBuffer;

  UsbIo = &(Dev->UsbController[0]->UsbIo);

  //
  // We get first 2 byte of langID table,
  // so we can have the whole table length
  //
  Result = UsbGetString(
                UsbIo,
                0,
                0,
                &TempBuffer,
                2,
                &Status
            );
  if(EFI_ERROR(Result)) {
    return EFI_UNSUPPORTED;
  }

  LanguageTable = (EFI_USB_SUPPORTED_LANGUAGES *)&TempBuffer;

  if (LanguageTable->Length == 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // If length is 2, then there is no string table
  //
  if (LanguageTable->Length == 2) {
    return EFI_UNSUPPORTED;
  }

  Result = gBS->AllocatePool (
                  EfiBootServicesData,
                  LanguageTable->Length,
                  &Buffer
                );

  if (EFI_ERROR (Result)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  EfiZeroMem(Buffer, LanguageTable->Length);

  //
  // Now we get the whole LangID table
  //
  Result = UsbGetString(
                  UsbIo,
                  0,
                  0,
                  Buffer,
                  LanguageTable->Length,
                  &Status
            );
  if (EFI_ERROR (Result)) {
    gBS->FreePool(Buffer);
    return EFI_UNSUPPORTED;
  }

  LanguageTable = (EFI_USB_SUPPORTED_LANGUAGES *)Buffer;

  //
  // ptr point to the LangID table
  //
  ptr = Buffer + 2;
  LangTableSize = (LanguageTable->Length - 2) / 2;

  for(i = 0; i < LangTableSize && i< USB_MAXLANID; i++) {
    Dev->LangID[i] = *((UINT16*)ptr);
    ptr += 2;
  }

  gBS->FreePool(Buffer);
  LanguageTable = NULL;

  return EFI_SUCCESS;
}


EFI_STATUS
UsbGetAllConfigurations (
  IN  USB_IO_DEVICE *UsbDev
  )
/*++

  Routine Description:
    This function is to parse all the configuration descriptor.
    
  Parameters:
    Dev     -     USB_IO_DEVICE device structure.
    
  Return Values:
    EFI_SUCCESS
    EFI_DEVICE_ERROR
    EFI_OUT_OF_RESOURCES  

--*/
{
  EFI_STATUS               Result;
  UINT32                   Status;
  UINTN                    i;
  UINTN                    TotalLength;
  UINT8                    *Buffer;
  CONFIG_DESC_LIST_ENTRY   *ConfigDescEntry;
  EFI_USB_IO_PROTOCOL      *UsbIo;

  UsbIo = &(UsbDev->UsbController[0]->UsbIo);

  for(i = 0; i < UsbDev->DeviceDescriptor.NumConfigurations; i++) {
    ConfigDescEntry = NULL;

    Result = gBS->AllocatePool(
                    EfiBootServicesData,
                    sizeof(CONFIG_DESC_LIST_ENTRY),
                    &ConfigDescEntry
                  );

    if (EFI_ERROR (Result)) {
      return EFI_OUT_OF_RESOURCES;
    }

    EfiZeroMem(ConfigDescEntry, sizeof(CONFIG_DESC_LIST_ENTRY));

    //
    // 1st only get 1st 4 bytes config descriptor,
    // so we can know the whole length
    //
    Result = UsbGetDescriptor(
                    UsbIo,
                    0x0200,
                    (UINT8)i,
                    4,
                    &ConfigDescEntry->CongfigDescriptor,
                    &Status
             );
    if (EFI_ERROR (Result)) {
      DEBUG ((EFI_D_ERROR, "Get 1st config descriptor error\n"));
      gBS->FreePool(ConfigDescEntry);
      return EFI_DEVICE_ERROR;
    }

    TotalLength = (ConfigDescEntry->CongfigDescriptor).TotalLength;

    Buffer = NULL;
    Result = gBS->AllocatePool(
                    EfiBootServicesData,
                    TotalLength,
                    &Buffer
                  );

    if (EFI_ERROR (Result)) {
      gBS->FreePool(ConfigDescEntry);
      return EFI_OUT_OF_RESOURCES;
    }

    EfiZeroMem(Buffer, TotalLength);

    //
    // Then we get the total descriptors for this configuration
    //
    Result = UsbGetDescriptor(
                UsbIo,
                0x0200,
                (UINT16)i,
                (UINT16)TotalLength,
                Buffer,
                &Status
             );
    if (EFI_ERROR (Result)) {
      DEBUG((EFI_D_ERROR, "Get whole config descriptor error\n"));
      gBS->FreePool(ConfigDescEntry);
      gBS->FreePool(Buffer);
      return EFI_DEVICE_ERROR;
    }

    InitializeListHead(&ConfigDescEntry->InterfaceDescListHead);

    //
    // Parse this whole configuration
    //
    Result = ParseThisConfig(ConfigDescEntry, Buffer, TotalLength);

    if (EFI_ERROR (Result)) {
      //
      // Ignore this configuration, parse next one
      //
      gBS->FreePool(ConfigDescEntry);
      gBS->FreePool(Buffer);
      continue;
    }

    InsertTailList(&UsbDev->ConfigDescListHead, &ConfigDescEntry->Link);

    gBS->FreePool(Buffer);

  }

  return EFI_SUCCESS;
}

static EFI_STATUS
GetExpectedDescriptor(
  IN  UINT8*      Buffer,
  IN  UINTN       Length,
  IN  UINT8       DescType,
  IN  UINT8       DescLength,
  OUT UINTN       *ParsedBytes
  )
/*++
  
  Routine Description:
    Get the start position of next wanted descriptor.
    
  Parameters:
  
  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  UINT16      DescriptorHeader;
  UINT8       Len;
  UINT8*      ptr;
  UINTN       Parsed;

  Parsed = 0;
  ptr = Buffer;

  while (TRUE) {
    //
    // Buffer length should not less than Desc length
    //
    if(Length < DescLength) {
      return EFI_DEVICE_ERROR;
    }

    //
    // DescriptorHeader = *((UINT16 *)ptr), compatible with IPF
    //
    DescriptorHeader = (UINT16)((*(ptr + 1) << 8) | *ptr);
    
    Len = ptr[0];

    //
    // Check to see if it is a start of expected descriptor
    //
    if (DescriptorHeader == ((DescType << 8) | DescLength)) {
      break;
    }

    if ((UINT8)(DescriptorHeader >> 8) == DescType) {
      if (Len > DescLength) {
        return EFI_DEVICE_ERROR;
      }
    }

    //
    // Descriptor length should be at least 2
    // and should not exceed the buffer length
    //
    if (Len < 2) {
      return EFI_DEVICE_ERROR;
    }

    if(Len > Length) {
      return EFI_DEVICE_ERROR;
    }

    //
    // Skip this mismatch descriptor
    //
    Length -= Len;
    ptr += Len;
    Parsed += Len;
  }

  *ParsedBytes = Parsed;

  return EFI_SUCCESS;
}


static
EFI_STATUS
ParseThisEndpoint (
  IN  ENDPOINT_DESC_LIST_ENTRY    *EndpointEntry,
  IN  UINT8                       *Buffer,
  IN  UINTN                       BufferLength,
  OUT UINTN                       *ParsedBytes
  )
/*++

  Routine Description:
    Get the start position of next wanted endpoint descriptor.

  Parameters:
  
  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  UINT8                       *ptr;
  EFI_STATUS                  Status;
  UINTN                       SkipBytes;

  //
  // Skip some data for this interface
  //
  Status = GetExpectedDescriptor (
              Buffer,
              BufferLength,
              USB_DT_ENDPOINT,
              sizeof (EFI_USB_ENDPOINT_DESCRIPTOR),
              &SkipBytes
           );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  ptr = Buffer + SkipBytes;
  *ParsedBytes = SkipBytes;

  EfiCopyMem (
    &EndpointEntry->EndpointDescriptor,
    ptr,
    sizeof (EFI_USB_ENDPOINT_DESCRIPTOR)
  );

  *ParsedBytes += sizeof(EFI_USB_ENDPOINT_DESCRIPTOR);

  return EFI_SUCCESS;
}

static
EFI_STATUS
ParseThisInterface(
  IN  INTERFACE_DESC_LIST_ENTRY   *InterfaceEntry,
  IN  UINT8                       *Buffer,
  IN  UINTN                       *BufferLen,
  OUT UINTN                       *ParsedBytes
  )
/*++

  Routine Description:
    Get the start position of next wanted interface descriptor.

  Parameters:
  
  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  UINT8                       *ptr;
  UINTN                       SkipBytes;
  UINTN                       i;
  UINTN                       Length;
  UINTN                       Parsed;
  ENDPOINT_DESC_LIST_ENTRY    *EndpointEntry;
  EFI_STATUS                  Status;

  //
  // Skip some data for this interface
  //
  Status = GetExpectedDescriptor (
            Buffer,
            *BufferLen,
            USB_DT_INTERFACE,
            sizeof(EFI_USB_INTERFACE_DESCRIPTOR),
            &SkipBytes
           );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  ptr = Buffer + SkipBytes;
  *ParsedBytes = SkipBytes;

  //
  // Copy the interface descriptor
  //
  EfiCopyMem (
    &InterfaceEntry->InterfaceDescriptor,
    ptr,
    sizeof(EFI_USB_INTERFACE_DESCRIPTOR)
  );

  ptr = Buffer + sizeof(EFI_USB_INTERFACE_DESCRIPTOR);
  *ParsedBytes += sizeof(EFI_USB_INTERFACE_DESCRIPTOR);

  InitializeListHead (&InterfaceEntry->EndpointDescListHead);

  Length = *BufferLen - SkipBytes - sizeof(EFI_USB_INTERFACE_DESCRIPTOR);

  for(i = 0; i < (InterfaceEntry->InterfaceDescriptor).NumEndpoints; i++) {
    EndpointEntry = NULL;
    Status = gBS->AllocatePool(
                    EfiBootServicesData,
                    sizeof(ENDPOINT_DESC_LIST_ENTRY),
                    &EndpointEntry
                  );

    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
    
    EfiZeroMem(EndpointEntry, sizeof(ENDPOINT_DESC_LIST_ENTRY));

    //
    // Parses all the endpoint descriptors within this interface.
    //
    Status = ParseThisEndpoint(EndpointEntry, ptr, Length, &Parsed);
    
    if (EFI_ERROR (Status)) {
      gBS->FreePool(EndpointEntry);
      return Status;
    }

    InsertTailList (
      &InterfaceEntry->EndpointDescListHead,
      &EndpointEntry->Link
    );

    Length -= Parsed;
    ptr += Parsed;
    *ParsedBytes += Parsed;
  }

  return EFI_SUCCESS;
}

static EFI_STATUS
ParseThisConfig(
  IN  CONFIG_DESC_LIST_ENTRY  *ConfigDescEntry,
  IN  UINT8                   *Buffer,
  IN  UINTN                   Length
  )
/*++

  Routine Description:
    Parse the current configuration descriptior.

  Parameters:
  
  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  UINT8                       *ptr;
  UINT8                       NumInterface;
  UINTN                       i;
  INTERFACE_DESC_LIST_ENTRY   *InterfaceEntry;
  UINTN                       SkipBytes;
  UINTN                       Parsed;
  EFI_STATUS                  Status;
  UINTN                       LengthLeft;

  //
  //  First skip the current config descriptor;
  //
  Status = GetExpectedDescriptor (
            Buffer,
            Length,
            USB_DT_CONFIG,
            sizeof(EFI_USB_CONFIG_DESCRIPTOR),
            &SkipBytes
           );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  ptr = Buffer + SkipBytes;

  EfiCopyMem (
    &ConfigDescEntry->CongfigDescriptor,
    ptr,
    sizeof(EFI_USB_CONFIG_DESCRIPTOR)
  );

  NumInterface = ConfigDescEntry->CongfigDescriptor.NumInterfaces;

  //
  // Skip size of Configuration Descriptor
  //
  ptr += sizeof(EFI_USB_CONFIG_DESCRIPTOR);

  LengthLeft = Length - SkipBytes - sizeof(EFI_USB_CONFIG_DESCRIPTOR);

  for(i = 0; i < NumInterface; i++) {
    //
    // Parse all Interface
    //
    InterfaceEntry = NULL;
    Status = gBS->AllocatePool(
                    EfiBootServicesData,
                    sizeof(INTERFACE_DESC_LIST_ENTRY),
                    &InterfaceEntry
                  );

    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    EfiZeroMem(InterfaceEntry, sizeof(INTERFACE_DESC_LIST_ENTRY));

    Status = ParseThisInterface(InterfaceEntry, ptr, &LengthLeft, &Parsed);
    if (EFI_ERROR (Status)) {
      gBS->FreePool(InterfaceEntry);
      return Status;
    }

    InsertTailList(
      &ConfigDescEntry->InterfaceDescListHead,
      &InterfaceEntry->Link
    );

    //
    // Parsed for next interface
    //
    LengthLeft -= Parsed;
    ptr += Parsed;
  }

  //
  // Parse for additional alt setting;
  //

  return EFI_SUCCESS;
}

EFI_STATUS
UsbSetConfiguration (
  IN  USB_IO_DEVICE   *Dev,
  IN  UINTN           ConfigurationValue
  )
/*++

  Routine Description:
    Set the device to a configuration value.
    
  Parameters:
    Dev                 -   USB_IO_DEVICE to be set configuration
    ConfigrationValue   -   The configuration value to be set to that device
    
  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR
    
--*/
{
  EFI_LIST_ENTRY          *NextEntry;
  CONFIG_DESC_LIST_ENTRY  *ConfigEntry;
  UINT32                  Status;
  EFI_STATUS              Result;
  EFI_USB_IO_PROTOCOL     *UsbIo;

  UsbIo = &(Dev->UsbController[0]->UsbIo);
  NextEntry = (Dev->ConfigDescListHead).ForwardLink;

  while(NextEntry != &Dev->ConfigDescListHead) {
    //
    // Get one entry
    //
    ConfigEntry = (CONFIG_DESC_LIST_ENTRY *)NextEntry;
    if ((ConfigEntry->CongfigDescriptor).ConfigurationValue
      == ConfigurationValue) {
      //
      // Find one, set to the active configuration
      //
      Dev->ActiveConfig = ConfigEntry;
      break;
    }
    NextEntry = NextEntry->ForwardLink;
  }

  //
  // Next Entry should not be null
  //

  Result = UsbSetDeviceConfiguration(
              UsbIo,
              (UINT16)ConfigurationValue,
              &Status
           );

  return Result;
}

EFI_STATUS
UsbSetDefaultConfiguration(
  IN  USB_IO_DEVICE      *Dev
  )
/*++

  Routine Description:
    Set the device to a default configuration value.
    
  Parameters:
    Dev       -    USB_IO_DEVICE to be set configuration
    
  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR
    
--*/
{
  CONFIG_DESC_LIST_ENTRY  *ConfigEntry;
  UINT16                  ConfigValue;
  EFI_LIST_ENTRY          *NextEntry;

  if (IsListEmpty (&Dev->ConfigDescListHead)) {
    return EFI_DEVICE_ERROR;
  }

  NextEntry = (Dev->ConfigDescListHead).ForwardLink;

  ConfigEntry = (CONFIG_DESC_LIST_ENTRY*) NextEntry;
  ConfigValue = (ConfigEntry->CongfigDescriptor).ConfigurationValue;

  return UsbSetConfiguration(Dev, ConfigValue);
}

VOID
UsbDestroyAllConfiguration (
  IN  USB_IO_DEVICE      *Dev
  )
/*++

  Routine Description:
    Delete all configuration data when device is not used.
    
  Parameter:
    Dev         -    USB_IO_DEVICE to be set configuration
  
  Return Value:
    N/A
    
--*/
{
  CONFIG_DESC_LIST_ENTRY      *ConfigEntry;
  INTERFACE_DESC_LIST_ENTRY   *InterfaceEntry;
  ENDPOINT_DESC_LIST_ENTRY    *EndpointEntry;
  EFI_LIST_ENTRY              *NextEntry;

  //
  // Delete all configuration descriptor data
  //
  ConfigEntry = (CONFIG_DESC_LIST_ENTRY*)(Dev->ConfigDescListHead).ForwardLink;

  while (ConfigEntry != (CONFIG_DESC_LIST_ENTRY *)&Dev->ConfigDescListHead) {
    //
    // Delete all its interface descriptors
    //
    InterfaceEntry = (INTERFACE_DESC_LIST_ENTRY *)ConfigEntry->InterfaceDescListHead.ForwardLink;

    while (InterfaceEntry != (INTERFACE_DESC_LIST_ENTRY *)&ConfigEntry->InterfaceDescListHead) {
      //
      // Delete all its endpoint descriptors
      //
      EndpointEntry = (ENDPOINT_DESC_LIST_ENTRY *)InterfaceEntry->EndpointDescListHead.ForwardLink;
      while (EndpointEntry != (ENDPOINT_DESC_LIST_ENTRY *)&InterfaceEntry->EndpointDescListHead) {
        NextEntry = ((EFI_LIST_ENTRY *)EndpointEntry)->ForwardLink;
        RemoveEntryList ((EFI_LIST_ENTRY *)EndpointEntry);
        gBS->FreePool (EndpointEntry);
        EndpointEntry = (ENDPOINT_DESC_LIST_ENTRY *)NextEntry;
      }

      NextEntry = ((EFI_LIST_ENTRY *)InterfaceEntry)->ForwardLink;
      RemoveEntryList((EFI_LIST_ENTRY *)InterfaceEntry);
      gBS->FreePool (InterfaceEntry);
      InterfaceEntry = (INTERFACE_DESC_LIST_ENTRY *)NextEntry;
    }

    NextEntry = ((EFI_LIST_ENTRY *)ConfigEntry)->ForwardLink;
    RemoveEntryList((EFI_LIST_ENTRY *)ConfigEntry);
    gBS->FreePool (ConfigEntry);
    ConfigEntry = (CONFIG_DESC_LIST_ENTRY *)NextEntry;
  }
}


