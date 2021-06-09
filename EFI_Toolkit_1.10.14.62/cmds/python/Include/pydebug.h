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

#ifndef Py_PYDEBUG_H
#define Py_PYDEBUG_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef EFI_LOADABLE_MODULE
extern DL_IMPORT(int) Py_DebugFlag;
extern DL_IMPORT(int) Py_VerboseFlag;
extern DL_IMPORT(int) Py_InteractiveFlag;
extern DL_IMPORT(int) Py_OptimizeFlag;
extern DL_IMPORT(int) Py_NoSiteFlag;
extern DL_IMPORT(int) Py_UseClassExceptionsFlag;
extern DL_IMPORT(int) Py_FrozenFlag;
extern DL_IMPORT(int) Py_TabcheckFlag;
#else
extern int *pPy_DebugFlag;
extern int *pPy_VerboseFlag;
extern int *pPy_InteractiveFlag;
extern int *pPy_OptimizeFlag;
extern int *pPy_NoSiteFlag;
extern int *pPy_UseClassExceptionsFlag;
extern int *pPy_FrozenFlag;
extern int *pPy_TabcheckFlag;
#define Py_DebugFlag (*pPy_DebugFlag)
#define Py_VerboseFlag (*pPy_VerboseFlag)
#define Py_InteractiveFlag (*pPy_InteractiveFlag)
#define Py_OptimixeFlag (*pPy_OptimizeFlag)
#define Py_NoSiteFlag (*pPy_NoSiteFlag)
#define Py_UseClassExceptionsFlag (*pPy_UseClassExceptionsFlag)
#define Py_FrozenFlag (*pPy_FrozenFlag)
#define Py_TabcheckFlag (*pPy_TabcheckFlag)
#endif

DL_IMPORT(void) Py_FatalError	Py_PROTO((char *));

#ifdef __cplusplus
}
#endif
#endif /* !Py_PYDEBUG_H */
