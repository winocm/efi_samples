/*-----------------------------------------------------------------------------
 *    File:   utc_time.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------------
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
 * This file verifies that an input string is either a UTC time or
 * Generalized time string.
 */
    
#include "x_fndefs.h"

/*-----------------------------------------------------------------------------
 * Name: cssm_date_atoi
 *
 * Description:
 * Converts a 2 character string into an integer
 *
 * Parameters: 
 * a (input) : The character string to convert
 *
 * Return value:
 * sint32 with the integer equivalent of the input string 
 * -1 if any characters are not digits
 *
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
sint32 cssm_date_atoi(char *a) 
{
    char *b = a;
    b++;

    if (*a < 0x30 || *a > 0x39) return -1;
    if (*b < 0x30 || *b > 0x39) return -1;
    
    return (10*(*a - 0x30) + (*b - 0x30));
}


/*-----------------------------------------------------------------------------
 * Name: cl_UtcToCssmTime
 *
 * Description:
 * Converts a UTC Time string, "YYMMDDhhmmss", to a 
 * CSSM_DATE_AND_TIME structure, where years, months, days, 
 * hours, minutes, and seconds are represented as integers.
 *
 * NOTE: The time is not corrected for timezone.
 * NOTE: The century is not filled in because it is obtained differently
 *       for UTC times and Generalized times.
 * 
 * Parameters: 
 * UTCTime  (input)  : UTC time string to be converted
 * CssmTime (output) : CSSM_DATE_AND_TIME structure to be filled in
 *
 * Return value:
 * None
 *
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
void cl_UtcToCssmTime( const uint8 *UTCTime, CSSM_DATE_AND_TIME_PTR CssmTime)
{
    char temp[3];
    sint32 *x, i;
  
    temp[2] = 0; 
    x = &CssmTime->Year;

    for (i=0; i < 12; x++)
    {
        temp[0] = UTCTime[i++]; 
        temp[1] = UTCTime[i++]; 
        *x = cssm_date_atoi(temp);
    }
}


#ifdef CSSM_BIS
/*-----------------------------------------------------------------------------
 * Name: cl_IsBadCommonTime
 *
 * Description:
 * Checks if the input string is a valid time string.
 * 
 * Parameters: 
 * Time    (input) : time string to be checked
 * Length  (input) : The length of the Time string
 *
 * Return value:
 * CSSM_TRUE  : time string is invalid
 * CSSM_FALSE : time string is valid
 *
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadCommonTime (const uint8* UTCTime, sint32 Length)
{
    CSSM_DATE_AND_TIME cssm_time;
    sint32 days_per_mo[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    char temp[3];
    sint32 century;
    sint32 febDays= 28;

    /* Check for Null, invalid length, and improper ending */
    if( UTCTime == NULL ) {
        goto ERROR_EXIT;       
    }
    else if ( Length == 13 && UTCTime[12] != 'Z'){  // 2 digit year format.
        goto ERROR_EXIT;       
    }
    else if ( Length == 15 && UTCTime[14] != 'Z'){  // 4 digit year format.
        goto ERROR_EXIT;       
    }
 
   
   
    /* Translate YYMMDDHHMMSS to integers */
    if ( Length == 13 )
    {
        cl_UtcToCssmTime(UTCTime, &cssm_time);
        if (cssm_time.Year   < 0 ){
            goto ERROR_EXIT;
        }
        
        //do the century heuristic
        if ( cssm_time.Year < 50 ) {
            cssm_time.Year += 2000;
        }
        else {
            cssm_time.Year += 1900;
        }        
        
    }
    
    /* Translate YYYYMMDDHHMMSS to integers */
    else 
    {
        cl_UtcToCssmTime(UTCTime+2, &cssm_time);

        /* Convert Century */
        temp[0] = UTCTime[0]; 
        temp[1] = UTCTime[1]; 
        temp[2] = 0; 
        century=  cssm_date_atoi(temp);
        if ( century == -1) 
        {
            goto ERROR_EXIT;       
        }
        cssm_time.Year += (century*100);
            
    }    
    

    /* Check for some invalid digits in YYMMDDHHMMSS */
    if (cssm_time.Month  < 1 || cssm_time.Month  > 12 ||
        cssm_time.Day    < 1 || 
        cssm_time.Hour   < 0 || cssm_time.Hour   > 23 ||
        cssm_time.Minute < 0 || cssm_time.Minute > 59 ||
        cssm_time.Second < 0 || cssm_time.Second > 59 )
    {
            goto ERROR_EXIT;       
    }
    
    
    // Filter out unreasonable dates.
    if ( cssm_time.Year < 1900 ){
        goto ERROR_EXIT;
    }
    
    //Decide if it's a leap year.
    if ( cssm_time.Year % 4   == 0 ){ febDays= 29; }
    if ( cssm_time.Year % 100 == 0 ){ febDays= 28; }
    if ( cssm_time.Year % 400 == 0 ){ febDays= 29; }
    days_per_mo[1]= febDays;
    
    // Check number of days in month 
    if (cssm_time.Day > days_per_mo[cssm_time.Month-1])
    {
        goto ERROR_EXIT;
    }
    
    return CSSM_FALSE;
    
ERROR_EXIT:
    return CSSM_TRUE; //is bad
}


/*-----------------------------------------------------------------------------
 * Name: cl_IsBadUtc
 *
 * Description:
 * Checks if the input string is a valid UTC time string.
 * 
 * Parameters: 
 * UTCTime (input) : UTC time string to be checked
 * Length  (input) : The length of the UTCTime string
 *
 * Return value:
 * CSSM_TRUE  : UTC string is invalid
 * CSSM_FALSE : UTC string is valid
 *
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadUtc (const uint8* UTCTime, sint32 Length)
{
    if ((Length != 13) || cl_IsBadCommonTime( UTCTime, 13) ){
        return CSSM_TRUE;
    }
    
    return CSSM_FALSE;
}

/*-----------------------------------------------------------------------------
 * Name: cl_IsBadGeneralizedTime
 *
 * Description:
 * Checks if the input string is a valid Generalized time string.
 * 
 * Parameters: 
 * GTime  (input) : Generalized time string to be checked
 * Length (input) : The length of the GTime string
 *
 * Return value:
 * CSSM_TRUE  : Generalized time string is invalid
 * CSSM_FALSE : Generalized time string is valid
 *
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadGeneralizedTime (const uint8* GTime, sint32 Length)
{
     if ((Length != 15) || cl_IsBadCommonTime( GTime, 15) ){
         return CSSM_TRUE;
     }
     
    return CSSM_FALSE;
}

#else /* #ifdef CSSM_BIS */

/********************************************************************** 
 *
 * cl_UtcToTime
 *    Converts null-terminated UTC Time string, "YYMMDDhhmmssZ", to a 
 *     time_t (== long == sint32) structure.
 *
 * Inputs:
 *    char* szTime        - null-terminated UTC time string to be 
 *                          converted
 *
 * Returns:
 *    sint32 structure containing data from UTC time string on success
 *    -1 on failure
 *  
 * Error Codes:
 *    none
 *
 * Side Effects:
 *    No known side effects.
 *
 ***********************************************************************/ 
sint32 cl_UtcToTime( const char *szTime, sint32 length )
{
  CSSM_DATE_AND_TIME newtime;

  /* check for Null, invalid length, and improper ending */
  if( szTime == NULL || length < 13 || szTime[12] != 'Z')
  {
    return -1;
  }
  /* check for isdigit on YYMMDDHHMMSS */
  cl_UtcToCssmTime(szTime, &newtime);

    /* Account for year 2000 rollover */
    /* Dates less than 50 are year 2000 by PKIX standard */
    if( newtime.Year < 50 ) newtime.Century = 20;
    else newtime.Century = 19;

  /* Check for valid digits for CCYYMMDDHHMMSS */
  return( cssm_mktime( &newtime ) );
}
 

/********************************************************************** 
 *
 * cl_GeneralizedTimeToTime
 *    Converts null-terminated generalized time string, 
 *      "YYYYMMDDhhmmssZ", to a time_t (== long == sint32) structure.
 *
 * Inputs:
 *    char* szTime        - null-terminated generalized time string 
 *                          to be converted
 *
 * Returns:
 *    sint32 structure containing data from UTC time string on success
 *    -1 on failure
 *  
 * Error Codes:
 *    none
 *
 * Side Effects:
 *    No known side effects.
 *
 ***********************************************************************/ 
sint32 cl_GeneralizedTimeToTime( const char *szTime, sint32 length )
{
  CSSM_DATE_AND_TIME newtime;
  char szTemp[3];

  /* check for Null, invalid length, and improper ending */
  if( szTime == NULL || length < 15 || szTime[14] != 'Z')
  {
      return -1;
  }
  /* check for isdigit on YYMMDDHHMMSS */
  cl_UtcToCssmTime(&szTime[2], &newtime);

  /* check for isdigit on CC */
  szTemp[0] = szTime[0]; 
  szTemp[1] = szTime[1]; 
  szTemp[2] = 0; 
  newtime.Century = cssm_date_atoi( szTemp );

  /* Check for valid digits for CCYYMMDDHHMMSS */
  return( cssm_mktime( &newtime ) );
}


CSSM_BOOL cl_IsBadUtc (const uint8* szTime, sint32 length)
{
    if (cl_UtcToTime((const char *) szTime, length) < 0) return CSSM_TRUE;
    else return CSSM_FALSE;
}
CSSM_BOOL cl_IsBadGeneralizedTime (const uint8* szGTime, sint32 length)
{
    if (cl_GeneralizedTimeToTime((const char *) szGTime, length) < 0) return CSSM_TRUE;
    else return CSSM_FALSE;
}

#endif /* #ifdef CSSM_BIS */
     
     
