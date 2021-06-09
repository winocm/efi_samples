///////////////////////////////////////////////////////////////////////////////
//
// This software is provided "as is" with no warranties, express or
// implied, including but not limited to any implied warranty of
// merchantability, fitness for a particular purpose, or freedom from
// infringement.
//
// Intel Corporation may have patents or pending patent applications,
// trademarks, copyrights, or other intellectual property rights that
// relate to this software.  The furnishing of this document does not
// provide any license, express or implied, by estoppel or otherwise,
// to any such patents, trademarks, copyrights, or other intellectual
// property rights.
//
// This software is furnished under license and may only be used or
// copied in accordance with the terms of the license. Except as
// permitted by such license, no part of this software may be reproduced,
// stored in a retrieval system, or transmitted in any form or by any
// means without the express written consent of Intel Corporation.
//
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

//****************************************************************************************//
// COLLECTION.H
//
//  This is a C object (data structure plus methods) that is
//  used to store and retrieve and iterate over a group of
//  homogeneous things.
//
//
//
//****************************************************************************************//



#ifndef _COLLECTION_H_
#define _COLLECTION_H_



//This constant will be compiled in only one module internal to BIS.

typedef struct _COLLECTION
{
    UINTN capacity;            //#of elements in 'theElements' (array).
    UINTN capacityIncrement;   //amount to increment by when expanding.
    UINTN currentCount;        //current number of active elements.
    UINTN maxCount;            //highest value ever of currentCount
    UINTN compareCount;        //nbr compares done during lifetime.
    UINTN *theElements;        //storage for 'capacity' 32 bit elements.
}
COLLECTION;

#define     COL_ERROR (0xFFFFFFFF)



COLLECTION *COL_New( UINTN initialCapacity, UINTN capacityIncrement);

void        COL_Destroy( COLLECTION *Col );

BIS_BOOLEAN COL_AddElement( COLLECTION *Col,      UINTN element);

BIS_BOOLEAN COL_ElementAt( COLLECTION *Col, UINTN index, UINTN *element);

BIS_BOOLEAN COL_RemoveElementAt( COLLECTION *Col, UINTN index);

BIS_BOOLEAN COL_RemoveElement( COLLECTION *Col,   UINTN element);

UINTN       COL_GetCount( COLLECTION *Col );

UINTN       COL_IndexOf( COLLECTION *Col, UINTN element);






#endif

//History
//   branched from smbios-bis /SMA/Src/Linux/inc/COLLECTION.H
//   to eliminate #include of bistype.h without introducing #ifdefs
//   Revision: 9
//   Date: 7/23/99 4:02p
