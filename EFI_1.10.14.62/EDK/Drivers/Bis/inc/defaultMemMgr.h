
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
  
    defaultMemMgr.h

Abstract:



Revision History

--*/

#ifndef DEFAULTMEMMGR_H
#define DEFAULTMEMMGR_H


EFI_STATUS
BISDMM_Create_Instance(
    IN  UINTN                         heapSize,
    OUT EFIBIS_MEMMGR_INTERFACE     **NewInstance
    );



EFI_STATUS
BISDMM_Destroy_Instance(
    IN EFIBIS_MEMMGR_INTERFACE      *ThisInstance
    );


EFI_STATUS
BISDMM_Getinfo (
    IN EFIBIS_MEMMGR_INTERFACE       *ThisInstance,
    IN OUT UINTN                      InfoBufferSize,
    OUT VOID                        **InfoBuffer
    );



EFI_STATUS
BISDMM_Malloc (
    IN EFIBIS_MEMMGR_INTERFACE       *ThisInstance,
    IN  UINTN                         Size,
    OUT VOID                        **Buffer
    );



EFI_STATUS
BISDMM_Free (
    IN EFIBIS_MEMMGR_INTERFACE      *ThisInstance,
    IN VOID                         *Buffer
    );



EFI_STATUS
BISDMM_Calloc(
    IN EFIBIS_MEMMGR_INTERFACE     *ThisInstance,
    IN UINTN                        Num,
    IN UINTN                        Size,
    OUT VOID                        **Buffer
    );

#endif
