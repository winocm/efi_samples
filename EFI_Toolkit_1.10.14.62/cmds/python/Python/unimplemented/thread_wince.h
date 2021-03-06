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

/* This code implemented by Mark Hammond (MHammond@skippinet.com.au) */

#include <windows.h>
#include <limits.h>
#include <pydebug.h>

long PyThread_get_thread_ident(void);

/*
 * Change all headers to pure ANSI as no one will use K&R style on an
 * NT
 */

/*
 * Initialization of the C package, should not be needed.
 */
static void PyThread__init_thread(void)
{
}

/*
 * Thread support.
 */
int PyThread_start_new_thread(void (*func)(void *), void *arg)
{
	long rv;
	int success = 0;

	dprintf(("%ld: PyThread_start_new_thread called\n", PyThread_get_thread_ident()));
	if (!initialized)
		PyThread_init_thread();

	rv = _beginthread(func, 0, arg); /* use default stack size */
 
	if (rv != -1) {
		success = 1;
		dprintf(("%ld: PyThread_start_new_thread succeeded:\n", PyThread_get_thread_ident()));
	}

	return success;
}

/*
 * Return the thread Id instead of an handle. The Id is said to uniquely identify the
 * thread in the system
 */
long PyThread_get_thread_ident(void)
{
	if (!initialized)
		PyThread_init_thread();
        
	return GetCurrentThreadId();
}

static void do_PyThread_exit_thread(int no_cleanup)
{
	dprintf(("%ld: do_PyThread_exit_thread called\n", PyThread_get_thread_ident()));
	if (!initialized)
		if (no_cleanup)
			exit(0); /* XXX - was _exit()!! */
		else
			exit(0);
	_endthread();
}

void PyThread_exit_thread(void)
{
	do_PyThread_exit_thread(0);
}

void PyThread__exit_thread(void)
{
	do_PyThread_exit_thread(1);
}

#ifndef NO_EXIT_PROG
static void do_PyThread_exit_prog(int status, int no_cleanup)
{
	dprintf(("PyThread_exit_prog(%d) called\n", status));
	if (!initialized)
		if (no_cleanup)
			_exit(status);
		else
			exit(status);
}

void PyThread_exit_prog(int status)
{
	do_PyThread_exit_prog(status, 0);
}

void PyThread__exit_prog _P1(int status)
{
	do_PyThread_exit_prog(status, 1);
}
#endif /* NO_EXIT_PROG */

/*
 * Lock support. It has to be implemented using Mutexes, as
 * CE doesnt support semaphores.  Therefore we use some hacks to
 * simulate the non reentrant requirements of Python locks
 */
PyThread_type_lock PyThread_allocate_lock(void)
{
    HANDLE aLock;

    dprintf(("PyThread_allocate_lock called\n"));
    if (!initialized)
        PyThread_init_thread();

    aLock = CreateEvent(NULL,           /* Security attributes      */
                        0,              /* Manual-Reset               */
						1,              /* Is initially signalled  */
                        NULL);          /* Name of event            */

    dprintf(("%ld: PyThread_allocate_lock() -> %lx\n", PyThread_get_thread_ident(), (long)aLock));

    return (PyThread_type_lock) aLock;
}

void PyThread_free_lock(PyThread_type_lock aLock)
{
    dprintf(("%ld: PyThread_free_lock(%lx) called\n", PyThread_get_thread_ident(),(long)aLock));

    CloseHandle(aLock);
}

/*
 * Return 1 on success if the lock was acquired
 *
 * and 0 if the lock was not acquired. This means a 0 is returned
 * if the lock has already been acquired by this thread!
 */
int PyThread_acquire_lock(PyThread_type_lock aLock, int waitflag)
{
    int success = 1;
    DWORD waitResult;

    dprintf(("%ld: PyThread_acquire_lock(%lx, %d) called\n", PyThread_get_thread_ident(),(long)aLock, waitflag));

#ifndef DEBUG
    waitResult = WaitForSingleObject(aLock, (waitflag == 1 ? INFINITE : 0));
#else
	/* To aid in debugging, we regularly wake up.  This allows us to
	break into the debugger */
	while (TRUE) {
		waitResult = WaitForSingleObject(aLock, waitflag ? 3000 : 0);
		if (waitflag==0 || (waitflag==1 && waitResult == WAIT_OBJECT_0))
			break;
	}
#endif

    if (waitResult != WAIT_OBJECT_0) {
		success = 0;    /* We failed */
    }

	dprintf(("%ld: PyThread_acquire_lock(%lx, %d) -> %d\n", PyThread_get_thread_ident(),(long)aLock, waitflag, success));

	return success;
}

void PyThread_release_lock(PyThread_type_lock aLock)
{
    dprintf(("%ld: PyThread_release_lock(%lx) called\n", PyThread_get_thread_ident(),(long)aLock));

    if (!SetEvent(aLock))
        dprintf(("%ld: Could not PyThread_release_lock(%lx) error: %l\n", PyThread_get_thread_ident(), (long)aLock, GetLastError()));
}


