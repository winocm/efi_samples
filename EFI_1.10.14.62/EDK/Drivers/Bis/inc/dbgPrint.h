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

//************************************************************************************************//
//
// Description:
//  Macros and low level functions for displaying trace data.
//
//************************************************************************************************//

#ifndef _DBGPRINT_H_
#define _DBGPRINT_H_



//Set this DO_BISDBGPRINT to 0 will suppress code generation by PUT_x macros
// and suppress compilation of dbgPrint.c function bodies.

//If code IS generated, output can be toggled by varying the value of
//the 'doOutputGen' to BISInitDbgPrint().
#define DO_BISDBGPRINT 1

//for compatiblity with older bis code
#if (DO_BISDBGPRINT==1)
#define DO_COMMPORT_IO 1
#endif

    //output macros: PUT_xyz
    //In xyz, x= S for string or may be omitted.
    //        y= X for HEX, D for decimal or may be omitted.
    //        z= N for newline or may be omitted.

#if (DO_BISDBGPRINT==1)

    //Put a string, decimal or hex value, or a newline.
#define PUT_S(s) BISPutData(s,0,0)
#define PUT_D(v) BISPutData(BIS_NULL,v,10)
#define PUT_X(v) BISPutData(BIS_NULL,v,16)
#define PUT_N()  BISPutData(BIS_NULL,0,CRLF)

    //Put a string, decimal or hex value, and a newline.
#define PUT_SN(s) BISPutData(s,0,CRLF)
#define PUT_DN(v) BISPutData(BIS_NULL,v,10+CRLF)
#define PUT_XN(v) BISPutData(BIS_NULL,v,16+CRLF)

    //Put a string followed by ...
    //a string, decimal or hex value, or a newline.
#define PUT_SS(s,s2)  {BISPutData(s,0,0); BISPutData(s2,0,0);}
#define PUT_SD(s,v)   BISPutData(s,v,10)
#define PUT_SX(s,v)   BISPutData(s,v,16)

    //Put a string followed by ...
    //a string, decimal or hex value, and newline.
#define PUT_SSN(s,s2)  {BISPutData(s,0,0); BISPutData(s2,0,CRLF);}
#define PUT_SDN(s,v)   BISPutData(s,v,10+CRLF)
#define PUT_SXN(s,v)   BISPutData(s,v,16+CRLF)

    //Put out a trace line.
#define TRACEOUT()     PUT_SDN(__FILE__ " ", __LINE__)

    //Announce heap corruption (if true).
#define CHECKHEAP()    isHeapOK(__FILE__ " ", __LINE__)

    //Dump all heap stats.
#define DUMPHEAPSTATS(label,val) dumpHeapStats(label,val)
#else
// NOP version of macros
#define PUT_S(s)
#define PUT_D(v)
#define PUT_X(v)
#define PUT_N()
#define PUT_SN(s)
#define PUT_DN(v)
#define PUT_XN(v)
#define PUT_SS(s,s2)
#define PUT_SD(s,v)
#define PUT_SX(s,v)
#define PUT_SSN(s,s2)
#define PUT_SDN(s,v)
#define PUT_SXN(s,v)
#define TRACEOUT()
#define DUMPHEAPSTATS(label,val)
#define CHECKHEAP()
#endif



UINT32 BISInitDbgPrint( UINT32 port, BIS_BOOLEAN doOutputGen );
UINT32 BISPutChar( UINT8 dataByte);
UINT32 BISPutString( UINT8* dataByte);
void   BISPutUINT32( UINT32 data );
void   BISPutHex( UINT32 data );
void   BISPutData( UINT8 *aString, UINT32 data, UINT32 flags);
void   BISPutBISDataSummary( UINT8 *aLabel, BIS_DATA_PTR bd );
void   BISPutBISData( UINT8 *aLabel, BIS_DATA_PTR bd, BIS_BOOLEAN fancy );

    //For aString: NULL means no string output.
    //For flags use: <base>+CRLF
    //<base> is 10,16 or 0 (no data output).
    //CRLF is optional

#define CRLF 0x80             //CarriageReturnLineFeed flag.


#endif


// History:
// this file cloned from smbios bis file ...
//   Archive: /SMA/Src/inc/commdbg.h
//   Revision: 12
//   Date: 12/18/98 5:45p
