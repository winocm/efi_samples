
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


//************************************************************************************************//
// BPCTYPES.H
//
// Description:
//
//
/*Placeholder_for_source_control_strings*/
//************************************************************************************************//


#ifndef _BPCTYPES_H_
#define _BPCTYPES_H_


            //------------------//
            //  simple types    //
            //------------------//

typedef     UINT16         BPC_HANDLE;
#define     BPC_BAD_HANDLE 0xFFFF

typedef     UINT16         BPC_STATUS;



        //-----------------------//
        // PACKET HEADER         //
        //-----------------------//

typedef struct _BPC_PACKET_HDR
{
    UINT8   soh;         //ascii start of header character.
    UINT8   flags;       //PACKET_TYPE and RESERVED1
    UINT16  pktSeqNbr;   //Packet Sequence number;
    UINT16  dataLength;  //logical length of the user data that follows.
    UINT16  fragInfo;     // (fragmentNbr<<8)+(totalFragments)
}
BPC_PACKET_HDR;



        //
        //  Packet types and access macros
        //

#define GET_PKT_TYPE(PktHdrPtr) ((PktHdrPtr)->flags&PACKET_TYPE_MASK)

#define SET_PKT_TYPE(PktHdrPtr,pktType) (PktHdrPtr)->flags= (~PACKET_TYPE_MASK & (PktHdrPtr)->flags) | (PACKET_TYPE_MASK  & pktType)

        //
        //  Packet types
        //
#define PACKET_TYPE_MASK 0xf

#define BPC_REQUEST    (1)       //a request to responder.
#define BPC_REPLY      (2)       //reply to request from responder.
#define BPC_TRACEDATA  (3)       //trace data from BIS.
#define BPC_DISCONNECT (4)       //request disconnect from responder.
#define BPC_CONNECT    (5)       //request connect    to   responder.
#define BPC_CONNECT_COMPLETE (6) //connect request succeeded.
#define BPC_DISCONNECT_ACK   (7) //disconnect request succeeded.
#define BPC_PACKET_FRAGMENT  (8) //a part of a fragmented packet.
#define BPC_ECHOPACKET       (10) //request the responder to echo the packet.


         //
         // Packet Structural Parms (Used by UDPDLL and UDPRSPND not BPC)
         //

#define  MAXDATAGRAMSIZE_DFLT  1400  //Max remoteBIS packet size in bytes.
                                     //The BIS remote transport will fragement and
                                     //reassemble anything larger than this.

                                     //Size of ethernet/token ring, IP, and UDP headers
                                     //are *NOT* included in this size.



                                     //The maximum number of fragments that may be
                                     //created. The default is set so that a reassembled
                                     //packet will fit in a 65K buffer.
#define  MAXFRAGCOUNT_DFLT     (65535/MAXDATAGRAMSIZE_DFLT)



        //----------------------------------
        //Connect/listen parm structures
        //-----------------------------------


        // Udp Com connection parm Union member.

#define UDP_CP_FORMAT   2
#define BIS01_XPORT     "//BIS01/"

typedef struct _BPC_UDP_CP
{
    UINT8   transportName[12];     /*  "//BIS01/" ==  BIS01_XPORT */
    UINT8   hostAdrPort[22];       /*  s.s.s.s:port */
    UINT16  alignTo32Bits;


    // TIMEOUT AND RETRY COUNTS. TIMEOUTS IN milliseconds.
    // SET ANY OR ALL VALUES TO 0 TO USE INTERNAL DEFAULTS.
    UINT32  responseTimeout;   //ms to wait for response from target.

    UINT32  retryCount;        //#times to retry timed-out or failed connect,
                               //receive, send, disconnect.

    UINT32  seqErrRetryCount;     //#times to send/rcv to recover from
                                  // sequence errors

}
BPC_UDP_CP;
        //
        //Generic connect parm struct
        //

typedef struct _BPC_CONNECT_PARMS
{
    UINT16  parmStructFormat;       //SERIAL_CP_FORMAT etc
    UINT16 alignTo32Bits;

    union
    {
        BPC_UDP_CP    udpCP;        //datagram tranport.
    }
    formats;

}
BPC_CONNECT_PARMS;



        //-----------------------------//
        // Function return error codes //
        //-----------------------------//
#define BPC_OK             (0)
#define BPC_BAD_CPARMS     (1)
#define BPC_CONNECT_ERR    (2)
#define BPC_GETSTATE_ERR   (3)
#define BPC_SETPARM_ERR    (4)
#define BPC_NOHANDLES      (5)      //the internal pool of handles is exhausted.
#define BPC_READ_FAILED    (6)
#define BPC_WRITE_FAILED   (7)
#define BPC_BUF_TOO_LARGE  (8)
#define BPC_PKT_TOO_LARGE  (9)
#define BPC_CHKSUM_ERR     (10)
#define BPC_PKTLENGTH_ERR  (11)
#define BPC_NOCOMSIGNAL    (12)
#define BPC_CREATEVENT_ERR (13)
#define BPC_TRANS_ERR      (14)     //tranmission error.
#define BPC_CANCEL         (15)     //external cancel signal received.
#define BPC_TIMEOUT        (16)     //timeout expired.
#define BPC_MEMERR         (17)     //memory allocation failed.
#define BPC_NOTIMPLEMENTED (18)     //a requested operation is not implemented.
#define BPC_IOCTL_FAILURE  (19)     //requested ioctl could not be completed.

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

    //
    //  popular constants
    //
#define ASCII_SOH 0x01
#define ASCII_ESC 0x1B
#define ASCII_EOT 0x04

#endif
