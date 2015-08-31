
# line 2 "ndr_parse.y"
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include "ndrgen.h"

typedef struct node *node_ptr;
#define YYSTYPE node_ptr
# define STRUCT_KW 257
# define UNION_KW 258
# define TYPEDEF_KW 259
# define ALIGN_KW 260
# define OPERATION_KW 261
# define IN_KW 262
# define OUT_KW 263
# define INTERFACE_KW 264
# define UUID_KW 265
# define _NO_REORDER_KW 266
# define EXTERN_KW 267
# define SIZE_IS_KW 268
# define LENGTH_IS_KW 269
# define STRING_KW 270
# define REFERENCE_KW 271
# define CASE_KW 272
# define DEFAULT_KW 273
# define SWITCH_IS_KW 274
# define TRANSMIT_AS_KW 275
# define ARG_IS_KW 276
# define BASIC_TYPE 277
# define TYPENAME 278
# define IDENTIFIER 279
# define INTEGER 280
# define STRING 281
# define LC 282
# define RC 283
# define SEMI 284
# define STAR 285
# define DIV 286
# define MOD 287
# define PLUS 288
# define MINUS 289
# define AND 290
# define OR 291
# define XOR 292
# define LB 293
# define RB 294
# define LP 295
# define RP 296
# define L_MEMBER 297

#include <inttypes.h>

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#define	YYCONST	const
#else
#include <malloc.h>
#include <memory.h>
#define	YYCONST
#endif

#include <values.h>

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
#ifndef yyerror
#if defined(__cplusplus)
	void yyerror(YYCONST char *);
#endif
#endif
#ifndef yylex
	int yylex(void);
#endif
	int yyparse(void);
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#endif

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 189 "ndr_parse.y"

static YYCONST yytabelem yyexca[] ={
-1, 0,
	0, 1,
	-2, 14,
-1, 1,
	0, -1,
	-2, 0,
-1, 2,
	0, 2,
	-2, 14,
	};
# define YYNPROD 66
# define YYLAST 192
static YYCONST yytabelem yyact[]={

    94,    95,    96,    97,    98,    99,   100,   101,   127,   126,
   125,   104,    94,    95,    96,    97,    98,    99,   100,   101,
   117,   110,   109,   102,    94,    95,    96,    97,    98,    99,
   100,   101,    65,   108,   107,    92,   116,   106,    63,    91,
    90,   115,    57,    56,    55,    54,    53,    52,    66,    51,
   114,   113,    50,    49,    48,   124,   123,   111,    69,    46,
     8,    10,    88,    10,    16,   122,   121,    10,    22,    21,
    19,    20,    31,    32,    33,    34,    24,    25,    23,    35,
    27,    28,    26,    30,    29,    59,    86,    58,    71,    72,
    73,    43,    44,   120,    10,   119,   118,    37,    38,    80,
    12,    13,    93,    61,    70,    84,    83,    36,    18,    17,
    64,    41,    37,    38,    14,    15,     7,     3,     7,    62,
    11,    39,     9,    42,    60,    40,    45,    47,     6,     5,
     4,     2,     1,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    67,    68,     0,    74,    75,    76,    77,    78,    79,
     0,    81,    82,     0,     0,     0,    85,    87,     0,     0,
    89,     0,     0,     0,     0,     0,     0,     0,     0,   103,
   105,     0,     0,     0,     0,     0,     0,     0,     0,   112,
     0,   112 };
static YYCONST yytabelem yypact[]={

  -199,-10000000,  -199,-10000000,-10000000,-10000000,-10000000,  -157,  -230,  -229,
  -192,-10000000,  -181,  -181,-10000000,  -166,  -192,  -235,  -192,-10000000,
-10000000,  -241,  -242,-10000000,  -243,  -246,  -248,  -249,-10000000,  -250,
  -251,  -252,  -253,-10000000,-10000000,-10000000,  -195,-10000000,-10000000,  -197,
  -247,-10000000,-10000000,  -181,  -181,  -236,-10000000,-10000000,  -191,  -191,
  -191,  -191,  -191,  -191,  -191,  -178,  -191,  -191,  -230,  -230,
  -198,-10000000,-10000000,  -247,  -231,-10000000,  -247,-10000000,-10000000,-10000000,
  -256,-10000000,-10000000,-10000000,  -257,  -261,  -273,  -285,  -259,  -262,
  -263,  -274,  -275,  -226,-10000000,  -232,-10000000,-10000000,  -244,  -276,
-10000000,-10000000,-10000000,  -184,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,  -185,-10000000,  -187,-10000000,-10000000,-10000000,-10000000,
-10000000,  -218,-10000000,  -219,-10000000,  -238,  -239,-10000000,  -286,  -287,
  -288,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000 };
static YYCONST yytabelem yypgo[]={

     0,   132,   131,   117,   130,   129,   128,   115,   107,   106,
   105,   125,   124,   122,   109,   108,   104,   102,   103,   119,
   110 };
static YYCONST yytabelem yyr1[]={

     0,     1,     1,     2,     2,     3,     3,     3,     4,     5,
     6,     9,     9,    10,     7,     7,    13,    13,    14,    14,
    15,    15,    15,    15,    15,    15,    15,    15,    15,    15,
    15,    15,    15,    15,    15,    15,    15,    15,    15,    15,
    16,    16,    16,    11,    11,    11,    11,     8,     8,    17,
    17,    17,    17,    17,    17,    17,    17,    12,    18,    18,
    19,    19,    19,    19,    20,    20 };
static YYCONST yytabelem yyr2[]={

     0,     0,     3,     2,     5,     2,     2,     2,    15,    15,
     5,     2,     5,     9,     1,     2,     7,     9,     2,     5,
     3,     3,     9,     9,     3,     9,    13,     9,    13,     9,
    13,     9,     3,     9,     9,     9,     9,     3,     3,     3,
     2,     2,     2,     2,     2,     5,     5,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     5,
     2,     7,     9,     9,     2,     7 };
static YYCONST yytabelem yychk[]={

-10000000,    -1,    -2,    -3,    -4,    -5,    -6,    -7,   259,   -13,
   293,    -3,   257,   258,   -10,    -7,   293,   -14,   -15,   262,
   263,   261,   260,   270,   268,   269,   274,   272,   273,   276,
   275,   264,   265,   266,   267,   271,    -8,   278,   279,    -8,
   -11,   277,    -8,   257,   258,   -14,   294,   -15,   295,   295,
   295,   295,   295,   295,   295,   295,   295,   295,   282,   282,
   -12,   -18,   -19,   285,   -20,   279,   295,    -8,    -8,   294,
   -16,   279,   280,   281,   -16,   -16,   -16,   -16,   -16,   -16,
   277,   -16,   -16,    -9,   -10,    -9,   284,   -18,   293,   -18,
   296,   296,   296,   -17,   285,   286,   287,   288,   289,   290,
   291,   292,   296,   -17,   296,   -17,   296,   296,   296,   296,
   296,   283,   -10,   283,   294,   285,   280,   296,   280,   280,
   280,   284,   284,   294,   294,   296,   296,   296 };
static YYCONST yytabelem yydef[]={

    -2,    -2,    -2,     3,     5,     6,     7,     0,    14,    15,
     0,     4,     0,     0,    10,     0,     0,     0,    18,    20,
    21,     0,     0,    24,     0,     0,     0,     0,    32,     0,
     0,     0,     0,    37,    38,    39,     0,    47,    48,     0,
     0,    43,    44,     0,     0,     0,    16,    19,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    14,    14,
     0,    57,    58,     0,    60,    64,     0,    45,    46,    17,
     0,    40,    41,    42,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    14,    11,    14,    13,    59,     0,     0,
    22,    23,    25,     0,    49,    50,    51,    52,    53,    54,
    55,    56,    27,     0,    29,     0,    31,    33,    34,    35,
    36,     0,    12,     0,    61,     0,     0,    65,     0,     0,
     0,     8,     9,    62,    63,    26,    28,    30 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{
#ifdef __cplusplus
const
#endif
char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"STRUCT_KW",	257,
	"UNION_KW",	258,
	"TYPEDEF_KW",	259,
	"ALIGN_KW",	260,
	"OPERATION_KW",	261,
	"IN_KW",	262,
	"OUT_KW",	263,
	"INTERFACE_KW",	264,
	"UUID_KW",	265,
	"_NO_REORDER_KW",	266,
	"EXTERN_KW",	267,
	"SIZE_IS_KW",	268,
	"LENGTH_IS_KW",	269,
	"STRING_KW",	270,
	"REFERENCE_KW",	271,
	"CASE_KW",	272,
	"DEFAULT_KW",	273,
	"SWITCH_IS_KW",	274,
	"TRANSMIT_AS_KW",	275,
	"ARG_IS_KW",	276,
	"BASIC_TYPE",	277,
	"TYPENAME",	278,
	"IDENTIFIER",	279,
	"INTEGER",	280,
	"STRING",	281,
	"LC",	282,
	"RC",	283,
	"SEMI",	284,
	"STAR",	285,
	"DIV",	286,
	"MOD",	287,
	"PLUS",	288,
	"MINUS",	289,
	"AND",	290,
	"OR",	291,
	"XOR",	292,
	"LB",	293,
	"RB",	294,
	"LP",	295,
	"RP",	296,
	"L_MEMBER",	297,
	"-unknown-",	-1	/* ends search */
};

#ifdef __cplusplus
const
#endif
char * yyreds[] =
{
	"-no such reduction-",
	"defn : /* empty */",
	"defn : construct_list",
	"construct_list : construct",
	"construct_list : construct_list construct",
	"construct : struct",
	"construct : union",
	"construct : typedef",
	"struct : advice STRUCT_KW typename LC members RC SEMI",
	"union : advice UNION_KW typename LC members RC SEMI",
	"typedef : TYPEDEF_KW member",
	"members : member",
	"members : members member",
	"member : advice type declarator SEMI",
	"advice : /* empty */",
	"advice : adv_list",
	"adv_list : LB adv_attrs RB",
	"adv_list : adv_list LB adv_attrs RB",
	"adv_attrs : adv_attr",
	"adv_attrs : adv_attr adv_attr",
	"adv_attr : IN_KW",
	"adv_attr : OUT_KW",
	"adv_attr : OPERATION_KW LP arg RP",
	"adv_attr : ALIGN_KW LP arg RP",
	"adv_attr : STRING_KW",
	"adv_attr : SIZE_IS_KW LP arg RP",
	"adv_attr : SIZE_IS_KW LP arg operator INTEGER RP",
	"adv_attr : LENGTH_IS_KW LP arg RP",
	"adv_attr : LENGTH_IS_KW LP arg operator INTEGER RP",
	"adv_attr : SWITCH_IS_KW LP arg RP",
	"adv_attr : SWITCH_IS_KW LP arg operator INTEGER RP",
	"adv_attr : CASE_KW LP arg RP",
	"adv_attr : DEFAULT_KW",
	"adv_attr : ARG_IS_KW LP arg RP",
	"adv_attr : TRANSMIT_AS_KW LP BASIC_TYPE RP",
	"adv_attr : INTERFACE_KW LP arg RP",
	"adv_attr : UUID_KW LP arg RP",
	"adv_attr : _NO_REORDER_KW",
	"adv_attr : EXTERN_KW",
	"adv_attr : REFERENCE_KW",
	"arg : IDENTIFIER",
	"arg : INTEGER",
	"arg : STRING",
	"type : BASIC_TYPE",
	"type : typename",
	"type : STRUCT_KW typename",
	"type : UNION_KW typename",
	"typename : TYPENAME",
	"typename : IDENTIFIER",
	"operator : STAR",
	"operator : DIV",
	"operator : MOD",
	"operator : PLUS",
	"operator : MINUS",
	"operator : AND",
	"operator : OR",
	"operator : XOR",
	"declarator : decl1",
	"decl1 : decl2",
	"decl1 : STAR decl1",
	"decl2 : decl3",
	"decl2 : decl3 LB RB",
	"decl2 : decl3 LB STAR RB",
	"decl2 : decl3 LB INTEGER RB",
	"decl3 : IDENTIFIER",
	"decl3 : LP decl1 RP",
};
#endif /* YYDEBUG */
# line	1 "/usr/share/lib/ccs/yaccpar"
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/* Copyright (c) 1988 AT&T */
/* All Rights Reserved */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yymaxdepth * sizeof (type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt = 0;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside
	switch should never be executed
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			long yyps_index = (yy_ps - yys);
			long yypv_index = (yy_pv - yyv);
			long yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register YYCONST int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 2:
# line 58 "ndr_parse.y"
{ construct_list = (struct node *)yypvt[-0]; } break;
case 4:
# line 62 "ndr_parse.y"
{ n_splice (yypvt[-1],yypvt[-0]); } break;
case 8:
# line 71 "ndr_parse.y"
{ yyval = n_cons (STRUCT_KW, yypvt[-6], yypvt[-4], yypvt[-2]);
		   construct_fixup (yyval);
		} break;
case 9:
# line 77 "ndr_parse.y"
{ yyval = n_cons (UNION_KW, yypvt[-6], yypvt[-4], yypvt[-2]);
		   construct_fixup (yyval);
		} break;
case 10:
# line 83 "ndr_parse.y"
{ yyval = n_cons (TYPEDEF_KW, 0, yypvt[-0]->n_m_name, yypvt[-0]);
		   construct_fixup (yyval);
		} break;
case 12:
# line 89 "ndr_parse.y"
{ n_splice (yypvt[-1],yypvt[-0]); } break;
case 13:
# line 93 "ndr_parse.y"
{ yyval = n_cons (L_MEMBER, yypvt[-3], yypvt[-2], yypvt[-1]);
		   member_fixup (yyval);
		} break;
case 14:
# line 98 "ndr_parse.y"
{ yyval = 0; } break;
case 16:
# line 102 "ndr_parse.y"
{ yyval = yypvt[-1]; } break;
case 17:
# line 103 "ndr_parse.y"
{ n_splice (yypvt[-3],yypvt[-1]); } break;
case 19:
# line 107 "ndr_parse.y"
{ n_splice (yypvt[-1],yypvt[-0]); } break;
case 20:
# line 110 "ndr_parse.y"
{ yyval = n_cons (IN_KW); } break;
case 21:
# line 111 "ndr_parse.y"
{ yyval = n_cons (OUT_KW); } break;
case 22:
# line 112 "ndr_parse.y"
{ yyval = n_cons (OPERATION_KW, yypvt[-1]); } break;
case 23:
# line 113 "ndr_parse.y"
{ yyval = n_cons (ALIGN_KW, yypvt[-1]); } break;
case 24:
# line 114 "ndr_parse.y"
{ yyval = n_cons (STRING_KW); } break;
case 25:
# line 117 "ndr_parse.y"
{ yyval = n_cons (SIZE_IS_KW, yypvt[-1], yypvt[-1], yypvt[-1]); } break;
case 26:
# line 119 "ndr_parse.y"
{ yyval = n_cons (SIZE_IS_KW, yypvt[-3], yypvt[-2], yypvt[-1]); } break;
case 27:
# line 122 "ndr_parse.y"
{ yyval = n_cons (LENGTH_IS_KW, yypvt[-1], yypvt[-1], yypvt[-1]); } break;
case 28:
# line 124 "ndr_parse.y"
{ yyval = n_cons (LENGTH_IS_KW, yypvt[-3], yypvt[-2], yypvt[-1]); } break;
case 29:
# line 127 "ndr_parse.y"
{ yyval = n_cons (SWITCH_IS_KW, yypvt[-1], yypvt[-1], yypvt[-1]); } break;
case 30:
# line 129 "ndr_parse.y"
{ yyval = n_cons (SWITCH_IS_KW, yypvt[-3], yypvt[-2], yypvt[-1]); } break;
case 31:
# line 131 "ndr_parse.y"
{ yyval = n_cons (CASE_KW, yypvt[-1]); } break;
case 32:
# line 132 "ndr_parse.y"
{ yyval = n_cons (DEFAULT_KW); } break;
case 33:
# line 134 "ndr_parse.y"
{ yyval = n_cons (ARG_IS_KW, yypvt[-1]); } break;
case 34:
# line 136 "ndr_parse.y"
{ yyval = n_cons (TRANSMIT_AS_KW, yypvt[-1]); } break;
case 35:
# line 138 "ndr_parse.y"
{ yyval = n_cons (INTERFACE_KW, yypvt[-1]); } break;
case 36:
# line 139 "ndr_parse.y"
{ yyval = n_cons (UUID_KW, yypvt[-1]); } break;
case 37:
# line 140 "ndr_parse.y"
{ yyval = n_cons (_NO_REORDER_KW); } break;
case 38:
# line 141 "ndr_parse.y"
{ yyval = n_cons (EXTERN_KW); } break;
case 39:
# line 142 "ndr_parse.y"
{ yyval = n_cons (REFERENCE_KW); } break;
case 45:
# line 152 "ndr_parse.y"
{ yyval = yypvt[-0]; } break;
case 46:
# line 153 "ndr_parse.y"
{ yyval = yypvt[-0]; } break;
case 59:
# line 174 "ndr_parse.y"
{ yyval = n_cons (STAR, yypvt[-0]); } break;
case 61:
# line 178 "ndr_parse.y"
{ yyval = n_cons (LB, yypvt[-2], 0); } break;
case 62:
# line 179 "ndr_parse.y"
{ yyval = n_cons (LB, yypvt[-3], 0); } break;
case 63:
# line 180 "ndr_parse.y"
{ yyval = n_cons (LB, yypvt[-3], yypvt[-1]); } break;
case 65:
# line 184 "ndr_parse.y"
{ yyval = n_cons (LP, yypvt[-1]); } break;
# line	556 "/usr/share/lib/ccs/yaccpar"
	}
	goto yystack;		/* reset registers in driver code */
}

