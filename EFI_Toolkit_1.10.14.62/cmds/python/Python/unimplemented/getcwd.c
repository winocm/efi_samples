/***********************************************************
Copyright 1991-1995 by Stichting Mathematisch Centrum, Amsterdam,
The Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI or Corporation for National Research Initiatives or
CNRI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

While CWI is the initial source for this software, a modified version
is made available by the Corporation for National Research Initiatives
(CNRI) at the Internet address ftp://ftp.python.org.

STICHTING MATHEMATISCH CENTRUM AND CNRI DISCLAIM ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH
CENTRUM OR CNRI BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/* Two PD getcwd() implementations.
   Author: Guido van Rossum, CWI Amsterdam, Jan 1991, <guido@cwi.nl>. */

#include <stdio.h>
#include <errno.h>

#ifdef HAVE_GETWD

/* Version for BSD systems -- use getwd() */

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

extern char *getwd();

char *
getcwd(buf, size)
	char *buf;
	int size;
{
	char localbuf[MAXPATHLEN+1];
	char *ret;
	
	if (size <= 0) {
		errno = EINVAL;
		return NULL;
	}
	ret = getwd(localbuf);
	if (ret != NULL && strlen(localbuf) >= size) {
		errno = ERANGE;
		return NULL;
	}
	if (ret == NULL) {
		errno = EACCES; /* Most likely error */
		return NULL;
	}
	strncpy(buf, localbuf, size);
	return buf;
}

#else /* !HAVE_GETWD */

/* Version for really old UNIX systems -- use pipe from pwd */

#ifndef PWD_CMD
#define PWD_CMD "/bin/pwd"
#endif

char *
getcwd(buf, size)
	char *buf;
	int size;
{
	FILE *fp;
	char *p;
	int sts;
	if (size <= 0) {
		errno = EINVAL;
		return NULL;
	}
	if ((fp = popen(PWD_CMD, "r")) == NULL)
		return NULL;
	if (fgets(buf, size, fp) == NULL || (sts = pclose(fp)) != 0) {
		errno = EACCES; /* Most likely error */
		return NULL;
	}
	for (p = buf; *p != '\n'; p++) {
		if (*p == '\0') {
			errno = ERANGE;
			return NULL;
		}
	}
	*p = '\0';
	return buf;
}

#endif /* !HAVE_GETWD */
