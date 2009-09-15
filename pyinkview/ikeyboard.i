//iv_keyboardhandler:
// typedef void (*iv_keyboardhandler)(char *text);
// void OpenKeyboard(char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);
// void OpenCustomKeyboard(char *filename, char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);
//
//!: 'buffer' can be updated
//
%{

static PyObject* PyOpenKeyboard_pyfunc_ptr = NULL;
static char* PyOpenKeyboard_buffer = NULL;

static void PyOpenKeyboard_callback(char* text)
{
	PyObject *arglist;
	PyObject *result;
	int ires = 0;
	PyObject* stext = NULL; 
	arglist = Py_BuildValue("(s)", text);             
	result = PyEval_CallObject(PyOpenKeyboard_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
   	Py_DECREF(PyOpenKeyboard_pyfunc_ptr);
   	PyOpenKeyboard_pyfunc_ptr = NULL;
   	free(PyOpenKeyboard_buffer);
   	PyOpenKeyboard_buffer = NULL;

	return;
}

void PyOpenKeyboard(char *title, char* buffer, int maxlen, int flags, PyObject *pyfunc)
{
  	if (PyOpenKeyboard_pyfunc_ptr) {
   		Py_DECREF(PyOpenKeyboard_pyfunc_ptr);
   		PyOpenKeyboard_pyfunc_ptr = NULL;
   		free(PyOpenKeyboard_buffer);
   		PyOpenKeyboard_buffer = NULL;
   	}
   	PyOpenKeyboard_pyfunc_ptr = pyfunc;
   	Py_INCREF(pyfunc);
   	PyOpenKeyboard_buffer = (char*)malloc((maxlen+1) * sizeof(char));
   	strncpy(PyOpenKeyboard_buffer, buffer, maxlen);
   	PyOpenKeyboard_buffer[maxlen] = '\0';
   	OpenKeyboard(title, PyOpenKeyboard_buffer, maxlen, flags, PyOpenKeyboard_callback);
}

%}

%rename(OpenKeyboard) PyOpenKeyboard;
extern void PyOpenKeyboard(char *title, char* buffer, int maxlen, int flags, PyObject *pyfunc);

