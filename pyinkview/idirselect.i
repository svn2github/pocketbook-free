//iv_dirselecthandler:
// typedef void (*iv_dirselecthandler)(char *path);
// void OpenDirectorySelector(char *title, char *buf, int len, iv_dirselecthandler hproc);

%{

static PyObject* PyOpenDirectorySelector_pyfunc_ptr = NULL;
static char* PyOpenDirectorySelector_buffer = NULL;

static void PyOpenDirectorySelector_callback(char* path)
{
	PyObject* arglist;
	PyObject* result;

	if (!PyOpenDirectorySelector_pyfunc_ptr)
		return;

	arglist = Py_BuildValue("(s)", path);             
	result = PyEval_CallObject(PyOpenDirectorySelector_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}

	Py_DECREF(PyOpenDirectorySelector_pyfunc_ptr);
	PyOpenDirectorySelector_pyfunc_ptr = NULL;   

	free(PyOpenDirectorySelector_buffer);
	PyOpenDirectorySelector_buffer = NULL;

	return;
}

void PyOpenDirectorySelector(char* title, char* buf, int len, PyObject* pyfunc)
{
	if (PyOpenDirectorySelector_pyfunc_ptr) {
		Py_DECREF(PyOpenDirectorySelector_pyfunc_ptr);
		PyOpenDirectorySelector_pyfunc_ptr = NULL;
		free(PyOpenDirectorySelector_buffer);
		PyOpenDirectorySelector_buffer = NULL;
	}
	
	PyOpenDirectorySelector_buffer = (char*)malloc((len+1) * sizeof(char));
	//TODO: MemoryError
	strncpy(PyOpenDirectorySelector_buffer, buf, len);
	PyOpenDirectorySelector_buffer[len] = '\0';
	
	PyOpenDirectorySelector_pyfunc_ptr = pyfunc;
	Py_INCREF(PyOpenDirectorySelector_pyfunc_ptr);
	OpenDirectorySelector(title, PyOpenDirectorySelector_buffer, len, PyOpenDirectorySelector_callback);
}

%}

%rename(OpenDirectorySelector) PyOpenDirectorySelector;
extern void PyOpenDirectorySelector(char* title, char* buf, int len, PyObject* pyfunc);

