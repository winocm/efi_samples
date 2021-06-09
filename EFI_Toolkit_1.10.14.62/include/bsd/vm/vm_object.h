/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * The Mach Operating System project at Carnegie-Mellon University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)vm_object.h	8.3 (Berkeley) 1/12/94
 *
 *
 * Copyright (c) 1987, 1990 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Authors: Avadis Tevanian, Jr., Michael Wayne Young
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
 *
 * $Id: vm_object.h,v 1.1.1.1 2003/11/19 01:49:13 kyu3 Exp $
 */

/*
 *	Virtual memory object module definitions.
 */

#ifndef	_VM_OBJECT_
#define	_VM_OBJECT_

#include <sys/queue.h>
#include <machine/atomic.h>

enum obj_type { OBJT_DEFAULT, OBJT_SWAP, OBJT_VNODE, OBJT_DEVICE, OBJT_DEAD };
typedef enum obj_type objtype_t;

/*
 *	Types defined:
 *
 *	vm_object_t		Virtual memory object.
 */

struct vm_object {
	TAILQ_ENTRY(vm_object) object_list; /* list of all objects */
	TAILQ_HEAD(, vm_object) shadow_head; /* objects that this is a shadow for */
	TAILQ_ENTRY(vm_object) shadow_list; /* chain of shadow objects */
	TAILQ_HEAD(, vm_page) memq;	/* list of resident pages */
	int generation;			/* generation ID */
	objtype_t type;			/* type of pager */
	vm_size_t size;			/* Object size */
	int ref_count;			/* How many refs?? */
	int shadow_count;		/* how many objects that this is a shadow for */
	int pg_color;			/* color of first page in obj */
	int	id;					/* ID for no purpose, other than info */
	u_short flags;			/* see below */
	u_short paging_in_progress;	/* Paging (in or out) so don't collapse or destroy */
	u_short	behavior;		/* see below */
	int resident_page_count;	/* number of resident pages */
	int cache_count;			/* number of cached pages */
	int	wire_count;			/* number of wired pages */
	vm_ooffset_t paging_offset;	/* Offset into paging space */
	struct vm_object *backing_object; /* object that I'm a shadow of */
	vm_ooffset_t backing_object_offset;/* Offset in backing object */
	vm_offset_t last_read;		/* last read in object -- detect seq behavior */
	vm_page_t page_hint;		/* hint for last looked-up or allocated page */
	TAILQ_ENTRY(vm_object) pager_object_list; /* list of all objects of this pager type */
	void *handle;
	union {
		struct {
			off_t vnp_size; /* Current size of file */
		} vnp;
		struct {
			TAILQ_HEAD(, vm_page) devp_pglist; /* list of pages allocated */
		} devp;
		struct {
			int swp_nblocks;
			int swp_allocsize;
			struct swblock *swp_blocks;
			short swp_poip;
		} swp;
	} un_pager;
};

/*
 * Flags
 */
#define OBJ_ACTIVE	0x0004		/* active objects */
#define OBJ_DEAD	0x0008		/* dead objects (during rundown) */
#define	OBJ_NOSPLIT	0x0010		/* dont split this object */
#define OBJ_PIPWNT	0x0040		/* paging in progress wanted */
#define	OBJ_WRITEABLE	0x0080		/* object has been made writable */
#define OBJ_MIGHTBEDIRTY	0x0100	/* object might be dirty */
#define OBJ_CLEANING	0x0200
#define OBJ_OPT		0x1000		/* I/O optimization */
#define	OBJ_ONEMAPPING	0x2000		/* One USE (a single, non-forked) mapping flag */

#define OBJ_NORMAL	0x0		/* default behavior */
#define OBJ_SEQUENTIAL	0x1		/* expect sequential accesses */
#define OBJ_RANDOM	0x2		/* expect random accesses */

#define IDX_TO_OFF(idx) (((vm_ooffset_t)(idx)) << PAGE_SHIFT)
#define OFF_TO_IDX(off) ((vm_pindex_t)(((vm_ooffset_t)(off)) >> PAGE_SHIFT))

#ifdef	KERNEL

#define	OBJPC_SYNC	0x1			/* sync I/O */
#define	OBJPC_INVAL	0x2			/* invalidate */

TAILQ_HEAD(object_q, vm_object);

extern struct object_q vm_object_list;	/* list of allocated objects */

 /* lock for object list and count */

extern vm_object_t kernel_object;	/* the single kernel object */
extern vm_object_t kmem_object;

#endif				/* KERNEL */

#ifdef KERNEL

static __inline void
vm_object_set_flag(vm_object_t object, u_int bits)
{
	atomic_set_short(&object->flags, bits);
}

static __inline void
vm_object_clear_flag(vm_object_t object, u_int bits)
{
	atomic_clear_short(&object->flags, bits);
}

static __inline void
vm_object_pip_add(vm_object_t object, int i)
{
	atomic_add_short(&object->paging_in_progress, i);
}

static __inline void
vm_object_pip_subtract(vm_object_t object, int i)
{
	atomic_subtract_short(&object->paging_in_progress, i);
}

static __inline void
vm_object_pip_wakeup(vm_object_t object)
{
	atomic_subtract_short(&object->paging_in_progress, 1);
	if ((object->flags & OBJ_PIPWNT) && object->paging_in_progress == 0) {
		vm_object_clear_flag(object, OBJ_PIPWNT);
		wakeup(object);
	}
}

static __inline void
vm_object_pip_sleep(vm_object_t object, char *waitid)
{
	int s;

	if (object->paging_in_progress) {
		s = splvm();
		if (object->paging_in_progress) {
			vm_object_set_flag(object, OBJ_PIPWNT);
			tsleep(object, PVM, waitid, 0);
		}
		splx(s);
	}
}

static __inline void
vm_object_pip_wait(vm_object_t object, char *waitid)
{
	while (object->paging_in_progress)
		vm_object_pip_sleep(object, waitid);
}

vm_object_t vm_object_allocate __P((objtype_t, vm_size_t));
void _vm_object_allocate __P((objtype_t, vm_size_t, vm_object_t));
boolean_t vm_object_coalesce __P((vm_object_t, vm_pindex_t, vm_size_t, vm_size_t));
void vm_object_collapse __P((vm_object_t));
void vm_object_copy __P((vm_object_t, vm_pindex_t, vm_object_t *, vm_pindex_t *, boolean_t *));
void vm_object_deallocate __P((vm_object_t));
void vm_object_terminate __P((vm_object_t));
void vm_object_vndeallocate __P((vm_object_t));
void vm_object_init __P((void));
void vm_object_page_clean __P((vm_object_t, vm_pindex_t, vm_pindex_t, boolean_t));
void vm_object_page_remove __P((vm_object_t, vm_pindex_t, vm_pindex_t, boolean_t));
void vm_object_pmap_copy __P((vm_object_t, vm_pindex_t, vm_pindex_t));
void vm_object_pmap_copy_1 __P((vm_object_t, vm_pindex_t, vm_pindex_t));
void vm_object_pmap_remove __P((vm_object_t, vm_pindex_t, vm_pindex_t));
void vm_object_reference __P((vm_object_t));
void vm_object_shadow __P((vm_object_t *, vm_ooffset_t *, vm_size_t));
void vm_object_madvise __P((vm_object_t, vm_pindex_t, int, int));
void vm_object_init2 __P((void));
#endif				/* KERNEL */

#endif				/* _VM_OBJECT_ */
