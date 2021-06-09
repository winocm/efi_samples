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
  
    GlobalVariable.h
    
Abstract:

  GUID for EFI (NVRAM) Variables. Defined in EFI 1.0.

--*/

#ifndef _GLOBAL_VARIABLE_GUID_H_
#define _GLOBAL_VARIABLE_GUID_H_

#define EFI_GLOBAL_VARIABLE_GUID  \
  { 0x8BE4DF61, 0x93CA, 0x11d2, 0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C }

extern EFI_GUID gEfiGlobalVariableGuid;

#endif
