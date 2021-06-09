/*
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)in_proto.c	8.2 (Berkeley) 2/9/95
 *	$Id: in_proto.c,v 1.1.1.1 2003/11/19 01:53:26 kyu3 Exp $
 */

#include "opt_ipdivert.h"
#include "opt_ipx.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/igmp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
/*
 * TCP/IP protocol family: IP, ICMP, UDP, TCP.
 */

#ifdef IPXIP
#include <netipx/ipx_ip.h>
#endif

#ifdef NSIP
#include <netns/ns.h>
#include <netns/ns_if.h>
#endif

extern	struct domain inetdomain;
static	struct pr_usrreqs nousrreqs;

struct protosw inetsw[] = {
{ 0,		&inetdomain,	0,		0,
  0,		0,		0,		0,
  0,
  ip_init,	0,		ip_slowtimo,	ip_drain,
  &nousrreqs
},
{ SOCK_DGRAM,	&inetdomain,	IPPROTO_UDP,	PR_ATOMIC|PR_ADDR,
  udp_input,	0,		udp_ctlinput,	ip_ctloutput,
  0,
  udp_init,	0,		0,		0,
  &udp_usrreqs
},
{ SOCK_STREAM,	&inetdomain,	IPPROTO_TCP,
	PR_CONNREQUIRED|PR_IMPLOPCL|PR_WANTRCVD,
  tcp_input,	0,		tcp_ctlinput,	tcp_ctloutput,
  0,
  tcp_init,	tcp_fasttimo,	tcp_slowtimo,	tcp_drain,
  &tcp_usrreqs
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_RAW,	PR_ATOMIC|PR_ADDR,
  rip_input,	0,		rip_ctlinput,	rip_ctloutput,
  0,
  0,		0,		0,		0,
  &rip_usrreqs
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_ICMP,	PR_ATOMIC|PR_ADDR,
  icmp_input,	0,		0,		rip_ctloutput,
  0,
  0,		0,		0,		0,
  &rip_usrreqs
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_IGMP,	PR_ATOMIC|PR_ADDR,
  igmp_input,	0,		0,		rip_ctloutput,
  0,
  igmp_init,	igmp_fasttimo,	igmp_slowtimo,	0,
  &rip_usrreqs
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_RSVP,	PR_ATOMIC|PR_ADDR,
  rsvp_input,	0,		0,		rip_ctloutput,
  0,
  0,		0,		0,		0,
  &rip_usrreqs
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_IPIP,	PR_ATOMIC|PR_ADDR,
  ipip_input,	0,	 	0,		rip_ctloutput,
  0,
  0,		0,		0,		0,
  &rip_usrreqs
},
#ifdef IPDIVERT
{ SOCK_RAW,	&inetdomain,	IPPROTO_DIVERT,	PR_ATOMIC|PR_ADDR,
  div_input,	0,	 	0,		ip_ctloutput,
  0,
  div_init,	0,		0,		0,
  &div_usrreqs,
},
#endif
#ifdef TPIP
{ SOCK_SEQPACKET,&inetdomain,	IPPROTO_TP,	PR_CONNREQUIRED|PR_WANTRCVD,
  tpip_input,	0,		tpip_ctlinput,	tp_ctloutput,
  tp_usrreq,
  tp_init,	0,		tp_slowtimo,	tp_drain,
},
#endif
/* EON (ISO CLNL over IP) */
#ifdef EON
{ SOCK_RAW,	&inetdomain,	IPPROTO_EON,	0,
  eoninput,	0,		eonctlinput,		0,
  0,
  eonprotoinit,	0,		0,		0,
},
#endif
#ifdef IPXIP
{ SOCK_RAW,	&inetdomain,	IPPROTO_IDP,	PR_ATOMIC|PR_ADDR,
  ipxip_input,	0,		ipxip_ctlinput,	0,
  0,
  0,		0,		0,		0,
  &rip_usrreqs
},
#endif
#ifdef NSIP
{ SOCK_RAW,	&inetdomain,	IPPROTO_IDP,	PR_ATOMIC|PR_ADDR,
  idpip_input,	0,		nsip_ctlinput,	0,
  0,
  0,		0,		0,		0,
  &rip_usrreqs
},
#endif
	/* raw wildcard */
{ SOCK_RAW,	&inetdomain,	0,		PR_ATOMIC|PR_ADDR,
  rip_input,	0,		0,		rip_ctloutput,
  0,
  rip_init,	0,		0,		0,
  &rip_usrreqs
},
};

extern int in_inithead __P((void **, int));

struct domain inetdomain =
    { AF_INET, "internet", 0, 0, 0, 
      inetsw, &inetsw[sizeof(inetsw)/sizeof(inetsw[0])], 0,
      in_inithead, 32, sizeof(struct sockaddr_in)
    };

DOMAIN_SET(inet);

SYSCTL_NODE(_net,      PF_INET,		inet,	CTLFLAG_RW, 0,
	"Internet Family");

SYSCTL_NODE(_net_inet, IPPROTO_IP,	ip,	CTLFLAG_RW, 0,	"IP");
SYSCTL_NODE(_net_inet, IPPROTO_ICMP,	icmp,	CTLFLAG_RW, 0,	"ICMP");
SYSCTL_NODE(_net_inet, IPPROTO_UDP,	udp,	CTLFLAG_RW, 0,	"UDP");
SYSCTL_NODE(_net_inet, IPPROTO_TCP,	tcp,	CTLFLAG_RW, 0,	"TCP");
SYSCTL_NODE(_net_inet, IPPROTO_IGMP,	igmp,	CTLFLAG_RW, 0,	"IGMP");
SYSCTL_NODE(_net_inet, IPPROTO_RAW,	raw,	CTLFLAG_RW, 0,	"RAW");
#ifdef IPDIVERT
SYSCTL_NODE(_net_inet, IPPROTO_DIVERT,	div,	CTLFLAG_RW, 0,	"DIVERT");
#endif