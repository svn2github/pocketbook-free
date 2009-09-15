//iv_menuhandler:
// void OpenMenu3x3(const ibitmap *mbitmap, const char *strings[9], iv_menuhandler hproc);

%{

static char** 
strings_to_stringarray(PyObject* list)
{
	char** array = NULL;
	if (PyList_Check(list)) {
		int i;
		int size = PyList_Size(list);
		array = (char**) malloc(size * sizeof(char*));
		for (i = 0; i < size; ++i) {
			PyObject* item = PyList_GetItem(list, i);
  			if (item != Py_None) {
				array[i] = SWIG_Python_str_AsChar(item);
			}
			else {
				array[i] = NULL;
			}
		}		
	}
	else {
		PyErr_SetString(PyExc_TypeError,"not a menuitem list");
		return NULL;
	}
	return array;
}



static PyObject* PyOpenMenu3x3_pyfunc_ptr = NULL;
static char** PyOpenMenu3x3_menu = NULL;

static void PyOpenMenu3x3_callback(int index)
{
	PyObject *arglist;
	PyObject *result;

	if (!PyOpenMenu3x3_pyfunc_ptr)
		return;

	arglist = Py_BuildValue("(i)", index );             
	result = PyEval_CallObject(PyOpenMenu3x3_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}

	Py_DECREF(PyOpenMenu3x3_pyfunc_ptr);
	PyOpenMenu3x3_pyfunc_ptr = NULL;   
	free(PyOpenMenu3x3_menu);
	PyOpenMenu3x3_menu = NULL;

	return;
}

void PyOpenMenu3x3(const ibitmap* mbitmap, PyObject* strings, PyObject *pyfunc)
{
	if (PyOpenMenu3x3_pyfunc_ptr) {
		Py_DECREF(PyOpenMenu3x3_pyfunc_ptr);
		PyOpenMenu3x3_pyfunc_ptr = NULL;
		free(PyOpenMenu3x3_menu);
		PyOpenMenu3x3_menu = NULL;
	}
	PyOpenMenu3x3_menu = strings_to_stringarray(strings);	//TODO: Check list size
	if (PyOpenMenu3x3_menu != NULL) {
		PyOpenMenu3x3_pyfunc_ptr = pyfunc;
		Py_INCREF(pyfunc);
		OpenMenu3x3(mbitmap, (const char**)PyOpenMenu3x3_menu, PyOpenMenu3x3_callback);
	}
}

%}

%rename(OpenMenu3x3) PyOpenMenu3x3;
extern void PyOpenMenu3x3(const ibitmap* mbitmap, PyObject* strings, PyObject *pyfunc);


