//iv_pageselecthandler:
// typedef void (*iv_pageselecthandler)(int page);
// void OpenPageSelector(iv_pageselecthandler hproc);

%{

static PyObject* PyOpenPageSelector_pyfunc_ptr = NULL;

static void PyOpenPageSelector_callback(int page)
{
	PyObject* arglist;
	PyObject* result;
	int ires = 0;
	PyObject* stext = NULL; 
	arglist = Py_BuildValue("(i)", page);             
	result = PyEval_CallObject(PyOpenPageSelector_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
   	Py_DECREF(PyOpenPageSelector_pyfunc_ptr);
   	PyOpenPageSelector_pyfunc_ptr = NULL;
	return;
}

void PyOpenPageSelector(PyObject* pyfunc)
{
   	if (PyOpenPageSelector_pyfunc_ptr) {
   		Py_DECREF(PyOpenPageSelector_pyfunc_ptr);
   		PyOpenPageSelector_pyfunc_ptr = NULL;
   	}
   	PyOpenPageSelector_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	OpenPageSelector(PyOpenPageSelector_callback);
}

%}

%rename(OpenPageSelector) PyOpenPageSelector;
extern void PyOpenPageSelector(PyObject* pyfunc);

