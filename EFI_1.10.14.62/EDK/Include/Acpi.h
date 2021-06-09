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


Abstract:


--*/
#ifndef _acpi_h_
#define _acpi_h_


// Table header signatures
#define RSDP_SIG "RSD PTR "          // RSDT Pointer signature
#define APIC_SIG "APIC"              // Multiple APIC Description Table
#define DSDT_SIG "DSDT"              // Differentiated Sys Descrip Table
#define SSDT_SIG "SSDT"              // Secondary Sys Descrip Table
#define FACP_SIG "FACP"              // Fixed ACPI Description Table
#define FACS_SIG "FACS"              // Firmware ACPI Control Structure
#define RSDT_SIG "RSDT"              // Root System Description Table


#pragma pack(1)

typedef struct
{                                    // Root System Descriptor Pointer
    UINT8  Signature[8];             // contains "RSD PTR "
    UINT8  Checksum;                 // to make sum of struct == 0
    UINT8  OemId[6];                 // OEM identification
    UINT8  Reserved;                 // reserved - must be zero
    UINT32 PhysicalAddress;          // physical address of RSDT
} ROOT_SYSTEM_DESCRIPTOR_POINTER;

extern ROOT_SYSTEM_DESCRIPTOR_POINTER RSDPTR;

typedef struct
{                                    // ACPI common table header
    UINT8  Signature[4];             // identifies type of table
    UINT32 Length;                   // length of table in bytes, including header
    UINT8  Revision;                 // specification minor version #
    UINT8  Checksum;                 // to make sum of entire table == 0
    UINT8  OemId[6];                 // OEM identification
    UINT8  OemTableId[8];            // OEM table identification
    UINT32 OemRevision;              // OEM revision number
    UINT8  AslCompilerId[4];         // ASL compiler vendor ID
    UINT32 AslCompilerRevision;      // ASL compiler revision number
} ACPI_TABLE_HEADER;

typedef struct
{                                    // Root System Description Table
    ACPI_TABLE_HEADER Header;        // table header
    UINT32 TableOffsetEntry[3];      // array of pointers to other tables' headers
} ROOT_SYSTEM_DESCRIPTION_TABLE;

typedef struct
{                                    // Firmware ACPI Control Structure
    UINT8  Signature[4];             // signature "FACS"
    UINT32 Length;                   // length of structure, in bytes
    UINT32 HardwareSignature;        // hardware configuration signature
    UINT32 FirmwareWakingVector;     // ACPI OS waking vector
    UINT32 GlobalLock;               // Global Lock
    UINT32 S4Bios_f;                 // Bit 0 set indicates S4BIOS support.
                                     // All other bits reserved.
    UINT8  abReserved3[40];          // reserved - must be zero
} FIRMWARE_ACPI_CONTROL_STRUCTURE;

// values of bIntModel
#define DUAL_PIC       0
#define MULTIPLE_APIC  1

// Flags - See table 5-6 in ACPI 1.0b
#define WBINVD          (1 << 0)     // wbinvd instruction works properly
#define WBINVD_FLUSH    (1 << 1)     // wbinvd flushes but does not invalidate
#define PROC_C1         (1 << 2)     // all processors support C1 state
#define P_LVL2_UP       (1 << 3)     // C2 state works on MP system
#define PWR_BUTTON      (1 << 4)     // Power button is handled as a generic feature
#define SLP_BUTTON      (1 << 5)     // Sleep button is handled as a generic feature, or no button present
#define FIX_RTC         (1 << 6)     // RTC wakeup stat not in fixed  register space
#define RTC_S4          (1 << 7)     // RTC wakeup stat not possible from S4
#define TMR_VAL_EXT     (1 << 8)     // tmr_val is 32 bits
#define DCK_CAP         (1 << 9)     // 0 indicates system does not support docking

typedef struct
{                                    // Fixed ACPI Description Table
    ACPI_TABLE_HEADER header;        // table header
    UINT32 FirmwareCtrl;             // Physical addesss of FACS
    UINT32 Dsdt;                     // Physical address of DSDT
    UINT8  IntModel;                 // System Interrupt Model
    UINT8  Reserved1;                // reserved
    UINT16 SciInt;                   // System vector of SCI interrupt
    UINT32 SmiCmd;                   // Port address of SMI command port
    UINT8  AcpiEnable;               // value to write to port smi_cmd to enable ACPI
    UINT8  AcpiDisable;              // value to write to port smi_cmd to disable ACPI
    UINT8  S4BiosReq;                // Value to write to SMI CMD port to enter the S4BIOS state
    UINT8  Reserved2;                // reserved - must be zero
    UINT32 Pm1aEvtBlk;               // Port address of Power Mgt 1a Event Reg Blk
    UINT32 Pm1bEvtBlk;               // Port address of Power Mgt 1b Event Reg Blk
    UINT32 Pm1aCntBlk;               // Port address of Power Mgt 1a Control Reg Blk
    UINT32 Pm1bCntBlk;               // Port address of Power Mgt 1b Control Reg Blk
    UINT32 Pm2CntBlk;                // Port address of Power Mgt 2 Control Reg Blk
    UINT32 PmTmrBlk;                 // Port address of Power Mgt Timer Ctrl Reg Blk
    UINT32 Gpe0Blk;                  // Port addr of General Purpose Event 0 Reg Blk
    UINT32 Gpe1Blk;                  // Port addr of General Purpose Event 1 Reg Blk
    UINT8  Pm1EvtLen;                // Byte Length of ports at pm1X_evt_blk
    UINT8  Pm1CntLen;                // Byte Length of ports at pm1X_cnt_blk
    UINT8  Pm2CntLen;                // Byte Length of ports at pm2_cnt_blk
    UINT8  PmTmLen;                  // Byte Length of ports at pm_tm_blk
    UINT8  Gpe0BlkLen;               // Byte Length of ports at gpe0_blk
    UINT8  Gpe1BlkLen;               // Byte Length of ports at gpe1_blk
    UINT8  Gpe1Base;                 // offset in gpe model where gpe1 events start
    UINT8  Reserved3;                // reserved
    UINT16 PLvl2Lat;                 // worst case HW latency to enter/exit C2 state
    UINT16 PLvl3Lat;                 // worst case HW latency to enter/exit C3 state
    UINT16 FlushSize;                // Size of area read to flush caches
    UINT16 FlushStride;              // Stride used in flushing caches
    UINT8  DutyOffset;               // bit location of duty cycle field in p_cnt reg
    UINT8  DutyWidth;                // bit width of duty cycle field in p_cnt reg
    UINT8  DayAlrm;                  // index to day-of-month alarm in RTC CMOS RAM
    UINT8  MonAlrm;                  // index to month-of-year alarm in RTC CMOS RAM
    UINT8  Century;                  // index to century in RTC CMOS RAM
    UINT8  Reserved4;                // reserved
    UINT8  Reserved4a;               // reserved
    UINT8  Reserved4b;               // reserved
    UINT32 Flags;
}  FIXED_ACPI_DESCRIPTION_TABLE;

typedef struct
{                                    // APIC Table
    ACPI_TABLE_HEADER Header;        // table header
    UINT32 LocalApicAddress;         // Physical address for accessing local APICs
    UINT32 Flags;                    // a one in bit 0 indicates system also has dual 8259s
                                     // All other bits reserved.
} APIC_TABLE;

// values of bType in APIC_HEADER
#define APIC_PROC       0
#define APIC_IO         1
#define INTSRC_OVERRIDE 2
#define NMISRC_OVERRIDE 3

// values for APIC interrupt source override flags
#define ACTIVE_HIGH     0x01    // 0001y
#define ACTIVE_LOW      0x03    // 0011y
#define EDGE_TRIGGERED  0x04    // 0100y
#define LEVEL_TRIGGERED 0x0c    // 1100y

typedef struct
{
    UINT8 Type;                     // APIC type.  Either APIC_PROC or APIC_IO
    UINT8 Length;                   // Length of APIC structure
} APIC_HEADER;

typedef struct
{
    APIC_HEADER Header;
    UINT8 ProcessorAcpiId;       // ACPI processor id
    UINT8 LocalApicId;           // processor's local APIC id
    UINT32 ProcessorEnabled;     // Processor is usable if bit 0 set
                                 // all other bits reserved.
} PROCESSOR_APIC;

typedef struct
{
    APIC_HEADER Header;
    UINT8  IOApicId;                // io APIC id
    UINT8  Reserved;                // reserved - must be zero
    UINT32 IOApicAddress;           // APIC's physical address
    UINT32 Vector;                  // interrupt vector index where INTI lines start
} IO_APIC;

typedef struct
{
    APIC_HEADER Header;
    UINT8  Bus;                     // Constant=0, meaning ISA
    UINT8  Source;                  // Bus-Relative IRQ
    UINT32 GlobalIntVector;         // Int Vector that this bus-relative IRQ source will trigger
    UINT16 Flags;                   // 1:0 = HIGH LOW, 3:2 = EDGE LEVEL.  all other bits reserved
} INT_SOURCE_OVERRIDE;


//
// Define for Pci Host Bridge Resource Allocation
// 
#define  ACPI_ADDRESS_SPACE_DESCRIPTOR   0x8A
#define  ACPI_END_TAG_DESCRIPTOR         0x79

#define  ACPI_ADDRESS_SPACE_TYPE_MEM        0x00
#define  ACPI_ADDRESS_SPACE_TYPE_IO         0x01
#define  ACPI_ADDRESS_SPACE_TYPE_BUS        0x02
 
typedef struct  {
  UINT8     Desc;
  UINT16    Len;
  UINT8     ResType;
  UINT8     GenFlag;
  UINT8     SpecificFlag;
  UINT64    AddrSpaceGranularity;
  UINT64    AddrRangeMin;
  UINT64    AddrRangeMax;
  UINT64    AddrTranslationOffset;
  UINT64    AddrLen;
} EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR;


typedef struct {
  UINT8   Desc;
  UINT8   Checksum;
} EFI_ACPI_END_TAG_DESCRIPTOR;

#pragma pack()


#endif 
