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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved	*/

#include "lint.h"
#include "thr_uberdata.h"
#include <ucontext.h>
#include <sys/types.h>
#include "libc.h"

#pragma weak _getcontext = getcontext

int
getcontext(ucontext_t *ucp)
{
	greg_t *reg;

	ucp->uc_flags = UC_ALL;
	if (__getcontext(ucp))
		return (-1);

	/*
	 * Note that %o1 and %g1 are modified by the system call
	 * routine. ABI calling conventions specify that the caller
	 * cannot depend upon %o0 through %o5 nor %g1, so no effort is
	 * made to maintain these registers. %o0 is forced to reflect
	 * an affirmative return code.
	 */
	reg = ucp->uc_mcontext.gregs;
	reg[REG_SP] = getfp();
	reg[REG_O7] = caller();
	reg[REG_PC] = reg[REG_O7] + 8;
	reg[REG_nPC] = reg[REG_PC] + 4;
	reg[REG_O0] = 0;

	return (0);
}
