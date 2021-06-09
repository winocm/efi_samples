/* SCCSID: inc/ber_der.h, dss_cdsa_fwk, fwk_rel2, dss_971010 1.5 10/23/97 17:53:30 */
/*-----------------------------------------------------------------------
 *      File:   ber_der.h
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
 * This is the header file for some Basic Encoding Rules and Distinguished
 * Encoding Rules (BER/DER) routines.
 */
/*
 * (C) COPYRIGHT International Business Machines Corp. 1996, 1997
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef BER_DER_H
#define BER_DER_H

#if defined (WIN32)
#pragma warning (disable:4201 4514 4214 4115)
#include <windows.h>
#pragma warning (default:4201 4214 4115)
#endif


#ifdef EFI64
/* Basic Types */
typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef          __int16 sint16;
typedef unsigned __int32 uint32;
typedef          __int32 sint32;

#else
/* Basic Types */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned int uint32;
typedef int sint32;
#endif

#ifndef CSSMAPI
	#if defined (WIN32)
		#define CSSMAPI __stdcall
	
	#elif defined (WIN31)
		#ifndef PASCAL
			#define PASCAL __stdcall
		#endif
		#define CSSMAPI PASCAL
	
	#elif defined (UNIX) 
		#define CSSMAPI

	#elif defined (OASIS) 
		#define CSSMAPI
	#endif
#endif

#ifndef CALLBACK
	#if defined (UNIX)
		#define CALLBACK
	#endif
#endif

typedef void *	BER_HANDLE;
typedef struct ber_memory_funcs {
    void *(CSSMAPI *malloc_func) (BER_HANDLE AddInHandle, uint32 Size);
    void (CSSMAPI *free_func) (BER_HANDLE AddInHandle, void *MemPtr);
    void *(CSSMAPI *realloc_func) (BER_HANDLE AddInHandle, void *MemPtr, uint32 Size);
    void *(CSSMAPI *calloc_func) (BER_HANDLE AddInHandle, uint32 Num, uint32 Size);
} BER_MEMORY_FUNCS, *BER_MEMORY_FUNCS_PTR;


#define BER_BOOLEAN					1 
#define BER_INTEGER                 2
#define BER_BIT_STRING              3
#define BER_OCTET_STRING            4
#define BER_NULL                    5
#define BER_OBJECT_IDENTIFIER       6
#define BER_SEQUENCE                16
#define BER_SET                     17
#define BER_PRINTABLE_STRING        19
#define BER_T61STRING               20
#define BER_IA5STRING               22
#define BER_UTCTIME                 23
#define BER_GENTIME                 24

#define BER_CLASS_MASK              0xc0
#define BER_CLASS_UNIVERSAL         0x00
#define BER_CLASS_APPLICATION       0x40
#define BER_CLASS_CONTEXTSPECIFIC   0x80
#define BER_CLASS_PRIVATE           0xc0

#define BER_CONSTRUCTED             0x20

#define BER_CONSTRUCTED_SEQUENCE    (BER_CONSTRUCTED | BER_SEQUENCE)
#define BER_CONSTRUCTED_SET         (BER_CONSTRUCTED | BER_SET)

#define BER_UNIVERSAL_TAG_LENGTH    1

#define ALL_TAG_VALUES_MASK      0x1F
#define IS_LONG_TAG(a)           ((a & 0x1F) == 0x1F) ? 1 : 0
#define IS_LAST_LONG_TAG_BYTE(a) ((a & 0x80) == 0x00) ? 1 : 0

#define LONG_LEN_INDICATOR    0x80
#define LONG_LEN_REQD         LONG_LEN_INDICATOR
#define IS_SHORT_FORM_LEN(a)  ((a & LONG_LEN_INDICATOR) == 0) ? 1 : 0
#define NUM_LENGTH_BYTES(a)   (a & ~LONG_LEN_INDICATOR)

#define INDEF_END_OF_CONTENTS_NUM_BYTES 2
#define INDEF_END_OF_CONTENTS_MARKER    0x00


/* the following structure is the same as CSSM SPI, but we don't depend on cssm */
typedef struct ber_parsed_item {
    const uint8  *Tag;
          uint32 ContentLength;
    const uint8  *Content;
} BER_PARSED_ITEM, *BER_PARSED_ITEM_PTR;

/* compare two tags per DER rules */
sint32 
BER_TagCompare(const uint8 *A, const uint8 *B);

/* get length of BER length field */
uint32 
BER_LengthOfLength(uint32 Length);

/* get length of BER tag */
uint32 
BER_LengthOfTag(const uint8 *Tag);

/* copy a BER tag to the output and return its length */
uint32 
BER_OutputTag(            /* return length of copied tag */
    const uint8 *Source,          /* input BER/DER tag */
          uint8 *Octets);         /* output buffer */

/* DER encode a length and return its length */
uint32 
BER_OutputLength(         /* return number of bytes it took to store the length */
    uint32 Value,           /* length as binary integer */
    uint8 *Octets);         /* output buffer */

/* given a pointer to a BER object, return its length in octets */
uint32 
BER_SizeofObject(const uint8 *Octets);

/* decode a small BER integer to a C unsigned long value */
uint32 
BER_BerToUnlong(const uint8 *Octets);

/* encode an unsigned long into a DER integer at the given memory, return length in octets */
uint32 
DER_UnlongToDer(
    uint32 Value, 
    uint8 *Octets);

/* 
 * Function to decode a primitive BER bit string into an array of bytes. 
 * The function returns -2 if the input is not a valid primitive bit string.
 * The function result is the length of the decoded bit string in *BITS*. 
 * If the target buffer (Bytes) is not sufficiently large to hold the result,
 * the function returns -1 and does not alter the output buffer. 
 * Any odd bits are in the most significant bits of the last byte.
 */
sint32 
BER_DecodeBitString(
	const uint8 *Octets,                  /* pointer to input BER_BIT_STRING octets */
          uint8 *Bytes,                   /* pointer to output buffer */
          uint32 SizeofBytes);            /* size of output buffer in bytes */
/*
 * Function to DER-encode a bit string. The function result is the number of *BYTES* used
 * to DER-encode the bit string. If the output buffer is too small, -1 is returned and it
 * is not changed. Any odd bytes must be in the most significant bits of the trailing byte.
 */
sint32  
DER_EncodeBitString(
	const uint8 *Bytes,                   /* pointer to input bit string */
          uint32 NumberOfBits,            /* length of bit string in *BITS* */
          uint8 *Octets,                  /* pointer to output buffer */
          uint32 SizeofOctets);           /* length of output buffer in BYTES */
/* decode an object identifier into an array of unsigned long integers */
sint32 
BER_OidDecode(                    /* return number of integers found */
    const uint8 *Octets,          /* input BER OBJECT IDENTIFIER */
          uint32 *Numbers,        /* array of unsigned long for output */
          sint32 Dimension);      /* dimension of numbers array */

/* encode an array of unsigned long to a DER object identifier octets */
sint32 
DER_EncodeOid(                    /* return number of bytes used to encode it */
    const uint32 *Numbers,        /* the array of integers */
          sint32 Dimension,       /* number of integers in OID */
          uint8 *Octets);         /* output buffer */

/* expand a single BER item */
const uint8 *
BER_ExpandItem(                   /* return updated octets if successful, 0 if not */
    const uint8 *Octets,          /* input BER to parse */
          uint32 MaxLength,       /* maximum length of BER to parse (may not all be in memory) */
          BER_PARSED_ITEM_PTR Output);        /* output is single structure */

/*
**  find a BER attribute value (still BER), given an OID and a BER-encoded attribute list
**  returns NIL if not found. Assumes SEQ(OID, value) structure somewhere in subtree.
*/
const uint8 *
BER_FindAttribute(          /* find BER attribute value for given OID in attributes */
    BER_HANDLE AppHandle,
    BER_MEMORY_FUNCS_PTR MemoryFunctions,
    const uint8 *Oid,             /* desired BER OID */
    const uint8 *Attributes);     /* BER with SEQ(OID, value) pairs somewhere */
/* return the cardinality of a series of BER items */
sint32
BER_CountItems(
	const uint8 *Octets, 
	      uint32 Length);
/*
**  In order to prevent multiple copying of data, and a general memory allocation free-for-all,
**  lets construct a parse tree, and then traverse it to get the size of the output. A second
**  traversal can then construct the output DER string. So we need a tree node structure...
*/
typedef struct der_complex_struct DER_COMPLEX_TYPE;
typedef DER_COMPLEX_TYPE *DER_COMPLEX_TYPE_PTR;

typedef struct der_explicit_choices {
    sint32 NumberOfChoices;         /* number of tag values */
    sint32 ParentField;             /* tag field (same field means implicit tag) */
    BER_PARSED_ITEM_PTR *Oids;      /* vector of object ID tag values (if implicit--just use tag) */
    DER_COMPLEX_TYPE_PTR *Subtypes; /* corresponding subtypes */
} DER_EXPLICIT_CHOICES, *DER_EXPLICIT_CHOICES_PTR;

typedef struct der_complex_struct {
    const uint8 *TypeTag;                 /* tag for this complex type */
          sint32 NumberOfDefaults;        /* number of fields (SEQUENCE) or types (SET) */
          DER_EXPLICIT_CHOICES_PTR *Choices;  /* if explicitly tagged, pointer to choices for each field*/
    const uint8 **ContextTags;            /* context tags for this type if implicit, else null. */
    const uint8 **OriginalTags;           /* real tags for each field */
          BER_PARSED_ITEM_PTR Defaults;   /* defaults for this type, else null */
          DER_COMPLEX_TYPE_PTR *Subtypes; /* complex subtypes, if any (may be null) */
          char *Name;                     /* name for debugging purposes */
} DER_COMPLEX_STRUCT, *DER_COMPLEX_STRUCT_PTR;

typedef struct der_node_struct DER_NODE;
typedef DER_NODE *DER_NODE_PTR;

typedef struct der_node_child_struct {
    uint8 IsLeaf;
    struct {
        BER_PARSED_ITEM Input;
        DER_NODE_PTR Node;
    } X;
} DER_NODE_CHILD, *DER_NODE_CHILD_PTR;

struct der_node_struct {
    const uint8 *Tag;
    const uint8 *OriginalTag;
    sint32 Count;
    uint32 Length;                  /* filled in by sizeofDERTree */
    DER_COMPLEX_TYPE *Type;
    DER_NODE_CHILD_PTR Child;
};

/* allocate DER_node with  nChildren children */
DER_NODE_PTR 
DER_AllocateNode(BER_HANDLE AppHandle,
                 BER_MEMORY_FUNCS_PTR MemoryFunctions, 
                 sint32 NumberOfChildren);

/* Copy parse tree nodes to create another duplicate tree */
DER_NODE_PTR
DER_DuplicateParseTree(
		BER_HANDLE AppHandle, 
		BER_MEMORY_FUNCS_PTR MemoryFunctions, 
		const DER_NODE *Tree);

/* recursively free DER nodes   */
void 
DER_RecycleTree(BER_HANDLE AppHandle,
                BER_MEMORY_FUNCS_PTR MemoryFunctions,
                DER_NODE_PTR Node);

/* expand one level in parsing a BER ASN.1 sequence */
sint32
BER_ExpandSequence(                           /* return count of items sucessfully parsed */
    const uint8 *Octets,                      /* input BER to parse */
          uint32 InputLength,                 /* length of BER to parse (may not all be in memory) */
          sint32 Count,                       /* input number of components */
    const DER_EXPLICIT_CHOICES_PTR *Choices,  /* explicit tag choices for each field, if any */
    const uint8 **ContextTags,                /* implicit context-specific tags for each component */
    const uint8 **OriginalTags,               /* original tags for each component */
    const BER_PARSED_ITEM *Defaults,          /* default values */
          BER_PARSED_ITEM *Output);           /* output: array of parsed items */

/* expand a BER set one level */
sint32 
BER_ExpandSet(                                /* return count of items in set sucessfully parsed */
    const uint8 *Octets,                      /* input BER to parse */
          uint32 InputLength,                 /* length of BER to parse (may not all be in memory) */
          sint32 MaxCount,                    /* maximum number of items we can accept (card of output) */
          sint32 NumberOfTypes,               /* number of tags */
    const BER_PARSED_ITEM *Defaults,          /* default values */
          BER_PARSED_ITEM_PTR Output);        /* output: array of parsed items */

/*
**  parse complex BER object
**  if the length of the root object is not the length of the octets, we have an error.
*/
DER_NODE_PTR 
BER_ParseComplexObject(                      /* return parse tree */
    BER_HANDLE AppHandle,
    BER_MEMORY_FUNCS_PTR MemoryFunctions,
    DER_COMPLEX_TYPE *Type,          /* type of object */
    BER_PARSED_ITEM *Item);          /* BER object broken into tag, length, content */

/* 
**  Gather up parts of values and concatentate into a larger BER object 
*/
uint32 
DER_ConstructItem(
    const uint8 *BerTag,                      /* top-level tag to output */
          sint32 Count,                       /* number of elements */
          BER_PARSED_ITEM *Inputs,    /* array of parsed items */
          uint8 *Octets);                     /* target memory for the output */
/* 
 * Generate a DER-encoded, flat octet representation of a given parse tree.
 * The input parse tree is *not* modified (a temporary copy is modified).
 * A buffer containing the DER is returned if successful, NULL if unsuccessful.
 * The user is responsible for deallocating the buffer using the memory free function.
 */
uint8 *
DER_Generate(
	BER_HANDLE AppHandle,
    BER_MEMORY_FUNCS_PTR MemoryFunctions,
	const DER_NODE *InputTree);
/*
**  add up sizes of parse tree, so we can reserve a sufficiently large output buffer
**  Eliminate instances of default values for sequence and set nodes (yes, a side effect). 
*/
uint32 
DER_SizeofTree(DER_NODE_PTR Node);

/* 
**  copy tree to contiguous octet buffer--we assume output is sufficiently large
**  Be sure to sort the set tags when traversing the tree.
*/
uint32 
DER_CopyTree(
    DER_NODE_PTR Node,  /* tree to linearize */
    uint8 *Octets);     /* output buffer */

/*
**  return true iff two DER nodes are equivalent
*/
sint32 
DER_EqualNodes(DER_NODE_PTR A, DER_NODE_PTR B);

/*
**  print out DER tree
*/
void 
DER_PrintTree(DER_NODE_PTR Node, sint32 Level);

#endif /* BER_DER_H */
