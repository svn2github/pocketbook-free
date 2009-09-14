//iv_fontselecthandler:
// typedef void (*iv_fontselecthandler)(char *fontr, char *fontb, char *fonti, char *fontbi);
// void OpenFontSelector(char *title, char *font, int with_size, iv_fontselecthandler hproc);

%{

static PyObject *PyOpenFontSelector_pyfunc_ptr = NULL;

static void PyOpenFontSelector_callback(char *fontr, char *fontb, char *fonti, char *fontbi)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;

	arglist = Py_BuildValue("(ssss)", fontr, fontb, fonti, fontbi);             
	result = PyEval_CallObject(PyOpenFontSelector_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	Py_DECREF(PyOpenFontSelector_pyfunc_ptr);
	PyOpenFontSelector_pyfunc_ptr = NULL;   
	return;
}

void PyOpenFontSelector(char *title, char *font, int with_size, PyObject *pyfunc)
{
	if (PyOpenFontSelector_pyfunc_ptr) {
   		Py_DECREF(PyOpenFontSelector_pyfunc_ptr);
   		PyOpenFontSelector_pyfunc_ptr = NULL;
   	}
   	PyOpenFontSelector_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	OpenFontSelector(title, font, with_size, PyOpenFontSelector_callback);
}

%}

%rename(OpenFontSelector) PyOpenFontSelector;
extern void PyOpenFontSelector(char *title, char *font, int with_size, PyObject *pyfunc);
