//iv_timerproc:
// typedef void (*iv_timerproc)();
// void SetHardTimer(char *name, iv_timerproc tproc, int ms);
// void SetWeakTimer(char *name, iv_timerproc tproc, int ms);
// void ClearTimer(iv_timerproc tproc);

%{

static PyObject *PySetHardTimer_pyfunc_ptr = NULL;

static void PySetHardTimer_callback()
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;
	if (!PySetHardTimer_pyfunc_ptr)
		return ;
	arglist = Py_BuildValue("()");             
	result = PyEval_CallObject(PySetHardTimer_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
//	Py_DECREF(PySetHardTimer_pyfunc_ptr);
//	PySetHardTimer_pyfunc_ptr = NULL;   
	return;
}

void PySetHardTimer(char *name, PyObject *pyfunc, int ms)
{
	Py_INCREF(pyfunc);
	if (PySetHardTimer_pyfunc_ptr) {
   		Py_DECREF(PySetHardTimer_pyfunc_ptr);
   		PySetHardTimer_pyfunc_ptr = NULL;
   	}
   	PySetHardTimer_pyfunc_ptr = pyfunc;
   	SetHardTimer(name, PySetHardTimer_callback, ms);
}


static PyObject *PySetWeakTimer_pyfunc_ptr = NULL;

static void PySetWeakTimer_callback()
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;
	if (!PySetWeakTimer_pyfunc_ptr)
		return ;
	arglist = Py_BuildValue("()");             
	result = PyEval_CallObject(PySetWeakTimer_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
//	Py_DECREF(PySetWeakTimer_pyfunc_ptr);
//	PySetWeakTimer_pyfunc_ptr = NULL;   
	return;
}

void PySetWeakTimer(char *name, PyObject *pyfunc, int ms)
{
	Py_INCREF(pyfunc);
	if (PySetWeakTimer_pyfunc_ptr) {
   		Py_DECREF(PySetWeakTimer_pyfunc_ptr);
   		PySetWeakTimer_pyfunc_ptr = NULL;
   	}
   	PySetWeakTimer_pyfunc_ptr = pyfunc;
   	SetWeakTimer(name, PySetWeakTimer_callback, ms);
}

void PyClearTimer(PyObject *pyfunc)
{
	if (pyfunc == PySetHardTimer_pyfunc_ptr) {
		ClearTimer(PySetHardTimer_callback);
		Py_DECREF(PySetHardTimer_pyfunc_ptr);
		PySetHardTimer_pyfunc_ptr = NULL;
	}
	if (pyfunc == PySetWeakTimer_pyfunc_ptr) {
		ClearTimer(PySetWeakTimer_callback);
		Py_DECREF(PySetWeakTimer_pyfunc_ptr);
		PySetWeakTimer_pyfunc_ptr = NULL;
	}
}

%}

%rename(SetHardTimer) PySetHardTimer;
extern void PySetHardTimer(char *name, PyObject *pyfunc, int ms);
%rename(SetWeakTimer) PySetWeakTimer;
extern void PySetWeakTimer(char *name, PyObject *pyfunc, int ms);
%rename(ClearTimer) PyClearTimer;
extern void PyClearTimer(PyObject *pyfunc);
