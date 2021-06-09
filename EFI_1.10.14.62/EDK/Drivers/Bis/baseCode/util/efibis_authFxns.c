/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name: efibis_authFxns.c

Abstract:    contain the replaceable authorization functions that bis
			 calls when no certificate is currently installed on a 
			 platform.

Revision History

--*/

#include <efi.h>
#include <efidriverlib.h>
#include <bisBaseCode.h>


///////////////////////////////////////////////////////////////////////////////
//
// The following functions are compiled only if the interactive version of
//     the authorization functions are to be used.  Uncomment the #define
//     below to enable the interactive authorization function.
//
///////////////////////////////////////////////////////////////////////////////

//#define INTERACTIVE_AUTHORIZATION_REQUIRED
#ifdef INTERACTIVE_AUTHORIZATION_REQUIRED


///////////////////////////////////////////////////////////////////////////////
//
// Type Name: AUTHORIZATION_STATE, AUTHORIZATION_STATE_PTR
//
// Description:
//     This  is  a  type  internal  to the authorization sample.  It stores the
//     previous  authorized identity certificate if there is one.  This is used
//     for   subsequent   automatic  authorization  if  the  same  identity  is
//     presented, or updated if a different identity is presented.
//
// Definitions:
//      AUTH_MAX_STORED_CERT - length  in  bytes of maximum storage space for a
//                             remembered authorized identity.
//      hasAuthorityCert     - TRUE if the authorization state is currently
//                             storing   a   (possibly   truncated)  authorized
//                             identity certificate.
//      authorityCertLen     - The  length in bytes of the (possibly truncated)
//                             authorized  identity  certificate.   Valid  only
//                             when hasAuthorityCert is TRUE.
//      authorityCert        - A pointer to the (possibly truncated) authorized
//                             identity  certificate  data.  Data is valid only
//                             when hasAuthorityCert is TRUE.
///////////////////////////////////////////////////////////////////////////////
#define AUTH_MAX_STORED_CERT 2048
typedef
struct tag_AUTHORIZATION_STATE {
    BOOLEAN       hasAuthorityCert;
    UINT32        authorityCertLen;
    UINT8 *       authorityCert;
} AUTHORIZATION_STATE;

///////////////////////////////////////////////////////////////////////////////
// Forward declarations:
///////////////////////////////////////////////////////////////////////////////

typedef
struct tag_CERT_SCAN_STATE
CERT_SCAN_STATE;

static
BOOLEAN
IsWithinBuffer(
    IN  CERT_SCAN_STATE         *scanState,
    IN  UINT32                  numBytes
    );

static
BOOLEAN
ExtractTag(
    IN OUT CERT_SCAN_STATE      *scanState,
    OUT    UINT8                *tag
    );

static
BOOLEAN
ExtractLength(
    IN OUT CERT_SCAN_STATE      *scanState,
    OUT    UINT32               *length
    );

static
BOOLEAN
SkipSequence(
    IN OUT CERT_SCAN_STATE      *scanState
    );

static
BOOLEAN
GetSignatureFromCert(
    IN   EFI_BIS_DATA           *cert,
    IN   UINT32                 maxLength,
    OUT  UINT32                 *length,
    OUT  UINT8                  *signature);

static
BOOLEAN
QueryUserAuthorization(
    IN  EFI_BIS_CALLING_FUNCTION    CallingFunction,
    IN  EFI_BIS_DATA                *thisAuthority
    );

static
BOOLEAN
MatchesAuthorityCert(
    IN  AUTHORIZATION_STATE     *rememberedAuthority,
    IN  EFI_BIS_DATA            *thisAuthority
    );

static
EFI_STATUS
SaveAuthority(
    IN OUT AUTHORIZATION_STATE  *rememberedAuthority,
    IN     EFI_BIS_DATA         *thisAuthority
    );

typedef
struct tag_ACCUMULATED_STRING
ACCUMULATED_STRING;

static
BOOLEAN
AppendToString(
    IN OUT  ACCUMULATED_STRING      *currentString,
    IN      CHAR16                  *newString,
    IN      UINT32                  newLength
    );


///////////////////////////////////////////////////////////////////////////////
//  Function Name: MatchesAuthorityCert
//
//  Description:
//      This   internal   function   tests  whether  the  passed  thisAuthority
//      certificate  matches the stored authority certificate, if any, at least
//      up to the point where the stored authority certificate is truncated.
//
//  Parameters:       
//      rememberedAuthority - (IN) Supplies   a   pointer   to  the  remembered
//                                 authority state structure
//      thisAuthority       - (IN) Supplies   a   pointer  to  the  new  signer
//                                 certificate  to  be  tested  to  see  if  it
//                                 matches the remembered certificate.
//
//  Returns:       
//      TRUE  if  there is a remembered certificate and the new certificate
//      matches up to the truncation point, otherwise FALSE.
//
///////////////////////////////////////////////////////////////////////////////
static
BOOLEAN
MatchesAuthorityCert(
    IN  AUTHORIZATION_STATE     *rememberedAuthority,
    IN  EFI_BIS_DATA            *thisAuthority
    )
{
    BOOLEAN  to_return;
    
    if (rememberedAuthority->hasAuthorityCert) 
    {
        UINT32  target_len = rememberedAuthority->authorityCertLen;
        UINT32  new_len = thisAuthority->Length;

        // Truncate the length we may compare
        if (new_len > AUTH_MAX_STORED_CERT) 
        {
            new_len = AUTH_MAX_STORED_CERT;
        }

        // If the lengths don't match, the certs are different
        if (target_len != new_len) 
        {
            to_return = FALSE;
        }

        // Otherwise  the (possibly truncated) lengths are the same, so we must
        // compare the contents
        else 
        {
            UINT32   i;
            UINT8 *  target_array = rememberedAuthority->authorityCert;
            UINT8 *  new_array = thisAuthority->Data;

            // Setup  a  "success"  result  in  case  we  fall through the loop
            // without finding a difference.
            to_return = TRUE;
            for (i=0; i<target_len; i++) 
            {
                if (target_array[i] != new_array[i]) 
                {
                    to_return = FALSE;
                    break;
                }
            }
        } // else compare contents
    } // if ... hasAuthorityCert
    else 
    {
        to_return = FALSE;
    }

    return to_return;
    
} // MatchesAuthorityCert

///////////////////////////////////////////////////////////////////////////////
//  Function Name: SaveAuthority
//
//  Description:
//      This internal procedure saves up to the allowed maximum number of bytes
//      of the indicated new thisAuthority certificate.
//
//  Parameters:       
//      rememberedAuthority - (IN OUT) Supplies  a  pointer  to  the remembered
//                                     authority   state  structure  where  the
//                                     (possibly   truncated)  new  certificate
//                                     will be stored.
//      thisAuthority       - (IN)     Supplies  a  pointer  to  the new signer
//                                     certificate to be saved.
//
//  Returns:       
//      none
//
///////////////////////////////////////////////////////////////////////////////
static
EFI_STATUS
SaveAuthority(
    IN OUT AUTHORIZATION_STATE      *rememberedAuthority,
    IN     EFI_BIS_DATA             *thisAuthority
    )
{
    EFI_STATUS  to_return = EFI_SUCCESS;
    UINT32      new_len = thisAuthority->Length;
    UINT8       *new_array = thisAuthority->Data;
    UINT8       *target_array;
    UINT32      i;

    // Truncate if required
    if (new_len > AUTH_MAX_STORED_CERT) 
    {
        new_len = AUTH_MAX_STORED_CERT;
    }
    
    rememberedAuthority->authorityCert = (UINT8*)AllocateZeroPool(new_len);
    if (rememberedAuthority->authorityCert)
    {
        target_array = rememberedAuthority->authorityCert;
        for (i=0; i<new_len; i++) 
        {
            target_array[i] = new_array[i];
        }
        rememberedAuthority->authorityCertLen = new_len;
        rememberedAuthority->hasAuthorityCert = TRUE;
    }
    else
    {
        DEBUG((D_ERROR,
                	"AUTHFXNS: Memory allocation failed\n"));
		to_return  = EFI_OUT_OF_RESOURCES;
    }

    return to_return;       
    
} // SaveAuthority


///////////////////////////////////////////////////////////////////////////////
// Internal user-interaction and string-fomatting functions
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Type Name:  ACCUMULATED_STRING, * ACCUMULATED_STRING_PTR
//
// Description:
//    This  type  is used to accumulate a string by appending characters to the
//    end.
//
// Definitions:
//    buffer    - The buffer where the string is assembled
//    maxLength - The length of the buffer in bytes
//    length    - Number of bytes currently in the string
///////////////////////////////////////////////////////////////////////////////
typedef
struct tag_ACCUMULATED_STRING {
    CHAR16      *buffer;
    UINT32      maxLength;
    UINT32      length;
} ACCUMULATED_STRING;


///////////////////////////////////////////////////////////////////////////////
//  Function Name: QueryUserAuthorization
//
//  Description:
//      This internal procedure queries the user to see if the indicated signer
//      certificate for the indicated operation is from an accepted authority.
//
//  Parameters:
//      CallingFunction - (IN) Supplies  the operation code of the operation 
//                             that needed external authorization.
//      thisAuthority   - (IN) Supplies  a  pointer to the new signer 
//                             certificate to be tested.
//
//  Returns:       
//      TRUE  if  the  user  accepts  the  indicated certificate, otherwise
//      FALSE;
//
///////////////////////////////////////////////////////////////////////////////

// The  following  strings  are  really  intended  to  be  constants.  They are
// declared  as  static  globals  instead  of  #define strings to keep multiple
// references from causing them to appear multiple times in the generated code.

static UINT16 QUERY_LEAD[]     = L"\n\nRequest to ";
static UINT16 QUERY_BOOT[]     = L"boot";
static UINT16 QUERY_UPDATE[]   = L"change boot security";
static UINT16 QUERY_UNKNOWN[]  = L"do <unknown>";
static UINT16 QUERY_TRAIL[]    = L" with signature:\n\r";
static UINT16 QUERY_BAD_SIG[]  = L"UNRECOGNIZED CERTIFICATE FORMAT\n\r";
static UINT16 QUERY_QUESTION[] = L"Allow request to proceed? (y or n) --> ";

// A  raw  DSA  signature  is  actually  a  SEQUENCE of two DER-encoded 20-byte
// integers.  Each raw integer needs a 1-byte sequence tag plus a 1-byte length
// value, plus a possible 1-byte leading-0 extension (depending on the value of
// the  most significant bit of the integer).  So each integer may take as many
// as 23 bytes to represent.  The SEQUENCE header in front of these consists of
// a  1-byte  SEQUENCE  tag plus a 1-byte length value.  Altogether the raw DSA
// signature  takes  up  a maximum of 1+1+(2*23) = 48 bytes.  The minimum is 46
// bytes.
#define MAX_SIGNATURE_BYTES_TO_SHOW (48)
#define RESP_BUFFER_LENGTH   2

#define QUERY_BUFFER_LEN ( \
    sizeof(QUERY_LEAD) - 2 + \
    sizeof(QUERY_UPDATE) - 2 + \
    sizeof(QUERY_TRAIL) - 2 + \
    2)

static
BOOLEAN
QueryUserAuthorization(
    IN  EFI_BIS_CALLING_FUNCTION    CallingFunction,
    IN  EFI_BIS_DATA                *thisAuthority
    )
{
    CHAR16              query_buffer[QUERY_BUFFER_LEN];
    ACCUMULATED_STRING  query;
    BOOLEAN             ok_so_far;
    UINT8               sig_buffer[MAX_SIGNATURE_BYTES_TO_SHOW];
    UINT32              sig_len;
    BOOLEAN             got_sig;
    BOOLEAN             to_return;
    CHAR16              *resp = NULL;

    query.buffer    = query_buffer;
    query.maxLength = QUERY_BUFFER_LEN;
    query.length    = 0;

    resp = AllocateZeroPool(RESP_BUFFER_LENGTH);
    if (!resp)
    {
        ok_so_far = FALSE;
    }

    if (ok_so_far)
    {
        ok_so_far = AppendToString(
            & query,                  // currentString
            QUERY_LEAD,               // newString
            BisStrLen(QUERY_LEAD));   // newLength
    }

    if (ok_so_far) 
    {
        switch (CallingFunction) 
        {
          case BisCallingFunctionVerify:
            ok_so_far = AppendToString(
                & query,                    // currentString
                QUERY_BOOT,                 // newString
                BisStrLen(QUERY_BOOT));     // newLength
            break;
          case BisCallingFunctionUpdate:
            ok_so_far = AppendToString(
                & query,                    // currentString
                QUERY_UPDATE,               // newString
                BisStrLen(QUERY_UPDATE));   // newLength
            break;
          default:
            ok_so_far = AppendToString(
                & query,                     // currentString
                QUERY_UNKNOWN,               // newString
                BisStrLen(QUERY_UNKNOWN));   // newLength
            break;
        } // switch CallingFunction
    }

    if (ok_so_far) 
    {
        ok_so_far = AppendToString(
            & query,                        // currentString
            QUERY_TRAIL,                    // newString
            BisStrLen(QUERY_TRAIL));        // newLength
    }

    if (ok_so_far)
    {
        // Null terminate the query
        ok_so_far = AppendToString(
            &query,                         // currentSgtring
            L"\0",                          // newString
            2);                             // newLength

        Output(query.buffer);
    }

    if (ok_so_far) 
    {
        got_sig = GetSignatureFromCert(
            thisAuthority,                // cert
            MAX_SIGNATURE_BYTES_TO_SHOW,  // maxLength
            & sig_len,                    // length
            sig_buffer);                  // signature
    }

    if (ok_so_far) 
    {
        if (got_sig) 
        {
            DumpHex( 2,                             // Indent
                     0,                             // Byte offset into UserData
                     MAX_SIGNATURE_BYTES_TO_SHOW,   // bytes to print
                     &sig_buffer);                  // User data
        }
        else 
        {
            Output(QUERY_BAD_SIG);
        }
    }

    Input(QUERY_QUESTION,           // Query
          resp,                     // character to read
          RESP_BUFFER_LENGTH);      // maximum characters to read    
    if ((*resp == L'y') ||
        (*resp == L'Y'))
    {
        to_return = TRUE;
    }
    else 
    {
        to_return = FALSE;
    }

    if (resp)
    {
        FreePool(resp);
    }

    Output(L"\n\n");   // give a couple of blank lines for readability purpose

    // I don't know how to clear the screen.....


    // Clear the screen by writing a form-feed '\f'.
    //query_buffer[0]= '\f';
    //AuthWriteConsoleString(
    //    query_buffer,   // msgBuffer
    //    1);             // msgLength

    return to_return;

} // QueryUserAuthorization

///////////////////////////////////////////////////////////////////////////////
// Function Name:  AppendToString
//
// Description:
//     This  internal  function  appends  the indicated bytes to the end of the
//     accumulated string.
//
// Parameters:
//     currentString - (IN OUT) The accumulated string so far
//     newString     - (IN)     The buffer containing contents to be added
//     newLength     - (IN)     Number of bytes to be added
//
// Returns:
//     TRUE if there was room to add the newString.  Otherwise the function
//     returns FALSE and currentString is not modified.
///////////////////////////////////////////////////////////////////////////////
static
BOOLEAN
AppendToString(
    IN OUT  ACCUMULATED_STRING      *currentString,
    IN      CHAR16                  *newString, 
    IN      UINT32                  newLength
    )
{
    BOOLEAN      to_return;
    UINT32       current_length = currentString->length;
    UINT16       *target_buffer = currentString->buffer;

    if (current_length + newLength > currentString->maxLength) 
    {
        to_return = FALSE;
    }
    else 
    {
        UINT32 i;

        to_return = TRUE;
        for (i=0; i<newLength; i++) 
        {
            target_buffer[current_length + i] = newString[i];
        }
        currentString->length += newLength;
    }

    return to_return;
    
} // AppendToString



///////////////////////////////////////////////////////////////////////////////
// Basic Certificate-parsing functions
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// The following is a small collection of types and procedures that are used to
// extract  a  minimal amount of information from a certificate, in particular,
// some  or  all  of  the  digital  signature bit string.  These procedures are
// designed  to  handle  a  DER-encoded  PKCS-6  ExtendedCertificate or a X.509
// Certificate.   From  the  point  of  view  of  these procedures, the overall
// structure of these two types is basically the same, consisting of:
//
// SEQUENCE {
//   SEQUENCE {
//     -- main contents of certificate
//   }
//   SEQUENCE {
//     -- signature algorithm identifier and parameters
//   }
//   signature  BIT STRING
// }
//
// PKCS-6   and   X.509  certificate  definitions  are  described  in  "PKCS#6:
// Extended-Certificate  Syntax  Standard".  A useful explanation of the syntax
// notation and a guide to the BER/DER encoding of certificates can be found in
// "A  Layman's  Guide  to  a  Subset  of  ASN.1, BER, and DER".  Both of these
// documents can be retrieved online through the URL:
//
//     http://www.rsa.com/rsalabs/pubs/PKCS/index.html
//
// Another  extensive  and  readable explanation of X.509v3 certificates is the
// "X.509 Style Guide", which can be found at the URL:
//
//     http://www.cs.auckland.ac.nz/~pgut001/pubs/x509guide.txt
//
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// Type Name: CERT_SCAN_STATE, CERT_SCAN_STATE_PTR
//
// Description:
//     This internal type is used by the collection of procedures that
//     scan through a certificate.  The type keeps track of how far the
//     scan has proceeded.  This type is only intended to be manipulated
//     by those procedures.
//
// Definitions:
//     certBuffer    - A pointer to the in-memory copy of the certificate data
//     certLength    - The  length,  in  bytes,  of  the  in-memory copy of the
//                     certificate data
//     currentOffset - The  current  offset,  in  bytes, of the next byte to be
//                     examined in the certificate buffer.
///////////////////////////////////////////////////////////////////////////////
typedef
struct tag_CERT_SCAN_STATE {
    UINT8       *certBuffer;
    UINT32      certLength;
    UINT32      currentOffset;
} CERT_SCAN_STATE;



///////////////////////////////////////////////////////////////////////////////
//  Function Name: GetSignatureFromCert
//
//  Description:
//      This  internal function extracts and returns up to the indicated number
//      of   bytes   of   the   raw   signature   BIT   STRING  from  a  PKCS-6
//      ExtendedCertificateOrCertificate.
//
//  Parameters:
//      cert      -  (IN) Supplies the certificate to be examined
//      maxLength -  (IN) Supplies the maximum number of bytes of signature BIT
//                        STRING that may be returned.
//      length    - (OUT) Returns  the  number of bytes of signature BIT STRING
//                        that were retrieved if the function is successful.
//      signature - (OUT) Returns  the  signature  BIT  STRING  contents if the
//                        function is successful.
//
//  Returns:       
//      TRUE  if  successful,  otherwise the function returns FALSE and
//      the OUT parameters are undefined.
//
///////////////////////////////////////////////////////////////////////////////
static
BOOLEAN
GetSignatureFromCert(
    IN   EFI_BIS_DATA   *cert,
    IN   UINT32         maxLength,
    OUT  UINT32         *length,
    OUT  UINT8          *signature)
{
    CERT_SCAN_STATE     scanState;
    BOOLEAN             ok_so_far;
    UINT8               tag;
    UINT32              sig_length;

    scanState.certBuffer    = cert->Data;
    scanState.certLength    = cert->Length;
    scanState.currentOffset = 0;
    
    // Extract and check the outer sequence tag
    ok_so_far = ExtractTag(
        & scanState,  // scanState
        & tag);       // tag
    if (ok_so_far) 
    {
        // The  first  tag  should  be  either  0x30  (sequence)  for  a  X.509
        // certificate  or 0xA0 (implicit context-specific contructed id 0) for
        // a  PKCS-6  ExtendedCertificate.   In  either  case, the tag actually
        // represents the outer SEQUENCE.
        #define BISCERT_X509_OUTERTAG       (0x30)
        #define BISCERT_PKCS6_EXT_OUTERTAG  (0xA0)
        if ((tag != BISCERT_X509_OUTERTAG) &&
            (tag != BISCERT_PKCS6_EXT_OUTERTAG)) 
        {
            ok_so_far = FALSE;
        }
    }
    
    // Extract and ignore the outer length
    if (ok_so_far) 
    {
        UINT32 dont_care;
        ok_so_far = ExtractLength(
            & scanState,    // scanState
            & dont_care);   // length
    }

    // Skip the main Certificate body sequence
    if (ok_so_far) 
    {
        ok_so_far = SkipSequence(
            & scanState);  // scanState
    }

    // Skip the signature algorithm identifier sequence
    if (ok_so_far) 
    {
        ok_so_far = SkipSequence(
            & scanState);  // scanState
    }

    // Remainder  should  be  a  Universal,  Primitive, BIT STRING which is the
    // signature.  Check and extract it.
    if (ok_so_far) 
    {
        ok_so_far = ExtractTag(
            & scanState,  // scanState
            & tag);       // tag
    }
    // The tag should be 0x03, representing a BIT STRING
    #define BISCERT_BIT_STRING_TAG   (0x03)
    if (ok_so_far) 
    {
        if (tag != BISCERT_BIT_STRING_TAG) 
        {
            ok_so_far = FALSE;
        }
    }
    if (ok_so_far) 
    {
        ok_so_far = ExtractLength(
            & scanState,    // scanState
            & sig_length);  // length
    }
    // Check containment of the BIT STRING contents
    if (ok_so_far) 
    {
        ok_so_far = IsWithinBuffer(
            & scanState,  // scanState
            sig_length);  // numBytes
    }
    // First  byte  of  BIT STRING contents tells how many non-significant bits
    // are  in  the  last  byte of the BIT STRING bits.  We simply ignore this,
    // skip it, and reduce our expected remaining content length accordingly.
    if (ok_so_far) 
    {
        scanState.currentOffset ++;
        sig_length --;
        // Require at least one byte of actual signature BIT STRING bits
        if (sig_length < 1) 
        {
            ok_so_far = FALSE;
        }
    }
    if (ok_so_far) 
    {
        UINT32  bytes_to_copy;
        UINT32  start_offset;
        UINT32  i;

        bytes_to_copy = maxLength;
        if (bytes_to_copy > sig_length) 
        {
            bytes_to_copy = sig_length;
        }
        start_offset = scanState.currentOffset;
        for (i=0; i<bytes_to_copy; i++) 
        {
            signature[i] = cert->Data[start_offset + i];
        }
        scanState.currentOffset += sig_length;  // for consistency
        * length = bytes_to_copy;
    }

    return ok_so_far;
    
} // GetSignatureFromCert

///////////////////////////////////////////////////////////////////////////////
//  Function Name: SkipSequence
//
//  Description:
//      This   internal  function  scans  past  a  SEQUENCE  in  the  indicated
//      certificate.   It  does  not  extract  and return any values.  The scan
//      state is updated appropriately.
//
//      The scan state must be pointing to the beginning of the SEQUENCE (i.e.,
//      the  SEQUENCE  type  tag)  when called.  When this function returns the
//      scan state points to the next byte past the SEQUENCE content.
//
//  Parameters:
//      scanState - (IN OUT) Holds  information  about  the certificate and the
//                           state of the scan
//
//  Returns:       
//      TRUE if successful, otherwise FALSE.
//
///////////////////////////////////////////////////////////////////////////////
static
BOOLEAN
SkipSequence(
    IN OUT CERT_SCAN_STATE      *scanState
    )
{
    UINT32          seq_len;
    UINT8           tag;
    BOOLEAN         ok_so_far;

    ok_so_far = ExtractTag(
        scanState,  // scanState
        & tag);     // tag
    if (ok_so_far) 
    {
        // Tag must be 0x30 for sequence
        #define BISCERT_SEQUENCE_TAG  (0x30)
        if (tag != BISCERT_SEQUENCE_TAG) 
        {
            ok_so_far = FALSE;
        }
    }
    if (ok_so_far) 
    {
        ok_so_far = ExtractLength(
            scanState,   // scanState
            & seq_len);  // length
    }
    if (ok_so_far) 
    {
        // check containment of skipped sequence content bytes
        ok_so_far = IsWithinBuffer(
            scanState,  // scanState
            seq_len);   // numBytes
    }
    if (ok_so_far) 
    {
        // skip the sequence content bytes
        scanState->currentOffset += seq_len;
    }

    return ok_so_far;
    
} // SkipSequence


///////////////////////////////////////////////////////////////////////////////
//  Function Name: ExtractTag
//
//  Description:
//      This  internal  function  extracts  and  returns  a  type  tag from the
//      indicated certificate.  The scan state is updated appropriately.
//
//      The  scan  state must be pointing to the beginning of the type tag when
//      called.   When  this function returns the scan state points to the next
//      byte past the type tag.
//
//  Parameters:
//      scanState - (IN OUT) Holds  information  about  the certificate and the
//                           state of the scan
//      tag       - (OUT)    Returns  the  retrieved  tag  if  the  function is
//                           successful.
//
//  Returns:       
//      TRUE if successful, otherwise FALSE.
//
///////////////////////////////////////////////////////////////////////////////
static
BOOLEAN
ExtractTag(
    IN OUT CERT_SCAN_STATE      *scanState,
    OUT    UINT8 *              tag
    )
{
    BOOLEAN         ok_so_far;
    UINT8           temp_tag;
    
    // The first byte of a type tag is as follows:
    //   8   7   6   5   4   3   2   1
    // +---+---+---+---+---+---+---+---+
    // | C C   |P/C| ID Code           |
    // +---+---+---+---+---+---+---+---+
    //   CC    is the "class", meaning:
    //   00    Universal
    //   01    Application-specific
    //   10    Context-specific
    //   11    Private
    //
    //   P/C   indicates Primitive or Constructed:
    //   0     Primitive type
    //   1     Constructed type
    //
    //   ID  Code  is a type identifier code.  The special identifier code 0x1F
    //   indicates  that  the  type  tag  extends into additional bytes.  We'll
    //   simply  report  a  failure for this since we don't expect to encounter
    //   any such type tags.
    #define BISCERT_IDCODE_MASK        (0x1F)
    #define BISCERT_EXTENDED_TYPE_TAG  (0x1F)

    ok_so_far = IsWithinBuffer(
        scanState,  // scanState
        1);         // numBytes
    if (ok_so_far) 
    {
        temp_tag = scanState->certBuffer[scanState->currentOffset];
        * tag = temp_tag;
        scanState->currentOffset ++;
        if ((temp_tag & BISCERT_IDCODE_MASK) == BISCERT_EXTENDED_TYPE_TAG) 
        {
            ok_so_far = FALSE;  // fail due to expanding tag
        }
    }

    return ok_so_far;
    
} // ExtractTag

///////////////////////////////////////////////////////////////////////////////
//  Function Name: ExtractLength
//
//  Description:
//      This  internal  function  extracts  and returns a length value from the
//      indicated certificate.  The scan state is updated appropriately.
//
//      The  scan  state  must be pointing to the beginning of the length value
//      when  called.   When this function returns the scan state points to the
//      next byte past the length value.
//
//  Parameters:
//      scanState - (IN OUT) Holds  information  about  the certificate and the
//                           state of the scan
//      length    - (OUT)    Returns the retrieved length value if the function
//                           is successful.
//
//  Returns:       
//      TRUE if successful, otherwise FALSE.
//
///////////////////////////////////////////////////////////////////////////////
static
BOOLEAN
ExtractLength(
    IN OUT CERT_SCAN_STATE      *scanState,
    OUT    UINT32               *length
    )
{
    BOOLEAN         ok_so_far;
    UINT8           length_of_length;
    
    // The first byte of a length field is as follows:
    //   8   7   6   5   4   3   2   1
    // +---+---+---+---+---+---+---+---+
    // |S/L|  primary_length           |
    // +---+---+---+---+---+---+---+---+
    //  S/L    indicates a short or long length form
    //  0      primary_length is the actual length of the content
    //  1      primary_length is a "length of length" or "indefinite" form
    //
    //  In the case of S/L=1, primary_length!=0:
    //     indicates  that  the  next  primary_length  bytes  are the length in
    //     big-endian  form.  Since we don't expect to encounter extremely long
    //     contents, we'll simply report a failure if the "length of length" is
    //     more than 4 bytes.
    //
    //  In the case of S/L=1, primary_length==0:
    //     indicates  that  the  length  is  indefinite  and  the  contents  is
    //     terminated  by a special end-of-contents value.  We'll simply report
    //     a failure for this case, since we don't expect to
    //     encounter indefinite lengths.
    #define BISCERT_SL_MASK            (0x80)
    #define BISCERT_LENGTH_MASK        (0x7F)
    #define BISCERT_INDEFINTE_LENGTH   (0x80)
    #define BISCERT_MAX_LEN_OF_LEN     (sizeof(UINT32))
    
    ok_so_far = IsWithinBuffer(
        scanState,  // scanState
        1);         // numBytes
    if (ok_so_far) 
    {
        length_of_length = scanState->certBuffer[scanState->currentOffset];
        scanState->currentOffset++;
        if (length_of_length == BISCERT_INDEFINTE_LENGTH) 
        {
            ok_so_far = FALSE;  // Don't handle indefinite length form
        }
    }
    if (ok_so_far) 
    {
        if (! (length_of_length & BISCERT_SL_MASK)) 
        {
            // Using short form
            * length = (UINT32) length_of_length;
        }
        else 
        {
            // Using long form
            length_of_length &= BISCERT_LENGTH_MASK;
            if (length_of_length > BISCERT_MAX_LEN_OF_LEN) 
            {
                // Longer than we want to handle
                ok_so_far = FALSE;
            }
            if (ok_so_far) 
            {
                // Check containment of length bytes
                ok_so_far = IsWithinBuffer(
                    scanState,          // scanState
                    length_of_length);  // numBytes
            }
            if (ok_so_far) 
            {
                // Extract big-endian multiple-byte length
                UINT32 i;
                UINT32 temp_length;
                
                temp_length = 0;
                for (i=0; i<length_of_length; i++) 
                {
                    temp_length *= 256;
                    temp_length += (UINT32)
                        scanState->certBuffer[scanState->currentOffset];
                    scanState->currentOffset ++;
                }
                * length = temp_length;
            }
        } // else long form
    } // if ok_so_far
    
    return ok_so_far;
    
} // ExtractLength

///////////////////////////////////////////////////////////////////////////////
//  Function Name: IsWithinBuffer
//
//  Description:
//      This internal function tests whether the desired number of bytes
//      are contained within the remaining certificate buffer (that is,
//      within the array index from currentOffset onward).
//
//  Parameters:
//      scanState - (IN) Holds  information about the certificate and the state
//                       of the scan
//      numBytes  - (IN) Supplies  the number of bytes desired in the remaining
//                       buffer.
//
//  Returns:       
//      TRUE  if  at  least this many bytes remain in the buffer, otherwise
//      FALSE
//
///////////////////////////////////////////////////////////////////////////////
static
BOOLEAN
IsWithinBuffer(
    IN  CERT_SCAN_STATE     *scanState,
    IN  UINT32              numBytes
    )
{
    UINT32          num_bytes_remaining;
    BOOLEAN         to_return;

    num_bytes_remaining = scanState->certLength - scanState->currentOffset;
    if (num_bytes_remaining >= numBytes) 
    {
        to_return = TRUE;
    }
    else 
    {
        to_return = FALSE;
    }

    return to_return;
    
} // IsWithinBuffer


///////////////////////////////////////////////////////////////////////////////
//  Function Name: EFIBIS_Authorization_fn
//
//  Description:    This is the interactive version of the authorization 
//              function.  When an authorize request is received it checks to 
//              see if a request has already been received. If so, it checks
//              the signing certificate with the one that was stored on a 
//              previous authorization call.  If it matches the IsAuthorized
//              is set to TRUE.  If it is not the same certificate, or it
//              is a new request the user us prompted for a yes/no decision
//              to allow the request.   
//
//  Parameters:
//      This              - (IN) 
//      CallingFunction   - (IN)  tells the operation for authorization needed
//      Credentials       - (IN) 
//      CredentialsSigner - (IN) 
//      DataObject        - (IN)
//      Reserved          - (IN)  Reserved
//      IsAuthorized      - (OUT) TRUE if authorized, FALSE otherwise
//
//  Returns:       
//      Status code
//
///////////////////////////////////////////////////////////////////////////////
EFI_STATUS
EFIBIS_Authorization_fn(
    IN   EFI_BIS_DEFAULT_AUTH_INTERFACE  *This,
    IN   EFI_BIS_CALLING_FUNCTION		 CallingFunction,
    IN   EFI_BIS_DATA     				 *Credentials,
    IN   EFI_BIS_DATA     				 *CredentialsSigner,
    IN   EFI_BIS_DATA     				 *DataObject,
    IN   VOID            				 *Reserved,
    OUT  BOOLEAN         				 *IsAuthorized
    )
{
	AUTHORIZATION_STATE      *auth_state;
    EFI_STATUS               to_return;
    BOOLEAN                  same_authority;
    
    

    // Parameter checking
    to_return = EFI_INVALID_PARAMETER;   // assume fail till success

    if ((CallingFunction == BisCallingFunctionVerify) ||
        (CallingFunction == BisCallingFunctionUpdate))
    {
        if (This)
        {
            if (Credentials && Credentials->Data == NULL)
            {
                if (CredentialsSigner && CredentialsSigner->Data == NULL)
                {
                    if (IsAuthorized)
                    {
                        if (Reserved == NULL)
                        {
                            if (CallingFunction == BisCallingFunctionVerify)
                            {
                                if (DataObject && DataObject->Data != NULL)
                                {
                                    to_return = EFI_SUCCESS;
                                }
                            }
                            else
                            {
                                // data object can be null in this case
                                to_return = EFI_SUCCESS;
                            }
                        } // End of if (Reserved == NULL)
                    } // End of if (IsAuthorized)
                } // End of if (CredentialsSigner && CredentialsSigner->Data)
            } // End of if (Credentials && Credentials->Data)
        } // End of if (This)
    } // End of if (CallingFunction == (BisCallingFunctionVerify || ..
    
    if (EFI_SUCCESS == to_return)
    {
        auth_state = (AUTHORIZATION_STATE *) This->InstanceData;

        same_authority = MatchesAuthorityCert(
                            auth_state,          // rememberedAuthority
                            CredentialsSigner);  // thisAuthority
        if (same_authority) 
        {
            // This  identity  is  the  one  we  have previously authorized, so
            // simply authorize the operation with no further user interaction.
            * IsAuthorized = TRUE;
        } // if same_authority
        else 
        {  // User interaction is required
            BOOLEAN  user_authorizes;
        
            // Remove  any  remembered  certificate  we  currently  have.  This may
            // require  modification  if  the  sample  is  altered  to  use dynamic
            // allocation for the remembered certificate.
            auth_state->authorityCertLen = 0;
            auth_state->hasAuthorityCert = FALSE;
            if (auth_state->authorityCert != NULL)
            {
                FreePool(auth_state->authorityCert);
            }

            // Ask the user
            user_authorizes = QueryUserAuthorization(
                CallingFunction,     // CallingFunction
                CredentialsSigner);  // thisAuthority
            if (user_authorizes) 
            {
                // Save the new authority
                to_return = SaveAuthority(
                    auth_state,          // rememberedAuthority
                    CredentialsSigner);  // thisAuthority
                // And propagate the "authorized" result
                * IsAuthorized = TRUE;
            }
            else 
            {
                // Propagate  the  "not authorized" result.  Old authority has been
                // erased.
                * IsAuthorized = FALSE;
            }
        } // else user interaction
    } // End of if (EFI_SUCCESS == to_return)

    return to_return;
} // End of EFIBIS_Authorization_fn

#else // End of #ifdef INTERACTIVE_AUTHORIZATION_REQUIRED
///////////////////////////////////////////////////////////////////////////////
//  Function Name: EFIBIS_Authorization_fn
//
//  Description:  This is the non interactive implementation of the 
//              authorization function.  It does simple parameter checking
//              and then sets IsAuthorized to TRUE.
//      
//  Parameters:
//      This              - (IN) 
//      CallingFunction   - (IN)  tells the operation for authorization needed
//      Credentials       - (IN) 
//      CredentialsSigner - (IN) 
//      DataObject        - (IN)
//      Reserved          - (IN)  Reserved
//      IsAuthorized      - (OUT) TRUE if authorized, FALSE otherwise
//
//  Returns:       
//      Status code
//
///////////////////////////////////////////////////////////////////////////////
EFI_STATUS
EFIBIS_Authorization_fn(
    IN   EFI_BIS_DEFAULT_AUTH_INTERFACE  *This,
    IN   EFI_BIS_CALLING_FUNCTION		 CallingFunction,
    IN   EFI_BIS_DATA     				 *Credentials,
    IN   EFI_BIS_DATA     				 *CredentialsSigner,
    IN   EFI_BIS_DATA     				 *DataObject,
    IN   VOID            				 *Reserved,
    OUT  BOOLEAN         				 *IsAuthorized
    )
{
	EFI_STATUS               to_return;
    
    // Parameter checking
    to_return = EFI_INVALID_PARAMETER;   // assume fail till success

    if ((CallingFunction == BisCallingFunctionVerify) ||
        (CallingFunction == BisCallingFunctionUpdate))
    {
        if (This)
        {
            if (Credentials && Credentials->Data != NULL)
            {
                if (CredentialsSigner && CredentialsSigner->Data != NULL)
                {
                    if (IsAuthorized)
                    {
                        if (Reserved == NULL)
                        {
                            if (CallingFunction == BisCallingFunctionVerify)
                            {
                                if (DataObject && DataObject->Data != NULL)
                                {
                                    to_return = EFI_SUCCESS;
                                }
                            }
                            else
                            {
                                // data object can be null in this case
                                to_return = EFI_SUCCESS;
                            }
                        } // End of if (Reserved == NULL)
                    } // End of if (IsAuthorized)
                } // End of if (CredentialsSigner && CredentialsSigner->Data)
            } // End of if (Credentials && Credentials->Data)
        } // End of if (This)
    } // End of if (CallingFunction == (BisCallingFunctionVerify || ..
    
    if (EFI_SUCCESS == to_return)
    {
        * IsAuthorized = TRUE;
    } // End of if (EFI_SUCCESS == to_return)

    return to_return;
} // End of EFIBIS_Authorization_fn
#endif // ! defined INTERACTIVE_AUTHORIZATION_REQUIRED



	//
	// EFIBIS_InitAuthFxnModule - called by efibis_basecode.c
	//	
	//	Parm: 
	//		EFI_BIS_DEFAULT_AUTH_INTERFACE  **authInterface
	//
	//	Description:
	//		fills in the pointer to the auth interface passed as a parm.
	//
	//  returns: true if all ok.
	//

EFI_STATUS
EFIBIS_InitAuthFxnModule(
	EFI_BIS_DEFAULT_AUTH_INTERFACE  **authInterface
    )
{
    EFI_STATUS              		Status= EFI_SUCCESS;
    EFI_BIS_DEFAULT_AUTH_INTERFACE  *authI_F = NULL;
    EFI_HANDLE                      tempHandle;

	#ifdef INTERACTIVE_AUTHORIZATION_REQUIRED
    AUTHORIZATION_STATE             *authorizationState = NULL;
	#endif
    

    tempHandle = NULL;
  
  authI_F = x_malloc (sizeof(EFI_BIS_DEFAULT_AUTH_INTERFACE));
	if ( authI_F == NULL)
    {
		Status= EFI_OUT_OF_RESOURCES;
	}

	#ifdef INTERACTIVE_AUTHORIZATION_REQUIRED
		// Only allocate it if the interactive version of auth fxns are used

		//Allocate space for the modules instance data.
		authorizationState= AllocateZeroPool( sizeof(AUTHORIZATION_STATE) );
		if ( authorizationState == NULL )
		{
			Status= EFI_OUT_OF_RESOURCES;
		}

		if (EFI_SUCCESS == Status)
		{
			// Initiallize authorization specific data
			authorizationState->hasAuthorityCert = FALSE;
			authorizationState->authorityCertLen = 0;
			authorizationState->authorityCert = NULL;
		}

		// Authorization state is the instance data defined
		authI_F->InstanceData= (VOID*)authorizationState;
	#else
	    // There is no real instance data needed
	    authI_F->InstanceData= (VOID*)authI_F;
	#endif


    // Fill out the interface structur
    if ( Status == EFI_SUCCESS)
    {
        authI_F->CheckCredentials = EFIBIS_Authorization_fn;
        *authInterface= authI_F;
    }


	 //Free memory if we are going to fail for EFI reasons
	if ( Status != EFI_SUCCESS )
	{

		#ifdef INTERACTIVE_AUTHORIZATION_REQUIRED
        // Only clean it up if the interactive version of auth fxns are used
        if ( authorizationState != NULL){
			FreePool( authorizationState );
		}
		#endif

		if ( authI_F != NULL){
			gBS->FreePool( authI_F );
		}
	}


    return Status;
}

//eof


