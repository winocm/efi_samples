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

error error error 
// This FILE IS OBSOLETE. Its contents are now part of bis32wireformat.
//perhaps you should be including that file.

//************************************************************************************************//
// inttest.H
//
// Description:
//
// This a private header file used by BIS internal dispatch function and the implementation
// of the public API.
//
// All functions return 0 on success or a diagnostic code for interal debug.
// This is for internal test and debug and not visible to the calling application.
//
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//

#if 0
#ifndef _INTTEST_H_
#define _INTTEST_H_





            //Private Bis Opcode
#define BISOP_GetInternalTestVector                   (BISOP_LAST+1)




    //----------------------------------------//
    //BIS API Internal FUNCTIONS Declarations //
    //----------------------------------------//


    //********************************************//
    //  BIS_InternalTest  parameter structure.    //
    //  function code= BISOP_InternalTest         //
    //     Field definitions follow structure def.//
    //********************************************//
#ifdef RESPONDER_APP
typedef UINT32		  BIS32_APP_HANDLE;
#endif

typedef
struct  BIS_InternalTest_PARMS
{
    UINT32                  sizeOfStruct;
    UINT32                  returnValue;        //[OUT] - value read or BIS_OK.
#ifdef RESPONDER_APP
    BIS32_APP_HANDLE        appHandle;
#else
    BIS_APPLICATION_HANDLE  appHandle;          //[in]  From BIS_Initialize( ).
#endif
    UINT32                  actionCode;         //the action to perform.
    UINT32                  flag_reg_mem_ID;    //flagword, reg or mem loc to operate on.
    UINT32                  flag_reg_mem_value; //flag, reg or mem value to write.

    union
    {
    	UINT32 placeholder1;
    }
    variableFormat;
}
BIS_INTERNALTEST_PARMS,
*BIS_INTERNALTEST_PARMS_PTR;

UINT32
    BIS_GetInternalTestVector( BIS_INTERNALTEST_PARMS *parmBlock );


//Special return code - returnValue is used for output data on
//   "GET" actions, so this value is used to indicate a
//   bad request since it is distinguishable.
#define INTRNL_TEST_ERROR 0xdeadd0d0

//Internal Test Action codes:
#define SET_FLAG_BITS (1)   //set bits in a flagword.
#define CLR_FLAG_BITS (2)   //clear bits ...
#define GET_FLAG_BITS (3)   //Get bit(s)
#define SET_REGISTER  (4)   //set a register value.
#define GET_REGISTER  (5)   //get a ...
#define SET_MEM       (6)   //set a memory value.
#define GET_MEM       (7)   //get a ...
#define GETMEMSTATS   (8)   //get heap manager stats.

#define AUTHINIT     (9)   //Set a different auth function and
                            // passback value from 'flag_reg_mem_ID'
                            // and 'flag_reg_mem_value' fields.

#define SETAUTHIO    (10)   //Inform bis of seg:off address of
                            //bios gets and puts funcionts in
                            //     'flag_reg_mem_ID'
                            // and 'flag_reg_mem_value' fields.

#define CALLAUTHFXN  (11)   //Set a different auth function and
                            // passback value from 'flag_reg_mem_ID'
                            // and 'flag_reg_mem_value' fields.

//Actions codes that represent control signals that
// are handle by the RESPONDER, without calling BIS.

#define RESPONDER_TERMINATE (1000) //responder should terminate
                                   //after next SHUTDOWN/disconnect
                                   // is received.

/////////////////////////////////////////////////////////
//Flagword indexes (32 flag bits/word)                 //
//  Used for flag_reg_mem_ID value when actionCode is  //
//  one of  SET_FLAG_BITS  CLR_FLAG_BITS GET_FLAG_BITS //
/////////////////////////////////////////////////////////
#define MMFLAGS       (0)     //memory mgr parms
#define TRACEFLAGS    (1)     //module trace controls
#define BEHAVFLAGS    (2)     //flags that modify runtime behavior

/////////////////////////////////////////////////////////
//Register indexes (32 flag bits/word)                 //
//  Used for flag_reg_mem_ID value when actionCode is  //
//  one of  SET_REGISTER or  GET_REGISTER              //
/////////////////////////////////////////////////////////
#define MALLOC_PADDING (0)    //#bytes to pad malloc requests by.

    //default reg value for MALLOC_PADDING register can be overrided from
    // command line.
    #ifndef PAD_MALLOC
    #define PAD_MALLOC (0)
    #endif

    //default value for free heap. this is the amount reported if
    //MM_TRACK_FREEHEAP is not set.
    #define AVAIL_HEAP  0x7FFFFFFF

/////////////////////////////////////////////////////////////
// Bit values used with Flags words: MMFLAGS, TRACEFLAGS,  //
// BEHAVFLAGS, etc                                         //
/////////////////////////////////////////////////////////////

//Bit values for MMFLAGS flag word.
#define MM_DISP_HSTAT     (0x01)   //dump heap stats in dispatch.
#define MM_REPORT_MALLOC  (0x02)   //report malloc and free activity.
#define MM_COLL_EXPAND    (0x04)   //log memory collection expanded.
#define MM_CSSM_FUNCTS    (0x08)   //log cssm mem fxn wrappers.
#define MM_ZERO_FLASH     (0x10)  //clear flash area on next shutdown.
#define MM_TRACK_FREEHEAP (0X20)  //compute total free heap on alloc/free calls.

//Bit values for TRACEFLAGS flag word.
#define TRACE_UBOA        (0x01) //trace UBOA function
#define TRACE_PREPSM      (0x02) //trace PrepareSignedManifest
#define DUMP_PREPSM_PARMS (0x04) //dump credential ect to com port.

//Bit values for BEHAVFLAGS flag word.
#define BEHAV_ALLOW_ANY_UPDATE (0x1) //Allow Update to succeed without
                                     //regard to the identity of the
                                     //signer or whether the token matches.


        /////////////////////////////////////////////////
        //These elements used by INTTEST.C to define globals used
        //for flagswords.
        //This array indexed by MMPARMS, TRACECTL, BEHAVFLAGS, ...
#define NBR_FLAGWORDS 3
extern  UINT32 dbgFlagWords[NBR_FLAGWORDS];

        //inttest.c uses this macro to init the dbgFlagWords.
#define _INTTEST_H_FLAG_INITIALIZERS { 0x00 , 0x00 , 0x00 };


        /////////////////////////////////////////////////
        //These elements used by INTTEST.C to define globals used
        //for DBGREGS.
        //This array indexed by MALLOC_PADDING, ...
#define NBR_DBGREGS 2
extern  UINT32 dbgRegs[NBR_DBGREGS];

        //inttest.c uses this symbol to init the dbgFlagWords.
#define _INTTEST_H_REG_INITIALIZERS { PAD_MALLOC, 0}


    //Macro to test a flag bit in given flag word.
    //Usage: if ( BIS_FLAG(MM,MM_DISP_HSTAT) ) { ... }
#define BIS_FLAG(fword,bit) (dbgFlagWords[fword]&bit)
    //Macros to set or clear a bit.
#define SET_BIS_FLAG(fword,bit) dbgFlagWords[fword]=(dbgFlagWords[fword]|bit)
#define CLR_BIS_FLAG(fword,bit) dbgFlagWords[fword]=(dbgFlagWords[fword]&(~bit))


//History: clones from smbios bis file:
// Workfile: inttest.h
// Revision: 18

#endif
#endif
