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
//


//************************************************************************************************//
//
// Description: debug Print functions
//
// See dbgPrint.h for actual macros to use.
//
// these low level functions allow primative output  console
//
// PREPROCESSOR SYMBOLS
//
//  DO_BISDBGPRINT == 1   enable compilation of print code in this module
//
//************************************************************************************************//



#include <bisBaseCode.h>

//This macro defines the function that knows how to write a character
//to the console (like putc in stdio).
#define PUTKHAR consolePutChar

static BIS_BOOLEAN genOutput= BIS_FALSE;

#if 0

//;----------------------------------------------------------------------------
//;
//;----------------------------------------------------------------------------
UINT32 BISInitDbgPrint(  UINT32 port, BIS_BOOLEAN doOutputGen  )
{
	genOutput= doOutputGen;

    #if (DO_BISDBGPRINT==1)
	if (genOutput){
		PUT_SN( "BISInitDbgPrint - OUTPUT ENABLED");
	}
	else {
		PUT_SN( "BISInitDbgPrint - OUTPUT DISABLED");
	}
	#endif

    return 0;
}

//----------------------------------------------------------------------------
//  consolePutChar - output a character to a dos box console.
//----------------------------------------------------------------------------
UINT32 consolePutChar( UINT8 dataByte )
{
	UINT32 rc=0;
	CHAR16 ucode[4];

	if (!genOutput){
		return 0;
	}


	//Convert input char to unicode string
	ucode[0]= dataByte;
	ucode[1]= 0;
	ucode[2]= 0;
	ucode[3]= 0;
//Do not print!	Print(ucode);

    return rc;

}



//--------------------------------------------------------
// BISPutString - put a null terminated string out to the
//      com port.
//--------------------------------------------------------
UINT32 BISPutString( UINT8* dataByte)
{
    UINT32 rc= 0;

    #if (DO_BISDBGPRINT==1)

	if (!genOutput){
		return 0;
	}

    while (*dataByte)   //not null
    {
	    rc= PUTKHAR( *dataByte );
	    ++dataByte;
	    if (rc!=0)break;
    }
    #endif


    return rc;

}


//--------------------------------------------------------
// BISPutHex - put a 32 bit value out to the com port as hex.
//--------------------------------------------------------
void
BISPutHex( UINT32 hexvalue )
{
    #if (DO_BISDBGPRINT==1)

    __int32 nibble, i;
    char    hstr[10];
    static  char hdigit[]="0123456789ABCDEF";

    if (!genOutput){
		return;
	}

    for (i=7; i>=0; --i)
    {
        nibble= hexvalue & 0xf;
        hstr[i]= hdigit[nibble];
        hexvalue= hexvalue >> 4;
    }

    hstr[8]= 0;
    BISPutString( hstr );

    #endif
}



//--------------------------------------------------------------
// BISPutHexByte - put a 8bit value out to the com port as hex.
//--------------------------------------------------------------
void
BISPutHexByte( UINT8 hexByte )
{
    #if (DO_BISDBGPRINT==1)

    __int32 nibble;
    char    hstr[4];
    static  char hdigit[]="0123456789ABCDEF";

	if (!genOutput){
		return;
	}
    hexByte= hexByte & 0xff;

    hstr[2]= 0;
    nibble= hexByte & 0xf;
    hstr[1]= hdigit[nibble];
    nibble= (hexByte >> 4) & 0xf;
    hstr[0]= hdigit[nibble];

    BISPutString( hstr );

    #endif
}




//--------------------------------------------------------
// BISPutUINT32 - put a UNSIGNED 32 BIT int out to the
//  commport.
//--------------------------------------------------------

void BISPutUINT32( UINT32 value)
{
    #if (DO_BISDBGPRINT==1)

    UINT32 d= 1000000000;  //largest place value in 32 bit number.
    UINT32 c, i, j=0;

	if (!genOutput){
		return;
	}

    for (i=0; i<10; ++i)
    {
        //Suppress leading zeros.
        if (value < d && j == 0)
        {
            d= d/10;    //next place value
            continue;
        }

        //Compute digit value
        c= value/d;
        PUTKHAR( (UINT8)('0'+c) );
        ++j;

        //Subtract c from value.
        value= value - (c*d);

        //Next place value.
        d= d/10;
    }

    //Special handling for 0. Zero supression results in blank output.
    if ( j==0){
        PUTKHAR('0');
    }


    #endif
}

//-----------------------------------------------------
// BISPutData - write a string (optional), a 32 data item
//      (optional) as hex or decimal, and an optional newline
//      out to the commport.
//
//      Pass a NULL pointer to omit the 1st parm.
//
//      Flags: sets the base 10 or 16 and determines if a
//          newline is sent and whether the 2nd parm is printed.
//
//  example: BISPutData( stringData, 123, 10+CRLF);
//              /*writes a string and a number in base 10 w/crlf */
//
//  example: BISPutData( NULL, 123, 16);
//              /*writes a  number in base 16 w/o crlf */
//
//  example: BISPutData( stringData, 123, CRLF);
//              /*writes a string and crlf */
//              /*The number is not output 'cause flags does not specify
//              /*10 or 16 for the base*/
//
//  example: BISPutData( NULL, 0, CRLF);
//              /*writes a crlf */
//
//

//
void BISPutData( UINT8 *aString, UINT32 aValue, UINT32 flags )
{
    #if (DO_BISDBGPRINT==1)

    UINT32 base;

	if (!genOutput){
		return;
	}

    if (aString!=BIS_NULL){BISPutString(aString);}

    base= flags & 0x1f;

    if (base == 10){ BISPutUINT32(aValue); }
    else
    if (base == 16){ BISPutHex(aValue);    }
    else
       { ;/*nop*/ }  //no supported base specified so no number is output

    if (flags&CRLF){ BISPutString("\r\n"); }
    #endif
}


//
// BISPutBISDataSummary - prints a label, and attribs of a bis data
//      including 1st two and last two bytes.
//

void BISPutBISDataSummary( UINT8 *aLabel, BIS_DATA_PTR bd )
{
    #if (DO_BISDBGPRINT==1)
    UINT8 *b;   //begin data
    UINT8 *e;   //end   data
    UINT32 l;   //length of data

    if (!genOutput){
		return;
	}

    PUT_N();
    PUT_SX("BIS_DATA@ 0x", (UINT32)bd);

    b= bd->data;
    l= bd->length;
    e= b+l;

    PUT_SS( " ", aLabel);
    PUT_SD( ".len=", l);
    PUT_SX( " .dat=", (UINT32)b);

    PUT_S( "value=0x");
    BISPutHexByte( *b );
    BISPutHexByte( *(b+1) );

    PUT_S( "..");
    BISPutHexByte( *(e-2) );
    BISPutHexByte( *(e-1) );
    PUT_N();

    #endif

}


//
// BISPutBISDataSummary - prints a label, and attribs of a bis data
//      including and then dumps the entire data area to com1.
//

void   BISPutBISData( UINT8 *aLabel, BIS_DATA_PTR bd, BIS_BOOLEAN fancy )
{
    #if (DO_BISDBGPRINT==1)
    UINT8 *b;   //begin data
    UINT8 *e;   //end   data
    UINT8 *b2;  //current output byte.

    UINT32 l;   //length of data
    UINT32 i,cnt; //dumpcount

	if (!genOutput){
		return;
	}

    PUT_N();
    PUT_SX("BIS_DATA@ 0x", (UINT32)bd);

    b= bd->data;
    l= bd->length;
    e= b+l;

    PUT_SS( " ", aLabel);
    PUT_SD( ".len=", l);
    PUT_SXN( " .dat=", (UINT32)b);

    cnt= 0;
    while ( cnt < l)
    {
        //Put out address if doing fancy display.
        if (fancy && cnt%16==0)
        {
            PUT_X((UINT32)b);
        }

        //Put out 16 hex bytes, upto final byte
        b2= b;
        for ( i=0; i<16 && b2 < e; ++i)
        {
            PUTKHAR(' ');
            BISPutHexByte( *b2 );
            ++b2;
        }


        //Put out 16 character bytes, upto final byte
        if (fancy)
        {
            PUT_S("  ");
            b2= b;
            for ( i=0; i<16 && b2 < e; ++i)
            {
                //translate non-printing to period '.'
                if (*b2 < ' ' || *b2 >= 0x7f ){ PUTKHAR('.');   }
                else                          { PUTKHAR( *b2 ); }

                ++b2;
            }
        }

        cnt= cnt + 16;      //update outer loop control.
        b=   b + 16;        //update 16 byte block pointer.

        PUT_N();

    }


    #endif
}
#endif

//void pause(char *msg)
//{
//	Print(L"%a\n", msg);
//	WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
//}


// eof
