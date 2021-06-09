/*
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)if_loop.c	8.1 (Berkeley) 6/10/93
 * $Id: if_loop.c,v 1.1.1.1 2003/11/19 01:53:22 kyu3 Exp $
 */

/*
 * Loopback interface driver for protocol testing and timing.
 */
#ifndef _ORG_FREEBSD_
#define NLOOP	1
#else
#include "loop.h"
#endif
#if NLOOP > 0

#include "opt_atalk.h"
#include "opt_inet.h"
#include "opt_ipx.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/netisr.h>
#include <net/route.h>
#include <net/bpf.h>

#ifdef	INET
#include <netinet/in.h>
#include <netinet/in_var.h>
#endif

#ifdef IPX
#include <netipx/ipx.h>
#include <netipx/ipx_if.h>
#endif

#ifdef NS
#include <netns/ns.h>
#include <netns/ns_if.h>
#endif

#ifdef ISO
#include <netiso/iso.h>
#include <netiso/iso_var.h>
#endif

#ifdef NETATALK
#include <netatalk/at.h>
#include <netatalk/at_var.h>
#endif NETATALK

#ifdef _ORG_FREEBSD_
#include "bpfilter.h"
#if NBPFILTER > 0
#include <net/bpfdesc.h>
#endif
#endif

static int loioctl __P((struct ifnet *, u_long, caddr_t));
static void lortrequest __P((int, struct rtentry *, struct sockaddr *));

#ifdef _ORG_FREEBSD_
static
#endif
void loopattach __P((void *));
#ifdef _ORG_FREEBSD_
PSEUDO_SET(loopattach, if_loop);
#endif

static int looutput __P((struct ifnet *ifp,
		struct mbuf *m, struct sockaddr *dst, struct rtentry *rt));

#ifdef TINY_LOMTU
#define	LOMTU	(1024+512)
#else
#define LOMTU	16384
#endif

struct	ifnet loif[NLOOP];

/* ARGSUSED */
#ifdef _ORG_FREEBSD_
static
#endif
void
loopattach(dummy)
	void *dummy;
{
	register struct ifnet *ifp;
	register int i = 0;

	for (ifp = loif; i < NLOOP; ifp++) {
	    ifp->if_name = "lo";
	    ifp->if_unit = i++;
	    ifp->if_mtu = LOMTU;
	    ifp->if_flags = (short)(IFF_LOOPBACK | IFF_MULTICAST);
	    ifp->if_ioctl = loioctl;
	    ifp->if_output = looutput;
	    ifp->if_type = IFT_LOOP;
	    ifp->if_snd.ifq_maxlen = IFQ_MAXLEN;
	    if_attach(ifp);
#if NBPFILTER > 0
	    bpfattach(ifp, DLT_NULL, sizeof(u_int));
#endif
	}
}

static int
looutput(ifp, m, dst, rt)
	struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
	register struct rtentry *rt;
{
	if ((m->m_flags & M_PKTHDR) == 0)
		panic("looutput no HDR");

	if (rt && rt->rt_flags & (RTF_REJECT|RTF_BLACKHOLE)) {
		m_freem(m);
		return (rt->rt_flags & RTF_BLACKHOLE ? 0 :
		        rt->rt_flags & RTF_HOST ? EHOSTUNREACH : ENETUNREACH);
	}
	ifp->if_opackets++;
	ifp->if_obytes += m->m_pkthdr.len;
#if 1	/* XXX */
	switch (dst->sa_family) {
	case AF_INET:
	case AF_IPX:
	case AF_NS:
	case AF_ISO:
	case AF_APPLETALK:
		break;
	default:
		printf("looutput: af=%d unexpected", dst->sa_family);
		m_freem(m);
		return (EAFNOSUPPORT);
	}
#endif
	return(if_simloop(ifp, m, dst, 0));
}

/*
 * if_simloop()
 *
 * This function is to support software emulation of hardware loopback,
 * i.e., for interfaces with the IFF_SIMPLEX attribute. Since they can't
 * hear their own broadcasts, we create a copy of the packet that we
 * would normally receive via a hardware loopback.
 *
 * This function expects the packet to include the media header of length hlen.
 */

int
if_simloop(ifp, m, dst, hlen)
	struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
	int hlen;
{
	int s, isr;
	register struct ifqueue *ifq = 0;

	if ((m->m_flags & M_PKTHDR) == 0)
		panic("if_simloop: no HDR");
	m->m_pkthdr.rcvif = ifp;
#if NBPFILTER > 0
	/* BPF write needs to be handled specially */
	if (dst->sa_family == AF_UNSPEC) {
		dst->sa_family = *(mtod(m, int *));
		m->m_len -= sizeof(int);
		m->m_pkthdr.len -= sizeof(int);
		m->m_data += sizeof(int);
	}

	if (ifp->if_bpf) {
		struct mbuf m0, *n = m;
		u_int af = dst->sa_family;

		if (ifp->if_bpf->bif_dlt == DLT_NULL) {
			/*
			 * We need to prepend the address family as
			 * a four byte field.  Cons up a dummy header
			 * to pacify bpf.  This is safe because bpf
			 * will only read from the mbuf (i.e., it won't
			 * try to free it or keep a pointer a to it).
			 */
			m0.m_next = m;
			m0.m_len = 4;
			m0.m_data = (char *)&af;
			n = &m0;
		}
		bpf_mtap(ifp, n);
	}
#endif

	/* Strip away media header */
	if (hlen > 0) {
#if defined(__alpha__) || defined(EFI64)
		/* The alpha doesn't like unaligned data.
		 * We move data down in the first mbuf */
		if (hlen & 3) {
			bcopy(m->m_data + hlen, m->m_data, m->m_len - hlen);
			m->m_len -= hlen;
			if (m->m_flags & M_PKTHDR)
				m->m_pkthdr.len -= hlen;
		} else
#endif
		m_adj(m, hlen);
	}

	switch (dst->sa_family) {
#ifdef INET
	case AF_INET:
		ifq = &ipintrq;
		isr = NETISR_IP;
		break;
#endif
#ifdef IPX
	case AF_IPX:
		ifq = &ipxintrq;
		isr = NETISR_IPX;
		break;
#endif
#ifdef NS
	case AF_NS:
		ifq = &nsintrq;
		isr = NETISR_NS;
		break;
#endif
#ifdef ISO
	case AF_ISO:
		ifq = &clnlintrq;
		isr = NETISR_ISO;
		break;
#endif
#ifdef NETATALK
	case AF_APPLETALK:
	        ifq = &atintrq2;
		isr = NETISR_ATALK;
		break;
#endif NETATALK
	default:
		printf("if_simloop: can't handle af=%d\n", dst->sa_family);
		m_freem(m);
		return (EAFNOSUPPORT);
	}
	s = splimp();
	if (IF_QFULL(ifq)) {
		IF_DROP(ifq);
		m_freem(m);
		splx(s);
		return (ENOBUFS);
	}
	IF_ENQUEUE(ifq, m);
	schednetisr(isr);
	ifp->if_ipackets++;
	ifp->if_ibytes += m->m_pkthdr.len;
	splx(s);
	return (0);
}

/* ARGSUSED */
static void
lortrequest(cmd, rt, sa)
	int cmd;
	struct rtentry *rt;
	struct sockaddr *sa;
{
	if (rt) {
		rt->rt_rmx.rmx_mtu = rt->rt_ifp->if_mtu; /* for ISO */
		/*
		 * For optimal performance, the send and receive buffers
		 * should be at least twice the MTU plus a little more for
		 * overhead.
		 */
		rt->rt_rmx.rmx_recvpipe = 
			rt->rt_rmx.rmx_sendpipe = 3 * LOMTU;
	}
}

/*
 * Process an ioctl request.
 */
/* ARGSUSED */
static int
loioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	u_long cmd;
	caddr_t data;
{
	register struct ifaddr *ifa;
	register struct ifreq *ifr = (struct ifreq *)data;
	register int error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP | IFF_RUNNING;
		ifa = (struct ifaddr *)data;
		ifa->ifa_rtrequest = lortrequest;
		/*
		 * Everything else is done at a higher level.
		 */
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		if (ifr == 0) {
			error = EAFNOSUPPORT;		/* XXX */
			break;
		}
		switch (ifr->ifr_addr.sa_family) {

#ifdef INET
		case AF_INET:
			break;
#endif

		default:
			error = EAFNOSUPPORT;
			break;
		}
		break;

	case SIOCSIFMTU:
		ifp->if_mtu = ifr->ifr_mtu;
		break;

	case SIOCSIFFLAGS:
		break;

	default:
		error = EINVAL;
	}
	return (error);
}
#endif /* NLOOP > 0 */
