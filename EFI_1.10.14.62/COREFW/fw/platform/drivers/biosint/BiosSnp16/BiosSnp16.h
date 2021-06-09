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

  BiosSnp16.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_SNP_16_H
#define _BIOS_SNP_16_H

#include "Efi.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "Pxe.h"
#include "Int86.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)

// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (EfiNetworkInterfaceIdentifier)
#include EFI_PROTOCOL_DEFINITION (SimpleNetwork)

//
// BIOS Simple Network Protocol Device Structure
//
#define EFI_SIMPLE_NETWORK_DEV_SIGNATURE   EFI_SIGNATURE_32('s','n','1','6')

#define INIT_PXE_STATUS  0xabcd

#define EFI_SIMPLE_NETWORK_MAX_TX_FIFO_SIZE  64

typedef struct {
  UINT32  First;
  UINT32  Last;
  VOID    *(Data[EFI_SIMPLE_NETWORK_MAX_TX_FIFO_SIZE]);
} EFI_SIMPLE_NETWORK_DEV_FIFO;

typedef struct {
  UINTN                                      Signature;
  EFI_HANDLE                                 Handle;
  EFI_SIMPLE_NETWORK_PROTOCOL                SimpleNetwork;
  EFI_SIMPLE_NETWORK_MODE                    SimpleNetworkMode;
  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL  NII;
  EFI_DEVICE_PATH_PROTOCOL                   *DevicePath;
  EFI_PCI_IO_PROTOCOL                        *PciIo;

  //
  // Local Data for Simple Network Protocol interface goes here
  //

  BOOLEAN                       UndiLoaded;
  EFI_EVENT                     Event;
  UINT16                        PxeEntrySegment;
  UINT16                        PxeEntryOffset;
  EFI_SIMPLE_NETWORK_DEV_FIFO   TxBufferFifo;
  EFI_DEVICE_PATH_PROTOCOL      *BaseDevicePath;
  PXE_t                         *Pxe;           // Pointer to !PXE structure
  PXENV_UNDI_GET_INFORMATION_t  GetInformation; // Data from GET INFORMATION
  PXENV_UNDI_GET_NIC_TYPE_t     GetNicType;     // Data from GET NIC TYPE
  PXENV_UNDI_GET_NDIS_INFO_t    GetNdisInfo;    // Data from GET NDIS INFO
  BOOLEAN                       IsrValid;       // TRUE if Isr contains valid data
  PXENV_UNDI_ISR_t              Isr;            // Data from ISR
  PXENV_UNDI_TBD_t              *Xmit;          // 
  VOID                          *TxRealModeMediaHeader; // < 1 MB Size = 0x100
  VOID                          *TxRealModeDataBuffer;  // < 1 MB Size = GetInformation.MaxTranUnit
  VOID                          *TxDestAddr;            // < 1 MB Size = 16
  UINT8                         InterruptStatus;   // returned/cleared by GetStatus, set in ISR
  UINTN                         UndiLoaderTablePages;
  UINTN                         DestinationDataSegmentPages;
  UINTN                         DestinationStackSegmentPages;
  UINTN                         DestinationCodeSegmentPages;
  VOID                          *UndiLoaderTable;
  VOID                          *DestinationDataSegment;
  VOID                          *DestinationStackSegment;
  VOID                          *DestinationCodeSegment;
} EFI_SIMPLE_NETWORK_DEV;

#define EFI_SIMPLE_NETWORK_DEV_FROM_THIS(a) CR(a, EFI_SIMPLE_NETWORK_DEV, SimpleNetwork, EFI_SIMPLE_NETWORK_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gBiosSnp16DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gBiosSnp16ComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
BiosSnp16DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosSnp16DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
BiosSnp16DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

//
// Simple Network Protocol functions
//
EFI_STATUS
Undi16SimpleNetworkStart (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  );

EFI_STATUS
Undi16SimpleNetworkStop (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  );

EFI_STATUS
Undi16SimpleNetworkInitialize (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN UINTN                        ExtraRxBufferSize  OPTIONAL,
	IN UINTN                        ExtraTxBufferSize  OPTIONAL
  );

EFI_STATUS
Undi16SimpleNetworkReset (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN BOOLEAN                      ExtendedVerification
  );

EFI_STATUS
Undi16SimpleNetworkShutdown (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  );

EFI_STATUS
Undi16SimpleNetworkReceiveFilters (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN UINT32                       Enable,
	IN UINT32                       Disable,
	IN BOOLEAN                      ResetMCastFilter,
	IN UINTN                        MCastFilterCnt     OPTIONAL,
	IN EFI_MAC_ADDRESS              *MCastFilter       OPTIONAL
  );

EFI_STATUS
Undi16SimpleNetworkStationAddress (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN BOOLEAN                      Reset,
	IN EFI_MAC_ADDRESS              *New       OPTIONAL
  );

EFI_STATUS
Undi16SimpleNetworkStatistics (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN BOOLEAN                      Reset,
	IN OUT UINTN                    *StatisticsSize   OPTIONAL,
	OUT EFI_NETWORK_STATISTICS      *StatisticsTable  OPTIONAL
  );

EFI_STATUS
Undi16SimpleNetworkMCastIpToMac (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN BOOLEAN                      IPv6,
	IN EFI_IP_ADDRESS               *IP,
	OUT EFI_MAC_ADDRESS             *MAC
  );

EFI_STATUS
Undi16SimpleNetworkNvData (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN BOOLEAN                      Write,
	IN UINTN                        Offset,
	IN UINTN                        BufferSize,
	IN OUT VOID                     *Buffer
  );

EFI_STATUS
Undi16SimpleNetworkGetStatus (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	OUT UINT32                      *InterruptStatus  OPTIONAL,
	OUT VOID                        **TxBuf           OPTIONAL
  );

EFI_STATUS
Undi16SimpleNetworkTransmit (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	IN UINTN                        HeaderSize,
	IN UINTN                        BufferSize,
	IN VOID                         *Buffer,
	IN EFI_MAC_ADDRESS              *SrcAddr     OPTIONAL,
	IN EFI_MAC_ADDRESS              *DestAddr    OPTIONAL,
	IN UINT16                       *Protocol    OPTIONAL
  );

EFI_STATUS
Undi16SimpleNetworkReceive (
	IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
	OUT UINTN                       *HeaderSize  OPTIONAL,
	IN OUT UINTN                    *BufferSize,
	OUT VOID                        *Buffer,
	OUT EFI_MAC_ADDRESS             *SrcAddr      OPTIONAL,
	OUT EFI_MAC_ADDRESS             *DestAddr     OPTIONAL,
	OUT UINT16                      *Protocol    OPTIONAL
  );

//
//
//

STATIC VOID
Undi16SimpleNetworkEvent (
	IN EFI_EVENT  Event,
	IN VOID       *Context
  );

static
EFI_STATUS
Undi16SimpleNetworkLoadUndi (
	EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  );

static
EFI_STATUS
Undi16SimpleNetworkUnloadUndi (
	EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  );

VOID 
Undi16SimpleNetworkWaitForPacket (
	IN EFI_EVENT               Event,
 	IN VOID                    *Context
  );

EFI_STATUS 
Undi16SimpleNetworkCheckForPacket (
	IN EFI_SIMPLE_NETWORK_PROTOCOL *This 
  );

EFI_STATUS 
SimpleNetworkAppendMacAddressDevicePath (
	EFI_DEVICE_PATH_PROTOCOL             **DevicePath,
	EFI_MAC_ADDRESS *CurrentAddress
	);

EFI_STATUS
CacheVectorAddress (
  UINT8   VectorNumber
  );

EFI_STATUS
RestoreCachedVectorAddress (
  UINT8   VectorNumber
  );

EFI_STATUS
LaunchBaseCode(
	EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice,
  UINTN                   RomAddress
  );

EFI_STATUS
PxeStartUndi (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_START_UNDI_t               *PxeUndiTable 
    );

EFI_STATUS
PxeUndiStartup (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_STARTUP_t             *PxeUndiTable 
    );

EFI_STATUS
PxeUndiCleanup (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_CLEANUP_t             *PxeUndiTable 
    );

EFI_STATUS
PxeUndiInitialize (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_INITIALIZE_t          *PxeUndiTable 
    );

EFI_STATUS
PxeUndiResetNic (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_RESET_t               *PxeUndiTable,
    IN UINT16 rx_filter
    );
    
EFI_STATUS
PxeUndiShutdown (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_SHUTDOWN_t            *PxeUndiTable 
    );

EFI_STATUS
PxeUndiOpen (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_OPEN_t                *PxeUndiTable 
    );

EFI_STATUS
PxeUndiClose (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_CLOSE_t               *PxeUndiTable 
    );

EFI_STATUS
PxeUndiTransmit (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_TRANSMIT_t            *PxeUndiTable 
    );

EFI_STATUS
PxeUndiSetMcastAddr (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_SET_MCAST_ADDR_t      *PxeUndiTable 
    );

EFI_STATUS
PxeUndiSetStationAddr (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_SET_STATION_ADDR_t    *PxeUndiTable     
    );

EFI_STATUS
PxeUndiSetPacketFilter (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_SET_PACKET_FILTER_t   *PxeUndiTable 
    );

EFI_STATUS
PxeUndiGetInformation (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_GET_INFORMATION_t     *PxeUndiTable 
    );

EFI_STATUS
PxeUndiGetStatistics (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_GET_STATISTICS_t      *PxeUndiTable 
    );

EFI_STATUS
PxeUndiClearStatistics (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_CLEAR_STATISTICS_t    *PxeUndiTable 
    );

EFI_STATUS
PxeUndiInitiateDiags (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_INITIATE_DIAGS_t      *PxeUndiTable 
    );

EFI_STATUS
PxeUndiForceInterrupt (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_FORCE_INTERRUPT_t     *PxeUndiTable 
    );

EFI_STATUS
PxeUndiGetMcastAddr (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_GET_MCAST_ADDR_t      *PxeUndiTable 
    );

EFI_STATUS
PxeUndiGetNicType (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_GET_NIC_TYPE_t        *PxeUndiTable 
    );

EFI_STATUS
PxeUndiGetNdisInfo (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_GET_NDIS_INFO_t       *PxeUndiTable 
    );

EFI_STATUS
PxeUndiIsr (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_ISR_t                 *PxeUndiTable 
    );

EFI_STATUS
PxeUndiStop (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_STOP_UNDI_t                *PxeUndiTable 
    );

EFI_STATUS
PxeUndiGetState (
  	IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
    IN OUT PXENV_UNDI_GET_STATE_t           *PxeUndiTable 
    );

EFI_STATUS
MakePxeCall (
	EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice,
  IN OUT VOID*    pTable,
  IN UINTN        TableSize,
  IN UINT16       CallIndex
  );

EFI_STATUS
BiosSnp16AllocatePagesBelowOneMb (
  UINTN  NumPages,
  VOID   **Buffer
  );

#endif
