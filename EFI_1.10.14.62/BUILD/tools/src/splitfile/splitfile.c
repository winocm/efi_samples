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

    splitfile.c
    
Abstract:



Revision History

--*/

//
// splitfile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "string.h"
#include "stdlib.h"

void helpmsg (void)
{
    printf(
        "SplitFile Filename Offset\n"
        "   Filename = Input file to split\n"
        "   Offset = offset at which to split file\n"
        "\n\n"
        "SplitFile will break a file in two pieces at the requested offset\n"
        "  outputting Filename1 and Filename2\n");
}

int main(int argc, char* argv[])
{
    FILE * In, * Out1, *Out2;
    char OutName1[512], OutName2[512];
    unsigned long i, splitpoint;
    char c;

    if (argc != 3)
    {
        helpmsg();
        return(-1);
    }
    
    In = fopen (argv[1], "rb");
    if (In == NULL)
    {
        printf ("Unable to open file \"%s\"\n", argv[1]);
        return(-1);
    }
    
    strncpy(OutName1, argv[1], 510);
    strncpy(OutName2, argv[1], 510);
    strcat(OutName1, "1");
    strcat(OutName2, "2");

    Out1 = fopen (OutName1, "wb");
    if (Out1 ==  NULL)
    {
        printf ("Unable to open file \"%s\"\n", OutName1);
        return(-1);
    }
    Out2 = fopen (OutName2, "wb");
    if (Out2 == NULL)
    {
        printf ("Unable to open file \"%s\"\n", OutName2);
        return(-1);
    }

    splitpoint = atoi(argv[2]);
    
    for (i = 0; i < splitpoint; i++)
    {
        c = fgetc(In);
        if (feof(In))
            break;
        fputc(c, Out1);
    }
    
    for(;;)
    {
        c = fgetc(In);
        if (feof(In))
            break;
        fputc(c, Out2);
    }
    
    fclose(In);
    fclose(Out1);
    fclose(Out2);
    
    return 0;
}

