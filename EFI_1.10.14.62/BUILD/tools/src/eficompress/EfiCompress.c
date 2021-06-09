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

  Compress.c

Abstract:

  Compression routine. The compression algorithm is a mixture of
  LZ77 and Huffman coding. LZ77 transforms the source data into a
  sequence of Original Characters and Pointers to repeated strings.
  This sequence is further divided into Blocks and Huffman codings
  are applied to each Block.

Revision History:

  - OPSD code

    $Revision:   1.0  $
    $Date:   08 Apr 1996 12:40:28  $
    $Log:   P:\source\tools.bio\grfxlogo\src\compress.c_z  $

    Rev 1.0   08 Apr 1996 12:40:28   BWTRIPLE
    Initial revision.

  - Ported to EFI, March 2001

    Items modified: 
     1. replace native C data types with EFI data types
     2. reformat
     3. remove warnings using explicit data type casting
     4. change the interface: 
        standalone utility -> callable function (Compress ())
     5. refine comments
  
--*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include "efi.h"
#include "Compress.h"

int
main (
  INT32 argc,
  CHAR8 *argv[]
  )
{
  EFI_STATUS  Status;
  FILE        *infile, *outfile;
  UINT32      SrcSize, DstSize;
  UINT8       *SrcBuffer, *DstBuffer;
  UINT8       Buffer[8];

  //
  //  Added for makefile debug - KCE
  //
  INT32       arg_counter;
  printf ("\n\n");
  for (arg_counter = 0; arg_counter < argc; arg_counter++) {
    printf ("%s ", argv[arg_counter]);
  }
  printf ("\n\n");


  SrcBuffer = DstBuffer = NULL;

  infile = outfile = NULL;

  if (argc != 3) {
    printf ("Usage: COMPRESS <infile> <outfile>\n");
    goto Done;
  }

  if ((outfile = fopen(argv[2], "wb")) == NULL) {
    printf ("Can't open output file\n");
    goto Done;
  }

  if ((infile = fopen(argv[1], "rb")) == NULL) {
    printf ("Can't open input file\n");
    goto Done;
  }

  //
  // Get the size of source file
  //

  SrcSize = 0;
  while (fread (Buffer, 1, 1, infile)) 
    SrcSize ++;
  
  //
  // Read in the source data
  //

  if ((SrcBuffer = malloc (SrcSize)) == NULL) {
    printf ("Can't allocate memory\n");
    goto Done;
  }

  rewind (infile);
  if (fread (SrcBuffer, 1, SrcSize, infile) != SrcSize) {
    printf ("Can't read from source\n");
    goto Done;
  }

  //
  // Get destination data size and do the compression
  //

  DstSize = 0;
  Status = Compress(SrcBuffer, SrcSize, DstBuffer, &DstSize);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    if ((DstBuffer = malloc (DstSize)) == NULL) {
      printf ("Can't allocate memory\n");
      goto Done;
    }

    Status = Compress (SrcBuffer, SrcSize, DstBuffer, &DstSize);
  }

  if (EFI_ERROR(Status)) {
    printf ("Compress Error\n");
    goto Done;
  }

  printf ("\nOrig Size = %ld\n", SrcSize);
  printf ("Comp Size = %ld\n", DstSize);

  //
  // Write out the result
  //

  if (fwrite (DstBuffer, 1, DstSize, outfile) != DstSize) {
    printf ("Can't write to destination file\n");
  }

Done:
  if (SrcBuffer) {
    free (SrcBuffer);
  }

  if (DstBuffer) {
    free (DstBuffer);
  }

  if (infile) {
    fclose (infile);
  }

  if (outfile) {
    fclose (outfile);
  }

  return 0;
}

