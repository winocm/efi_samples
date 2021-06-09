/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

*/


//****************************************************************************************//
// COLLECTION.H
//
//	This is a C object (data structure plus methods) that is
//	used to store and retrieve and iterate over a group of
//	homogeneous things. 
//
//
/*Placeholder_for_source_control_strings*/
//
//****************************************************************************************//

#ifdef _STANDALONE_TEST         //used by win32 debug
    #include <windows.h>
    #include <stdlib.h>
    #include <stdio.h>
#endif
    

#include <bis_priv.h>



//-------------------------------------------------------------------
// Collection Constructor:
//  Create a collection with sufficient capacity for 
// 'initialCapacity' elements. When the collection needs to
//  be expanded, allocate 'capacityIncrement' additional elements
//  at a time.
//
//  Returns: pointer to collection or NULL if memory unavailable.
//

COLLECTION *
COL_New( UINTN initialCapacity, UINTN capacityIncrement)
{
    COLLECTION *col;

    //Allocate collection structure
    col = x_malloc (sizeof(COLLECTION));
    if ( col !=BIS_NULL )
    {

        //Fill it in.
        col->capacity=          initialCapacity;           
        col->capacityIncrement= capacityIncrement;

        col->theElements = x_malloc((UINT32)(initialCapacity * sizeof(UINTN)));
        //Delete collection struct if 'theElements' allocation failed.
        if ( col->theElements == BIS_NULL)
        {
    			gBS->FreePool(col);
        }

    }


    return col;
}



//-------------------------------------------------------------------
//  COL_Destroy - destroy the collection.
    
void COL_Destroy( COLLECTION *Col )
{
    gBS->FreePool(Col->theElements );
    gBS->FreePool(Col );
}




//-------------------------------------------------------------------
//add an element to the collection.
//cast your element type into a UINTN.
//returns true unless memory is exhausted and can;t expand.

BIS_BOOLEAN 
COL_AddElement( COLLECTION *Col, UINTN element)
{
    BIS_BOOLEAN rv= BIS_TRUE;
    UINTN      *newArray;
    UINTN       newCap;
    UINTN       i;


    //Add the element if there is room.
    if ( Col->currentCount < Col->capacity )
    {
        Col->theElements[Col->currentCount]= element;
        Col->currentCount += 1;
    }

    //Need to expand.
    else
    {
        //Allocate a new array, incrementing the capacity.
        newCap= Col->capacity + Col->capacityIncrement;

        newArray = x_malloc((UINT32)(newCap * sizeof(UINTN)));

        //Fail if new elements allocation failed.
        if ( newArray == BIS_NULL)
        {
            #if (COMPILE_SELFTEST_CODE==1)
            if (BIS_FLAG(MMFLAGS,MM_COLL_EXPAND))
            {
                DUMPHEAPSTATS("COLLECTION EXPANSION **FAILED** ", __LINE__);
            }
            #endif
            
            rv= BIS_FALSE;
        }

        //Copy old array to new
        else
        {
            //Copy old array to new
            for (i=0; i<Col->capacity; ++i){ newArray[i]= Col->theElements[i]; }
        
            //Delete the old array;
            gBS->FreePool(Col->theElements);

            //Install the new array.
            Col->theElements= newArray;

            //update capacity.
            Col->capacity= newCap;

            //add the element as originally requested.
            Col->theElements[Col->currentCount]= element;
            Col->currentCount += 1;

            #if (COMPILE_SELFTEST_CODE==1)
            if (BIS_FLAG(MMFLAGS,MM_COLL_EXPAND))
            {
                PUT_SDN("COLLECTION EXPANDED Size (in bytes) now ", newCap*4);
            }
            #endif
        }

    }

    if (Col->currentCount > Col->maxCount)
    {
        Col->maxCount= Col->currentCount;
    }
    return rv;
}


    //-------------------------------------------------------------------
    //retreive the element at 'index' from the collection.
    //and returns it in UINTN pointed to by 'element'.
    //returns true if success, false if index out of range.

BIS_BOOLEAN  COL_ElementAt( COLLECTION *Col, UINTN index, UINTN *element)
{
    //Validate index and bail if out of range.
    if ( index >= Col->currentCount) return BIS_FALSE;
    *element= Col->theElements[index];
    return BIS_TRUE;
}


    //-------------------------------------------------------------------
    //remove the element at 'index' from the collection.
    //returns true if succeess, false if index out of range.

    // This op invalidates elements at index+1.
    // Their index value becomes one less than they were before being invalidated.


BIS_BOOLEAN 
COL_RemoveElementAt( COLLECTION *Col, UINTN index)
{
    UINTN moves;

    //Validate index and bail if out of range.
    if ( index >= Col->currentCount) return BIS_FALSE;
    
    //Adjust count
    Col->currentCount -= 1;
    
    //Return if now empty.
    if (Col->currentCount == 0) return BIS_TRUE;

    //Compute number elements to move down
    moves= Col->currentCount  - index;

    //Move them.
    for ( ; moves > 0; --moves)
    {
        Col->theElements[index]= Col->theElements[index+1];
        ++index;
    }

    return BIS_TRUE;


}


    //-------------------------------------------------------------------
    //remove an element (specified by it's value) from the collection.
    //cast your element type into a UINTN.
    //returns true if succeess, false if element not present.

    // This op invalidates indexes for elements pass the
    // one removed.
    // Their index value becomes one less.

BIS_BOOLEAN 
COL_RemoveElement( COLLECTION *Col, UINTN element)
{
    UINTN i;
    BIS_BOOLEAN rv= BIS_FALSE;

    //Find index of the element.
    i= COL_IndexOf( Col, element );
    if ( i != COL_ERROR )
    {
        //remove it by index.
        rv= COL_RemoveElementAt( Col, i);
    }

    return rv;
}



    //-------------------------------------------------------------------
    // return the current number of elements in the collection.
    // valid indexes will be 0 .. ( COL_GetCount() - 1 )

UINTN 
COL_GetCount( COLLECTION *Col )
{
    return Col->currentCount;
}


//-------------------------------------------------------------------
    //Find the given element and return it's index.
    //cast your element type into a UINTN.
    //returns index if succeess
    // COL_ERROR (0xffffffff) if not found.

UINTN 
COL_IndexOf( COLLECTION *Col, UINTN element )
{
    INTN i;
    
    for ( i= Col->currentCount-1; i>=0; --i )
    {
        Col->compareCount += 1;    
        if ( Col->theElements[i] == element ) return i;    
    }
    
    return COL_ERROR;
}





    //
    // main() - interaction console test for win32 environ.
    //

#ifdef _STANDALONE_TEST
COLLECTION *col;

int dumpit();

int main( )
{
    int cap, inc;
    char cmd, extra;
    BIS_BOOLEAN tf;


    printf("capacity? "); scanf("%d", &cap);
    printf("capIncre? "); scanf("%d%c", &inc, &extra);

    col= COL_New( cap, inc);
    if (col==NULL){
        printf("new failed\n"); return 1;
    }
    dumpit();


    printf("a)dd r)emoveByIx R)emoveByValue q)uit "); 
    scanf("%c%c", &cmd, &extra);

    while (cmd!='q')
    {
        switch(cmd)
        {
        case 'a':
                printf("addvalue? "); scanf("%d%c", &inc, &extra);
                tf=COL_AddElement(col, inc);
                if (!tf){printf("ERROR in AddElement\n"); }
                dumpit();
                break;
            
        case 'r':
                printf("removeElementAt(Index)? "); scanf("%d%c", &inc, &extra);
                tf=COL_RemoveElementAt(col, inc);
                if (!tf){printf("ERROR in RemoveElementAt\n"); }
                dumpit();
                break;

        case 'R':
                printf("removeElement(by value)? "); scanf("%d%c", &inc, &extra);
                tf=COL_RemoveElement(col, inc);
                if (!tf){printf("ERROR in RemoveElement\n"); }
                dumpit();
                break;

        default:
            printf("what?\n");
        }

        printf("a)dd r)emoveByIx R)emoveByValue q)uit "); 
        scanf("%c%c", &cmd, &extra);
    }




    COL_Destroy(col);
}


dumpit()
{
    int i,k;
    UINTN el;
    BIS_BOOLEAN tf;

    printf("Elements=%d Capacity=%d ", k= COL_GetCount(col), col->capacity );

    for (i=0; i<k; ++i)
    {
        tf= COL_ElementAt( col, i, &el);
        if (!tf){printf("ERROR in dumpit\n"); }
        printf("[%d]=%d ", i, el );
    }
    printf("\n");
    return 0;
}



#endif
