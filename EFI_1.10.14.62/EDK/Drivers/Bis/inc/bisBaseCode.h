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
//
//

#ifndef BISBASECODE_H
#define BISBASECODE_H

#define MEMFREEBUG 1 		//workaround for bug that is visible in EFI but
							//is somehow obscured by the BIS memmgr.
							//see core_updt_boa.c for reference of this symbol.

#include <efi.h>
//#include <efibis.h>
#include <efidriverlib.h>
#include EFI_PROTOCOL_DEFINITION (Bis)
#include EFI_GUID_DEFINITION (SmBios)

// The following definitions were variously located in bis.h and bistypes.h.
// There were not propagated to efibis.h to avoid clutter in the EFI spec and
// to conform with there typography conventions.
// After this set of typedefs etc there are several more #includes that depend of
// the following definitions. The next set of #includes is preceded by
// this comment: "Second Wave of Includes"
//


//
//  Typedef for pBISEntry32( ).
//  BIS Entry Point Function Definition for 32bit flat address callers.
//

typedef UINT8 (_cdecl *pBisEntry32)(
        UINT32    opCode,                       //BIS OPCODE
        void     *pParamBundle,                 //OPCODE'S PARM BUNDLE
        UINT32    checkFlag);                   //CHECKSUM BIS REQUEST FLAG

typedef struct _BIS_ENTRY_POINT
{
    UINT8  length;          // Length  of  BIS_ENTRY_POINT structure,
                            // not including the two-byte null
                            // terminator. Value = sizeof(BIS_ENTRY_POINT) - 2;
    pBisEntry32 bisEntry32; // Entry point for 32-bit flat-mode callers.

    UINT16 doubleNull;      // 0000h structure terminator-See SMBIOS spec.
}
BIS_ENTRY_POINT,
*pBIS_ENTRY_POINT;


//----------------------------------------------------//
//          BIS Opcodes definitions                   //
//----------------------------------------------------//

#define BISOPBASE (0)

#define BISOP_Initialize                              (BISOPBASE+ 1)
#define BISOP_Free                                    (BISOPBASE+ 2)
#define BISOP_Shutdown                                (BISOPBASE+ 3)
#define BISOP_GetBootObjectAuthorizationCertificate   (BISOPBASE+ 4)
#define BISOP_VerifyBootObject                        (BISOPBASE+ 5)
#define BISOP_GetBootObjectAuthorizationCheckFlag     (BISOPBASE+ 6)
#define BISOP_GetBootObjectAuthorizationUpdateToken   (BISOPBASE+ 7)
#define BISOP_UpdateBootObjectAuthorization           (BISOPBASE+ 8)
#define BISOP_VerifyObjectWithCredential              (BISOPBASE+ 9)
#define BISOP_GetSignatureInfo                        (BISOPBASE+ 10)


//Update BISOP_LAST when adding opcodes. It should be
//equal to the highest numbered opcode used above.
#define BISOP_LAST (BISOP_GetSignatureInfo)

//
// BIS_DATA type.
//
// BIS_DATA instances obtained from BIS must be freed by calling Free( ).
//
typedef struct _BIS_DATA
{
    UINT32       length;        //Length of data in 8 bit bytes.
    UINT8        *data;          //32 Bit Flat Address of data.
}
BIS_DATA;

//
// BIS_VERSION type.
//
typedef struct _BIS_VERSION
{
    UINT32 major;               //BIS Interface version number.
    UINT32 minor;               //Build number.
}
BIS_VERSION;

//
// BIS_SIGNATURE_INFO and related pointers.
//
typedef struct _BIS_SIGNATURE_INFO
{
    BIS_CERT_ID   certificateID; //Truncated hash of platform Boot Object
                                 //  authorization certificate.
    BIS_ALG_ID    algorithmID;   //A signature algorithm number.
    UINT16        keyLength;     //Length of alg. keys in bits.
}
BIS_SIGNATURE_INFO,
*BIS_SIGNATURE_INFO_PTR;


typedef UINT32        BIS_STATUS;
typedef BIS_DATA     *BIS_DATA_PTR;
typedef UINT32        BIS_BOOLEAN;
typedef UINT32       *BIS_BOOLEAN_PTR;
typedef UINT8*        BIS_BYTE_PTR;


//
//  Common constants.
//
#define BIS_TRUE  (1)
#define BIS_FALSE (0)
#define BIS_NULL ((void*)0)


//
//BIS_STATUS values returned in BIS parm bundles 'returnValue' field.
//
#define BIS_OK                 (0)
#define BIS_INVALID_OPCODE     (1) //Returned by BIS_FunctionDispatch()
#define BIS_INVALID_PARMSTRUCT (2) //Null parm or bundle length is wrong.
#define BIS_MEMALLOC_FAILED    (3) //Couldn't alloc requested memory.
#define BIS_BAD_APPHANDLE      (4) //Invalid BIS_APPLICATION_HANDLE passed.
#define BIS_NOT_IMPLEMENTED    (5) //Unimplemented BIS function called.
#define BIS_BAD_PARM           (6) //A parm in the parm struct is invalid.
#define BIS_BOA_CERT_READ_ERR  (7) //An error occurred on CERT READ.
#define BIS_BOA_CERT_NOTFOUND  (8) //A BOA CERT is not configured.
#define BIS_SECURITY_FAILURE   (9) //A security check failed.
#define BIS_INIT_FAILURE       (10) //An internal failure occured in init.
#define BIS_INCOMPAT_VER       (11) //BIS interface version requested is not
                                    //  compatible with available version.
#define BIS_NVM_AREA_IO_LENGTH_ERROR    (12) //Length+offset combo incorrect.
#define BIS_NVM_AREA_UNKNOWN            (13) //Unknown area guid was specified.
#define BIS_NVM_CREATE_ERR_NO_ROOM      (14) //No space to create the new area.
#define BIS_NVM_CREATE_ERR_DUPLICATE_ID (15) //Area already exists.
#define BIS_NVM_BAD_HANDLE              (16) //Invalid handle passed.
#define BIS_NVM_PSI_FXNS_NOT_AVAIL      (17) //Bad/No PSI fxns passed to BIS_main.


#define BIS_PROTOIF_ERR                 (18) //bad protocol/interface array parm.
#define BIS_PROTO_UNDEF                 (19) //a protocol required by BIS is unavailable
#define BIS_PERSIST_MEM_SIZE_MISMATCH   (20) //memory len in persistent data did not match
#define BIS_GETGUID_ERROR               (21) //failed to get system guid




//****************************************//
//  BIS_Free( ) parameter bundle.         //
//  Op code= BISOP_Free                   //
//****************************************//

typedef
struct _BIS_Free_PARMS
{
    UINT32                  sizeOfStruct;   //[in]  Byte length of this structure.
    BIS_STATUS              returnValue;    //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE  appHandle;      //[int] From BIS_Initialize( ).
    BIS_DATA_PTR            toFree;         //[in]  BIS_DATA being freed.
}
BIS_FREE_PARMS,
*BIS_FREE_PARMS_PTR;



//*******************************************************************//
//  BIS_GetBootObjectAuthorizationCertificate( ) parameter bundle.   //
//  Op code= BISOP_GetBootObjectAuthorizationCertificate             //
//*******************************************************************//

typedef
struct _BIS_GetBootObjectAuthorizationCertificate_PARMS
{
    UINT32                  sizeOfStruct;   //[in]  Byte length of this structure.
    BIS_STATUS              returnValue;    //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE  appHandle;      //[in]  From BIS_Initialize( ).
    BIS_DATA_PTR            certificate;    //[out] Pointer to certificate.
}
BIS_GBOAC_PARMS,
*BIS_GBOAC_PARMS_PTR;



//**********************************************************************//
//   BIS_GetBootObjectAuthorizationCheckFlag( ) parameter bundle.       //
//  Op code= BISOP_IsBootObjectAuthorizationCheckRequired               //
//**********************************************************************//

typedef
struct _BIS_GetBootObjectAuthorizationCheckFlag_PARMS
{
    UINT32                  sizeOfStruct;    //[in]  Byte length of this structure.
    BIS_STATUS              returnValue;     //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE  appHandle;       //[in]  From BIS_Initialize( ).
    BIS_BOOLEAN             checkIsRequired; //[out] Value of check flag.
}
BIS_GBOACF_PARMS,
*BIS_GBOACF_PARMS_PTR;


//***********************************************************************//
//  BIS_GetBootObjectAuthorizationUpdateToken( ) parameter bundle.       //
//  Op code= BISOP_GetBootObjectAuthorizationUpdateToken                 //
//***********************************************************************//

typedef
struct _BIS_GetBootObjectAuthorizationUpdateToken_PARMS
{
    UINT32                  sizeOfStruct;   //[in]  Byte length of this structure.
    BIS_STATUS              returnValue;    //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE  appHandle;      //[in]  From BIS_Initialize( ).
    BIS_DATA_PTR            updateToken;    //[out] Value of update token.
}
BIS_GBOAUT_PARMS,
*BIS_GBOAUT_PARMS_PTR;



//**************************************************//
//  BIS_GetSignatureInfo( ) parameter bundle.       //
//  Op code= BISOP_GetSignatureInfo                 //
//      Two macros are define in BISTYPES.H to help //
//      manage 'signatureInfo':                     //
//          BIS_GET_SIGINFO_COUNT                   //
//          BIS_GET_SIGINFO_ARRAY                   //
//**************************************************//

typedef
struct _BIS_GetSignatureInfo_PARMS
{
    UINT32                  sizeOfStruct;   //[in]  Byte length of this structure.
    BIS_STATUS              returnValue;    //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE  appHandle;      //[in]  From BIS_Initialize( ).
    BIS_DATA_PTR            signatureInfo;  //[out] Signature info struct.
}
BIS_GSI_PARMS,
*BIS_GSI_PARMS_PTR;



//**************************************************//
//   BIS_Initialize( ) parameter bundle.            //
//  Op code= BISOP_Initialize                       //
//**************************************************//

typedef
struct _BIS_Initialize_PARMS
{
    UINT32                  sizeOfStruct;  //[in]  Byte length of this struct.
    BIS_STATUS              returnValue;   //[out] BIS_OK | error code.
    BIS_VERSION             interfaceVersion; //[in/out] ver needed/available.
    BIS_APPLICATION_HANDLE  appHandle;     //[out] Application handle.
    BIS_DATA                targetAddress; //[in] Address of BIS platform.
}
BIS_INIT_PARMS,
*BIS_INIT_PARMS_PTR;





//**********************************************//
//  BIS_Shutdown( ) parameter bundle.           //
//  Op code= BISOP_Shutdown                     //
//**********************************************//

typedef
struct _BIS_Shutdown_PARMS
{
    UINT32                  sizeOfStruct; //[in]  Byte length of this structure.
    BIS_STATUS              returnValue;  //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE  appHandle;    //[in]  From BIS_Initialize( ).
}
BIS_SHUTDOWN_PARMS,
*BIS_SHUTDOWN_PARMS_PTR;



//***********************************************************//
// BIS_UpdateBootObjectAuthorization( ) parameter bundle.    //
//  Op code= BISOP_UpdateBootObjectAuthorization             //
//***********************************************************//

typedef
struct _BIS_UpdateBootObjectAuthorization_PARMS
{
    UINT32                 sizeOfStruct;   //[in]  Byte length of this struct.
    BIS_STATUS             returnValue;    //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE appHandle;      //[in]  From BIS_Initialize( ).
    BIS_DATA               requestCredential; //[in]  Update Request Manifest.
    BIS_DATA_PTR           newUpdateToken;    //[out] Next update token.
}
BIS_UBOA_PARMS,
*BIS_UBOA_PARMS_PTR;




//************************************************//
//  BIS_VerifyBootObject( ) parameter bundle.     //
//  Op code= BISOP_VerifyBootObject               //
//************************************************//

typedef
struct _BIS_VerifyBootObject_PARMS
{
    UINT32                 sizeOfStruct; //[in]  Byte length of this structure.
    BIS_STATUS             returnValue;  //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE appHandle;    //[in]  From BIS_Initialize( ).
    BIS_DATA               credentials;  //[in]  Verification signed manifest.
    BIS_DATA               dataObject;   //[in]  Boot object to verify.
    BIS_BOOLEAN            isVerified;   //[out] Result of verifcation.
}
BIS_VBO_PARMS,
*BIS_VBO_PARMS_PTR;



//**************************************************************//
//  BIS_VerifyObjectWithCredential( ) parameter bundle.         //
//  Op code= BISOP_VerifyObjectWithCredential                   //
//**************************************************************//

typedef
struct _BIS_VerifyObjectWithCredential_PARMS
{
    UINT32        sizeOfStruct;   //[in]  Byte length of this structure.
    BIS_STATUS    returnValue;    //[out] BIS_OK | error code.
    BIS_APPLICATION_HANDLE  appHandle;    //[in]  From BIS_Initialize( ).
    BIS_DATA      credentials;    //[in]  Verification signed manifest.
    BIS_DATA      dataObject;     //[in]  Data object to verify.
    BIS_DATA      sectionName;    //[in]  Name of credential section to use.
    BIS_DATA      authorityCertificate;   //[in]  Certificate for credentials.
    BIS_BOOLEAN   isVerified;     //[out] Result of verification.
}
BIS_VOWC_PARMS,
*BIS_VOWC_PARMS_PTR;



// ******************************************************
//  Constant strings used to create and parse manifests *
// ******************************************************

//Manifest section name for Update-Request manifests
#define UPDATE_PARMS_SECTION_NAME     "memory:UpdateRequestParameters"

//Manifest section name for Boot Object signature manifests
#define BOOT_OBJECT_SECTION_NAME      "memory:BootObject"


//Update Manifest Data Attribute Names
#define OEM_UNIQUENESS_PREFIX "X-Intel-BIS-"
#define PARMSET_ATTR_NAME     OEM_UNIQUENESS_PREFIX "ParameterSet"
#define UPDATETOKEN_ATTR_NAME OEM_UNIQUENESS_PREFIX "ParameterSetToken"
#define PARMID_ATTR_NAME      OEM_UNIQUENESS_PREFIX "ParameterId"
#define PARMVALUE_ATTR_NAME   OEM_UNIQUENESS_PREFIX "ParameterValue"


//Valid values of the "ParameterId" data object in an update manifest.
#define BOAC_PARMID  "BootObjectAuthorizationCertificate"
#define BOACF_PARMID "BootAuthorizationCheckFlag"


//  The following string constants are used when calling
//  PrepareSignedManifestHandle to locate the signer info
//  in a manifest by name.

#define VERIFIABLE_OBJECT_SIGINFO_NAME "BIS_VerifiableObjectSignerInfoName"

#define UPDATE_MANIFEST_SIGINFO_NAME   "BIS_UpdateManifestSignerInfoName"


// Binary Value of "X-Intel-BIS-ParameterSet" Attribute.
// (Value is Base64 encoded in actual signed manifest).

// {EDD35E31-07B9-11d2-83A3-00A0C91FADCF}
#define BOOT_OBJECT_AUTHORIZATION_PARMSET_GUIDVALUE     \
{ 0xedd35e31, 0x7b9, 0x11d2,                            \
{ 0x83, 0xa3, 0x0, 0xa0, 0xc9, 0x1f, 0xad, 0xcf } }



// ****************************************************
// 			Second Group of Includes
// ****************************************************


#include <nvm.h>
#include <dbgPrint.h>
#include <psd.h>
#include <collection.h>
#include <buildNbr.h>

#define  BBCIDSIG 0x27bc86aa	//BISBC_INSTANCEDATA magic number/signature value.

typedef struct _EFI_BIS_DEFAULT_AUTH_INTERFACE EFI_BIS_DEFAULT_AUTH_INTERFACE;
typedef struct _EFI_BIS_PERSISTENCE_INTERFACE  EFI_BIS_PERSISTENCE_INTERFACE;


	//
	//
	//
typedef struct _BisBaseCodeInstanceData
{
	UINTN	sizeOfThis;
	UINTN   Signature;

	//Interface pointers for BIS specific protocols.
	EFI_BIS_PERSISTENCE_INTERFACE		*Persist;
	EFI_BIS_DEFAULT_AUTH_INTERFACE		*Authorize;

	//Application memory allocation tracking.
	COLLECTION *appInfoTrackingCollection;


	//Non-volatile memory nvm.c items.
	UINT8*      flashData;           //A copy of the entire flash area.
	UINT32      flashDataLength;     //length of the data area.
	BIS_BOOLEAN flashDataChanged;    //The copy has changed.
	BIS_BOOLEAN doValidityCheck;     //Indicates whether the one time NVM validity check is
                                     // to be done in NVM_Open.
                                     
	BIS_BOOLEAN bisInUse;			 //Indicates that a BIS_Initialize has been done.
									 // No further BIS_Initialize requests will suceed
									 // until a BIS_shutdown is executed.
                                     
	EFI_BIS_PROTOCOL	bisInterface; //the public structure containing function pointers.

}
BISBC_INSTANCEDATA;


extern  EFI_BIS_PROTOCOL   *BISBC;

BISBC_INSTANCEDATA *getInstanceData( EFI_BIS_PROTOCOL *bisInterface );



//----------------------------------------------------------//
// BIS_APPINFO - this is the data structure pointed to by	//
//	the opaque BIS_APPLICATION_HANDLE						//
typedef
struct _BIS_APPINFO
{
    UINT32          	SizeOfStruct;       //size of this structure.
    UINT32          	LastError;          //errcode of last op.
    COLLECTION*     	memoryAllocations;  //tracks memory obtained by MEM_Alloc()
	NVM_AREA_HANDLE 	nvmHandle;          //used for NVM i/o ops.
    APP_CSSMINFO_PTR 	pCssmInfo;         //this apps cssm instance data.
	BISBC_INSTANCEDATA *efiInstanceData;
}
BIS_APPINFO, *BIS_APPINFO_PTR;

#define CAST_APP_HANDLE(h)   ((BIS_APPINFO*) h)
#define IS_APPINFO_VALID(i)  (i->SizeOfStruct == sizeof(BIS_APPINFO))
#define SET_LASTERROR(i,err) (i->LastError= err)


//----------------------------------------------------------//
//
// Memory mgt wrappers implemented in util\mem.c
//

void 		*MEM_malloc( BIS_APPINFO *appInfo,  UINT32 size );
BIS_BOOLEAN  MEM_free( BIS_APPINFO *appInfo,    void *memblock );
BIS_DATA_PTR MEM_allocBisData( BIS_APPINFO *appInfo,  UINT32 size );
void		 MEM_copy( UINT8 *dest, UINT8 *source, UINT32 byteLength );

void 		*x_malloc( UINT32 size );
void  		 x_free( void *memblock );
void 		*x_calloc( UINT32 num, UINT32 size );


        //
        // EFIBIS_BaseCodeModuleInit creates a COLLECTION named 'appInfoTrackingCollection'.
        // BIS_Initialize and BIS_Shutdowns us it to track BIS_APPINFO  objects.
        // These constants determine the initial size and expansion increment
        // for this collections.
#define APPINFO_COLLECTION_SIZE (16)
#define APPINFO_COLLECTION_INCR (16)


        //
        // BIS_Initialize creates a COLLECTION inside the BIS_APPINO struct
        // to track memory objects.
        // These constants determine the initial size and expansion increment
        // for these collections.

#define BIS_INIT_COLLECTION_SIZE (384)   //see note 1
#define BIS_INIT_COLLECTION_INCR (64)


//The following defs came from:
// Workfile: bis_priv.h
// Revision: 39

            //used by the dispatch function
#define     BIS_FUNCTION_MASK  (0x0000ffff)
#define     BIS_MAX_OPCODE     (BISOP_LAST&BIS_FUNCTION_MASK)


typedef UINT32  (*BIS_INTERNAL_FXN)( void* parmBlock );





			//----------------------------------------//
            //BIS API Internal FUNCTIONS Declarations //
			//----------------------------------------//

UINT32
    BIS_Initialize( BIS_INIT_PARMS *parmBlock );

UINT32
    BIS_Shutdown(   BIS_SHUTDOWN_PARMS *parmBlock );

UINT32
    BIS_Free(       BIS_FREE_PARMS *parmBlock );


UINT32
    BIS_GetBootObjectAuthorizationCertificate( BIS_GBOAC_PARMS *parmBlock );

UINT32
    BIS_GetBootObjectAuthorizationCheckFlag( BIS_GBOACF_PARMS *parmBlock );

UINT32
    BIS_GetBootObjectAuthorizationUpdateToken( BIS_GBOAUT_PARMS *parmBlock );


UINT32
    BIS_GetSigInfo( BIS_GSI_PARMS *parmBlock );


UINT32
    BIS_VerifyBootObject( BIS_VBO_PARMS *parmBlock );

UINT32
    BIS_VerifyObjectWithCredential( BIS_VOWC_PARMS *parmBlock );

UINT32
    BIS_UpdateBootObjectAuthorization( BIS_UBOA_PARMS *parmBlock );

//
// misc function prototypes.
//


BIS_STATUS
	GetSystemGuid( CSSM_GUID_PTR theSystemGuid );

EFI_STATUS
	sha1Digest(
	     BIS_APPINFO_PTR  appInfo
	    ,CSSM_DATA        dataBufs[]
	    ,UINT32           nbrDataBufs
	    ,BIS_DATA_PTR     *digestOut
	    );

int
	BIS_strcmp( UINT8 *s1, UINT8 *s2 );

int
	BIS_strncmp( UINT8 *s1, UINT8 *s2, UINT32 count);

int
	BIS_strlen( UINT8 *s1 );


//
// Define opcode symbols in terms of a efibis.h enum.

// This can probally be removed. There is a need to have all of the BISOPs
// defined ( handling incoming bis requests over the wire) Therefore all
// the BISOPs are defined at the beginning of this header file
//
//#define BISOP_UpdateBootObjectAuthorization   BisCallingFunctionUpdate
//#define BISOP_VerifyBootObject				  BisCallingFunctionVerify

BIS_STATUS
CallAuthorization(
    IN   UINT32           opCode,
    IN   BIS_DATA_PTR     credentials,
    IN   BIS_DATA_PTR     credentialsSigner,
    IN   BIS_DATA_PTR     dataObject,
    IN   BIS_DATA_PTR     reserved,
    OUT  BIS_BOOLEAN *    isAuthorized
    );



//These methods are usually accessed through the protocol interface
//but parmBlockAdaptors.c is part of the implementation of that interface
//and takes the liberty of calling the functions directly and so here are
//there prototype definitions which otherwise are undefined.

	EFI_STATUS
	EFI_BIS_GetBootObjectAuthorizationCheckFlag(
		IN  BIS_APPLICATION_HANDLE  AppHandle,        // From Initialize( ).
		OUT BOOLEAN                 *CheckIsRequired  // Value of check flag.
		);


	EFI_STATUS
	EFI_BIS_VerifyObjectWithCredential(
		IN  BIS_APPLICATION_HANDLE AppHandle,     //  From Initialize( ).
		IN  EFI_BIS_DATA           *Credentials,  //  Verification signed manifest.
		IN  EFI_BIS_DATA           *DataObject,   //  Boot object to verify.
		IN  EFI_BIS_DATA           *SectionName,  //  Name of credential section to use.
		IN  EFI_BIS_DATA           *AuthorityCertificate,  // Certificate for credentials.
		OUT BOOLEAN                *IsVerified    // Result of verifcation.
		);


	EFI_STATUS
	EFI_BIS_GetBootObjectAuthorizationCertificate(
	    IN  BIS_APPLICATION_HANDLE  AppHandle,      // From Initialize( ).
	    OUT EFI_BIS_DATA            **Certificate   // Pointer to certificate.
		);


	EFI_STATUS
	EFI_BIS_GetBootObjectAuthorizationUpdateToken(
	    IN  BIS_APPLICATION_HANDLE  AppHandle,      // From Initialize( ).
	    OUT EFI_BIS_DATA            **UpdateToken   // Pointer to update token.
		);




	//
	// BIS / EFI_BIS Status Code Mapping Functions.
	//

EFI_STATUS
mapBisToEfi(
    IN BIS_STATUS code
    );

BIS_STATUS
mapEfiToBis(
    IN EFI_STATUS code
);

EFI_STATUS  allocStatusCodeMaps();				//initialization fxn



    //-----------------------------------//
    // DEFAULT AUTH Method
    //-----------------------------------//




// Values for CallingFunction being passed in to the CheckCredentials function
typedef enum {
    BisCallingFunctionVerify,
    BisCallingFunctionUpdate
} EFI_BIS_CALLING_FUNCTION;


typedef
EFI_STATUS
(EFIAPI *EFI_BIS_DEFAULT_AUTH_CHECK_CREDENTIALS) (
    IN   EFI_BIS_DEFAULT_AUTH_INTERFACE  *This,
    IN   EFI_BIS_CALLING_FUNCTION        CallingFunction,
    IN   EFI_BIS_DATA                    *Credentials,
    IN   EFI_BIS_DATA                    *CredentialsSigner,
    IN   EFI_BIS_DATA                    *DataObject,
    IN   VOID                            *Reserved,
    OUT  BOOLEAN                         *IsAuthorized
    );

#define EFI_BIS_DEFAULT_AUTH_INTERFACE_REVISION 0x00010000

typedef struct _EFI_BIS_DEFAULT_AUTH_INTERFACE
{
    //member vars
    UINT64           Revision;
    VOID             *InstanceData;

    //Methods
    EFI_BIS_DEFAULT_AUTH_CHECK_CREDENTIALS   CheckCredentials;
}
EFI_BIS_DEFAULT_AUTH_INTERFACE;

EFI_STATUS
EFIBIS_InitAuthFxnModule(
	EFI_BIS_DEFAULT_AUTH_INTERFACE  **authInterface
    );


    //-----------------------------------//
    // EFI_BIS_PERSISTENCE_PROTOCOL
    //-----------------------------------//

#define EFI_BIS_PERSISTENCE_PROTOCOL   \
 { 0x0c63db00, 0x5429, 0x11d4, 0x98, 0x16, 0x00, 0xa0, 0xc9, 0x1f, 0xad, 0xcf }


typedef
EFI_STATUS
(EFIAPI  *EFI_BIS_PERSISTENCE_READ)
    (
    IN  EFI_BIS_PERSISTENCE_INTERFACE *This,
    OUT UINT8                         *Buffer,
    IN  VOID                          *Reserved
    );

typedef
EFI_STATUS
(EFIAPI  *EFI_BIS_PERSISTENCE_WRITE)
    (
    IN EFI_BIS_PERSISTENCE_INTERFACE *This,
    IN UINT8                         *Buffer,
    IN VOID                          *Reserved
    );

typedef
EFI_STATUS
(EFIAPI  *EFI_BIS_PERSISTENCE_GETLENGTH)
    (
    IN EFI_BIS_PERSISTENCE_INTERFACE *This,
    OUT UINT32                       *Length,
    IN  VOID                         *Reserved
    );


#define EFI_BIS_PERSISTENCE_INTERFACE_REVISION 0x00010000

typedef struct _EFI_BIS_PERSISTENCE_INTERFACE
{
    //member vars
    UINT64           Revision;
    VOID             *InstanceData;

    //Methods
    EFI_BIS_PERSISTENCE_READ           Read;
    EFI_BIS_PERSISTENCE_WRITE          Write;
    EFI_BIS_PERSISTENCE_GETLENGTH      GetLength;

}
EFI_BIS_PERSISTENCE_INTERFACE;

EFI_STATUS
EFIBIS_InitPersistModule(
    EFI_BIS_PERSISTENCE_INTERFACE   **persistInterface
    );
    

//
// Define SMBIOS tables.
//
#pragma pack(1)
typedef struct {
    UINT8   AnchorString[4];
    UINT8   EntryPointStructureChecksum;
    UINT8   EntryPointLength;
    UINT8   MajorVersion;
    UINT8   MinorVersion;
    UINT16  MaxStructureSize;
    UINT8   EntryPointRevision;
    UINT8   FormattedArea[5];
    UINT8   IntermediateAnchorString[5];
    UINT8   IntermediateChecksum;
    UINT16  TableLength;
    UINT32  TableAddress;
    UINT16  NumberOfSmbiosStructures;
    UINT8   SmbiosBcdRevision;
} SMBIOS_STRUCTURE_TABLE;

//
// Please note that SMBIOS structures can be odd byte aligned since the
//  unformated section of each record is a set of arbitrary size strings.
//

typedef struct {
    UINT8   Type;
    UINT8   Length;
    UINT8   Handle[2];
} SMBIOS_HEADER;

typedef UINT8   SMBIOS_STRING;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Vendor;
    SMBIOS_STRING   BiosVersion;
    UINT8           BiosSegment[2];
    SMBIOS_STRING   BiosReleaseDate;
    UINT8           BiosSize;
    UINT8           BiosCharacteristics[8];
} SMBIOS_TYPE0;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Manufacturer;
    SMBIOS_STRING   ProductName;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;

    //
    // always byte copy this data to prevent alignment faults!
    //
    EFI_GUID        Uuid;
    
    UINT8           WakeUpType;
} SMBIOS_TYPE1;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Manufacturer;
    SMBIOS_STRING   ProductName;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;
} SMBIOS_TYPE2;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Manufacturer;
    UINT8           Type;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;
    SMBIOS_STRING   AssetTag;
    UINT8           BootupState;
    UINT8           PowerSupplyState;
    UINT8           ThermalState;
    UINT8           SecurityStatus;
    UINT8           OemDefined[4];
} SMBIOS_TYPE3;

typedef struct {
    SMBIOS_HEADER   Hdr;
    UINT8           Socket;
    UINT8           ProcessorType;
    UINT8           ProcessorFamily;
    SMBIOS_STRING   ProcessorManufacture;
    UINT8           ProcessorId[8];
    SMBIOS_STRING   ProcessorVersion;
    UINT8           Voltage;
    UINT8           ExternalClock[2];
    UINT8           MaxSpeed[2];
    UINT8           CurrentSpeed[2];
    UINT8           Status;
    UINT8           ProcessorUpgrade;
    UINT8           L1CacheHandle[2];
    UINT8           L2CacheHandle[2];
    UINT8           L3CacheHandle[2];
} SMBIOS_TYPE4;

typedef union {
    SMBIOS_HEADER   *Hdr;
    SMBIOS_TYPE0    *Type0;
    SMBIOS_TYPE1    *Type1;
    SMBIOS_TYPE2    *Type2;
    SMBIOS_TYPE3    *Type3;
    SMBIOS_TYPE4    *Type4;
    UINT8           *Raw;
} SMBIOS_STRUCTURE_POINTER;
#pragma pack()



#endif
