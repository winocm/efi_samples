/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
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

//#define LIBC_DEBUG

#include <rune.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "setlocale.h"

#include <libc_debug.h>

extern int		_none_init __P((_RuneLocale *));
#ifdef XPG4
extern int              _xpg4_setrunelocale __P((char *));


#ifdef _ORG_FREEBSD_
extern int		_UTF2_init __P((_RuneLocale *));
extern int		_EUC_init __P((_RuneLocale *));
extern int		_BIG5_init __P((_RuneLocale *));
extern int		_MSKanji_init __P((_RuneLocale *));
#else 
extern int		_BIG5_init __P((_RuneLocale *));
extern int		_GB2312_init __P((_RuneLocale *));
extern int		_SHIFTJIS_init __P((_RuneLocale *));
extern int		_KSC_init __P((_RuneLocale *));
#endif

#endif
extern _RuneLocale      *_Read_RuneMagi __P((FILE *));

#ifdef XPG4
int
setrunelocale(encoding)
	char *encoding;
{
	return _xpg4_setrunelocale(encoding);
}
#endif

int
#ifndef XPG4
setrunelocale(encoding)
#else
_xpg4_setrunelocale(encoding)
#endif
	char *encoding;
{
	FILE *fp;
	char name[PATH_MAX];
	_RuneLocale *rl;

	if (!encoding || strlen(encoding) > ENCODING_LEN)
	    return(EFAULT);

	/*
	 * The "C" and "POSIX" locale are always here.
	 */
	if (!strcmp(encoding, "C") || !strcmp(encoding, "POSIX")) {
		_CurrentRuneLocale = &_DefaultRuneLocale;
		return(0);
	}

	if (_PathLocale == NULL) {
		char *p = getenv("PATH_LOCALE");

		if (p != NULL

#ifdef _ORG_FREEBSD_
#ifndef __NETBSD_SYSCALLS
			&& !issetugid()
#endif
#endif
			) {
			if (strlen(p) + 1/*"/"*/ + ENCODING_LEN +
			    1/*"/"*/ + CATEGORY_LEN >= PATH_MAX)
				return(EFAULT);
			_PathLocale = strdup(p);
			if (_PathLocale == NULL)
				return (errno);
		} else
			_PathLocale = _PATH_LOCALE;
	}
	/* Range checking not needed, encoding length already checked above */
	(void) strcpy(name, _PathLocale);
	(void) strcat(name, "/");
	(void) strcat(name, encoding);
	(void) strcat(name, "/LC_CTYPE");

	if ((fp = fopen(name, "r")) == NULL)
		return(ENOENT);

	if ((rl = _Read_RuneMagi(fp)) == 0) {
		fclose(fp);
		return(EFTYPE);
	}

	fclose(fp);

#ifdef XPG4

	if (!rl->encoding[0] || !strcmp(rl->encoding, "NONE"))
		return(_none_init(rl));

#else
	if (!rl->encoding[0])
		return(EINVAL);
#endif

	else if (!strcmp(rl->encoding, "NONE"))
		return(_none_init(rl));

#ifdef XPG4

#ifdef _ORG_FREEBSD_
	else if (!strcmp(rl->encoding, "EUC"))
		return(_EUC_init(rl));
	else if (!strcmp(rl->encoding, "BIG5"))
		return(_BIG5_init(rl));
	else if (!strcmp(rl->encoding, "MSKanji"))
		return(_MSKanji_init(rl));
#else 
#ifdef BIG5_LANG
	else if (!strcmp(rl->encoding, "BIG5"))
		return(_BIG5_init(rl));
#endif
#ifdef GB2312_LANG
	else if (!strcmp(rl->encoding, "GB2312"))
		return(_GB2312_init(rl));
#endif
#ifdef SHIFT_JIS_LANG
	else if (!strcmp(rl->encoding, "SHIFT-JIS"))
		return(_SHIFTJIS_init(rl));
#endif
#ifdef KSC_LANG
	else if (!strcmp(rl->encoding, "KSC"))
		return(_KSC_init(rl));
#endif
#endif /* _ORG_FREEBSD_ */


#endif /* XPG4 */
	else
		return(EINVAL);
}

