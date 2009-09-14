//iv_handler:
//
// typedef int (*iv_handler)(int type, int par1, int par2);
//
// void InkViewMain(iv_handler h);
// iv_handler SetEventHandler(iv_handler hproc);
// iv_handler GetEventHandler();
//
// void SendEvent(iv_handler hproc, int type, int par1, int par2);

%{

static PyObject *PyInkViewMain_pyfunc_ptr = NULL;

static int PyInkViewMain_callback(int type, int par1, int par2)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;
	arglist = Py_BuildValue("(iii)", type, par1, par2);             
	result = PyEval_CallObject(PyInkViewMain_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {   
		ires = PyLong_AsLong(result);
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	return ires;
}

void PyInkViewMain(PyObject *pyfunc)
{
	if (PyInkViewMain_pyfunc_ptr) {
		PyErr_SetString(PyExc_RuntimeError, "PyInkViewMain_pyfunc_ptr already called");
		return;
   	}
   	PyInkViewMain_pyfunc_ptr = pyfunc;
   	Py_INCREF(PyInkViewMain_pyfunc_ptr);
   	InkViewMain(PyInkViewMain_callback);
   	Py_DECREF(PyInkViewMain_pyfunc_ptr);
   	PyInkViewMain_pyfunc_ptr = NULL;   
}

PyObject* PySetEventHandler(PyObject* pyfunc)
{
	PyObject* prevhandler = PyInkViewMain_pyfunc_ptr;
	PyInkViewMain_pyfunc_ptr = pyfunc;
	SetEventHandler(PyInkViewMain_callback);
	Py_INCREF(pyfunc);
	if (prevhandler) {
		return prevhandler;
	}
	else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}

PyObject* PyGetEventHandler()
{
	if (PyInkViewMain_pyfunc_ptr) {
		Py_INCREF(PyInkViewMain_pyfunc_ptr);
		return PyInkViewMain_pyfunc_ptr;
	}
	else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}


static PyObject* PySendEvent_pyfunc_ptr = NULL;

static int PySendEvent_callback(int type, int par1, int par2)
{
	PyObject* arglist;
	PyObject* result;
	int ires = 0;
	arglist = Py_BuildValue("(iii)", type, par1, par2);             
	result = PyEval_CallObject(PySendEvent_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);
	if (result) {
		ires = PyLong_AsLong(result);
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	Py_DECREF(PySendEvent_pyfunc_ptr);
	PySendEvent_pyfunc_ptr = NULL;
	return ires;
}

void PySendEvent(PyObject* pyfunc, int type, int par1, int par2)
{
	if (PySendEvent_pyfunc_ptr) {
		PyErr_SetString(PyExc_RuntimeError, "PySendEvent_pyfunc_ptr already called");
		return;
   	}
	PySendEvent_pyfunc_ptr = pyfunc;
   	Py_INCREF(PySendEvent_pyfunc_ptr);
   	SendEvent(PySendEvent_callback, type, par1, par2);
}

%}

%rename(InkViewMain) PyInkViewMain;
extern void PyInkViewMain(PyObject *pyfunc);
%rename(SetEventHandler) PySetEventHandler;
extern PyObject* PySetEventHandler(PyObject* pyfunc);
%rename(GetEventHandler) PyGetEventHandler;
extern PyObject* PyGetEventHandler();
%rename(PySendEvent) PySendEvent;
extern void PySendEvent(PyObject* pyfunc, int type, int par1, int par2);
