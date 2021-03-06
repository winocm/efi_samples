/***********************************************************
Copyright 1991-1995 by Stichting Mathematisch Centrum, Amsterdam,
The Netherlands.

Portions Copyright (c) 2000 by Intel Corporation. 

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI or Corporation for National Research Initiatives or
CNRI or Intel Corporation not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

While CWI is the initial source for this software, a modified version
is made available by the Corporation for National Research Initiatives
(CNRI) at the Internet address ftp://ftp.python.org.

STICHTING MATHEMATISCH CENTRUM AND CNRI AND INTEL CORPORATION DISCLAIM ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH
CENTRUM OR CNRI OR INTEL CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/* Complex math module */

/* much code borrowed from mathmodule.c */

#include "Python.h"

#include "mymath.h"

#ifdef i860
/* Cray APP has bogus definition of HUGE_VAL in <math.h> */
#undef HUGE_VAL
#endif

#if !defined(EFI32) && !defined(EFI64)
#ifdef HUGE_VAL
#define CHECK(x) if (errno != 0) ; \
	else if (-HUGE_VAL <= (x) && (x) <= HUGE_VAL) ; \
	else errno = ERANGE
#else
#define CHECK(x) /* Don't know how to check */
#endif
#else
#define CHECK(x) /* Don't know how to check */
#endif

#ifndef M_PI
#define M_PI (3.141592653589793239)
#endif

/* First, the C functions that do the real work */

/* constants */
static Py_complex c_1 = {1., 0.};
static Py_complex c_half = {0.5, 0.};
static Py_complex c_i = {0., 1.};
static Py_complex c_i2 = {0., 0.5};
#if 0
static Py_complex c_mi = {0., -1.};
static Py_complex c_pi2 = {M_PI/2., 0.};
#endif

/* forward declarations */
staticforward Py_complex c_log();
staticforward Py_complex c_prodi();
staticforward Py_complex c_sqrt();


static Py_complex c_acos(x)
	Py_complex x;
{
	return c_neg(c_prodi(c_log(c_sum(x,c_prod(c_i,
		    c_sqrt(c_diff(c_1,c_prod(x,x))))))));
}

static char c_acos_doc [] =
"acos(x)\n\
\n\
Return the arc cosine of x.";


static Py_complex c_acosh(x)
	Py_complex x;
{
	return c_log(c_sum(x,c_prod(c_i,
		    c_sqrt(c_diff(c_1,c_prod(x,x))))));
}

static char c_acosh_doc [] =
"acosh(x)\n\
\n\
Return the hyperbolic arccosine of x.";


static Py_complex c_asin(x)
	Py_complex x;
{
	return c_neg(c_prodi(c_log(c_sum(c_prod(c_i,x),
		    c_sqrt(c_diff(c_1,c_prod(x,x)))))));
}

static char c_asin_doc [] =
"asin(x)\n\
\n\
Return the arc sine of x.";


static Py_complex c_asinh(x)
	Py_complex x;
{
	/* Break up long expression for WATCOM */
	Py_complex z;
	z = c_sum(c_1,c_prod(x,x));
	z = c_diff(c_sqrt(z),x);
	return c_neg(c_log(z));
}

static char c_asinh_doc [] =
"asinh(x)\n\
\n\
Return the hyperbolic arc sine of x.";


static Py_complex c_atan(x)
	Py_complex x;
{
	return c_prod(c_i2,c_log(c_quot(c_sum(c_i,x),c_diff(c_i,x))));
}

static char c_atan_doc [] =
"atan(x)\n\
\n\
Return the arc tangent of x.";


static Py_complex c_atanh(x)
	Py_complex x;
{
	return c_prod(c_half,c_log(c_quot(c_sum(c_1,x),c_diff(c_1,x))));
}

static char c_atanh_doc [] =
"atanh(x)\n\
\n\
Return the hyperbolic arc tangent of x.";


static Py_complex c_cos(x)
	Py_complex x;
{
	Py_complex r;
	r.real = cos(x.real)*cosh(x.imag);
	r.imag = -sin(x.real)*sinh(x.imag);
	return r;
}

static char c_cos_doc [] =
"cos(x)\n\
\n\
Return the cosine of x.";


static Py_complex c_cosh(x)
	Py_complex x;
{
	Py_complex r;
	r.real = cos(x.imag)*cosh(x.real);
	r.imag = sin(x.imag)*sinh(x.real);
	return r;
}

static char c_cosh_doc [] =
"cosh(x)\n\
\n\
Return the hyperbolic cosine of x.";


static Py_complex c_exp(x)
	Py_complex x;
{
	Py_complex r;
	double l = exp(x.real);
	r.real = l*cos(x.imag);
	r.imag = l*sin(x.imag);
	return r;
}

static char c_exp_doc [] =
"exp(x)\n\
\n\
Return the exponential value e**x.";


static Py_complex c_log(x)
	Py_complex x;
{
	Py_complex r;
	double l = hypot(x.real,x.imag);
	r.imag = atan2(x.imag, x.real);
	r.real = log(l);
	return r;
}

static char c_log_doc [] =
"log(x)\n\
\n\
Return the natural logarithm of x.";


static Py_complex c_log10(x)
	Py_complex x;
{
	Py_complex r;
	double l = hypot(x.real,x.imag);
	r.imag = atan2(x.imag, x.real)/log(10.);
	r.real = log10(l);
	return r;
}

static char c_log10_doc [] =
"log10(x)\n\
\n\
Return the base-10 logarithm of x.";


/* internal function not available from Python */
static Py_complex c_prodi(x)
     Py_complex x;
{
	Py_complex r;
	r.real = -x.imag;
	r.imag = x.real;
	return r;
}


static Py_complex c_sin(x)
	Py_complex x;
{
	Py_complex r;
	r.real = sin(x.real)*cosh(x.imag);
	r.imag = cos(x.real)*sinh(x.imag);
	return r;
}

static char c_sin_doc [] =
"sin(x)\n\
\n\
Return the sine of x.";


static Py_complex c_sinh(x)
	Py_complex x;
{
	Py_complex r;
	r.real = cos(x.imag)*sinh(x.real);
	r.imag = sin(x.imag)*cosh(x.real);
	return r;
}

static char c_sinh_doc [] =
"sinh(x)\n\
\n\
Return the hyperbolic sine of x.";


static Py_complex c_sqrt(x)
	Py_complex x;
{
	Py_complex r;
	double s,d;
	if (x.real == 0. && x.imag == 0.)
		r = x;
	else {
		s = sqrt(0.5*(fabs(x.real) + hypot(x.real,x.imag)));
		d = 0.5*x.imag/s;
		if (x.real > 0.) {
			r.real = s;
			r.imag = d;
		}
		else if (x.imag >= 0.) {
			r.real = d;
			r.imag = s;
		}
		else {
			r.real = -d;
			r.imag = -s;
		}
	}
	return r;
}

static char c_sqrt_doc [] =
"sqrt(x)\n\
\n\
Return the square root of x.";


static Py_complex c_tan(x)
	Py_complex x;
{
	Py_complex r;
	double sr,cr,shi,chi;
	double rs,is,rc,ic;
	double d;
	sr = sin(x.real);
	cr = cos(x.real);
	shi = sinh(x.imag);
	chi = cosh(x.imag);
	rs = sr*chi;
	is = cr*shi;
	rc = cr*chi;
	ic = -sr*shi;
	d = rc*rc + ic*ic;
	r.real = (rs*rc+is*ic)/d;
	r.imag = (is*rc-rs*ic)/d;
	return r;
}

static char c_tan_doc [] =
"tan(x)\n\
\n\
Return the tangent of x.";


static Py_complex c_tanh(x)
	Py_complex x;
{
	Py_complex r;
	double si,ci,shr,chr;
	double rs,is,rc,ic;
	double d;
	si = sin(x.imag);
	ci = cos(x.imag);
	shr = sinh(x.real);
	chr = cosh(x.real);
	rs = ci*shr;
	is = si*chr;
	rc = ci*chr;
	ic = si*shr;
	d = rc*rc + ic*ic;
	r.real = (rs*rc+is*ic)/d;
	r.imag = (is*rc-rs*ic)/d;
	return r;
}

static char c_tanh_doc [] =
"tanh(x)\n\
\n\
Return the hyperbolic tangent of x.";


/* And now the glue to make them available from Python: */

static PyObject *
math_error()
{
	if (errno == EDOM)
		PyErr_SetString(PyExc_ValueError, "math domain error");
	else if (errno == ERANGE)
		PyErr_SetString(PyExc_OverflowError, "math range error");
	else    /* Unexpected math error */
		PyErr_SetFromErrno(PyExc_ValueError); 
	return NULL;
}

static PyObject *
math_1(args, func)
	PyObject *args;
	Py_complex (*func) Py_FPROTO((Py_complex));
{
	Py_complex x;
	if (!PyArg_ParseTuple(args, "D", &x))
		return NULL;
	errno = 0;
	PyFPE_START_PROTECT("complex function", return 0)
	x = (*func)(x);
	PyFPE_END_PROTECT(x)
	CHECK(x.real);
	CHECK(x.imag);
	if (errno != 0)
		return math_error();
	else
		return PyComplex_FromCComplex(x);
}

#if defined(EFI32) || defined(EFI64)
#define FUNC1(stubname, func) \
	static PyObject * stubname(PyObject *self, PyObject *args) { \
		return math_1(args, func); \
	}
#else
#define FUNC1(stubname, func) \
	static PyObject * stubname(self, args) PyObject *self, *args; { \
		return math_1(args, func); \
	}
#endif

FUNC1(cmath_acos, c_acos)
FUNC1(cmath_acosh, c_acosh)
FUNC1(cmath_asin, c_asin)
FUNC1(cmath_asinh, c_asinh)
FUNC1(cmath_atan, c_atan)
FUNC1(cmath_atanh, c_atanh)
FUNC1(cmath_cos, c_cos)
FUNC1(cmath_cosh, c_cosh)
FUNC1(cmath_exp, c_exp)
FUNC1(cmath_log, c_log)
FUNC1(cmath_log10, c_log10)
FUNC1(cmath_sin, c_sin)
FUNC1(cmath_sinh, c_sinh)
FUNC1(cmath_sqrt, c_sqrt)
FUNC1(cmath_tan, c_tan)
FUNC1(cmath_tanh, c_tanh)


static char module_doc [] =
"This module is always available. It provides access to mathematical\n\
functions for complex numbers.";


static PyMethodDef cmath_methods[] = {
	{"acos", cmath_acos, 1, c_acos_doc},
	{"acosh", cmath_acosh, 1, c_acosh_doc},
	{"asin", cmath_asin, 1, c_asin_doc},
	{"asinh", cmath_asinh, 1, c_asinh_doc},
	{"atan", cmath_atan, 1, c_atan_doc},
	{"atanh", cmath_atanh, 1, c_atanh_doc},
	{"cos", cmath_cos, 1, c_cos_doc},
	{"cosh", cmath_cosh, 1, c_cosh_doc},
	{"exp", cmath_exp, 1, c_exp_doc},
	{"log", cmath_log, 1, c_log_doc},
	{"log10", cmath_log10, 1, c_log10_doc},
	{"sin", cmath_sin, 1, c_sin_doc},
	{"sinh", cmath_sinh, 1, c_sinh_doc},
	{"sqrt", cmath_sqrt, 1, c_sqrt_doc},
	{"tan", cmath_tan, 1, c_tan_doc},
	{"tanh", cmath_tanh, 1, c_tanh_doc},
	{NULL,		NULL}		/* sentinel */
};

DL_EXPORT(void)
initcmath()
{
	PyObject *m, *d, *v;
	
	m = Py_InitModule3("cmath", cmath_methods, module_doc);
	d = PyModule_GetDict(m);
	PyDict_SetItemString(d, "pi",
			     v = PyFloat_FromDouble(atan(1.0) * 4.0));
	Py_DECREF(v);
	PyDict_SetItemString(d, "e", v = PyFloat_FromDouble(exp(1.0)));
	Py_DECREF(v);
}

#ifdef EFI_LOADABLE_MODULE
struct _inittab Efi_InitTab = { "cmath", initcmath };
#endif
