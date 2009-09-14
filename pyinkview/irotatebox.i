//iv_rotatehandler:
// typedef void (*iv_rotatehandler)(int direction);
// void OpenRotateBox(iv_rotatehandler hproc);

%{

static PyObject *PyOpenRotateBox_pyfunc_ptr = NULL;

static void PyOpenRotateBox_callback(int direction)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;

	arglist = Py_BuildValue("(i)", direction);             
	result = PyEval_CallObject(PyOpenRotateBox_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	Py_DECREF(PyOpenRotateBox_pyfunc_ptr);
	PyOpenRotateBox_pyfunc_ptr = NULL;   
	return;
}

void PyOpenRotateBox(PyObject* pyfunc)
{
	if (PyOpenRotateBox_pyfunc_ptr) {
   		Py_DECREF(PyOpenRotateBox_pyfunc_ptr);
   		PyOpenRotateBox_pyfunc_ptr = NULL;
   	}
   	PyOpenRotateBox_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	OpenRotateBox(PyOpenRotateBox_callback);
}

%}

%rename(OpenRotateBox) PyOpenRotateBox;
extern void PyOpenRotateBox(PyObject* pyfunc);

