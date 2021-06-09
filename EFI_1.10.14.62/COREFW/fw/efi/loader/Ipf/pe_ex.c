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

    pe_ex.c

Abstract:

    Fixes IA64 specific relocation types


Revision History

--*/

#include "..\loader.h"
#include "pe.h"


//
// Declare runtime functions
//

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(ConvertPeImage_Ex)
#endif

#define ALIGN_POINTER(p,s)  ((VOID *) (p + ((s - ((UINTN)p)) & (s-1))))

#define EXT_IMM64(Value, Address, Size, InstPos, ValPos)  \
    Value |= (((UINT64)((*(Address) >> InstPos) & (((UINT64)1 << Size) - 1))) << ValPos)

#define INS_IMM64(Value, Address, Size, InstPos, ValPos)  \
    *(UINT32*)Address = (*(UINT32*)Address & ~(((1 << Size) - 1) << InstPos)) | \
          ((UINT32)((((UINT64)Value >> ValPos) & (((UINT64)1 << Size) - 1))) << InstPos)

#define IMM64_IMM7B_INST_WORD_X         3  
#define IMM64_IMM7B_SIZE_X              7  
#define IMM64_IMM7B_INST_WORD_POS_X     4  
#define IMM64_IMM7B_VAL_POS_X           0  

#define IMM64_IMM9D_INST_WORD_X         3  
#define IMM64_IMM9D_SIZE_X              9  
#define IMM64_IMM9D_INST_WORD_POS_X     18  
#define IMM64_IMM9D_VAL_POS_X           7  

#define IMM64_IMM5C_INST_WORD_X         3  
#define IMM64_IMM5C_SIZE_X              5  
#define IMM64_IMM5C_INST_WORD_POS_X     13  
#define IMM64_IMM5C_VAL_POS_X           16  

#define IMM64_IC_INST_WORD_X            3  
#define IMM64_IC_SIZE_X                 1  
#define IMM64_IC_INST_WORD_POS_X        12  
#define IMM64_IC_VAL_POS_X              21  

#define IMM64_IMM41a_INST_WORD_X        1  
#define IMM64_IMM41a_SIZE_X             10  
#define IMM64_IMM41a_INST_WORD_POS_X    14  
#define IMM64_IMM41a_VAL_POS_X          22  

#define IMM64_IMM41b_INST_WORD_X        1  
#define IMM64_IMM41b_SIZE_X             8  
#define IMM64_IMM41b_INST_WORD_POS_X    24  
#define IMM64_IMM41b_VAL_POS_X          32  

#define IMM64_IMM41c_INST_WORD_X        2  
#define IMM64_IMM41c_SIZE_X             23  
#define IMM64_IMM41c_INST_WORD_POS_X    0  
#define IMM64_IMM41c_VAL_POS_X          40  

#define IMM64_SIGN_INST_WORD_X          3  
#define IMM64_SIGN_SIZE_X               1  
#define IMM64_SIGN_INST_WORD_POS_X      27  
#define IMM64_SIGN_VAL_POS_X            63  

STATIC
EFI_STATUS
RUNTIMEFUNCTION
LoadPeRelocate_Ex (
    IN UINT16      *Reloc,
    IN OUT CHAR8   *Fixup, 
    IN OUT CHAR8   **FixupData,
    IN UINT64      Adjust
    )
{
    UINT64                      *F64, FixupVal;
    EFI_STATUS                  Status;

    switch ((*Reloc) >> 12) {

        case IMAGE_REL_BASED_DIR64:
            F64 = (UINT64 *) Fixup;
            *F64 = *F64 + (UINT64) Adjust;
            *FixupData = ALIGN_POINTER(*FixupData, sizeof(UINT64));
            *(UINT64 *)(*FixupData) = *F64;
            *FixupData = *FixupData + sizeof(UINT64);
            break;

        case IMAGE_REL_BASED_IA64_IMM64:
            //
            // Align it to bundle address before fixing up the
            // 64-bit immediate value of the movl instruction.
            //
                       
            (UINT64) Fixup = ((UINT64) Fixup & (UINT64) ~(15));
            FixupVal = (UINT64)0;
                       
            //
            // Extract the lower 32 bits of IMM64 from bundle
            //
                      
            EXT_IMM64(FixupVal,
                     (UINT32 *)Fixup + IMM64_IMM7B_INST_WORD_X,
                     IMM64_IMM7B_SIZE_X,
                     IMM64_IMM7B_INST_WORD_POS_X,
                     IMM64_IMM7B_VAL_POS_X);

            EXT_IMM64(FixupVal,
                     (UINT32 *)Fixup + IMM64_IMM9D_INST_WORD_X,
                     IMM64_IMM9D_SIZE_X,
                     IMM64_IMM9D_INST_WORD_POS_X,
                     IMM64_IMM9D_VAL_POS_X);

            EXT_IMM64(FixupVal,
                     (UINT32 *)Fixup + IMM64_IMM5C_INST_WORD_X,
                     IMM64_IMM5C_SIZE_X,
                     IMM64_IMM5C_INST_WORD_POS_X,
                     IMM64_IMM5C_VAL_POS_X);

            EXT_IMM64(FixupVal,
                     (UINT32 *)Fixup + IMM64_IC_INST_WORD_X,
                     IMM64_IC_SIZE_X,
                     IMM64_IC_INST_WORD_POS_X,
                     IMM64_IC_VAL_POS_X);

            EXT_IMM64(FixupVal,
                     (UINT32 *)Fixup + IMM64_IMM41a_INST_WORD_X,
                     IMM64_IMM41a_SIZE_X,
                     IMM64_IMM41a_INST_WORD_POS_X,
                     IMM64_IMM41a_VAL_POS_X);
                       
            //
            // Update 64-bit address
            //
                       
            FixupVal += Adjust;

            //
            // Insert IMM64 into bundle
            //
                       
            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_IMM7B_INST_WORD_X),
                     IMM64_IMM7B_SIZE_X,
                     IMM64_IMM7B_INST_WORD_POS_X,
                     IMM64_IMM7B_VAL_POS_X);

            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_IMM9D_INST_WORD_X),
                     IMM64_IMM9D_SIZE_X,
                     IMM64_IMM9D_INST_WORD_POS_X,
                     IMM64_IMM9D_VAL_POS_X);

            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_IMM5C_INST_WORD_X),
                     IMM64_IMM5C_SIZE_X,
                     IMM64_IMM5C_INST_WORD_POS_X,
                     IMM64_IMM5C_VAL_POS_X);

            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_IC_INST_WORD_X),
                     IMM64_IC_SIZE_X,
                     IMM64_IC_INST_WORD_POS_X,
                     IMM64_IC_VAL_POS_X);

            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_IMM41a_INST_WORD_X),
                     IMM64_IMM41a_SIZE_X,
                     IMM64_IMM41a_INST_WORD_POS_X,
                     IMM64_IMM41a_VAL_POS_X);

            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_IMM41b_INST_WORD_X),
                     IMM64_IMM41b_SIZE_X,
                     IMM64_IMM41b_INST_WORD_POS_X,
                     IMM64_IMM41b_VAL_POS_X);

            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_IMM41c_INST_WORD_X),
                     IMM64_IMM41c_SIZE_X,
                     IMM64_IMM41c_INST_WORD_POS_X,
                     IMM64_IMM41c_VAL_POS_X);

            INS_IMM64(FixupVal,
                     ((UINT32 *)Fixup + IMM64_SIGN_INST_WORD_X),
                     IMM64_SIGN_SIZE_X,
                     IMM64_SIGN_INST_WORD_POS_X,
                     IMM64_SIGN_VAL_POS_X);

            F64 = (UINT64 *) Fixup;
            *FixupData = ALIGN_POINTER(*FixupData, sizeof(UINT64));
            *(UINT64 *)(*FixupData) = *F64;
            *FixupData = *FixupData + sizeof(UINT64);
            break;

        default:
            DEBUG((D_LOAD|D_ERROR, "PeRelocate_Ex: unknown fixed type\n"));
            Status = EFI_UNSUPPORTED;
            goto Done;
    }

    Status = EFI_SUCCESS;

Done:

    return Status;
}


STATIC
EFI_STATUS
RUNTIMEFUNCTION INTERNAL
ConvertPeImage_Ex (
    IN UINT16      *Reloc,
    IN OUT CHAR8   *Fixup, 
    IN OUT CHAR8   **FixupData,
    IN UINT64      Adjust
    )
// Reapply fixups to move an image to a new address
{
    UINT64                      *F64, FixupVal;
    EFI_STATUS                  Status;

    switch ((*Reloc) >> 12) {
        case IMAGE_REL_BASED_DIR64:
            F64 = (UINT64 *) Fixup;
            *FixupData = ALIGN_POINTER(*FixupData, sizeof(UINT64));
            if (*(UINT64 *)(*FixupData) == *F64) {
                *F64 = *F64 + (UINT64) Adjust;
            }
            *FixupData = *FixupData + sizeof(UINT64);
            break;

        case IMAGE_REL_BASED_IA64_IMM64:
            F64 = (UINT64 *) Fixup;
            *FixupData = ALIGN_POINTER(*FixupData, sizeof(UINT64));
            if (*(UINT64 *)(*FixupData) == *F64) {

                //
                // Align it to bundle address before fixing up the
                // 64-bit immediate value of the movl instruction.
                //
                       
                (UINT64) Fixup = ((UINT64) Fixup & (UINT64) ~(15));
                FixupVal = (UINT64)0;
                       
                //
                // Extract the lower 32 bits of IMM64 from bundle
                //
                      
                EXT_IMM64(FixupVal,
                        (UINT32 *)Fixup + IMM64_IMM7B_INST_WORD_X,
                        IMM64_IMM7B_SIZE_X,
                        IMM64_IMM7B_INST_WORD_POS_X,
                        IMM64_IMM7B_VAL_POS_X);

                EXT_IMM64(FixupVal,
                        (UINT32 *)Fixup + IMM64_IMM9D_INST_WORD_X,
                        IMM64_IMM9D_SIZE_X,
                        IMM64_IMM9D_INST_WORD_POS_X,
                        IMM64_IMM9D_VAL_POS_X);

                EXT_IMM64(FixupVal,
                        (UINT32 *)Fixup + IMM64_IMM5C_INST_WORD_X,
                        IMM64_IMM5C_SIZE_X,
                        IMM64_IMM5C_INST_WORD_POS_X,
                        IMM64_IMM5C_VAL_POS_X);

                EXT_IMM64(FixupVal,
                        (UINT32 *)Fixup + IMM64_IC_INST_WORD_X,
                        IMM64_IC_SIZE_X,
                        IMM64_IC_INST_WORD_POS_X,
                        IMM64_IC_VAL_POS_X);

                EXT_IMM64(FixupVal,
                        (UINT32 *)Fixup + IMM64_IMM41a_INST_WORD_X,
                        IMM64_IMM41a_SIZE_X,
                        IMM64_IMM41a_INST_WORD_POS_X,
                        IMM64_IMM41a_VAL_POS_X);
                       
                //
                // Update 64-bit address
                //
                       
                FixupVal += Adjust;

                //
                // Insert IMM64 into bundle
                //
                       
                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_IMM7B_INST_WORD_X),
                        IMM64_IMM7B_SIZE_X,
                        IMM64_IMM7B_INST_WORD_POS_X,
                        IMM64_IMM7B_VAL_POS_X);

                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_IMM9D_INST_WORD_X),
                        IMM64_IMM9D_SIZE_X,
                        IMM64_IMM9D_INST_WORD_POS_X,
                        IMM64_IMM9D_VAL_POS_X);

                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_IMM5C_INST_WORD_X),
                        IMM64_IMM5C_SIZE_X,
                        IMM64_IMM5C_INST_WORD_POS_X,
                        IMM64_IMM5C_VAL_POS_X);
    
                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_IC_INST_WORD_X),
                        IMM64_IC_SIZE_X,
                        IMM64_IC_INST_WORD_POS_X,
                        IMM64_IC_VAL_POS_X);

                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_IMM41a_INST_WORD_X),
                        IMM64_IMM41a_SIZE_X,
                        IMM64_IMM41a_INST_WORD_POS_X,
                        IMM64_IMM41a_VAL_POS_X);

                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_IMM41b_INST_WORD_X),
                        IMM64_IMM41b_SIZE_X,
                        IMM64_IMM41b_INST_WORD_POS_X,
                        IMM64_IMM41b_VAL_POS_X);

                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_IMM41c_INST_WORD_X),
                        IMM64_IMM41c_SIZE_X,
                        IMM64_IMM41c_INST_WORD_POS_X,
                        IMM64_IMM41c_VAL_POS_X);

                INS_IMM64(FixupVal,
                        ((UINT32 *)Fixup + IMM64_SIGN_INST_WORD_X),
                        IMM64_SIGN_SIZE_X,
                        IMM64_SIGN_INST_WORD_POS_X,
                        IMM64_SIGN_VAL_POS_X);

                *(UINT64 *)(*FixupData) = *F64;
            }
            
            *FixupData = *FixupData + sizeof(UINT64);
            break;
        
        default:
            DEBUG((D_LOAD|D_ERROR, "ConvertPeImage_Ex: unknown fixed type\n"));
            Status = EFI_UNSUPPORTED;
            goto Done;
    }

    Status = EFI_SUCCESS;

Done:

    return Status;

}
