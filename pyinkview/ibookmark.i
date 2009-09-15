//iv_bmkhandler:
// typedef void (*iv_bmkhandler)(int action, int page, long long position);
// void OpenBookmarks(int page, long long position, int *bmklist, long long *poslist,
// 		int *bmkcount, int maxbmks, iv_bmkhandler hproc);
// void SwitchBookmark(int page, long long position, int *bmklist, long long *poslist,
// 		int *bmkcount, int maxbmks, iv_bmkhandler hproc);
//
//!: 'bmklist', 'poslist' and 'bmkcount' can be updated 
//

%{

static PyObject* PyOpenBookmark_pyfunc_ptr = NULL;

static void PyOpenBookmark_callback(int action, int page, long long position)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;
	PyObject* stext = NULL; 
	arglist = Py_BuildValue("(iiL)", action, page, position);             
	result = PyEval_CallObject(PyOpenBookmark_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
   	Py_DECREF(PyOpenBookmark_pyfunc_ptr);
   	PyOpenBookmark_pyfunc_ptr = NULL;
	return;
}

//void PyOpenBookmark(int page, long long position, int* bmklist, long long *poslist,
// 		int* bmkcount, int maxbmks, PyObject *pyfunc)
//{
//   if (PyOpenBookmark_pyfunc_ptr) {
//       PyErr_SetString(PyExc_RuntimeError, "PyOpenBookmark_pyfunc_ptr already called");
//       return;
//   }
//   PyOpenBookmark_pyfunc_ptr = pyfunc;
//   Py_INCREF(pyfunc);
//   OpenBookmark(page, position, bmklist, poslist, bmkcount, maxbmks, PyOpenBookmark_callback);
//}

%}

//%rename(OpenBookmark) PyOpenBookmark;
//extern void PyOpenBookmark(PyObject *pyfunc);

