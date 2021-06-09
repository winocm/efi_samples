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

//************************************************************************************************//
// stringFxns.c
//
// Contains local implementation of common string functions.
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#include <bis_priv.h>

    //
    //canonical STRCMP
    //

//NOT USED
#if 0
int BIS_strcmp( UINT8 *s1, UINT8 *s2 )
{
    while ( *s1 && *s2 && (*s1 == *s2) )
    {
        ++s1; ++s2;
    }
    
    return *s1-*s2; 
}
#endif

    //        
    //canonical STRNCMP, 
    //
int BIS_strncmp( UINT8 *s1, UINT8 *s2, UINT32 count)
{
    while ( *s1 && *s2 && (*s1 == *s2) && (count > 0))
    {
        ++s1; ++s2; --count;
    }
    
    if (count==0) return 0;
    return *s1-*s2; 
}


    //        
    // canonical STRLEN 
    //
int BIS_strlen( UINT8 *s1 )
{
    int count= 0;

    while ( s1 && *s1 )
    {
        ++s1; ++count;
    }
    
    return count; 
}


//eof
