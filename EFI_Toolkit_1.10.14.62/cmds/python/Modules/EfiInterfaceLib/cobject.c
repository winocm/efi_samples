/*
 * Copyright (c) 2000
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
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Copyright (c) 2000  Intel Corporation

Module Name:

    cobject.c

Abstract:

    This file implements wrappers for the functions declared in cobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyCObject_Type = NULL;

PyObject * PyCObject_FromVoidPtr (void *cobj, void (*destruct)(void*))
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCObject_FromVoidPtr) {
        return PythonApiFunctions->EPA_PyCObject_FromVoidPtr(cobj, destruct);
    } else {
        return NULL;
    }
}

PyObject * PyCObject_FromVoidPtrAndDesc (void *cobj, void *desc, void (*destruct)(void*,void*))
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCObject_FromVoidPtrAndDesc) {
        return PythonApiFunctions->EPA_PyCObject_FromVoidPtrAndDesc(cobj, desc, destruct);
    } else {
        return NULL;
    }
}

void * PyCObject_AsVoidPtr (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCObject_AsVoidPtr) {
        return PythonApiFunctions->EPA_PyCObject_AsVoidPtr(O);
    } else {
        return NULL;
    }
}

void * PyCObject_GetDesc (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCObject_GetDesc) {
        return PythonApiFunctions->EPA_PyCObject_GetDesc(O);
    } else {
        return NULL;
    }
}

void * PyCObject_Import (char *module_name, char *cobject_name)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCObject_Import) {
        return PythonApiFunctions->EPA_PyCObject_Import(module_name, cobject_name);
    } else {
        return NULL;
    }
}

EFI_STATUS InitCobject() {
    if (PythonApiFunctions) {
        pPyCObject_Type = PythonApiFunctions->EPA_PyCObject_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
