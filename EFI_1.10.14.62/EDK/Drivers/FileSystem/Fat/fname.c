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

  fname.c
  
Abstract:

  Functions for manipulating file names

Revision History

--*/

#include "fat.h"


UINT8
FatDirEntryChecksum (
  FAT_DIRECTORY_ENTRY *Dir
  )
{
  UINT8       Check;
  UINTN       Index;

  //
  // Computes checksum for LFN
  //
  Check = 0;
  for (Index=0; Index < 11; Index++) {
    Check = (UINT8)(((Check & 1) ? 0x80 : 0) + 
                    (Check >> 1) + 
                    Dir->FileName[Index]
                    );
  }

  return Check;
}


BOOLEAN
FatIsDotEntry (
  IN FAT_OFILE        *OFile
  )
{
  //
  // Returns TRUE if OFile is either the "." or ".." entry
  //
  return (BOOLEAN)(
                   OFile->DirType == IsDir && 
                   (!EfiStrCmp(OFile->FileString, L".") ||
                    !EfiStrCmp(OFile->FileString, L"..")
                   )
                  );
}


STATIC
UINT8
FatCheckNameCase (
  IN CHAR16           *Str,
  IN UINT8            StatusLower,
  IN OUT BOOLEAN      *Lfn
  )
// Used by fatname to dir entry count
{
  CHAR16              Buffer[20];
  UINT8               Status;

  Status = 0;
  if (EfiStrLen(Str) >= 20) {
    return Status;
  }

  //
  // Lower case a copy of the string, if it matches the
  // original then the string is lower case
  //

  EfiStrCpy (Buffer, Str);
  FatStrLwr(Buffer);
  if (EfiStrCmp (Str, Buffer) == 0) {
    Status = StatusLower;
    *Lfn = TRUE;
  }

  //
  // Upper case a copy of the string, if it matches the
  // original then the string is upper case
  //

  EfiStrCpy (Buffer, Str);
  FatStrUpr (Buffer);
  if (EfiStrCmp (Str, Buffer) != 0  &&  Status == 0) {
    *Lfn = TRUE;
  }

  return Status;
}

     

UINTN
FatNameToDirEntryCount (
  IN CHAR16           *Name,
  OUT UINT8           *CaseFlag
  )
// Returns the # of directory entries needed for the file
// (Which is determined by the file's name)
{    
  CHAR16              *Ext, *End;
  BOOLEAN             Lfn;
  UINT8               Flags;
  UINTN               Len, Entries, Count;
  CHAR16              NameBuffer[20];
  CHAR8               TempBuffer[8];
  CHAR16              Converted[20];

  Lfn = FALSE;
  Flags = 0;
  *CaseFlag = 0;

  // If name is > 8 chars, then Lfn needed
  for (Ext = Name; *Ext && *Ext != '.'; Ext++) {
    ;
  }
  
  Len = Ext - Name;

  if (Len > 8) {
    Lfn = TRUE;
  } else {
    EfiCopyMem (NameBuffer, Name, Len * sizeof(CHAR16));
    NameBuffer[Len] = 0;
  }

  // If ext > 3 chars or anymore dots, then Lfn needed
  if (*Ext) {
    Ext += 1;
    for (End = Ext; *End && *End != '.'; End++) ;
    if (End - Ext > 3 || *End == '.') {
      Lfn = TRUE;
    }
  }

  //
  // If name looks good so far, convert it to a fat name to see if fits
  //

  if (!Lfn) {
    if (FatStrToFatLen(NameBuffer) > 8) {
      Lfn = TRUE;
    }
  
    if (!Lfn) {
      if (FatStrToFatLen(Ext) > 3) {
        Lfn = TRUE;
      }
    }
    
    if (!Lfn) {      
      EfiSetMem (TempBuffer, 8, ' ');
      FatStrToFat(NameBuffer, 8, TempBuffer);
      for (Count = 8; Count > 0 && TempBuffer[Count-1]==' '; Count--) {
        ;
      }
      
      FatFatToStr(Count, TempBuffer, Converted);
      if (FatStriCmp(NameBuffer, Converted)) {
        Lfn = TRUE;
      }      
    }
    
    if (!Lfn) {
      EfiSetMem (TempBuffer, 8, ' ');
      FatStrToFat(Ext, 3, TempBuffer);
      for (Count = 3; Count > 0 && TempBuffer[Count-1]==' '; Count--) {
        ;
      }
      
      FatFatToStr(Count, TempBuffer, Converted);
      if (FatStriCmp(Ext, Converted)) {
        Lfn = TRUE;
      }      
    }
    
    //
    // If the case is mixed then we store an Lfn; however, if the
    // only reason to add an Lfn is to just get the case correct we
    // set an internal flag to indicate that the 8.3 name is otherwise OK.
    //

    if (!Lfn) {
      *CaseFlag |= FAT_CASE_ONLY;
      *CaseFlag |= FatCheckNameCase (NameBuffer, FAT_CASE_NAME_LOWER, &Lfn);
      *CaseFlag |= FatCheckNameCase (Ext, FAT_CASE_EXT_LOWER, &Lfn);
    }
  }    
  
  //
  // Always need one dir entry
  //

  Entries = 1;

  //
  // If we need Lfn info as well, add the dir entries needed for that
  //

  if (Lfn) {
    Entries = Entries + (EfiStrLen(Name) + 12) / 13;
  }


  return Entries;
}



CHAR16 *
FatGetNameComp (
  IN CHAR16    *Path,
  OUT CHAR16   *Name
  )
// Takes Path as input, returns the next name component
// in Name, and returns the position after Name (e.g., the
// start of the next name component)
{
  UINTN                Len;

  Len = 0;

  while (*Path && *Path != '\\') {
    if (Len < EFI_FILE_STRING_SIZE-1) {
      Name[Len] = *Path;
      Len += 1;
    }

    Path += 1;
  }

  Name[Len] = 0;

  //
  // Get off of trailing \
  //

  while (*Path == '\\') {
    Path += 1;
  }

  return Path;
}


EFI_STATUS
FatGenerate8Dot3Name (
  IN FAT_VOLUME           *Vol,
  IN FAT_OFILE            *Parent,
  IN FAT_OFILE            *OFile
  )
// N.B. We only get here in the case where EFI is creating a file
// and the 8.3 name needs to be determined for the name
{
  CHAR16                  *IName, *Ext;
  CHAR8                   *OName;
  UINT64                  Position;
  FAT_OFILE               *Entry;
  EFI_STATUS              Status;
  UINT8                   *InUse;
  UINTN                   Index;
  UINTN                   accum;
  CHAR16                  AccumHex[17];
  UINTN                   Width;
  CHAR16                  Str1[14], Str2[14];
  CHAR16                  Str1Ext[4], Str2Ext[4];
  CHAR16                  *p1, *p2;
  CHAR16                  c;

  //
  // 8.3 names are space padded
  //

  EfiSetMem (OFile->File8Dot3Name, sizeof(OFile->File8Dot3Name), ' ');
  IName = OFile->FileString;
  OName = OFile->File8Dot3Name;
  InUse = NULL;
  Status = EFI_SUCCESS;

  //
  // If this is a '.' or '..' name then it's a special form
  //

  if (IName[0] == '.' && IName[1] == 0) {
    OName[0] = '.';
    goto Done;
  }

  if (IName[0] == '.' && IName[1] == '.' && IName[2] == 0) {
    OName[0] = '.';
    OName[1] = '.';
    goto Done;
  }
  
  //
  // Find the last '.'
  //

  Ext = NULL;
  for (p1 = IName; *p1; p1++) {
    if (*p1 == '.') {
      Ext = p1;    
    }
  }

  if (!Ext) {
    Ext = p1;
  }

  //
  // Create 8.3 name.  Convert chars to fat values.  Skip
  // any '.' or ' '.
  //

  c = *Ext;
  *Ext = 0;
  FatStrToFat (IName, 8, OName);

  *Ext = c;
  FatStrToFat (Ext, 3, OName+8);

  //
  // If we need not a long name or the only reason to add a long name is to just 
  // get the case correct, we need not to append a numeric-tail. So, we've done.
  //
  if ((OFile->DirCount == 1) || (OFile->CaseFlag & FAT_CASE_ONLY)) {
    if (*OName == ' ') {
      *OName = '1';
    }
    goto Done;
  }

  Status = EFI_OUT_OF_RESOURCES;

  //
  // This name was generated from a long name that doesn't map to
  // an 8.2 name. We need to at least append a ~digit to the first 
  // 6 chars and then otherwise change the name to make sure it's 
  // unique.
  //
  // We will scan the directory once and find the first version
  // of the name with an appended number that is not in use for
  // the first 4096 values.  If they are all in use, then we
  // will error. 
  //
  // N.B. this differs from what Windows does but the only 
  // requirement is to derive a unique name and record it properly.
  //

  //
  // Allocate an allocation bit map
  //

  InUse = EfiLibAllocateZeroPool(MAX_LFN_NUMBER / 8);
  if (!InUse) {
    goto Done;
  }
  
  //
  // Convert the name to unicode so we don't have
  // to deal with dbsc compares
  //

  FatFatToStr (8, OName, Str1);
  FatFatToStr (3, OName+8, Str1Ext);
  
  //
  // Strip off trailing blanks
  //
  for (Index = EfiStrLen(Str1) - 1;
       (INTN)Index >= 0 && Str1[Index] == ' ';
       Index -- ) {
    ;  
  }
  
  Str1[Index + 1] = '\0';
  
  for (Index = EfiStrLen(Str1Ext) - 1;
       (INTN)Index >= 0 && Str1Ext[Index] == ' ';
       Index --) {
    ;
  }
  
  Str1Ext[Index + 1] = '\0';

  //
  // Read all the entries
  //

  Position = 0;
  for (; ;) {
    Status = FatGetDirOFile (Parent, &Position, &Entry);
    if (EFI_ERROR(Status) || Entry->IsBlank) {
      break;
    }
  
    //
    // If this entry has a name we care about?
    //

    if (Entry->DirType == IsFile || Entry->DirType == IsDir) {

      //
      // Only care about the file name, not the extension
      //

      FatFatToStr (8, Entry->File8Dot3Name, Str2);
      FatFatToStr (3, Entry->File8Dot3Name+8, Str2Ext);
      
      //
      // Strip off trailing Blanks
      //
      for (Index = EfiStrLen(Str2) - 1;
           (INTN)Index >= 0 && Str2[Index] == ' ';
           Index --) {
        ;
      }
      
      Str2[Index + 1] = '\0';

      for (Index = EfiStrLen(Str2Ext) - 1;
           (INTN)Index >= 0 && Str2Ext[Index] == ' ';
           Index --) {
        ;
      }
      
      Str2Ext[Index + 1] = '\0';

      //
      // If extensions don't match, then skip it
      //

      if (EfiStrCmp(Str1Ext, Str2Ext) != 0) {
        continue;
      }

      //
      // See if this name has a similar base name
      //

      p1 = Str1;
      p2 = Str2;
      c = 0;
      while (*p1 && *p1 == *p2) {
        if (c != '~') {
          c = *p1;
        }        
        p1 += 1;
        p2 += 1;
      }
      
      if (*p2 != '~' && c != '~') {
        
        //
        // this is a different base name
        //
        continue;
      }
      
      if (c == '~') {
        
        //
        // We have encountered some '~', should set p2 to the last '~'
        //
        for (p2 = Str2 + EfiStrLen(Str2) - 1;
             p2 >= Str2 && *p2 != '~';
             p2 --) {
          ;
        }
        
      }
      
      p2 ++;   

      //
      // It looks like it could be a match.  If the rest
      // of the name is a valid number that we would generate,
      // remember it so we will pick a different number.
      //
      // If the rest of the name isn't something we could
      // generate, just call it a zero which is not used.
      //

      accum = 0;
      while (*p2) {
        if (*p2 >= '0' && *p2 <= '9') {
          accum = (accum << 4) + *p2 - '0';
        } else if (*p2 >= 'A' && *p2 <= 'F') {
          accum = (accum << 4) + *p2 - 'A' + 10;
        } else {
          accum = 0;
          break;
        }

        p2 += 1;
      }

      //
      // Mark this # as in use
      //

      if (accum < MAX_LFN_NUMBER) {
        InUse[accum/8] |= 1 << (accum % 8);
      }
    }
  }


  //
  // Find the first value not in use
  //
  InUse[0] |= 1;
  for (Index=0; Index < MAX_LFN_NUMBER/8; Index++) {
    if (InUse[Index] != 0xFF) {
      break;
    }
  }

  if (Index < MAX_LFN_NUMBER/8) {
    accum = Index * 8;
    while (InUse[Index] & 1) {
      accum = accum + 1;
      InUse[Index] = (UINT8)(InUse[Index] >> 1);
    }
  } else {
    //
    // All values are used, have to reuse here
    //
    accum = 1;
  }
    
  //
  // Add the # to the end of the name - pull off as many 
  // chars as it takes to do it, and space pad the fat name
  // in case it's total length is now less then 8.  (Which
  // can happen if we pulled off a unicode char that goes
  // into fat as a dbcs encoding)
  //

  Width = 1;
  EfiValueToHexStr (AccumHex, accum, 0, 0);
  
  for (Width = EfiStrLen(Str1); Width >= 0; Width -= 1) {
    
    //
    // copy the string truncated by Width, then '~', then the number.
    // Since Width <= 8, StrLen(AccumHex) <= 3, and sizeof(Str2) == 14,
    // so there is always enough space in Str2 to accommodate all the stuff.
    // 'if ()' is just defensive.
    //
    
    for (Index = 0; Index < Width && Index < sizeof(Str2) - 1; Index ++) {
      Str2[Index] = Str1[Index];      
    }
    
    if (Index < sizeof(Str2) - 1) {
      Str2[Index] = '~';
      Index ++;
    }
    
    if (sizeof(Str2) - 1 - Index >= EfiStrLen (AccumHex)) {
      EfiStrCpy (&Str2[Index], AccumHex);
      Index += EfiStrLen (AccumHex);
    }
    
    Str2[Index] = 0;

    if (Index <= 8) {
      EfiSetMem (OName, 8, ' ');
      FatStrToFat (Str2, 8, OName);      
      Status = EFI_SUCCESS;
      break;
    }
  }

Done:
  if (InUse) {
    gBS->FreePool(InUse);
  }

  return Status;
}



BOOLEAN FatLfnIsValid (
  CHAR16  *Name
  )
/*++

Routine Description:
  
  Strip off starting/trailing spaces and trailing periods for a long
  file name and judge if it is a valid long file name.
  
Arguments:

  Name    - the name
  
Returns:
  TRUE    - the name is valid
  FALSE   - the name is invalid

--*/
{
  CHAR16  *p1, *p2;
  BOOLEAN IsAllDot;

  //
  // Treat a all-dot file name as a valid long file name. Do not turn it into a
  // all-blank file name by stripping off the trailing periods.
  //  
  IsAllDot = TRUE;

  p1 = Name;
  do {
    if (*p1 != '.') {
      IsAllDot = FALSE;
      break;
    }
    p1++;
  } while (*p1);

  if (IsAllDot) {
    return TRUE;
  }
 
  //
  // Strip off starting/trailing spaces and trailing periods
  //
  for (p1 = Name; *p1 && *p1 == ' '; p1 ++) {
    ;
  }  
  
  p2 = Name;
  
  while (*p1) {
    *p2 = *p1;
    p1 ++;
    p2 ++;
  }
  
  *p2 = 0;
  
  for (p1 = Name + EfiStrLen(Name) - 1; 
      p1 >= Name && (*p1 == ' ' || *p1 == '.');
      p1 --) {
    ;
  }
  
  *(p1 + 1) = 0;
  
  //
  // We don't allow zero length name
  //
  if (*Name == 0) {
    return FALSE;
  }
  
  //
  // See if there is any illegal characters within the name
  //  
  while (*Name) {
    if (*Name < 0x20 ||
        *Name == '\"' ||
        *Name == '*' ||
        *Name == '/' ||
        *Name == ':' ||
        *Name == '<' ||
        *Name == '>' ||
        *Name == '?' ||
        *Name == '\\' ||
        *Name == '|' ) {
      return FALSE;
    }
    Name ++;
  }  
  
  return TRUE;
}
