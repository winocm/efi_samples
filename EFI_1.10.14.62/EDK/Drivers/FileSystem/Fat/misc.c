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

  misc.c
  
Abstract:

  Miscellaneous functions

Revision History

--*/

#include "fat.h"


EFI_STATUS
FatSetVolumeDirty (
  IN FAT_VOLUME       *Vol,
  IN BOOLEAN          Dirty
  )
/*++

Routine Description:
  Set the volume as dirty or not
  
Arguments: 
  
Returns:

--*/
{
  EFI_STATUS  Status;
  
  Status = EFI_SUCCESS;
  
  if (Vol->FatType == FAT12) {
    //
    // Do nothing
    //
    ;
  } else if (Vol->FatType == FAT16) {

    if (Dirty) {
      Status = FatDiskIo (Vol, WRITE_DISK, Vol->FatPos + 2,
                  2, &Vol->DirtyValue);
    } else {
      Status = FatDiskIo (Vol, WRITE_DISK, Vol->FatPos + 2,
                  2, &Vol->NotDirtyValue);
    }    
  } else if (Vol->FatType == FAT32) {

    if (Dirty) {
      Status = FatDiskIo (Vol, WRITE_DISK, Vol->FatPos + 4,
                  4, &Vol->DirtyValue);
    } else {
      Status = FatDiskIo (Vol, WRITE_DISK, Vol->FatPos + 4,
                  4, &Vol->NotDirtyValue);
    }    
  }
  
  //
  // Flush all dirty cache entries to disk
  //
  if (Vol->Valid) {
    Status = FatVolumeFlushCache (Vol);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }  
  
  return Status;
}


EFI_STATUS
FatDiskIo (
  IN FAT_VOLUME       *Vol,
  IN DISK_IO_MODE     IoMode,
  IN UINT64           Offset,
  IN UINTN            BufferSize,
  IN OUT VOID         *Buffer
  )
/*++

Routine Description:
  General disk access function
  
Arguments: 
  
Returns:

--*/

{
  EFI_STATUS            Status;

  if (Vol->Valid == FALSE) {
    return EFI_MEDIA_CHANGED;
  }
  
  //
  // Verify the IO is in devices range
  //

  Status = EFI_VOLUME_CORRUPTED;
  if (Offset + BufferSize <= Vol->VolSize) {

    switch (IoMode) {
      case READ_DISK:
        Status = FatDiskIoReadVolume (Vol, Offset, BufferSize, Buffer);
        break;
      case WRITE_DISK:
        Status = FatDiskIoWriteVolume (Vol, Offset, BufferSize, Buffer);
        break;
      default:
        Status = EFI_UNSUPPORTED;
        break;
    }
  }

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "FatDiskIo: error %r\n", Status));
  }

  return Status;
}


VOID
FatAcquireLock ()
/*++

Routine Description:
  Lock the volume.

Arguments:
  Null.
  
Returns:
  VOID.
  
--*/
{
  EfiAcquireLock (&FatFsLock);
}


BOOLEAN
FatIsLocked ()
/*++

Routine Description:
  Get the locking status of the volume.
  
Arguments:
  Null.
  
Returns:
  TRUE      - the volume is locked
  FALSE     - the volume is not locked
  
--*/
{
  return (BOOLEAN)(FatFsLock.Lock);
}


VOID
FatReleaseLock ()
/*++

Routine Description:
  Unlock the volume.
  
Arguments:
  Null.
  
Returns:
  VOID.
  
--*/
{
  EfiReleaseLock (&FatFsLock);
}


UINTN
FatSizeToClusters (
  IN UINT64           Size,
  IN UINTN            ClusterSize
  )
/*++

Routine Description:
  Count the number of clusters given a size
  
Arguments: 
  
Returns:

--*/

{
  UINTN               Rem;
  UINTN               Clusters;    

  Clusters = (UINTN) DriverLibDivU64x32(Size, (UINT32) ClusterSize, &Rem);
  if (Rem) {
    Clusters += 1;
  }

  return Clusters;
}


FAT_OFILE *
FatAllocateOFile (
  IN FAT_VOLUME   *Vol
  )
/*++

Routine Description:
  Allocate OFile structure
  
Arguments: 
  
Returns:

--*/
{
  FAT_OFILE   *OFile;

  OFile = EfiLibAllocateZeroPool(sizeof(FAT_OFILE));
  if (OFile) {
    OFile->Signature = FAT_OFILE_SIGNATURE;
    OFile->Vol = Vol;
    InitializeListHead (&OFile->Opens);
    InitializeListHead (&OFile->ChildHead);

  }
  return OFile;
}

VOID
FatFreeOFile (
  IN FAT_OFILE  *OFile
  )
/*++

Routine Description:
  Free OFile structure
  
Arguments: 
  
Returns:

--*/
{
  UINTN   Index;
  
  if (OFile->NameNotFound) {
    gBS->FreePool (OFile->NameNotFound);
  }
  if (OFile->FileString) {
    gBS->FreePool (OFile->FileString);
  }
  
  if (OFile->ChildHashTable) {
    for (Index = 0; Index < HASH_WORD_MASK + 1; Index ++) {
      if (OFile->ChildHashTable[Index].HashNode) {
        gBS->FreePool (OFile->ChildHashTable[Index].HashNode);
      }
    }
    gBS->FreePool (OFile->ChildHashTable);
  }
  gBS->FreePool (OFile);
}

VOID
FatEfiTimeToFatTime (
  IN EFI_TIME         *ETime,
  OUT FAT_DATE_TIME   *FTime
  )
/*++

Routine Description:
  Translate EFI time to FAT time
  
Arguments: 
  
Returns:

--*/
{
  //ignores timezone info in source ETime

  if (ETime->Year > 1980) {
    FTime->Date.Year = (UINT16)(ETime->Year - 1980);
  }

  if (ETime->Year >= 1980 + 128) {
    FTime->Date.Year = (UINT16)-1;
  }

  FTime->Date.Month = ETime->Month;
  FTime->Date.Day = ETime->Day;
  FTime->Time.Hour = ETime->Hour;
  FTime->Time.Minute = ETime->Minute;
  FTime->Time.DoubleSecond = (UINT16)(ETime->Second / 2);
}

VOID
FatFatTimeToEfiTime (
  IN FAT_DATE_TIME    *FTime,
  OUT EFI_TIME        *ETime
  )
/*++

Routine Description:
  Translate Fat time to EFI time
  
Arguments: 
  
Returns:

--*/
{
  ETime->Year = (UINT16) (FTime->Date.Year + 1980);
  ETime->Month = (UINT8) FTime->Date.Month;
  ETime->Day = (UINT8) FTime->Date.Day;
  ETime->Hour = (UINT8) FTime->Time.Hour;
  ETime->Minute = (UINT8) FTime->Time.Minute;
  ETime->Second = (UINT8) (FTime->Time.DoubleSecond * 2);
  ETime->Nanosecond = 0;
  ETime->TimeZone = EFI_UNSPECIFIED_TIMEZONE;
  ETime->Daylight = 0;
}

BOOLEAN
FatIsValidTime (
  IN EFI_TIME         *Time
  )
/*++

Routine Description:
  Check whether a time is valid
  
Arguments: 
  
Returns:

--*/
{
  static UINT8 MonthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  UINTN        Day;
  BOOLEAN      Status;

  Status = TRUE;

  //
  // Check the fields for range problems
  //

  if (Time->Year < 1980  ||           // Fat can only support from 1980
    Time->Month < 1 ||
    Time->Month > 12 ||             // Month is 1-12
    Time->Day < 1 ||
    Time->Day > 31 ||
    Time->Hour > 23 ||
    Time->Minute > 59 ||
    Time->Second > 59 ||           
    Time->Nanosecond > 999999999) {

    Status = FALSE;

  } else {

    //
    // Perform a more specific check of the day of the month
    //

    Day = MonthDays[Time->Month - 1];
    if (Time->Month == 2 &&         // Is it February & a leap year?
      Time->Year % 4 == 0 &&      // leap is every 4 years,
       (Time->Year % 100 != 0 ||    // but not on century year
        Time->Year % 400 == 0)      // unless it's every 4th century
      ) {

        Day += 1;               // 1 extra day this month
    }

    if (Time->Day > Day) {
      Status = FALSE;
    }
  }

  return Status;
}


UINTN
FatStrToFatLen (
  IN CHAR16                           *String
  )
/*++

Routine Description:
  Convert CHAR16 string to FAT string
  
Arguments: 
  
Returns:

--*/
// N.B. only works on lengths up to 20 chars (but that enough
// for all 8.3 style calculations
{
  CHAR8           Buffer[20];

  EfiSetMem (Buffer, sizeof(Buffer), 0);
  FatStrToFat (String, sizeof(Buffer)-1, Buffer);
  return EfiAsciiStrLen (Buffer);
}

VOID
FatStrUpr (
  IN CHAR16   *Str
  )
/*++

Routine Description:
  Uppercase a string
  
Arguments: 
  
Returns:

--*/
{
  if (gUnicodeCollationInterface) {
    gUnicodeCollationInterface->StrUpr (gUnicodeCollationInterface, Str);
  }  
}


VOID
FatStrLwr (
  IN CHAR16   *Str
  )
/*++

Routine Description:
  Lowercase a string
  
Arguments: 
  
Returns:

--*/
{
  if (gUnicodeCollationInterface) {
    gUnicodeCollationInterface->StrLwr (gUnicodeCollationInterface, Str);
  }  
}


INTN
FatStriCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2
  )
/*++

Routine Description:
  Case insensitive string compare
  
Arguments: 
  
Returns:

--*/
{
  if (gUnicodeCollationInterface) {
    return gUnicodeCollationInterface->StriColl (gUnicodeCollationInterface,
                                                 s1,
                                                 s2);
  } else {
    return 1;
  }    
}


VOID
FatFatToStr (
  IN UINTN                            FatSize,
  IN CHAR8                            *Fat,
  OUT CHAR16                          *String
  )
/*++

Routine Description:
  Convert FAT string to unicode string
  
Arguments: 
  
Returns:

--*/
{
  if (gUnicodeCollationInterface) {
    gUnicodeCollationInterface->FatToStr(gUnicodeCollationInterface,
                                         FatSize,
                                         Fat,
                                         String);
  }  
}


BOOLEAN
FatStrToFat (
  IN CHAR16                           *String,
  IN UINTN                            FatSize,
  OUT CHAR8                           *Fat
  )
/*++

Routine Description:
  Convert unicode string to Fat string
  
Arguments: 
  
Returns:

--*/
{
  if (gUnicodeCollationInterface) {
    return gUnicodeCollationInterface->StrToFat(gUnicodeCollationInterface,
                                                String,
                                                FatSize,
                                                Fat);
  } else {
    return FALSE;
  }  
}

