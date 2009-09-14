//iv_dialoghandler:
// typedef void (*iv_dialoghandler)(int button);
// void Dialog(int icon, char *title, char *text, char *button1, char *button2, iv_dialoghandler hproc);

%{

static PyObject *PyDialog_pyfunc_ptr = NULL;

static void PyDialog_callback(int button)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;

	arglist = Py_BuildValue("(i)", button);             
	result = PyEval_CallObject(PyDialog_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	Py_DECREF(PyDialog_pyfunc_ptr);
	PyDialog_pyfunc_ptr = NULL;   
	return;
}

void PyDialog(int icon, char *title, char *text, char *button1, char *button2, PyObject *pyfunc)
{
	if (PyDialog_pyfunc_ptr) {
   		Py_DECREF(PyDialog_pyfunc_ptr);
   		PyDialog_pyfunc_ptr = NULL;
   	}
   	PyDialog_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	Dialog(icon, title, text, button1, button2, PyDialog_callback);
}

%}

%rename(Dialog) PyDialog;
extern void PyDialog(int icon, char *title, char *text, char *button1, char *button2, PyObject *pyfunc);
