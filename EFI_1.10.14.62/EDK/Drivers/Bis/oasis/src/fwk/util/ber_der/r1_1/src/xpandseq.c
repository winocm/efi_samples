/*-----------------------------------------------------------------------
 *      File:   xpandseq.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------
 */
/* 
 * INTEL CONFIDENTIAL 
 * This file, software, or program is supplied under the terms of a 
 * license agreement or nondisclosure agreement with Intel Corporation 
 * and may not be copied or disclosed except in accordance with the 
 * terms of that agreement. This file, software, or program contains 
 * copyrighted material and/or trade secret information of Intel 
 * Corporation, and must be treated as such. Intel reserves all rights 
 * in this material, except as the license agreement or nondisclosure 
 * agreement specifically indicate. 
 */ 
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software is subject to the U.S. Export Administration Regulations 
 * and other U.S. law, and may not be exported or re-exported to certain 
 * countries (currently Afghanistan (Taliban-controlled areas), Cuba, Iran, 
 * Iraq, Libya, North Korea, Serbia (except Kosovo), Sudan and Syria) or to 
 * persons or entities prohibited from receiving U.S. exports (including Denied 
 * Parties, Specially Designated Nationals, and entities on the Bureau of 
 * Export Administration Entity List or involved with missile technology or 
 * nuclear, chemical or biological weapons).
 */ 
/*
 * DISCLAIMER: CODE QUALITY AND DOCUMENTATION
 * Basic Encoding Rules and Distinguished Encoding Rules (BER/DER) routines.
 * This is alpha-quality code and modifications should be expected for the
 * next release.  This code is provided by Intel "as is", and Intel makes no
 * warranties, express, implied or statutory, and expressly disclaims any
 * warranties of merchantability, noninfringement of intellectual property
 * rights, and fitness for a particular purpose.
 */

#include "ber_der.h"
#include "ber_internal.h"

/*------------------------------------------------------------------------------
 * Name: BER_ExpandSequence
 *
 * Description:
 * BER_ParseComplexObject uses this function when a SEQUENCE is encountered. 
 * The function fills in the output array of BER_PARSED_ITEM elements. 
 * It detects missing fields and variant data structures using the input template.
 *
 * Parameters: 
 * Octets   (input) - A pointer to the octets to be parsed.
 * InputLength (input) - Number of octets to be parsed.
 * Count    (input) - The maximum number of output components to be parsed.  
 *                    This is also the rank of the ContextTags, OriginalTags,
 *                    and Defaults arrays, for each array that is present.
 * Choices  (input) - A pointer to an array of pointers to DER_EXPLICIT_CHOICES
 *                    indicating variant data structures.  If there are no  
 *                    variants, the pointer is NULL.  Components that 
 *                    do not have variants are indicated by a corresponding 
 *                    NULL array element.  Variant values may be referenced by 
 *                    the output BER_PARSED_ITEM values, and therefore must have 
 *                    a longer lifetime than the output.
 * ContextTags (input) - A pointer to an array of the tags expected to be 
 *                    found in the input BER-encoded Octets.  This will have the 
 *                    context-specific tag value for components that have 
 *                    context-specific tags.  It will have a NULL tag value for 
 *                    variant components.  It will have the original universal or 
 *                    private tag value for all other components.  These values  
 *                    are used to resolve ambiguity in the presence of 
 *                    missing fields.  If there are no context-specific tags, 
 *                    this input can be NULL.  Context-specific tags may be 
 *                    referenced by the output BER_PARSED_ITEM values, 
 *                    and therefore must have a longer lifetime than the output.
 * OriginalTags (input) - A pointer to an array of pointers to underlying tags
 *                    for the components. Where context-specific tags are used, 
 *                    these are the original tags of the underlying data types.  
 *                    The original tags may be used to detect missing components.
 *                    They are required if defaults or context-specific tags 
 *                    are used in the sequence.  If none of these are present 
 *                    in the sequence, the OriginalTags parameter may be NULL.
 * Defaults (input) - A pointer to an array of pointers to BER-encoded 
 *                    default values for each component. Missing components are 
 *                    replaced by their default values.  If there are no  
 *                    default values, the pointer is NULL. Components that 
 *                    do not have default values are indicated by a corresponding 
 *                    NULL array element.  Default values may be referenced by 
 *                    the output BER_PARSED_ITEM values, and therefore must have 
 *                    a longer lifetime than the output.
 * Output  (output) - This parameter points to an array that is filled in with 
 *                    the components of the sequence. The array must be 
 *                    of sufficient size to hold the output.  
 *                    Missing components with no default values are indicated by
 *                    NULL tag pointers, zero lengths, and NULL content pointers.
 *
 * Returns: 
 * The number of components successfully parsed is returned.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
sint32 
BER_ExpandSequence(             /* returns the number of components parsed    */
    const uint8 *Octets,        /* input BER to parse                         */
    uint32 InputLength,         /* length of BER to parse                     */
    sint32 Count,               /* expected number of components              */
    const DER_EXPLICIT_CHOICES_PTR *Choices, /* variant tag choices           */
    const uint8 **ContextTags,  /* expected tags for non-variant components.  */
    const uint8 **OriginalTags, /* universal or private tags. These are       */
                                /* the same as the expected tags for          */
                                /* non-ctx-specific, non-variant components   */
                                /* For ctx-specific tags, this tag represents */
                                /* the underlying universal or private tag.   */
    const BER_PARSED_ITEM *Defaults, /* default values                        */
    BER_PARSED_ITEM *Output)    /* output array of parsed items               */
{
    const uint8 *currentPosition;
    uint32 remainingInput;
    uint32 extTagLength;
    sint32 i, j, k;

    /* Initialize variables */
    currentPosition = Octets;
    remainingInput = InputLength;

    /* For each component in the SEQUENCE,                           */
    /* - Expand it into its Tag, Length and Content representation.  */
    /* - Use the input ContextTags, OriginalTags, and Choices arrays */
    /*   to obtain the correct tag value                             */
    /* - Use the input ContextTags, OriginalTags and Choices arrays  */
    /*   to identify missing components.                             */
    /* - Use the defaults array to fill in missing components.       */
    for (i = 0; i < Count; i++) {

      /* If there are no more octets to parse, this is a missing component */
      /* Fill it with its default value or zeroes */
      if (remainingInput > 0) {

        /* Expand the current component into its  */
        /* Tag, Length and Content representation */
        currentPosition = 
            BER_ExpandItem(currentPosition, remainingInput, &Output[i]);
        if (!currentPosition) return i;
        remainingInput = InputLength - (uint32)(currentPosition - Octets);

        /* If there is no context-specific tag and no variant tag          */
        /* for this component, verify that either the original tag matches */
        /* or we are accepting any tags (OriginalTags == NULL).            */
        if ((!ContextTags || !ContextTags[i]) && (!Choices || !Choices[i]) ) { 
            if (OriginalTags && 
                BER_TagCompare(Output[i].Tag, OriginalTags[i]) != 0)
                return i;     /* tag doesn't match, return error */
            continue;         /* tag matches or we are accepting anything */
        }

        /* There is a variant or context specific tag for this component. */
        /* Determine what the original tag should be for this component.  */
        /* If the current tag does not match either the current context-  */
        /* specific tag or a variant tag, assume that there is a missing  */
        /* component in the input BER. Proceed to the next expected tag   */
        /* and the next variant to look for a match for the current tag.  */
        for (j = i; j < Count; j++) {

            /* If the current tag matches the expected tag, break.        */
            /*    If the expected tag is context-specific,                */
            /*    set the current tag to the appropriate original tag     */
            if (ContextTags && ContextTags[j] &&
                BER_TagCompare(Output[i].Tag, ContextTags[j]) == 0) {
                if (OriginalTags && OriginalTags[j] && 
                    (Output[i].Tag[0] & BER_CLASS_MASK) == 
                                            BER_CLASS_CONTEXTSPECIFIC)
                    Output[i].Tag = OriginalTags[j];
                break;
            }

            /* If the current tag matches one of the variant tags,        */
            /* set the current tag to the appropriate variant tag value   */
            k = -1;
            if (Choices && Choices[j]) {
                /* Implicit tag */
                if (Choices[j]->ParentField == j) {
                    for (k = 0; k < Choices[j]->NumberOfChoices; k++) {
                        if (BER_TagCompare(Output[i].Tag, 
                                           Choices[j]->Oids[k]->Tag) == 0) { 
                            if (Choices[j]->Subtypes[k]) 
                                Output[i].Tag = Choices[j]->Subtypes[k]->TypeTag;
                            break;
                        }
                    }
                } else {    
                /* Explicit tag */
                    extTagLength = Output[Choices[j]->ParentField].ContentLength;
                    for (k = 0; k < Choices[j]->NumberOfChoices; k++) {
                        if (extTagLength == Choices[j]->Oids[k]->ContentLength &&
                            !cssm_memcmp(Output[Choices[j]->ParentField].Content,
                            Choices[j]->Oids[k]->Content, extTagLength))
                            break;
                    }
                }
                if (k < Choices[j]->NumberOfChoices)
                    break;
            }
        }

        /* If there is no match for this component (j == Count), FAIL   */
        /* Otherwise */
        /* - copy the current component to its appropriate location (j) */
        /* - if there were missing components (j > i),                  */
        /*   set the missing components to either their default values  */
        /*   or to 0 if no default was provided.                        */
        if (j < Count) {
            Output[j] = Output[i];
            for (; i < j; i++) {    
                if (Defaults && Defaults[i].Tag) 
                    Output[i] = Defaults[i];
                else 
                    cssm_memset(&Output[i], 0, sizeof(BER_PARSED_ITEM));
            }
        } else 
            return i;  /* there is no match for this component, return */

      } else {   
      /* If there is no remaining input, the remaining components are missing. */
      /* Fill them in with either their default values or 0. */
        if (Defaults && Defaults[i].Tag) 
            Output[i] = Defaults[i];
        else 
            cssm_memset(&Output[i], 0, sizeof(BER_PARSED_ITEM));
      }

    } /* End for (i = 0; i < Count; i++) */

    /* Return the number of components parsed */
    return i;
}

