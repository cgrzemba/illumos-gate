
# line 2 "nwamcfg_grammar.y"
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
 * Copyright 2010 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <stdio.h>
#include <sys/types.h>

#include "nwamcfg.h"

static cmd_t *cmd = NULL;		/* Command being processed */

/* yacc externals */
extern int yydebug;
extern void yyerror(char *s);

extern boolean_t newline_terminated;


# line 43 "nwamcfg_grammar.y"
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
 {
	int ival;
	char *strval;
	cmd_t *cmd;
} YYSTYPE;
# define CANCEL 257
# define CLEAR 258
# define COMMIT 259
# define CREATE 260
# define DESTROY 261
# define END 262
# define EXIT 263
# define EXPORT 264
# define GET 265
# define HELP 266
# define LIST 267
# define REVERT 268
# define SELECT 269
# define SET 270
# define VERIFY 271
# define WALKPROP 272
# define LOC 273
# define NCP 274
# define NCU 275
# define ENM 276
# define WLAN 277
# define PHYS 278
# define IP 279
# define TOKEN 280
# define EQUAL 281
# define OPTION 282
# define UNKNOWN 283
# define ACTIVATION_MODE 284
# define CONDITIONS 285
# define ENABLED 286
# define TYPE 287
# define CLASS 288
# define PARENT 289
# define PRIORITY_GROUP 290
# define PRIORITY_MODE 291
# define LINK_MACADDR 292
# define LINK_AUTOPUSH 293
# define LINK_MTU 294
# define IP_VERSION 295
# define IPV4_ADDRSRC 296
# define IPV4_ADDR 297
# define IPV4_DEFAULT_ROUTE 298
# define IPV6_ADDRSRC 299
# define IPV6_ADDR 300
# define IPV6_DEFAULT_ROUTE 301
# define ENM_STATE 302
# define ENM_FMRI 303
# define ENM_START 304
# define ENM_STOP 305
# define LOC_NAMESERVICES 306
# define LOC_NAMESERVICES_CONFIG 307
# define LOC_DNS_CONFIGSRC 308
# define LOC_DNS_DOMAIN 309
# define LOC_DNS_SERVERS 310
# define LOC_DNS_SEARCH 311
# define LOC_NIS_CONFIGSRC 312
# define LOC_NIS_SERVERS 313
# define LOC_LDAP_CONFIGSRC 314
# define LOC_LDAP_SERVERS 315
# define LOC_DEFAULT_DOMAIN 316
# define LOC_NFSV4_DOMAIN 317
# define LOC_IPF_CONFIG 318
# define LOC_IPF_V6_CONFIG 319
# define LOC_IPNAT_CONFIG 320
# define LOC_IPPOOL_CONFIG 321
# define LOC_IKE_CONFIG 322
# define LOC_IPSECPOL_CONFIG 323
# define WLAN_SSID 324
# define WLAN_BSSID 325
# define WLAN_KEYNAME 326
# define WLAN_PRIORITY 327
# define WLAN_DISABLED 328
# define WLAN_EAP_USER 329
# define WLAN_EAP_ANON 330
# define WLAN_CA_CERT 331
# define WLAN_PRIV 332
# define WLAN_CLI_CERT 333

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

# line 908 "nwamcfg_grammar.y"

static YYCONST yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 144
# define YYLAST 532
static YYCONST yytabelem yyact[]={

   110,   123,   112,    44,    45,    46,    47,    48,    49,    50,
    51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
    61,    62,   147,    63,    64,    65,    66,    67,    68,    69,
    70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
    80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
    90,    91,    92,    93,   121,   169,   168,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,   167,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89,    90,    91,    92,    93,    42,   164,
   162,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
   161,    63,    64,    65,    66,    67,    68,    69,    70,    71,
    72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
    82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
    92,    93,    44,    45,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
    62,    21,    63,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
    91,    92,    93,    98,    99,   102,   100,   101,   160,   133,
   114,   134,   115,   159,    98,    99,   102,   100,   101,   156,
    22,   106,   155,   107,    98,    99,   102,   100,   101,   154,
   151,    94,   148,    97,    98,    99,   102,   100,   101,   144,
   127,   128,   165,   103,    98,    99,   102,   100,   101,   141,
   135,   118,   127,   128,   157,   127,   128,   145,   127,   128,
   142,   127,   128,   136,   127,   128,   131,    98,    99,   102,
   100,   101,   130,   127,   128,    21,   129,   125,   113,    96,
    43,    21,     4,    20,    19,    39,    41,    18,    17,    16,
    15,    14,    13,   126,    12,    11,    10,     9,     8,     7,
     6,     5,     2,     1,     0,     0,     0,   105,     0,     0,
   109,     0,   111,   117,     0,   120,     0,   122,     0,    95,
     0,     0,     0,   124,    22,     0,     0,     0,     0,     0,
    22,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   104,     0,     0,
   108,     0,     0,   116,     0,   119,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   138,     0,   140,     0,     0,     0,   132,
     0,     0,     0,   137,     0,     0,     0,     0,     0,   150,
     0,   143,     0,   153,   146,     0,     0,     3,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
    35,    36,    37,    38,   158,   139,     0,     0,     0,     0,
     0,     0,     0,     0,   163,     0,     0,   166,     0,   149,
     0,     0,     0,   152,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    40 };
static YYCONST yytabelem yypact[]={

   171,-10000000,   275,   281,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,  -172,-10000000,   -39,   -29,-10000000,-10000000,
   -49,  -280,     8,   -60,-10000000,   -19,  -226,-10000000,  -281,-10000000,
   281,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,     7,     5,     6,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,     2,    -4,-10000000,   -61,   -20,    -7,
-10000000,-10000000,  -121,-10000000,-10000000,     4,   -21,   -10,-10000000,   -31,
   -13,-10000000,  -259,-10000000,-10000000,-10000000,   -38,-10000000,-10000000,     4,
-10000000,-10000000,   -40,     4,   -41,-10000000,-10000000,   -48,-10000000,   -51,
   -16,-10000000,-10000000,   -57,-10000000,-10000000,   -62,  -150,-10000000,  -170,
     5,-10000000,  -171,   -28,-10000000,-10000000,-10000000,-10000000,  -204,-10000000,
-10000000,-10000000,-10000000,  -224,-10000000,-10000000,  -225,-10000000,-10000000,-10000000 };
static YYCONST yytabelem yypgo[]={

     0,   313,   329,   289,   303,   290,   312,   311,   310,   309,
   308,   307,   306,   305,   304,   302,   301,   300,   299,   298,
   297,   294,   293,   292 };
static YYCONST yytabelem yyr1[]={

     0,     1,     1,     1,     1,     6,     6,     6,     6,     6,
     6,     6,     6,     6,     6,     6,     6,     6,     6,     6,
     6,    23,    23,     7,     8,     8,     8,     9,    10,    10,
    10,    10,    10,    10,    10,    10,    11,    11,    11,    11,
    11,    11,    11,    12,    13,    14,    14,    14,    14,    14,
    14,    14,    14,    14,    14,    14,    15,    15,    15,    15,
    16,    16,    17,    17,    17,    17,    17,    17,    17,    17,
    17,    17,    17,    18,    19,    19,    19,    19,    19,    19,
    19,    20,    20,    20,    21,    22,    22,     2,     2,     2,
     2,     3,     4,     4,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5 };
static YYCONST yytabelem yyr2[]={

     0,     5,     7,     5,     3,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     3,     3,     3,     3,     5,     5,     3,     3,     5,
     5,     5,     7,     9,    11,    13,     3,     5,     5,     5,
     7,     7,     9,     3,     3,     3,     5,     5,     7,     9,
     7,     7,     9,    11,    11,    13,     3,     5,     5,     7,
     3,     5,     3,     5,     5,     5,     5,     7,     7,     9,
     9,     9,    11,     3,     3,     5,     5,     5,     7,     7,
     9,     3,     5,     9,     3,     3,     5,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3 };
static YYCONST yytabelem yychk[]={

-10000000,    -1,    -6,   256,   -23,    -7,    -8,    -9,   -10,   -11,
   -12,   -13,   -14,   -15,   -16,   -17,   -18,   -19,   -20,   -21,
   -22,    10,    59,   257,   258,   259,   260,   261,   262,   263,
   264,   265,   266,   267,   268,   269,   270,   271,   272,   -23,
   256,   -23,   280,    -5,   283,   284,   285,   286,   287,   288,
   289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
   299,   300,   301,   303,   304,   305,   306,   307,   308,   309,
   310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
   320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
   330,   331,   332,   333,   280,    -2,    -3,   282,   273,   274,
   276,   277,   275,   282,    -2,    -3,   280,   282,    -2,    -3,
   280,    -5,   282,   280,   280,   282,    -2,    -3,   280,    -2,
    -3,   280,    -5,   282,   -23,   280,    -4,   278,   279,   280,
   280,   280,    -4,   280,   282,   280,   280,    -4,    -5,    -2,
    -3,   280,   280,    -4,   280,   280,    -4,   281,   280,    -2,
    -3,   280,    -2,    -3,   280,   280,   280,   280,    -4,   280,
   280,   280,   280,    -4,   280,   280,    -4,   280,   280,   280 };
static YYCONST yytabelem yydef[]={

     0,    -2,     0,     0,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    27,    28,    36,    43,    44,
    45,    56,    60,    62,    73,    74,    81,    84,    85,     1,
     0,     3,    25,    26,    94,    95,    96,    97,    98,    99,
   100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
   110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
   120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
   130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
   140,   141,   142,   143,    29,    30,    31,     0,    87,    88,
    89,    90,    91,    37,    38,    39,    46,    47,     0,     0,
    57,    58,     0,    61,    63,    64,    65,    66,    75,    76,
    77,    82,     0,    86,     2,    32,     0,    92,    93,     0,
    40,    41,     0,    48,     0,    50,    51,     0,    59,     0,
     0,    67,    68,     0,    78,    79,     0,     0,    33,     0,
     0,    42,     0,     0,    49,    52,    70,    71,     0,    69,
    80,    83,    34,     0,    53,    54,     0,    72,    35,    55 };
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
	"CANCEL",	257,
	"CLEAR",	258,
	"COMMIT",	259,
	"CREATE",	260,
	"DESTROY",	261,
	"END",	262,
	"EXIT",	263,
	"EXPORT",	264,
	"GET",	265,
	"HELP",	266,
	"LIST",	267,
	"REVERT",	268,
	"SELECT",	269,
	"SET",	270,
	"VERIFY",	271,
	"WALKPROP",	272,
	"LOC",	273,
	"NCP",	274,
	"NCU",	275,
	"ENM",	276,
	"WLAN",	277,
	"PHYS",	278,
	"IP",	279,
	"TOKEN",	280,
	"EQUAL",	281,
	"OPTION",	282,
	"UNKNOWN",	283,
	"ACTIVATION_MODE",	284,
	"CONDITIONS",	285,
	"ENABLED",	286,
	"TYPE",	287,
	"CLASS",	288,
	"PARENT",	289,
	"PRIORITY_GROUP",	290,
	"PRIORITY_MODE",	291,
	"LINK_MACADDR",	292,
	"LINK_AUTOPUSH",	293,
	"LINK_MTU",	294,
	"IP_VERSION",	295,
	"IPV4_ADDRSRC",	296,
	"IPV4_ADDR",	297,
	"IPV4_DEFAULT_ROUTE",	298,
	"IPV6_ADDRSRC",	299,
	"IPV6_ADDR",	300,
	"IPV6_DEFAULT_ROUTE",	301,
	"ENM_STATE",	302,
	"ENM_FMRI",	303,
	"ENM_START",	304,
	"ENM_STOP",	305,
	"LOC_NAMESERVICES",	306,
	"LOC_NAMESERVICES_CONFIG",	307,
	"LOC_DNS_CONFIGSRC",	308,
	"LOC_DNS_DOMAIN",	309,
	"LOC_DNS_SERVERS",	310,
	"LOC_DNS_SEARCH",	311,
	"LOC_NIS_CONFIGSRC",	312,
	"LOC_NIS_SERVERS",	313,
	"LOC_LDAP_CONFIGSRC",	314,
	"LOC_LDAP_SERVERS",	315,
	"LOC_DEFAULT_DOMAIN",	316,
	"LOC_NFSV4_DOMAIN",	317,
	"LOC_IPF_CONFIG",	318,
	"LOC_IPF_V6_CONFIG",	319,
	"LOC_IPNAT_CONFIG",	320,
	"LOC_IPPOOL_CONFIG",	321,
	"LOC_IKE_CONFIG",	322,
	"LOC_IPSECPOL_CONFIG",	323,
	"WLAN_SSID",	324,
	"WLAN_BSSID",	325,
	"WLAN_KEYNAME",	326,
	"WLAN_PRIORITY",	327,
	"WLAN_DISABLED",	328,
	"WLAN_EAP_USER",	329,
	"WLAN_EAP_ANON",	330,
	"WLAN_CA_CERT",	331,
	"WLAN_PRIV",	332,
	"WLAN_CLI_CERT",	333,
	"-unknown-",	-1	/* ends search */
};

#ifdef __cplusplus
const
#endif
char * yyreds[] =
{
	"-no such reduction-",
	"commands : command terminator",
	"commands : command error terminator",
	"commands : error terminator",
	"commands : terminator",
	"command : cancel_command",
	"command : clear_command",
	"command : commit_command",
	"command : create_command",
	"command : destroy_command",
	"command : end_command",
	"command : exit_command",
	"command : export_command",
	"command : get_command",
	"command : help_command",
	"command : list_command",
	"command : revert_command",
	"command : select_command",
	"command : set_command",
	"command : verify_command",
	"command : walkprop_command",
	"terminator : '\n'",
	"terminator : ';'",
	"cancel_command : CANCEL",
	"clear_command : CLEAR",
	"clear_command : CLEAR TOKEN",
	"clear_command : CLEAR property_type",
	"commit_command : COMMIT",
	"create_command : CREATE",
	"create_command : CREATE TOKEN",
	"create_command : CREATE resource1_type",
	"create_command : CREATE resource2_type",
	"create_command : CREATE resource1_type TOKEN",
	"create_command : CREATE resource2_type ncu_class_type TOKEN",
	"create_command : CREATE OPTION TOKEN resource1_type TOKEN",
	"create_command : CREATE OPTION TOKEN resource2_type ncu_class_type TOKEN",
	"destroy_command : DESTROY",
	"destroy_command : DESTROY OPTION",
	"destroy_command : DESTROY resource1_type",
	"destroy_command : DESTROY resource2_type",
	"destroy_command : DESTROY resource1_type TOKEN",
	"destroy_command : DESTROY resource2_type TOKEN",
	"destroy_command : DESTROY resource2_type ncu_class_type TOKEN",
	"end_command : END",
	"exit_command : EXIT",
	"export_command : EXPORT",
	"export_command : EXPORT TOKEN",
	"export_command : EXPORT OPTION",
	"export_command : EXPORT OPTION TOKEN",
	"export_command : EXPORT OPTION OPTION TOKEN",
	"export_command : EXPORT resource1_type TOKEN",
	"export_command : EXPORT resource2_type TOKEN",
	"export_command : EXPORT resource2_type ncu_class_type TOKEN",
	"export_command : EXPORT OPTION TOKEN resource1_type TOKEN",
	"export_command : EXPORT OPTION TOKEN resource2_type TOKEN",
	"export_command : EXPORT OPTION TOKEN resource2_type ncu_class_type TOKEN",
	"get_command : GET",
	"get_command : GET TOKEN",
	"get_command : GET property_type",
	"get_command : GET OPTION property_type",
	"help_command : HELP",
	"help_command : HELP TOKEN",
	"list_command : LIST",
	"list_command : LIST TOKEN",
	"list_command : LIST OPTION",
	"list_command : LIST resource1_type",
	"list_command : LIST resource2_type",
	"list_command : LIST resource1_type TOKEN",
	"list_command : LIST resource2_type TOKEN",
	"list_command : LIST resource2_type ncu_class_type TOKEN",
	"list_command : LIST OPTION resource1_type TOKEN",
	"list_command : LIST OPTION resource2_type TOKEN",
	"list_command : LIST OPTION resource2_type ncu_class_type TOKEN",
	"revert_command : REVERT",
	"select_command : SELECT",
	"select_command : SELECT TOKEN",
	"select_command : SELECT resource1_type",
	"select_command : SELECT resource2_type",
	"select_command : SELECT resource1_type TOKEN",
	"select_command : SELECT resource2_type TOKEN",
	"select_command : SELECT resource2_type ncu_class_type TOKEN",
	"set_command : SET",
	"set_command : SET TOKEN",
	"set_command : SET property_type EQUAL TOKEN",
	"verify_command : VERIFY",
	"walkprop_command : WALKPROP",
	"walkprop_command : WALKPROP OPTION",
	"resource1_type : LOC",
	"resource1_type : NCP",
	"resource1_type : ENM",
	"resource1_type : WLAN",
	"resource2_type : NCU",
	"ncu_class_type : PHYS",
	"ncu_class_type : IP",
	"property_type : UNKNOWN",
	"property_type : ACTIVATION_MODE",
	"property_type : CONDITIONS",
	"property_type : ENABLED",
	"property_type : TYPE",
	"property_type : CLASS",
	"property_type : PARENT",
	"property_type : PRIORITY_GROUP",
	"property_type : PRIORITY_MODE",
	"property_type : LINK_MACADDR",
	"property_type : LINK_AUTOPUSH",
	"property_type : LINK_MTU",
	"property_type : IP_VERSION",
	"property_type : IPV4_ADDRSRC",
	"property_type : IPV4_ADDR",
	"property_type : IPV4_DEFAULT_ROUTE",
	"property_type : IPV6_ADDRSRC",
	"property_type : IPV6_ADDR",
	"property_type : IPV6_DEFAULT_ROUTE",
	"property_type : ENM_FMRI",
	"property_type : ENM_START",
	"property_type : ENM_STOP",
	"property_type : LOC_NAMESERVICES",
	"property_type : LOC_NAMESERVICES_CONFIG",
	"property_type : LOC_DNS_CONFIGSRC",
	"property_type : LOC_DNS_DOMAIN",
	"property_type : LOC_DNS_SERVERS",
	"property_type : LOC_DNS_SEARCH",
	"property_type : LOC_NIS_CONFIGSRC",
	"property_type : LOC_NIS_SERVERS",
	"property_type : LOC_LDAP_CONFIGSRC",
	"property_type : LOC_LDAP_SERVERS",
	"property_type : LOC_DEFAULT_DOMAIN",
	"property_type : LOC_NFSV4_DOMAIN",
	"property_type : LOC_IPF_CONFIG",
	"property_type : LOC_IPF_V6_CONFIG",
	"property_type : LOC_IPNAT_CONFIG",
	"property_type : LOC_IPPOOL_CONFIG",
	"property_type : LOC_IKE_CONFIG",
	"property_type : LOC_IPSECPOL_CONFIG",
	"property_type : WLAN_SSID",
	"property_type : WLAN_BSSID",
	"property_type : WLAN_KEYNAME",
	"property_type : WLAN_PRIORITY",
	"property_type : WLAN_DISABLED",
	"property_type : WLAN_EAP_USER",
	"property_type : WLAN_EAP_ANON",
	"property_type : WLAN_CA_CERT",
	"property_type : WLAN_PRIV",
	"property_type : WLAN_CLI_CERT",
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
		
case 1:
# line 111 "nwamcfg_grammar.y"
{
		if (yypvt[-1].cmd != NULL) {
			if (yypvt[-1].cmd->cmd_handler != NULL)
				if (check_scope(yypvt[-1].cmd->cmd_num))
					yypvt[-1].cmd->cmd_handler(yypvt[-1].cmd);
			free_cmd(yypvt[-1].cmd);
		}
		return (0);
	} break;
case 2:
# line 121 "nwamcfg_grammar.y"
{
		if (yypvt[-2].cmd != NULL)
			free_cmd(yypvt[-2].cmd);
		if (YYRECOVERING())
			YYABORT;
		yyclearin;
		yyerrok;
	} break;
case 3:
# line 130 "nwamcfg_grammar.y"
{
		if (YYRECOVERING())
			YYABORT;
		yyclearin;
		yyerrok;
	} break;
case 4:
# line 137 "nwamcfg_grammar.y"
{
		return (0);
	} break;
case 21:
# line 158 "nwamcfg_grammar.y"
{ newline_terminated = B_TRUE; } break;
case 22:
# line 159 "nwamcfg_grammar.y"
{ newline_terminated = B_FALSE; } break;
case 23:
# line 162 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_CANCEL;
		yyval.cmd->cmd_handler = &cancel_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 24:
# line 173 "nwamcfg_grammar.y"
{
		command_usage(CMD_CLEAR);
		YYERROR;
	} break;
case 25:
# line 178 "nwamcfg_grammar.y"
{
		properr(yypvt[-0].strval);
		YYERROR;
	} break;
case 26:
# line 183 "nwamcfg_grammar.y"
{
		/* clear prop */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_CLEAR;
		yyval.cmd->cmd_handler = &clear_func;
		yyval.cmd->cmd_prop_type = yypvt[-0].ival;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 27:
# line 196 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_COMMIT;
		yyval.cmd->cmd_handler = &commit_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 28:
# line 207 "nwamcfg_grammar.y"
{
		command_usage(CMD_CREATE);
		YYERROR;
	} break;
case 29:
# line 212 "nwamcfg_grammar.y"
{
		command_usage(CMD_CREATE);
		YYERROR;
	} break;
case 30:
# line 217 "nwamcfg_grammar.y"
{
		command_usage(CMD_CREATE);
		YYERROR;
	} break;
case 31:
# line 222 "nwamcfg_grammar.y"
{
		command_usage(CMD_CREATE);
		YYERROR;
	} break;
case 32:
# line 227 "nwamcfg_grammar.y"
{
		/* create enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_CREATE;
		yyval.cmd->cmd_handler = &create_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 33:
# line 240 "nwamcfg_grammar.y"
{
		/* create ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_CREATE;
		yyval.cmd->cmd_handler = &create_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 34:
# line 255 "nwamcfg_grammar.y"
{
		/* create -t old enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_CREATE;
		yyval.cmd->cmd_handler = &create_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 3;
		yyval.cmd->cmd_argv[0] = yypvt[-3].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-2].strval;
		yyval.cmd->cmd_argv[2] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[3] = NULL;
	} break;
case 35:
# line 270 "nwamcfg_grammar.y"
{
		/* create -t old ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_CREATE;
		yyval.cmd->cmd_handler = &create_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 3;
		yyval.cmd->cmd_argv[0] = yypvt[-4].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-3].strval;
		yyval.cmd->cmd_argv[2] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[3] = NULL;
	} break;
case 36:
# line 288 "nwamcfg_grammar.y"
{
		command_usage(CMD_DESTROY);
		YYERROR;
	} break;
case 37:
# line 293 "nwamcfg_grammar.y"
{
		/* destroy -a */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_DESTROY;
		yyval.cmd->cmd_handler = &destroy_func;
		yyval.cmd->cmd_res1_type = -1; /* special value */
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 38:
# line 306 "nwamcfg_grammar.y"
{
		command_usage(CMD_DESTROY);
		YYERROR;
	} break;
case 39:
# line 311 "nwamcfg_grammar.y"
{
		command_usage(CMD_DESTROY);
		YYERROR;
	} break;
case 40:
# line 316 "nwamcfg_grammar.y"
{
		/* destroy enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_DESTROY;
		yyval.cmd->cmd_handler = &destroy_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 41:
# line 329 "nwamcfg_grammar.y"
{
		/* destroy ncu test (class inferred) */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_DESTROY;
		yyval.cmd->cmd_handler = &destroy_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-1].ival;
		yyval.cmd->cmd_ncu_class_type = NCU_CLASS_ANY;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 42:
# line 344 "nwamcfg_grammar.y"
{
		/* destroy ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_DESTROY;
		yyval.cmd->cmd_handler = &destroy_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 43:
# line 360 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_END;
		yyval.cmd->cmd_handler = &end_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 44:
# line 371 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXIT;
		yyval.cmd->cmd_handler = &exit_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 45:
# line 382 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 46:
# line 392 "nwamcfg_grammar.y"
{
		command_usage(CMD_EXPORT);
		YYERROR;
	} break;
case 47:
# line 397 "nwamcfg_grammar.y"
{
		/* export -d */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 48:
# line 409 "nwamcfg_grammar.y"
{
		/* export -f file */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_argc = 2;
		yyval.cmd->cmd_argv[0] = yypvt[-1].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[2] = NULL;
	} break;
case 49:
# line 422 "nwamcfg_grammar.y"
{
		/* export -d -f file */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_argc = 3;
		yyval.cmd->cmd_argv[0] = yypvt[-2].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-1].strval;
		yyval.cmd->cmd_argv[2] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[3] = NULL;
	} break;
case 50:
# line 436 "nwamcfg_grammar.y"
{
		/* export enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 51:
# line 449 "nwamcfg_grammar.y"
{
		/* export ncu test (all ncu's named test) */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-1].ival;
		yyval.cmd->cmd_ncu_class_type = NCU_CLASS_ANY;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 52:
# line 464 "nwamcfg_grammar.y"
{
		/* export ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 53:
# line 479 "nwamcfg_grammar.y"
{
		/* export -f file enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 3;
		yyval.cmd->cmd_argv[0] = yypvt[-3].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-2].strval;
		yyval.cmd->cmd_argv[2] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[3] = NULL;
	} break;
case 54:
# line 494 "nwamcfg_grammar.y"
{
		/* export -f file ncu test (all ncu's named test) */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-1].ival;
		yyval.cmd->cmd_ncu_class_type = NCU_CLASS_ANY;
		yyval.cmd->cmd_argc = 3;
		yyval.cmd->cmd_argv[0] = yypvt[-3].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-2].strval;
		yyval.cmd->cmd_argv[2] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[3] = NULL;
	} break;
case 55:
# line 511 "nwamcfg_grammar.y"
{
		/* export -f file ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_EXPORT;
		yyval.cmd->cmd_handler = &export_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 3;
		yyval.cmd->cmd_argv[0] = yypvt[-4].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-3].strval;
		yyval.cmd->cmd_argv[2] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[3] = NULL;
	} break;
case 56:
# line 529 "nwamcfg_grammar.y"
{
		command_usage(CMD_GET);
		YYERROR;
	} break;
case 57:
# line 534 "nwamcfg_grammar.y"
{
		properr(yypvt[-0].strval);
		YYERROR;
	} break;
case 58:
# line 539 "nwamcfg_grammar.y"
{
		/* get prop */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_GET;
		yyval.cmd->cmd_handler = &get_func;
		yyval.cmd->cmd_prop_type = yypvt[-0].ival;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 59:
# line 551 "nwamcfg_grammar.y"
{
		/* get -V prop */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_GET;
		yyval.cmd->cmd_handler = &get_func;
		yyval.cmd->cmd_prop_type = yypvt[-0].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-1].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 60:
# line 565 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_HELP;
		yyval.cmd->cmd_handler = &help_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 61:
# line 575 "nwamcfg_grammar.y"
{
		/* help command */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_HELP;
		yyval.cmd->cmd_handler = &help_func;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 62:
# line 588 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 63:
# line 598 "nwamcfg_grammar.y"
{
		command_usage(CMD_LIST);
		YYERROR;
	} break;
case 64:
# line 603 "nwamcfg_grammar.y"
{
		/* list -a */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_res1_type = -1; /* special value */
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 65:
# line 616 "nwamcfg_grammar.y"
{
		command_usage(CMD_LIST);
		YYERROR;
	} break;
case 66:
# line 621 "nwamcfg_grammar.y"
{
		command_usage(CMD_LIST);
		YYERROR;
	} break;
case 67:
# line 626 "nwamcfg_grammar.y"
{
		/* list enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 68:
# line 639 "nwamcfg_grammar.y"
{
		/* list ncu test (all ncu's named test) */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-1].ival;
		yyval.cmd->cmd_ncu_class_type = NCU_CLASS_ANY;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 69:
# line 654 "nwamcfg_grammar.y"
{
		/* list ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 70:
# line 669 "nwamcfg_grammar.y"
{
		/* list -a enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 2;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-2].strval;
		yyval.cmd->cmd_argv[2] = NULL;
	} break;
case 71:
# line 683 "nwamcfg_grammar.y"
{
		/* list -a ncu test (all ncu's named test) */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-1].ival;
		yyval.cmd->cmd_ncu_class_type = NCU_CLASS_ANY;
		yyval.cmd->cmd_argc = 2;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-2].strval;
		yyval.cmd->cmd_argv[2] = NULL;
	} break;
case 72:
# line 699 "nwamcfg_grammar.y"
{
		/* list -a ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_LIST;
		yyval.cmd->cmd_handler = &list_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 2;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = yypvt[-3].strval;
		yyval.cmd->cmd_argv[2] = NULL;
	} break;
case 73:
# line 716 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_REVERT;
		yyval.cmd->cmd_handler = &revert_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 74:
# line 727 "nwamcfg_grammar.y"
{
		command_usage(CMD_SELECT);
		YYERROR;
	} break;
case 75:
# line 732 "nwamcfg_grammar.y"
{
		command_usage(CMD_SELECT);
		YYERROR;
	} break;
case 76:
# line 737 "nwamcfg_grammar.y"
{
		command_usage(CMD_SELECT);
		YYERROR;
	} break;
case 77:
# line 742 "nwamcfg_grammar.y"
{
		command_usage(CMD_SELECT);
		YYERROR;
	} break;
case 78:
# line 747 "nwamcfg_grammar.y"
{
		/* select enm/loc/ncp test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_SELECT;
		yyval.cmd->cmd_handler = &select_func;
		yyval.cmd->cmd_res1_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 79:
# line 760 "nwamcfg_grammar.y"
{
		/* select ncu test (class inferred) */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_SELECT;
		yyval.cmd->cmd_handler = &select_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-1].ival;
		yyval.cmd->cmd_ncu_class_type = NCU_CLASS_ANY;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 80:
# line 775 "nwamcfg_grammar.y"
{
		/* select ncu ip/phys test */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_SELECT;
		yyval.cmd->cmd_handler = &select_func;
		yyval.cmd->cmd_res1_type = RT1_NCP;
		yyval.cmd->cmd_res2_type = yypvt[-2].ival;
		yyval.cmd->cmd_ncu_class_type = yypvt[-1].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 81:
# line 791 "nwamcfg_grammar.y"
{
		command_usage(CMD_SET);
		YYERROR;
	} break;
case 82:
# line 796 "nwamcfg_grammar.y"
{
		properr(yypvt[-0].strval);
		YYERROR;
	} break;
case 83:
# line 801 "nwamcfg_grammar.y"
{
		/* set prop=value */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_SET;
		yyval.cmd->cmd_handler = &set_func;
		yyval.cmd->cmd_prop_type = yypvt[-2].ival;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 84:
# line 815 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_VERIFY;
		yyval.cmd->cmd_handler = &verify_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 85:
# line 826 "nwamcfg_grammar.y"
{
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_WALKPROP;
		yyval.cmd->cmd_handler = &walkprop_func;
		yyval.cmd->cmd_argc = 0;
		yyval.cmd->cmd_argv[0] = NULL;
	} break;
case 86:
# line 836 "nwamcfg_grammar.y"
{
		/* walkprop -a */
		if ((yyval.cmd = alloc_cmd()) == NULL)
			YYERROR;
		cmd = yyval.cmd;
		yyval.cmd->cmd_num = CMD_WALKPROP;
		yyval.cmd->cmd_handler = &walkprop_func;
		yyval.cmd->cmd_argc = 1;
		yyval.cmd->cmd_argv[0] = yypvt[-0].strval;
		yyval.cmd->cmd_argv[1] = NULL;
	} break;
case 87:
# line 848 "nwamcfg_grammar.y"
{ yyval.ival = RT1_LOC; } break;
case 88:
# line 849 "nwamcfg_grammar.y"
{ yyval.ival = RT1_NCP; } break;
case 89:
# line 850 "nwamcfg_grammar.y"
{ yyval.ival = RT1_ENM; } break;
case 90:
# line 851 "nwamcfg_grammar.y"
{ yyval.ival = RT1_WLAN; } break;
case 91:
# line 853 "nwamcfg_grammar.y"
{ yyval.ival = RT2_NCU; } break;
case 92:
# line 855 "nwamcfg_grammar.y"
{ yyval.ival = NCU_CLASS_PHYS; } break;
case 93:
# line 856 "nwamcfg_grammar.y"
{ yyval.ival = NCU_CLASS_IP; } break;
case 94:
# line 858 "nwamcfg_grammar.y"
{ yyval.ival = PT_UNKNOWN; } break;
case 95:
# line 859 "nwamcfg_grammar.y"
{ yyval.ival = PT_ACTIVATION_MODE; } break;
case 96:
# line 860 "nwamcfg_grammar.y"
{ yyval.ival = PT_CONDITIONS; } break;
case 97:
# line 861 "nwamcfg_grammar.y"
{ yyval.ival = PT_ENABLED; } break;
case 98:
# line 862 "nwamcfg_grammar.y"
{ yyval.ival = PT_TYPE; } break;
case 99:
# line 863 "nwamcfg_grammar.y"
{ yyval.ival = PT_CLASS; } break;
case 100:
# line 864 "nwamcfg_grammar.y"
{ yyval.ival = PT_PARENT; } break;
case 101:
# line 865 "nwamcfg_grammar.y"
{ yyval.ival = PT_PRIORITY_GROUP; } break;
case 102:
# line 866 "nwamcfg_grammar.y"
{ yyval.ival = PT_PRIORITY_MODE; } break;
case 103:
# line 867 "nwamcfg_grammar.y"
{ yyval.ival = PT_LINK_MACADDR; } break;
case 104:
# line 868 "nwamcfg_grammar.y"
{ yyval.ival = PT_LINK_AUTOPUSH; } break;
case 105:
# line 869 "nwamcfg_grammar.y"
{ yyval.ival = PT_LINK_MTU; } break;
case 106:
# line 870 "nwamcfg_grammar.y"
{ yyval.ival = PT_IP_VERSION; } break;
case 107:
# line 871 "nwamcfg_grammar.y"
{ yyval.ival = PT_IPV4_ADDRSRC; } break;
case 108:
# line 872 "nwamcfg_grammar.y"
{ yyval.ival = PT_IPV4_ADDR; } break;
case 109:
# line 873 "nwamcfg_grammar.y"
{ yyval.ival = PT_IPV4_DEFAULT_ROUTE; } break;
case 110:
# line 874 "nwamcfg_grammar.y"
{ yyval.ival = PT_IPV6_ADDRSRC; } break;
case 111:
# line 875 "nwamcfg_grammar.y"
{ yyval.ival = PT_IPV6_ADDR; } break;
case 112:
# line 876 "nwamcfg_grammar.y"
{ yyval.ival = PT_IPV6_DEFAULT_ROUTE; } break;
case 113:
# line 877 "nwamcfg_grammar.y"
{ yyval.ival = PT_ENM_FMRI; } break;
case 114:
# line 878 "nwamcfg_grammar.y"
{ yyval.ival = PT_ENM_START; } break;
case 115:
# line 879 "nwamcfg_grammar.y"
{ yyval.ival = PT_ENM_STOP; } break;
case 116:
# line 880 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_NAMESERVICES; } break;
case 117:
# line 881 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_NAMESERVICES_CONFIG; } break;
case 118:
# line 882 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_DNS_CONFIGSRC; } break;
case 119:
# line 883 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_DNS_DOMAIN; } break;
case 120:
# line 884 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_DNS_SERVERS; } break;
case 121:
# line 885 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_DNS_SEARCH; } break;
case 122:
# line 886 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_NIS_CONFIGSRC; } break;
case 123:
# line 887 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_NIS_SERVERS; } break;
case 124:
# line 888 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_LDAP_CONFIGSRC; } break;
case 125:
# line 889 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_LDAP_SERVERS; } break;
case 126:
# line 890 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_DEFAULT_DOMAIN; } break;
case 127:
# line 891 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_NFSV4_DOMAIN; } break;
case 128:
# line 892 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_IPF_CONFIG; } break;
case 129:
# line 893 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_IPF_V6_CONFIG; } break;
case 130:
# line 894 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_IPNAT_CONFIG; } break;
case 131:
# line 895 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_IPPOOL_CONFIG; } break;
case 132:
# line 896 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_IKE_CONFIG; } break;
case 133:
# line 897 "nwamcfg_grammar.y"
{ yyval.ival = PT_LOC_IPSECPOL_CONFIG; } break;
case 134:
# line 898 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_SSID; } break;
case 135:
# line 899 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_BSSID; } break;
case 136:
# line 900 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_KEYNAME; } break;
case 137:
# line 901 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_PRIORITY; } break;
case 138:
# line 902 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_DISABLED; } break;
case 139:
# line 903 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_EAP_USER; } break;
case 140:
# line 904 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_EAP_ANON; } break;
case 141:
# line 905 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_CA_CERT; } break;
case 142:
# line 906 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_PRIV; } break;
case 143:
# line 907 "nwamcfg_grammar.y"
{ yyval.ival = PT_WLAN_CLI_CERT; } break;
# line	556 "/usr/share/lib/ccs/yaccpar"
	}
	goto yystack;		/* reset registers in driver code */
}

