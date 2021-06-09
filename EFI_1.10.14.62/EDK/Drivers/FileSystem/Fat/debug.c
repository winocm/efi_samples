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

  debug.c
  
Abstract:

  Debug functions for fat driver

Revision History

--*/

#include "fat.h"

VOID
FatDumpFatTable (
  IN FAT_VOLUME   *Vol
  )
{
  UINTN           v;
  UINTN           MaxIndex, Index;
  CHAR16          *p;

  MaxIndex = Vol->MaxCluster+2;

  Print (L"Dump of Fat Table, MaxCluster %x\n", MaxIndex);
  for (Index = 0; Index < MaxIndex; Index++) {
    v = FatGetFatEntry(Vol, Index);
    if (v != FAT_CLUSTER_FREE) {
      p = NULL;
      switch (v) {
      case FAT_CLUSTER_RESERVED:  p = L"RESREVED";     break;
      case FAT_CLUSTER_BAD:       p = L"BAD";          break;
      }
      if (FAT_END_OF_FAT_CHAIN(v)) {
        p = L"LAST";
      }

      if (p) {
        Print (L"Entry %x = %s\n", Index, p);
      } else {
        Print (L"Entry %x = %x\n", Index, v);
      }
    }
  }
}
