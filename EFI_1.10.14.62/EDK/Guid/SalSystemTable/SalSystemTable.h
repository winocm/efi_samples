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
  
    SalSystemTable.h
    
Abstract:

  GUIDs used for SAL system table entries in the in the EFI 1.0 system table.

  SAL System Table contains Itanium-based processor centric information about
  the system.

--*/

#ifndef _SAL_SYSTEM_TABLE_GUID_H_

#define EFI_SAL_SYSTEM_TABLE_GUID \
  { 0xeb9d2d32, 0x2d88, 0x11d3, 0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

extern EFI_GUID gEfiSalSystemTableGuid;

#endif
