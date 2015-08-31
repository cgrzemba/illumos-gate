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
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved	*/

/*
 * University Copyright- Copyright (c) 1982, 1986, 1988
 * The Regents of the University of California
 * All Rights Reserved
 *
 * University Acknowledgment- Portions of this document are derived from
 * software developed by the University of California, Berkeley, and its
 * contributors.
 */
/* tnames.c: Made automatically from caps and maketerm.ed - don't edit me! */
char	*boolcodes[] =
	{
"bw", "am", "xb", "xs", "xn", "eo", "gn", "hc", "km", "hs",
"in", "da", "db", "mi", "ms", "os", "es", "xt", "hz", "ul",
"xo", "nx", "5i", "HC", "NR", "NP", "ND", "cc", "ut", "hl",
"YA", "YB", "YC", "YD", "YE", "YF", "YG",
0
	};

char	*numcodes[] =
	{
"co", "it", "li", "lm", "sg", "pb", "vt", "ws", "Nl", "lh",
"lw", "ma", "MW", "Co", "pa", "NC", "Ya", "Yb", "Yc", "Yd",
"Ye", "Yf", "Yg", "Yh", "Yi", "Yj", "Yk", "Yl", "Ym", "Yn",
"BT", "Yo", "Yp",
0
	};

char	*strcodes[] =
	{
"bt", "bl", "cr", "cs", "ct", "cl", "ce", "cd", "ch", "CC",
"cm", "do", "ho", "vi", "le", "CM", "ve", "nd", "ll", "up",
"vs", "dc", "dl", "ds", "hd", "as", "mb", "md", "ti", "dm",
"mh", "im", "mk", "mp", "mr", "so", "us", "ec", "ae", "me",
"te", "ed", "ei", "se", "ue", "vb", "ff", "fs", "i1", "is",
"i3", "if", "ic", "al", "ip", "kb", "ka", "kC", "kt", "kD",
"kL", "kd", "kM", "kE", "kS", "k0", "k1", "k;", "k2", "k3",
"k4", "k5", "k6", "k7", "k8", "k9", "kh", "kI", "kA", "kl",
"kH", "kN", "kP", "kr", "kF", "kR", "kT", "ku", "ke", "ks",
"l0", "l1", "la", "l2", "l3", "l4", "l5", "l6", "l7", "l8",
"l9", "mo", "mm", "nw", "pc", "DC", "DL", "DO", "IC", "SF",
"AL", "LE", "RI", "SR", "UP", "pk", "pl", "px", "ps", "pf",
"po", "rp", "r1", "r2", "r3", "rf", "rc", "cv", "sc", "sf",
"sr", "sa", "st", "wi", "ta", "ts", "uc", "hu", "iP", "K1",
"K3", "K2", "K4", "K5", "pO", "rP", "ac", "pn", "kB", "SX",
"RX", "SA", "RA", "XN", "XF", "eA", "LO", "LF", "@1", "@2",
"@3", "@4", "@5", "@6", "@7", "@8", "@9", "@0", "%1", "%2",
"%3", "%4", "%5", "%6", "%7", "%8", "%9", "%0", "&1", "&2",
"&3", "&4", "&5", "&6", "&7", "&8", "&9", "&0", "*1", "*2",
"*3", "*4", "*5", "*6", "*7", "*8", "*9", "*0", "#1", "#2",
"#3", "#4", "%a", "%b", "%c", "%d", "%e", "%f", "%g", "%h",
"%i", "%j", "!1", "!2", "!3", "RF", "F1", "F2", "F3", "F4",
"F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE",
"FF", "FG", "FH", "FI", "FJ", "FK", "FL", "FM", "FN", "FO",
"FP", "FQ", "FR", "FS", "FT", "FU", "FV", "FW", "FX", "FY",
"FZ", "Fa", "Fb", "Fc", "Fd", "Fe", "Ff", "Fg", "Fh", "Fi",
"Fj", "Fk", "Fl", "Fm", "Fn", "Fo", "Fp", "Fq", "Fr", "cb",
"MC", "ML", "MR", "Lf", "SC", "DK", "RC", "CW", "WG", "HU",
"DI", "QD", "TO", "PU", "fh", "PA", "WA", "u0", "u1", "u2",
"u3", "u4", "u5", "u6", "u7", "u8", "u9", "op", "oc", "Ic",
"Ip", "sp", "Sf", "Sb", "ZA", "ZB", "ZC", "ZD", "ZE", "ZF",
"ZG", "ZH", "ZI", "ZJ", "ZK", "ZL", "ZM", "ZN", "ZO", "ZP",
"ZQ", "ZR", "ZS", "ZT", "ZU", "ZV", "ZW", "ZX", "ZY", "ZZ",
"Za", "Zb", "Zc", "Zd", "Ze", "Zf", "Zg", "Zh", "Zi", "Zj",
"Zk", "Zl", "Zm", "Zn", "Zo", "Zp", "Zq", "Zr", "Zs", "Zt",
"Zu", "Zv", "Zw", "Zx", "Zy", "Km", "Mi", "RQ", "Gm", "AF",
"AB", "xl", "dv", "ci", "s0", "s1", "s2", "s3", "ML", "MT",
"Xy", "Zz", "Yv", "Yw", "Yx", "Yy", "Yz", "YZ", "S1", "S2",
"S3", "S4", "S5", "S6", "S7", "S8", "??", "??", "??", "??",
"??", "??", "??", "YI",
0
	};

