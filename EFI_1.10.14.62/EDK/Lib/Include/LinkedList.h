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

  LinkedList.h

Abstract:

  Macros to support doubly linked lists

  The Head of the list is an EFI_LIST_ENTRY. It points to the list elements, 
  but it is not a list element. Each link eliment is an EFI_LIST_ENTRY that 
  contains a forward and back pointer for the list.

  You can place the EFI_LIST_ENTRY in your structure that needs to 
  be in a linked list and use the Containment Record, CR(), macro to
  get from an EFI_LIST_ENTRY to the private data.

  Example of useing Linked List to get to the private data

  #define PRIVATE_DATA_SIGNATURE EFI_SIGNATURE_32('P','S','i','g')
  typedef struct {
    UINT32          Signature;

    EFI_LIST_ENTRY  Link;

    PRIVATE_INFO    PrivateData;
  } PRIVATE_DATA;

  #define PRIVATE_DATA_FROM_LINK(_link)  \
            CR(_link, PRIVATE_DATA, Link, PRIVATE_DATA_SIGNATURE)

  
  EFI_LIST_ENTRY  *Link;
  PRIVATE_DATA    *PrivateDataPtr;

  ...
  PrivateDataPtr = PRIVATE_DATA_FROM_LINK (Link);
  ...

  A list is declared as an EFI_LIST_ENTRY reguardless of the data type:

  EFI_LIST_ENTRY  ListHead;   // List of PRIVATE_DATA

  You must perform an InitializeListHead (&ListHead); prior to using the list.

--*/

//
// If the Utility driver lib has been included, don't use this version
//
#ifndef _LINK_H

#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_


typedef struct _EFI_LIST_ENTRY {
  struct _EFI_LIST_ENTRY  *ForwardLink;
  struct _EFI_LIST_ENTRY  *BackLink;
} EFI_LIST_ENTRY;

//
//  VOID
//  InitializeListHead (
//    EFI_LIST_ENTRY *ListHead
//    );
/*++

Routine Description:

  Initialize ListHead. An ListHead points to it's self. The caller must 
  allocate the memory for ListHead. This function must be called before
  the other linked list macros can be used.
    
Arguments:

  ListHead - Pointer to list head to initialize

   
Returns:

  None.

--*/


#define InitializeListHead(ListHead)          \
          (ListHead)->ForwardLink = ListHead; \
          (ListHead)->BackLink = ListHead;

#define INITIALIZE_LIST_HEAD(ListHead)  InitializeListHead (ListHead)

//
//  BOOLEAN
//  IsListEmpty (
//    EFI_LIST_ENTRY  *ListHead
//    );
/*++

Routine Description:

  Return TRUE is the list contains zero nodes. Otherzise return FALSE.
  The list must have been initialized with InitializeListHead () before using 
  this function.
    
Arguments:

  ListHead - Pointer to list head to test

   
Returns:

  Return TRUE is the list contains zero nodes. Otherzise return FALSE.

--*/

#define IsListEmpty(ListHead) \
          ((ListHead)->ForwardLink == (ListHead))

#define IS_LIST_EMPTY(ListHead) IsListEmpty(ListHead)

//
//  VOID
//  RemoveEntryList (
//    EFI_LIST_ENTRY *Entry
//    );
/*++

Routine Description:

  Remove Entry from the doubly linked list. It is the callers responcibility
  to free any memory used by the entry if needed. The list must have been 
  initialized with InitializeListHead () before using this function.
    
Arguments:

  Entry - Element to remove from the list.
   
Returns:
  
  None

--*/
#define _RemoveEntryList(Entry) {                   \
          EFI_LIST_ENTRY *_BackLink, *_ForwardLink; \
          _ForwardLink = (Entry)->ForwardLink;      \
          _BackLink = (Entry)->BackLink;            \
          _BackLink->ForwardLink = _ForwardLink;    \
          _ForwardLink->BackLink = _BackLink;       \
          }

#ifdef EFI_DEBUG
    #define RemoveEntryList(Entry)                                        \
              _RemoveEntryList(Entry);                                    \
              (Entry)->ForwardLink = (EFI_LIST_ENTRY *) EFI_BAD_POINTER;  \
              (Entry)->BackLink = (EFI_LIST_ENTRY *) EFI_BAD_POINTER; 
#else
    #define RemoveEntryList(Entry)  \
              _RemoveEntryList(Entry);
#endif

#define REMOVE_ENTRY_LIST(Entry)  RemoveEntryList(Entry)

//
//  VOID
//  InsertTailList (
//    EFI_LIST_ENTRY  *ListHead,
//    EFI_LIST_ENTRY  *Entry
//    );
/*++

Routine Description:

  Insert an Entry into the end of a doubly linked list. The list must have 
  been initialized with InitializeListHead () before using this function.
    
Arguments:

  ListHead - Head of doubly linked list

  Entry    - Element to insert in the list from the list.
   
Returns:
  
  None

--*/

#define InsertTailList(ListHead,Entry) {          \
          EFI_LIST_ENTRY *_ListHead, *_BackLink;  \
          _ListHead = (ListHead);                 \
          _BackLink = _ListHead->BackLink;        \
          (Entry)->ForwardLink = _ListHead;       \
          (Entry)->BackLink = _BackLink;          \
          _BackLink->ForwardLink = (Entry);       \
          _ListHead->BackLink = (Entry);          \
          }

#define INSERT_TAIL_LIST(ListHead, Entry) InsertTailList(ListHead, Entry)

//
//  VOID
//  InsertHeadList (
//    EFI_LIST_ENTRY  *ListHead,
//    EFI_LIST_ENTRY  *Entry
//    );
/*++

Routine Description:

  Insert an Entry into the start of a doubly linked list. The list must have 
  been initialized with InitializeListHead () before using this function.
    
Arguments:

  ListHead - Head of doubly linked list

  Entry    - Element to insert in the list from the list.
   
Returns:
  
  None

--*/

#define InsertHeadList(ListHead,Entry) {            \
          EFI_LIST_ENTRY *_ListHead, *_ForwardLink; \
          _ListHead = (ListHead);                   \
          _ForwardLink = _ListHead->ForwardLink;    \
          (Entry)->ForwardLink = _ForwardLink;      \
          (Entry)->BackLink = _ListHead;            \
          _ForwardLink->BackLink = (Entry);         \
          _ListHead->ForwardLink = (Entry);         \
          }

#define INSERT_HEAD_LIST(ListHead, Entry) InsertHeadList(ListHead, Entry)

//  VOID
//  SwapListEntries (
//    EFI_LIST_ENTRY  *Entry1,
//    EFI_LIST_ENTRY  *Entry2
//    );
/*++

Routine Description:

  Swap the location of the two elements of a doubly linked list. Entry2 
  is placed in front of Entry1. The list must have been initialized with 
  InitializeListHead () before using this function.
    
Arguments:

  Entry1 - Element in the doubly linked list in front of Entry2. 

  Entry2 - Element in the doubly linked list behind Entry1.
   
Returns:
  
  None

--*/

#define SwapListEntries(Entry1,Entry2) {                      \
          EFI_LIST_ENTRY *Entry1ForwardLink, *Entry1BackLink; \
          EFI_LIST_ENTRY *Entry2ForwardLink, *Entry2BackLink; \
          Entry2ForwardLink = (Entry2)->ForwardLink;          \
          Entry2BackLink = (Entry2)->BackLink;                \
          Entry1ForwardLink = (Entry1)->ForwardLink;          \
          Entry1BackLink = (Entry1)->BackLink;                \
          Entry2BackLink->ForwardLink = Entry2ForwardLink;    \
          Entry2ForwardLink->BackLink = Entry2BackLink;       \
          (Entry2)->ForwardLink = Entry1;                     \
          (Entry2)->BackLink = Entry1BackLink;                \
          Entry1BackLink->ForwardLink = (Entry2);             \
          (Entry1)->BackLink = (Entry2);                      \
          }

#define SWAP_LIST_ENTRIES(Entry1, Entry2)   SwapListEntries(Entry1, Entry2) 

#endif

#endif
