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

    print.c

Abstract:




Revision History

--*/

#include "lib.h"
#include "efistdarg.h"                        // !!!


//
//
//


#define PRINT_STRING_LEN            1024
#define PRINT_ITEM_BUFFER_LEN       100

typedef struct {
    BOOLEAN             Ascii;
    UINTN               Index;
    union {
        CHAR16          *pw;
        CHAR8           *pc;
    }u ;
} POINTER;


typedef struct _pitem {

    POINTER     Item;
    CHAR16      Scratch[PRINT_ITEM_BUFFER_LEN];
    UINTN       Width;
    UINTN       FieldWidth;
    UINTN       *WidthParse;
    CHAR16      Pad;
    BOOLEAN     PadBefore;
    BOOLEAN     Comma;
    BOOLEAN     Long;
} PRINT_ITEM;


typedef struct _pstate {
    // Input
    POINTER     fmt;
    va_list     args;

    // Output
    CHAR16      *Buffer;
    CHAR16      *End;
    CHAR16      *Pos;
    UINTN       Len;

    UINTN       Attr;    
    UINTN       RestoreAttr;

    UINTN       AttrNorm;
    UINTN       AttrHighlight;
    UINTN       AttrError;

    INTN        (*Output)(VOID *context, CHAR16 *str);
    INTN        (*SetAttr)(VOID *context, UINTN attr);
    VOID        *Context;    

    // Current item being formatted
    struct _pitem  *Item;
} PRINT_STATE;


typedef struct {
  BOOLEAN  PageBreak;
  BOOLEAN  AutoWrap;
  INTN     MaxRow;
  INTN     MaxColumn;
  INTN     InitRow;
  INTN     Row;
  INTN     Column;
  BOOLEAN  OmitPrint;
} PRINT_MODE;

static PRINT_MODE mPrintMode;

//
// Internal fucntions
//

STATIC
UINTN
_Print (
    IN PRINT_STATE     *ps
    );

STATIC
UINTN
_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CHAR16                           *fmt,
    IN CHAR8                            *fmta,
    IN va_list                          args
    );

STATIC
INTN
_DbgOut (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    );

STATIC
VOID
PFLUSH (
    IN OUT PRINT_STATE     *ps
    );

STATIC
VOID
FlushWithPageBreak (
  IN OUT PRINT_STATE     *ps
  );

STATIC
VOID
PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
    );

STATIC
VOID
PITEM (
    IN OUT PRINT_STATE  *ps
    );

STATIC
CHAR16
PGETC (
    IN POINTER      *p
    );

STATIC
VOID
PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
    );

//
//
//

INTN
DbgPrint (
    IN INTN      mask,
    IN CHAR8     *fmt,
    ...
    )
/*++

Routine Description:

    Prints a formatted unicode string to the default StandardError console

Arguments:

    mask        - Bit mask of debug string.  If a bit is set in the
                  mask that is also set in EFIDebug the string is 
                  printed; otherwise, the string is not printed

    fmt         - Format string

Returns:

    Length of string printed to the StandardError console

--*/
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *DbgOut;
    PRINT_STATE     ps;
    va_list         args;
    UINTN           back;
    UINTN           attr;
    UINTN           SavedAttribute;


    if (!(EFIDebug & mask)) {
        return 0;
    }

    va_start (args, fmt);
    ZeroMem (&ps, sizeof(ps));

    ps.Output = _DbgOut; 
    ps.fmt.Ascii = TRUE;
    ps.fmt.u.pc = fmt;
    ps.args = args;
    ps.Attr = EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_RED); 

    DbgOut = LibRuntimeDebugOut;

    if (!DbgOut) {
        DbgOut = ST->StdErr;
    }

    if (DbgOut) {
        ps.Attr = DbgOut->Mode->Attribute;
        ps.Context = DbgOut;
        ps.SetAttr = (INTN (*)(VOID *, UINTN))  DbgOut->SetAttribute;
    }

    SavedAttribute = ps.Attr;

    back = (ps.Attr >> 4) & 0xf;
    ps.AttrNorm = EFI_TEXT_ATTR(EFI_LIGHTGRAY, back);
    ps.AttrHighlight = EFI_TEXT_ATTR(EFI_WHITE, back);
    ps.AttrError = EFI_TEXT_ATTR(EFI_YELLOW, back);

    attr = ps.AttrNorm;

    if (mask & D_WARN) {
        attr = ps.AttrHighlight;
    }

    if (mask & D_ERROR) {
        attr = ps.AttrError;
    }

    if (ps.SetAttr) {
        ps.Attr = attr;
        ps.SetAttr (ps.Context, attr);
    }

    _Print (&ps);

    //
    // Restore original attributes
    //

    if (ps.SetAttr) {
        ps.SetAttr (ps.Context, SavedAttribute);
    }
    
    return 0;
}



STATIC
INTN
_DbgOut (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )
// Append string worker for DbgPrint
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *DbgOut;

    DbgOut = Context;
//    if (!DbgOut && ST && ST->ConOut) {
//        DbgOut = ST->ConOut;
//    }

    if (DbgOut) {
        DbgOut->OutputString (DbgOut, Buffer);
    }

    return 0;
}

INTN
_SPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )
// Append string worker for SPrint, PoolPrint and CatPrint
{
    UINTN           len;
    POOL_PRINT      *spc;

    spc = Context;
    len = StrLen(Buffer);

    //
    // Is the string is over the max truncate it
    //

    if (spc->len + len > spc->maxlen) {
        len = spc->maxlen - spc->len;
    }

    //
    // Append the new text
    //

    CopyMem (spc->str + spc->len, Buffer, len * sizeof(CHAR16));
    spc->len += len;

    //
    // Null terminate it
    //

    if (spc->len < spc->maxlen) {
        spc->str[spc->len] = 0;
    } else if (spc->maxlen) {
        spc->str[spc->maxlen-1] = 0;
    }

    return 0;
}


INTN
_PoolPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )
// Append string worker for PoolPrint and CatPrint
{
    UINTN           newlen;
    POOL_PRINT      *spc;

    spc = Context;
    newlen = spc->len + StrLen(Buffer) + 1;

    //
    // Is the string is over the max, grow the buffer
    //

    if (newlen > spc->maxlen) {

        //
        // Grow the pool buffer
        //

        newlen += PRINT_STRING_LEN;
        spc->maxlen = newlen;
        spc->str = ReallocatePool (
                        spc->str, 
                        spc->len * sizeof(CHAR16), 
                        spc->maxlen * sizeof(CHAR16)
                        );

        if (!spc->str) {
            spc->len = 0;
            spc->maxlen = 0;
        }
    }

    //
    // Append the new text
    //

    return _SPrint (Context, Buffer);
}



VOID
_PoolCatPrint (
    IN CHAR16           *fmt,
    IN va_list          args,
    IN OUT POOL_PRINT   *spc,
    IN INTN             (*Output)(VOID *context, CHAR16 *str)
    )
// Dispath function for SPrint, PoolPrint, and CatPrint
{
    PRINT_STATE         ps;

    ZeroMem (&ps, sizeof(ps));
    ps.Output  = Output;
    ps.Context = spc;
    ps.fmt.u.pw = fmt;
    ps.args = args;
    _Print (&ps);
}



UINTN
SPrint (
    OUT CHAR16  *Str,
    IN UINTN    StrSize,
    IN CHAR16   *fmt,
    ...
    )
/*++

Routine Description:

    Prints a formatted unicode string to a buffer

Arguments:

    Str         - Output buffer to print the formatted string into

    StrSize     - Size of Str.  String is truncated to this size.
                  A size of 0 means there is no limit

    fmt         - The format string

Returns:

    String length returned in buffer

--*/
{
    POOL_PRINT          spc;
    va_list             args;


    va_start (args, fmt);
    spc.str    = Str;
    spc.maxlen = StrSize / sizeof(CHAR16) - 1;
    spc.len    = 0;

    _PoolCatPrint (fmt, args, &spc, _SPrint);
    return spc.len;
}


CHAR16 *
PoolPrint (
    IN CHAR16           *fmt,
    ...
    )
/*++

Routine Description:

    Prints a formatted unicode string to allocated pool.  The caller
    must free the resulting buffer.

Arguments:

    fmt         - The format string

Returns:

    Allocated buffer with the formatted string printed in it.  
    The caller must free the allocated buffer.   The buffer
    allocation is not packed.

--*/
{
    POOL_PRINT          spc;
    va_list             args;

    ZeroMem (&spc, sizeof(spc));
    va_start (args, fmt);
    _PoolCatPrint (fmt, args, &spc, _PoolPrint);
    return spc.str;
}



CHAR16 *
CatPrint (
    IN OUT POOL_PRINT   *Str,
    IN CHAR16           *fmt,
    ...
    )
/*++

Routine Description:

    Concatenates a formatted unicode string to allocated pool.  
    The caller must free the resulting buffer.

Arguments:

    Str         - Tracks the allocated pool, size in use, and 
                  amount of pool allocated.

    fmt         - The format string

Returns:

    Allocated buffer with the formatted string printed in it.  
    The caller must free the allocated buffer.   The buffer
    allocation is not packed.

--*/
{
    va_list             args;

    va_start (args, fmt);
    _PoolCatPrint (fmt, args, Str, _PoolPrint);
    return Str->str;
}



UINTN
Print (
    IN CHAR16   *fmt,
    ...
    )
/*++

Routine Description:

    Prints a formatted unicode string to the default console

Arguments:

    fmt         - Format string

Returns:

    Length of string printed to the console

--*/
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint ((UINTN) -1, (UINTN) -1, ST->ConOut, fmt, NULL, args);
}

UINTN
PrintAt (
    IN UINTN     Column,
    IN UINTN     Row,
    IN CHAR16    *fmt,
    ...
    )
/*++

Routine Description:

    Prints a formatted unicode string to the default console, at 
    the supplied cursor position

Arguments:

    Column, Row - The cursor position to print the string at

    fmt         - Format string

Returns:

    Length of string printed to the console

--*/
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint (Column, Row, ST->ConOut, fmt, NULL, args);
}


UINTN
IPrint (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE    *Out,
    IN CHAR16                          *fmt,
    ...
    )
/*++

Routine Description:

    Prints a formatted unicode string to the specified console

Arguments:

    Out         - The console to print the string too

    fmt         - Format string

Returns:

    Length of string printed to the console

--*/
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint ((UINTN) -1, (UINTN) -1, Out, fmt, NULL, args);
}


UINTN
IPrintAt (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN CHAR16                           *fmt,
    ...
    )
/*++

Routine Description:

    Prints a formatted unicode string to the specified console, at
    the supplied cursor position

Arguments:

    Out         - The console to print the string too

    Column, Row - The cursor position to print the string at

    fmt         - Format string

Returns:

    Length of string printed to the console

--*/
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint (Column, Row, ST->ConOut, fmt, NULL, args);
}


UINTN
_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CHAR16                           *fmt,
    IN CHAR8                            *fmta,
    IN va_list                          args
    )
// Display string worker for: Print, PrintAt, IPrint, IPrintAt
{
    PRINT_STATE     ps;
    UINTN            back;

    ZeroMem (&ps, sizeof(ps));
    ps.Context = Out;
    ps.Output  = (INTN (*)(VOID *, CHAR16 *)) Out->OutputString;
    ps.SetAttr = (INTN (*)(VOID *, UINTN))  Out->SetAttribute;
    ps.Attr = Out->Mode->Attribute;
   
    back = (ps.Attr >> 4) & 0xF;
    ps.AttrNorm = EFI_TEXT_ATTR(EFI_LIGHTGRAY, back);
    ps.AttrHighlight = EFI_TEXT_ATTR(EFI_WHITE, back);
    ps.AttrError = EFI_TEXT_ATTR(EFI_YELLOW, back);

    if (fmt) {
        ps.fmt.u.pw = fmt;
    } else {
        ps.fmt.Ascii = TRUE;
        ps.fmt.u.pc = fmta;
    }

    ps.args = args;

    if (Column != (UINTN) -1) {
        Out->SetCursorPosition(Out, Column, Row);
    }

    return _Print (&ps);
}


UINTN
APrint (
    IN CHAR8    *fmt,
    ...
    )
/*++

Routine Description:

    For those whom really can't deal with unicode, a print
    function that takes an ascii format string

Arguments:

    fmt         - ascii format string

Returns:

    Length of string printed to the console

--*/

{
    va_list     args;

    va_start (args, fmt);
    return _IPrint ((UINTN) -1, (UINTN) -1, ST->ConOut, NULL, fmt, args);
}


STATIC
VOID
PFLUSH (
    IN OUT PRINT_STATE     *ps
    )
{
    *ps->Pos = 0;
    if (((UINTN)ps->Context == (UINTN)ST->ConOut) && mPrintMode.PageBreak) {
      FlushWithPageBreak (ps);
    } else {
      ps->Output(ps->Context, ps->Buffer);
    }
    ps->Pos = ps->Buffer;
}

STATIC
VOID
PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
    )
{
   PFLUSH (ps);

   ps->RestoreAttr = ps->Attr;
   if (ps->SetAttr) {
        ps->SetAttr (ps->Context, Attr);
   }

   ps->Attr = Attr;
}   

STATIC
VOID
PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
    )
{
    //
    // If Omit print to ConOut, then return.
    //
    if (mPrintMode.OmitPrint && ((UINTN)ps->Context == (UINTN)ST->ConOut)) {
      return;
    }

    // if this is a newline and carriage return does not exist,
    // add a carriage return
    if (c == '\n' && (CHAR16)*(ps->Pos-1) != '\r' ) {
        PPUTC (ps, '\r');
    }

    *ps->Pos = c;
    ps->Pos += 1;
    ps->Len += 1;

    // if at the end of the buffer, flush it
    if (ps->Pos >= ps->End) {
        PFLUSH(ps);
    }
}


STATIC
CHAR16
PGETC (
    IN POINTER      *p
    )
{
    CHAR16      c;

    c = (CHAR16)(p->Ascii ? p->u.pc[p->Index] : p->u.pw[p->Index]);
    p->Index += 1;

    return  c;
}


STATIC
VOID
PITEM (
    IN OUT PRINT_STATE  *ps
    )
{
    UINTN               Len, i;
    PRINT_ITEM          *Item;
    CHAR16              c;

    // Get the length of the item
    Item = ps->Item;
    Item->Item.Index = 0;
    while (Item->Item.Index < Item->FieldWidth) {
        c = PGETC(&Item->Item);
        if (!c) {
            Item->Item.Index -= 1;
            break;
        }
    }
    Len = Item->Item.Index;

    // if there is no item field width, use the items width
    if (Item->FieldWidth == (UINTN) -1) {
        Item->FieldWidth = Len;
    }

    // if item is larger then width, update width
    if (Len > Item->Width) {
        Item->Width = Len;
    }


    // if pad field before, add pad char
    if (Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }

    // pad item
    for (i=Len; i < Item->Width; i++) {
        PPUTC (ps, Item->Pad);
    }

    // add the item
    Item->Item.Index=0; 
    while (Item->Item.Index < Len) {
        PPUTC (ps, PGETC(&Item->Item));
    }

    // If pad at the end, add pad char
    if (!Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }
}


STATIC
UINTN
_Print (
    IN PRINT_STATE     *ps
    )
/*++

Routine Description:

    %w.lF   -   w = width
                l = field width
                F = format of arg

  Args F:
    0       -   pad with zeros
    -       -   justify on left (default is on right)
    ,       -   add comma's to field    
    *       -   width provided on stack
    n       -   Set output attribute to normal (for this field only)
    h       -   Set output attribute to highlight (for this field only)
    e       -   Set output attribute to error (for this field only)
    l       -   Value is 64 bits

    a       -   ascii string
    s       -   unicode string
    X       -   fixed 8 byte value in hex
    x       -   hex value
    d       -   value as decimal    
    c       -   Unicode char
    t       -   EFI time structure
    g       -   Pointer to GUID
    r       -   EFI status code (result code)

    N       -   Set output attribute to normal
    H       -   Set output attribute to highlight
    E       -   Set output attribute to error
    %       -   Print a %
    
Arguments:

    SystemTable     - The system table

Returns:

    Number of charactors written   

--*/
{
    CHAR16          c;
    UINTN           Attr;
    PRINT_ITEM      Item;
    CHAR16          Buffer[PRINT_STRING_LEN];
    //
    // If Omit print to ConOut, then return 0.
    //
    if (mPrintMode.OmitPrint && ((UINTN)ps->Context == (UINTN)ST->ConOut)) {
      return 0;
    }

    ps->Len = 0;
    ps->Buffer = Buffer;
    ps->Pos = Buffer;
    ps->End = Buffer + PRINT_STRING_LEN - 1;
    ps->Item = &Item;

    ps->fmt.Index = 0;
    c = PGETC(&ps->fmt);
    while (c) {

        if (c != '%') {
            PPUTC ( ps, c );
            c = PGETC(&ps->fmt);
            continue;   
        }

        // setup for new item
        Item.FieldWidth = (UINTN) -1;
        Item.Width = 0;
        Item.WidthParse = &Item.Width;
        Item.Pad = ' ';
        Item.PadBefore = TRUE;
        Item.Comma = FALSE;
        Item.Long = FALSE;
        Item.Item.Ascii = FALSE;
        Item.Item.u.pw = NULL;
        ps->RestoreAttr = 0;
        Attr = 0;

        c = PGETC(&ps->fmt);
        while (c) {

            switch (c) {
            
            case '%':
                //
                // %% -> %
                //
                Item.Item.u.pw = Item.Scratch;
                Item.Item.u.pw[0] = '%';  
                Item.Item.u.pw[1] = 0;
                break;

            case '0':
                Item.Pad = '0';
                break;

            case '-':
                Item.PadBefore = FALSE;
                break;

            case ',':
                Item.Comma = TRUE;
                break;

            case '.':
                Item.WidthParse = &Item.FieldWidth;
                break;

            case '*':
                *Item.WidthParse = va_arg(ps->args, UINTN);
                break;
            
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                *Item.WidthParse = 0;
                do {
                    *Item.WidthParse = *Item.WidthParse * 10 + c - '0';
                    c = PGETC(&ps->fmt);
                } while (c >= '0'  &&  c <= '9') ;
                ps->fmt.Index -= 1;
                break;

            case 'a':
                Item.Item.u.pc = va_arg(ps->args, CHAR8 *);
                Item.Item.Ascii = TRUE;
                if (!Item.Item.u.pc) {
                    Item.Item.u.pc = "(null)";
                }
                break;

            case 's':
                Item.Item.u.pw = va_arg(ps->args, CHAR16 *);
                if (!Item.Item.u.pw) {
                    Item.Item.u.pw = L"(null)";
                }
                break;

            case 'c':
                Item.Item.u.pw = Item.Scratch;
                Item.Item.u.pw[0] = (CHAR16) va_arg(ps->args, UINTN);  
                Item.Item.u.pw[1] = 0;
                break;

            case 'l':
                Item.Long = TRUE;
                break;

            case 'X':
                Item.Width = Item.Long ? 16 : 8;
                Item.Pad = '0';
            case 'x':
                Item.Item.u.pw = Item.Scratch;
                ValueToHex (
                    Item.Item.u.pw, 
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINTN)
                    );

                break;
        

            case 'g':
                Item.Item.u.pw = Item.Scratch;
                GuidToString (Item.Item.u.pw, va_arg(ps->args, EFI_GUID *));
                break;

            case 'd':
                Item.Item.u.pw = Item.Scratch;
                ValueToString (
                    Item.Item.u.pw, 
                    Item.Comma, 
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINTN)
                    );
                break
                    ;
            case 't':
                Item.Item.u.pw = Item.Scratch;
                TimeToString (Item.Item.u.pw, va_arg(ps->args, EFI_TIME *));
                break;

            case 'r':
                Item.Item.u.pw = Item.Scratch;
                StatusToString (Item.Item.u.pw, va_arg(ps->args, EFI_STATUS));
                break;

            case 'n':
                PSETATTR(ps, ps->AttrNorm);
                break;

            case 'h':
                PSETATTR(ps, ps->AttrHighlight);
                break;

            case 'e':
                PSETATTR(ps, ps->AttrError);
                break;

            case 'N':
                Attr = ps->AttrNorm;
                break;

            case 'H':
                Attr = ps->AttrHighlight;
                break;

            case 'E':
                Attr = ps->AttrError;
                break;

            default:
                Item.Item.u.pw = Item.Scratch;
                Item.Item.u.pw[0] = '?';
                Item.Item.u.pw[1] = 0;
                break;
            }

            // if we have an Item
            if (Item.Item.u.pw) {
                PITEM (ps);
                break;
            }

            // if we have an Attr set
            if (Attr) {
                PSETATTR(ps, Attr);
                ps->RestoreAttr = 0;
                break;
            }
            c = PGETC(&ps->fmt);
        }

        if (ps->RestoreAttr) {
            PSETATTR(ps, ps->RestoreAttr);
        }
        c = PGETC(&ps->fmt);
    }

    // Flush buffer
    PFLUSH (ps);
    return ps->Len;
}

VOID
ValueToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN INT64    v
    )
{
    STATIC CHAR8 ca[] = {  3, 1, 2 };
    CHAR8        str[40], *p1;
    CHAR16       *p2;
    UINTN        c, r;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    if (v < 0) {
        *(p2++) = '-';
        v = -v;
    }

    while (v) {
        v = (INT64)DivU64x32 ((UINT64)v, 10, &r);
        *(p1++) = (CHAR8)((CHAR8)r + '0');
    }

    c = (Comma ? ca[(p1 - str) % 3] : 999) + 1;
    while (p1 != str) {

        c -= 1;
        if (!c) {
            *(p2++) = ',';
            c = 3;
        }

        *(p2++) = *(--p1);
    }
    *p2 = 0;
}

VOID
TimeToString (
    OUT CHAR16      *Buffer,
    IN EFI_TIME     *Time
    )
{
    UINTN       Hour, Year;
    CHAR16      AmPm;

    AmPm = 'a';
    Hour = Time->Hour;
    if (Time->Hour == 0) {
        Hour = 12;
    } else if (Time->Hour >= 12) {
        AmPm = 'p';
        if (Time->Hour >= 13) {
            Hour -= 12;
        }
    }

    Year = Time->Year % 100;
    
    // bugbug: for now just print it any old way
    SPrint (Buffer, 0, L"%02d/%02d/%02d  %02d:%02d%c",
        Time->Month,
        Time->Day,
        Year,
        Hour,
        Time->Minute,
        AmPm
        );
} 



STATIC CHAR8 Hex[] = {'0','1','2','3','4','5','6','7',
                      '8','9','A','B','C','D','E','F'};

VOID
ValueToHex (
    IN CHAR16   *Buffer,
    IN UINT64   v
    )
{
    CHAR8           str[30], *p1;
    CHAR16          *p2;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    while (v) {
        *(p1++) = Hex[v & 0xf];
        v = RShiftU64 (v, 4);
    }

    while (p1 != str) {
        *(p2++) = *(--p1);
    }
    *p2 = 0;
}


VOID
DumpHex (
    IN UINTN        Indent,
    IN UINTN        Offset,
    IN UINTN        DataSize,
    IN VOID         *UserData
    )
{
    CHAR8           *Data, Val[50], Str[20], c;
    UINTN           Size, Index;

    Data = UserData;
    while (DataSize) {
        Size = 16;
        if (Size > DataSize) {
            Size = DataSize;
        }

        for (Index=0; Index < Size; Index += 1) {
            c = Data[Index];
            Val[Index*3+0] = Hex[c>>4];
            Val[Index*3+1] = Hex[c&0xF];
            Val[Index*3+2] = (CHAR8)((Index == 7)?'-':' ');
            Str[Index] = (CHAR8)((c < ' ' || c > 'z') ? '.' : c);
        }

        Val[Index*3] = 0;
        Str[Index] = 0;
        Print (L"%*a%X: %-.48a *%a*\n", Indent, "", Offset, Val, Str);

        Data += Size;
        Offset += Size;
        DataSize -= Size;
    }
}

VOID 
EnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  )
{
  mPrintMode.PageBreak = TRUE;
  mPrintMode.OmitPrint = FALSE;
  mPrintMode.InitRow   = StartRow;
  mPrintMode.AutoWrap  = AutoWrap;

  //
  // Query Mode
  //
  ST->ConOut->QueryMode (
                ST->ConOut, 
                ST->ConOut->Mode->Mode, 
                &mPrintMode.MaxColumn, 
                &mPrintMode.MaxRow
                );

  mPrintMode.Row = StartRow;
}

VOID 
DisablePageBreak (
  VOID
  )
{
  mPrintMode.PageBreak = FALSE;
  mPrintMode.OmitPrint = FALSE;
}


BOOLEAN
GetPageBreak (
  VOID
  )
{
  return mPrintMode.PageBreak;
}

STATIC
BOOLEAN
SetPageBreak (
  IN OUT PRINT_STATE     *ps
  )
{
  EFI_INPUT_KEY   Key;
  CHAR16          Str[3];

  ps->Output(ps->Context, L"Press ENTER to continue, 'q' to exit:");

  //
  // Wait for user input
  //
  Str[0] = ' ';
  Str[1] = 0;
  Str[2] = 0;
  for (;;) {
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
    ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);

    //
    // handle control keys
    //
    if (Key.UnicodeChar == CHAR_NULL) {
      if (Key.ScanCode == SCAN_ESC) {
        ps->Output(ps->Context, L"\r\n");
        mPrintMode.OmitPrint = TRUE;
        break;
      }
      continue;
    }

    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      ps->Output(ps->Context, L"\r\n");
      mPrintMode.Row = mPrintMode.InitRow;
      break; 
    }

    //
    // Echo input
    //
    Str[1] = Key.UnicodeChar;
    if (Str[1] == CHAR_BACKSPACE) {
      Str[1] = L' ';
    }
    ps->Output(ps->Context, Str);

    if ((Str[1] == L'q') || (Str[1] == L'Q')) { 
      mPrintMode.OmitPrint = TRUE;
    } else {
      mPrintMode.OmitPrint = FALSE;
    }

    Str[0] = CHAR_BACKSPACE;
  }

  return mPrintMode.OmitPrint;
}

STATIC
VOID
FlushWithPageBreak (
  IN OUT PRINT_STATE     *ps
  )
{
  CHAR16   *Pos;
  CHAR16   *LineStart;
  CHAR16   LineEndChar;

  Pos = ps->Buffer;
  LineStart = Pos;
  while ((*Pos != 0) && (Pos < ps->Pos)) {
    if (( *Pos == L'\n') && ( *(Pos - 1) == L'\r')) {
      //
      // Output one line
      //
      LineEndChar = *(Pos+1);
      *(Pos+1) = 0;
      ps->Output (ps->Context, LineStart);
      *(Pos+1) = LineEndChar; //restore line end char

      LineStart = Pos + 1;
      mPrintMode.Column = 0;
      mPrintMode.Row ++;
      if (mPrintMode.Row == mPrintMode.MaxRow) {
        if (SetPageBreak (ps)) {
          return;
        }
      }
    } 
    else {
      if (*Pos == CHAR_BACKSPACE) {
        mPrintMode.Column --;
      } else {
        mPrintMode.Column ++;
      }

      //
      // If column is at the end of line, output a new line feed.
      //
      if ((mPrintMode.Column == mPrintMode.MaxColumn) && (*Pos != L'\n') && (*Pos != L'\r')) {

        LineEndChar = *(Pos+1);
        *(Pos+1) = 0;
        ps->Output (ps->Context, LineStart);
        *(Pos+1) = LineEndChar; //restore line end char

        if (mPrintMode.AutoWrap) {
          ps->Output (ps->Context, L"\r\n");
        }

        LineStart = Pos + 1;
        mPrintMode.Column = 0;
        mPrintMode.Row ++;
        if (mPrintMode.Row == mPrintMode.MaxRow) {
          if (SetPageBreak (ps)) {
            return;
          }
        }
      }
    }
    Pos ++;
  }

  if (*LineStart != 0) {
    ps->Output (ps->Context, LineStart);
  }
}
