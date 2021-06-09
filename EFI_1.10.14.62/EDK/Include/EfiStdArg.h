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

  EfiStdArg.h

Abstract:

  Support for variable length argument lists using the ANSI standard.
  
  Since we are using the ANSI standard we used the standard nameing and
  did not folow the coding convention

  VA_LIST  - typedef for argument list.
  VA_START (VA_LIST Marker, argument before the ...) - Init Marker for use.
  VA_END (VA_LIST Marker) - Clear Marker
  VA_ARG (VA_LIST Marker, var arg size) - Use Marker to get an argumnet from
    the ... list. You must know the size and pass it in this macro.

  example:

  UINTN
  ExampleVarArg (
    IN UINTN  NumberOfArgs,
    ...
    )
  {
    VA_LIST Marker;
    UINTN   Index;
    UINTN   Result;

    //
    // Initialize the Marker
    //
    VA_START (Marker, NumberOfArgs);
    for (Index = 0, Result = 0; Index < NumberOfArgs; Index++) {
      //
      // The ... list is a series of UINTN values, so average them up.
      //
      Result += VA_ARG (Marker, UINTN);
    }

    VA_END (Marker);
    return Result
  }

--*/

#ifndef _EFISTDARG_H_
#define _EFISTDARG_H_

#define _EFI_INT_SIZE_OF(n)  \
  ( (sizeof(n) + sizeof(UINTN) - 1) & ~(sizeof(UINTN) - 1) )


//
// Also support coding convention rules for var arg macros
//
#ifndef VA_START

  typedef CHAR8   *VA_LIST;
  #define VA_START(ap,v)  ( ap = (VA_LIST)&(v) + _EFI_INT_SIZE_OF(v) )
  #define VA_ARG(ap,t)    ( *(t *)((ap += _EFI_INT_SIZE_OF(t)) - _EFI_INT_SIZE_OF(t)) )
  #define VA_END(ap)      ( ap = (VA_LIST)0 )

#endif

#endif
