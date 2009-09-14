//iv_listhandler:
// typedef void (*iv_listhandler)(int action, int x, int y, int idx, int state);
// void OpenList(char *title, ibitmap *background, int itemw, int itemh, int itemcount, int cpos, iv_listhandler hproc);
// void OpenDummyList(char *title, ibitmap *background, char *text, iv_listhandler hproc);

%{

static PyObject *PyOpenList_pyfunc_ptr = NULL;

static void PyOpenList_callback(int action, int x, int y, int idx, int state)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;

	arglist = Py_BuildValue("(iiiii)", action, x, y, idx, state);             
	result = PyEval_CallObject(PyOpenList_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	Py_DECREF(PyOpenList_pyfunc_ptr);
	PyOpenList_pyfunc_ptr = NULL;   
	return;
}

void PyOpenList(char *title, ibitmap *background, int itemw, int itemh, int itemcount, int cpos, PyObject *pyfunc)
{
	if (PyOpenList_pyfunc_ptr) {
   		Py_DECREF(PyOpenList_pyfunc_ptr);
   		PyOpenList_pyfunc_ptr = NULL;
   	}
   	PyOpenList_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	OpenList(title, background, itemw, itemh, itemcount, cpos, PyOpenList_callback);
}

void PyOpenDummyList(char *title, ibitmap *background, char *text, PyObject* pyfunc)
{
	if (PyOpenList_pyfunc_ptr) {
   		Py_DECREF(PyOpenList_pyfunc_ptr);
   		PyOpenList_pyfunc_ptr = NULL;
   	}
   	PyOpenList_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	OpenDummyList(title, background, text, PyOpenList_callback);
}

%}

%rename(OpenList) PyOpenList;
extern void PyOpenList(char *title, ibitmap *background, int itemw, int itemh, int itemcount, int cpos, PyObject* pyfunc);
%rename(OpenDummyList) PyOpenDummyList;
extern void PyOpenDummyList(char *title, ibitmap *background, char *text, PyObject* pyfunc);
