/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  hash.c
  
Abstract:

  Hash table operations

Revision History

--*/

#include "fat.h"

VOID
Fat8Dot3NameToString (
  CHAR8       *File8Dot3Name,
  CHAR16      *UnicodeBuffer
  )
{
  CHAR16    *p;
  
  FatFatToStr (8, File8Dot3Name, UnicodeBuffer);
  
  for (p = UnicodeBuffer + EfiStrLen(UnicodeBuffer) - 1;
       p >= UnicodeBuffer && *p == ' ';
       p --) {
    ;
  }
  
  *(p + 1) = 0;
  
  if (! (File8Dot3Name[8] == ' ' &&
         File8Dot3Name[9] == ' ' && 
         File8Dot3Name[10]== ' ')
     ) {
    UnicodeBuffer[EfiStrLen(UnicodeBuffer) + 1] = 0;
    UnicodeBuffer[EfiStrLen(UnicodeBuffer)] = '.';
  
    FatFatToStr (3, File8Dot3Name + 8, 
                  UnicodeBuffer + EfiStrLen(UnicodeBuffer));
  
    for (p = UnicodeBuffer + EfiStrLen(UnicodeBuffer) - 1;
       p >= UnicodeBuffer && *p == ' ';
       p --) {
      ;
    }
    
    *(p + 1) = 0;
  }
}


HASH_ENTRY*
FatInsertHashNode (
  FAT_OFILE     *OFile,
  CHAR8         *String,
  UINTN         StringSize,
  UINT32        Position
  )
/*++

Routine Description:

  Insert a hash node.
  
Arguments:

  OFile     - The directory whose hash table is to be updated
  String    - The string whose hash value is to be computed (a char16 filename
              or an 8dot3 name);
  StringSize- The size of string (in bytes), including terminating double zero
              if the string is a char16 filename
  Position  - The position of the dir entry which is recorded in the hash node
  
Returns:
  
  The hash entry that is updated.
  
--*/
{
  UINTN         Index, Index1;
  UINT8         Sum;
  HASH_ENTRY    *HashEntry;
  HASH_NODE     *HashNodeBuffer;
  BOOLEAN       Inserted;
  CHAR16        *HashString;
  
  Inserted = FALSE;
  HashString = NULL;
  
  //
  // ChildHashTable already there?
  //
  if (!OFile->ChildHashTable) {
    OFile->ChildHashTable = 
      EfiLibAllocateZeroPool ((HASH_WORD_MASK + 1) * sizeof (HASH_ENTRY));
    if (!OFile->ChildHashTable) {
      return NULL;
    }    
  }
  
  //
  // Calculate Hash result
  //
  Sum = 0;
  
  if (StringSize == 11) {
    //
    // This is an 8Dot3 Name
    //
    HashString = EfiLibAllocateZeroPool (26);
    if (!HashString) {
      return NULL;
    }
    
    Fat8Dot3NameToString (String, HashString);
    FatStrUpr (HashString);
  } else {
    //
    // This is a normal char16 string
    //
    HashString = EfiLibAllocateZeroPool (StringSize);
    if (!HashString) {
      return NULL;
    }
    EfiStrCpy (HashString, (CHAR16*)String);
    FatStrUpr (HashString);
  }
  
  for (Index = 0; Index < EfiStrLen(HashString); Index++) {
    Sum = (UINT8)(Sum + HashString[Index]);
  }
  
  Sum = (UINT8)(Sum & HASH_WORD_MASK);
  HashEntry = &OFile->ChildHashTable[Sum];
  
  //
  // See if there is a match node already
  //
  for (Index = 0; Index < HashEntry->CountOfNodes; Index ++) {
    if (HashEntry->HashNode[Index].Position == Position) {
      gBS->FreePool (HashString);
      return HashEntry;
    }
  }
  
  if (!HashEntry->HashNode ||
      HashEntry->CountOfNodes + 1 > HashEntry->NodeBufferSize) {
    
    //
    // Have to allocate new storage
    //
    
    HashNodeBuffer = 
      EfiLibAllocateZeroPool ( sizeof (HASH_NODE) 
      * (HashEntry->NodeBufferSize + HASH_NODE_BLOCK_UNIT) );
    if (!HashNodeBuffer) {
      gBS->FreePool (HashString);
      return NULL;
    }    
  
    Inserted = FALSE;
    for (Index = 0, Index1 = 0; Index < HashEntry->CountOfNodes;) {
      if (HashEntry->HashNode[Index].Position < Position || Inserted) {
        HashNodeBuffer[Index1].Position = HashEntry->HashNode[Index].Position;
      } else {
        
        //
        // If there are original hash nodes, always enter here once
        // since there is a HASH_NODE_TERMINATOR
        //
        HashNodeBuffer[Index1].Position = Position;
        Inserted = TRUE;
        Index1 ++;
        HashNodeBuffer[Index1].Position = HashEntry->HashNode[Index].Position;
      }
      Index1++;
      Index++;
    }
    
    if (!Inserted) {
      //
      // No original Hash nodes
      //
      HashNodeBuffer[0].Position = Position;
      HashNodeBuffer[1].Position = HASH_NODE_TERMINATOR;      
    }
    
    if (HashEntry->HashNode) {
      gBS->FreePool (HashEntry->HashNode);
    }
    
    HashEntry->HashNode = HashNodeBuffer;    
    if (HashEntry->CountOfNodes > 0) {
      HashEntry->CountOfNodes = HashEntry->CountOfNodes + 1;
    } else {
      HashEntry->CountOfNodes = 2;
    }
    
    HashEntry->NodeBufferSize = 
      HashEntry->NodeBufferSize + HASH_NODE_BLOCK_UNIT;
    
  } else {
    
    //
    // Insert within original storage
    //
    
    for (Index = HashEntry->CountOfNodes; Index > 0; Index --) {
      HashEntry->HashNode[Index].Position = 
        HashEntry->HashNode[Index-1].Position;
      if (HashEntry->HashNode[Index].Position <= Position) {
        break;
      }            
    }
    HashEntry->HashNode[Index].Position = Position;
    HashEntry->CountOfNodes ++;
  }
  
  if (OFile->HashTopPosition < Position) {
    OFile->HashTopPosition = Position;
  }
  
  gBS->FreePool (HashString);
  return HashEntry;
}

VOID
FatRemoveHashNode (
  HASH_ENTRY    *Entry,
  UINT32        Position
  )
/*++

Routine Description:

  Remove a hash node.
  
Arguments:

  Entry       - the hash entry whose node is to be deleted
  Position    - the node which is labeled with Position is to be deleted
  
Returns:

  Void
  
--*/
{
  UINTN       Index, Index1;
  UINT32      OrigCountOfNodes;
  HASH_NODE   *NodeBuffer;
  
  OrigCountOfNodes = Entry->CountOfNodes;
  for (Index = 0, Index1 = 0; Index1 < OrigCountOfNodes;) {
    if (Entry->HashNode[Index1].Position == Position) {
      Entry->CountOfNodes --;
      Index1 ++;
    } else {
      Entry->HashNode[Index] = Entry->HashNode[Index1];
      Index ++;
      Index1 ++;
    }
  }
  
  if (Entry->CountOfNodes <= Entry->NodeBufferSize - HASH_NODE_BLOCK_UNIT) {
    if (Entry->CountOfNodes) {
      //
      // Shrink buffer
      //
      NodeBuffer = EfiLibAllocateZeroPool (
                      sizeof (HASH_NODE)*
                      (Entry->NodeBufferSize - HASH_NODE_BLOCK_UNIT)
                      );
      if (NodeBuffer) {
        EfiCopyMem (NodeBuffer,
                    Entry->HashNode,
                    Entry->CountOfNodes * sizeof (HASH_NODE)
                    );
        gBS->FreePool (Entry->HashNode);
        Entry->HashNode = NodeBuffer;
        Entry->NodeBufferSize = Entry->NodeBufferSize - HASH_NODE_BLOCK_UNIT;
      }
    } else {
      //
      // Nodes are cleaned up, just free
      //
      gBS->FreePool (Entry->HashNode);
      Entry->HashNode = NULL;
      Entry->NodeBufferSize = 0;
    }    
  }  
}
  
FAT_OFILE*
FatHashSearch (
  FAT_OFILE     *OFile,
  CHAR16        *Name, 
  UINT32        PreferedPosition
  )
/*++

Routine Description:

  Search hash table for a matching node
  
Arguments:

  OFile       - The directory whose hash table is to be searched
  Name        - The file name
  PreferedPosition - The position which is expected to be searched for first
  
Returns:

  The target Ofile. (NULL indicates Not Found)

--*/
{
  EFI_STATUS    Status;
  UINT8         Sum;
  HASH_ENTRY    *HashEntry;
  UINTN         Index, RefIndex;
  BOOLEAN       FirstRound;
  FAT_OFILE     *Entry;
  UINT64        Position;
  CHAR16        *HashString;

  Status = EFI_SUCCESS;
  Entry = NULL;
  HashString = NULL;

  if (!OFile->ChildHashTable) {
    return NULL;
  }
  
  HashString = EfiLibAllocateZeroPool (EfiStrSize (Name));
  if (!HashString) {    
    return NULL;
  }
  EfiStrCpy (HashString, Name);
  FatStrUpr (HashString);
  
  //
  // Calculate Hash result
  //
  
  Sum = 0;
  
  for (Index = 0; Index < EfiStrLen(HashString); Index++) {
    Sum = (UINT8)(Sum + HashString[Index]);
  }
  Sum = (UINT8)(Sum & HASH_WORD_MASK);
  HashEntry = &OFile->ChildHashTable[Sum];

  if (HashEntry->CountOfNodes == 0) {
    gBS->FreePool (HashString);
    return NULL;
  }
  
  for (Index = 0; Index < HashEntry->CountOfNodes; Index++) {
    if (HashEntry->HashNode[Index].Position > PreferedPosition) {
      break;
    }    
  }
  if (Index > 0) {
    Index --;
  }
  
  RefIndex = Index;
  FirstRound = TRUE;  
  for (;;) {
    if (HashEntry->HashNode[Index].Position == HASH_NODE_TERMINATOR && FirstRound) {
      FirstRound = FALSE;
      Index = 0;
      continue;
    }    
    if (!FirstRound && Index >= RefIndex) {
      Status = EFI_NOT_FOUND;
      break;
    }
    
    Position = HashEntry->HashNode[Index].Position;
    Status = FatGetDirOFile (OFile, &Position, &Entry);
    if (EFI_ERROR(Status)) {
      Status = EFI_NOT_FOUND;
      break;
    }
    
    //
    // If this isn't a file or directory, skip it
    //
    if (Entry->DirType != IsFile && Entry->DirType != IsDir) {
      //
      // This is not what we want.
      // Try to free this OFile if it is not referenced.
      //
      FatCheckOFileRef (Entry);
      Index ++;
      continue;
    }

    //
    // If the filename matches, we're done
    //        
    if (FatMatchFileName (Entry, Name)) {
      OFile->CurrentEntryPos = Entry->DirPosition;
      Status = EFI_SUCCESS;
      break;
    }

    //
    // This is not what we want.
    // Try to free this OFile if it is not referenced.
    //
    FatCheckOFileRef (Entry);

    Index ++;
  }

  gBS->FreePool (HashString);
  if (EFI_ERROR(Status)) {
    return NULL;
  } else {
    return Entry;
  }  
}

