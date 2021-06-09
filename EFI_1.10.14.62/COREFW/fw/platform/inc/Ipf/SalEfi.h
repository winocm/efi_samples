//
// Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
// This software and associated documentation (if any) is furnished
// under a license and may only be used or copied in accordance
// with the terms of the license. Except as permitted by such
// license, no part of this software or documentation may be
// reproduced, stored in a retrieval system, or transmitted in any
// form or by any means without the express written consent of
// Intel Corporation.
//
//Module Name:
//
//    SalEfi.h
//
//Abstract:
//
//    Contains defines used between EFI and SAL code
//
//      THIS CODE IS ALSO USED IN THE SAL BUILD!!!!!
//
//      DO NOT CHANGE THIS CODE WITH OUT MAKEING SURE THE
//      SAL STILL BUILDS. THE SAL ENVIRNEMNT HAS EXTRA 
//      RESTRICTIONS NOT PLACED ON NORMAL C CODE.
//
//
//Revision History
//
#ifndef _SALEFI_H_
#define _SALEFI_H_

// (1)
// define for calling Bios Int's through SAL callback
// Parameter list while calling SALCallBack:
//  Arg1->Bios Interrupt number using define below
//  Arg2->Pointer to filled in IA32_BIOS_REGISTER_STATE
//  Arg3-Arg8 -> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success
//              -1  -   not supported
//              -2  -   invalid argument
//   rArg.p1 ->  if(rArg.p0 == -2), this field will contain first
//               invalid argument index from input list (1 - based)
#define ID_SALCB_BIOSCALL(a)    (UINT64)(a | 0x8000000000000000)

// (1a)
// define for calling IA-32 real mode 
// Parameter list while calling SALCallBack:
//  Arg1->Pointer to filled in IA32_BIOS_REGISTER_STATE
//  Arg2->Max size of the stack that is supported. Only used for error 
//          checking.
//  Arg3->Stack Size it will be zero if the stack is not being used or
//          the size of the stack. Used to support error checking.
//  Arg4-Arg8 -> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success
//              -1  -   not supported
//              -2  -   invalid argument
//   rArg.p1 ->  if(rArg.p0 == -2), this field will contain first
//               invalid argument index from input list (1 - based)
#define ID_SALCB_REALMODE   ID_SALCB_BIOSCALL(0x100)


// (2)
// define for getting device info through SAL callback
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->SubType for media of interest(CD, LS120 etc..)
//  Arg3->Buffer size of Arg4 parameter
//  Arg4->Buffer ptr that will contain structure(s) corresponding to Subtype
//  Arg5-Arg8 -> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success 
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   Buffer size too small
//   rArg.p1 ->  if(rArg.p0 == 0), this field will contain 
//               number of devices identified
//   rArg.p1 ->  if(rArg.p0 == -2), this field will contain first
//               invalid argument index from input list (1 - based)
//   rArg.p1 -> if (rArg.p0 == -3), this field contains required buf size
#define ID_SALCB_GETMEDIAINFO           0x8000000000001000
// Subtype for GetMediaInfo
#define ID_SALCB_GETMEDIAINFO_CDROM     1
#define ID_SALCB_GETMEDIAINFO_LS120     2

// (3)
// define for booting IA32 OS through SAL callback
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->Buffer size of Arg3 parameter
//  Arg3->Buffer ptr (??Need to decide if it is MBR data or some other record,
//                    may also need drive # as another parameter)
//  Arg4-Arg8 -> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success (callback would return only on error)
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   failure
//   rArg.p1 ->  if(rArg.p0 == -2), this field will contain first
//               invalid argument index from input list (1 - based)
//   rArg.p1 -> if (rArg.p0 == -3), this field contains optional non-zero error code
#define ID_SALCB_BOOTIA32OS             0x8000000000002000

// (4)
// define for accessing NVRAM through SAL callback
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->Operation to be performed on NVRAM (read, clear, update etc.)
//  Arg3->Bank number
//  Arg4->Offset
//  Arg5->Buffer size of Arg6 parameter
//  Arg6->Buffer ptr 
//  Arg7->Ptr to Scratch buffer (64K)
//  Arg8-> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success 
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   failure
//   rArg.p1 ->  if(rArg.p0 == -2), this field will contain first
//               invalid argument index from input list (1 - based)
#define ID_SALCB_NVRAM                  0x8000000000003000
// NVRAM operation
#define ID_SALCB_NVRAM_CLEAR            1 // Arg1,Arg2,Arg3 and Arg7 only valid
#define ID_SALCB_NVRAM_READ             2
#define ID_SALCB_NVRAM_UPDATE           3

//
// BugBug Added functionality
//
#define ID_SALCB_NVRAM_LOCK             4
#define ID_SALCB_NVRAM_UNLOCK           5


// (5)
// define for virtual address related calls through SAL callback
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->Total Memory Map size 
//  Arg3->Size of each memory descriptor within memory map (EFI_MEMORY_DESCRIPTOR)
//  Arg4->Pointer to array of memory descriptor structures 
//  Arg5-Arg8-> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success 
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   failure
//   rArg.p1 -> if(rArg.p0 == 0), this field will contain the
//              new virtual address of SALCallBack function
//   rArg.p1 -> if(rArg.p0 == -2), this field will contain first
//              invalid argument index from input list (1 - based)
//   rArg.p1 -> if (rArg.p0 == -3), this field contains first index
//              into table that failed virtual registration
#define ID_SALCB_VA                  0x8000000000004000
// VA operation
#define ID_SALCB_VA_REGISTER_AND_ACTIVATE         1
///#define ID_SALCB_VA_ACTIVATE         2

// (6)
// define for calling system reset through SAL callback
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->Reset SubType (cold,warm)
//  Arg3->Buffer size of Arg3 parameter
//  Arg4->pointer to string 
//  Arg5-Arg8-> 0
//  Results (On return. Can return only if error encountered)
//   rArg.p0 ->  
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   failure
//   rArg.p1 -> if(rArg.p0 == -2), this field will contain first
//              invalid argument index from input list (1 - based)
#define ID_SALCB_RESET              0x8000000000005000
// Reset operation
#define ID_SALCB_RESET_WARM         1
#define ID_SALCB_RESET_COLD         2


// (7)
// define for hooking timer interrupt through SAL callback
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->Timer sub type(start, cancel, oneshot)
//  Arg3->Timer period in ms
//  Arg4->Pointer to function to callback after timeout
//  Arg5->GP value of EFI module
//  Arg6-Arg8-> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success (implies timer interrupt has been hooked)
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   failure
//   rArg.p1 -> if(rArg.p0 == -2), this field will contain first
//              invalid argument index from input list (1 - based)
#define ID_SALCB_TIMER              0x8000000000006000
//Timer operation
#define ID_SALCB_TIMER_START        1
#define ID_SALCB_TIMER_ONESHOT      2
#define ID_SALCB_TIMER_CANCEL       3 // Arg3,Arg4,Arg5 will also be 0 for this call

// (8)
// define for turning interrupts on or off
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->ENABLE (1) or DISABLE (0)
//  Arg3-Arg8-> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success 
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   failure
//   rArg.p1 -> if(rArg.p0 == -2), this field will contain first
//              invalid argument index from input list (1 - based)
#define ID_SALCB_INTERRUPT_STATE    0x8000000000007000

// (9)
// define for setting proper handoff state 
// Parameter list while calling SALCallBack:
//  Arg1->ID number using define below
//  Arg2->Subtype (on start, after exit)
//  Arg2-Arg8-> 0
//  Results (On return)
//   rArg.p0 ->  0  -   success 
//              -1  -   not supported
//              -2  -   invalid argument
//              -3  -   failure
//   rArg.p1 -> if(rArg.p0 == -2), this field will contain first
//              invalid argument index from input list (1 - based)
#define ID_SALCB_SET_STATE          0x8000000000008000
#define ID_SALCB_STATE_ONSTART      1       // before start of EFI binary
#define ID_SALCB_STATE_AFTEREXIT    2       // after exit of EFI binary




#endif _SALEFI_H_


