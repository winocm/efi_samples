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

    dskimage.c
    
Abstract:



Revision History

--*/
#define _WIN32_WINNT 0X0500
#include "stdio.h"
#include "windows.h"
#include "Winioctl.h"



#define DEFAULT_FLOPPY_PATH   "\\\\.\\A:"
#define FLOPPY_DISK_RAW_SIZE	1474560

void helpmsg( void )
{
    printf(
        "Read disk A: to image file:\n"
        "   Dskimage -r image.img\n"
        "Write image file to disk A:\n"
        "   Dskimage -w image.img\n"
        "\n\n" );
}

int main( int argc, char *argv[] )
{
  BOOL    bReadFromFloppy;
	HANDLE  hFloppy;
	HANDLE  hImageFile;
	void    *pDiskBuffer	= NULL;
	BOOL    bResult;
	DWORD   nBytesRead;
	DWORD   nBytesWrite;

  if( argc != 3 )
  {
    helpmsg();
    return (-1);
  }

  if( !(  strcmp( argv[1], "-r" ) == 0 ||
          strcmp( argv[1], "-R" ) == 0 ||
          strcmp( argv[1], "-w" ) == 0 ||
          strcmp( argv[1], "-W" ) == 0 ) )
  {
    helpmsg();
    return (-1);
  }

  if( strcmp( argv[1], "-r" ) == 0 ||
      strcmp( argv[1], "-R" ) == 0 )
  {
    bReadFromFloppy = TRUE;
  }
  else
  {
    bReadFromFloppy = FALSE;
  }

  if( bReadFromFloppy )
  {
    //
    // Read the image from floppy disk A: to the file
    //
  	hFloppy = CreateFile( DEFAULT_FLOPPY_PATH,
            							GENERIC_READ,
            							FILE_SHARE_READ | FILE_SHARE_WRITE, 
            							NULL,                   // default security attributes
            							OPEN_EXISTING,          // disposition
            							FILE_ATTRIBUTE_NORMAL,  // file attributes
            							NULL );                 // do not copy file attributes
            							
  	if( hFloppy == INVALID_HANDLE_VALUE )
  	{
  	  //
  	  // Cannot open the drive
  	  //
  		printf( "Can not open floppy device A:.\n" );
  		printf( "0x%08X\n", GetLastError() );
  		return (-1);
  	}

  	hImageFile = CreateFile(  argv[ 2 ],
              								GENERIC_WRITE,
              								0, 
              								NULL,                   // default security attributes
              								CREATE_ALWAYS,          // disposition
              								FILE_ATTRIBUTE_NORMAL,  // file attributes
              								NULL );                 // do not copy file attributes
  	if( hImageFile == INVALID_HANDLE_VALUE )
  	{
  	  //
  	  // Cannot open the image file
  	  //
  		printf( "Can not open image file: %s.\n", argv[ 2 ] );
  		printf( "Error number:      0x%08X\n", GetLastError() );
  		return (-1);
  	}

  	pDiskBuffer	= malloc( FLOPPY_DISK_RAW_SIZE );
  	if( pDiskBuffer == NULL )
  	{
  		printf( "Not enough memory resource!\n" );
  		return (-1);
  	}

  	bResult	= ReadFile( hFloppy, pDiskBuffer, FLOPPY_DISK_RAW_SIZE, &nBytesRead, NULL );
    
  	if( bResult && nBytesRead == FLOPPY_DISK_RAW_SIZE ) 
  	{ 
  		// Make sure we're at the end of the file
  		bResult	= ReadFile( hFloppy, pDiskBuffer, FLOPPY_DISK_RAW_SIZE, &nBytesRead, NULL );
      
  		if( bResult &&  nBytesRead == 0 ) 
  		{
  			printf( "Read floppy disk A: OK\nWritting to file %s ...", argv[ 2 ] );

  			bResult	= WriteFile( hImageFile, pDiskBuffer, FLOPPY_DISK_RAW_SIZE, &nBytesWrite, NULL );

  			if( bResult && nBytesWrite == FLOPPY_DISK_RAW_SIZE )
  			{
  				// Write OK
  				printf( "OK\n" );
          free( pDiskBuffer );
          return 0;
  			}
  			else
  			{
  				printf( "Fail\n" );
          printf( "Error number:      0x%08X\n", GetLastError() );
          free( pDiskBuffer );
          return (-1);
  			}
  		}
      else
      {
    		printf( "Error reading the floppy!\n" );
    		printf( "0x%08X\n", GetLastError() );
        free( pDiskBuffer );
    		return (-1);
      }
  	}
  	else
  	{
  		printf( "Error reading the floppy!\n" );
  		printf( "0x%08X\n", GetLastError() );
      free( pDiskBuffer );
  		return (-1);
  	}
  }
  else
  {
    //
    // Write the image file to the floppy disk A:
    //
  	hImageFile = CreateFile(  argv[ 2 ],
                							GENERIC_READ,
                							FILE_SHARE_READ | FILE_SHARE_WRITE, 
                							NULL,                   // default security attributes
                							OPEN_EXISTING,          // disposition
                							FILE_ATTRIBUTE_NORMAL,  // file attributes
                							NULL );                 // do not copy file attributes
            							
  	if( hImageFile == INVALID_HANDLE_VALUE )
  	{
  	  //
  	  // Cannot open the image file
  	  //
  		printf( "Can not open image file: %s.\n", argv[ 2 ] );
  		printf( "Error number:      0x%08X\n", GetLastError() );
  		return (-1);
  	}

  	hFloppy = CreateFile( DEFAULT_FLOPPY_PATH,
          								GENERIC_WRITE,
          								FILE_SHARE_READ | FILE_SHARE_WRITE, 
          								NULL,                   // default security attributes
          								OPEN_EXISTING,          // disposition
          								FILE_ATTRIBUTE_NORMAL,  // file attributes
          								NULL );                 // do not copy file attributes
  	if( hFloppy == INVALID_HANDLE_VALUE )
  	{
  	  //
  	  // Cannot open the floppy disk A:
  	  //
  		printf( "Can not open floppy disk A: %s.\n", argv[ 2 ] );
  		printf( "Error number:      0x%08X\n", GetLastError() );
  		return (-1);
  	}
    
  	pDiskBuffer	= malloc( FLOPPY_DISK_RAW_SIZE );
  	if( pDiskBuffer == NULL )
  	{
  		printf( "Not enough memory resource!\n" );
  		return (-1);
  	}

  	bResult	= ReadFile( hImageFile, pDiskBuffer, FLOPPY_DISK_RAW_SIZE, &nBytesRead, NULL );
    
  	if( bResult && nBytesRead == FLOPPY_DISK_RAW_SIZE ) 
  	{ 
  		// Make sure we're at the end of the file
  		bResult	= ReadFile( hImageFile, pDiskBuffer, FLOPPY_DISK_RAW_SIZE, &nBytesRead, NULL );
      
  		if( bResult &&  nBytesRead == 0 ) 
  		{
  			printf( "Read from file: %s OK\nWritting to floppy disk A: ...", argv[ 2 ] );

  			bResult	= WriteFile( hFloppy, pDiskBuffer, FLOPPY_DISK_RAW_SIZE, &nBytesWrite, NULL );

  			if( bResult && nBytesWrite == FLOPPY_DISK_RAW_SIZE )
  			{
  				// Write OK
  				printf( "OK\n" );
          free( pDiskBuffer );
          return 0;
  			}
  			else
  			{
  				printf( "Fail\n" );
          printf( "Error number:      0x%08X\n", GetLastError() );
          free( pDiskBuffer );
          return (-1);
  			}
  		}
      else
      {
    		printf( "Error reading file: %s\n", argv[ 2 ] );
    		printf( "0x%08X\n", GetLastError() );
        free( pDiskBuffer );
    		return (-1);
      }

  	}
  	else
  	{
  		printf( "Error reading file: %s\n", argv[ 2 ] );
  		printf( "0x%08X\n", GetLastError() );
      free( pDiskBuffer );
  		return (-1);
  	}
  }
}

