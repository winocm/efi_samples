/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 * 
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
 *    This product includes software developed by the University of
 *    California, Berkeley, Intel Corporation, and its contributors.
 * 
 * 4. Neither the name of University, Intel Corporation, or their respective
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, INTEL CORPORATION AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS,
 * INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)fflush.c	8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
		"$Id: fflush.c,v 1.1.1.1 2003/11/19 01:50:07 kyu3 Exp $";
#endif /* LIBC_SCCS and not lint */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include "local.h"
#include "libc_private.h"

#include <libc_debug.h>

/* Flush a single file, or (if fp is NULL) all files.  */
int
fflush(fp)
	register FILE *fp;
{
	int retval;

    	if (fp == NULL) {
		return (_fwalk(__sflush));
    	}
	FLOCKFILE(fp);
	if ((fp->_flags & (__SWR | __SRW)) == 0) {
		errno = EBADF;
		retval = EOF;
	} else {
		retval = __sflush(fp);
	}
	FUNLOCKFILE(fp);
	return (retval);
}

int
__sflush(fp)
	register FILE *fp;
{
	register unsigned char *p;
	register size_t        n;
        register ssize_t       t;

    DPRINT((L"__sflush\n"));
	t = fp->_flags;
    if ((t & __SWR) == 0) {
        DPRINT((L"__sflush t & __SWR ret 0\n"));
		return (0);
    }

    if ((p = fp->_bf._base) == NULL) {
        DPRINT((L"__sflush (p = fp->_bf._base) == NULL ret 0\n"));
		return (0);
    }

	n = (size_t)(fp->_p - p); /* write this much */

	/*
	 * Set these immediately to avoid problems with longjmp and to allow
	 * exchange buffering (via setvbuf) in user write function.
	 */
	fp->_p = p;
        /* DHEXDMP(p,n,L"__sflush"); */
	fp->_w = t & (__SLBF|__SNBF) ? 0 : fp->_bf._size;

	for (; n > 0; n -= t, p += t) {
                DPRINT((L"__sflush calling write, len %d\n", n));
		t = (*fp->_write)(fp->_cookie, (char *)p, (int)n);
		if (t <= 0) {
			fp->_flags |= __SERR;
                        DPRINT((L"__sflush error from write!\n"));
			return (EOF);
		}
	}
	return (0);
}
