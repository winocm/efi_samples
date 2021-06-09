/*
 * Author: George V. Neville-Neil
 */

#include "Python.h"

/* Our stuff... */
#include "timing.h"

#if defined(EFI32) || defined(EFI64)
static PyObject *start_timing(PyObject *self, PyObject *args);
static PyObject *finish_timing(PyObject *self, PyObject *args);
static PyObject *seconds(PyObject *self, PyObject *args);
static PyObject *milli(PyObject *self, PyObject *args);
static PyObject *micro(PyObject *self, PyObject *args);
#endif

static PyObject *
start_timing(self, args)
	PyObject *self;
	PyObject *args;
{
	if (!PyArg_Parse(args, ""))
		return NULL;

	Py_INCREF(Py_None);
	BEGINTIMING;
	return Py_None;
}

static PyObject *
finish_timing(self, args)
	PyObject *self;
	PyObject *args;
{
	if (!PyArg_Parse(args, ""))
		return NULL;

	ENDTIMING    
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
seconds(self, args)
	PyObject *self;
	PyObject *args;
{
	if (!PyArg_Parse(args, ""))
		return NULL;

	return PyInt_FromLong(TIMINGS);

}

static PyObject *
milli(self, args)
	PyObject *self;
	PyObject *args;
{
	if (!PyArg_Parse(args, ""))
		return NULL;

	return PyInt_FromLong(TIMINGMS);

}
static PyObject *
micro(self, args)
	PyObject *self;
	PyObject *args;
{
	if (!PyArg_Parse(args, ""))
		return NULL;

	return PyInt_FromLong(TIMINGUS);

}


static PyMethodDef timing_methods[] = {
	{"start",   start_timing},
	{"finish",  finish_timing},
	{"seconds", seconds},
	{"milli",   milli},
	{"micro",   micro},
	{NULL,      NULL}
};


DL_EXPORT(void) inittiming()
{
	(void)Py_InitModule("timing", timing_methods);
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module timing");
}

#ifdef EFI_LOADABLE_MODULE
struct _inittab Efi_InitTab = { "timing", inittiming };
#endif
