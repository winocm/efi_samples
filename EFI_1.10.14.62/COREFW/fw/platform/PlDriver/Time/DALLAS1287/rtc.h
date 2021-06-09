#ifndef _RTC_H
#define _RTC_H
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

    rtc.h

Abstract:

    Include for real time clock driver

Revision History

--*/

//
// PC AT RTC I/O Addresses
//

#define PCAT_RTC_ADDRESS_REGISTER 0x70
#define PCAT_RTC_DATA_REGISTER    0x71

//
// Dallas DS12C887 Real Time Clock
//

#define RTC_ADDRESS_SECONDS          0   // R/W  Range 0..59
#define RTC_ADDRESS_SECONDS_ALARM    1   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES          2   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES_ALARM    3   // R/W  Range 0..59
#define RTC_ADDRESS_HOURS            4   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_HOURS_ALARM      5   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_DAY_OF_THE_WEEK  6   // R/W  Range 1..7
#define RTC_ADDRESS_DAY_OF_THE_MONTH 7   // R/W  Range 1..31
#define RTC_ADDRESS_MONTH            8   // R/W  Range 1..12
#define RTC_ADDRESS_YEAR             9   // R/W  Range 0..99
#define RTC_ADDRESS_REGISTER_A       10  // R/W[0..6]  R0[7]
#define RTC_ADDRESS_REGISTER_B       11  // R/W
#define RTC_ADDRESS_REGISTER_C       12  // RO
#define RTC_ADDRESS_REGISTER_D       13  // RO
#define RTC_ADDRESS_CENTURY          50  // R/W  Range 19..20 Bit 8 is R/W

#pragma pack(1)

//
// Register A
//

typedef struct {
    UINT8 RS:4;         // Rate Selection Bits
    UINT8 DV:3;         // Divisor
    UINT8 UIP:1;        // Update in progress
} RTC_REGISTER_A_BITS;

typedef union {
    RTC_REGISTER_A_BITS  Bits;
    UINT8                Data;
} RTC_REGISTER_A;

//
// Register B
//

typedef struct {
    UINT8 DSE:1;        // 0 - Daylight saving disabled  1 - Daylight savings enabled
    UINT8 MIL:1;        // 0 - 12 hour mode              1 - 24 hour mode
    UINT8 DM:1;         // 0 - BCD Format                1 - Binary Format
    UINT8 SQWE:1;       // 0 - Disable SQWE output       1 - Enable SQWE output
    UINT8 UIE:1;        // 0 - Update INT disabled       1 - Update INT enabled
    UINT8 AIE:1;        // 0 - Alarm INT disabled        1 - Alarm INT Enabled
    UINT8 PIE:1;        // 0 - Periodic INT disabled     1 - Periodic INT Enabled
    UINT8 SET:1;        // 0 - Normal operation.         1 - Updates inhibited
} RTC_REGISTER_B_BITS;

typedef union {
    RTC_REGISTER_B_BITS  Bits;
    UINT8                Data;
} RTC_REGISTER_B;

//
// Register C
//

typedef struct {
    UINT8 Reserved:4;   // Read as zero.  Can not be written.
    UINT8 UF:1;         // Update End Interrupt Flag
    UINT8 AF:1;         // Alarm Interrupt Flag
    UINT8 PF:1;         // Periodic Interrupt Flag
    UINT8 IRQF:1;       // Iterrupt Request Flag = PF & PIE | AF & AIE | UF & UIE
} RTC_REGISTER_C_BITS;

typedef union {
    RTC_REGISTER_C_BITS  Bits;
    UINT8                Data;
} RTC_REGISTER_C;

//
// Register D
//

typedef struct {
    UINT8 Reserved:7;   // Read as zero.  Can not be written.
    UINT8 VRT:1;        // Valid RAM and Time
} RTC_REGISTER_D_BITS;

typedef union {
    RTC_REGISTER_D_BITS  Bits;
    UINT8                Data;
} RTC_REGISTER_D;

#pragma pack()

#endif