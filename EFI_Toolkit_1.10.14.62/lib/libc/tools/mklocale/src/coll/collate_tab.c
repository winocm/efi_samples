
/*  A Bison parser, made from collate.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	SUBSTITUTE	258
#define	WITH	259
#define	ORDER	260
#define	RANGE	261
#define	STRING	262
#define	CHAIN	263
#define	DEFN	264
#define	CHAR	265

#line 1 "collate.y"

/*-
 * Copyright (c) 1995 Alex Tatmanjants <alex@elvisti.kiev.ua>
 *		at Electronni Visti IA, Kiev, Ukraine.
 *			All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: collate_tab.c,v 1.1.1.1 2003/11/19 01:50:40 kyu3 Exp $
 */

#include <error.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "collate.h"

extern int line_no;
extern FILE *yyin;
void yyerror(char *fmt, ...);
int yyparse(void);
int yylex(void);
void print_usage (void);
void collate_print_tables();

char map_name[FILENAME_MAX] = ".";

char __collate_version[STR_LEN];
u_char charmap_table[UCHAR_MAX + 1][STR_LEN];
u_char __collate_substitute_table[UCHAR_MAX + 1][STR_LEN];
struct __collate_st_char_pri __collate_char_pri_table[UCHAR_MAX + 1];
struct __collate_st_chain_pri __collate_chain_pri_table[TABLE_SIZE];
int chain_index;
int prim_pri = 1, sec_pri = 1;
int debug = 0;

char *out_file = "LC_COLLATE";
char *in_file;


#line 63 "collate.y"
typedef union {
	u_char ch;
	u_char str[STR_LEN];
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		45
#define	YYFLAG		-32768
#define	YYNTBASE	18

#define YYTRANSLATE(x) ((unsigned)(x) <= 265 ? yytranslate[x] : 30)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    11,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    15,
    16,     2,     2,    17,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    12,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    13,     2,    14,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     8,     9,    11,    13,    15,    18,    23,
    26,    28,    32,    34,    36,    40,    44,    48,    50,    54,
    56,    60,    62,    66,    68,    70,    74
};

static const short yyrhs[] = {    19,
     0,    20,     0,    19,    11,    20,     0,     0,    21,     0,
    22,     0,    23,     0,     9,    10,     0,     3,     7,     4,
     7,     0,     5,    24,     0,    25,     0,    24,    12,    25,
     0,    10,     0,     8,     0,    10,     6,    10,     0,    13,
    26,    14,     0,    15,    27,    16,     0,    28,     0,    26,
    17,    28,     0,    29,     0,    27,    17,    29,     0,    10,
     0,    10,     6,    10,     0,     8,     0,    10,     0,    10,
     6,    10,     0,     8,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    73,    75,    76,    78,    79,    80,    81,    83,    87,    91,
   121,   122,   124,   129,   135,   147,   150,   155,   156,   158,
   159,   161,   166,   179,   186,   192,   206
};

static const char * const yytname[] = {   "$","error","$undefined.","SUBSTITUTE",
"WITH","ORDER","RANGE","STRING","CHAIN","DEFN","CHAR","'\\n'","';'","'{'","'}'",
"'('","')'","','","collate","statment_list","statment","charmap","substitute",
"order","order_list","item","prim_order_list","sec_order_list","prim_sub_item",
"sec_sub_item",""
};
#endif

static const short yyr1[] = {     0,
    18,    19,    19,    20,    20,    20,    20,    21,    22,    23,
    24,    24,    25,    25,    25,    25,    25,    26,    26,    27,
    27,    28,    28,    28,    29,    29,    29
};

static const short yyr2[] = {     0,
     1,     1,     3,     0,     1,     1,     1,     2,     4,     2,
     1,     3,     1,     1,     3,     3,     3,     1,     3,     1,
     3,     1,     3,     1,     1,     3,     1
};

static const short yydefact[] = {     4,
     0,     0,     0,     1,     2,     5,     6,     7,     0,    14,
    13,     0,     0,    10,    11,     8,     4,     0,     0,    24,
    22,     0,    18,    27,    25,     0,    20,     0,     3,     9,
    15,     0,    16,     0,     0,    17,     0,    12,    23,    19,
    26,    21,     0,     0,     0
};

static const short yydefgoto[] = {    43,
     4,     5,     6,     7,     8,    14,    15,    22,    26,    23,
    27
};

static const short yypact[] = {     1,
     2,    -8,     3,     6,-32768,-32768,-32768,-32768,    14,-32768,
    13,    -7,     4,     8,-32768,-32768,     1,    15,    11,-32768,
    17,    -6,-32768,-32768,    18,    -1,-32768,    -8,-32768,-32768,
-32768,    16,-32768,    -7,    19,-32768,     4,-32768,-32768,-32768,
-32768,-32768,    25,    27,-32768
};

static const short yypgoto[] = {-32768,
-32768,    20,-32768,-32768,-32768,-32768,     0,-32768,-32768,    -4,
    -5
};


#define	YYLAST		37


static const short yytable[] = {    10,
    20,    11,    21,     1,    12,     2,    13,    33,     9,     3,
    34,    24,    16,    25,    36,    37,    17,    18,    19,    28,
    31,    30,    32,    35,    44,    39,    45,    38,    41,    40,
     0,    42,     0,     0,     0,     0,    29
};

static const short yycheck[] = {     8,
     8,    10,    10,     3,    13,     5,    15,    14,     7,     9,
    17,     8,    10,    10,    16,    17,    11,     4,     6,    12,
    10,     7,     6,     6,     0,    10,     0,    28,    10,    34,
    -1,    37,    -1,    -1,    -1,    -1,    17
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 8:
#line 83 "collate.y"
{
	strcpy(charmap_table[yyvsp[0].ch], yyvsp[-1].str);
;
    break;}
case 9:
#line 87 "collate.y"
{
	strcpy(__collate_substitute_table[yyvsp[-2].str[0]], yyvsp[0].str);
;
    break;}
case 10:
#line 91 "collate.y"
{
	FILE *fp;
	int ch;

	for (ch = 0; ch < UCHAR_MAX + 1; ch++)
		if (!__collate_char_pri_table[ch].prim)
			yyerror("Char 0x%02x not present", ch);

	fp = fopen(out_file, "wb");
	if(!fp)
		printf( "can't open destination file %s",
		    out_file);

	strcpy(__collate_version, COLLATE_VERSION);
	fwrite(__collate_version, sizeof(__collate_version), 1, fp);

	fwrite(__collate_substitute_table, sizeof(__collate_substitute_table), 1, fp);
	fwrite(__collate_char_pri_table, sizeof(__collate_char_pri_table), 1, fp);
	fwrite(__collate_chain_pri_table, sizeof(__collate_chain_pri_table), 1, fp);
	if (fflush(fp))
		printf( "IO error writting to destination file %s",
		    out_file);
	fclose(fp);

	if (debug)
		collate_print_tables();

	exit(0);
;
    break;}
case 13:
#line 124 "collate.y"
{
	if (__collate_char_pri_table[yyvsp[0].ch].prim)
		yyerror("Char 0x%02x duplicated", yyvsp[0].ch);
	__collate_char_pri_table[yyvsp[0].ch].prim = prim_pri++;
;
    break;}
case 14:
#line 129 "collate.y"
{
	if (chain_index >= TABLE_SIZE - 1)
		yyerror("__collate_chain_pri_table overflow");
	strcpy(__collate_chain_pri_table[chain_index].str, yyvsp[0].str);
	__collate_chain_pri_table[chain_index++].prim = prim_pri++;
;
    break;}
case 15:
#line 135 "collate.y"
{
	u_int i;

	if (yyvsp[0].ch <= yyvsp[-2].ch)
		yyerror("Illegal range 0x%02x -- 0x%02x", yyvsp[-2].ch, yyvsp[0].ch);

	for (i = yyvsp[-2].ch; i <= yyvsp[0].ch; i++) {
		if (__collate_char_pri_table[(u_char)i].prim)
			yyerror("Char 0x%02x duplicated", (u_char)i);
		__collate_char_pri_table[(u_char)i].prim = prim_pri++;
	}
;
    break;}
case 16:
#line 147 "collate.y"
{
	prim_pri++;
;
    break;}
case 17:
#line 150 "collate.y"
{
	prim_pri++;
	sec_pri = 1;
;
    break;}
case 22:
#line 161 "collate.y"
{
	if (__collate_char_pri_table[yyvsp[0].ch].prim)
		yyerror("Char 0x%02x duplicated", yyvsp[0].ch);
	__collate_char_pri_table[yyvsp[0].ch].prim = prim_pri;
;
    break;}
case 23:
#line 166 "collate.y"
{
	u_int i;

	if (yyvsp[0].ch <= yyvsp[-2].ch)
		yyerror("Illegal range 0x%02x -- 0x%02x",
			yyvsp[-2].ch, yyvsp[0].ch);

	for (i = yyvsp[-2].ch; i <= yyvsp[0].ch; i++) {
		if (__collate_char_pri_table[(u_char)i].prim)
			yyerror("Char 0x%02x duplicated", (u_char)i);
		__collate_char_pri_table[(u_char)i].prim = prim_pri;
	}
;
    break;}
case 24:
#line 179 "collate.y"
{
	if (chain_index >= TABLE_SIZE - 1)
		yyerror("__collate_chain_pri_table overflow");
	strcpy(__collate_chain_pri_table[chain_index].str, yyvsp[0].str);
	__collate_chain_pri_table[chain_index++].prim = prim_pri;
;
    break;}
case 25:
#line 186 "collate.y"
{
	if (__collate_char_pri_table[yyvsp[0].ch].prim)
		yyerror("Char 0x%02x duplicated", yyvsp[0].ch);
	__collate_char_pri_table[yyvsp[0].ch].prim = prim_pri;
	__collate_char_pri_table[yyvsp[0].ch].sec = sec_pri++;
;
    break;}
case 26:
#line 192 "collate.y"
{
	u_int i;

	if (yyvsp[0].ch <= yyvsp[-2].ch)
		yyerror("Illegal range 0x%02x -- 0x%02x",
			yyvsp[-2].ch, yyvsp[0].ch);

	for (i = yyvsp[-2].ch; i <= yyvsp[0].ch; i++) {
		if (__collate_char_pri_table[(u_char)i].prim)
			yyerror("Char 0x%02x duplicated", (u_char)i);
		__collate_char_pri_table[(u_char)i].prim = prim_pri;
		__collate_char_pri_table[(u_char)i].sec = sec_pri++;
	}
;
    break;}
case 27:
#line 206 "collate.y"
{
	if (chain_index >= TABLE_SIZE - 1)
		yyerror("__collate_chain_pri_table overflow");
	strcpy(__collate_chain_pri_table[chain_index].str, yyvsp[0].str);
	__collate_chain_pri_table[chain_index].prim = prim_pri;
	__collate_chain_pri_table[chain_index++].sec = sec_pri++;
;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 214 "collate.y"

int
main(ac, av)
	char **av;
{
	int ch;
	int acpos;
	char *pch1;
	char *pch2;
	char *pch;
	int i;
	
// no parameter
	if ( ac < 2 || ac > 4 ) {
		print_usage();
		exit(1);
	}

    acpos = 1;
    
    if (stricmp(av[1], "-d") == 0 ) {
    	debug = 1;
    	acpos = 2;
    }
    

// no input file name provided    
	if ( ac - acpos < 1 ) {
		print_usage();
		exit(1);
	}

    if ( ac - acpos == 1 ) {
    
    // no output file name provided
		in_file = av[acpos];
	} else {
	
	//output file name provided
		out_file = av[acpos];
		in_file = av[acpos + 1];
	}
	

	if (( yyin = fopen(in_file, "r")) == 0) {
	    perror(in_file);
	    exit(1);
	}

    if ( strstr(in_file,"/") == NULL && strstr(in_file,"\\") == NULL) {
     	strcpy( map_name , ".");
	} else {
		pch1 = strstr(in_file,"/");
		pch2 = strstr(in_file,"\\");
		
		while ( pch1 != NULL || pch2 != NULL ) {
			if ( pch1 != NULL ) 
				pch = pch1;
			else
				pch = pch2;
				
			pch1 = strstr( pch + 1, "/");
			pch2 = strstr( pch + 1 , "\\");
		}
		
		// if the only "/" or "\\" is in the begnning
		if ( pch == in_file ) {
			strcpy(map_name, "/");
		} else {

			// get things before the last "/" or "\\"
			for ( i = 0 ; i < ( pch - in_file) ; i++ )
				map_name[i] = in_file[i];
						
			map_name[i] = '\0';
		}
	}
	
	
	for(ch = 0; ch <= UCHAR_MAX; ch++)
		__collate_substitute_table[ch][0] = ch;
	yyparse();
	return 0;
}

void print_usage()
{
	printf("Usage: mkcollate [-d] [outputfile] inputfile\n");
}


void yyerror(char *fmt, ...)
{
	va_list ap;
	char msg[128];

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);
	printf( "%s near line %d", msg, line_no);
}

void
collate_print_tables()
{
	int i;
	struct __collate_st_chain_pri *p2;

	printf("Substitute table:\n");
	for (i = 0; i < UCHAR_MAX + 1; i++)
	    if (i != *__collate_substitute_table[i])
		printf("\t'%c' --> \"%s\"\n", i,
		       __collate_substitute_table[i]);
	printf("Chain priority table:\n");
	for (p2 = __collate_chain_pri_table; p2->str[0]; p2++)
		printf("\t\"%s\" : %d %d\n\n", p2->str, p2->prim, p2->sec);
	printf("Char priority table:\n");
	for (i = 0; i < UCHAR_MAX + 1; i++)
		printf("\t'%c' : %d %d\n", i, __collate_char_pri_table[i].prim,
		       __collate_char_pri_table[i].sec);
}

