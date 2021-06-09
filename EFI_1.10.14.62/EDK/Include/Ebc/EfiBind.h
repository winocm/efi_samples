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

  EfiBind.h

Abstract:

  Processor or compiler specific defines and types for EBC.

--*/

#ifndef _EFI_BIND_H_
#define _EFI_BIND_H_

#define EFI_DRIVER_ENTRY_POINT(InitFunction)
#define EFI_APPLICATION_ENTRY_POINT EFI_DRIVER_ENTRY_POINT

//
// Native integer types
//
typedef char                  int8_t;
typedef unsigned char         uint8_t;

typedef short                 int16_t;
typedef unsigned short        uint16_t;

typedef int                   int32_t;
typedef unsigned int          uint32_t;

typedef __int64               int64_t;
typedef unsigned __int64      uint64_t;

//
// "long" type scales to the processor native size with EBC compiler
//
typedef long                  intn_t;
typedef unsigned long         uintn_t;

//
// Scalable macro to set the most significant bit in a natural number
//
#define EFI_MAX_BIT           ((UINTN)0x01 << ((sizeof (char *) * 8) - 1))

//
// Maximum legal EBC address
//
#define EFI_MAX_ADDRESS   (UINTN)~0

//
//  Bad pointer value to use in check builds.
//  if you see this value you are using uninitialized or free'ed data
//
#define EFI_BAD_POINTER          (UINTN)0xAFAFAFAFAFAFAFAF
#define EFI_BAD_POINTER_AS_BYTE  (UINTN)0xAF

//
// _break() is an EBC compiler intrinsic function
//
extern 
uint64_t 
_break (
  unsigned char BreakCode
  );

//
// Macro to inject a break point in the code to assist debugging.
//
#define EFI_BREAKPOINT()  _break ( 3 )
#define EFI_DEADLOOP()    while (TRUE)

//
// Memory Fence forces serialization, and is needed to support out of order
//  memory transactions. The Memory Fence is mainly used to make sure IO
//  transactions complete in a deterministic sequence, and to syncronize locks
//  an other MP code. Currently no memory fencing is required.
//
#define MEMORY_FENCE()

//
// Some compilers don't support the forward reference construct:
//  typedef struct XXXXX. The forward reference is required for 
//  ANSI compatibility.
//
// The following macro provide a workaround for such cases.
//


#ifdef EFI_NO_INTERFACE_DECL
  #define EFI_FORWARD_DECLARATION(x)
  #define EFI_INTERFACE_DECL(x)
#else
  #define EFI_FORWARD_DECLARATION(x) typedef struct _##x x
  #define EFI_INTERFACE_DECL(x) typedef struct x
#endif


#define _EFIAPI       

#endif // ifndef _EFI_BIND_H_

