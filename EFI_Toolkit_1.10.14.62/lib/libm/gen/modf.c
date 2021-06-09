/*	$NetBSD: modf.c,v 1.1 1995/02/10 17:50:25 cgd Exp $	*/

/*
 * Copyright (c) 1994, 1995 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 * Portions copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <sys/types.h>
#include <machine/ieee.h>
#include <errno.h>
#include <math.h>
#include <atk_libc.h>

/*
 * double modf(double val, double *iptr)
 * returns: f and i such that |f| < 1.0, (f + i) = val, and
 *	sign(f) == sign(i) == sign(val).
 *
 * Beware signedness when doing subtraction, and also operand size!
 */
double
modf(val, iptr)
	double val, *iptr;
{
	union doub {
		double v;
		struct ieee_double s;
	} u, v;
	u_int64_t frac;

	/*
	 * If input is Inf or NaN, return it and leave i alone.
	 */
	u.v = val;
	if (u.s.dbl_exp == DBL_EXP_INFNAN)
		return (u.v);

	/*
	 * If input can't have a fractional part, return
	 * (appropriately signed) zero, and make i be the input.
	 */
	if ((int)u.s.dbl_exp - DBL_EXP_BIAS > DBL_FRACBITS - 1) {
		*iptr = u.v;
		v.v = 0.0;
		v.s.dbl_sign = u.s.dbl_sign;
		return (v.v);
	}

	/*
	 * If |input| < 1.0, return it, and set i to the appropriately
	 * signed zero.
	 */
	if (u.s.dbl_exp < DBL_EXP_BIAS) {
		v.v = 0.0;
		v.s.dbl_sign = u.s.dbl_sign;
		*iptr = v.v;
		return (u.v);
	}

	/*
	 * There can be a fractional part of the input.
	 * If you look at the math involved for a few seconds, it's
	 * plain to see that the integral part is the input, with the
	 * low (DBL_FRACBITS - (exponent - DBL_EXP_BIAS)) bits zeroed,
	 * the the fractional part is the part with the rest of the
	 * bits zeroed.  Just zeroing the high bits to get the
	 * fractional part would yield a fraction in need of
	 * normalization.  Therefore, we take the easy way out, and
	 * just use subtraction to get the fractional part.
	 */
	v.v = u.v;
	/* Zero the low bits of the fraction, the sleazy way. */
	frac = LIBC_LShiftU64((u_int64_t)v.s.dbl_frach , 32) + v.s.dbl_fracl;
	frac = LIBC_RShiftU64(frac, DBL_FRACBITS - (u.s.dbl_exp - DBL_EXP_BIAS));
	frac = LIBC_LShiftU64( frac,DBL_FRACBITS - (u.s.dbl_exp - DBL_EXP_BIAS));
	v.s.dbl_fracl = (u_int)(frac & 0xffffffff);
	v.s.dbl_frach = (u_int)(LIBC_RShiftU64(frac, 32));
	*iptr = v.v;

	u.v -= v.v;
	u.s.dbl_sign = v.s.dbl_sign;
	return (u.v);
}
