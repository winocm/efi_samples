/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/
//
// GenHelp.c : Convert Help data .src file to .c source file.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

//
// Intel header
//
#define FILE_HEADER "/*++\n\n\
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved\n\
This software and associated documentation (if any) is furnished\n\
under a license and may only be used or copied in accordance\n\
with the terms of the license. Except as permitted by such\n\
license, no part of this software or documentation may be\n\
reproduced, stored in a retrieval system, or transmitted in any\n\
form or by any means without the express written consent of\n\
Intel Corporation.\n\n\
\
Module Name:\n\n\
  Verbose Help Information\n\n\
Abstract:\n\n\
  Verbose Help Infomation of Shell Command\n\n\
Revision History\n\n\
--*/\n\n\
#include \"shell.h\" \n\
"

#define LINE_MAX_NUM    127
#define LINE_MAX_CHAR    80

#define FIND_FAILURE     -1

wchar_t buf[LINE_MAX_NUM];
wchar_t CMD_TAG[] = L"<!>";
wchar_t TAB_TAG[] = L"  ";
long  pos1, pos2;


int FindTag(wchar_t *line, wchar_t *tag)
/*++

Description:
  Check if 'tag' is at the head of 'line'.

Arguments:
  line      - input UNICODE string
  tag       - tag UNICODE string
  
Returns:
   i               - string length of tag
  GENHELP_FAILURE  - Not found

--*/
{
  int i;
  //
  // Check tag if it is at the head of line
  //
  for (i=0; tag[i] != 0; i++)
  {
    if (line[i] != tag[i]) {
      return FIND_FAILURE;
    }
  }
  return i;
}

void AddTail(FILE *f, int header)
/*++

Description:
  Since command is separated with CMD_TAG - <!>, 
  Finding this TAG means found the next command scope, 
  so Add a tail to the command's info. Tail is a '\0'.

Arguments:
  f         - file to get data
  header    - if it is header, shall not add tail
  
Returns:
  None

Notes:  
  header = 1 ---- it is file header, needn't add tail.
  header = 0 ---- add the tail of first cmd.

--*/
{
  //
  // IF header == 1 THEN 
  //    it's first cmd, needn't append '\0' to end
  // ELSE
  //    it's not first cmd, append a '\0' to end 
  //    to tag first cmd verbose help end
  //
  if(header != 1) {
    fputwc (L'\0', f);
  }
}

int ReadLine(FILE *f, wchar_t *buf)
/*++

Description:
  Read a line from a file to a buffer.
  
Arguments:
  f      - file handle for read
  buf    - buffer for store data
  
Returns:
  -1     - file end
   i     - line length

--*/
{
  int   i;
  wint_t ch=0;
  memset (buf, 0, LINE_MAX_NUM*sizeof(wchar_t) );
  fseek (f, pos1, SEEK_SET);
  for(i=0; i < (LINE_MAX_NUM-1) && ch != WEOF ; i++)
  {
    ch = fgetwc(f);
    //
    // Unicode line is ended with L"\r\n"
    //
    if(ch != L'\n' && ch != L'\r') {
      buf[i] = (wchar_t)ch;
    }
    else {
      //
      // To filter the L'\n' if needed. If no L'\n', step back.
      //
      if (fgetwc (f) != L'\n' && fgetwc (f) != L'\r') {
        pos1 = ftell (f) - sizeof(wchar_t);
        fseek (f, pos1, SEEK_SET);
      }
      break;
    }
  }
  buf[i] = 0;
  pos1 = ftell (f);

  if(ch == WEOF) {
    return -1;
  }
  else {
    return i;
  }  
}

void LineOut(wchar_t *buf, FILE *f)
/*++

Description:
  Output a UNICODE string from buffer to a file.
  Output is string. For example:
  buf[] = L"ls",
  Outpurt: 0x006C, 0x0073, 0x0000
  
Arguments:
  buf    - buffer for output data
  f      - file handle for write
  
Returns:
  None

--*/
{
  wchar_t *p;
  wchar_t str[32];
  int   ch=0;
  int   i;

  p = buf;
  //
  // Read a char from buffer and output to file until line end
  //
  for(i=0; ch != WEOF && *p != L'\0'; i++, p++){
    //
    // If LINE_MAX_CHAR, feed a new line
    //
    if ( i >= LINE_MAX_CHAR - 1) {
      fputwc (L'\n', f);
      i = 0;
    }
    memset (str, 0, 32*sizeof(wchar_t) );
    //
    // '%' shall be replaced as '%''%'
    //
    if(*p == L'%')  {
      swprintf (str, L"%c%c", *p, *p);
    }
    else {
      swprintf (str, L"%c", *p);
    }
    ch = fputws (str, f);
  }
}

long WriteLine(wchar_t *buf, FILE *f)
/*++

Description:
  Output a line to a file.
  <!> cmd name          - one line, LineOut and append a '\0'
  <1> cmd syntax        - one line, LineOut and append a '\0'
  <2> cmd line help     - one line, LineOut and append a '\0'
  <3> cmd verbose help  - multiple lines, LineOut and append a '\n'
                          If find next <!>cmd THEN AddTail().
  
Arguments:
  buf    - buffer for output data
  f      - file handle for output
  
Returns:
  len    - File length added after write this time

--*/
{
  wchar_t *p;
  wchar_t cmd[256];
  wint_t  ch=0;
  long    pos;
  int     i;
  static int header=1;

  //
  // remember pos
  //
  fseek (f, pos2, SEEK_SET);
  pos = pos2;
  
  if(FindTag (buf, CMD_TAG) != FIND_FAILURE)
  {
    //
    // First cmd Tag the header
    //
    AddTail (f, header);
    header = 0;
    //
    // Skip Tag: <!>,<1>,<2>,<3>, all length is 3.
    //
    p = buf + 3;
    
    memset (cmd, 0, 256*sizeof(wchar_t));
    for(i=0; ch != WEOF && *p != L' ' && *p != L'\0'; i++, p++) {
      cmd[i] = *p;
    }

    p = (wchar_t *)cmd;
    //
    // Transfer cmd name to lower case
    //
    _wcslwr (p);
    LineOut (p, f);
    //
    // Mark end of string
    //
    fputwc (L'\0', f);
  } else if( FindTag (buf, L"<1>") != FIND_FAILURE) {
    //
    // Find cmd syntax
    //
    p = buf + 3;
    LineOut (p, f);
    fputwc (L'\0', f);
  } else if (FindTag (buf, L"<2>") != FIND_FAILURE) {
    //
    // Find cmd line help
    //
    p = buf + 3;
    LineOut (p, f);
    fputwc (L'\0', f);
  } else if (FindTag (buf, L"<3>") != FIND_FAILURE) {
    //
    // Find the first line of cmd verbose help
    //
    p = buf + 3;
    LineOut (p, f);
    fputwc (L'\n', f);
  } else {
    //
    // Other lines of cmd verbose help
    //
    LineOut (buf, f);
    fputwc (L'\n', f);
  }

  pos2 = ftell (f);
  return (pos2 - pos);
}

int backup(char* filename, char *backupname)
/*++

Description:
  Back up the HelpData.c file of the first version.

Arguments:
  filename     - file to to backup
  backupname   - backup file name
  
Returns:
   0      - success
  -1      - failure
  
--*/
{
  int ret;
  FILE *f, *fb;
  char cmd[LINE_MAX_NUM*2];

  f = NULL;
  f = fopen (filename, "rb");
  fb = NULL;
  fb = fopen (backupname, "wb");
  if(f != NULL && fb == NULL) {
    fclose (f);
    //
    // Use copy cmd to back up
    //
    memset (cmd, 0, sizeof(cmd));
    sprintf (cmd, "copy %s %s", filename, backupname);
    system (cmd);

    ret = 0;
  }
  ret = -1;
  return ret;
}

void ShowHelp()
{
  //
  // Usage guide
  //
  printf ("\nGenHelp [infile] [outfile]\n");
  printf ("  infile   - Help raw data file, default is \"HelpData.src\".\n");
  printf ("  outfile  - Help src data file, default is \"HelpData.c\".\n\n");
  printf ("\n");
  //
  // Format of input file
  //
  printf ("Format of HelpData.src:\n");
  printf ("<!>cmd name\n");
  printf ("<1>cmd syntax\n");
  printf ("<2>cmd line help\n");
  printf ("<3>cmd verbose help\n");
  printf ("   ... \n");
  printf ("\n");
}

int main(int argc, char* argv[])
{
  FILE *f1,*f2,*f3;
  long StartPos;
  char file1[LINE_MAX_NUM + 1] = "";
  char file2[LINE_MAX_NUM + 1] = "";

  int  ch;
  char str[128];
  int  i;
  int  Len;
  int  uchar;

  //
  // Show help
  //
  for (i=1; i<argc; i++) {
    if ((stricmp (argv[1], "-?") == 0) || (stricmp (argv[1], "/?") == 0)) {
      ShowHelp ();
      return 0;
    }
  }

  if(argc >= 3) {
    strcpy (file2, argv[2]);
  }
  else {
    strcpy (file2, "HelpData.c");
  }

  strcpy (file1, file2);
//  backup(file2, strcat(file1, ".bak"));

  if (argc >= 2) {
    strcpy (file1, argv[1]);
  }
  else {
    strcpy (file1, "HelpData.src");
  }

  f1 = fopen (file1, "rb");
  if ( f1 == NULL) {
    printf("GenHelp: Open input file \"%s\" error, exit..\n", file1);
    exit(0);
  }
  printf ("Input file is: %s\n", file1);

  //
  // Open a tmp file for output
  //
  f2 = tmpfile();
  if( f2 == NULL ) {
    printf ("GenHelp: Could not open new temporary file\n" );
    exit(0);
  }

  //
  // Skip the header of UNICODE file: FF FE, size=2.
  //
  if ((fgetc (f1) != 0xFF) && (fgetc(f1) != 0xFE)) {
    printf ("Input file is not a UNICODE file, exit..\n");
    exit(1);
  }

  if (fseek (f1, 2L, SEEK_SET)) {
     perror( "Fseek failed" );
  }
  pos1 = 2L;

  //
  // Skip the header of file
  //
  for (;;)  {
    StartPos = pos1;
    ReadLine (f1, buf);
    if( FindTag (buf, CMD_TAG) != -1) {
      pos1 = StartPos;
      break;
    }
  }

  while (ReadLine (f1, buf) != -1) {
    WriteLine (buf, f2);
  }

  //
  // file end
  //
  AddTail (f2, 0);
  fputwc (L'\0', f2);

  fclose (f1);
  f3 = fopen(file2, "wb");
  if (f3 == NULL) {
    printf("GenHelp: Open output file \"%s\" error, exit.\n", file2);
    exit(0);
  }
  printf ("Output file is: %s\n", file2);

  fputs (FILE_HEADER, f3);
  fputs ("\nCHAR16 gHelpData[] = {", f3);

  //
  // Count the length of tmp File
  //
  if (fseek (f2, 0L, SEEK_END) != 0) {
    printf( "Fseek failed" );
    exit(0);
  }
  Len = ftell (f2);
  if (fseek (f2, 0L, SEEK_SET) != 0) {
    printf( "Fseek failed" );
    exit(0);
  }
  i = ftell (f2);
  Len -= i;

  //
  // Transfer tmp file to .c file
  //
  for (i=0; i<Len; i++) {
    //
    // Every line contains 11 items
    //
    if ((i % 11) == 0) {
      fputc ('\n', f3);
      fputs ("  ", f3);
    }
    //
    // Get two byte and make as UNICODE form
    //
    ch = fgetc (f2);
    if (ch == EOF) {
      break;
    }
    uchar = ch;
    i++;
    ch = fgetc (f2);
    if (ch == EOF) {
      break;
    }
    uchar += ch << 8;
    
    memset (str, 0, sizeof(str));
    sprintf (str, "0x%04x,", uchar);
    fputs (str, f3);
  }

  //
  // add end of file
  //
  fputs ("\n};\n\n", f3); 

  _rmtmp ();
  fclose (f3);

   return 0;
}
