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

    ide.h

Abstract:

    Header file for IDE Bus Driver, containing the helper functions' 
    entire prototype.

Revision History
++*/

#ifndef _IDE_H
#define _IDE_H

//
// Helper functions Prototype
//  

EFI_STATUS 
DeRegisterIdeDevice ( 
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  EFI_HANDLE                     Handle
  );
  
EFI_STATUS 
EnableIdeDevice ( 
  IN EFI_HANDLE                          Controller,
  IN EFI_PCI_IO_PROTOCOL                 *PciIo,
  IN EFI_DEVICE_PATH_PROTOCOL            *ParentDevicePath,
  IN EFI_DEVICE_PATH_PROTOCOL            *RemainingDevicePath
  );

UINT8
IDEReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  );

VOID
IDEReadPortWMultiple (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINTN                 Count,
  OUT  VOID                  *Buffer
  );

VOID
IDEWritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT8                 Data
  );

VOID
IDEWritePortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT16                Data
  );

VOID
IDEWritePortWMultiple (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINTN                 Count,
  IN  VOID                  *Buffer
  );

EFI_STATUS
DiscoverIdeDevice (
  IN IDE_BLK_IO_DEV *IdeDev
  );

  
BOOLEAN
DetectIDEController (
  IN  IDE_BLK_IO_DEV  *IdeDev
  );

EFI_STATUS  
DRQClear (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );

EFI_STATUS  
DRQClear2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );

EFI_STATUS  
DRQReady (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );

EFI_STATUS  
DRQReady2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );

EFI_STATUS  
WaitForBSYClear (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );


EFI_STATUS  
WaitForBSYClear2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );

EFI_STATUS
DRDYReady (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           DelayInMilliSeconds
  );

EFI_STATUS  
DRDYReady2 (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  IN  UINTN           DelayInMilliSeconds
  );

VOID
SwapStringChars ( 
  IN CHAR8  *Destination,  
  IN CHAR8  *Source,   
  IN UINT32 Size  
  );
  
//
//  ATA device functions' prototype
//

EFI_STATUS                                                         
ATAIdentify (                                                  
  IN  IDE_BLK_IO_DEV  *IdeDev
  );

VOID 
PrintAtaModuleName(
  IN  IDE_BLK_IO_DEV  *IdeDev
  );

EFI_STATUS
AtaPioDataIn (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *Buffer,
  IN  UINT32          ByteCount,
  IN  UINT8           AtaCommand,
  IN  UINT8           Head,
  IN  UINT8           SectorCount,
  IN  UINT8           SectorNumber,
  IN  UINT8           CylinderLsb,
  IN  UINT8           CylinderMsb
  );

EFI_STATUS
AtaPioDataOut (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *Buffer,
  IN  UINT32          ByteCount,
  IN  UINT8           AtaCommand,
  IN  UINT8           Head,
  IN  UINT8           SectorCount,
  IN  UINT8           SectorNumber,
  IN  UINT8           CylinderLsb,
  IN  UINT8           CylinderMsb
  );

EFI_STATUS
CheckErrorStatus (
  IN  IDE_BLK_IO_DEV  *IdeDev
  );

EFI_STATUS
AtaReadSectors (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *DataBuffer, 
  IN  EFI_LBA         Lba, 
  IN  UINTN           NumberOfBlocks
  );

EFI_STATUS
AtaWriteSectors (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *BufferData, 
  IN  EFI_LBA         Lba, 
  IN  UINTN           NumberOfBlocks
  );

EFI_STATUS
AtaSoftReset (
  IN  IDE_BLK_IO_DEV  *IdeDev
  );

EFI_STATUS
AtaBlkIoReadBlocks (
  IN IDE_BLK_IO_DEV   *IdeBlkIoDevice,
  IN UINT32           MediaId,
  IN EFI_LBA          LBA,
  IN UINTN            BufferSize,
  OUT VOID            *Buffer
  );


EFI_STATUS
AtaBlkIoWriteBlocks (
  IN IDE_BLK_IO_DEV   *IdeBlkIoDevice,
  IN UINT32           MediaId,
  IN EFI_LBA          LBA,
  IN UINTN            BufferSize,
  OUT VOID            *Buffer
  );
  
//
// ATAPI device functions' prototype
// 


EFI_STATUS
ATAPIIdentify (                                                    
  IN  IDE_BLK_IO_DEV  *IdeDev
  );

EFI_STATUS 
AtapiInquiry (
  IN  IDE_BLK_IO_DEV  *IdeDev 
  );

EFI_STATUS 
AtapiPacketCommandIn (
  IN  IDE_BLK_IO_DEV        *IdeDev, 
  IN  ATAPI_PACKET_COMMAND  *Packet, 
  IN  UINT16                *Buffer, 
  IN  UINT32                ByteCount,
  IN  UINTN                 TimeOut
  );

EFI_STATUS 
AtapiPacketCommandOut (
  IN  IDE_BLK_IO_DEV        *IdeDev, 
  IN  ATAPI_PACKET_COMMAND  *Packet, 
  IN  UINT16                *Buffer, 
  IN  UINT32                ByteCount,
  IN  UINTN                 TimeOut
  );


EFI_STATUS 
PioReadWriteData (       
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  UINT16          *Buffer,   
  IN  UINT32          ByteCount,
  IN  BOOLEAN         Read,
  IN  UINTN           TimeOut
  );

EFI_STATUS                                                         
AtapiTestUnitReady (                                                  
  IN  IDE_BLK_IO_DEV  *IdeDev 
  );

EFI_STATUS  
AtapiRequestSense ( 
  IN  IDE_BLK_IO_DEV  *IdeDev,
  OUT UINTN           *SenseCounts
  );

EFI_STATUS 
AtapiReadCapacity (
  IN  IDE_BLK_IO_DEV  *IdeDev 
  );

EFI_STATUS 
AtapiDetectMedia (
  IN  IDE_BLK_IO_DEV  *IdeDev,
  OUT BOOLEAN         *MediaChange
  );
  
EFI_STATUS
AtapiReadSectors (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *Buffer, 
  IN  EFI_LBA         Lba, 
  IN  UINTN           NumberOfBlocks
  );

EFI_STATUS
AtapiWriteSectors (
  IN  IDE_BLK_IO_DEV  *IdeDev, 
  IN  VOID            *Buffer, 
  IN  EFI_LBA         Lba, 
  IN  UINTN           NumberOfBlocks
  );


EFI_STATUS
AtapiSoftReset (
  IN  IDE_BLK_IO_DEV  *IdeDev
  );


EFI_STATUS
AtapiBlkIoReadBlocks (
  IN IDE_BLK_IO_DEV   *IdeBlkIoDevice,
  IN UINT32           MediaId,
  IN EFI_LBA          LBA,
  IN UINTN            BufferSize,
  OUT VOID            *Buffer
  );

EFI_STATUS
AtapiBlkIoWriteBlocks (
  IN IDE_BLK_IO_DEV   *IdeBlkIoDevice,
  IN UINT32           MediaId,
  IN EFI_LBA          LBA,
  IN UINTN            BufferSize,
  OUT VOID            *Buffer
  );


BOOLEAN
IsNoMedia(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );


BOOLEAN
IsMediaError(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );

BOOLEAN
IsMediaChange(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );

BOOLEAN
IsDriveReady(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts,
  OUT BOOLEAN               *NeedRetry
  );

BOOLEAN
HaveSenseKey(
  IN  REQUEST_SENSE_DATA    *SenseData,
  IN  UINTN                 SenseCounts
  );

EFI_STATUS
IsLS120orZipWriteProtected(
  IN  IDE_BLK_IO_DEV    *IdeDev,
  OUT BOOLEAN           *WriteProtected
  );

VOID
ReleaseIdeResources (
  IN  IDE_BLK_IO_DEV  *IdeBlkIoDevice
  );

VOID
GetDeviceBestPIOMode (
  IN  IDE_BLK_IO_DEV  *IdeDev
  );
    
EFI_STATUS
IdeConfigureTiming (
  IN  IDE_BLK_IO_DEV  *IdeDev
  );

EFI_STATUS
SetPioMode(
  IN  IDE_BLK_IO_DEV  *IdeDev
    );  
    
#endif  
