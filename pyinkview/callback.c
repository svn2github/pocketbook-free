#include <Python.h>
#include <stdlib.h>
#include <ffi.h>
#include "callback.h"

#define ffi_type_slonglong ffi_type_sint64
#define ffi_type_ulonglong ffi_type_uint64

static PyObject*
Python_str_FromChar(const char *c)
{
#if PY_VERSION_HEX >= 0x03000000
  return PyUnicode_FromString(c);
#else
  return PyString_FromString(c);
#endif
}

static PyObject*
convert_ffitype_to_py(const ffi_type* ffitype, const void* value)
{
	if (ffitype == &ffi_type_sint) {
		return PyLong_FromLong(*((int*)value));
	}
	else if (ffitype == &ffi_type_pointer) {
		return Python_str_FromChar((char*)value);
	}
	else {
		PyErr_SetString(PyExc_TypeError, "invalid argument type");
		printf("[%d]\n", (int)ffitype->type);
		return NULL;
	}
}

static int
convert_py_to_fftype(/*const*/PyObject* obj, const ffi_type* ffitype, ffi_arg* rvalue)
{
	if (ffitype == &ffi_type_sint) {
		// PyLong to int
		if (!PyLong_Check(obj)) {
			PyErr_SetString(PyExc_TypeError, "expected integer value");
			return -1;
		}
		long v = PyLong_AsLong(obj);
		if (v == -1 && PyErr_Occurred()) {
			PyErr_SetString(PyExc_OverflowError, "long int too long to convert");
			return -1;
		}
		*rvalue = (int)v;
		return 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError,"invalid return value type");
		return -1;
	}
}

// Generic callback. Supports only simple types.
void
generic_callback(ffi_cif* cif, void* resp, void** args, void* userdata)
{
	PyObject* pyfunc = (PyObject*)userdata;
	PyObject *arglist = NULL;
	PyObject *result = NULL;
	int i;

	// Convert arguments from C to Python.
	if (cif->nargs < 0) {
		PyErr_SetString(PyExc_OverflowError, "wrong nargs");
		goto fail;
	}

	arglist = PyTuple_New(cif->nargs);
	for (i = 0; i < cif->nargs; ++i) {
		PyObject* arg = convert_ffitype_to_py(cif->arg_types[i], args[i]);
		if (arg == NULL) {
			goto fail;
		}
		PyTuple_SetItem(arglist, i, arg);
	}
	// Call Python function.
	result = PyEval_CallObject(pyfunc, arglist);

	Py_XDECREF(arglist);

	if (result != NULL) {
		if (cif->rtype != &ffi_type_void) {
			/*int res =*/convert_py_to_fftype(result, cif->rtype, (ffi_arg*)resp);
		}
		Py_DECREF(result);
	}

fail:
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	return;
}
