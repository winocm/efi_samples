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

    BiosInt13.c
    
Abstract:

     Routines that use BIOS to support INT 13 devices.


Revision History

--*/
#include "BiosDriver.h"                                

INTN
PrintBlkIO (
    BIOS_BLK_IO_DEV     *Dev
    );

EFI_STATUS 
BiosBlkIoReset (
    IN  EFI_BLOCK_IO        *This,
    IN  BOOLEAN             ExtendedVerification
    ); 
    
EFI_STATUS 
BiosBlkIoReadBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );   
    
EFI_STATUS
BiosBlkIoWriteBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    );
    
EFI_STATUS 
BiosBlkIoFlushBlocks (
    IN  EFI_BLOCK_IO  *This
    ); 
    
STATIC UINTN
Int13GetDeviceParameters (
    IN BIOS_LEGACY_DRIVE    *Drive
    );  

STATIC UINTN
Int13Extensions(
    IN BIOS_LEGACY_DRIVE    *Drive
    );
    
STATIC UINTN                           
GetDriveParameters(
    IN  BIOS_LEGACY_DRIVE   *Drive
    );    

VOID
PrintOnDriveAccess (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer,
    IN  CHAR16              *TypeString
    );

//
// Address packet is a buffer under 1 MB for all version EDD calls
//
extern EDD_DEVICE_ADDRESS_PACKET   *GlobalEDDBufferUnder1MB;

//
// This is a buffer for INT 13h func 48 information
//
extern BIOS_LEGACY_DRIVE           *GlobalLegacyDriverUnder1MB;

//
// Buffer of 0xFE00 bytes for EDD 1.1 transfer must be under 1 MB
//  0xFE00 bytes is the max transfer size supported.
//
extern VOID                        *GlobalEDD11Buffer;

//
// Global Io Functions 
//
extern EFI_DEVICE_IO_INTERFACE   *GlobalIoFncs; 

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

UINTN
BiosGetNumberOfDiskettes (
    VOID
    )
{
    UINT8   NumberOfDrives;

#ifdef SOFT_SDV
    NumberOfDrives = 0;
    return NumberOfDrives;
#endif
    //
    // Read BIOS Data Area for Info
    //
    NumberOfDrives = *(UINT8 *)0x410;
    return (UINTN)(((NumberOfDrives >> 6) & 0x03) + 1);
}

UINTN
BiosGetNumberOfHardDrives (
    VOID
    )
{
    UINTN   NumberOfDrives;
    //
    // Return the number of drives from the BIOS data area
    //
    NumberOfDrives = (*(UINT8 *)0x0475);    
    return NumberOfDrives;
}


BOOLEAN
BiosInitBlkIo (
    IN  BIOS_BLK_IO_DEV         *Dev
    )
{
    EFI_BLOCK_IO                *BlkIo;
    EFI_BLOCK_IO_MEDIA          *BlkMedia;
    BIOS_LEGACY_DRIVE           *Bios;

    BlkIo = &Dev->BlkIo;
    BlkIo->Media = &Dev->BlkMedia;
    BlkMedia = BlkIo->Media;
    Bios = &Dev->Bios;
    if (Int13GetDeviceParameters (Bios)) {
        if (Int13Extensions (Bios)) {
            BlkMedia->LastBlock = (EFI_LBA)Bios->Parameters.PhysicalSectors - 1;
            BlkMedia->BlockSize = (UINT32)Bios->Parameters.BytesPerSector;

            if ((Bios->Parameters.Flags & EDD_DEVICE_REMOVABLE) == EDD_DEVICE_REMOVABLE) {
              BlkMedia->RemovableMedia = TRUE;
            }

        } else {
            //
            // Legacy Interfaces
            //
            BlkMedia->BlockSize = 512;
            BlkMedia->LastBlock = (Bios->MaxHead + 1) * Bios->MaxSector * (Bios->MaxCylinder + 1) - 1;
        }

        DEBUG ((D_INIT, "BlockSize = %d  LastBlock = %d\n",BlkMedia->BlockSize,BlkMedia->LastBlock));    

        BlkMedia->LogicalPartition = FALSE;
        BlkMedia->WriteCaching = FALSE;

        //
        // BugBug: Need to set this for removable media devices if they do not
        //  have media present
        //
        BlkMedia->ReadOnly = FALSE;
        BlkMedia->MediaPresent = TRUE; 
    
        BlkIo->Reset = BiosBlkIoReset;
        BlkIo->FlushBlocks = BiosBlkIoFlushBlocks;
        if (!Bios->ExtendedInt13) {
            //
            // Legacy interfaces
            //
            BlkIo->ReadBlocks = BiosReadLegacyDrive;    
            BlkIo->WriteBlocks = BiosWriteLegacyDrive;
        } else if ((Bios->EDDVersion == EDD_VERSION_30) && (Bios->Extensions64Bit)) {   
            //
            // EDD 3.0 Required for Device path, but extended reads are not required.
            //

            BlkIo->ReadBlocks = EDD30BiosReadBlocks;
            BlkIo->WriteBlocks = EDD30BiosWriteBlocks;
        } else {
            //
            // Assume EDD 1.1 - Read and Write functions. 
            //  This could be EDD 3.0 without Extensions64Bit being set.
            // If it's EDD 1.1 this will work, but the device path will not
            //  be correct. This will cause confusion to EFI OS installation.
            //  
            //  ASSERT (Bios->EDD);
            BlkIo->ReadBlocks = EDD11BiosReadBlocks;
            BlkIo->WriteBlocks = EDD11BiosWriteBlocks;
        }
    
        BlkMedia->LogicalPartition = FALSE;
        BlkMedia->WriteCaching = FALSE;

        return TRUE;
    } 
    return FALSE;
}

#if EFI_DEBUG
INTN
PrintBlkIO (
    BIOS_BLK_IO_DEV     *Dev
    )
{   
    BIOS_LEGACY_DRIVE   *Bios; 
    BOOLEAN             Valid = FALSE;
    
    Bios = &Dev->Bios;   

    Print (L"\n\n Letter %c Number %x EDD Version %x\n", Bios->Letter, Bios->Number, Bios->EDDVersion);
    Print (L" Block Size = %X\n", Dev->BlkIo.Media->BlockSize);
    if (Bios->ExtendedInt13) {
        Print (L"XINT13 ");
    }
    if (Bios->DriveLockingAndEjecting) { 
        Print (L"EJECT ");
    }
    if (Bios->EDD) {
        Print (L"EDD ");
    }
    if (Bios->Floppy) {
        Print (L"Floppy ");
    } 
    if (Bios->ParametersValid) {      
        Print (L"Valid ");            
        Valid = TRUE;
    }                     
    
    Print (L"\n");
    
    if (Valid) {
        Print (L" Sector Size %x", Bios->Parameters.BytesPerSector);
        Print (L" Size %lx", Bios->Parameters.PhysicalSectors);
        Print (L" Struct Size %x \n", Bios->Parameters.StructureSize);
    }
    return 0;
}
#endif

VOID
PrintOnDriveAccess (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer,
    IN  CHAR16              *TypeString
    )
{
    Print (L"BlkIo %s LBA %lx MediaID %x Size %x", TypeString, LBA, MediaId, BufferSize);
}

STATIC UINTN
Int13GetDeviceParameters (
    IN BIOS_LEGACY_DRIVE    *Drive
    )      
{
    UINTN               CarryFlag;
    UINT16              Cylinder;
    IA32_RegisterSet_t  Regs;
    
    Regs.h.AH = 0x08;
    Regs.h.DL = Drive->Number;
    CarryFlag = Int86(0x13, &Regs);
    DEBUG ((D_INIT, "Int13GetDeviceParameters: INT 13 08 DL=%02x : CF=%d AH=%02x\n",Drive->Number,CarryFlag,Regs.h.AH));    
    if (CarryFlag || Regs.h.AH != 0x00) {
        Drive->ErrorCode = Regs.h.AH;
        return FALSE;   
    }                         
    
    if (Drive->Floppy) {
        if (Regs.h.BL == 0x10) {
            Drive->ATAPI_Floppy = TRUE;
        } else {
            Drive->MaxHead = Regs.h.DH;
            Drive->MaxSector = Regs.h.CL;
            Drive->MaxCylinder = Regs.h.CH;
            if (Drive->MaxSector == 0) {
                //
                // BugBug: You can not trust the Carry flag. 
                //
                return FALSE;
            }
        }
    } else {
        Drive->MaxHead = Regs.h.DH & 0x3f;
        Cylinder = ((UINT16)Regs.h.DH & 0xc0) << 4;
        Cylinder |= ((UINT16)Regs.h.CL & 0xc0) << 2;
        Drive->MaxCylinder =  Cylinder + Regs.h.CH; 
        Drive->MaxSector = Regs.h.CL & 0x3f;
    }
    return TRUE;
}

STATIC UINTN
Int13Extensions(
    IN BIOS_LEGACY_DRIVE    *Drive
    )
{
    INTN CarryFlag;
    IA32_RegisterSet_t Regs;
    
    Regs.h.AH = 0x41;
    Regs.x.BX = 0x55aa;
    Regs.h.DL = Drive->Number;
    CarryFlag = Int86(0x13, &Regs);
    DEBUG ((D_INIT, "Int13Extensions: INT 13 41 DL=%02x : CF=%d BX=%04x\n",Drive->Number,CarryFlag,Regs.x.BX));    
    if (CarryFlag || Regs.x.BX != 0xaa55) {
        Drive->ExtendedInt13 = FALSE;
        Drive->DriveLockingAndEjecting = FALSE;
        Drive->EDD = FALSE;
        return(FALSE);  
    }
    Drive->EDDVersion = Regs.h.AH;
    Drive->ExtendedInt13 = (BOOLEAN)((Regs.x.CX & 0x01) == 0x01);
    Drive->DriveLockingAndEjecting = (BOOLEAN)((Regs.x.CX & 0x02) == 0x02);
    Drive->EDD = (BOOLEAN)((Regs.x.CX & 0x04) == 0x04);  
    Drive->Extensions64Bit = (BOOLEAN)(Regs.x.CX & 0x08);

    Drive->ParametersValid = (UINT8)GetDriveParameters(Drive);
    return Drive->ParametersValid;
}

STATIC UINTN                            
GetDriveParameters(
    IN  BIOS_LEGACY_DRIVE   *Drive
    )
{
    INTN CarryFlag;
    IA32_RegisterSet_t  Regs; 
    UINTN               PointerMath;
    
    Regs.h.AH = 0x48;
    Regs.h.DL = Drive->Number;

    //
    // EDD Buffer must be passed in with max buffer size as first entry in the buffer
    //
    GlobalLegacyDriverUnder1MB->Parameters.StructureSize = sizeof(EDD_DRIVE_PARAMETERS);
    Regs.x.DS = _FP_SEG (&GlobalLegacyDriverUnder1MB->Parameters);
    Regs.x.SI = _FP_OFF (&GlobalLegacyDriverUnder1MB->Parameters);
    CarryFlag = Int86(0x13, &Regs);
    DEBUG ((D_INIT, "GetDriveParameters: INT 13 48 DL=%02x : CF=%d AH=%02x\n",Drive->Number,CarryFlag,Regs.h.AH));    
    if (CarryFlag || Regs.h.AH != 0x00) {
        Drive->ErrorCode = Regs.h.AH;
        SetMem(&Drive->Parameters, sizeof(Drive->Parameters), 0xaf);
        return FALSE;   
    } 
   
    //
    // We only have one buffer < 1MB, so copy into our instance data
    //
    CopyMem (
        &Drive->Parameters, 
        &GlobalLegacyDriverUnder1MB->Parameters,
        sizeof(Drive->Parameters)
        );    

    if (Drive->ATAPI_Floppy) {

        //
        // Sense Media Type
        //

        Regs.h.AH = 0x20;
        Regs.h.DL = Drive->Number;
        CarryFlag = Int86(0x13, &Regs);
        DEBUG ((D_INIT, "GetDriveParameters: INT 13 20 DL=%02x : CF=%d AL=%02x\n",Drive->Number,CarryFlag,Regs.h.AL));    
        if (CarryFlag) {

            //
            // Media not present or unknown media present
            //

            if ((Drive->Parameters.Flags & EDD_GEOMETRY_VALID) == EDD_GEOMETRY_VALID) {
                Drive->MaxHead = (UINT8)(Drive->Parameters.MaxHeads - 1);
                Drive->MaxSector = (UINT8)Drive->Parameters.SectorsPerTrack;
                ASSERT(Drive->MaxSector != 0);
                Drive->MaxCylinder = (UINT16)Drive->Parameters.MaxCylinders - 1;
            } else {
                Drive->MaxHead = 0;
                Drive->MaxSector = 1;
                Drive->MaxCylinder = 0;
            }

        } else {

            //
            // Media Present
            //

            switch(Regs.h.AL) {
            case 0x03 :         // 720 KB
                Drive->MaxHead = 1;
                Drive->MaxSector = 9;
                Drive->MaxCylinder = 79;
                break;
            case 0x04 :         // 1.44MB
                Drive->MaxHead = 1;
                Drive->MaxSector = 18;
                Drive->MaxCylinder = 79;
                break;
            case 0x06 :         // 2.88MB
                Drive->MaxHead = 1;
                Drive->MaxSector = 36;
                Drive->MaxCylinder = 79;
                break;
            case 0x0C :         // 360 KB
                Drive->MaxHead = 1;
                Drive->MaxSector = 9;
                Drive->MaxCylinder = 39;
                break;
            case 0x0D :         // 1.2 MB
                Drive->MaxHead = 1;
                Drive->MaxSector = 15;
                Drive->MaxCylinder = 79;
                break;
            case 0x0E :         // Toshiba 3 mode
            case 0x0F :         // NEC 3 mode
            case 0x10 :         // Default Media
                if ((Drive->Parameters.Flags & EDD_GEOMETRY_VALID) == EDD_GEOMETRY_VALID) {
                    Drive->MaxHead = (UINT8)(Drive->Parameters.MaxHeads - 1);
                    Drive->MaxSector = (UINT8)Drive->Parameters.SectorsPerTrack;
                    ASSERT(Drive->MaxSector != 0);
                    Drive->MaxCylinder = (UINT16)Drive->Parameters.MaxCylinders - 1;
                } else {
                    Drive->MaxHead = 0;
                    Drive->MaxSector = 1;
                    Drive->MaxCylinder = 0;
                }
                break;
            default :         // Unknown media type.
                Drive->MaxHead = 0;
                Drive->MaxSector = 1;
                Drive->MaxCylinder = 0;
                break;
            }
        }
        Drive->Parameters.PhysicalSectors = (Drive->MaxHead + 1) * Drive->MaxSector * (Drive->MaxCylinder + 1);
        Drive->Parameters.BytesPerSector = 512;
    }

    //
    // This data comes from the BIOS so it may not allways be valid
    //  since the BIOS may reuse this buffer for future accesses
    // 
    PointerMath = _FP_SEG(Drive->Parameters.FDPT) << 4;
    PointerMath += _FP_OFF(Drive->Parameters.FDPT);
    Drive->FDPTPointer = (VOID *)PointerMath;
        
    return TRUE;
}   

//
// Block IO Routines
//

EFI_STATUS
EDD30BiosReadBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    )
{
    EFI_BLOCK_IO_MEDIA          *Media;
    BIOS_BLK_IO_DEV             *BiosBlkIoDev;  
    EDD_DEVICE_ADDRESS_PACKET   *AddressPacket; // I exist only for readability
    IA32_RegisterSet_t          Regs;
    UINT64                      TransferBuffer;
    UINTN                       NumberOfBlocks;
    UINTN                       TransferByteSize;
    UINTN                       BlockSize;
    BIOS_LEGACY_DRIVE           *Bios;
    UINTN                       CarryFlag;
    UINTN                       MaxTransferBlocks;
    EFI_BLOCK_IO *BlkIo;

    Media = This->Media;
    BlockSize = Media->BlockSize;

    //
    //Check buffer alignment
    //
    if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
      return EFI_INVALID_PARAMETER;
    }
    
    if (MediaId != Media->MediaId) {    
        return EFI_MEDIA_CHANGED;
    }

    if (LBA > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if ((LBA + (BufferSize / BlockSize) - 1) > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize % BlockSize != 0) {
        return EFI_BAD_BUFFER_SIZE;
    }

    if (Buffer == NULL) {    
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0 ) {    
        return EFI_SUCCESS;
    }

    BiosBlkIoDev = BIOS_BLK_IO_FROM_THIS (This);
    AddressPacket = GlobalEDDBufferUnder1MB; 

    MaxTransferBlocks = MAX_EDD11_XFER/BlockSize;

    TransferBuffer = (UINT64)Buffer;
    for (;BufferSize > 0;) {
        NumberOfBlocks = BufferSize/BlockSize;
        NumberOfBlocks = NumberOfBlocks > MaxTransferBlocks ? MaxTransferBlocks : NumberOfBlocks; // Max transfer MaxTransferBlocks
    
        AddressPacket->PacketSizeInBytes = sizeof(EDD_DEVICE_ADDRESS_PACKET); 
        AddressPacket->Zero = 0;
        AddressPacket->NumberOfBlocks = (UINT8)NumberOfBlocks;
        AddressPacket->Zero2 = 0; 
        AddressPacket->SegOffset = 0xffffffff;
        AddressPacket->LBA = (UINT64)LBA;
        AddressPacket->TransferBuffer = TransferBuffer;
    
        Regs.h.AH = 0x42;
        Regs.h.DL = BiosBlkIoDev->Bios.Number;
        Regs.x.SI = _FP_OFF (AddressPacket);
        Regs.x.DS = _FP_SEG (AddressPacket);
    
        CarryFlag = Int86 (0x13, &Regs);
        DEBUG ((D_BLKIO_ULTRA, "EDD30BiosReadBlocks: INT 13 42 DL=%02x : CF=%d AH=%02x\n",BiosBlkIoDev->Bios.Number,CarryFlag,Regs.h.AH));    

        Media->MediaPresent = TRUE;
        if (CarryFlag) {
            //
            // Return Error Status
            //

            BiosBlkIoDev->Bios.ErrorCode = Regs.h.AH;
            if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DISK_CHANGED) {
                Media->MediaId++;
                Bios = &BiosBlkIoDev->Bios;
                if (Int13GetDeviceParameters(Bios)) {
                    if (Int13Extensions (Bios)) {
                        Media->LastBlock = (EFI_LBA)Bios->Parameters.PhysicalSectors - 1;
                        Media->BlockSize = (UINT32)Bios->Parameters.BytesPerSector;
                    } else {
                        //
                        // Legacy Interfaces
                        //
                        Media->LastBlock = (Bios->MaxHead + 1) * Bios->MaxSector * (Bios->MaxCylinder + 1) - 1;
                        Media->BlockSize = 512;
                    }
                    Media->ReadOnly = FALSE;
                    BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
                    BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
                    return EFI_MEDIA_CHANGED;
                }
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DRIVE_TIMEOUT &&
                       Media->RemovableMedia) {
              //
              // Treat this case as NO_MEDIA
              //
              Media->MediaPresent = FALSE;
              BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
              BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
              return EFI_NO_MEDIA;
            }

            if (Media->RemovableMedia) {
                Media->MediaPresent = FALSE;
            }

            return EFI_DEVICE_ERROR;
        } 

        TransferByteSize = NumberOfBlocks * BlockSize;
        BufferSize = BufferSize - TransferByteSize; 
        TransferBuffer += TransferByteSize;
        LBA += NumberOfBlocks;
    }
    
    return EFI_SUCCESS;
}

         
EFI_STATUS
EDD30BiosWriteBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    )
{
    EFI_BLOCK_IO_MEDIA          *Media;
    BIOS_BLK_IO_DEV             *BiosBlkIoDev;  
    EDD_DEVICE_ADDRESS_PACKET   *AddressPacket; // I exist only for readability
    IA32_RegisterSet_t          Regs;
    UINT64                      TransferBuffer;
    UINTN                       NumberOfBlocks;
    UINTN                       TransferByteSize;
    UINTN                       BlockSize;    
    BIOS_LEGACY_DRIVE           *Bios;
    UINTN                       CarryFlag;
    UINTN                       MaxTransferBlocks;
    EFI_BLOCK_IO *BlkIo;

    Media = This->Media;
    BlockSize = Media->BlockSize;

    //
    //Check buffer alignment
    //
    if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
      return EFI_INVALID_PARAMETER;
    }

    if (MediaId != Media->MediaId) {    
        return EFI_MEDIA_CHANGED;
    }
    
    if (LBA > Media->LastBlock) {
        return EFI_DEVICE_ERROR;
    }

    if ((LBA + (BufferSize / BlockSize) - 1) > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize % BlockSize != 0) {
        return EFI_BAD_BUFFER_SIZE;
    }

    if (Buffer == NULL) {    
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0 ) {    
        return EFI_SUCCESS;
    }

    BiosBlkIoDev = BIOS_BLK_IO_FROM_THIS (This);
    AddressPacket = GlobalEDDBufferUnder1MB; 
    
    MaxTransferBlocks = MAX_EDD11_XFER/BlockSize;

    TransferBuffer = (UINT64)Buffer;
    for (;BufferSize > 0;) {
        NumberOfBlocks = BufferSize/BlockSize;
        NumberOfBlocks = NumberOfBlocks > MaxTransferBlocks ? MaxTransferBlocks : NumberOfBlocks; // Max transfer MaxTransferBlocks
        AddressPacket->PacketSizeInBytes = sizeof(EDD_DEVICE_ADDRESS_PACKET); 
        AddressPacket->Zero = 0;
        AddressPacket->NumberOfBlocks = (UINT8)NumberOfBlocks;
        AddressPacket->Zero2 = 0; 
        AddressPacket->SegOffset = 0xffffffff;
        AddressPacket->LBA = (UINT64)LBA;
        AddressPacket->TransferBuffer = TransferBuffer;
    
        Regs.h.AH = 0x43;
        Regs.h.AL = 0x00; // Write Verify Off
        Regs.h.DL = BiosBlkIoDev->Bios.Number;
        Regs.x.SI = _FP_OFF (AddressPacket);
        Regs.x.DS = _FP_SEG (AddressPacket);
    
        CarryFlag = Int86 (0x13, &Regs);
        DEBUG ((D_BLKIO_ULTRA, "EDD30BiosWriteBlocks: INT 13 43 DL=%02x : CF=%d AH=%02x\n",BiosBlkIoDev->Bios.Number,CarryFlag,Regs.h.AH));    

        Media->MediaPresent = TRUE;
        if (CarryFlag) {
            //
            // Return Error Status
            //

            BiosBlkIoDev->Bios.ErrorCode = Regs.h.AH;
            if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DISK_CHANGED) {
                Media->MediaId++;
                Bios = &BiosBlkIoDev->Bios;
                if (Int13GetDeviceParameters(Bios)) {
                    if (Int13Extensions (Bios)) {
                        Media->LastBlock = (EFI_LBA)Bios->Parameters.PhysicalSectors - 1;
                        Media->BlockSize = (UINT32)Bios->Parameters.BytesPerSector;
                    } else {
                        //
                        // Legacy Interfaces
                        //
                        Media->LastBlock = (Bios->MaxHead + 1) * Bios->MaxSector * (Bios->MaxCylinder + 1) - 1;
                        Media->BlockSize = 512;
                    }
                    Media->ReadOnly = FALSE;
                    BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
                    BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
                    return EFI_MEDIA_CHANGED;
                }
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_WRITE_PROTECTED) {
                Media->ReadOnly = TRUE;
                return EFI_WRITE_PROTECTED;
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DRIVE_TIMEOUT &&
                       Media->RemovableMedia) {
              //
              // Treat this case as NO_MEDIA
              //
              Media->MediaPresent = FALSE;
              BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
              BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
              return EFI_NO_MEDIA;
            }

            if (Media->RemovableMedia) {
                Media->MediaPresent = FALSE;
            }

            return EFI_DEVICE_ERROR;
        } 

        Media->ReadOnly = FALSE;
        TransferByteSize = NumberOfBlocks * BlockSize;
        BufferSize = BufferSize - TransferByteSize; 
        TransferBuffer += TransferByteSize;
        LBA += NumberOfBlocks;
    }
    
    return EFI_SUCCESS;
}


EFI_STATUS 
BiosBlkIoFlushBlocks (
    IN  EFI_BLOCK_IO  *This
    )
{
    return EFI_SUCCESS;
}               

EFI_STATUS 
BiosBlkIoReset (
    IN  EFI_BLOCK_IO        *This,
    IN  BOOLEAN             ExtendedVerification
    )
{
    BIOS_BLK_IO_DEV     *BiosBlkIoDev;
    IA32_RegisterSet_t  Regs;
    UINTN               CarryFlag;
    
    BiosBlkIoDev = BIOS_BLK_IO_FROM_THIS (This);

    Regs.h.AH = 0x00;
    Regs.h.DL = BiosBlkIoDev->Bios.Number; 
    CarryFlag = Int86 (0x13, &Regs);
    DEBUG ((D_INIT, "BiosBlkIoReset: INT 13 00 DL=%02x : CF=%d AH=%02x\n",BiosBlkIoDev->Bios.Number,CarryFlag,Regs.h.AH));    
    if (CarryFlag) {
        if (Regs.h.AL == BIOS_RESET_FAILED) {
            Regs.h.AH = 0x00;
            Regs.h.DL = BiosBlkIoDev->Bios.Number; 
            CarryFlag = Int86 (0x13, &Regs);
            DEBUG ((D_INIT, "BiosBlkIoReset: INT 13 00 DL=%02x : CF=%d AH=%02x\n",BiosBlkIoDev->Bios.Number,CarryFlag,Regs.h.AH));    
            if (CarryFlag) {
                BiosBlkIoDev->Bios.ErrorCode = Regs.h.AH;
                return EFI_DEVICE_ERROR;
            }
        }
    }
    return EFI_SUCCESS;
}

//
//
// These functions need to double buffer all data under 1MB!
//
//

EFI_STATUS
EDD11BiosReadBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    )
{
    EFI_BLOCK_IO_MEDIA          *Media;
    BIOS_BLK_IO_DEV             *BiosBlkIoDev;  
    EDD_DEVICE_ADDRESS_PACKET   *AddressPacket; // I exist only for readability
    IA32_RegisterSet_t          Regs;
    UINT64                      TransferBuffer;
    UINTN                       NumberOfBlocks;
    UINTN                       TransferByteSize;
    UINTN                       BlockSize;
    BIOS_LEGACY_DRIVE           *Bios;
    UINTN                       CarryFlag;
    UINTN                       MaxTransferBlocks;
    EFI_BLOCK_IO *BlkIo;

    Media = This->Media;
    BlockSize = Media->BlockSize;

    //
    //Check buffer alignment
    //
    if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
      return EFI_INVALID_PARAMETER;
    }
    

    if (MediaId != Media->MediaId) {    
        return EFI_MEDIA_CHANGED;
    }

    if (LBA > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if ((LBA + (BufferSize / BlockSize) - 1) > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }
    
    if (BufferSize % BlockSize != 0) {
        return EFI_BAD_BUFFER_SIZE;
    }

    if (Buffer == NULL) {    
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0 ) {    
        return EFI_SUCCESS;
    }

    BiosBlkIoDev = BIOS_BLK_IO_FROM_THIS (This);
    AddressPacket = GlobalEDDBufferUnder1MB; 

    MaxTransferBlocks = MAX_EDD11_XFER/BlockSize;

    TransferBuffer = (UINT64)GlobalEDD11Buffer;
    for (;BufferSize > 0;) {
        NumberOfBlocks = BufferSize / BlockSize;
        NumberOfBlocks = NumberOfBlocks > MaxTransferBlocks ? MaxTransferBlocks : NumberOfBlocks; // Max transfer MaxTransferBlocks
        AddressPacket->PacketSizeInBytes = sizeof(EDD_DEVICE_ADDRESS_PACKET); 
        AddressPacket->Zero = 0;
        AddressPacket->NumberOfBlocks = (UINT8)NumberOfBlocks;
        AddressPacket->Zero2 = 0; 
        AddressPacket->SegOffset = _FP_SEG (TransferBuffer) << 16;
        AddressPacket->SegOffset |= _FP_OFF (TransferBuffer);
        AddressPacket->LBA = (UINT64)LBA;
        
        Regs.h.AH = 0x42;
        Regs.h.DL = BiosBlkIoDev->Bios.Number;
        Regs.x.SI = _FP_OFF (AddressPacket);
        Regs.x.DS = _FP_SEG (AddressPacket);

        CarryFlag = Int86 (0x13, &Regs);
        DEBUG ((D_BLKIO_ULTRA, "EDD11BiosReadBlocks: INT 13 42 DL=%02x : CF=%d AH=%02x : LBA 0x%lx  Block(s) %0d \n",BiosBlkIoDev->Bios.Number, CarryFlag, Regs.h.AH, LBA, NumberOfBlocks));    
        Media->MediaPresent = TRUE;
        if (CarryFlag) {
            //
            // Return Error Status
            //

            BiosBlkIoDev->Bios.ErrorCode = Regs.h.AH;
            if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DISK_CHANGED) {
                Media->MediaId++;
                Bios = &BiosBlkIoDev->Bios;
                if (Int13GetDeviceParameters(Bios)) {
                    if (Int13Extensions (Bios)) {
                        Media->LastBlock = (EFI_LBA)Bios->Parameters.PhysicalSectors - 1;
                        Media->BlockSize = (UINT32)Bios->Parameters.BytesPerSector;
                    } else {
                        //
                        // Legacy Interfaces
                        //
                        Media->LastBlock = (Bios->MaxHead + 1) * Bios->MaxSector * (Bios->MaxCylinder + 1) - 1;
                        Media->BlockSize = 512;
                    }
                    Media->ReadOnly = FALSE;
                    BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
                    BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
                    return EFI_MEDIA_CHANGED;
                }
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DRIVE_TIMEOUT &&
                       Media->RemovableMedia) {
              //
              // Treat this case as NO_MEDIA
              //
              Media->MediaPresent = FALSE;
              BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
              BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
              return EFI_NO_MEDIA;
            }

            if (Media->RemovableMedia) {
                Media->MediaPresent = FALSE;
            }

            return EFI_DEVICE_ERROR;
        } 

        TransferByteSize = NumberOfBlocks * BlockSize;
        CopyMem (Buffer, (VOID *)TransferBuffer, TransferByteSize);
        BufferSize = BufferSize - TransferByteSize;
        Buffer = (VOID *)((UINT8 *)Buffer + TransferByteSize);
        LBA += NumberOfBlocks;
    }
    
    return EFI_SUCCESS;
}

EFI_STATUS
EDD11BiosWriteBlocks (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    )
{
    EFI_BLOCK_IO_MEDIA          *Media;
    BIOS_BLK_IO_DEV             *BiosBlkIoDev;  
    EDD_DEVICE_ADDRESS_PACKET   *AddressPacket; // I exist only for readability
    IA32_RegisterSet_t          Regs;
    UINT64                      TransferBuffer;
    UINTN                       NumberOfBlocks;
    UINTN                       TransferByteSize;
    UINTN                       BlockSize;
    BIOS_LEGACY_DRIVE           *Bios;
    UINTN                       CarryFlag;
    UINTN                       MaxTransferBlocks;
    EFI_BLOCK_IO *BlkIo;

    Media = This->Media;
    BlockSize = Media->BlockSize;

    //
    //Check buffer alignment
    //
    if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
      return EFI_INVALID_PARAMETER;
    }

    if (MediaId != Media->MediaId) {    
        return EFI_MEDIA_CHANGED;
    }

    if (LBA > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if ((LBA + (BufferSize / BlockSize) - 1) > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize % BlockSize != 0) {
        return EFI_BAD_BUFFER_SIZE;
    }

    if (Buffer == NULL) {    
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0 ) {    
        return EFI_SUCCESS;
    }

    BiosBlkIoDev = BIOS_BLK_IO_FROM_THIS (This);
    AddressPacket = GlobalEDDBufferUnder1MB; 
    
    MaxTransferBlocks = MAX_EDD11_XFER/BlockSize;

    TransferBuffer = (UINT64)GlobalEDD11Buffer;
    for (;BufferSize > 0;) {
        NumberOfBlocks = BufferSize/BlockSize;
        NumberOfBlocks = NumberOfBlocks > MaxTransferBlocks ? MaxTransferBlocks : NumberOfBlocks; // Max transfer MaxTransferBlocks
        AddressPacket->PacketSizeInBytes = sizeof(EDD_DEVICE_ADDRESS_PACKET); 
        AddressPacket->Zero = 0;
        AddressPacket->NumberOfBlocks = (UINT8)NumberOfBlocks;
        AddressPacket->Zero2 = 0; 
        AddressPacket->SegOffset = _FP_SEG (TransferBuffer) << 16;
        AddressPacket->SegOffset |= _FP_OFF (TransferBuffer);
        AddressPacket->LBA = (UINT64)LBA;
        
        Regs.h.AH = 0x43;
        Regs.h.AL = 0x00; // Write Verify disable
        Regs.h.DL = BiosBlkIoDev->Bios.Number;
        Regs.x.SI = _FP_OFF (AddressPacket);
        Regs.x.DS = _FP_SEG (AddressPacket);
    
        TransferByteSize = NumberOfBlocks * BlockSize;
        CopyMem ((VOID *)TransferBuffer, Buffer, TransferByteSize);

        CarryFlag = Int86 (0x13, &Regs);
        DEBUG ((D_BLKIO_ULTRA, "EDD11BiosWriteBlocks: INT 13 43 DL=%02x : CF=%d AH=%02x\n: LBA 0x%lx  Block(s) %0d \n",BiosBlkIoDev->Bios.Number, CarryFlag, Regs.h.AH, LBA, NumberOfBlocks));    
        Media->MediaPresent = TRUE;
        if (CarryFlag) {
            //
            // Return Error Status
            //

            BiosBlkIoDev->Bios.ErrorCode = Regs.h.AH;
            if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DISK_CHANGED) {
                Media->MediaId++;
                Bios = &BiosBlkIoDev->Bios;
                if (Int13GetDeviceParameters(Bios)) {
                    if (Int13Extensions (Bios)) {
                        Media->LastBlock = (EFI_LBA)Bios->Parameters.PhysicalSectors - 1;
                        Media->BlockSize = (UINT32)Bios->Parameters.BytesPerSector;
                    } else {
                        //
                        // Legacy Interfaces
                        //
                        Media->LastBlock = (Bios->MaxHead + 1) * Bios->MaxSector * (Bios->MaxCylinder + 1) - 1;
                        Media->BlockSize = 512;
                    }
                    Media->ReadOnly = FALSE;
                    BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
                    BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
                    return EFI_MEDIA_CHANGED;
                }
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_WRITE_PROTECTED) {
                Media->ReadOnly = TRUE;
                return EFI_WRITE_PROTECTED;
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DRIVE_TIMEOUT &&
                       Media->RemovableMedia) {
              //
              // Treat this case as NO_MEDIA
              //
              Media->MediaPresent = FALSE;
              BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
              BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
              return EFI_NO_MEDIA;
            }

            if (Media->RemovableMedia) {
                Media->MediaPresent = FALSE;
            }

            return EFI_DEVICE_ERROR;
        } 

        Media->ReadOnly = FALSE;
        BufferSize = BufferSize - TransferByteSize; 
        Buffer = (VOID *)((UINT8 *)Buffer + TransferByteSize);
        LBA += NumberOfBlocks;
    }
    
    return EFI_SUCCESS;
}


EFI_STATUS
BiosReadLegacyDrive (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    )
{
    EFI_BLOCK_IO_MEDIA      *Media;   
    BIOS_BLK_IO_DEV         *BiosBlkIoDev;  
    IA32_RegisterSet_t      Regs;
    UINTN                   UpperCylinder, Temp;
    UINTN                   Cylinder, Head, Sector;
    UINTN                   NumberOfBlocks, TransferByteSize;
    UINTN                   ShortLba, CheckLba;
    UINTN                   BlockSize;
    BIOS_LEGACY_DRIVE       *Bios;
    UINTN                   CarryFlag;
    UINTN                   Retry;
    EFI_BLOCK_IO *BlkIo;

    
    Media = This->Media;
    BlockSize = Media->BlockSize;

    //
    //Check buffer alignment
    //
    if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
      return EFI_INVALID_PARAMETER;
    }
    
    if (MediaId != Media->MediaId) {    
        return EFI_MEDIA_CHANGED;
    }

    if (LBA > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if ((LBA + (BufferSize / BlockSize) - 1) > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize % BlockSize != 0) {
        return EFI_BAD_BUFFER_SIZE;
    }
    
    if (Buffer == NULL) {    
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0 ) {    
        return EFI_SUCCESS;
    }

    BiosBlkIoDev = BIOS_BLK_IO_FROM_THIS (This);
    ShortLba = (UINTN) LBA;

    while (BufferSize) {
        //
        // Compute I/O location in Sector, Head, Cylinder format
        //

        Sector = (ShortLba % BiosBlkIoDev->Bios.MaxSector) + 1;
        Temp = ShortLba / BiosBlkIoDev->Bios.MaxSector;
        Head = Temp % (BiosBlkIoDev->Bios.MaxHead + 1);
        Cylinder = Temp / (BiosBlkIoDev->Bios.MaxHead + 1);

        //
        // Limit transfer to this Head & Cylinder
        //

        NumberOfBlocks = BufferSize/BlockSize;
        Temp = BiosBlkIoDev->Bios.MaxSector - Sector + 1;
        NumberOfBlocks = NumberOfBlocks > Temp ? Temp : NumberOfBlocks;

        Retry = 3;
        do {
          //
          // Perform the IO
          //

          Regs.h.AH = 2;
          Regs.h.AL = (UINT8) NumberOfBlocks;
          Regs.h.DL = BiosBlkIoDev->Bios.Number;

          UpperCylinder = (Cylinder & 0x0f00) >> 2;

          CheckLba = Cylinder*(BiosBlkIoDev->Bios.MaxHead + 1) + Head;
          CheckLba = CheckLba*BiosBlkIoDev->Bios.MaxSector +  Sector - 1;

          DEBUG ((D_BLKIO_ULTRA, "RLD: LBA %x (%x), Sector %x (%x), Head %x (%x), Cyl %x, UCyl %x\n",
                      ShortLba,           CheckLba,
                      Sector,             BiosBlkIoDev->Bios.MaxSector,
                      Head,               BiosBlkIoDev->Bios.MaxHead,
                      Cylinder,
                      UpperCylinder
                      ));
          ASSERT(CheckLba == ShortLba);

          Regs.h.CL = (UINT8) ((Sector & 0x3f) + (UpperCylinder & 0xff));  
          Regs.h.DH = (UINT8) (Head & 0x3f);
          Regs.h.CH = (UINT8) (Cylinder & 0xff);

          Regs.x.BX = _FP_OFF (GlobalEDD11Buffer);
          Regs.x.ES = _FP_SEG (GlobalEDD11Buffer);

          DEBUG ((D_BLKIO_ULTRA, "INT 13h: AX:(02%02x) DX:(%02x%02x) CX:(%02x%02x) BX:(%04x) ES:(%04x)\n", 
                      Regs.h.AL,
                      (UINT8) (Head & 0x3f),
                      Regs.h.DL,
                      (UINT8) (Cylinder & 0xff),
                      (UINT8) ((Sector & 0x3f) + (UpperCylinder & 0xff)),
                      _FP_OFF (GlobalEDD11Buffer),
                      _FP_SEG (GlobalEDD11Buffer)
                      ));

          CarryFlag = Int86 (0x13, &Regs);
          DEBUG ((D_BLKIO_ULTRA, "BiosReadLegacyDrive: INT 13 02 DL=%02x : CF=%d AH=%02x\n",BiosBlkIoDev->Bios.Number,CarryFlag,Regs.h.AH));    
          Retry--;
        } while (CarryFlag && Retry !=0 && Regs.h.AH != BIOS_DISK_CHANGED); 

        Media->MediaPresent = TRUE;
        if (CarryFlag) {
            //
            // Return Error Status
            //

            BiosBlkIoDev->Bios.ErrorCode = Regs.h.AH;
            if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DISK_CHANGED) {
                Media->MediaId++;
                Bios = &BiosBlkIoDev->Bios;
                if (Int13GetDeviceParameters(Bios)) {
                    //
                    // If the size of the media changed we need to reset the disk geometry
                    //
                    if (Int13Extensions (Bios)) {
                        Media->LastBlock = (EFI_LBA)Bios->Parameters.PhysicalSectors - 1;
                        Media->BlockSize = (UINT32)Bios->Parameters.BytesPerSector;
                    } else {
                        //
                        // Legacy Interfaces
                        //
                        Media->LastBlock = (Bios->MaxHead + 1) * Bios->MaxSector * (Bios->MaxCylinder + 1) - 1;
                        Media->BlockSize = 512;
                    }
                    Media->ReadOnly = FALSE;
                    BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
                    BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
                    return EFI_MEDIA_CHANGED;
                }
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DRIVE_TIMEOUT &&
                       Media->RemovableMedia) {
              //
              // Treat this case as NO_MEDIA
              //
              Media->MediaPresent = FALSE;
              BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
              BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
              return EFI_NO_MEDIA;
            }

            if (Media->RemovableMedia) {
                Media->MediaPresent = FALSE;
            }

            return EFI_DEVICE_ERROR;
        }

        TransferByteSize = NumberOfBlocks * BlockSize;
        CopyMem (Buffer, GlobalEDD11Buffer, TransferByteSize);

        ShortLba = ShortLba + NumberOfBlocks;
        BufferSize = BufferSize - TransferByteSize; 
        Buffer = (VOID *)((UINT8 *)Buffer + TransferByteSize);
    }

    return EFI_SUCCESS;
}
          

EFI_STATUS
BiosWriteLegacyDrive (
    IN  EFI_BLOCK_IO        *This,
    IN  UINT32              MediaId,
    IN  EFI_LBA             LBA,
    IN  UINTN               BufferSize,
    OUT VOID                *Buffer
    )
{
    EFI_BLOCK_IO_MEDIA      *Media;
    BIOS_BLK_IO_DEV         *BiosBlkIoDev;  
    IA32_RegisterSet_t      Regs;
    UINTN                   UpperCylinder, Temp;
    UINTN                   Cylinder, Head, Sector;
    UINTN                   NumberOfBlocks, TransferByteSize;
    UINTN                   ShortLba, CheckLba;
    UINTN                   BlockSize;
    BIOS_LEGACY_DRIVE       *Bios;
    UINTN                   CarryFlag;
    UINTN                   Retry;
    EFI_BLOCK_IO *BlkIo;

    
    Media = This->Media;
    BlockSize = Media->BlockSize;

    //
    //Check buffer alignment
    //
    if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
      return EFI_INVALID_PARAMETER;
    }
    
    if (MediaId != Media->MediaId) {    
        return EFI_MEDIA_CHANGED;
    }

    if (LBA > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if ((LBA + (BufferSize / BlockSize) - 1) > Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize % BlockSize != 0) {
        return EFI_BAD_BUFFER_SIZE;
    }

    if (Buffer == NULL) {    
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0 ) {    
        return EFI_SUCCESS;
    }

    BiosBlkIoDev = BIOS_BLK_IO_FROM_THIS (This);
    ShortLba = (UINTN) LBA;

    while(BufferSize) {
        
        //
        // Compute I/O location in Sector, Head, Cylinder format
        //

        Sector = (ShortLba % BiosBlkIoDev->Bios.MaxSector) + 1;
        Temp = ShortLba / BiosBlkIoDev->Bios.MaxSector;
        Head = Temp % (BiosBlkIoDev->Bios.MaxHead + 1);
        Cylinder = Temp / (BiosBlkIoDev->Bios.MaxHead + 1);

        //
        // Limit transfer to this Head & Cylinder
        //

        NumberOfBlocks = BufferSize/BlockSize;
        Temp = BiosBlkIoDev->Bios.MaxSector - Sector + 1;
        NumberOfBlocks = NumberOfBlocks > Temp ? Temp : NumberOfBlocks;

        Retry = 3;
        do {
        
          //
          // Perform the IO
          //

          Regs.h.AH = 3;
          Regs.h.AL = (UINT8) NumberOfBlocks;
          Regs.h.DL = BiosBlkIoDev->Bios.Number;

          UpperCylinder = (Cylinder & 0x0f00) >> 2;

          CheckLba = Cylinder*(BiosBlkIoDev->Bios.MaxHead + 1) + Head;
          CheckLba = CheckLba*BiosBlkIoDev->Bios.MaxSector +  Sector - 1;

          DEBUG ((D_BLKIO_ULTRA, "RLD: LBA %x (%x), Sector %x (%x), Head %x (%x), Cyl %x, UCyl %x\n",
                      ShortLba,           CheckLba,
                      Sector,             BiosBlkIoDev->Bios.MaxSector,
                      Head,               BiosBlkIoDev->Bios.MaxHead,
                      Cylinder,
                      UpperCylinder
                      ));
          ASSERT(CheckLba == ShortLba);

          Regs.h.CL = (UINT8) ((Sector & 0x3f) + (UpperCylinder & 0xff));
          Regs.h.DH = (UINT8) (Head & 0x3f);
          Regs.h.CH = (UINT8) (Cylinder & 0xff);

          Regs.x.BX = _FP_OFF (GlobalEDD11Buffer);
          Regs.x.ES = _FP_SEG (GlobalEDD11Buffer);

          TransferByteSize = NumberOfBlocks * BlockSize;
          CopyMem (GlobalEDD11Buffer, Buffer, TransferByteSize);

          DEBUG ((D_BLKIO_ULTRA, "INT 13h: AX:(03%02x) DX:(%02x%02x) CX:(%02x%02x) BX:(%04x) ES:(%04x)\n", 
                      Regs.h.AL,
                      (UINT8) (Head & 0x3f),
                      Regs.h.DL,
                      (UINT8) (Cylinder & 0xff),
                      (UINT8) ((Sector & 0x3f) + (UpperCylinder & 0xff)),
                      _FP_OFF (GlobalEDD11Buffer),
                      _FP_SEG (GlobalEDD11Buffer)
                      ));

          CarryFlag = Int86 (0x13, &Regs);
          DEBUG ((D_BLKIO_ULTRA, "BiosWriteLegacyDrive: INT 13 03 DL=%02x : CF=%d AH=%02x\n",BiosBlkIoDev->Bios.Number,CarryFlag,Regs.h.AH));    
          Retry--;
        } while (CarryFlag && Retry !=0 && Regs.h.AH != BIOS_DISK_CHANGED); 

        Media->MediaPresent = TRUE;
        if (CarryFlag) {
            //
            // Return Error Status
            //

            BiosBlkIoDev->Bios.ErrorCode = Regs.h.AH;
            if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DISK_CHANGED) {
                Media->MediaId++;
                Bios = &BiosBlkIoDev->Bios;
                if (Int13GetDeviceParameters(Bios)) {
                    if (Int13Extensions (Bios)) {
                        Media->LastBlock = (EFI_LBA)Bios->Parameters.PhysicalSectors - 1;
                        Media->BlockSize = (UINT32)Bios->Parameters.BytesPerSector;
                    } else {
                        //
                        // Legacy Interfaces
                        //
                        Media->LastBlock = (Bios->MaxHead + 1) * Bios->MaxSector * (Bios->MaxCylinder + 1) - 1;
                        Media->BlockSize = 512;
                     }
                    //
                    // If the size of the media changed we need to reset the disk geometry
                    //
                    Media->ReadOnly = FALSE;
                    BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
                    BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
                    return EFI_MEDIA_CHANGED;
                }
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_WRITE_PROTECTED) {
                Media->ReadOnly = TRUE;
                return EFI_WRITE_PROTECTED;
            } else if (BiosBlkIoDev->Bios.ErrorCode == BIOS_DRIVE_TIMEOUT &&
                       Media->RemovableMedia) {
              //
              // Treat this case as NO_MEDIA
              //
              Media->MediaPresent = FALSE;
              BS->HandleProtocol (BiosBlkIoDev->Handle, &BlockIoProtocol, (VOID **)&BlkIo);
              BS->ReinstallProtocolInterface (BiosBlkIoDev->Handle, &BlockIoProtocol, BlkIo, BlkIo);
              return EFI_NO_MEDIA;
            }

            if (Media->RemovableMedia) {
                Media->MediaPresent = FALSE;
            }

            return EFI_DEVICE_ERROR;
        }
        Media->ReadOnly = FALSE;
        ShortLba = ShortLba + NumberOfBlocks;
        BufferSize = BufferSize - TransferByteSize; 
        Buffer = (VOID *)((UINT8 *)Buffer + TransferByteSize);
    }

    return EFI_SUCCESS;
}
          
