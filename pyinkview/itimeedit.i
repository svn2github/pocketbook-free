//iv_timeedithandler:
// typedef void (*iv_timeedithandler)(long newtime);
// void OpenTimeEdit(char *title, int x, int y, long intime, iv_timeedithandler hproc);

%{

static PyObject *PyOpenTimeEdit_pyfunc_ptr = NULL;

static void PyOpenTimeEdit_callback(long newtime)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;

	arglist = Py_BuildValue("(l)", newtime);             
	result = PyEval_CallObject(PyOpenTimeEdit_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	Py_DECREF(PyOpenTimeEdit_pyfunc_ptr);
	PyOpenTimeEdit_pyfunc_ptr = NULL;   
	return;
}

void PyOpenTimeEdit(char *title, int x, int y, long intime, PyObject *pyfunc)
{
	if (PyOpenTimeEdit_pyfunc_ptr) {
   		Py_DECREF(PyOpenTimeEdit_pyfunc_ptr);
   		PyOpenTimeEdit_pyfunc_ptr = NULL;
   	}
   	PyOpenTimeEdit_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	OpenTimeEdit(title, x, y, intime, PyOpenTimeEdit_callback);
}

%}

%rename(OpenTimeEdit) PyOpenTimeEdit;
extern void PyOpenTimeEdit(char *title, int x, int y, long intime, PyObject *pyfunc);
