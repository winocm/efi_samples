/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)sys_generic.c	8.5 (Berkeley) 1/21/94
 * $Id: select.c,v 1.1.1.1 2003/11/19 01:50:30 kyu3 Exp $
 */

#include <atk_libc.h>
#include "./filedesc.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/param.h>
#ifndef KERNEL
#define KERNEL
#include <errno.h>
#undef KERNEL
#else
#include <errno.h>
#endif

#ifdef	EFI_NT_EMULATOR 
#define	_SELECT_DELAY_	10000
#else
#define	_SELECT_DELAY_	1000
#endif

static int
selscan(ibits, obits, nfd, nselected)
	fd_mask **ibits, **obits;
	int nfd, *nselected;
{
	int		msk, i, j, fd, n;
	fd_mask		bits;
	EFILIBC_POLL_T	Poll;
	void		*DevSpecific;
	EFI_STATUS	Status;
	/* Note: backend also returns POLLHUP/POLLERR if appropriate. */
	static int	flag[3] = { POLLRDNORM, POLLWRNORM, POLLRDBAND };

	for (msk = 0, n = 0; msk < 3; msk++) {
		if (ibits[msk] == NULL)
			continue;
		for (i = 0; i < nfd; i += NFDBITS) {
			bits = ibits[ msk ][ i / NFDBITS ];
			while ((j = ffs(bits)) && (fd = i + --j) < nfd) {
				bits &= ~(1 << j);

				if ( _GetOpenFilePoll( fd, &Poll ) != 0 )
					return (EBADF);
				if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0) {
					return (EBADF);
				}

				Status = Poll( flag[msk], DevSpecific );
				switch ( Status ) {
				case EFI_SUCCESS:
					obits[msk][(fd)/NFDBITS] |=
						(1 << ((fd) % NFDBITS));
					n++;
					break;
				case EFI_NOT_READY:
					break;
				default:
					return _EfiErrToErrno( Status );
				}
			}
		}
	}
	*nselected = n;
	return (0);
}

int
select(nd, in, ou, ex, tv)
	int	nd;
	fd_set	*in, *ou, *ex;
	struct	timeval *tv;
{
	fd_mask *ibits[3], *obits[3], *selbits, *sbp;
	int	error, forever, nselected;
	u_int	nbufbytes, ncpbytes, nfdbits;
	int64_t	timo;

	if (nd < 0)
		return (EINVAL);

	/*
	 * Allocate just enough bits for the non-null fd_sets.  Use the
	 * preallocated auto buffer if possible.
	 */
	nfdbits = roundup(nd, NFDBITS);
	ncpbytes = nfdbits / NBBY;
	nbufbytes = 0;
	if (in != NULL)
		nbufbytes += 2 * ncpbytes;
	if (ou != NULL)
		nbufbytes += 2 * ncpbytes;
	if (ex != NULL)
		nbufbytes += 2 * ncpbytes;
	selbits = malloc(nbufbytes);

	/*
	 * Assign pointers into the bit buffers and fetch the input bits.
	 * Put the output buffers together so that they can be bzeroed
	 * together.
	 */
	sbp = selbits;
#define	getbits(name, x) \
	do {								\
		if (name == NULL)					\
			ibits[x] = NULL;				\
		else {							\
			ibits[x] = sbp + nbufbytes / 2 / sizeof *sbp;	\
			obits[x] = sbp;					\
			sbp += ncpbytes / sizeof *sbp;			\
			bcopy(name, ibits[x], ncpbytes);		\
		}							\
	} while (0)
	getbits(in, 0);
	getbits(ou, 1);
	getbits(ex, 2);
#undef	getbits
	if (nbufbytes != 0)
		bzero(selbits, nbufbytes / 2);

	if (tv) {
		timo = tv->tv_usec + (tv->tv_sec * 1000000);
		forever = 0;
	} else {
		timo = 0;
		forever = 1;
	}

	/*
	 *  Poll for I/O events
	 */
	nselected = 0;
	do {
		/*
		 *  Scan for pending I/O
		 */
		error = selscan(ibits, obits, nd, &nselected);
		if (error || nselected)
			break;

		/*
		 *  Adjust timeout is needed
		 */
		if (timo)  {
			/*
			 *  Give it a rest
			 */
			usleep( _SELECT_DELAY_ );
			timo -= _SELECT_DELAY_;
		}
		
	} while (timo > 0 || forever);

	/* select is not restarted after signals... */
	if (error == ERESTART)
		error = EINTR;
	else if (error == EWOULDBLOCK)
		error = 0;

#define	putbits(name, x)	if (name) bcopy(obits[x], name, ncpbytes)
	if (error == 0) {
		putbits(in, 0);
		putbits(ou, 1);
		putbits(ex, 2);
#undef putbits
	} else {
		errno = error;
		nselected = -1;
	}

	free( selbits );
	return ( nselected );
}
