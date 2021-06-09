/*
 * Copyright (c) 1980, 1986, 1993
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
 *	@(#)if.c	8.3 (Berkeley) 1/4/94
 *	$Id: if.c,v 1.2 2003/11/24 02:17:31 ychen36 Exp $
 */

#include "opt_compat.h"

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/kernel.h>
#include <sys/sockio.h>
#include <sys/syslog.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/radix.h>

#ifndef _ORG_FREEBSD_
#include <unistd.h>
#endif

/*
 * System initialization
 */

static int ifconf __P((u_long, caddr_t));
#ifdef _ORG_FREEBSD_
static
#endif
void ifinit __P((void *));
static void if_qflush __P((struct ifqueue *));
static void if_slowtimo __P((void *));
#ifndef _ORG_FREEBSD_
static void if_poll_recv __P((void *));
static void if_poll_xmit __P((void *));
#endif
static void link_rtrequest __P((int, struct rtentry *, struct sockaddr *));

SYSINIT(interfaces, SI_SUB_PROTO_IF, SI_ORDER_FIRST, ifinit, NULL)

MALLOC_DEFINE(M_IFADDR, "ifaddr", "interface address");
MALLOC_DEFINE(M_IFMADDR, "ether_multi", "link-level multicast address");

int	ifqmaxlen = IFQ_MAXLEN;
struct	ifnethead ifnet;	/* depend on static init XXX */

/*
 * Network interface utility routines.
 *
 * Routines with ifa_ifwith* names take sockaddr *'s as
 * parameters.
 *
 * This routine assumes that it will be called at splimp() or higher.
 */
/* ARGSUSED*/
void
ifinit(dummy)
	void *dummy;
{
	register struct ifnet *ifp;

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
		if (ifp->if_snd.ifq_maxlen == 0)
			ifp->if_snd.ifq_maxlen = ifqmaxlen;
	if_slowtimo(0);
#ifndef _ORG_FREEBSD_
	if_poll_recv(0);
	if_poll_xmit(0);
#endif
}

int if_index = 0;
struct ifaddr **ifnet_addrs;


/*
 * Attach an interface to the
 * list of "active" interfaces.
 */
void
if_attach(ifp)
	struct ifnet *ifp;
{
	unsigned socksize, ifasize;
	int namelen, masklen;
	char workbuf[64];
	register struct sockaddr_dl *sdl;
	register struct ifaddr *ifa;
	static int if_indexlim = 8;
	static int inited;

	if (!inited) {
		TAILQ_INIT(&ifnet);
		inited = 1;
	}

	TAILQ_INSERT_TAIL(&ifnet, ifp, if_link);
	ifp->if_index = ++if_index;
	/*
	 * XXX -
	 * The old code would work if the interface passed a pre-existing
	 * chain of ifaddrs to this code.  We don't trust our callers to
	 * properly initialize the tailq, however, so we no longer allow
	 * this unlikely case.
	 */
	TAILQ_INIT(&ifp->if_addrhead);
	LIST_INIT(&ifp->if_multiaddrs);
	getmicrotime(&ifp->if_lastchange);
	if (ifnet_addrs == 0 || if_index >= if_indexlim) {
		unsigned n = (if_indexlim <<= 1) * sizeof(ifa);
		struct ifaddr **q = (struct ifaddr **)
					malloc(n, M_IFADDR, M_WAITOK);
		bzero((caddr_t)q, n);
		if (ifnet_addrs) {
			bcopy((caddr_t)ifnet_addrs, (caddr_t)q, n/2);
			free((caddr_t)ifnet_addrs, M_IFADDR);
		}
		ifnet_addrs = q;
	}
	/*
	 * create a Link Level name for this device
	 */
	namelen = snprintf(workbuf, sizeof(workbuf),
	    "%s%d", ifp->if_name, ifp->if_unit);
#define _offsetof(t, m) ((int)((size_t)((caddr_t)&((t *)0)->m)))
	masklen = _offsetof(struct sockaddr_dl, sdl_data[0]) + namelen;
	socksize = masklen + ifp->if_addrlen;
#define ROUNDUP(a) (1 + (((a) - 1) | (sizeof(long) - 1)))
	if (socksize < sizeof(*sdl))
		socksize = sizeof(*sdl);
	socksize = ROUNDUP(socksize);
	ifasize = sizeof(*ifa) + 2 * socksize;
	ifa = (struct ifaddr *)malloc(ifasize, M_IFADDR, M_WAITOK);
	if (ifa) {
		bzero((caddr_t)ifa, ifasize);
		sdl = (struct sockaddr_dl *)(ifa + 1);
		sdl->sdl_len = socksize;
		sdl->sdl_family = AF_LINK;
		bcopy(workbuf, sdl->sdl_data, namelen);
		sdl->sdl_nlen = namelen;
		sdl->sdl_index = ifp->if_index;
		sdl->sdl_type = ifp->if_type;
		ifnet_addrs[if_index - 1] = ifa;
		ifa->ifa_ifp = ifp;
		ifa->ifa_rtrequest = link_rtrequest;
		ifa->ifa_addr = (struct sockaddr *)sdl;
		sdl = (struct sockaddr_dl *)(socksize + (caddr_t)sdl);
		ifa->ifa_netmask = (struct sockaddr *)sdl;
		sdl->sdl_len = masklen;
		while (namelen != 0)
			sdl->sdl_data[--namelen] = (char)0xff;
		TAILQ_INSERT_HEAD(&ifp->if_addrhead, ifa, ifa_link);

		log(0, "Interface attached to %s%d\n", ifp->if_name, ifp->if_unit);
	}
}

/*
 * Locate an interface based on a complete address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

#define	equal(a1, a2) \
  (bcmp((caddr_t)(a1), (caddr_t)(a2), ((struct sockaddr *)(a1))->sa_len - 8 ) == 0)
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	    for (ifa = ifp->if_addrhead.tqh_first; ifa; 
		 ifa = ifa->ifa_link.tqe_next) {
		if (ifa->ifa_addr->sa_family != addr->sa_family)
			continue;
		if (equal(addr, ifa->ifa_addr))
			return (ifa);
		if ((ifp->if_flags & IFF_BROADCAST) && ifa->ifa_broadaddr &&
		    equal(ifa->ifa_broadaddr, addr))
			return (ifa);
	}
	return ((struct ifaddr *)0);
}
/*
 * Locate the point to point interface with a given destination address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithdstaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	    if (ifp->if_flags & IFF_POINTOPOINT)
		for (ifa = ifp->if_addrhead.tqh_first; ifa; 
		     ifa = ifa->ifa_link.tqe_next) {
			if (ifa->ifa_addr->sa_family != addr->sa_family)
				continue;
			if (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr))
				return (ifa);
	}
	return ((struct ifaddr *)0);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is most specific found.
 */
struct ifaddr *
ifa_ifwithnet(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	struct ifaddr *ifa_maybe = (struct ifaddr *) 0;
	u_int af = addr->sa_family;
	char *addr_data = addr->sa_data, *cplim;

	/*
	 * AF_LINK addresses can be looked up directly by their index number,
	 * so do that if we can.
	 */
	if (af == AF_LINK) {
	    register struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
	    if (sdl->sdl_index && sdl->sdl_index <= if_index)
		return (ifnet_addrs[sdl->sdl_index - 1]);
	}

	/* 
	 * Scan though each interface, looking for ones that have
	 * addresses in this address family.
	 */
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		for (ifa = ifp->if_addrhead.tqh_first; ifa;
		     ifa = ifa->ifa_link.tqe_next) {
			register char *cp, *cp2, *cp3;

			if (ifa->ifa_addr->sa_family != af)
next:				continue;
			if (ifp->if_flags & IFF_POINTOPOINT) {
				/*
				 * This is a bit broken as it doesn't 
				 * take into account that the remote end may 
				 * be a single node in the network we are
				 * looking for.
				 * The trouble is that we don't know the 
				 * netmask for the remote end.
				 */
				if (ifa->ifa_dstaddr != 0
				    && equal(addr, ifa->ifa_dstaddr))
 					return (ifa);
			} else {
				/*
				 * if we have a special address handler,
				 * then use it instead of the generic one.
				 */
	          		if (ifa->ifa_claim_addr) {
					if ((*ifa->ifa_claim_addr)(ifa, addr)) {
						return (ifa);
					} else {
						continue;
					}
				}

				/*
				 * Scan all the bits in the ifa's address.
				 * If a bit dissagrees with what we are
				 * looking for, mask it with the netmask
				 * to see if it really matters.
				 * (A byte at a time)
				 */
				if (ifa->ifa_netmask == 0)
					continue;
				cp = addr_data;
				cp2 = ifa->ifa_addr->sa_data;
				cp3 = ifa->ifa_netmask->sa_data;
				cplim = ifa->ifa_netmask->sa_len
					+ (char *)ifa->ifa_netmask;
				while (cp3 < cplim)
					if ((*cp++ ^ *cp2++) & *cp3++)
						goto next; /* next address! */
				/*
				 * If the netmask of what we just found
				 * is more specific than what we had before
				 * (if we had one) then remember the new one
				 * before continuing to search
				 * for an even better one.
				 */
				if (ifa_maybe == 0 ||
				    rn_refines((caddr_t)ifa->ifa_netmask,
				    (caddr_t)ifa_maybe->ifa_netmask))
					ifa_maybe = ifa;
			}
		}
	}
	return (ifa_maybe);
}

/*
 * Find an interface address specific to an interface best matching
 * a given address.
 */
struct ifaddr *
ifaof_ifpforaddr(addr, ifp)
	struct sockaddr *addr;
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;
	register char *cp, *cp2, *cp3;
	register char *cplim;
	struct ifaddr *ifa_maybe = 0;
	u_int af = addr->sa_family;

	if (af >= AF_MAX)
		return (0);
	for (ifa = ifp->if_addrhead.tqh_first; ifa; 
	     ifa = ifa->ifa_link.tqe_next) {
		if (ifa->ifa_addr->sa_family != af)
			continue;
		if (ifa_maybe == 0)
			ifa_maybe = ifa;
		if (ifa->ifa_netmask == 0) {
			if (equal(addr, ifa->ifa_addr) ||
			    (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
				return (ifa);
			continue;
		}
		if (ifp->if_flags & IFF_POINTOPOINT) {
			if (equal(addr, ifa->ifa_dstaddr))
				return (ifa);
		} else {
			cp = addr->sa_data;
			cp2 = ifa->ifa_addr->sa_data;
			cp3 = ifa->ifa_netmask->sa_data;
			cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
			for (; cp3 < cplim; cp3++)
				if ((*cp++ ^ *cp2++) & *cp3)
					break;
			if (cp3 == cplim)
				return (ifa);
		}
	}
	return (ifa_maybe);
}

#include <net/route.h>

/*
 * Default action when installing a route with a Link Level gateway.
 * Lookup an appropriate real ifa to point to.
 * This should be moved to /sys/net/link.c eventually.
 */
static void
link_rtrequest(cmd, rt, sa)
	int cmd;
	register struct rtentry *rt;
	struct sockaddr *sa;
{
	register struct ifaddr *ifa;
	struct sockaddr *dst;
	struct ifnet *ifp;

	if (cmd != RTM_ADD || ((ifa = rt->rt_ifa) == 0) ||
	    ((ifp = ifa->ifa_ifp) == 0) || ((dst = rt_key(rt)) == 0))
		return;
	ifa = ifaof_ifpforaddr(dst, ifp);
	if (ifa) {
		IFAFREE(rt->rt_ifa);
		rt->rt_ifa = ifa;
		ifa->ifa_refcnt++;
		if (ifa->ifa_rtrequest && ifa->ifa_rtrequest != link_rtrequest)
			ifa->ifa_rtrequest(cmd, rt, sa);
	}
}

/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_unroute(ifp, flag, fam)
	register struct ifnet *ifp;
	int flag, fam;
{
	register struct ifaddr *ifa;

	ifp->if_flags &= ~flag;
	getmicrotime(&ifp->if_lastchange);
	TAILQ_FOREACH(ifa, &ifp->if_addrhead, ifa_link)
		if (fam == PF_UNSPEC || (fam == ifa->ifa_addr->sa_family))
			pfctlinput(PRC_IFDOWN, ifa->ifa_addr);
	if_qflush(&ifp->if_snd);
	rt_ifmsg(ifp);
}

/*
 * Mark an interface up and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_route(ifp, flag, fam)
	register struct ifnet *ifp;
	int flag, fam;
{
	register struct ifaddr *ifa;

	ifp->if_flags |= flag;
	getmicrotime(&ifp->if_lastchange);
	TAILQ_FOREACH(ifa, &ifp->if_addrhead, ifa_link)
		if (fam == PF_UNSPEC || (fam == ifa->ifa_addr->sa_family))
			pfctlinput(PRC_IFUP, ifa->ifa_addr);
	rt_ifmsg(ifp);
}

/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_down(ifp)
	register struct ifnet *ifp;
{

	if_unroute(ifp, IFF_UP, AF_UNSPEC);
}

/*
 * Mark an interface up and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_up(ifp)
	register struct ifnet *ifp;
{

	if_route(ifp, IFF_UP, AF_UNSPEC);
}

/*
 * Flush an interface queue.
 */
static void
if_qflush(ifq)
	register struct ifqueue *ifq;
{
	register struct mbuf *m, *n;

	n = ifq->ifq_head;
	while ((m = n) != 0) {
		n = m->m_act;
		m_freem(m);
	}
	ifq->ifq_head = 0;
	ifq->ifq_tail = 0;
	ifq->ifq_len = 0;
}

/*
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 */
static void
if_slowtimo(arg)
	void *arg;
{
	register struct ifnet *ifp;
	int s = splimp();

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		if (ifp->if_timer == 0 || --ifp->if_timer)
			continue;
		if (ifp->if_watchdog)
			(*ifp->if_watchdog)(ifp);
	}
	splx(s);
	timeout(if_slowtimo, (void *)0, hz / IFNET_SLOWHZ);
}

#ifndef _ORG_FREEBSD_
#define	IFNET_RECV_POLL_MAX	1
#define	IFNET_RECV_POLL_MIN	(hz * 1)

#define	IFNET_XMIT_POLL_MAX	1
#define	IFNET_XMIT_POLL_MIN	(hz * 1)

#define ISR_BIT( shift )	(1 << shift)

#include <netinet/in.h>
#include <net/netisr.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>

extern void ipintr( void );
extern void arpintr( void );

volatile unsigned int	netisr;


/*
 *  this funtion is here for debug.  It allows systems without
 *  timer services to force polling the send and receive queues.
 */
void
if_force_poll()
{
	if_poll_recv(0);
	if_poll_xmit(0);
}

/*
 * Handle interface recv polling timer routines.  Called
 * from softclock.
 */
static void
if_poll_recv(arg)
	void *arg;
{
	static int poll_rate = IFNET_RECV_POLL_MAX;

	register struct ifnet *ifp;
	int s;
	int receiving;

retry:
	receiving = 0;
	s = splimp();

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		if (ifp->if_poll_recv) {
			receiving += (*ifp->if_poll_recv)(ifp, 0);
		}
	}
	splx(s);

	/*
	 *  If the interface received something, increase the poll rate
	 *  and check netisr's.
	 */
	if ( receiving || ipintrq.ifq_len || arpintrq.ifq_len ) {
//		poll_rate = max( poll_rate - 1, IFNET_RECV_POLL_MAX );

		s = splnet();
 		while ( netisr ) {
			if ( netisr & ISR_BIT( NETISR_IP ) ) {
				netisr &= ~ISR_BIT( NETISR_IP );
				ipintr();
			} else if ( netisr & ISR_BIT( NETISR_ARP ) ) {
				netisr &= ~ISR_BIT( NETISR_ARP );
				arpintr();
			} else if ( netisr & ISR_BIT( NETISR_PPP ) ) {
				netisr &= ~ISR_BIT( NETISR_PPP );
#ifdef not_yet
				pppintr();
#endif
			} else {
				netisr = 0;
			}
		}
		splx(s);
		
		/*
		 *  Since the EFI implementation is poll driven, try to stream
		 *  input by processing delayed acks here in addition to tcp_fasttimo()
		 */
		if ( receiving ) {
			extern int tcp_do_delack(void);

			if ( tcp_do_delack() ) {
				/*
				 *  Give remote system time to send more
				 */
				usleep(1000);
				goto retry;
			}
		}
	} else {
//		poll_rate = min( poll_rate + 1, IFNET_RECV_POLL_MIN );
	}

	timeout(if_poll_recv, (void *)0, poll_rate);
}

/*
 * Handle interface xmit polling timer routines.  Called
 * from softclock.
 */
static void
if_poll_xmit(arg)
	void *arg;
{
	static int poll_rate = IFNET_XMIT_POLL_MAX;

	register struct ifnet *ifp;
	int s = splimp();

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		if (ifp->if_poll_xmit) {
#ifdef not_yet
			/*
			 *  If the interface has transmitted something,
			 *  increase the poll rate
			 */
			if ( (*ifp->if_poll_xmit)(ifp, 0) )
				poll_rate = max( poll_rate - 1, IFNET_XMIT_POLL_MAX );
			else
				poll_rate = min( poll_rate + 2, IFNET_XMIT_POLL_MIN );
#else
			(void)(*ifp->if_poll_xmit)(ifp, 0);
#endif
		}
	}
	splx(s);
	timeout(if_poll_xmit, (void *)0, poll_rate);
}
#endif

/*
 * Map interface name to
 * interface structure pointer.
 */
struct ifnet *
ifunit(name)
	register char *name;
{
	char namebuf[IFNAMSIZ + 1];
	register char *cp, *cp2;
	char *end;
	register struct ifnet *ifp;
	int unit;
	unsigned len;
	register char c = '\0';

	/*
	 * Look for a non numeric part
	 */
	end = name + IFNAMSIZ; 
	cp2 = namebuf;
	cp = name; 
	while ((cp < end) && (c = *cp)) {
		if (c >= '0' && c <= '9')
			break;
		*cp2++ = c;
		cp++;
	}
	if ((cp == end) || (c == '\0') || (cp == name))
		return ((struct ifnet *)0);
	*cp2 = '\0';
	/*
	 * check we have a legal number (limit to 7 digits?)
	 */
	len = (unsigned)(cp - name + 1);
	for (unit = 0;
	    ((c = *cp) >= '0') && (c <= '9') && (unit < 1000000); cp++ ) 
		unit = (unit * 10) + (c - '0');
	if (*cp != '\0')
		return 0;	/* no trailing garbage allowed */
	/*
	 * Now search all the interfaces for this name/number
	 */
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		if (bcmp(ifp->if_name, namebuf, len))
			continue;
		if (unit == ifp->if_unit)
			break;
	}
	return (ifp);
}

/*
 * Interface ioctls.
 */
int
ifioctl(so, cmd, data, p)
	struct socket *so;
	u_long cmd;
	caddr_t data;
	struct proc *p;
{
	register struct ifnet *ifp;
	register struct ifreq *ifr;
	int error;

	switch (cmd) {

	case SIOCGIFCONF:
	case OSIOCGIFCONF:
		return (ifconf(cmd, data));
	}
	ifr = (struct ifreq *)data;
	ifp = ifunit(ifr->ifr_name);
	if (ifp == 0)
		return (ENXIO);
	switch (cmd) {

	case SIOCGIFFLAGS:
		ifr->ifr_flags = ifp->if_flags;
		break;

	case SIOCGIFMETRIC:
		ifr->ifr_metric = ifp->if_metric;
		break;

	case SIOCGIFMTU:
		ifr->ifr_mtu = ifp->if_mtu;
		break;

	case SIOCGIFPHYS:
		ifr->ifr_phys = ifp->if_physical;
		break;

	case SIOCSIFFLAGS:
		error = p ? suser(p->p_ucred, &p->p_acflag) : 0;
		if (error)
			return (error);
		if (ifp->if_flags & IFF_UP && (ifr->ifr_flags & IFF_UP) == 0) {
			int s = splimp();
			if_down(ifp);
			splx(s);
		}
		if (ifr->ifr_flags & IFF_UP && (ifp->if_flags & IFF_UP) == 0) {
			int s = splimp();
			if_up(ifp);
			splx(s);
		}
		ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
			(ifr->ifr_flags &~ IFF_CANTCHANGE);
		if (ifp->if_ioctl)
			(void) (*ifp->if_ioctl)(ifp, cmd, data);
		getmicrotime(&ifp->if_lastchange);
		break;

	case SIOCSIFMETRIC:
		error = p ? suser(p->p_ucred, &p->p_acflag) : 0;
		if (error)
			return (error);
		ifp->if_metric = ifr->ifr_metric;
		getmicrotime(&ifp->if_lastchange);
		break;

	case SIOCSIFPHYS:
		error = p ? suser(p->p_ucred, &p->p_acflag) : 0;
		if (error)
			return error;
		if (!ifp->if_ioctl)
		        return EOPNOTSUPP;
		error = (*ifp->if_ioctl)(ifp, cmd, data);
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		return(error);

	case SIOCSIFMTU:
		error = p ? suser(p->p_ucred, &p->p_acflag) : 0;
		if (error)
			return (error);
		if (ifp->if_ioctl == NULL)
			return (EOPNOTSUPP);
		/*
		 * 72 was chosen below because it is the size of a TCP/IP
		 * header (40) + the minimum mss (32).
		 */
		if (ifr->ifr_mtu < 72 || ifr->ifr_mtu > 65535)
			return (EINVAL);
		error = (*ifp->if_ioctl)(ifp, cmd, data);
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		return(error);

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		error = p ? suser(p->p_ucred, &p->p_acflag) : 0;
		if (error)
			return (error);

		/* Don't allow group membership on non-multicast interfaces. */
		if ((ifp->if_flags & IFF_MULTICAST) == 0)
			return EOPNOTSUPP;

		/* Don't let users screw up protocols' entries. */
		if (ifr->ifr_addr.sa_family != AF_LINK)
			return EINVAL;

		if (cmd == SIOCADDMULTI) {
			struct ifmultiaddr *ifma;
			error = if_addmulti(ifp, &ifr->ifr_addr, &ifma);
		} else {
			error = if_delmulti(ifp, &ifr->ifr_addr);
		}
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		return error;

        case SIOCSIFMEDIA:
	case SIOCSIFGENERIC:
		error = p ? suser(p->p_ucred, &p->p_acflag) : 0;
		if (error)
			return (error);
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		error = (*ifp->if_ioctl)(ifp, cmd, data);
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		return error;

	case SIOCGIFMEDIA:
	case SIOCGIFGENERIC:
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		return ((*ifp->if_ioctl)(ifp, cmd, data));

	default:
		if (so->so_proto == 0)
			return (EOPNOTSUPP);
#ifndef COMPAT_43
		return ((*so->so_proto->pr_usrreqs->pru_control)(so, cmd,
								 data,
								 ifp, p));
#else
	    {
		int ocmd = cmd;

		switch (cmd) {

		case SIOCSIFDSTADDR:
		case SIOCSIFADDR:
		case SIOCSIFBRDADDR:
		case SIOCSIFNETMASK:
#if BYTE_ORDER != BIG_ENDIAN
			if (ifr->ifr_addr.sa_family == 0 &&
			    ifr->ifr_addr.sa_len < 16) {
				ifr->ifr_addr.sa_family = ifr->ifr_addr.sa_len;
				ifr->ifr_addr.sa_len = 16;
			}
#else
			if (ifr->ifr_addr.sa_len == 0)
				ifr->ifr_addr.sa_len = 16;
#endif
			break;

		case OSIOCGIFADDR:
			cmd = SIOCGIFADDR;
			break;

		case OSIOCGIFDSTADDR:
			cmd = SIOCGIFDSTADDR;
			break;

		case OSIOCGIFBRDADDR:
			cmd = SIOCGIFBRDADDR;
			break;

		case OSIOCGIFNETMASK:
			cmd = SIOCGIFNETMASK;
		}
		error =  ((*so->so_proto->pr_usrreqs->pru_control)(so,
								   cmd,
								   data,
								   ifp, p));
		switch (ocmd) {

		case OSIOCGIFADDR:
		case OSIOCGIFDSTADDR:
		case OSIOCGIFBRDADDR:
		case OSIOCGIFNETMASK:
			*(u_short *)&ifr->ifr_addr = ifr->ifr_addr.sa_family;
		}
		return (error);

	    }
#endif
	}
	return (0);
}

/*
 * Set/clear promiscuous mode on interface ifp based on the truth value
 * of pswitch.  The calls are reference counted so that only the first
 * "on" request actually has an effect, as does the final "off" request.
 * Results are undefined if the "off" and "on" requests are not matched.
 */
int
ifpromisc(ifp, pswitch)
	struct ifnet *ifp;
	int pswitch;
{
	struct ifreq ifr;
	int error;

	if (pswitch) {
		/*
		 * If the device is not configured up, we cannot put it in
		 * promiscuous mode.
		 */
		if ((ifp->if_flags & IFF_UP) == 0)
			return (ENETDOWN);
		if (ifp->if_pcount++ != 0)
			return (0);
		ifp->if_flags |= IFF_PROMISC;
		log(LOG_INFO, "%s%d: promiscuous mode enabled\n",
		    ifp->if_name, ifp->if_unit);
	} else {
		if (--ifp->if_pcount > 0)
			return (0);
		ifp->if_flags &= ~IFF_PROMISC;
	}
	ifr.ifr_flags = ifp->if_flags;
	error = (*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, (caddr_t)&ifr);
	if (error == 0)
		rt_ifmsg(ifp);
	return error;
}

/*
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 */
/*ARGSUSED*/
static int
ifconf(cmd, data)
	u_long cmd;
	caddr_t data;
{
	register struct ifconf *ifc = (struct ifconf *)data;
	register struct ifnet *ifp = ifnet.tqh_first;
	register struct ifaddr *ifa;
	struct ifreq ifr, *ifrp;
	int space = ifc->ifc_len, error = 0;

	ifrp = ifc->ifc_req;
	for (; space > sizeof (ifr) && ifp; ifp = ifp->if_link.tqe_next) {
		char workbuf[64];
		int ifnlen;

		ifnlen = snprintf(workbuf, sizeof(workbuf),
		    "%s%d", ifp->if_name, ifp->if_unit);
		if(ifnlen + 1 > sizeof ifr.ifr_name) {
			error = ENAMETOOLONG;
		} else {
			strcpy(ifr.ifr_name, workbuf);
		}

		if ((ifa = ifp->if_addrhead.tqh_first) == 0) {
			bzero((caddr_t)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
			    sizeof (ifr));
			if (error)
				break;
			space -= sizeof (ifr), ifrp++;
		} else
		    for ( ; space > sizeof (ifr) && ifa; 
			 ifa = ifa->ifa_link.tqe_next) {
			register struct sockaddr *sa = ifa->ifa_addr;
#ifdef COMPAT_43
			if (cmd == OSIOCGIFCONF) {
				struct osockaddr *osa =
					 (struct osockaddr *)&ifr.ifr_addr;
				ifr.ifr_addr = *sa;
				osa->sa_family = sa->sa_family;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else
#endif
			if (sa->sa_len <= sizeof(*sa)) {
				ifr.ifr_addr = *sa;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else {
				space -= sa->sa_len - sizeof(*sa);
				if (space < sizeof (ifr))
					break;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr.ifr_name));
				if (error == 0)
				    error = copyout((caddr_t)sa,
				      (caddr_t)&ifrp->ifr_addr, sa->sa_len);
				ifrp = (struct ifreq *)
					(sa->sa_len + (caddr_t)&ifrp->ifr_addr);
			}
			if (error)
				break;
			space -= sizeof (ifr);
		}
	}
	ifc->ifc_len -= space;
	return (error);
}

/*
 * Just like if_promisc(), but for all-multicast-reception mode.
 */
int
if_allmulti(ifp, onswitch)
	struct ifnet *ifp;
	int onswitch;
{
	int error = 0;
	int s = splimp();

	if (onswitch) {
		if (ifp->if_amcount++ == 0) {
			ifp->if_flags |= IFF_ALLMULTI;
			error = ifp->if_ioctl(ifp, SIOCSIFFLAGS, 0);
		}
	} else {
		if (ifp->if_amcount > 1) {
			ifp->if_amcount--;
		} else {
			ifp->if_amcount = 0;
			ifp->if_flags &= ~IFF_ALLMULTI;
			error = ifp->if_ioctl(ifp, SIOCSIFFLAGS, 0);
		}
	}
	splx(s);

	if (error == 0)
		rt_ifmsg(ifp);
	return error;
}

/*
 * Add a multicast listenership to the interface in question.
 * The link layer provides a routine which converts 
 */
int
if_addmulti(ifp, sa, retifma)
	struct ifnet *ifp;	/* interface to manipulate */
	struct sockaddr *sa;	/* address to add */
	struct ifmultiaddr **retifma;
{
	struct sockaddr *llsa, *dupsa;
	int error, s;
	struct ifmultiaddr *ifma;

	/*
	 * If the matching multicast address already exists
	 * then don't add a new one, just add a reference
	 */
	for (ifma = ifp->if_multiaddrs.lh_first; ifma; 
	     ifma = ifma->ifma_link.le_next) {
		if (equal(sa, ifma->ifma_addr)) {
			ifma->ifma_refcount++;
			if (retifma)
				*retifma = ifma;
			return 0;
		}
	}

	/*
	 * Give the link layer a chance to accept/reject it, and also
	 * find out which AF_LINK address this maps to, if it isn't one
	 * already.
	 */
	if (ifp->if_resolvemulti) {
		error = ifp->if_resolvemulti(ifp, &llsa, sa);
		if (error) return error;
	} else {
		llsa = 0;
	}

	MALLOC(ifma, struct ifmultiaddr *, sizeof *ifma, M_IFMADDR, M_WAITOK);
	MALLOC(dupsa, struct sockaddr *, sa->sa_len, M_IFMADDR, M_WAITOK);
	bcopy(sa, dupsa, sa->sa_len);

	ifma->ifma_addr = dupsa;
	ifma->ifma_lladdr = llsa;
	ifma->ifma_ifp = ifp;
	ifma->ifma_refcount = 1;
	ifma->ifma_protospec = 0;
	rt_newmaddrmsg(RTM_NEWMADDR, ifma);

	/*
	 * Some network interfaces can scan the address list at
	 * interrupt time; lock them out.
	 */
	s = splimp();
	LIST_INSERT_HEAD(&ifp->if_multiaddrs, ifma, ifma_link);
	splx(s);
	*retifma = ifma;

	if (llsa != 0) {
		for (ifma = ifp->if_multiaddrs.lh_first; ifma;
		     ifma = ifma->ifma_link.le_next) {
			if (equal(ifma->ifma_addr, llsa))
				break;
		}
		if (ifma) {
			ifma->ifma_refcount++;
		} else {
			MALLOC(ifma, struct ifmultiaddr *, sizeof *ifma,
			       M_IFMADDR, M_WAITOK);
			MALLOC(dupsa, struct sockaddr *, llsa->sa_len,
			       M_IFMADDR, M_WAITOK);
			bcopy(llsa, dupsa, llsa->sa_len);
			ifma->ifma_addr = dupsa;
			ifma->ifma_ifp = ifp;
			ifma->ifma_refcount = 1;
			s = splimp();
			LIST_INSERT_HEAD(&ifp->if_multiaddrs, ifma, ifma_link);
			splx(s);
		}
	}
	/*
	 * We are certain we have added something, so call down to the
	 * interface to let them know about it.
	 */
	s = splimp();
	ifp->if_ioctl(ifp, SIOCADDMULTI, 0);
	splx(s);

	return 0;
}

/*
 * Remove a reference to a multicast address on this interface.  Yell
 * if the request does not match an existing membership.
 */
int
if_delmulti(ifp, sa)
	struct ifnet *ifp;
	struct sockaddr *sa;
{
	struct ifmultiaddr *ifma;
	int s;

	for (ifma = ifp->if_multiaddrs.lh_first; ifma; 
	     ifma = ifma->ifma_link.le_next)
		if (equal(sa, ifma->ifma_addr))
			break;
	if (ifma == 0)
		return ENOENT;

	if (ifma->ifma_refcount > 1) {
		ifma->ifma_refcount--;
		return 0;
	}

	rt_newmaddrmsg(RTM_DELMADDR, ifma);
	sa = ifma->ifma_lladdr;
	s = splimp();
	LIST_REMOVE(ifma, ifma_link);
	splx(s);
	free(ifma->ifma_addr, M_IFMADDR);
	free(ifma, M_IFMADDR);
	if (sa == 0)
		return 0;

	/*
	 * Now look for the link-layer address which corresponds to
	 * this network address.  It had been squirreled away in
	 * ifma->ifma_lladdr for this purpose (so we don't have
	 * to call ifp->if_resolvemulti() again), and we saved that
	 * value in sa above.  If some nasty deleted the
	 * link-layer address out from underneath us, we can deal because
	 * the address we stored was is not the same as the one which was
	 * in the record for the link-layer address.  (So we don't complain
	 * in that case.)
	 */
	for (ifma = ifp->if_multiaddrs.lh_first; ifma; 
	     ifma = ifma->ifma_link.le_next)
		if (equal(sa, ifma->ifma_addr))
			break;
	if (ifma == 0)
		return 0;

	if (ifma->ifma_refcount > 1) {
		ifma->ifma_refcount--;
		return 0;
	}

	s = splimp();
	LIST_REMOVE(ifma, ifma_link);
	ifp->if_ioctl(ifp, SIOCDELMULTI, 0);
	splx(s);
	free(ifma->ifma_addr, M_IFMADDR);
	free(sa, M_IFMADDR);
	free(ifma, M_IFMADDR);

	return 0;
}

struct ifmultiaddr *
ifmaof_ifpforaddr(sa, ifp)
	struct sockaddr *sa;
	struct ifnet *ifp;
{
	struct ifmultiaddr *ifma;
	
	for (ifma = ifp->if_multiaddrs.lh_first; ifma;
	     ifma = ifma->ifma_link.le_next)
		if (equal(ifma->ifma_addr, sa))
			break;

	return ifma;
}

SYSCTL_NODE(_net, PF_LINK, link, CTLFLAG_RW, 0, "Link layers");
SYSCTL_NODE(_net_link, 0, generic, CTLFLAG_RW, 0, "Generic link-management");

