/*-
 * Copyright (c) 1986, 1989, 1991, 1993
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
 *	@(#)proc.h	8.15 (Berkeley) 5/19/95
 * $Id: proc.h,v 1.1.1.1 2003/11/19 01:49:01 kyu3 Exp $
 */

#ifndef _SYS_PROC_H_
#define	_SYS_PROC_H_

#include <machine/proc.h>		/* Machine-dependent proc substruct. */
#include <sys/callout.h>		/* For struct callout_handle. */
#include <sys/rtprio.h>			/* For struct rtprio. */
#include <sys/select.h>			/* For struct selinfo. */
#include <sys/signal.h>
#include <sys/signalvar.h>
#ifndef KERNEL
#include <sys/time.h>			/* For structs itimerval, timeval. */
#endif
#include <sys/ucred.h>
#include <sys/queue.h>
#ifdef _ORG_FREEBSD_
#include <sys/filedesc.h>
#endif

/*
 * One structure allocated per session.
 */
struct	session {
	int	s_count;		/* Ref cnt; pgrps in session. */
	struct	proc *s_leader;		/* Session leader. */
	struct	vnode *s_ttyvp;		/* Vnode of controlling terminal. */
	struct	tty *s_ttyp;		/* Controlling terminal. */
	pid_t	s_sid;			/* Session ID */
	char	s_login[roundup(MAXLOGNAME, sizeof(long))];	/* Setlogin() name. */
};

/*
 * One structure allocated per process group.
 */
struct	pgrp {
	LIST_ENTRY(pgrp) pg_hash;	/* Hash chain. */
	LIST_HEAD(_na_, proc) pg_members;/* Pointer to pgrp members. */
	struct	session *pg_session;	/* Pointer to session. */
#ifdef _ORG_FREEBSD_
	struct  sigiolst pg_sigiolst;	/* List of sigio sources. */
#endif
	pid_t	pg_id;			/* Pgrp id. */
	int	pg_jobc;	/* # procs qualifying pgrp for job control */
};

struct	procsig {
#define ps_begincopy ps_sigignore
	sigset_t ps_sigignore;	/* Signals being ignored. */
	sigset_t ps_sigcatch;	/* Signals being caught by user. */
	int      ps_flag;
	struct	 sigacts *ps_sigacts;
#define ps_endcopy ps_refcnt
	int	 ps_refcnt;
};

/*
 * pasleep structure, used by asleep() syscall to hold requested priority
 * and timeout values for await().
 */
struct  pasleep {
	int	as_priority;	/* Async priority. */
	int	as_timo;	/* Async timeout. */
};

/*
 * Description of a process.
 *
 * This structure contains the information needed to manage a thread of
 * control, known in UN*X as a process; it has references to substructures
 * containing descriptions of things that the process uses, but may share
 * with related processes.  The process structure and the substructures
 * are always addressable except for those marked "(PROC ONLY)" below,
 * which might be addressable only on a processor on which the process
 * is running.
 */
struct	proc {
	TAILQ_ENTRY(proc) p_procq;	/* run/sleep queue. */
	LIST_ENTRY(proc) p_list;	/* List of all processes. */

	/* substructures: */
	struct	pcred *p_cred;		/* Process owner's identity. */
	struct	filedesc *p_fd;		/* Ptr to open files structure. */
	struct	pstats *p_stats;	/* Accounting/statistics (PROC ONLY). */
	struct	plimit *p_limit;	/* Process limits. */
	struct	vm_object *p_upages_obj;/* Upages object */
	struct	procsig *p_procsig;
#define p_sigacts	p_procsig->ps_sigacts
#define p_sigignore	p_procsig->ps_sigignore
#define p_sigcatch	p_procsig->ps_sigcatch

#define	p_ucred		p_cred->pc_ucred
#define	p_rlimit	p_limit->pl_rlimit

	int	p_flag;			/* P_* flags. */
	char	p_stat;			/* S* process status. */
	char	p_pad1[3];

	pid_t	p_pid;			/* Process identifier. */
	LIST_ENTRY(proc) p_hash;	/* Hash chain. */
	LIST_ENTRY(proc) p_pglist;	/* List of processes in pgrp. */
	struct	proc *p_pptr;	 	/* Pointer to parent process. */
	LIST_ENTRY(proc) p_sibling;	/* List of sibling processes. */
	LIST_HEAD(_na_, proc) p_children;/* Pointer to list of children. */

	struct callout_handle p_ithandle; /*
					      * Callout handle for scheduling
					      * p_realtimer.
					      */
/* The following fields are all zeroed upon creation in fork. */
#define	p_startzero	p_oppid

	pid_t	p_oppid;	 /* Save parent pid during ptrace. XXX */
	int	p_dupfd;	 /* Sideways return value from fdopen. XXX */

	struct	vmspace *p_vmspace;	/* Address space. */

	/* scheduling */
	u_int	p_estcpu;	 /* Time averaged value of p_cpticks. */
	int	p_cpticks;	 /* Ticks of cpu time. */
	fixpt_t	p_pctcpu;	 /* %cpu for this process during p_swtime */
	void	*p_wchan;	 /* Sleep address. */
	const char *p_wmesg;	 /* Reason for sleep. */
	u_int	p_swtime;	 /* Time swapped in or out. */
	u_int	p_slptime;	 /* Time since last blocked. */

	struct	itimerval p_realtimer;	/* Alarm timer. */
	u_int64_t	p_runtime;	/* Real time in microsec. */
	struct	timeval p_switchtime;	/* When last scheduled */
	u_quad_t p_uticks;		/* Statclock hits in user mode. */
	u_quad_t p_sticks;		/* Statclock hits in system mode. */
	u_quad_t p_iticks;		/* Statclock hits processing intr. */

	int	p_traceflag;		/* Kernel trace points. */
	struct	vnode *p_tracep;	/* Trace to vnode. */

	int	p_siglist;		/* Signals arrived but not delivered. */

	struct	vnode *p_textvp;	/* Vnode of executable. */

	char	p_lock;			/* Process lock (prevent swap) count. */
	char	p_oncpu;		/* Which cpu we are on */
	char	p_lastcpu;		/* Last cpu we were on */
	char	p_pad2;			/* alignment */

	short	p_locks;		/* DEBUG: lockmgr count of held locks */
	short	p_simple_locks;		/* DEBUG: count of held simple locks */
	unsigned int	p_stops;	/* procfs event bitmask */
	unsigned int	p_stype;	/* procfs stop event type */
	char	p_step;			/* procfs stop *once* flag */
	unsigned char	p_pfsflags;	/* procfs flags */
	char	p_pad3[2];		/* padding for alignment */
	register_t p_retval[2];		/* syscall aux returns */
#ifdef _ORG_FREEBSD_
	struct	sigiolst p_sigiolst;	/* list of sigio sources */
#endif
	int	p_sigparent;		/* signal to parent on exit */
	sigset_t p_oldsigmask;		/* saved mask from before sigpause */
	int	p_sig;			/* for core dump/debugger XXX */
        u_long	p_code;	  	        /* for core dump/debugger XXX */

/* End area that is zeroed on creation. */
#define	p_endzero	p_startcopy

/* The following fields are all copied upon creation in fork. */
#define	p_startcopy	p_sigmask

	sigset_t p_sigmask;	/* Current signal mask. */
	u_char	p_priority;	/* Process priority. */
	u_char	p_usrpri;	/* User-priority based on p_cpu and p_nice. */
	char	p_nice;		/* Process "nice" value. */
	char	p_comm[MAXCOMLEN+1];

	struct 	pgrp *p_pgrp;	/* Pointer to process group. */

	struct 	sysentvec *p_sysent; /* System call dispatch information. */

	struct	rtprio p_rtprio;	/* Realtime priority. */
/* End area that is copied on creation. */
#define	p_endcopy	p_addr
	struct	user *p_addr;	/* Kernel virtual addr of u-area (PROC ONLY). */
	struct	mdproc p_md;	/* Any machine-dependent fields. */

	u_short	p_xstat;	/* Exit status for wait; also stop signal. */
	u_short	p_acflag;	/* Accounting flags. */
	struct	rusage *p_ru;	/* Exit information. XXX */

	int	p_nthreads;	/* number of threads (only in leader) */
	void	*p_aioinfo;	/* ASYNC I/O info */
	int	p_wakeup;	/* thread id */
	struct proc *p_peers;	
	struct proc *p_leader;
	struct	pasleep p_asleep;	/* Used by asleep()/await(). */
};

#define	p_session	p_pgrp->pg_session
#define	p_pgid		p_pgrp->pg_id

/* Status values. */
#define	SIDL	1		/* Process being created by fork. */
#define	SRUN	2		/* Currently runnable. */
#define	SSLEEP	3		/* Sleeping on an address. */
#define	SSTOP	4		/* Process debugging or suspension. */
#define	SZOMB	5		/* Awaiting collection by parent. */

/* These flags are kept in p_flags. */
#define	P_ADVLOCK	0x00001	/* Process may hold a POSIX advisory lock. */
#define	P_CONTROLT	0x00002	/* Has a controlling terminal. */
#define	P_INMEM		0x00004	/* Loaded into memory. */
#define	P_NOCLDSTOP	0x00008	/* No SIGCHLD when children stop. */
#define	P_PPWAIT	0x00010	/* Parent is waiting for child to exec/exit. */
#define	P_PROFIL	0x00020	/* Has started profiling. */
#define	P_SELECT	0x00040	/* Selecting; wakeup/waiting danger. */
#define	P_SINTR		0x00080	/* Sleep is interruptible. */
#define	P_SUGID		0x00100	/* Had set id privileges since last exec. */
#define	P_SYSTEM	0x00200	/* System proc: no sigs, stats or swapping. */
#define	P_TIMEOUT	0x00400	/* Timing out during sleep. */
#define	P_TRACED	0x00800	/* Debugged process being traced. */
#define	P_WAITED	0x01000	/* Debugging process has waited for child. */
#define	P_WEXIT		0x02000	/* Working on exiting. */
#define	P_EXEC		0x04000	/* Process called exec. */

/* Should probably be changed into a hold count. */
#define	P_NOSWAP	0x08000	/* Another flag to prevent swap out. */
#define	P_PHYSIO	0x10000	/* Doing physical I/O. */

/* Should be moved to machine-dependent areas. */
#define	P_OWEUPC	0x20000	/* Owe process an addupc() call at next ast. */

#define	P_SWAPPING	0x40000	/* Process is being swapped. */
#define	P_SWAPINREQ	0x80000	/* Swapin request due to wakeup */

/* Marked a kernel thread */
#define P_KTHREADP	0x200000 /* Process is really a kernel thread */

#define	P_NOCLDWAIT	0x400000 /* No zombies if child dies */


/*
 * MOVE TO ucred.h?
 *
 * Shareable process credentials (always resident).  This includes a reference
 * to the current user credentials as well as real and saved ids that may be
 * used to change ids.
 */
struct	pcred {
	struct	ucred *pc_ucred;	/* Current credentials. */
	uid_t	p_ruid;			/* Real user id. */
	uid_t	p_svuid;		/* Saved effective user id. */
	gid_t	p_rgid;			/* Real group id. */
	gid_t	p_svgid;		/* Saved effective group id. */
	int	p_refcnt;		/* Number of references. */
};

#ifdef KERNEL

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_SESSION);
MALLOC_DECLARE(M_SUBPROC);
#endif

/*
 * We use process IDs <= PID_MAX; PID_MAX + 1 must also fit in a pid_t,
 * as it is used to represent "no process group".
 */
#define	PID_MAX		99999
#define	NO_PID		100000

#define SESS_LEADER(p)	((p)->p_session->s_leader == (p))
#define	SESSHOLD(s)	((s)->s_count++)
#define	SESSRELE(s) {							\
	if (--(s)->s_count == 0)					\
		FREE(s, M_SESSION);					\
}

extern void stopevent(struct proc*, unsigned int, unsigned int);
#define	STOPEVENT(p,e,v)	do { \
	if ((p)->p_stops & (e)) stopevent(p,e,v); } while (0)

/* hold process U-area in memory, normally for ptrace/procfs work */
#define PHOLD(p) {							\
	if ((p)->p_lock++ == 0 && ((p)->p_flag & P_INMEM) == 0)	\
		faultin(p);						\
}
#define PRELE(p)	(--(p)->p_lock)

#define	PIDHASH(pid)	(&pidhashtbl[(pid) & pidhash])
extern LIST_HEAD(pidhashhead, proc) *pidhashtbl;
extern u_long pidhash;

#define	PGRPHASH(pgid)	(&pgrphashtbl[(pgid) & pgrphash])
extern LIST_HEAD(pgrphashhead, pgrp) *pgrphashtbl;
extern u_long pgrphash;

extern struct proc *curproc;		/* Current running proc. */
extern struct proc proc0;		/* Process slot for swapper. */
extern int hogticks;			/* Limit on kernel cpu hogs. */
extern int nprocs, maxproc;		/* Current and max number of procs. */
extern int maxprocperuid;		/* Max procs per uid. */
extern int switchticks;			/* `ticks' at last context switch. */
extern struct timeval switchtime;	/* Uptime at last context switch */

LIST_HEAD(proclist, proc);
extern struct proclist allproc;		/* List of all processes. */
extern struct proclist zombproc;	/* List of zombie processes. */
extern struct proc *initproc, *pageproc; /* Process slots for init, pager. */

#define	NQS	32			/* 32 run queues. */
extern struct prochd qs[];
extern struct prochd rtqs[];
extern struct prochd idqs[];
extern int	whichqs;	/* Bit mask summary of non-empty Q's. */
extern int	whichrtqs;	/* Bit mask summary of non-empty Q's. */
extern int	whichidqs;	/* Bit mask summary of non-empty Q's. */
struct	prochd {
	struct	proc *ph_link;		/* Linked list of running processes. */
	struct	proc *ph_rlink;
};

struct proc *pfind __P((pid_t));	/* Find process by id. */
struct pgrp *pgfind __P((pid_t));	/* Find process group by id. */

struct vm_zone;
extern struct vm_zone *proc_zone;

int	chgproccnt __P((uid_t uid, int diff));
int	enterpgrp __P((struct proc *p, pid_t pgid, int mksess));
void	fixjobc __P((struct proc *p, struct pgrp *pgrp, int entering));
int	inferior __P((struct proc *p));
int	leavepgrp __P((struct proc *p));
void	mi_switch __P((void));
void	procinit __P((void));
void	resetpriority __P((struct proc *));
int	roundrobin_interval __P((void));
void	setrunnable __P((struct proc *));
void	setrunqueue __P((struct proc *));
void	sleepinit __P((void));
void	remrq __P((struct proc *));
void	cpu_switch __P((struct proc *));
void	unsleep __P((struct proc *));
void	wakeup_one __P((void *chan));

void	cpu_exit __P((struct proc *)) __dead2;
void	exit1 __P((struct proc *, int)) __dead2;
void	cpu_fork __P((struct proc *, struct proc *));
int	fork1 __P((struct proc *, int));
int	trace_req __P((struct proc *));
void	cpu_wait __P((struct proc *));
int	cpu_coredump __P((struct proc *, struct vnode *, struct ucred *));
void		setsugid __P((struct proc *p));
#endif	/* KERNEL */

#endif	/* !_SYS_PROC_H_ */
