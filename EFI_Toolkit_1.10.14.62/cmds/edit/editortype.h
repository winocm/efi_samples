/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    editortype.h


--*/

#ifndef _EDITOR_TYPE_H_
#define _EDITOR_TYPE_H_


#include "efi.h"
#include "efilib.h"
#include "shell.h"


#define EDITOR_NAME     L"EFI Editor\0"
#define EDITOR_VERSION  L"0.99\0"

#define EFI_EDITOR_LINE_LIST    'eell'

#define INSERT_MODE_STR L"INS"
#define OVERWR_MODE_STR L"OVR"



#define FILE_ATTRIBUTES     EFI_FILE_MODE_READ  | \
                            EFI_FILE_MODE_WRITE | \
                            EFI_FILE_MODE_CREATE
#define FILE_READ_WRITE     EFI_FILE_MODE_READ  | \
                            EFI_FILE_MODE_WRITE
#define FILE_CREATE         EFI_FILE_MODE_READ  | \
                            EFI_FILE_MODE_WRITE | \
                            EFI_FILE_MODE_CREATE


#define DISPLAY_START_ROW		1
#define DISPLAY_START_COLUMN 	1
#define DISPLAY_MAX_ROW			MainEditor.ScreenSize.Row
#define DISPLAY_MAX_COLUMN 		MainEditor.ScreenSize.Column

#define TITLE_BAR_LENGTH		1
#define STATUS_BAR_LENGTH		1
#define MENU_BAR_LENGTH			3

#define TITLE_BAR_LOCATION  	DISPLAY_START_ROW
#define STATUS_BAR_LOCATION 	( DISPLAY_MAX_ROW - MENU_BAR_LENGTH )
#define INPUT_BAR_LOCATION  	STATUS_BAR_LOCATION
#define MENU_BAR_LOCATION   	( DISPLAY_MAX_ROW - MENU_BAR_LENGTH + 1 )
#define LAST_LINE_LOCATION  	DISPLAY_MAX_ROW 

#define TEXT_START_ROW		( TITLE_BAR_LOCATION + TITLE_BAR_LENGTH )
#define TEXT_START_COLUMN   DISPLAY_START_COLUMN
#define TEXT_END_ROW        ( STATUS_BAR_LOCATION - 1 )
#define TEXT_END_COLUMN     DISPLAY_MAX_COLUMN
#define MAX_TEXT_COLUMNS    DISPLAY_MAX_COLUMN
#define MAX_TEXT_ROWS   	TEXT_END_ROW    

#define NUM_TEXT_ROWS		( MAX_TEXT_ROWS - TEXT_START_ROW + 1 ) 
#define NUM_TEXT_COLUMNS	( MAX_TEXT_COLUMNS - TEXT_START_COLUMN + 1 )


#define MIN_POOL_SIZE       125
#define MAX_STRING_LENGTH   127

#define min(a,b) \
    (( a > b) ? b : a)
#define max(a,b) \
    (( a > b) ? a : b)

//Global variables for input and output
#define Out     ST->ConOut
#define In      ST->ConIn

#define SCAN_CODE_NULL  0x00
#define SCAN_CODE_UP    0x01
#define SCAN_CODE_DOWN  0x02
#define SCAN_CODE_RIGHT 0x03
#define SCAN_CODE_LEFT  0x04
#define SCAN_CODE_HOME  0x05
#define SCAN_CODE_END   0x06
#define SCAN_CODE_INS   0x07
#define SCAN_CODE_DEL   0x08
#define SCAN_CODE_PGUP  0x09
#define SCAN_CODE_PGDN  0x0A
#define SCAN_CODE_F1    0x0B
#define SCAN_CODE_F2    0x0C
#define SCAN_CODE_F3    0x0D
#define SCAN_CODE_F4    0x0E
#define SCAN_CODE_F5    0x0F
#define SCAN_CODE_F6    0x10
#define SCAN_CODE_F7    0x11
#define SCAN_CODE_F8    0x12
#define SCAN_CODE_F9    0x13
#define SCAN_CODE_F10   0x14
#define SCAN_CODE_F11   0x15
#define SCAN_CODE_F12   0x16
#define SCAN_CODE_ESC   0x17
#define CHAR_BS         0x08
#define CHAR_LF         0x0a
#define CHAR_CR         0x0d

#define IS_VALID_CHAR(x) \
    (x == SCAN_CODE_NULL)
#define IS_DIRECTION_KEY(x) \
    ((x >= SCAN_CODE_UP) && (x <= SCAN_CODE_PGDN))
#define IS_FUNCTION_KEY(x) \
    ((x >= SCAN_CODE_F1) && x <= (SCAN_CODE_F12))
#define IS_ESCAPE(x) \
    (x == SCAN_CODE_ESC) 




typedef struct  {
    UINTN   Row;
    UINTN   Column;
} EFI_EDITOR_POSITION;


typedef 
EFI_STATUS
(*EFI_MENU_ITEM_FUNCTION) (VOID);

typedef struct  {
    CHAR16  Name[50];
    CHAR16  Key[3];
    EFI_MENU_ITEM_FUNCTION  Function;
} MENU_ITEMS;

typedef struct  _EFI_EDITOR_LINE {
	UINTN               Signature;
    CHAR16              *Buffer;
    UINTN               Size;  // unit is unicode
	UINTN				TotalSize; // unit is unicode, exclude '\0'
    LIST_ENTRY          Link;
} EFI_EDITOR_LINE;

typedef struct  _EFI_EDITOR_MENU_ITEM   {
    CHAR16                      *Name;
    CHAR16                      *FunctionKey;
    EFI_MENU_ITEM_FUNCTION      Function;
} EFI_EDITOR_MENU_ITEM;



typedef struct  {
    UINT32  Foreground:4;
    UINT32  Background:4;
} EFI_EDITOR_COLOR_ATTRIBUTES;

typedef union {
    EFI_EDITOR_COLOR_ATTRIBUTES Colors;
    UINT8                       Data;
} EFI_EDITOR_COLOR_UNION;

typedef struct  {
    UINTN   Columns;
    UINTN   Rows;
} EFI_EDITOR_TEXT_MODE;

typedef enum    {
    ASCII_FILE,
    UNICODE_FILE 
} EFI_EDITOR_FILE_TYPE;

typedef enum    {
    USE_LF,
    USE_CRLF
}   EE_NEWLINE_TYPE;




typedef struct  {
	CHAR16 	*TitlePrefix;
} EFI_EDITOR_TITLE_BAR;


typedef struct  {
    CHAR16                  *FileName;		// file name current edited in editor
    EFI_EDITOR_FILE_TYPE    FileType;		// unicode file or ascii file

    LIST_ENTRY              *ListHead;		// list head of lines
    EFI_EDITOR_LINE         *Lines;			// lines of current file
    UINTN                   NumLines;		// total line numbers
    EE_NEWLINE_TYPE         NewLineType;	// line type CRLF or LF

//    EFI_FILE_HANDLE         CurrentDir;		// current dir of file opened
    	
    EFI_EDITOR_POSITION     DisplayPosition;		// curosr postion in screen
    EFI_EDITOR_POSITION     FilePosition;			// cursor positon in file
    EFI_EDITOR_POSITION     LowVisibleRange;		// file position of first byte displayed on screen

    BOOLEAN 			 	FileModified;			// file is modified or not
    BOOLEAN                 ModeInsert;				// input mode INS or OVR
	BOOLEAN					ReadOnly;				// file is readonly or not
		
    EFI_EDITOR_LINE         *CurrentLine;			// current line cursor is at
} EFI_EDITOR_FILE_BUFFER;


typedef struct  {
    CHAR16                  *StatusString;
} EFI_EDITOR_STATUS_BAR;



typedef struct  {
    CHAR16                  *Prompt;				// input bar prompt string
    CHAR16                  *ReturnString;
    UINTN                   StringSize;				// size of ReturnString space size
} EFI_EDITOR_INPUT_BAR;


typedef struct  {
    EFI_EDITOR_MENU_ITEM        *MenuItems;
} EFI_EDITOR_MENU_BAR;


typedef struct {
    EFI_EDITOR_TITLE_BAR    *TitleBar;
    EFI_EDITOR_MENU_BAR     *MenuBar;
    EFI_EDITOR_STATUS_BAR   *StatusBar;
    EFI_EDITOR_INPUT_BAR    *InputBar;
    EFI_EDITOR_FILE_BUFFER  *FileBuffer;

    EFI_EDITOR_COLOR_UNION  ColorAttributes;			
    EFI_EDITOR_POSITION     ScreenSize;			// row number and column number
    
    EFI_EDITOR_LINE			*CutLine;			// clip board
} EFI_EDITOR_GLOBAL_EDITOR;

#endif