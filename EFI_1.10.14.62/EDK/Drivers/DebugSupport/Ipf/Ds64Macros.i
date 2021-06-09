//++
// Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
// This software and associated documentation (if any) is furnished
// under a license and may only be used or copied in accordance
// with the terms of the license. Except as permitted by such
// license, no part of this software or documentation may be
// reproduced, stored in a retrieval system, or transmitted in any
// form or by any means without the express written consent of
// Intel Corporation.
//
//
// Module Name:
//
//  Ds64Macros.i
//
// Abstract:
//
//  This is set of macros used in calculating offsets in the IVT
//
// Revision History:
//
//--

#define EXCPT_EXTERNAL_INTERRUPT 12
#define MASK_0_4        0x000000000000001F  // mask bits 0 through 4
#define SLOT0           0
#define SLOT1           1
#define SLOT2           2

#define PSR_DT          17
#define PSR_TB          26
#define PSR_RT          27
#define PSR_IS          34
#define PSR_IT          36
#define PSR_IC          13
#define PSR_I           14
#define PSR_SS          40
#define PSR_BN          44
#define PSR_RI_MASK     0x60000000000

#define EXCPT_EXTERNAL_INTERRUPT 12

#define SCRATCH_REG0    r23
#define SCRATCH_REG1    r24
#define SCRATCH_REG2    r25
#define SCRATCH_REG3    r26
#define SCRATCH_REG4    r27
#define SCRATCH_REG5    r28
#define SCRATCH_REG6    r29
#define PR_REG          r30
#define B0_REG          r31


// EXT_INT_OFFSET is the offset of the external interrupt entry in the IVT
#define EXT_INT_ENTRY_OFFSET    0x03000

// PATCH_ENTRY_OFFSET is the offset into the IVT of the entry that is coopted (stolen)
// for use by the handler.  The entire entry is restored when the handler is
// unloaded.
#define PATCH_ENTRY_OFFSET      0x03400

// PATCH_BUNDLES is the number of bundles actually in the patch
#define NUM_PATCH_BUNDLES       ((EndPatchCode - PatchCode) / 0x10)

// A hard coded branch back into the external interrupt IVT entry's second bundle
// is put here, just in case the original bundle zero did not have a branch
// This is the last bundle in the reserved IVT entry
#define FAILSAFE_BRANCH_OFFSET  (PATCH_ENTRY_OFFSET + 0x400 - 0x10)

// the original external interrupt IVT entry bundle zero is copied and relocated
// here... also in the reserved IVT entry
// This is the second-to-last bundle in the reserved IVT entry
#define RELOCATED_EXT_INT       (PATCH_ENTRY_OFFSET + 0x400 - 0x20)

// The patch is actually stored at the end of IVT:PATCH_ENTRY.  The PATCH_OFFSET
// is the offset into IVT where the patch is actually stored.  It is carefully
// located so that when we run out of patch code, the next bundle is the
// relocated bundle 0 from the original external interrupt handler
#define PATCH_OFFSET            (PATCH_ENTRY_OFFSET + 0x400 - ( EndPatchCode - PatchCode ) - 0x20)

#define PATCH_RETURN_OFFSET     (PATCH_ENTRY_OFFSET + 0x400 - ( EndPatchCode - PatchCodeRet ) - 0x20)

// PATCH_BRANCH is used only in the new bundle that is placed at the beginning
// of the external interrupt IVT entry.
#define PATCH_BRANCH            (PATCH_OFFSET - EXT_INT_ENTRY_OFFSET)

