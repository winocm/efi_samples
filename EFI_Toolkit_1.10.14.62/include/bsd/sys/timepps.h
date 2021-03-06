/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * $Id: timepps.h,v 1.1.1.1 2003/11/19 01:49:09 kyu3 Exp $
 *
 * The is a FreeBSD protype version of the "draft-mogul-pps-api-02.txt" 
 * specification for Pulse Per Second timing interfaces.  
 *
 */

#ifndef _SYS_TIMEPPS_H_
#define _SYS_TIMEPPS_H_

#include <sys/ioccom.h>

typedef int pps_handle_t;	

typedef unsigned pps_seq_t;

typedef struct ntp_fp {
	unsigned int	integral;
	unsigned int	fractional;
} ntp_fp_t;

typedef union pps_timeu {
	struct timespec	tspec;
	ntp_fp_t	ntpfp;
	unsigned long	longpair[2];
} pps_timeu_t;

typedef struct {
	pps_seq_t	assert_sequence;	/* assert event seq # */
	pps_seq_t	clear_sequence;		/* clear event seq # */
	pps_timeu_t	assert_tu;
	pps_timeu_t	clear_tu;
	int		current_mode;		/* current mode bits */
} pps_info_t;

#define assert_timestamp        assert_tu.tspec
#define clear_timestamp         clear_tu.tspec

#define assert_timestamp_ntpfp  assert_tu.ntpfp
#define clear_timestamp_ntpfp   clear_tu.ntpfp

typedef struct {
	int mode;				/* mode bits */
	pps_timeu_t assert_off_tu;
	pps_timeu_t clear_off_tu;
} pps_params_t;

#define assert_offset   assert_off_tu.tspec
#define clear_offset    clear_off_tu.tspec

#define assert_offset_ntpfp     assert_off_tu.ntpfp
#define clear_offset_ntpfp      clear_off_tu.ntpfp


#define PPS_CAPTUREASSERT	0x01
#define PPS_CAPTURECLEAR	0x02
#define PPS_CAPTUREBOTH		0x03

#define PPS_OFFSETASSERT	0x10
#define PPS_OFFSETCLEAR		0x20

#define PPS_HARDPPSONASSERT	0x04
#define PPS_HARDPPSONCLEAR	0x08

#define PPS_ECHOASSERT		0x40
#define PPS_ECHOCLEAR		0x80

#define PPS_CANWAIT		0x100

#define PPS_TSFMT_TSPEC		0x1000
#define PPS_TSFMT_NTPFP		0x2000

struct pps_wait_args {
	struct timespec	timeout;
	pps_info_t	pps_info_buf;
};

#define PPS_IOC_CREATE		_IO('1', 1)
#define PPS_IOC_DESTROY		_IO('1', 2)
#define PPS_IOC_SETPARAMS	_IOW('1', 3, pps_params_t)
#define PPS_IOC_GETPARAMS	_IOR('1', 4, pps_params_t)
#define PPS_IOC_GETCAP		_IOR('1', 5, int)
#define PPS_IOC_FETCH		_IOWR('1', 6, pps_info_t)
#define PPS_IOC_WAIT		_IOWR('1', 6, struct pps_wait_args)

#ifdef KERNEL
struct pps_state {
	pps_params_t	ppsparam;
	pps_info_t	ppsinfo;
	int		ppscap;
	struct timecounter *ppstc;
	unsigned	ppscount[3];
};

void pps_event __P((struct pps_state *pps, struct timecounter *tc, unsigned count, int event));
void pps_init __P((struct pps_state *pps));
int pps_ioctl __P((u_long cmd, caddr_t data, struct pps_state *pps));
void hardpps __P((struct timespec *tsp, long nsec));

#else /* !KERNEL */

static __inline int
time_pps_create(int filedes, pps_handle_t *handle)
{
	int error;

	*handle = -1;
	error = ioctl(filedes, PPS_IOC_CREATE, 0);
	if (error < 0) 
		return (-1);
	*handle = filedes;
	return (0);
}

static __inline int
time_pps_destroy(pps_handle_t handle)
{
	return (ioctl(handle, PPS_IOC_DESTROY, 0));
}

static __inline int
time_pps_setparams(pps_handle_t handle, const pps_params_t *ppsparams)
{
	return (ioctl(handle, PPS_IOC_SETPARAMS, ppsparams));
}

static __inline int
time_pps_getparams(pps_handle_t handle, pps_params_t *ppsparams)
{
	return (ioctl(handle, PPS_IOC_GETPARAMS, ppsparams));
}

static __inline int 
time_pps_getcap(pps_handle_t handle, int *mode)
{
	return (ioctl(handle, PPS_IOC_GETCAP, mode));
}

static __inline int
time_pps_fetch(pps_handle_t handle, pps_info_t *ppsinfobuf)
{
	return (ioctl(handle, PPS_IOC_FETCH, ppsinfobuf));
}

static __inline int
time_pps_wait(pps_handle_t handle, const struct timespec *timeout,
        pps_info_t *ppsinfobuf)
{
	int error;
	struct pps_wait_args arg;

	arg.timeout = *timeout;
	error = ioctl(handle, PPS_IOC_WAIT, &arg);
	*ppsinfobuf = arg.pps_info_buf;
	return (error);
}

#endif /* !KERNEL */
#endif /* _SYS_TIMEPPS_H_ */
