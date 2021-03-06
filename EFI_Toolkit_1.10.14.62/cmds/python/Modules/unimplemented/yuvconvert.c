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

#include "yuv.h"

void
yuv_sv411_to_cl422dc(int invert, void *data, void *yuv, int width, int height)
{
	struct yuv411 *in = data;
	struct yuv422 *out_even = yuv;
	struct yuv422 *out_odd = out_even + width / 2;
	int i, j;		/* counters */

	for (i = height / 2; i--; ) {
		for (j = width / 4; j--; ) {
			YUV422_Y0(*out_even) = YUV411_Y00(*in);
			YUV422_U0(*out_even) = YUV411_U00(*in);
			YUV422_V0(*out_even) = YUV411_V00(*in);
			YUV422_Y1(*out_even) = YUV411_Y01(*in);
			out_even++;
			YUV422_Y0(*out_even) = YUV411_Y02(*in);
			YUV422_U0(*out_even) = YUV411_U02(*in);
			YUV422_V0(*out_even) = YUV411_V02(*in);
			YUV422_Y1(*out_even) = YUV411_Y03(*in);
			out_even++;
			YUV422_Y0(*out_odd) = YUV411_Y10(*in);
			YUV422_U0(*out_odd) = YUV411_U10(*in);
			YUV422_V0(*out_odd) = YUV411_V10(*in);
			YUV422_Y1(*out_odd) = YUV411_Y11(*in);
			out_odd++;
			YUV422_Y0(*out_odd) = YUV411_Y12(*in);
			YUV422_U0(*out_odd) = YUV411_U12(*in);
			YUV422_V0(*out_odd) = YUV411_V12(*in);
			YUV422_Y1(*out_odd) = YUV411_Y13(*in);
			out_odd++;
			in++;
		}
		out_even += width / 2;
		out_odd += width / 2;
	}
}

void
yuv_sv411_to_cl422dc_quartersize(int invert, void *data, void *yuv,
				 int width, int height)
{
	int w4 = width / 4;	/* quarter of width is used often */
	struct yuv411 *in_even = data;
	struct yuv411 *in_odd = in_even + w4;
	struct yuv422 *out_even = yuv;
	struct yuv422 *out_odd = out_even + w4;
	int i, j;		/* counters */
	int u, v;		/* U and V values */

	for (i = height / 4; i--; ) {
		for (j = w4; j--; ) {
			u = YUV411_U00(*in_even);
			v = YUV411_V00(*in_even);

			YUV422_Y0(*out_even) = YUV411_Y00(*in_even);
			YUV422_U0(*out_even) = u;
			YUV422_V0(*out_even) = v;
			YUV422_Y1(*out_even) = YUV411_Y02(*in_even);

			YUV422_Y0(*out_odd) = YUV411_Y10(*in_odd);
			YUV422_U0(*out_odd) = u;
			YUV422_V0(*out_odd) = v;
			YUV422_Y1(*out_odd) = YUV411_Y12(*in_odd);

			in_even++;
			in_odd++;
			out_even++;
			out_odd++;
		}
		in_even += w4;
		in_odd += w4;
		out_even += w4;
		out_odd += w4;
	}
}

void
yuv_sv411_to_cl422dc_sixteenthsize(int invert, void *data, void *yuv,
				   int width, int height)
{
	int w4_3 = 3 * width / 4; /* three quarters of width is used often */
	int w8 = width / 8;	/* and so is one eighth */
	struct yuv411 *in_even = data;
	struct yuv411 *in_odd = in_even + width / 2;
	struct yuv422 *out_even = yuv;
	struct yuv422 *out_odd = out_even + w8;
	int i, j;		/* counters */
	int u, v;		/* U and V values */

	for (i = height / 8; i--; ) {
		for (j = w8; j--; ) {
			u = YUV411_U00(in_even[0]);
			v = YUV411_V00(in_even[0]);

			YUV422_Y0(*out_even) = YUV411_Y00(in_even[0]);
			YUV422_U0(*out_even) = u;
			YUV422_V0(*out_even) = v;
			YUV422_Y1(*out_even) = YUV411_Y00(in_even[1]);

			YUV422_Y0(*out_odd) = YUV411_Y00(in_odd[0]);
			YUV422_U0(*out_odd) = u;
			YUV422_V0(*out_odd) = v;
			YUV422_Y1(*out_odd) = YUV411_Y00(in_even[1]);

			in_even += 2;
			in_odd += 2;
			out_even++;
			out_odd++;
		}
		in_even += w4_3;
		in_odd += w4_3;
		out_even += w8;
		out_odd += w8;
	}
}
