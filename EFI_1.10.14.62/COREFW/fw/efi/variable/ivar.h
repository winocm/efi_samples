/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:


Abstract:




Revision History

--*/


#include "efifw.h"
#include "efivar.h"

//
// Storage bank header
//

#define BANK_HEADER_SIGNATURE       EFI_SIGNATURE_32('v','b','r','l')
#define VARH_FORMAT                 0x00    

typedef struct {
    UINT32                  Signature;      
    UINT16                  Sequence;       
    UINT8                   Format;
    UINT8                   State;
} BANK_HEADER;

#define VARH_ADD_TRANSITION         0x01    // Bank being added
#define VARH_INUSE_TRANSITION       0x02    // Bank being transitioned too
#define VARH_INUSE                  0x04    // Bank has valid data in it
#define VARH_OBSOLETE_TRANSITION    0x08    // Bank being removed

#define VARH_MAX_SEQUENCE           0xFFFF

//
//  Variables are stored in the following packed stream format
//  in each bank:
//
//      [----------- Variable #1 -----------]       [-------Variable #2...                         
//      [size] [guid] [name] 0 [data] [state] [pad] [size] ...
//         2     16      n   2    n      1    ^
//      |                                     |
//      +---- size ---------------------------+
//
//  Note the State is stored at the end to make garbage collection 
//  easier.  Also note that the state bits are stored inverted to
//  make reading the code easier.
//
//  A pad is implied between variable entries to align the next 
//  variable entry to 4 byte boudry. 
//
//  Note the sizeof BANK_HEADER has the proper initialize alignment.
//

typedef struct _VARIABLE {
    UINT16                  Size;
    UINT16                  Attributes;
    EFI_GUID                VendorGuid;
} VARIABLE;

#define FIRST_VARIABLE_OFFSET   (sizeof(BANK_HEADER))

#define VAR_SIZE_MASK               0x0FFF
#define VAR_MAX_SIZE                1024

// State flags
#define VAR_OBSOLETE_TRANSITION     0x01        // Entry is in transistion to be obsolete
#define VAR_OBSOLETE                0x02        // Entry is obsolete
#define VAR_ADDED                   0x80        // Entry has been completely added
#define VAR_VALID_BITS              0x83

//
// All valid attribute bits that may be supplied on SetVariable
//

#define EFI_VARIABLE_VALID_ATTRIBUTES               \
            (EFI_VARIABLE_NON_VOLATILE          |   \
             EFI_VARIABLE_BOOTSERVICE_ACCESS    |   \
             EFI_VARIABLE_RUNTIME_ACCESS)

//
// Track each storage bank
//

#define STORAGE_BANK_SIGNATURE      EFI_SIGNATURE_32('s','b','n','k')
typedef struct {
    UINTN                   Signature;      
    struct _VARIABLE_STORE  *VarStore;
    LIST_ENTRY              Link;           // VARIABLE_STORE.Banks

    UINT8                   State;          // Inverted bank state
    UINTN                   BankNo;         // Which bank this is
    UINTN                   BankSize;       // The size of the bank
    EFI_VARIABLE_STORE      *Device;        // Access functions

    BOOLEAN                 DeviceError;    // Device error while accessing the bank

    UINTN                   InUse;          // Space in use by active variables
    UINTN                   Tail;           // Data tail pointer
    UINTN                   *TSize;         // Current TBank size

    union {
        CHAR8               *Data;
        BANK_HEADER         *Header;        // Memory copy of the data
    } u;

} STORAGE_BANK;


//
// Variable storage by storage type
//

typedef struct _VARIABLE_STORE {
    struct _VARIABLE_STORE  *Next;          // Next store
    LIST_ENTRY              Active;         // List of all storage banks

    EFI_MEMORY_TYPE         MemoryType;     // Memory type of this store
    UINT32                  Attributes;     // Attributes for this store 
    CHAR8                   *Type;          // Desc of this store (debug)
    BOOLEAN                 RuntimeUpdate;  // Store can be updated at runtime

    BOOLEAN                 TBankRequired;  // Store requires a TBank
    UINT16                  Sequence;       // Current best TBank sequence #

    STORAGE_BANK            *TBank;         // Transaction bank
    UINTN                   TSize;          // Transaction bank's size
} VARIABLE_STORE;

#define VAR_ATTRIBUTE_TYPE          0x7     // mask to turn attribute into type
#define MAX_VARIABLE_STORAGE_TYPE     4

//
//
//

typedef struct {
    BOOLEAN                 Valid;          // Info is valid
    UINT8                   State;          // Inverted state of variable from store

    STORAGE_BANK            *Bank;          // The bank the variable is from
    UINTN                   VarOffset;      // Offset within bank to variable 
    UINTN                   VarSize;        // The size of the variable in the bank (includes pad)
    UINTN                   NameOffset;     // Offset within bank to the name
    UINTN                   DataOffset;     // Offset within bank to the data
    UINTN                   StateOffset;    // Offset within bank to state value
    UINTN                   NextVarOffset;  // Offset within bank to the next variable

    UINT32                  Attributes;     // Variables attributes
    EFI_GUID                *VendorGuid;    // Vendor guid of the variable
    UINTN                   NameSize;       // Size (in bytes) of Name
    CHAR16                  *Name;          // NULL terminated name of the variable
    UINTN                   DataSize;       // Size of the variables data
    CHAR8                   *Data;          // The variables data
} VARIABLE_INFO;


//
// Internal functions
//

VOID
INTERNAL
InitNvVarStoreVirtual (
    IN UINTN    Attributes,
    IN UINTN    BankSize,
    IN UINTN    NoBanks
    );

VOID
INTERNAL RUNTIMEFUNCTION
RtAcquireStoreLock (
    VOID
    );

VOID
INTERNAL RUNTIMEFUNCTION
RtReleaseStoreLock (
    VOID
    );

VOID
INTERNAL
VarAddDevice (
    EFI_VARIABLE_STORE  *Device
    );

VOID
INTERNAL
VarCheckBanks (
    VOID
    );

INTERNAL
EFI_STATUS
VarStoreAddBank (
    IN VARIABLE_STORE       *VarStore,
    IN EFI_VARIABLE_STORE   *Device,
    IN UINTN                BankNo
    );

EFI_STATUS
INTERNAL RUNTIMEFUNCTION
RtVarClearStore (
    IN STORAGE_BANK         *Bank
    );

VOID *
INTERNAL RUNTIMEFUNCTION
RtVarReadStore (
    IN STORAGE_BANK         *Bank, 
    IN UINTN                Offset,
    IN UINTN                Size
    );

VOID
INTERNAL RUNTIMEFUNCTION
RtVarUpdateStore (
    IN STORAGE_BANK         *Bank, 
    IN UINTN                Offset,
    IN UINTN                Size
    );

BOOLEAN
INTERNAL RUNTIMEFUNCTION
RtVarParseStore (
    IN OUT VARIABLE_INFO    *VarInfo
    );

BOOLEAN
INTERNAL RUNTIMEFUNCTION
RtVarUpdateState (
    IN VARIABLE_INFO        *VarInfo,
    IN UINTN                NewBit
    );

VOID
INTERNAL RUNTIMEFUNCTION
RtVarUpdateBankState (
    IN STORAGE_BANK         *Bank,
    IN UINTN                NewBit
    );

VOID
INTERNAL RUNTIMEFUNCTION
RtVarBankError (
    IN STORAGE_BANK         *Bank
    );


VOID
INTERNAL RUNTIMEFUNCTION
RtVarMove (
    IN BOOLEAN                      BankTransition,
    IN VARIABLE_INFO                *VarInfo,
    IN STORAGE_BANK                 *NewBank
    );

BOOLEAN
INTERNAL RUNTIMEFUNCTION
RtVarGarbageCollect (
    IN VARIABLE_STORE               *VarStore
    );

VOID
INTERNAL RUNTIMEFUNCTION
RtVarGarbageCollectBank (
    IN STORAGE_BANK         *OldBank
    );

VOID
INTERNAL RUNTIMEFUNCTION
RtVarGetTransactionBank (
    IN VARIABLE_STORE       *VarStore
    );

EFI_STATUS
INTERNAL RUNTIMEFUNCTION
RtVarFind (
    IN CHAR16               *Name,
    IN EFI_GUID             *VendorGuid,
    OUT VARIABLE_INFO       *VarInfo
    );

STORAGE_BANK *
STATIC RUNTIMEFUNCTION
RtVarFindFreeSpace (
    IN VARIABLE_STORE           *VarStore,
    IN UINTN                    Size,
    OUT BOOLEAN                 *GarbageCollected OPTIONAL
    );

//
// Globals
//

extern FLOCK VariableStoreLock;
extern VOID *VariableStoreReg;
extern VARIABLE_STORE VariableStore[MAX_VARIABLE_STORAGE_TYPE];
extern VARIABLE_STORE *VariableStoreType[VAR_ATTRIBUTE_TYPE+1];
