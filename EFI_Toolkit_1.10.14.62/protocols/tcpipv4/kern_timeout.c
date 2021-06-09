/*-
 * Copyright (c) 1982, 1986, 1991, 1993
 *	  The Regents of the University of California.  All rights reserved.
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
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kern_clock.c	8.5 (Berkeley) 1/21/94
 * $Id: kern_timeout.c,v 1.1.1.1 2003/11/19 01:53:32 kyu3 Exp $
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/callout.h>
#include <sys/kernel.h>

/*
 * TODO:
 *	allocate more timeout table slots when table overflows.
 */

/* Exported to machdep.c and/or kern_clock.c.  */
struct callout *callout;
struct callout_list callfree;
int callwheelsize, callwheelbits, callwheelmask;
struct callout_tailq *callwheel;
int softticks;			/* Like ticks, but for softclock(). */

static struct callout *nextsoftcheck;	/* Next callout to be checked. */

/*
 * The callout mechanism is based on the work of Adam M. Costello and 
 * George Varghese, published in a technical report entitled "Redesigning
 * the BSD Callout and Timer Facilities" and modified slightly for inclusion
 * in FreeBSD by Justin T. Gibbs.  The original work on the data structures
 * used in this implementation was published by G.Varghese and A. Lauck in
 * the paper "Hashed and Hierarchical Timing Wheels: Data Structures for
 * the Efficient Implementation of a Timer Facility" in the Proceedings of
 * the 11th ACM Annual Symposium on Operating Systems Principles,
 * Austin, Texas Nov 1987.
 */

/*
 * Software (low priority) clock interrupt.
 * Run periodic events from timeout queue.
 */
void
softclock()
{
	register struct callout *c;
	register struct callout_tailq *bucket;
	register int s;
	register int curticks;
	register int steps;	/* #steps since we last allowed interrupts */

#ifndef MAX_SOFTCLOCK_STEPS
#define MAX_SOFTCLOCK_STEPS 100 /* Maximum allowed value of steps. */
#endif /* MAX_SOFTCLOCK_STEPS */

	steps = 0;
	s = splhigh();
	while (softticks != ticks) {
		softticks++;
		/*
		 * softticks may be modified by hard clock, so cache
		 * it while we work on a given bucket.
		 */
		curticks = softticks;
		bucket = &callwheel[curticks & callwheelmask];
		c = TAILQ_FIRST(bucket);
		while (c) {
			if (c->c_time != curticks) {
				c = TAILQ_NEXT(c, c_links.tqe);
				++steps;
				if (steps >= MAX_SOFTCLOCK_STEPS) {
					nextsoftcheck = c;
					/* Give interrupts a chance. */
					splx(s);
					s = splhigh();
					c = nextsoftcheck;
					steps = 0;
				}
			} else {
#ifdef _ORG_FREEBSD_
				void (*c_func)(void *);
#else
				void (*c_func)();
#endif
				void *c_arg;

				nextsoftcheck = TAILQ_NEXT(c, c_links.tqe);
				TAILQ_REMOVE(bucket, c, c_links.tqe);
				c_func = c->c_func;
				c_arg = c->c_arg;
				c->c_func = NULL;
				SLIST_INSERT_HEAD(&callfree, c, c_links.sle);
				splx(s);
				c_func(c_arg);
				s = splhigh();
				steps = 0;
				c = nextsoftcheck;
			}
		}
	}
	nextsoftcheck = NULL;
	splx(s);
}

/*
 * timeout --
 *	Execute a function after a specified length of time.
 *
 * untimeout --
 *	Cancel previous timeout function call.
 *
 * callout_handle_init --
 *	Initialize a handle so that using it with untimeout is benign.
 *
 *	See AT&T BCI Driver Reference Manual for specification.  This
 *	implementation differs from that one in that although an 
 *	identification value is returned from timeout, the original
 *	arguments to timeout as well as the identifier are used to
 *	identify entries for untimeout.
 */
#ifdef _ORG_FREEBSD_
struct callout_handle
#else
void
#endif
timeout(ftn, arg, to_ticks)
	timeout_t *ftn;
	void *arg;
	register int to_ticks;
{
	int s;
	struct callout *new;
#ifdef _ORG_FREEBSD_
	struct callout_handle handle;
#endif

	if (to_ticks <= 0)
		to_ticks = 1;

	/* Lock out the clock. */
	s = splhigh();

	/* Fill in the next free callout structure. */
	new = SLIST_FIRST(&callfree);
	if (new == NULL)
		/* XXX Attempt to malloc first */
		panic("timeout table full");

	SLIST_REMOVE_HEAD(&callfree, c_links.sle);
	new->c_arg = arg;
	new->c_func = ftn;
	new->c_time = ticks + to_ticks;
	TAILQ_INSERT_TAIL(&callwheel[new->c_time & callwheelmask],
			  new, c_links.tqe);

	splx(s);

#ifdef _ORG_FREEBSD_
	handle.callout = new;
	return (handle);
#endif
}

void
untimeout(ftn, arg, handle)
	timeout_t *ftn;
	void *arg;
	struct callout_handle handle;
{
	register int s;

	/*
	 * Check for a handle that was initialized
	 * by callout_handle_init, but never used
	 * for a real timeout.
	 */
	if (handle.callout == NULL)
		return;

	s = splhigh();
	if ((handle.callout->c_func == ftn)
	 && (handle.callout->c_arg == arg)) {
		if (nextsoftcheck == handle.callout) {
			nextsoftcheck = TAILQ_NEXT(handle.callout, c_links.tqe);
		}
		TAILQ_REMOVE(&callwheel[handle.callout->c_time & callwheelmask],
			     handle.callout, c_links.tqe);
		handle.callout->c_func = NULL;
		SLIST_INSERT_HEAD(&callfree, handle.callout, c_links.sle);
	}
	splx(s);
}

void
callout_handle_init(struct callout_handle *handle)
{
	handle->callout = NULL;
}

#ifdef APM_FIXUP_CALLTODO
/* 
 * Adjust the kernel calltodo timeout list.  This routine is used after 
 * an APM resume to recalculate the calltodo timer list values with the 
 * number of hz's we have been sleeping.  The next hardclock() will detect 
 * that there are fired timers and run softclock() to execute them.
 *
 * Please note, I have not done an exhaustive analysis of what code this
 * might break.  I am motivated to have my select()'s and alarm()'s that
 * have expired during suspend firing upon resume so that the applications
 * which set the timer can do the maintanence the timer was for as close
 * as possible to the originally intended time.  Testing this code for a 
 * week showed that resuming from a suspend resulted in 22 to 25 timers 
 * firing, which seemed independant on whether the suspend was 2 hours or
 * 2 days.  Your milage may vary.   - Ken Key <key@cs.utk.edu>
 */
void
adjust_timeout_calltodo(time_change)
    struct timeval *time_change;
{
	register struct callout *p;
	unsigned long delta_ticks;
	int s;

	/* 
	 * How many ticks were we asleep?
	 * (stolen from tvtohz()).
	 */

	/* Don't do anything */
	if (time_change->tv_sec < 0)
		return;
	else if (time_change->tv_sec <= LONG_MAX / 1000000)
		delta_ticks = (time_change->tv_sec * 1000000 +
			       time_change->tv_usec + (tick - 1)) / tick + 1;
	else if (time_change->tv_sec <= LONG_MAX / hz)
		delta_ticks = time_change->tv_sec * hz +
			      (time_change->tv_usec + (tick - 1)) / tick + 1;
	else
		delta_ticks = LONG_MAX;

	if (delta_ticks > INT_MAX)
		delta_ticks = INT_MAX;

	/* 
	 * Now rip through the timer calltodo list looking for timers
	 * to expire.
	 */

	/* don't collide with softclock() */
	s = splhigh(); 
	for (p = calltodo.c_next; p != NULL; p = p->c_next) {
		p->c_time -= delta_ticks;

		/* Break if the timer had more time on it than delta_ticks */
		if (p->c_time > 0)
			break;

		/* take back the ticks the timer didn't use (p->c_time <= 0) */
		delta_ticks = -p->c_time;
	}
	splx(s);

	return;
}
#endif /* APM_FIXUP_CALLTODO */
